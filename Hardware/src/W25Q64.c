#include "W25Q64.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "spi.h"

/**
 * @brief  选择 Flash 片选引脚，拉低使能通信
 * @retval None
 */
void SelectFlashCS()
{
    HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  释放 Flash 片选引脚，拉高禁用通信
 * @retval None
 */
void ReleaseFlashCS()
{
    HAL_GPIO_WritePin(Flash_CS_GPIO_Port, Flash_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief  发送命令到 Flash
 * @param  command: 指向要发送的命令数据的指针
 * @param  size: 命令数据的大小
 * @retval None
 */
void SendCommand(uint8_t *command, uint16_t size)
{
    HAL_SPI_Transmit(&hspi1, command, size, HAL_MAX_DELAY);
}

/**
 * @brief  从 Flash 接收数据
 * @param  data: 指向接收数据的缓冲区的指针
 * @param  size: 要接收的数据的大小
 * @retval None
 */
void ReceiveData(uint8_t *data, uint16_t size)
{
    HAL_SPI_Receive(&hspi1, data, size, HAL_MAX_DELAY);
}

/**
 * @brief  读取 Flash 的生产商和设备 ID
 * @param  data: 指向保存返回数据的缓冲区的指针,大小为2
 * @retval None
 */
void ReadManufacturerAndDeviceID(uint8_t *data)
{
    SelectFlashCS();
    uint8_t command[4] = {W25Q64_Manufacturer_Device_ID, 0x0, 0x0, 0x0};
    SendCommand(command, sizeof(command));
    ReceiveData(data, 2);
    ReleaseFlashCS();
}

/**
 * @brief  读取 Flash 状态寄存器 1
 * @param  data: 指向保存返回状态数据的缓冲区的指针，大小为1
 * @retval None
 */
void ReadStatusRegister1(uint8_t *data)
{
    SelectFlashCS();
    uint8_t command = W25Q64_Read_Status_register_1;
    SendCommand(&command, sizeof(command));
    ReceiveData(data, 1);
    ReleaseFlashCS();
}

/**
 * @brief  使能写操作
 * @retval None
 */
void WriteEnable()
{
    SelectFlashCS();
    uint8_t command = W25Q64_Write_Enable;
    SendCommand(&command, 1);
    ReleaseFlashCS();
}

/**
 * @brief  禁用写操作
 * @retval None
 */
void WriteDisable()
{
    SelectFlashCS();
    uint8_t command = W25Q64_Write_Disable;
    SendCommand(&command, 1);
    ReleaseFlashCS();
}

/**
 * @brief  检查 Flash 是否处于忙碌状态
 * @retval Busy: Flash 正在忙碌，Spare: Flash 空闲
 */
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

/**
 * @brief  对指定地址进行 4KB 扇区擦除操作
 * @param  address: 指向要擦除的地址的 3 字节数组
 * @retval None
 */
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

/**
 * @brief  从 Flash 读取数据
 * @param  address: 指向要读取的地址的 3 字节数组
 * @param  data: 指向存储读取数据的缓冲区
 * @param  dataSize: 要读取的数据的大小
 * @retval None
 */
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

/**
 * @brief  将数据写入 Flash 的指定页
 * @param  address: 指向要写入的地址的 3 字节数组
 * @param  data: 指向要写入的数据的数组
 * @param  dataSize: 要写入的数据的大小
 * @retval None
 */
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

/**
 * @brief  将 24 位地址转换为 3 字节（高位在前）
 * @param  address: 24 位地址
 * @param  addr_bytes: 存储转换结果的 3 字节数组
 * @retval None
 */
void Convert24BitAddress(uint32_t address, uint8_t addr_bytes[3])
{
    addr_bytes[0] = (address >> 16) & 0xFF; // 高 8 位
    addr_bytes[1] = (address >> 8) & 0xFF;  // 中间 8 位
    addr_bytes[2] = address & 0xFF;         // 低 8 位
}

/**
 * @brief  全片擦除
 * @retval None
 */
void ChipErase(void)
{
    // 等待 Flash 空闲
    while (CheckBusy() == Busy)
        ;

    // 写使能
    WriteEnable();

    // 选中 Flash
    SelectFlashCS();

    // 发送全片擦除指令
    uint8_t cmd = W25Q64_Chip_Erase;
    SendCommand(&cmd, 1);

    // 取消选中
    ReleaseFlashCS();

    // 等待擦除完成（擦除全片可能耗时几秒）
    while (CheckBusy() == Busy)
        ;
}