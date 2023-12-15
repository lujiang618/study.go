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


