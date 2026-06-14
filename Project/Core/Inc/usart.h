/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#define CMD_LINE_SIZE 64

extern char cmd_line[CMD_LINE_SIZE];
extern uint16_t cmd_index;
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* USER CODE BEGIN Private defines */
void UART_HandleLine(char *Line);
void UART_ParaData(uint8_t *Data, uint16_t len);
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void USART_Send(uint8_t *Buff, uint32_t Size);
void FPGA_Send_FreqWord(uint32_t word);
void FPGA_Send_FreqHz(uint32_t hz);
void FPGA_Set_AmpPermille(uint16_t permille);

#define hz(x)  FPGA_Send_FreqHz((uint32_t)((uint64_t)(x)))
#define khz(x) FPGA_Send_FreqHz((uint32_t)((uint64_t)(x) * 1000ULL))
#define mhz(x) FPGA_Send_FreqHz((uint32_t)((uint64_t)(x) * 1000000ULL))
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

