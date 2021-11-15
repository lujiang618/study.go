package main

import (
	"fmt"
	"time"

	clientv3 "go.etcd.io/etcd/client/v3"
)

func TestEtcdLock() {
	var conf = clientv3.Config{
		Endpoints:   []string{"172.16.196.129:2380", "192.168.50.250:2380"},
		DialTimeout: 5 * time.Second,
	}
	eMutex1 := &EtcdMutex{
		Conf: conf,
		Ttl:  10,
		Key:  "lock",
	}
	eMutex2 := &EtcdMutex{
		Conf: conf,
		Ttl:  10,
		Key:  "lock",
	}
	//groutine1
	go func() {
		err := eMutex1.Lock()
		if err != nil {
			fmt.Println("groutine1抢锁失败")
			fmt.Println(err)
			return
		}
		fmt.Println("groutine1抢锁成功")
		time.Sleep(10 * time.Second)
		defer eMutex1.UnLock()
	}()

	//groutine2
	go func() {
		err := eMutex2.Lock()
		if err != nil {
			fmt.Println("groutine2抢锁失败")
			fmt.Println(err)
			return
		}
		fmt.Println("groutine2抢锁成功")
		defer eMutex2.UnLock()
	}()
	time.Sleep(30 * time.Second)
}
