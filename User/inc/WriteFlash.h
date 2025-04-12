#ifndef WRITEFLASH_H
#define WRITEFLASH_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#define RX_DATA_LEN 256
#define RX_TOTAL_LEN (RX_DATA_LEN + 3) // 256数据 + 2字节CRC + 1字节状态

// 接收缓冲区
extern uint8_t rxBuff[RX_TOTAL_LEN];
// 准备状态变量
extern volatile uint8_t prepareStatus;

// 枚举：准备接收状态状态
typedef enum
{
    Receive_READY = 0x0,
    Receive_NOT_READY = 0x1,
    Receive_OK = 0x02,
    Receive_ERROR = 0x4
} ReceivePrapareStatus;

// 枚举：CRC校验结果
typedef enum
{
    CRC_CORRECT = 0x0,
    CRC_INCORRECT = 0x1
} CrcStatus;

// 枚举：发送状态（结束，未结束）
typedef enum
{
    Transmit_Over = 0,
    Transmit_Not_Over = 1
} TransmitStatus;

// 通信命令（可以后续扩展）
extern const uint8_t READY_CMD;
extern const uint8_t DATA_CORRECT;
extern const uint8_t DATA_INCORRECT;
extern const uint8_t FLASH_OK;
extern const uint8_t WRITE_OVER;
extern const uint8_t COMPUTER_PREPARED;

// 功能函数声明
uint8_t checkFlash(void);
void getData(void);
uint8_t checkData(void);
void WriteFlash(uint32_t address);
void WritePageFromPC(uint32_t flashAddress, uint8_t *data);
uint16_t CRC16_Modbus(uint8_t *data, uint16_t len);

#endif // WRITEFLASH_H
