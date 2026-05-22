#ifndef __PAN3029_BSP_H__
#define __PAN3029_BSP_H__

#include "main.h"
#include "spi.h"

#define USE_SPI_3LINE      0
#define USE_SPI_4LINE      1
#define INTERFACE_MODE     USE_SPI_4LINE

#define SPI_NSS_PORT       spi2csn_GPIO_Port
#define SPI_NSS_PIN        spi2csn_Pin
#define GPIO_PORT_RF_IRQ   spi2irq_GPIO_Port
#define GPIO_PIN_RF_IRQ    spi2irq_Pin
#define GPIO_PORT_RF_RST   spi2nrst_GPIO_Port
#define GPIO_PIN_RF_RST    spi2nrst_Pin

#define GPIO_PORT_RF_CAD   spi2irq_GPIO_Port
#define GPIO_PIN_RF_CAD    spi2irq_Pin

#define PORT_SetBits(port, pin)       HAL_GPIO_WritePin((port), (pin), GPIO_PIN_SET)
#define PORT_ResetBits(port, pin)     HAL_GPIO_WritePin((port), (pin), GPIO_PIN_RESET)
#define PORT_GetBit(port, pin)        (HAL_GPIO_ReadPin((port), (pin)) == GPIO_PIN_SET)

#define Reset                      0
#define SpiFlagSendBufferEmpty     0
#define SpiFlagReceiveBufferFull   0
#define M4_SPI1                    0
#define SPI_GetFlag(spi, flag)     1
#define SPI_SendData8(spi, value)  ((void)0)
#define SPI_ReceiveData8(spi)      0

typedef enum {
    Pin_Mode_In = 0,
    Pin_Mode_Out = 1
} en_pin_mode_t;

static inline void BSP_SetGpioMode(GPIO_TypeDef *port, uint16_t pin, en_pin_mode_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = (mode == Pin_Mode_Out) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

#endif /* __PAN3029_BSP_H__ */
