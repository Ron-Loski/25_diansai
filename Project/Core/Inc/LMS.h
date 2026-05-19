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

extern volatile uint8_t adc_done;

extern uint16_t ADC1_Buff[];
extern float ADC1_FFTIN[];
extern float ADC1_FFTOUT[];
extern float ADC1_Amp[];		//ADC1扫频各个频率分量的幅值

extern uint16_t ADC2_Buff[];
extern float ADC2_FFTIN[];
extern float ADC2_FFTOUT[];
extern float ADC2_Amp[];    	//ADC2扫频各个频率分量的幅值

FilterType_t Filter_Learn(void);

#endif
