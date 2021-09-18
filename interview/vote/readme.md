# 问题
有3台redis，分别分布在中国，美国，欧洲。他们距离比较远，数据同步有延迟。现在需要用redis来做分布式锁，如何保证一致性？

# 分析
单机的redis做分布式锁是完全没问题的。但是redis集群做分布式锁时，redis是ap的。并不能保证数据的一致性。

# TODO
- [ ] redis 主从集群如何构建
- [ ] 如何通过etcd构建一个redis集群
- [ ] share nothing 的集群是什么样的？ 数据之间同步的问题如何解决？


# 参考资料
- [使用etcd实现动态分布式选主] (https://blog.csdn.net/liyunlong41/article/details/107619563)
- [etcd使用](https://www.cnblogs.com/yjt1993/p/13183634.html)
- [etcd-client v3 demo](https://github.com/etcd-io/etcd/blob/main/tests/integration/clientv3/examples/example_metrics_test.go)
- [etcd](https://github.com/etcd-io/etcd)