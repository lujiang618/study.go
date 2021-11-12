package main

// 参考资料：
// - [golang etcd选主实验](https://blog.csdn.net/oqqYuan1234567890/article/details/105153665/)
import (
	"context"
	"flag"
	"fmt"
	"os"
	"os/signal"
	"time"

	clientv3 "go.etcd.io/etcd/client/v3"
	"go.etcd.io/etcd/client/v3/concurrency"
)

var (
	serverName = flag.String("name", "", "name of this server")
)

func main() {
	flag.Parse()
	if len(*serverName) == 0 {
		panic("server name empty")
	}

	endpoints := []string{"http://localhost:2379"}
	cli, err := clientv3.New(clientv3.Config{Endpoints: endpoints})
	if err != nil {
		panic(err)
	}
	defer cli.Close()

	s1, err := concurrency.NewSession(cli)
	if err != nil {
		panic(err)
	}
	defer s1.Close()
	e1 := concurrency.NewElection(s1, "/my-election")

	go func() {
		// 开始竞选 这里只执行1次，集群内每个对象都有该方法，执行时，说明是master
		if err := e1.Campaign(context.Background(), *serverName); err != nil {
			fmt.Println(err)
		} else {
			fmt.Println("campaign..............................................")
		}

	}()

	masterName := ""

	// 每隔1s 查看一下当前的master是谁
	go func() {
		cctx, cancel := context.WithCancel(context.TODO())
		defer cancel()
		timer := time.NewTimer(time.Second)
		for range timer.C {
			timer.Reset(time.Second)
			select {
			case resp := <-e1.Observe(cctx):
				if len(resp.Kvs) > 0 {
					masterName = string(resp.Kvs[0].Value)
					fmt.Println("get master with:", masterName)
				}

			}
		}

	}()

	// 每隔5s查看一下自己是否master
	go func() {
		timer := time.NewTimer(5 * time.Second)
		for range timer.C {
			timer.Reset(5 * time.Second)
			if masterName == *serverName {
				fmt.Println("oh, i'm master")
			} else {
				fmt.Println("slave!!")
			}
		}

	}()

	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt, os.Kill)

	s := <-c
	fmt.Println("Got signal:", s)
	e1.Resign(context.TODO()) // 辞去 master

}
