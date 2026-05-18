/* ==================================================================
 * PN532 NFC Module I2C Driver (STM32F103C8T6 + HAL I2C)
 * ================================================================== */
#include "pn532.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
static uint8_t addr = (0x24 << 1);

#define CMD_GET_FW_VERSION          0x02
#define CMD_SAM_CONFIG              0x14
#define CMD_IN_LIST_PASSIVE_TARGET  0x4A

static int tx(uint8_t *buf, uint16_t len) {
    return HAL_I2C_Master_Transmit(&hi2c1, addr, buf, len, 200);
}
static int rx(uint8_t *buf, uint16_t len, uint32_t to) {
    return HAL_I2C_Master_Receive(&hi2c1, addr, buf, len, to);
}

/* 组 PN532 帧 */
static int mkframe(uint8_t *in, int n, uint8_t *out) {
    uint8_t cs = 0xD4;
    out[0]=0x00; out[1]=0x00; out[2]=0xFF;
    out[3]=n+1;  out[4]=(uint8_t)(0x100-out[3]);
    out[5]=0xD4;
    for(int i=0;i<n;i++){ out[i+6]=in[i]; cs+=in[i]; }
    out[n+6]=(uint8_t)(0x100-cs);
    out[n+7]=0x00;
    return n+8;
}

/* 发帧 → 读 ACK */
static int ack(uint8_t *f, int len) {
    uint8_t a[9]={0};
    if(tx(f,len)!=0) return -1;
    HAL_Delay(5);
    if(rx(a,9,100)!=0) return -1;
    return (a[0]==0x01 && a[1]==0x00 && a[2]==0x00 && a[3]==0xFF) ? 0 : -1;
}

/* 等 PN532 就绪 → 读帧 → 返回 payload(不含 TFI) */
static int read_resp(uint8_t *out, uint16_t max, uint32_t timeout_ms) {
    uint8_t b[41];
    uint32_t t0 = HAL_GetTick();
    while(HAL_GetTick()-t0 < timeout_ms) {
        HAL_Delay(20);
        if(rx(b,sizeof(b),50)!=0) continue;
        if(b[0]!=0x01) continue;
        /* 帧从 b[1] 开始 */
        uint8_t *f=&b[1];
        int fl=sizeof(b)-1;
        if(fl<7||f[0]!=0x00||f[1]!=0x00||f[2]!=0xFF) continue;
        if(f[3]!=(uint8_t)(0x100-f[4])) continue;
        if(f[5]!=0xD5) continue;
        uint8_t l=f[3];
        if(l<2||l+6>=fl) continue;
        if(f[l+6]!=0x00) continue;
        uint8_t pl=l-1;
        if(pl<1||pl>(int)max) continue;
        memcpy(out,&f[6],pl);
        return pl;
    }
    return -1;
}

/* 扫描 I2C 地址 */
static uint8_t scan(void) {
    uint8_t as[]={(0x24<<1),(0x25<<1),(0x26<<1)};
    for(int i=0;i<3;i++)
        if(HAL_I2C_IsDeviceReady(&hi2c1,as[i],1,20)==HAL_OK)
            { printf("[PN532] I2C 0x%02X\r\n",as[i]); return as[i]; }
    printf("[PN532] I2C fail\r\n");
    return 0;
}

/* ==================== API ==================== */

void PN532_Init(void) {
    uint8_t f[32], r[16];
    uint8_t a=scan(); if(a) addr=a; else return;
    HAL_Delay(10);

    /* 读版本 */
    uint8_t p1[]={CMD_GET_FW_VERSION};
    if(ack(f,mkframe(p1,1,f))==0 && read_resp(r,sizeof(r),200)>0)
        printf("[PN532] FW IC=%02X Ver=%d.%d\r\n",r[1],r[2],r[3]);

    /* SAM */
    uint8_t p2[]={CMD_SAM_CONFIG,0x01,0x00};
    if(ack(f,mkframe(p2,3,f))==0 && read_resp(r,sizeof(r),200)>0)
        printf("[PN532] SAM OK\r\n");
}

bool PN532_GetFirmwareVersion(uint8_t *ic, uint8_t *ver, uint8_t *rev, uint8_t *sup) {
    uint8_t f[32], r[16];
    uint8_t p[]={CMD_GET_FW_VERSION};
    if(ack(f,mkframe(p,1,f))!=0) return false;
    if(read_resp(r,sizeof(r),200)<5) return false;
    *ic=r[1]; *ver=r[2]; *rev=r[3]; *sup=r[4];
    return true;
}

bool PN532_ReadCardUID(uint8_t *uid, uint8_t *uid_len) {
    uint8_t f[32], r[32];
    uint8_t p[]={CMD_IN_LIST_PASSIVE_TARGET,0x01,0x00};
    if(ack(f,mkframe(p,3,f))!=0) return false;
    int pl=read_resp(r,sizeof(r),3000);  /* 等 3 秒 */
    /* r[0]=cmd+1, r[1]=NbTg, r[2..]=target */
    if(pl<7||r[1]!=1) return false;
    uint8_t idl=r[6]; if(idl>10) idl=10;
    memcpy(uid,&r[7],idl);
    *uid_len=idl;
    return true;
}