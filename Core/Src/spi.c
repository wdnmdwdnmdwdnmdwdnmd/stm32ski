/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances and SSD1309 OLED driver.
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
#include "spi.h"

/* USER CODE BEGIN 0 */

/* ==================================================================
 * SSD1309 OLED 128x64 SPI 驱动
 * 引脚: CS-PB0  DC-PA4  RES-PA3  SDA-PA7(MOSI)  SCL-PA5(SCK)
 * ================================================================== */

/* --- 引脚宏, 已在 main.h 中定义: ---
   OLED_CS_Pin  / OLED_CS_GPIO_Port   (PB0)
   OLED_DC_Pin  / OLED_DC_GPIO_Port   (PA4)
   OLED_RES_Pin / OLED_RES_GPIO_Port  (PA3)                          */

#define OLED_CS_LOW()   HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
#define OLED_CS_HIGH()  HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
#define OLED_DC_CMD()   HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)
#define OLED_DC_DATA()  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define OLED_RES_LOW()  HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_RESET)
#define OLED_RES_HIGH() HAL_GPIO_WritePin(OLED_RES_GPIO_Port, OLED_RES_Pin, GPIO_PIN_SET)

/* --- 帧缓冲区: 128col x 8page = 1024 字节 --- */
static uint8_t OLED_FrameBuffer[8][128];

/* ==================================================================
 * 底层 SPI 写入
 * ================================================================== */

/* 写一个字节 (命令或数据) */
static void OLED_SPI_Write(uint8_t val)
{
    HAL_SPI_Transmit(&hspi1, &val, 1, 10);
}

/* 写命令 */
static void OLED_WriteCmd(uint8_t cmd)
{
    OLED_DC_CMD();
    OLED_CS_LOW();
    OLED_SPI_Write(cmd);
    OLED_CS_HIGH();
}

/* 写数据 */
static void OLED_WriteData(uint8_t dat)
{
    OLED_DC_DATA();
    OLED_CS_LOW();
    OLED_SPI_Write(dat);
    OLED_CS_HIGH();
}

/* ==================================================================
 * 6x8 字体 (ASCII 32~126) 每字符 6 字节
 * ================================================================== */
static const uint8_t Font6x8[][6] = {
    {0x00,0x00,0x00,0x00,0x00,0x00}, /*   space */
    {0x00,0x00,0x00,0x2f,0x00,0x00}, /* ! */
    {0x00,0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x00,0x14,0x7f,0x14,0x7f,0x14}, /* # */
    {0x00,0x24,0x2a,0x7f,0x2a,0x12}, /* $ */
    {0x00,0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x00,0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x00,0x1c,0x22,0x41,0x00}, /* ( */
    {0x00,0x00,0x41,0x22,0x1c,0x00}, /* ) */
    {0x00,0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x00,0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x00,0x00,0xA0,0x60,0x00}, /* , */
    {0x00,0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x00,0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x00,0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x00,0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x00,0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x00,0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x00,0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x00,0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x00,0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x00,0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x00,0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x00,0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x00,0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x00,0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x00,0x32,0x49,0x59,0x51,0x3E}, /* @ */
    {0x00,0x7C,0x12,0x11,0x12,0x7C}, /* A */
    {0x00,0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x00,0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x00,0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x00,0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x00,0x7F,0x09,0x09,0x09,0x01}, /* F */
    {0x00,0x3E,0x41,0x49,0x49,0x7A}, /* G */
    {0x00,0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x00,0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x00,0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x00,0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x00,0x7F,0x02,0x0C,0x02,0x7F}, /* M */
    {0x00,0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x00,0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x00,0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x00,0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x00,0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x00,0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x00,0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x00,0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x00,0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x00,0x3F,0x40,0x38,0x40,0x3F}, /* W */
    {0x00,0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x00,0x07,0x08,0x70,0x08,0x07}, /* Y */
    {0x00,0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x00,0x7F,0x41,0x41,0x00}, /* [ */
    {0x00,0x55,0x2A,0x55,0x2A,0x55}, /* 55 */
    {0x00,0x00,0x41,0x41,0x7F,0x00}, /* ] */
    {0x00,0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x00,0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x00,0x01,0x02,0x04,0x00}, /* ' */
    {0x00,0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x00,0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x00,0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x00,0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x00,0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x00,0x08,0x7E,0x09,0x01,0x02}, /* f */
    {0x00,0x18,0xA4,0xA4,0xA4,0x7C}, /* g */
    {0x00,0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x00,0x40,0x80,0x84,0x7D,0x00}, /* j */
    {0x00,0x7F,0x10,0x28,0x44,0x00}, /* k */
    {0x00,0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x00,0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x00,0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x00,0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x00,0xFC,0x24,0x24,0x24,0x18}, /* p */
    {0x00,0x18,0x24,0x24,0x28,0xFC}, /* q */
    {0x00,0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x00,0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x00,0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x00,0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x00,0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x00,0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x00,0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x00,0x1C,0xA0,0xA0,0xA0,0x7C}, /* y */
    {0x00,0x44,0x64,0x54,0x4C,0x44}, /* z */
    {0x00,0x00,0x06,0x09,0x09,0x06}, /* 123 */
};

/* ==================================================================
 * OLED 初始化
 * ================================================================== */
