#include <iostream>
#include <string>
#include <utility> // std::forward

// std::string 和 char*是不一样的
// 一个接受std::string的函数
void processString(const std::string& str)
{
    std::cout << "Processing lvalue: " << str << std::endl;
}

void processString(std::string&& str)
{
    std::cout << "Processing rvalue: " << str << std::endl;
}

// 完美转发函数
template <typename T>
void forwardString(T&& arg)
{
    processString(std::forward<T>(arg)); // 完美转发参数
}

int main()
{
    std::string s1 = "Hello";
    forwardString(s1);                              // lvalue
    forwardString("World");                         // rvalue
    forwardString(std::string("Temporary String")); // rvalue
    return 0;
}
