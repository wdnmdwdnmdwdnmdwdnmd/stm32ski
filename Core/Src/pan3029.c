#include "pan3029.h"
#include "pan_rf.h"
#include "spi.h"
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi2;

#define PAN3029_SPI_TIMEOUT_MS     100u
#define PAN3029_CMD_READ           0x00u
#define PAN3029_CMD_WRITE          0x01u

static volatile bool pan3029_irq_pending = false;
static volatile bool pan3029_tx_busy = false;

static void PAN3029_CsnLow(void)
{
    HAL_GPIO_WritePin(spi2csn_GPIO_Port, spi2csn_Pin, GPIO_PIN_RESET);
}

static void PAN3029_CsnHigh(void)
{
    HAL_GPIO_WritePin(spi2csn_GPIO_Port, spi2csn_Pin, GPIO_PIN_SET);
}

static uint8_t PAN3029_MakeCmd(uint8_t addr, uint8_t rw)
{
    return (uint8_t)(((addr & 0x7Fu) << 1) | (rw & 0x01u));
}

static PAN3029_Status PAN3029_TxRx(const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    if (HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)tx, rx, len, PAN3029_SPI_TIMEOUT_MS) != HAL_OK) {
        return PAN3029_ERROR_SPI;
    }

    return PAN3029_OK;
}

