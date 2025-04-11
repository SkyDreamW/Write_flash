#ifndef W25Q64_H
#define W25Q64_H

#include "main.h"
#include "stm32f1xx_hal.h"

// W25Q64指令表1
#define W25Q64_Write_Enable 0x06
#define W25Q64_Write_Disable 0x04
#define W25Q64_Read_Status_register_1 0x05
#define W25Q64_Read_Status_register_2 0x35
#define W25Q64_Write_Status_register 0x01
#define W25Q64_Page_Program 0x02
#define W25Q64_Quad_Page_Program 0x32
#define W25Q64_Block_Erase_64KB 0xD8
#define W25Q64_Block_Erase_32KB 0x52
#define W25Q64_Sector_Erase_4KB 0x20
#define W25Q64_Chip_Erase 0xC7
#define W25Q64_Erase_Suspend 0x75
#define W25Q64_Erase_Resume 0x7A
#define W25Q64_Power_down 0xB9
#define W25Q64_High_Performance_Mode 0xA3
#define W25Q64_Continuous_Read_Mode_Reset 0xFF
#define W25Q64_Release_Power_Down_HPM_Device_ID 0xAB
#define W25Q64_Manufacturer_Device_ID 0x90
#define W25Q64_Read_Uuique_ID 0x4B
#define W25Q64_JEDEC_ID 0x9F

void SelectFlashCS();
void ReleaseFlashCS();
void WriteEnable();
void WriteDisable();
void SendCommand(uint8_t *command, uint8_t size);
void ReceiveData(uint8_t *data, uint8_t size);
void ReadManufacturerAndDeviceID(uint8_t *data, uint8_t size);
void ReadStatusRegister1(uint8_t *data, uint8_t size);

enum BusyStatus
{
    Spare = 0,
    Busy = 1
};

#endif // !W25Q64_H