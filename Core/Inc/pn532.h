#ifndef __PN532_H__
#define __PN532_H__

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

/* PN532 I2C 地址: 7-bit 0x24, HAL 需要左移1位 → 0x48 */
#define PN532_I2C_ADDR  0x48

/* 初始化 PN532（发送唤醒 + SAM 配置，进入正常读卡模式） */
void PN532_Init(void);

/* 读取 PN532 固件版本（验证通信是否正常） */
bool PN532_GetFirmwareVersion(uint8_t *ic, uint8_t *ver, uint8_t *rev, uint8_t *support);

/*
 * 扫描并读取一张卡的 UID
 *   uid     → 输出缓冲区（至少 10 字节）
 *   uid_len → 输出 UID 实际长度
 *   返回 true 表示成功读到卡
 */
bool PN532_ReadCardUID(uint8_t *uid, uint8_t *uid_len);

#endif /* __PN532_H__ */