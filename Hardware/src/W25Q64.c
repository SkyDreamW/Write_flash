#include "W25Q64.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "spi.h"

void SelectFlashCS()
{
    HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET);
}

void ReleaseFlashCS()
{
    HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET);
}

void SendCommand(uint8_t *command, uint16_t size)
{
    HAL_SPI_Transmit(&hspi1, command, size, HAL_MAX_DELAY);
}

void ReceiveData(uint8_t *data, uint16_t size)
{
    HAL_SPI_Receive(&hspi1, data, size, HAL_MAX_DELAY);
}

void ReadManufacturerAndDeviceID(uint8_t *data)
{
    SelectFlashCS();
    uint8_t command[4] = {W25Q64_Manufacturer_Device_ID, 0x0, 0x0, 0x0};
    SendCommand(command, sizeof(command));
    ReceiveData(data, 2);
    ReleaseFlashCS();
}

void ReadStatusRegister1(uint8_t *data)
{
    SelectFlashCS();
    uint8_t command = W25Q64_Read_Status_register_1;
    SendCommand(&command, sizeof(command));
    ReceiveData(data, 1);
    ReleaseFlashCS();
}

void WriteEnable()
{
    SelectFlashCS();
    uint8_t command = W25Q64_Write_Enable;
    SendCommand(&command, 1);
    ReleaseFlashCS();
}

void WriteDisable()
{
    SelectFlashCS();
    uint8_t command = W25Q64_Write_Disable;
    SendCommand(&command, 1);
    ReleaseFlashCS();
}

uint8_t CheckBusy()
{
    uint8_t data;
    ReadStatusRegister1(&data);
    if ((data & 0x1) == 0x1)
    {
        return Busy;
    }
    else
    {
        return Spare;
    }
}

void SectorErase4KB(uint8_t *address)
{
    while (CheckBusy() == Busy)
        ;
    WriteEnable();
    SelectFlashCS();
    uint8_t command[4];
    command[0] = W25Q64_Sector_Erase_4KB;
    for (size_t i = 0; i < 3; i++)
    {
        command[i + 1] = address[i];
    }
    SendCommand(command, sizeof(command));
    ReleaseFlashCS();
}

void ReadData(uint8_t *address, uint8_t *data, uint16_t dataSize)
{
    while (CheckBusy() == Busy)
        ;
    SelectFlashCS();
    uint8_t command[4];
    command[0] = W25Q64_Read_Data;
    for (size_t i = 0; i < 3; i++)
    {
        command[i + 1] = address[i];
    }
    SendCommand(command, sizeof(command));
    ReceiveData(data, dataSize);
    ReleaseFlashCS();
}

void PageProgram(uint8_t *address, uint8_t *data, uint16_t dataSize)
{

    while (CheckBusy() == Busy)
        ;
    WriteEnable();
    SelectFlashCS();
    uint8_t command[4];
    command[0] = W25Q64_Page_Program;
    for (size_t i = 0; i < 3; i++)
    {
        command[i + 1] = address[i];
    }
    SendCommand(command, sizeof(command));
    SendCommand(data, dataSize);
    ReleaseFlashCS();
}

void Convert24BitAddress(uint32_t address, uint8_t addr_bytes[3])
{
    addr_bytes[0] = (address >> 16) & 0xFF; // 高 8 位
    addr_bytes[1] = (address >> 8) & 0xFF;  // 中间 8 位
    addr_bytes[2] = address & 0xFF;         // 低 8 位
}