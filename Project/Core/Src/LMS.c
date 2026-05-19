#include "LMS.h"
#include <stddef.h>

#define LMS_DEFAULT_LEAKAGE	1.0f
#define LMS_DEFAULT_EPS		0.000001f
#define LMS_ADC_MID			32768.0f
#define LMS_ADC_SCALE		32768.0f

uint16_t SampleSize = 1024;
uint16_t HalfSize = 512;

/* 清零用户提供的数组，避免使用动态内存 */
static void LMS_ClearBuff(float *Buff, uint16_t Size)
{
	if (Buff == NULL)
	{
		return;
	}

	for (uint16_t i = 0; i < Size; i ++)
	{
		Buff[i] = 0.0f;
	}
}

/* 检查LMS句柄和外部数组是否有效 */
static uint8_t LMS_Check(LMS_t *p)
{
	if ((p == NULL) || (p->Coeff == NULL) || (p->State == NULL) || (p->Order == 0))
	{
		return 0;
	}

	return 1;
}

/* 更新输入延时线，State[0]始终保存最新输入 */
static void LMS_PushInput(LMS_t *p, float Input)
{
	for (uint16_t i = p->Order - 1; i > 0; i --)
	{
		p->State[i] = p->State[i - 1];
	}

	p->State[0] = Input;
}

/* 计算当前FIR输出，同时累加输入功率用于归一化更新 */
static float LMS_CalcOut(LMS_t *p)
{
	float Out = 0.0f;
	float Power = p->Eps;

	if (Power <= 0.0f)
	{
		Power = LMS_DEFAULT_EPS;
	}

	for (uint16_t i = 0; i < p->Order; i ++)
	{
		Out += p->Coeff[i] * p->State[i];
		Power += p->State[i] * p->State[i];
	}

	p->Out = Out;
	p->Power = Power;

	return Out;
}

/* 将16位ADC采样转换到约-1.0f到1.0f的浮点范围 */
static float LMS_AdcToFloat(uint16_t ADC_Val)
{
	return ((float)ADC_Val - LMS_ADC_MID) / LMS_ADC_SCALE;
}

/* 初始化LMS滤波器，系数和状态数组由调用者在外部分配 */
void LMS_Init(LMS_t *p, float *Coeff, float *State, uint16_t Order, float Mu)
{
	if (p == NULL)
	{
		return;
	}

	if (Mu < 0.0f)
	{
		Mu = 0.0f;
	}

	p->Mu = Mu;
	p->Leakage = LMS_DEFAULT_LEAKAGE;
	p->Eps = LMS_DEFAULT_EPS;
	p->Out = 0.0f;
	p->Error = 0.0f;
	p->Power = 0.0f;
	p->Order = Order;
	p->Coeff = Coeff;
	p->State = State;

	LMS_ClearBuff(p->Coeff, p->Order);
	LMS_ClearBuff(p->State, p->Order);
}

/* 复位滤波器状态和系数，保留阶数、步长等配置 */
void LMS_Reset(LMS_t *p)
{
	if (p == NULL)
	{
		return;
	}

	p->Out = 0.0f;
	p->Error = 0.0f;
	p->Power = 0.0f;

	LMS_ClearBuff(p->Coeff, p->Order);
	LMS_ClearBuff(p->State, p->Order);
}

/* 设置LMS参数，非法参数会回退到安全默认值 */
void LMS_SetPara(LMS_t *p, float Mu, float Leakage, float Eps)
{
	if (p == NULL)
	{
		return;
	}

	if (Mu < 0.0f)
	{
		Mu = 0.0f;
	}

	if ((Leakage <= 0.0f) || (Leakage > 1.0f))
	{
		Leakage = LMS_DEFAULT_LEAKAGE;
	}

	if (Eps <= 0.0f)
	{
		Eps = LMS_DEFAULT_EPS;
	}

	p->Mu = Mu;
	p->Leakage = Leakage;
	p->Eps = Eps;
}

/* 只运行当前滤波器，不更新系数，适合训练完成后的实时输出 */
float LMS_Run(LMS_t *p, float Input)
{
	if (LMS_Check(p) == 0)
	{
		return 0.0f;
	}

	LMS_PushInput(p, Input);
	p->Error = 0.0f;

	return LMS_CalcOut(p);
}

/* 单点归一化LMS更新：输入Input，期望输出Ref，返回滤波输出 */
float LMS_Update(LMS_t *p, float Input, float Ref)
{
	float Step = 0.0f;
	float Out = 0.0f;

	if (LMS_Check(p) == 0)
	{
		return 0.0f;
	}

	LMS_PushInput(p, Input);
	Out = LMS_CalcOut(p);

	p->Error = Ref - Out;
	/* 归一化步长，输入幅值变化时也能保持较稳定的收敛速度 */
	Step = p->Mu * p->Error / p->Power;

	for (uint16_t i = 0; i < p->Order; i ++)
	{
		/* Leakage小于1时会让旧系数缓慢衰减，减少长期漂移 */
		p->Coeff[i] = p->Leakage * p->Coeff[i] + Step * p->State[i];
	}

	return Out;
}

/* 对一段浮点数据连续训练，Out和Error允许传入NULL */
void LMS_UpdateBlock(LMS_t *p, float *Input, float *Ref, float *Out,
                     float *Error, uint16_t Size)
{
	float TempOut = 0.0f;

	if ((p == NULL) || (Input == NULL) || (Ref == NULL))
	{
		return;
	}

	for (uint16_t i = 0; i < Size; i ++)
	{
		TempOut = LMS_Update(p, Input[i], Ref[i]);

		if (Out != NULL)
		{
			Out[i] = TempOut;
		}

		if (Error != NULL)
		{
			Error[i] = p->Error;
		}
	}
}

