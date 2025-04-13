# STM32F103C8T6 与 W25Q64 SPI 接口连接说明

## 项目简介
STM32F103C8T6实现W25Q64的写入，需要USB转串口模块连接电脑和stm32

## 硬件连接

按照CubeMX文件Write_flash_c8t6.ioc连线即可

## 软件烧录

CubeMX打开Write_flash_c8t6.ioc生成代码，把Hardware和User目录的内容添加进path和项目文件即可
