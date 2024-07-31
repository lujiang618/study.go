#include <iostream>

/**
在 C++ 中，函数调用的解析遵循静态绑定（静态分派）的规则，而不是动态绑定（动态分派）。

具体来说，解析过程如下：

静态绑定：

当你调用 p->test()，其中 p 的静态类型是 B*，编译器会根据静态类型 B* 来查找调用的方法。在 B 类中并没有重新定义 test() 方法，所以它会沿着继承链向上查找，找到 A 类中的 test() 方法。

虚函数的影响：

尽管 func() 在 B 类中被重新定义（而非重写），但是在 A 类中，test() 调用的 func() 是虚函数，并且它的默认参数是 1。

函数调用解析：

当 test() 内部调用 func() 时，编译器会根据 p 的静态类型 B* 来确定调用的具体函数。因为 func() 在 A 类中是虚函数，所以它会按照 A 类的声明来解析。
在运行时，虽然 p 指向的是 B 类的对象，但编译器已经决定了调用 A 类中的 func(int val = 1)。
默认参数的影响：

func(int val = 1) 中的参数默认值是 1。因此，即使 func() 在 B 类中有一个参数默认值为 0 的版本，但这个默认值在静态绑定时不会影响调用，因为调用的是 A 类中的函数。
综上所述，输出 "A->1" 的原因是 test() 方法内部调用的 func() 函数通过静态绑定解析为 A 类中的虚函数 func(int val = 1)，而不是 B 类中的非虚函数 func(int val = 0)。

 */

class A
{
public:
    virtual void func(int val = 1) { std::cout << "A->" << val << std::endl; }
    virtual void test() { func(); }
};

class B : public A
{
private:
    void func(int val = 0)
    {
        std::cout << "B->" << val << std::endl;
    }
};

int main(int argc, char *argv[])
{
    B *p = new B;
    p->test();
    return 0;
}
