#include "LMS.h"

uint16_t SampleSize = 1024;
uint16_t HalfSize = 512;

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

		HAL_TIM_Base_Stop(&htim3);
		
		Math_PreFFT(ADC1_Buff, ADC1_FFTIN, SampleSize);
		Math_PreFFT(ADC2_Buff, ADC2_FFTIN, SampleSize);

		/* 单一正弦输入时，FFT最大频点就是当前信号幅值 */
		arm_cfft_f32(&cfft_handler, ADC1_FFTIN, 0, 1);
        arm_cmplx_mag_f32(ADC1_FFTIN, ADC1_FFTOUT, HalfSize);
		arm_cfft_f32(&cfft_handler, ADC2_FFTIN, 0, 1);
        arm_cmplx_mag_f32(ADC2_FFTIN, ADC2_FFTOUT, HalfSize);

		ADC1_Amp[i - 1] = Math_GetFreqAmp(ADC1_FFTOUT, SampleSize);
		ADC2_Amp[i - 1] = Math_GetFreqAmp(ADC2_FFTOUT, SampleSize);
				
		printf("%f,%f\r\n", ADC1_Amp[i - 1], ADC1_Amp[i - 1]);

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