void OLED_Init(void)
{
    /* 硬件复位 */
    OLED_RES_LOW();
    HAL_Delay(10);
    OLED_RES_HIGH();
    HAL_Delay(10);

    /* SSD1309 初始化序列 */
    OLED_WriteCmd(0xAE); /* Display OFF */

    OLED_WriteCmd(0xD5); /* Set Display Clock Divide Ratio / Oscillator Frequency */
    OLED_WriteCmd(0x80); /* Default */

    OLED_WriteCmd(0xA8); /* Set Multiplex Ratio */
    OLED_WriteCmd(0x3F); /* 64 lines */

    OLED_WriteCmd(0xD3); /* Set Display Offset */
    OLED_WriteCmd(0x00); /* 0 */

    OLED_WriteCmd(0x40); /* Set Display Start Line */

    OLED_WriteCmd(0x8D); /* Charge Pump Setting */
    OLED_WriteCmd(0x14); /* Enable charge pump */

    OLED_WriteCmd(0x20); /* Set Memory Addressing Mode */
    OLED_WriteCmd(0x00); /* Horizontal mode */

    OLED_WriteCmd(0xA1); /* Set Segment Re-Map (column 127 mapped to SEG0) */

    OLED_WriteCmd(0xC8); /* COM Output Scan Direction (remap mode) */

    OLED_WriteCmd(0xDA); /* Set COM Pins Hardware Configuration */
    OLED_WriteCmd(0x12); /* Alternative pin config */

    OLED_WriteCmd(0x81); /* Set Contrast Control */
    OLED_WriteCmd(0xCF); /* Bright */

    OLED_WriteCmd(0xD9); /* Set Pre-charge Period */
    OLED_WriteCmd(0xF1);

    OLED_WriteCmd(0xDB); /* Set VCOMH Deselect Level */
    OLED_WriteCmd(0x40);

    OLED_WriteCmd(0xA4); /* Entire Display ON (resume to RAM content) */

    OLED_WriteCmd(0xA6); /* Set Normal Display (not inverted) */

    OLED_WriteCmd(0x2E); /* Deactivate scrolling */

    OLED_WriteCmd(0xAF); /* Display ON */

    /* 清空缓冲区 */
    OLED_Clear();
    OLED_Update();
}

/* ==================================================================
 * OLED 清屏 (帧缓冲区写0)
 * ================================================================== */
void OLED_Clear(void)
{
    for (uint8_t y = 0; y < 8; y++) {
        for (uint8_t x = 0; x < 128; x++) {
            OLED_FrameBuffer[y][x] = 0x00;
        }
    }
}

/* ==================================================================
 * OLED 全屏刷新 (缓冲区 → OLED GDDRAM)
 * ================================================================== */
void OLED_Update(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page);   /* Set page address */
        OLED_WriteCmd(0x00);          /* Set lower column start address */
        OLED_WriteCmd(0x10);          /* Set higher column start address */

        /* 写一页 128 字节数据 */
        OLED_DC_DATA();
        OLED_CS_LOW();
        HAL_SPI_Transmit(&hspi1, OLED_FrameBuffer[page], 128, 100);
        OLED_CS_HIGH();
    }
}

/* ==================================================================
 * OLED 在 (x, y) 处显示一个 6x8 字符
 * x: 0~20 (列, 每个字符占6像素), y: 0~7 (页)
 * ================================================================== */
void OLED_PutChar(uint8_t x, uint8_t y, char c)
{
    uint8_t col = x * 6;  /* 起始像素列 */

    /* 超出边界保护 */
    if (col > 121) return;
    if (y > 7) return;

    if (c < ' ' || c > 'z') c = ' '; /* 无效字符显示空格 */
    const uint8_t *font = Font6x8[c - ' '];

    for (uint8_t i = 0; i < 6; i++) {
        OLED_FrameBuffer[y][col + i] = font[i];
    }
}

/* ==================================================================
 * OLED 在 (x, y) 处显示字符串
 * ================================================================== */
void OLED_PrintString(uint8_t x, uint8_t y, const char *str)
{
    while (*str) {
        OLED_PutChar(x, y, *str);
        str++;
        x++;
        if (x > 20) {   /* 自动换行 */
            x = 0;
            y++;
            if (y > 7) y = 0;
        }
    }
}

/* ==================================================================
 * OLED 打印有符号整数 (简易 itoa, 不依赖 printf)
 * ================================================================== */
void OLED_PrintInt(uint8_t x, uint8_t y, int32_t num)
{
    char buf[12];
    int  idx = 0;
    uint8_t neg = 0;

    if (num < 0) {
        neg = 1;
        num = -num;
    }

    if (num == 0) {
        buf[idx++] = '0';
    } else {
        uint32_t u = (uint32_t)num;
        while (u > 0) {
            buf[idx++] = '0' + (u % 10);
            u /= 10;
        }
    }

    if (neg) buf[idx++] = '-';

    /* 反转 */
    for (int i = idx - 1; i >= 0; i--) {
        OLED_PutChar(x, y, buf[i]);
        x++;
        if (x > 20) { x = 0; y++; if (y > 7) y = 0; }
    }
}

/* ==================================================================
 * OLED 打印浮点数 (复用 usart.c 中的 ftoa)
 * ================================================================== */
/* 声明外部 ftoa (定义在 usart.c) */
extern char *ftoa(float value, char *buf, int precision);

void OLED_PrintFloat(uint8_t x, uint8_t y, float num, uint8_t precision)
{
    char buf[32];
    ftoa(num, buf, (int)precision);
    OLED_PrintString(x, y, buf);
}

/* USER CODE END 0 */

SPI_HandleTypeDef hspi1;

/* SPI1 init function */
void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspInit 0 */

  /* USER CODE END SPI1_MspInit 0 */
    /* SPI1 clock enable */
    __HAL_RCC_SPI1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

  if(spiHandle->Instance==SPI1)
  {
  /* USER CODE BEGIN SPI1_MspDeInit 0 */

  /* USER CODE END SPI1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI1_CLK_DISABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

  /* USER CODE BEGIN SPI1_MspDeInit 1 */

  /* USER CODE END SPI1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