void PAN3029_Reset(void)
{
    PAN3029_CsnHigh();
    HAL_GPIO_WritePin(spi2nrst_GPIO_Port, spi2nrst_Pin, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(spi2nrst_GPIO_Port, spi2nrst_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

void PAN3029_Init(void)
{
    PAN3029_CsnHigh();
    PAN3029_ClearIrqPending();
    PAN3029_Reset();
    printf("[PAN3029] low-level SPI driver ready\r\n");
}

PAN3029_Status PAN3029_ReadReg(uint8_t addr, uint8_t *value)
{
    uint8_t tx[2] = {PAN3029_MakeCmd(addr, PAN3029_CMD_READ), 0x00};
    uint8_t rx[2] = {0};
    PAN3029_Status ret;

    if (value == NULL) {
        return PAN3029_ERROR_PARAM;
    }

    PAN3029_CsnLow();
    ret = PAN3029_TxRx(tx, rx, sizeof(tx));
    PAN3029_CsnHigh();

    if (ret == PAN3029_OK) {
        *value = rx[1];
    }

    return ret;
}

PAN3029_Status PAN3029_WriteReg(uint8_t addr, uint8_t value)
{
    uint8_t tx[2] = {PAN3029_MakeCmd(addr, PAN3029_CMD_WRITE), value};
    uint8_t rx[2] = {0};
    PAN3029_Status ret;

    PAN3029_CsnLow();
    ret = PAN3029_TxRx(tx, rx, sizeof(tx));
    PAN3029_CsnHigh();

    return ret;
}

PAN3029_Status PAN3029_ReadBurst(uint8_t addr, uint8_t *buf, uint16_t len)
{
    uint8_t cmd;
    PAN3029_Status ret = PAN3029_OK;

    if ((buf == NULL) || (len == 0u)) {
        return PAN3029_ERROR_PARAM;
    }

    cmd = PAN3029_MakeCmd(addr, PAN3029_CMD_READ);

    PAN3029_CsnLow();
    if (HAL_SPI_Transmit(&hspi2, &cmd, 1, PAN3029_SPI_TIMEOUT_MS) != HAL_OK) {
        ret = PAN3029_ERROR_SPI;
    } else if (HAL_SPI_Receive(&hspi2, buf, len, PAN3029_SPI_TIMEOUT_MS) != HAL_OK) {
        ret = PAN3029_ERROR_SPI;
    }
    PAN3029_CsnHigh();

    return ret;
}

PAN3029_Status PAN3029_WriteBurst(uint8_t addr, const uint8_t *buf, uint16_t len)
{
    uint8_t cmd;
    PAN3029_Status ret = PAN3029_OK;

    if ((buf == NULL) || (len == 0u)) {
        return PAN3029_ERROR_PARAM;
    }

    cmd = PAN3029_MakeCmd(addr, PAN3029_CMD_WRITE);

    PAN3029_CsnLow();
    if (HAL_SPI_Transmit(&hspi2, &cmd, 1, PAN3029_SPI_TIMEOUT_MS) != HAL_OK) {
        ret = PAN3029_ERROR_SPI;
    } else if (HAL_SPI_Transmit(&hspi2, (uint8_t *)buf, len, PAN3029_SPI_TIMEOUT_MS) != HAL_OK) {
        ret = PAN3029_ERROR_SPI;
    }
    PAN3029_CsnHigh();

    return ret;
}

PAN3029_Status PAN3029_ReadFifo(uint8_t *buf, uint16_t len)
{
    if (len > PAN3029_MAX_FIFO_LEN) {
        return PAN3029_ERROR_PARAM;
    }

    return PAN3029_ReadBurst(PAN3029_FIFO_ADDR, buf, len);
}

PAN3029_Status PAN3029_WriteFifo(const uint8_t *buf, uint16_t len)
{
    if (len > PAN3029_MAX_FIFO_LEN) {
        return PAN3029_ERROR_PARAM;
    }

    return PAN3029_WriteBurst(PAN3029_FIFO_ADDR, buf, len);
}

bool PAN3029_IrqPending(void)
{
    return pan3029_irq_pending;
}

void PAN3029_ClearIrqPending(void)
{
    pan3029_irq_pending = false;
}

void PAN3029_OnGpioExti(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == spi2irq_Pin) {
        pan3029_irq_pending = true;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    PAN3029_OnGpioExti(GPIO_Pin);
}

void PAN3029_DumpRegs(uint8_t start_addr, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) {
        uint8_t addr = (uint8_t)(start_addr + i);
        uint8_t val = 0;

        if (PAN3029_ReadReg(addr, &val) == PAN3029_OK) {
            printf("[PAN3029] reg[0x%02X] = 0x%02X\r\n", addr, val);
        } else {
            printf("[PAN3029] reg[0x%02X] read FAIL\r\n", addr);
        }
    }
}

PAN3029_Status PAN3029_StartContinuousRx(void)
{
    PAN3029_ClearIrqPending();

    if (RF_Init() != RF_OK) {
        printf("[PAN3029] RF init FAIL\r\n");
        return PAN3029_ERROR;
    }

    RF_ConfigUserParams();
    RF_EnterContinousRxState();
    RF_ClrIRQFlag(0x7Fu);

    printf("[PAN3029] RX continuous mode ready\r\n");
    printf("[PAN3029] params: freq=%lu Hz, SF7, BW500K, CR4/5, CRC on\r\n",
           (unsigned long)RF_FREQ_DEFAULT);
    printf("[PAN3029] state=0x%02X irq=%u flag=0x%02X\r\n",
           RF_ReadReg(0x02),
           CHECK_RF_IRQ() ? 1u : 0u,
           RF_GetIRQFlag());

    return PAN3029_OK;
}

PAN3029_Status PAN3029_ReadRxPacket(PAN3029_RxPacket *pkt)
{
    uint8_t tmp[255] = {0};
    uint8_t rx_len;

    if (pkt == NULL) {
        return PAN3029_ERROR_PARAM;
    }

    pkt->len = 0;
    pkt->snr = 0;
    pkt->rssi = 0;

    pkt->snr = (int8_t)RF_GetPktSnr();
    pkt->rssi = RF_GetPktRssi();
    rx_len = RF_GetRecvPayload(tmp);

    pkt->len = (rx_len > sizeof(pkt->data)) ? sizeof(pkt->data) : rx_len;
    memcpy(pkt->data, tmp, pkt->len);

    return PAN3029_OK;
}

void PAN3029_RxDebugTask(void)
{
    uint8_t irq_flag;
    static uint32_t rx_count = 0;
    static uint32_t last_idle_tick = 0;
    uint8_t state;

    if (!PAN3029_IrqPending() && !CHECK_RF_IRQ()) {
        if ((HAL_GetTick() - last_idle_tick) >= 2000u) {
            last_idle_tick = HAL_GetTick();
            state = RF_ReadReg(0x02);
            printf("[PAN3029] RX idle state=0x%02X irq=%u flag=0x%02X rssi=%d dBm\r\n",
                   state,
                   CHECK_RF_IRQ() ? 1u : 0u,
                   RF_GetIRQFlag(),
                   RF_GetRealTimeRssi());
            if (state != RF_STATE_RX) {
                printf("[PAN3029] RX state lost, re-enter RX\r\n");
                RF_EnterContinousRxState();
                RF_ClrIRQFlag(0x7Fu);
            }
        }
        return;
    }

    PAN3029_ClearIrqPending();
    irq_flag = RF_GetIRQFlag();

    if (irq_flag & RF_IRQ_RX_DONE) {
        PAN3029_RxPacket pkt;

        if (PAN3029_ReadRxPacket(&pkt) == PAN3029_OK) {
            rx_count++;
            printf("[PAN3029] RX len=%u count=%lu SNR=%d dB RSSI=%d dBm\r\n",
                   pkt.len, (unsigned long)rx_count, pkt.snr, pkt.rssi);
            printf("[PAN3029] RX data:");
            for (uint8_t i = 0; i < pkt.len; i++) {
                printf(" %02X", pkt.data[i]);
            }
            printf("\r\n");
        }

        RF_ClrIRQFlag(RF_IRQ_RX_DONE);
        irq_flag &= (uint8_t)~RF_IRQ_RX_DONE;
    }

    if (irq_flag & RF_IRQ_CRC_ERR) {
        RF_ClrIRQFlag(RF_IRQ_CRC_ERR);
        irq_flag &= (uint8_t)~RF_IRQ_CRC_ERR;
        printf("[PAN3029] RX CRC error\r\n");
    }

    if (irq_flag & RF_IRQ_RX_TIMEOUT) {
        RF_ClrIRQFlag(RF_IRQ_RX_TIMEOUT);
        irq_flag &= (uint8_t)~RF_IRQ_RX_TIMEOUT;
        printf("[PAN3029] RX timeout\r\n");
    }

    if (irq_flag) {
        RF_ClrIRQFlag(irq_flag);
        printf("[PAN3029] IRQ other=0x%02X\r\n", irq_flag);
    }
}

PAN3029_Status PAN3029_StartTxNode(void)
{
    PAN3029_ClearIrqPending();
    pan3029_tx_busy = false;

    if (RF_Init() != RF_OK) {
        printf("[PAN3029] RF init FAIL\r\n");
        return PAN3029_ERROR;
    }

    RF_ConfigUserParams();

    printf("[PAN3029] TX node ready\r\n");
    printf("[PAN3029] params: freq=%lu Hz, SF7, BW500K, CR4/5, CRC on\r\n",
           (unsigned long)RF_FREQ_DEFAULT);

    return PAN3029_OK;
}

PAN3029_Status PAN3029_SendPacket(const uint8_t *data, uint8_t len)
{
    if ((data == NULL) || (len == 0u)) {
        return PAN3029_ERROR_PARAM;
    }

    if (pan3029_tx_busy) {
        return PAN3029_ERROR;
    }

    PAN3029_ClearIrqPending();
    pan3029_tx_busy = true;
    RF_SetTx((uint8_t *)data, len);

    return PAN3029_OK;
}

void PAN3029_TxTask(void)
{
    static uint32_t last_tx_tick = 0;
    static uint32_t tx_count = 0;
    char msg[32];
    uint8_t irq_flag;

    if (PAN3029_IrqPending() || CHECK_RF_IRQ()) {
        PAN3029_ClearIrqPending();
        irq_flag = RF_GetIRQFlag();

        if (irq_flag & RF_IRQ_TX_DONE) {
            RF_ClrIRQFlag(RF_IRQ_TX_DONE);
            irq_flag &= (uint8_t)~RF_IRQ_TX_DONE;
            pan3029_tx_busy = false;
            printf("[PAN3029] TX done\r\n");
        }

        if (irq_flag) {
            RF_ClrIRQFlag(irq_flag);
            printf("[PAN3029] IRQ other=0x%02X\r\n", irq_flag);
        }
    }

    if (pan3029_tx_busy) {
        return;
    }

    if ((HAL_GetTick() - last_tx_tick) < 1000u) {
        return;
    }

    last_tx_tick = HAL_GetTick();
    tx_count++;
    (void)snprintf(msg, sizeof(msg), "hello %lu", (unsigned long)tx_count);

    if (PAN3029_SendPacket((const uint8_t *)msg, (uint8_t)strlen(msg)) == PAN3029_OK) {
        printf("[PAN3029] TX send: %s\r\n", msg);
    } else {
        printf("[PAN3029] TX send FAIL\r\n");
        pan3029_tx_busy = false;
    }
}
