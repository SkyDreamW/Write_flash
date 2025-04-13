# STM32 Write Flash 项目

## 项目简介
该项目包含一个 STM32 系统的 Flash 写入模块和一个 PC 端的上位机程序。用户可以通过上位机程序将数据写入 STM32 上的 Flash 存储器。

- **源代码** 位于 `User` 文件夹。
- **PC 端上位机程序** 位于 `Windows` 文件夹，包含用于生成数据并发送至 STM32 的程序。

## 使用方法

### 1. 生成 .bin 文件
在 `Windows` 文件夹中的 `makebin.cpp` 脚本用于将数组生成 `.bin` 文件。你可以通过 vscode 的 `run_code` 插件来执行该程序并生成 `.bin` 文件。

### 2. 使用上位机发送程序
`WriteToFlash.bat` 是上位机程序的启动脚本。执行此脚本将启动 `WriteToFlash.exe`，并允许你通过串口将 `.bin` 文件中的数据发送到 STM32。

```bash
WriteToFlash.bat
