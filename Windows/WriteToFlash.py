import serial
import struct
import time


# CRC16-MODBUS计算函数
def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc


# 定义命令和响应
CMD_PC_READY = b'P'  # PC准备完成（Prepare）
CMD_DATA_READY = b'R'  # PC准备发送数据（Ready to Send）
CMD_FINISH = b'F'  # 发送结束（Finish）

RESP_FLASH_READY = b'K'  # Flash连接正常（OK）
RESP_DATA_OK = b'C'  # 数据校验正确（Correct）
RESP_DATA_ERR = b'E'  # 数据校验错误（Error）
RESP_WRITE_OVER = b'O'  # 写入完毕（Over）
RESP_DATA_RECEIVE_READY = b'T'  # 接收数据准备完成（Transmit Ready）
RESP_DATA_RECEIVE_ERROR = b'X'  # 接收数据错误（eXception）


# 读取.bin文件并发送数据
def send_file(serial_port: str, file_path: str):
    try:
        # 打开串口
        ser = serial.Serial(serial_port, 115200, timeout=1)
        print(f"连接到串口 {serial_port}")

        # 发送 PC 准备完成命令
        ser.write(CMD_PC_READY)

        # 等待 从机 响应 "Flash连接正常"
        while ser.read(1) != RESP_FLASH_READY:
            pass
        print("Flash连接正常，准备发送数据")

        # 打开.bin文件
        with open(file_path, 'rb') as file:
            data = file.read()

        # 发送数据前，发送 "PC准备发送数据" 命令
        ser.write(CMD_DATA_READY)

        # 发送数据
        for i in range(0, len(data), 256):  # 分批发送，每次发送256字节
            chunk = data[i:i + 256]

            # 如果剩余字节不足256字节，使用0xFF填充
            if len(chunk) < 256:
                chunk = chunk + b'\xFF' * (256 - len(chunk))
                endval = 0
            else:
                endval = 1

            # 计算 CRC 校验
            crc = crc16_modbus(chunk)
            # CRC 校验低字节和高字节
            crc_low = crc & 0xFF
            crc_high = (crc >> 8) & 0xFF

            # 发送数据块和 CRC 校验
            package = chunk + bytes([endval,crc_low, crc_high])

            # 调用发送函数并处理重发
            send_data_with_retry(ser, package)

        # 等待 从机 响应 "写入完毕"
        if ser.read(1) == RESP_WRITE_OVER:
            print("数据写入完毕")

        ser.close()

    except Exception as e:
        print(f"发生错误: {e}")


# 发送数据并进行重试处理
def send_data_with_retry(ser, package, max_retries=5):
    retries = 0
    while retries < max_retries:
        # 接收从机的接收准备完成指令
        while ser.read(1) != RESP_DATA_RECEIVE_READY:
            pass

        # 发送数据
        print("开始发送")
        ser.write(package)
        print("发送完毕")

        # 等待 从机 响应
        response = b'0'
        while response != RESP_DATA_OK and response != RESP_DATA_ERR:
            response = ser.read(1)

        if response == RESP_DATA_OK:
            print("数据校验正确")
            break  # 数据校验正确，退出循环
        elif response == RESP_DATA_ERR:
            print("数据校验错误, 正在重试...")
            retries += 1
            if retries >= max_retries:
                print("重试次数超过最大限制，放弃该数据块的发送")
                error()
        else:
            print(f"接收到未知响应: {response}")
            error()

        time.sleep(0.1)  # 等待一下再重试

def error():
    print("发送失败")
    while 1:
        pass

if __name__ == "__main__":
    # 设置串口和文件路径
    serial_port = 'COM5'  # 根据实际情况修改
    file_path = 'E:\\code\\stm32\\Write_flash_c8t6\\Windows\\data.bin'  # 根据实际情况修改

    send_file(serial_port, file_path)
