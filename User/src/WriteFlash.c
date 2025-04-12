#include "WriteFlash.h"
#include "stm32f1xx_hal.h"
#include "stdint.h"
#include "W25Q64.h"
#include "usart.h"

const uint8_t READY_CMD = 'R';
const uint8_t DATA_CORRECT = 'C';
const uint8_t DATA_INCORRECT = 'E';
const uint8_t FLASH_OK = 'K';
const uint8_t WRITE_OVER = 'O';
const uint8_t COMPUTER_PREPARED = 'P';

uint8_t rxBuff[RX_TOTAL_LEN] = {0xFF}; // 0-255字节数据,256 257为两字节的crc校验码,258为发送状态判断，1为发送未完必，0为发送完毕
volatile uint8_t prepareStatus = Receive_NOT_READY;

void WriteFlash(uint32_t address)
{
    // 检查flash的连接是否正常
    if (checkFlash() == 0)
    {
        HAL_UART_Transmit(&huart1, &FLASH_OK, 1, 50);
    }

    uint8_t computerStatus = 0;

    while (computerStatus != COMPUTER_PREPARED)
    {
        HAL_UART_Receive(&huart1, &computerStatus, 1, 50);
    }

    // 获取数据,直到rxBuff[258]即发送状态为0表示发送完毕
    while (rxBuff[258] == Transmit_Not_Over)
    {

        // 获取data
        getData();
        // 往flash写数据
        WritePageFromPC(address, rxBuff);
        address += 256;
    }

    // 告诉电脑写入结束
    HAL_UART_Transmit(&huart1, &WRITE_OVER, 1, 50);
}

uint8_t checkFlash()
{
    uint8_t ID[2];
    // 读flash的厂商和设备id，直到正确，确保通讯正常
    while (ID[0] != 0xEF || ID[1] != 0x16)
    {
        ReadManufacturerAndDeviceID(ID);
    }
    return 0;
}

void getData()
{
retry:
    // 启动中断接收电脑发送的数据，存入rxBuff
    HAL_UART_Receive_IT(&huart1, rxBuff, 259);
    // prepareStatus置为Receive_READY表示准备好接收数据
    prepareStatus = Receive_READY;
    HAL_UART_Transmit(&huart1, &READY_CMD, 1, 50);
    while (prepareStatus == Receive_OK || prepareStatus == Receive_ERROR)
        ;

    if (prepareStatus == Receive_OK)
    {
        if (checkData())
        {
            // 如果crc数据不一致，重发
            HAL_UART_Transmit(&huart1, &DATA_INCORRECT, 1, 50);
            goto retry;
        }
        else
        {
            // 告诉电脑，数据正常
            HAL_UART_Transmit(&huart1, &DATA_CORRECT, 1, 50);
        }
    }
    else
    {
        // 没有准备好接收数据电脑就发送，重发
        HAL_UART_Transmit(&huart1, &DATA_INCORRECT, 1, 50);
        goto retry;
    }
}

uint8_t checkData()
{
    uint16_t calculatedCrc16 = CRC16_Modbus(rxBuff, 256);
    uint16_t receieveCrc16;
    receieveCrc16 = (rxBuff[257] << 8) | rxBuff[256];
    if (calculatedCrc16 == receieveCrc16)
    {
        return CRC_CORRECT;
    }
    else
    {
        return CRC_INCORRECT;
    }
}

// 计算 CRC-16（MODBUS）值
uint16_t CRC16_Modbus(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF; // 初始值

    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i]; // 将数据与CRC低字节异或

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001) // 如果最低位是1
            {
                crc >>= 1;     // 右移1位
                crc ^= 0xA001; // 与多项式异或
            }
            else
            {
                crc >>= 1; // 右移1位
            }
        }
    }

    return crc;
}

void WritePageFromPC(uint32_t flashAddress, uint8_t *data)
{
    uint8_t addr_bytes[3];
    Convert24BitAddress(flashAddress, addr_bytes);

    // 等待Flash空闲，然后写入一页
    PageProgram(addr_bytes, data, 256);
}