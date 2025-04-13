#include "WriteFlash.h"
#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "W25Q64.h" // 你自己实现的 W25Q64 驱动
#include "usart.h"  // 包含 USART_PORT 对应定义（如 huart1）

// ==================== 与 PC 通信的命令字节 ====================
const uint8_t CMD_PC_READY = 'P';   // PC 准备完成（Prepare）
const uint8_t CMD_DATA_READY = 'R'; // PC 发送数据准备好（Ready to Send）

// ==================== STM32 发送给 PC 的回应命令字节 ====================
const uint8_t RESP_FLASH_READY = 'K';        // Flash 准备完毕
const uint8_t RESP_DATA_OK = 'C';            // 数据 CRC 正确
const uint8_t RESP_DATA_ERR = 'E';           // 数据 CRC 错误
const uint8_t RESP_WRITE_OVER = 'O';         // 所有写入完成
const uint8_t RESP_DATA_RECEIVE_READY = 'T'; // STM32 准备接收数据
const uint8_t RESP_DATA_RECEIVE_ERROR = 'X'; // 接收失败

// 接收缓冲区（256数据 + 1状态 + 2CRC）
uint8_t rxBuff[RX_TOTAL_LEN] = {0xFF};

// 接收状态控制变量（由中断回调修改）
volatile uint8_t ReceivePrepareStatus = Receive_NOT_READY;

// ========== 基本通信函数 ==========

// 向 PC 发送 1 字节命令
void TransmitCMD(uint8_t cmd)
{
    HAL_UART_Transmit(&USART_PORT, &cmd, 1, 50); // 阻塞发送，超时 50ms
}

// 启动中断接收数据包
void ReceiveITFromComputer(uint8_t *data, uint16_t size)
{
    HAL_UART_Receive_IT(&USART_PORT, data, size);
}

// 阻塞接收 1 字节命令（用于握手）
uint8_t ReceiveCMD()
{
    uint8_t cmd;
    HAL_UART_Receive(&USART_PORT, &cmd, 1, 50);
    return cmd;
}

// ========== Flash 写入主流程 ==========

void WriteFlash(uint32_t address)
{
    // 等待 PC 准备完成
    while (ReceiveCMD() != CMD_PC_READY)
        ;

    // 检查 Flash 并发送回应
    if (CheckFlash() == 0)
    {
        // ⚠️ 你需要自行实现 ChipErase()：全片擦除
        ChipErase();

        // 等待 PC 发起数据发送指令
        while (ReceiveCMD() != CMD_DATA_READY)
        {
            TransmitCMD(RESP_FLASH_READY);
        }
    }

    // 持续获取数据包并写入 Flash
    do
    {
        GetData();                        // 获取一帧数据
        WritePageFromPC(address, rxBuff); // 写入数据
        address += RX_DATA_LEN;
    } while (rxBuff[RX_TOTAL_LEN - 3] == Transmit_Not_Over); // 检查是否还有数据

    // 通知 PC 写入完毕
    TransmitCMD(RESP_WRITE_OVER);
}

// ========== Flash 状态确认 ==========

// ⚠️ ReadManufacturerAndDeviceID(ID)：你需要自己实现，读 Flash ID
uint8_t CheckFlash()
{
    uint8_t ID[2];
    while (ID[0] != 0xEF || ID[1] != 0x16)
    {
        ReadManufacturerAndDeviceID(ID);
    }
    return 0;
}

// ========== 数据接收和校验 ==========

void GetData()
{
    uint8_t retryCount = 0;

retry:
    if (retryCount == MAX_RETRIES)
    {
        ReceiveError();
    }

    retryCount++;

    ReceiveITFromComputer(rxBuff, RX_TOTAL_LEN);
    ReceivePrepareStatus = Receive_READY;

    while (ReceivePrepareStatus == Receive_READY)
    {
        TransmitCMD(RESP_DATA_RECEIVE_READY);
    }

    if (ReceivePrepareStatus == Receive_OK)
    {
        if (CheckData())
        {
            TransmitCMD(RESP_DATA_ERR);
            goto retry;
        }
        else
        {
            TransmitCMD(RESP_DATA_OK);
        }
    }
    else
    {
        TransmitCMD(RESP_DATA_ERR);
        goto retry;
    }
}

// CRC 校验函数（MODBUS 协议）
uint8_t CheckData()
{
    uint16_t calculatedCrc16 = CRC16_Modbus(rxBuff, RX_DATA_LEN);
    uint16_t receiveCrc16 = (rxBuff[RX_TOTAL_LEN - 1] << 8) | rxBuff[RX_TOTAL_LEN - 2];

    return (calculatedCrc16 == receiveCrc16) ? CRC_CORRECT : CRC_INCORRECT;
}

// 计算 CRC-16（MODBUS）
uint16_t CRC16_Modbus(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

// 写入数据到 Flash（页写）
void WritePageFromPC(uint32_t flashAddress, uint8_t *data)
{
    uint8_t addr_bytes[3];

    // ⚠️ Convert24BitAddress()：你需要自己实现（将地址分成3个字节）
    Convert24BitAddress(flashAddress, addr_bytes);

    // ⚠️ PageProgram()：你需要自己实现（写入一个页）
    PageProgram(addr_bytes, data, RX_DATA_LEN);
}

// 错误处理函数（数据接收失败）
void ReceiveError()
{
    while (1)
    {
        TransmitCMD(RESP_DATA_RECEIVE_ERROR);
        HAL_Delay(100);
    }
}
