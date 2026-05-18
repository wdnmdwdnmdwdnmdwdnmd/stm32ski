/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usart.c
  * @brief          : printf redirect to USART1
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
#include "usart.h"

/* USER CODE BEGIN 0 */
/* printf 重定向到 USART1 — 使用 __io_putchar (newlib-nano / GCC) */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

/* 补充: _write 实现(兼容部分 toolchain, 起效后会走这里而不是 __io_putchar) */
int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

#include <stdbool.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * ftoa() - 浮点数转字符串, 替代 printf("%f")
 * 输出格式: 整数部分.小数部分  (无科学计数法, 不依赖 malloc/newlib float)
 * 不支持 NaN/Inf, 精度固定 6 位小数
 * -------------------------------------------------------------------------- */
char *ftoa(float value, char *buf, int precision)
{
    if (precision < 0) precision = 0;
    if (precision > 9)  precision = 9;

    /* 处理 0 和负零 */
    if (value == 0.0f) {
        buf[0] = (*(uint32_t *)&value == 0x80000000UL) ? '-' : '0'; /* 负零 */
        if (buf[0] != '-') buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    char *p = buf;

    /* 符号 */
    if (value < 0.0f) {
        *p++ = '-';
        value = -value;
    }

    /* 整数部分 */
    int32_t int_part = (int32_t)value;
    float   frac     = value - (float)int_part;

    /* 处理小数进位: 当 frac 非常接近 1.0 时向上进位 */
    if (frac >= 0.9999999f) {
        int_part++;
        frac = 0.0f;
    }

    /* 整数部分 → 字符串 (逆序) */
    char tmp[12];
    int  idx = 0;
    if (int_part == 0) {
        tmp[idx++] = '0';
    } else {
        uint32_t u = (uint32_t)int_part; /* 已确保非负 */
        while (u > 0) {
            tmp[idx++] = '0' + (u % 10);
            u /= 10;
        }
    }
    while (idx > 0) *p++ = tmp[--idx];

    /* 小数部分 */
    if (precision > 0) {
        *p++ = '.';
        for (int i = 0; i < precision; i++) {
            frac *= 10.0f;
            int digit = (int)frac;
            *p++ = '0' + digit;
            frac -= (float)digit;
        }
    }
    *p = '\0';
    return buf;
}

/* --------------------------------------------------------------------------
 * print_float() - 便捷宏, 通过 printf("%s") 打印浮点数, 例如:
 *     print_float(3.14);   → 输出 "3.140000"
 *     print_float(-2.7, 3); → 输出 "-2.700"
 * -------------------------------------------------------------------------- */
void print_float(float value, int precision)
{
    char buf[32];
    printf("%s", ftoa(value, buf, precision));
}

/* -- 数值打印辅助 (不依赖 printf 浮点, Flash 开销极小) ------------ */
void print_int(int32_t value)
{
    printf("%ld", (long)value);
}

void print_uint(uint32_t value)
{
    printf("%lu", (unsigned long)value);
}

/* USER CODE END 1 */

