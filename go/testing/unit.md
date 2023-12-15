
# 概念
- mock
- 打桩、调桩、桩注入
- 表格驱动测试


# 单元测试一般有两个目标：幂等、稳定。
- 幂等：重复执行一个用例、调用一个接口，返回的结果是一样的。
- 稳定：单元测试是相互隔离的，在任何时间都能独立运行。


# 总结
1. 测试的数据，需要mock
2. 对于需要调用外部api的功能，最好mock数据。
3. 关键在于覆盖率，不在于每个方法都有单元测试
4. 单元测试过后，机器状态保持不变
5. 注意循环依赖的情况


常用的单元测试框架有：
- go testing
- [goconvey](https://github.com/smartystreets/goconvey)
- [testify](https://github.com/stretchr/testify)
- [gostub](https://github.com/prashantv/gostub)
- [gomock](https://github.com/golang/mock)
- [gomonkey](https://github.com/agiledragon/gomonkey)


覆盖率
```
# 生成覆盖率profile
go test -coverprofile c.out
go tool cover -html=c.out
```
# 参考资料
- [Golang单元测试框架整理](https://www.shuzhiduo.com/A/gGdXeaQpz4/)
- [allure-go](https://github.com/dailymotion/allure-go)
- [golang单元测试框架实践](https://blog.csdn.net/wtl1992/article/details/124773222)
- [GoLang单元测试（思想、框架、实践）](https://zhuanlan.zhihu.com/p/502690977)
- [接口Mock注入的5种姿势](https://mp.weixin.qq.com/s?__biz=MzI4NzczNjkxOQ==&mid=2247485304&idx=1&sn=fa55fb21a7244bc5ea5d075df0e1beaf)
- [gocheck](https://gopkg.in/check.v1)
- [github-gocheck](https://github.com/go-check/check/tree/v1)
- [Golang 单元测试框架 gocheck 使用介绍](https://www.infoq.cn/article/jRuJKgFUESpgUaqugiwe/)
- [go测试框架gomonkey的使用](https://cloud.tencent.com/developer/article/1872029)