/* 对一段16位ADC采样连续训练，内部先将ADC值转换为浮点数 */
void LMS_UpdateAdcBlock(LMS_t *p, uint16_t *Input, uint16_t *Ref, float *Out,
                        float *Error, uint16_t Size)
{
	float InputVal = 0.0f;
	float RefVal = 0.0f;
	float TempOut = 0.0f;

	if ((p == NULL) || (Input == NULL) || (Ref == NULL))
	{
		return;
	}

	for (uint16_t i = 0; i < Size; i ++)
	{
		InputVal = LMS_AdcToFloat(Input[i]);
		RefVal = LMS_AdcToFloat(Ref[i]);
		TempOut = LMS_Update(p, InputVal, RefVal);

		if (Out != NULL)
		{
			Out[i] = TempOut;
		}

		if (Error != NULL)
		{
			Error[i] = p->Error;
		}
	}
}

FilterType_t Filter_Learn(void)
{
	float LowSum = 0.0f;          // 低频段增益累加值
	float HighSum = 0.0f;         // 高频段增益累加值
	float MaxGain = 0.0f;         // 扫频过程中记录的最大增益
	float MinGain = 1000000.0f;   // 扫频过程中记录的最小增益
	uint16_t MaxIndex = 0;        // 最大增益对应的扫频点下标
	uint16_t MinIndex = 0;        // 最小增益对应的扫频点下标
	uint16_t PointNum = Sweep_Num - 1;  // 实际扫频点数
	uint16_t EdgeNum = PointNum / 10;   // 用前后10%的点估计低频段/高频段增益

	if (EdgeNum == 0)
	{
		EdgeNum = 1;
	}

	/* 正弦扫频，记录每个频点的输入/输出幅值 */
	for (uint32_t i = 1; i < Sweep_Num; i ++)
	{
		uint32_t Freq = i * 1000;
		AD98332_SetFrequencyQuick(Freq, AD9833_OUT_SINUS);
		HAL_Delay(5);

		/* ADC1采滤波器输入，ADC2采滤波器输出 */
		HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC1_Buff, SampleSize);
		HAL_ADC_Start_DMA(&hadc2, (uint32_t *)ADC2_Buff, SampleSize);
		HAL_TIM_Base_Start(&htim3);

		while ((adc1_done == 0) || (adc2_done == 0))
		{
			
		}
		
		HAL_TIM_Base_Stop(&htim3);
		adc1_done = 0;
		adc2_done = 0;

		Math_PreFFT(ADC1_Buff, ADC1_FFTIN, SampleSize);
		Math_PreFFT(ADC2_Buff, ADC2_FFTIN, SampleSize);

		/* 单一正弦输入时，FFT最大频点就是当前信号幅值 */
		arm_cfft_f32(&cfft_handler, ADC1_FFTIN, 0, 1);
        arm_cmplx_mag_f32(ADC1_FFTIN, ADC1_FFTOUT, HalfSize);
		arm_cfft_f32(&cfft_handler, ADC2_FFTIN, 0, 1);
        arm_cmplx_mag_f32(ADC2_FFTIN, ADC2_FFTOUT, HalfSize);

		ADC1_Amp[i - 1] = Math_GetFreqAmp(ADC1_FFTOUT, SampleSize);
		ADC2_Amp[i - 1] = Math_GetFreqAmp(ADC2_FFTOUT, SampleSize);

		printf("%f,%f\r\n", ADC1_Amp[i - 1], ADC2_Amp[i - 1]);

	}

	/* 构建增益曲线 Gain(f) = Vout(f) / Vin(f) */
	for (uint16_t i = 0; i < PointNum; i ++)
	{
		float Gain = 0.0f;

		if (ADC1_Amp[i] > 0.000001f)
		{
			Gain = ADC2_Amp[i] / ADC1_Amp[i];
		}

		if (i < EdgeNum)
		{
			LowSum += Gain;
		}

		if (i >= (PointNum - EdgeNum))
		{
			HighSum += Gain;
		}

		if (Gain > MaxGain)
		{
			MaxGain = Gain;
			MaxIndex = i;
		}

		if (Gain < MinGain)
		{
			MinGain = Gain;
			MinIndex = i;
		}
	}

	float LowGain = LowSum / EdgeNum;
	float HighGain = HighSum / EdgeNum;
	uint16_t MidStart = PointNum / 4;
	uint16_t MidEnd = (PointNum * 3) / 4;

	/* 根据低/高频增益和中间峰值/谷值判断滤波器类型 */
	if ((MaxIndex >= MidStart) && (MaxIndex <= MidEnd) &&
		(MaxGain > LowGain * 2.0f) && (MaxGain > HighGain * 2.0f))
	{
		return Filter_BandPass;
	}

	if ((MinIndex >= MidStart) && (MinIndex <= MidEnd) &&
		(LowGain > MinGain * 2.0f) && (HighGain > MinGain * 2.0f))
	{
		return Filter_BandStop;
	}

	if (LowGain > HighGain * 2.0f)
	{
		return Filter_LowPass;
	}

	if (HighGain > LowGain * 2.0f)
	{
		return Filter_HighPass;
	}

	return Filter_Unknown;
}
