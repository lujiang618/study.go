#!/usr/local/bin/lua

-- 函数


-- 在Lua中，函数是对语句和表达式进行抽象的主要方法。既可以用来处理一些特殊的工作，也可以用来计算一些值。

-- Lua 提供了许多的内建函数，你可以很方便的在程序中调用它们，如print()函数可以将传入的参数打印在控制台上。

-- Lua 函数主要有两种用途：

-- 1.完成指定的任务，这种情况下函数作为调用语句使用；
-- 2.计算并返回值，这种情况下函数作为赋值语句的表达式使用。

-- Lua 中我们可以将函数作为参数传递给函数
function max(n1, n2, p)
    local result = n2
    if n1 > n2 then
        result = n1
    end

    p(result)

    return result
end

max(19, 4, print)
max(3, 4, print)

-- Lua函数可以返回多个结果值
-- Lua函数中，在return后列出要返回的值的列表即可返回多值
-- Lua 函数可以接受可变数目的参数，和 C 语言类似，在函数参数列表中使用三点 ... 表示函数有可变的参数。
-- 我们也可以通过 select("#",...) 来获取可变参数的数量:
-- 固定参数必须放在变长参数之前

s, e = string.find("www.runoob.com", "runoob")
print(s, e)

print("**************************************************************************")

function add(...)
    local s = 0
    for i, v in ipairs { ... } do --> {...} 表示一个由所有变长参数构成的数组
        s = s + v
    end
    return s
end

print(add(3, 4, 5, 6, 7)) --->25

function average(...)
    result = 0
    local arg = { ... } --> arg 为一个表，局部变量
    for i, v in ipairs(arg) do
        result = result + v
    end
    print("总共传入 " .. #arg .. " 个数")
    return result / #arg
end

print("平均值为", average(10, 5, 3, 4, 5, 6))

function average(...)
    result = 0
    local arg = { ... }
    for i, v in ipairs(arg) do
        result = result + v
    end
    print("总共传入 " .. select("#", ...) .. " 个数")
    return result / select("#", ...)
end

print("平均值为", average(10, 5, 3, 4, 5, 6))


function fwrite(fmt, ...) ---> 固定的参数fmt
    return io.write(string.format(fmt, ...))
end

fwrite("runoob\n")     --->fmt = "runoob", 没有变长参数。
fwrite("%d%d\n", 1, 2) --->fmt = "%d%d", 变长参数为 1 和 2


print("**************************************************************************")

-- 通常在遍历变长参数的时候只需要使用 {…}，然而变长参数可能会包含一些 nil，那么就可以用 select 函数来访问变长参数了：select('#', …) 或者 select(n, …)

-- select('#', …) 返回可变参数的长度。
-- select(n, …) 用于返回从起点 n 开始到结束位置的所有参数列表。
-- 调用 select 时，必须传入一个固定实参 selector(选择开关) 和一系列变长参数。如果 selector 为数字 n，那么 select 返回参数列表中从索引 n 开始到结束位置的所有参数列表，否则只能为字符串 #，这样 select 返回变长参数的总数。

function f(...)
    a = select(3, ...)    -->从第三个位置开始，变量 a 对应右边变量列表的第一个参数
    print(a)
    print(select(3, ...)) -->打印所有列表参数
end

f(0, 1, 2, 3, 4, 5)



do
    function foo(...)
        for i = 1, select('#', ...) do  -->获取参数总数
            local arg = select(i, ...); -->读取参数，arg 对应的是右边变量列表的第一个参数
            print("arg", arg);
        end
    end

    foo(1, 2, 3, 4);
end
