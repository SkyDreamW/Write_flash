#ifndef WRITEFLASH_H
#define WRITEFLASH_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#define USART_PORT huart1              // 串口使用 USART1（需在 usart.c 中初始化 huart1）
#define RX_DATA_LEN 256                // 单帧有效数据长度（不含状态位和 CRC）
#define RX_TOTAL_LEN (RX_DATA_LEN + 3) // 整体接收长度 = 数据(256) + 状态(1) + CRC(2)

#define MAX_RETRIES 5 // 数据包接收最大重试次数

// 外部接收缓冲区（接收 PC 端发来的数据包）
extern uint8_t rxBuff[RX_TOTAL_LEN];

// 数据接收状态标志
extern volatile uint8_t ReceivePrepareStatus;

// 接收准备状态枚举
enum
{
    Receive_READY = 0x0,     // 已准备好接收数据
    Receive_NOT_READY = 0x1, // 未准备好接收
    Receive_OK = 0x02,       // 接收完成，等待处理
    Receive_ERROR = 0x4      // 接收出错
};

// CRC 校验状态
enum
{
    CRC_CORRECT = 0x0,  // CRC 校验通过
    CRC_INCORRECT = 0x1 // CRC 校验失败
};

// 数据包发送状态标志（由 PC 设置在 rxBuff[256]）
enum
{
    Transmit_Over = 0,    // 全部数据发送完毕
    Transmit_Not_Over = 1 // 数据仍在继续发送中
};

// ======================= 串口通信接口 =======================

// 向 PC 发送 1 字节命令
void TransmitCMD(uint8_t cmd);

// 启动中断方式接收一帧数据
void ReceiveITFromComputer(uint8_t *data, uint16_t size);

// 阻塞方式接收 1 字节命令（带超时）
uint8_t ReceiveCMD();

// ======================= Flash 写入流程 =======================

// Flash 写入主控流程（建议从主程序中调用）
// address: Flash 开始写入地址
void WriteFlash(uint32_t address);

// 将数据写入 W25Q64 一页（RX_DATA_LEN 字节）
// flashAddress: 写入地址，必须是页对齐
// data: 数据缓冲区（来自 rxBuff）
void WritePageFromPC(uint32_t flashAddress, uint8_t *data);

// 读取 Flash 厂商和设备 ID，确认连接正常
// 返回 0 表示正常
uint8_t CheckFlash();

// ======================= 数据接收与校验 =======================

// 接收一帧完整数据，并进行 CRC 校验，自动重试错误帧
void GetData();

// CRC 校验数据帧（MODBUS 协议）
// 返回 CRC_CORRECT 或 CRC_INCORRECT
uint8_t CheckData();

// 计算 CRC16-MODBUS 校验值
// data: 输入数据指针
// len: 有效数据长度
// 返回 16 位 CRC 值
uint16_t CRC16_Modbus(uint8_t *data, uint16_t len);

// ======================= 错误处理接口 =======================

// 数据连续接收失败的错误处理函数（默认进入死循环发错码）
void ReceiveError();

#endif // WRITEFLASH_H
