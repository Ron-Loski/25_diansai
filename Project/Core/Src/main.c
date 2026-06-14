/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "arm_math.h"
#include "math.h"
#include "AD9833.h"
#include "My_Math.h"
#include "LMS.h"
#include "PID.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define	Sample_Size		1024
#define Half_size  		512
#define Double_size  		2048
#define USART1_Rx_DMA_Size	128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
arm_cfft_instance_f32 cfft_handler;
arm_cfft_instance_f32 cfft_handler_512;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t  USART1_RxBuff[USART1_Rx_DMA_Size];	//串口接收缓冲数组

volatile uint8_t adc1_done = 0;	//ADC转换完成标志位
volatile uint8_t adc2_done = 0;	//ADC2转换完成标志位
static uint32_t Base1_CNT = 1;

volatile uint8_t FilterType = 0;	//滤波器类型

uint16_t ADC1_Buff[Sample_Size] = {0};
float ADC1_FFTIN[Double_size] = {0};
float ADC1_FFTOUT[Half_size] = {0};
float ADC1_Amp[Sweep_Num - 1] = {0};		//ADC1扫频各个频率分量的幅值

uint16_t ADC2_Buff[Sample_Size] = {0};
float ADC2_FFTIN[Double_size] = {0};
float ADC2_FFTOUT[Half_size] = {0};
float ADC2_Amp[Sweep_Num - 1] = {0};		//ADC2扫频各个频率分量的幅值

PID_t Base1_Require = {
	.Kp = 0,
	.Ki = 0,
	.Kd = 0,
};

uint16_t maxindex1 = 0;					//测试变量
float maxamp1 = 0;
uint16_t maxindex2 = 0;
float maxamp2 = 0;
uint16_t FREQ1;
uint16_t FREQ2;
float AMP1;
float AMP2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t Flag = 0;

FilterType_t Filter_Learn(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_ADC2_Init();
  MX_DAC1_Init();
  /* USER CODE BEGIN 2 */
  arm_cfft_init_f32(&cfft_handler, Sample_Size);
  arm_cfft_init_f32(&cfft_handler_512, Sample_Size / 2);
  
  AD98331_Init();
  AD98332_Init();
  
  PID_Init(&Base1_Require);

  khz(10);
  FPGA_Set_AmpPermille(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, USART1_RxBuff, USART1_Rx_DMA_Size);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  
//  AD98332_SetFrequencyQuick(1000, AD9833_OUT_SINUS);
//  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 400);
//  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
//  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC1_Buff, Sample_Size);
//  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)ADC2_Buff, Sample_Size);
//  HAL_TIM_Base_Start(&htim3);

  uint8_t Type = Math_JudegeFilter();

  while (1)
  {
//	  if (adc1_done == 1  && adc2_done == 1)
//	  {
//		  HAL_TIM_Base_Stop(&htim3);
//		  
//		  adc1_done = 0;
//		  adc2_done = 0;		  		  
//		  
//		  
//		  Math_PreFFT(ADC1_Buff, ADC1_FFTIN, Sample_Size);
//		  Math_PreFFT(ADC2_Buff, ADC2_FFTIN, Sample_Size);
//		  
//		  arm_cfft_f32(&cfft_handler, ADC1_FFTIN, 0, 1);
//		  arm_cfft_f32(&cfft_handler, ADC2_FFTIN, 0, 1);
//		  
//		  arm_cmplx_mag_f32(ADC1_FFTIN, ADC1_FFTOUT, Sample_Size / 2);
//		  arm_cmplx_mag_f32(ADC2_FFTIN, ADC2_FFTOUT, Sample_Size / 2);
//  
//		  
//		  maxindex1 = 0;
//		  maxamp1 = 0;
//		  maxindex2 = 0;
//		  maxamp2 = 0;
//		  
//		  for (uint16_t i = 1; i < Sample_Size / 2; i ++)
//		  {
//			  if (ADC1_FFTOUT[i] > maxamp1){
//				  maxamp1 = ADC1_FFTOUT[i];
//				  maxindex1 = i;
//			  }
//			  if (ADC2_FFTOUT[i] > maxamp2){
//				  maxamp2 = ADC2_FFTOUT[i];
//				  maxindex2 = i;
//			  }
//			  printf("%f,%f\r\n", ADC1_FFTOUT[i], ADC2_FFTOUT[i]);
//		  }
//		  FREQ1 = 1024000 * maxindex1 / Sample_Size;
//		  FREQ2 = 1024000 * maxindex2 / Sample_Size;
//		  AMP1 = 4.0f * ADC1_FFTOUT[maxindex1] / Sample_Size;
//		  AMP2 = 4.0f * ADC2_FFTOUT[maxindex2] / Sample_Size;
//	  	    
//		  if (!Flag)
//		  {
//			  Flag ++;
//			  float Value = (400.0f * log10(2.0f / AMP2) + 150.0f) / 3300.0f * 4095.0f;
//			  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Value);
//			  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
//		  }
////		  float Error_Voltage = AMP2 - 2.0f;
////		  Base1_Require.Actual = Error_Voltage;
////		  PID_Update(&Base1_Require);
////		  
////		  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
////		  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Base1_Require.Out);
////		  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
//		  
//		  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC1_Buff, Sample_Size);
//		  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)ADC2_Buff, Sample_Size);
//		  HAL_TIM_Base_Start(&htim3);
//	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    khz(10);
    FPGA_Set_AmpPermille(1000);//100%
    HAL_Delay(2000);

    khz(20);
    FPGA_Set_AmpPermille(500);//50%
    HAL_Delay(2000);

    khz(30);
    FPGA_Set_AmpPermille(250);//25%
    HAL_Delay(2000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI2;
  PeriphClkInitStruct.PLL2.PLL2M = 2;
  PeriphClkInitStruct.PLL2.PLL2N = 12;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim3){
		
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc == &hadc1){	
		adc1_done = 1;
	}
	if (hadc == &hadc2){
		adc2_done = 1;
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart == &huart1)
	{
		UART_ParaData(USART1_RxBuff, Size);
		
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1, USART1_RxBuff, USART1_Rx_DMA_Size);
		__HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
	}
}

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
