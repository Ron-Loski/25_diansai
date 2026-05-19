#ifndef __LMS_H_
#define __LMS_H_

#include "stm32h7xx_hal.h"
#include "tim.h"
#include "adc.h"
#include "My_Math.h"
#include "AD9833.h"

#define Sweep_Num	50

typedef enum {
	Filter_Unknown = 0,
	Filter_LowPass,
	Filter_HighPass,
	Filter_BandPass,
	Filter_BandStop
} FilterType_t;

typedef struct {
	float Mu;			// LMS步长，越大收敛越快，但过大容易振荡
	float Leakage;		// 泄漏因子，1.0f表示系数不衰减
	float Eps;			// 归一化分母保护值，防止除0
	float Out;			// 最近一次滤波输出
	float Error;		// 最近一次误差，Error = Ref - Out
	float Power;		// 最近一次输入功率估计
	uint16_t Order;		// 滤波器阶数
	float *Coeff;		// 滤波器系数数组，由外部提供，长度为Order
	float *State;		// 输入延时状态数组，由外部提供，长度为Order
}LMS_t;

extern volatile uint8_t adc_done;

extern uint16_t ADC1_Buff[];
extern float ADC1_FFTIN[];
extern float ADC1_FFTOUT[];
extern float ADC1_Amp[];		//ADC1扫频各个频率分量的幅值

extern uint16_t ADC2_Buff[];
extern float ADC2_FFTIN[];
extern float ADC2_FFTOUT[];
extern float ADC2_Amp[];    	//ADC2扫频各个频率分量的幅值

extern volatile uint8_t adc1_done;	//ADC转换完成标志位
extern volatile uint8_t adc2_done;	//ADC2转换完成标志位

/* LMS滤波器接口：不使用动态内存，Coeff和State由调用者分配 */
void LMS_Init(LMS_t *p, float *Coeff, float *State, uint16_t Order, float Mu);
void LMS_Reset(LMS_t *p);
void LMS_SetPara(LMS_t *p, float Mu, float Leakage, float Eps);
float LMS_Run(LMS_t *p, float Input);
float LMS_Update(LMS_t *p, float Input, float Ref);
void LMS_UpdateBlock(LMS_t *p, float *Input, float *Ref, float *Out,
                     float *Error, uint16_t Size);
void LMS_UpdateAdcBlock(LMS_t *p, uint16_t *Input, uint16_t *Ref, float *Out,
                        float *Error, uint16_t Size);
FilterType_t Filter_Learn(void);

#endif
