/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef hspi1;

extern SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_SPI1_Init(void);
void MX_SPI2_Init(void);

/* USER CODE BEGIN Prototypes */

/* --- OLED SSD1309 驱动接口 (SPI) --- */

/* OLED 初始化 */
void OLED_Init(void);

/* OLED 清屏 (写入全0) */
void OLED_Clear(void);

/* OLED 全屏刷新 (将缓冲区内容发送到OLED) */
void OLED_Update(void);

/* OLED 在 (x, y) 处显示一个 6x8 字符, x 范围 0~20, y 范围 0~7 (page) */
void OLED_PutChar(uint8_t x, uint8_t y, char c);

/* OLED 在 (x, y) 处显示字符串 (自动换行) */
void OLED_PrintString(uint8_t x, uint8_t y, const char *str);

/* 打印有符号整数 */
void OLED_PrintInt(uint8_t x, uint8_t y, int32_t num);

/* 打印浮点数 (通过 ftoa, precision 位小数) */
void OLED_PrintFloat(uint8_t x, uint8_t y, float num, uint8_t precision);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */

