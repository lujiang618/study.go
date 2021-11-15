package vote

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	clientv3 "go.etcd.io/etcd/client/v3"
)

func Vote() error {
	cli, err := clientv3.New(clientv3.Config{
		Endpoints:   []string{"127.0.0.1:2379"},
		DialTimeout: 5 * time.Second,
	})

	if err != nil {
		return errors.Wrap(err, "create etcd client failed")
	}

	var redisCluster = []string{
		"127.0.0.1:6360",
		"127.0.0.1:6361",
		"127.0.0.1:6362",
	}

	var wg *sync.WaitGroup

	v := NewSelectMaster("redis.cluster", cli)
	for _, redis := range redisCluster {
		fmt.Printf("redis is :%s\n", redis)
		v.Campaign(context.Background(), wg, redis)
		time.Sleep(1 * time.Second)
	}

	for {
		master := v.GetMaster()
		fmt.Println("master is ", master)
		time.Sleep(5 * time.Second)
	}

	wg.Wait()

	return nil
}

func run() error {
	cli, err := clientv3.New(clientv3.Config{
		Endpoints:   []string{"127.0.0.1:2379"},
		DialTimeout: 5 * time.Second,
	})

	if err != nil {
		return errors.Wrap(err, "create etcd client failed")
	}

	var redisCluster = []string{
		"127.0.0.1:6360",
		"127.0.0.1:6361",
		"127.0.0.1:6362",
	}

	prefix := "redis.cluster"
	resp, err := cli.Grant(context.TODO(), 50)
	if err != nil {
		return errors.Wrap(err, "grant failed")
	}
	for _, redis := range redisCluster {
		fmt.Printf("redis is :%s\n", redis)

		conn := strings.Split(redis, ":")
		key := fmt.Sprintf("%s:%s", prefix, conn[1])
		_, err := cli.Put(context.TODO(), key, redis, clientv3.WithLease(resp.ID))
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("put %s failed", conn[1]))
		}
	}

	res, err := cli.Get(context.TODO(), prefix, clientv3.WithPrefix())
	if err != nil {
		return errors.Wrap(err, "etcd get error")
	}

	fmt.Printf("get key:%+v", res)

	return nil
}
