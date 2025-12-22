#include <iostream>
#include <cstdint> // 包含 int32_t 和 int64_t 的定义
#include <vector>
#include <utility>

int64_t combineInt32ToInt64(int32_t high, int32_t low)
{
    // 将 high 左移 32 位，放到 int64 的高位
    int64_t result = static_cast<int64_t>(high) << 32;
    // 将 low 放到 int64 的低位
    result |= static_cast<int64_t>(low) & 0xFFFFFFFF; // 确保 low 只占用低 32 位
    return result;
}

int32_t extractHighInt32FromInt64(int64_t combined)
{
    // 提取高位：将 combined 右移 32 位
    return static_cast<int32_t>(combined >> 32);
}

int32_t extractLowInt32FromInt64(int64_t combined)
{
    // 提取低位：将 combined 与 0xFFFFFFFF 进行按位与操作
    return static_cast<int32_t>(combined & 0xFFFFFFFF);
}

int main()
{

    std::vector<std::pair<int, int>> data;
    data.push_back(std::make_pair(1, 2));
    data.push_back(std::make_pair(1, 3));
    data.push_back(std::make_pair(1, 4));
    data.push_back(std::make_pair(2, 2));
    data.push_back(std::make_pair(2, 3));
    data.push_back(std::make_pair(2, 4));
    data.push_back(std::make_pair(3, 2));
    data.push_back(std::make_pair(3, 3));
    data.push_back(std::make_pair(3, 4));
    data.push_back(std::make_pair(11, -4));
    data.push_back(std::make_pair(-11, 4));

    for (auto it : data)
    {
        int64_t combined = combineInt32ToInt64(it.first, it.second);
        std::cout << "Combined int64: " << combined << std::endl;
        // std::cout << "Combined int64 hex: " << std::hex << combined << std::endl;

        int32_t high = extractHighInt32FromInt64(combined);
        int32_t low = extractLowInt32FromInt64(combined);

        std::cout << "Extracted high int32: " << high << std::endl;
        std::cout << "Extracted low int32: " << low << std::endl;
    }


    return 0;
}