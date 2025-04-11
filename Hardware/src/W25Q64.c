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

void SendCommand(uint8_t *command, uint8_t size)
{
    HAL_SPI_Transmit(&hspi1, command, size, HAL_MAX_DELAY);
}

void ReceiveData(uint8_t *data, uint8_t size)
{
    HAL_SPI_Receive(&hspi1, data, size, HAL_MAX_DELAY);
}

void ReadManufacturerAndDeviceID(uint8_t *data, uint8_t size)
{
    SelectFlashCS();
    uint8_t command[4] = {W25Q64_Manufacturer_Device_ID, 0x0, 0x0, 0x0};
    SendCommand(command, sizeof(command));
    ReceiveData(data, size);
    ReleaseFlashCS();
}

void ReadStatusRegister1(uint8_t *data, uint8_t size)
{
    SelectFlashCS();
    uint8_t command = W25Q64_Read_Status_register_1;
    SendCommand(&command, sizeof(command));
    ReceiveData(data, size);
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

uint8_t ChekBusy()
{
    uint8_t data;
    ReadStatusRegister1(&data, 1);
    if (data & 0x1 == 0x1)
    {
        return Busy;
    }
    else
    {
        return Spare;
    }
}