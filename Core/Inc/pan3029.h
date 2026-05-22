#ifndef __PAN3029_H__
#define __PAN3029_H__

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define PAN3029_FIFO_ADDR          0x01u
#define PAN3029_MAX_FIFO_LEN       255U

typedef enum {
    PAN3029_OK = 0,
    PAN3029_ERROR = 1,
    PAN3029_ERROR_PARAM = 2,
    PAN3029_ERROR_SPI = 3,
    PAN3029_ERROR_NEED_SDK = 4
} PAN3029_Status;

typedef struct {
    uint8_t data[64];
    uint8_t len;
    int8_t snr;
    int16_t rssi;
} PAN3029_RxPacket;

void PAN3029_Init(void);
void PAN3029_Reset(void);

PAN3029_Status PAN3029_ReadReg(uint8_t addr, uint8_t *value);
PAN3029_Status PAN3029_WriteReg(uint8_t addr, uint8_t value);
PAN3029_Status PAN3029_ReadBurst(uint8_t addr, uint8_t *buf, uint16_t len);
PAN3029_Status PAN3029_WriteBurst(uint8_t addr, const uint8_t *buf, uint16_t len);
PAN3029_Status PAN3029_ReadFifo(uint8_t *buf, uint16_t len);
PAN3029_Status PAN3029_WriteFifo(const uint8_t *buf, uint16_t len);

bool PAN3029_IrqPending(void);
void PAN3029_ClearIrqPending(void);
void PAN3029_OnGpioExti(uint16_t GPIO_Pin);
void PAN3029_DumpRegs(uint8_t start_addr, uint8_t count);

PAN3029_Status PAN3029_StartContinuousRx(void);
PAN3029_Status PAN3029_ReadRxPacket(PAN3029_RxPacket *pkt);
void PAN3029_RxDebugTask(void);

PAN3029_Status PAN3029_StartTxNode(void);
PAN3029_Status PAN3029_SendPacket(const uint8_t *data, uint8_t len);
void PAN3029_TxTask(void);

#endif /* __PAN3029_H__ */
