/**
  ******************************************************************************
  * @file    pn532_i2c.h
  * @author  zhujun
  * @version V1.0
  * @date    2019-07-01
  * @brief   PN532相关操作头文件
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2017 MindX</center></h2>
  ******************************************************************************
  */ 
#ifndef __PN532_I2C_H__
#define __PN532_I2C_H__

#include "stm32f10x.h"
#include "sw_i2c.h"

#define PN532_RESET_RCC             RCC_APB2Periph_GPIOB
#define PN532_RESET_PORT            GPIOB
#define PN532_RESET_PIN             GPIO_Pin_8

#define PN532_IRQ_RCC               RCC_APB2Periph_GPIOB
#define PN532_IRQ_PORT              GPIOB
#define PN532_IRQ_PIN               GPIO_Pin_5

#define PN532_PREAMBLE              0x00
#define PN532_STARTCODE1            0x00
#define PN532_STARTCODE2            0xFF

#define STM32_TO_PN532              0xD4
#define PN532_TO_STM32              0xD5

/*Miscellaneous*/
#define CMD_DIAGONOSE               0x00
#define CMD_GET_FW_VERSION          0x02
#define CMD_GET_GEN_STATUS          0x04
#define CMD_READ_REG                0x06
#define CMD_WRITE_REG               0x08
#define CMD_READ_GPIO               0x0C
#define CMD_WRITE_GPIO              0x0E
#define CMD_SET_SERIAL_RATE         0x10
#define CMD_SET_PARAM               0x12
#define CMD_SAM_CONFIG              0x14
#define CMD_POWERDOWN               0x16

/*RF Communication*/
#define CMD_RFConfig                0x32
#define CMD_RFRegationTest          0x58

/*Initiator*/
#define CMD_IN_JUMP_FOR_DEP         0x56
#define CMD_In_JUMP_FOR_PSL         0x46
#define CMD_IN_LIST_PASSIVE_TARGET  0x4a
#define CMD_IN_ATR                  0x50
#define CMD_IN_PSL                  0x4E
#define CMD_IN_DATA_EXCHANGE        0x40
#define CMD_IN_COMM_THRU            0x42
#define CMD_IN_DESELECT             0x44
#define CMD_IN_RELEASE              0x52
#define CMD_IN_SELECET              0x54
#define CMD_IN_AUTOPOLL             0x60

#define FIND_NFCCARD_MAXNUM         0x01
#define NFC_106K_PROTOCOL           0x00

#define  PN532_I2C_ADDR             (0x48) 

typedef enum
{
    PN532_OK = 0,
    PN532_TIMEOUT = -1,
    PN532_DATA_ERROR = -2,
}PN532_RES;

void PN532_GPIO_Init(void);
void PN532_Reset(void);
PN532_RES PN532_Get_Version(uint8_t *ver);
PN532_RES PN532_SAMConfig(void);
PN532_RES PN532_ReadPassiveTargetID(uint8_t *card_id, uint8_t *id_len);

#endif

