package vote

import (
	"context"
	"errors"
	"fmt"
	"net"
	"sync"
	"time"

	"github.com/go-redis/redis/v8"
	clientv3 "go.etcd.io/etcd/client/v3"
	"go.etcd.io/etcd/client/v3/concurrency"
)

const CampaignPrefix = "/redis.cluster" // 这是选举的prefix

type SelectMaster struct {
	client   *clientv3.Client
	prefix   string
	session  *concurrency.Session
	election *concurrency.Election
}

func NewSelectMaster(prefix string, c *clientv3.Client) *SelectMaster {
	s, err := concurrency.NewSession(c, concurrency.WithTTL(5))
	if err != nil {
		fmt.Println("NewSession", "error", "err", err)
		time.Sleep(time.Second * 2)
		return nil
	}

	e := concurrency.NewElection(s, prefix)

	return &SelectMaster{
		client:   c,
		prefix:   prefix,
		session:  s,
		election: e,
	}
}

func (v *SelectMaster) Campaign(parentCtx context.Context, wg *sync.WaitGroup, ip string) (success <-chan struct{}) {
	// 我们设置etcd的value为当前机器的ip，这个不是关键
	//ip, _ := getLocalIP()
	// 当外层的context关闭时，我们也会优雅的退出。
	ctx, cancel := context.WithCancel(parentCtx)
	// ctx的作用是让外面通知我们要退出，wg的作用是我们通知外面已经完全退出了。当然外面要wg.Wait等待我们。
	if wg != nil {
		wg.Add(1)
	}

	go func() {
		redisClient := redis.NewClient(&redis.Options{
			Addr:     ip,
			DB:       0,
			Password: "gelu8888",
		})

		for {
			if err := redisClient.Ping(ctx).Err(); err != nil {
				fmt.Printf("%s ping err %+v\n", ip, err)
				cancel()
				return
			}
			time.Sleep(1 * time.Second)
		}
	}()

	// 创建一个信号channel，并返回，所有worker可以监听这个channel，这种实现可以让worker阻塞等待节点成为leader，而不是轮询是否是leader节点。
	// 返回只读channel，所有worker可以阻塞在这。
	notify := make(chan struct{}, 100)
	go func() {
		defer func() {
			if wg != nil {
				wg.Done()
			}
		}()
		//调用Campaign方法，成为leader的节点会运行出来，非leader节点会阻塞在里面。
		if err := v.election.Campaign(ctx, ip); err != nil {
			fmt.Println("Campaign", "error", "err", err)
		}
		fmt.Println("campaign", "success", "ip", ip)
	}()

	<-ctx.Done()
	ctxTmp, _ := context.WithTimeout(context.Background(), time.Second*1)
	v.election.Resign(ctxTmp)

	return notify
}

func (v *SelectMaster) GetMaster() string {
	var masterName string
	cctx, cancel := context.WithCancel(context.TODO())
	defer cancel()

	select {
	case resp := <-v.election.Observe(cctx):
		if len(resp.Kvs) > 0 {
			masterName = string(resp.Kvs[0].Value)
			// fmt.Println("get master with:", masterName)
		}
	}

	return masterName
}

// 获取本机网卡IP
func getLocalIP() (ipv4 string, err error) {
	var (
		addrs   []net.Addr
		addr    net.Addr
		ipNet   *net.IPNet // IP地址
		isIpNet bool
	)
	// 获取所有网卡
	if addrs, err = net.InterfaceAddrs(); err != nil {
		return
	}
	// 取第一个非lo的网卡IP
	for _, addr = range addrs {
		//fmt.Println(addr)
		// 这个网络地址是IP地址: ipv4, ipv6
		if ipNet, isIpNet = addr.(*net.IPNet); isIpNet && !ipNet.IP.IsLoopback() {
			// 跳过IPV6
			if ipNet.IP.To4() != nil {
				ipv4 = ipNet.IP.String() // 192.168.1.1
				return
			}
		}
	}

	err = errors.New("no local ip")
	return
}
