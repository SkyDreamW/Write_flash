#include <iostream>
#include <fstream>

/*  将size和data改为你需要生成.bin文件的数据，然后使用vscode的run code插件运行就可以生成data.bin文件*/
int main()
{
    // 定义数组大小
    const int size = 1025; // 0 到 1024 共 1025 个元素

    // 创建 uint8_t 数组并填充数据
    uint8_t data[size];

    // 打开 .bin 文件进行写入
    std::ofstream outfile("data.bin", std::ios::binary);

    // 检查文件是否成功打开
    if (!outfile)
    {
        std::cerr << "无法打开文件进行写入!" << std::endl;
        return 1; // 如果无法打开文件，返回错误
    }

    // 写入数据到文件
    outfile.write(reinterpret_cast<const char *>(data), size);

    // 关闭文件
    outfile.close();

    std::cout << "数据已成功保存为 data.bin" << std::endl;

    return 0;
}
