#!/usr/local/bin/lua

-- lua的基本语法

--[[
多行注释
--]]
print("hello")
print("monday")

-- 标识符： 字母 、_ 开头， 由字母、数字、下划线组成 不允许使用特殊字符
-- lua保留字是 下划线+大写字母 格式
-- 区分大小写  


-- 关键字： and break do else elseif end false for function if in local nil not or repeat return then true until while goto
-- 一般约定，以下划线连接一串大写字母的名字（如 _VERSION） 被保留用于lua内部全局变量
-- 在默认情况下，变量总是认为是全局的。
-- 全局变量不需要声明，给一个变量赋值后即创建了这个全局变量，访问一个没有初始化的全局变量也不会出错，只不过得到的结果是：nil。


-- lua 是动态类型语言，变量不要类型定义，只需要为变量赋值。值可以存储在变量中，作为参数传递或结果返回。

-- lua中有8个基本类型分别为： nil、boolean、number、string、userdata、function、 thread 、 table 

-- nil 这个最简单，只有值nil属于该类，表示一个无效值（在条件表达式中相当于false）。
-- boolean	包含两个值：false和true。
-- number	表示双精度类型的实浮点数
-- string	字符串由一对双引号或单引号来表示
-- function	由 C 或 Lua 编写的函数
-- userdata	表示任意存储在变量中的C数据结构
-- thread	表示执行的独立线路，用于执行协同程序
-- table	Lua 中的表（table）其实是一个"关联数组"（associative arrays），数组的索引可以是数字、字符串或表类型。在 Lua 里，table 的创建是通过"构造表达式"来完成，最简单构造表达式是{}，用来创建一个空表。
print(type("Hello world")) --> string
print(type(10.4*3))   --> number
print(type(print))   --> function
print(type(type))    --> function
print(type(true))    --> boolean
print(type(nil))    --> nil
print(type(type(X)))  --> string

print("**************************************************************************")
-- 对于全局变量和 table，nil 还有一个"删除"作用，给全局变量或者 table 表里的变量赋一个 nil 值，等同于把它们删掉
tab1 = { key1 = "val1", key2 = "val2", "val3" }
for k, v in pairs(tab1) do
  print(k .. " - " .. v)
end

tab1.key1 = nil
for k, v in pairs(tab1) do
  print(k .. " - " .. v)
end

-- nil 作比较时应该加上双引号 "：
-- type(X)==nil 结果为 false 的原因是 type(X) 实质是返回的 "nil" 字符串，是一个 string 类
print(type(X)==nil)
print(type(X)=="nil")


-- Lua 把 false 和 nil 看作是 false，其他的都为 true，数字 0 也是 true:


print("**************************************************************************")
-- 可以用 2 个方括号 "[[]]" 来表示"一块"字符串。

html = [[
<html>
<head></head>
<body>
    <a href="http://www.runoob.com/">菜鸟教程</a>
</body>
</html>
]]
print(html)

-- 在对一个数字字符串上进行算术操作时，Lua 会尝试将这个数字字符串转成一个数字
-- 字符串连接使用的是 ..
print("2" + 6)
print("2 + 6")


print("**************************************************************************")
-- 在 Lua 里，table 的创建是通过"构造表达式"来完成，最简单构造表达式是{}，用来创建一个空表。也可以在表里添加一些数据，直接初始化表:
-- Lua 中的表（table）其实是一个"关联数组"（associative arrays），数组的索引可以是数字或者是字符串。 这点和php的数组一样
-- 不同于其他语言的数组把 0 作为数组的初始索引，在 Lua 里表的默认初始索引一般以 1 开始。
-- table 不会固定长度大小，有新数据添加时 table 长度会自动增长，没初始的 table 都是 nil。

-- 创建一个空的 table
local tbl1 = {}

-- 直接初始表
local tbl2 = {"apple", "pear", "orange", "grape"}
tbl2[5] = "banana"

for k, v in pairs(tbl2) do
    print(k..":"..v)
end

print("**************************************************************************")
-- 在 Lua 中，函数是被看作是"第一类值（First-Class Value）"，函数可以存在变量里:

-- function_test.lua 脚本文件
function factorial1(n)
    if n == 0 then
        return 1
    else
        return n * factorial1(n - 1)
    end
end

print(factorial1(5))
factorial2 = factorial1
print(factorial2(5))

-- function 可以以匿名函数（anonymous function）的方式通过参数传递:
-- function_test2.lua 脚本文件
function testFun(tab,fun)
    for k ,v in pairs(tab) do
        print(fun(k,v));
    end
end

tab={key1="val1",key2="val2"};
testFun(tab,
function(key,val)--匿名函数
    return key.."="..val;
end
);

print("**************************************************************************")
-- 在 Lua 里，最主要的线程是协同程序（coroutine）。它跟线程（thread）差不多，拥有自己独立的栈、局部变量和指令指针，可以跟其他协同程序共享全局变量和其他大部分东西。

-- 线程跟协程的区别：线程可以同时多个运行，而协程任意时刻只能运行一个，并且处于运行状态的协程只有被挂起（suspend）时才会暂停。

-- userdata 是一种用户自定义数据，用于表示一种由应用程序或 C/C++ 语言库所创建的类型，可以将任意 C/C++ 的任意数据类型的数据（通常是 struct 和 指针）存储到 Lua 变量中调用。

