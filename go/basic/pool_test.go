package basic

import (
	"fmt"
	"runtime"
	"sync"
	"testing"
	"time"
)

// 定义一个 Person 结构体，有Name和Age变量
type Person struct {
	Name string
	Age  int
}

// 初始化sync.Pool，new函数就是创建Person结构体
func initPool() *sync.Pool {
	return &sync.Pool{
		New: func() interface{} {
			fmt.Println("创建一个 person.")
			return &Person{}
		},
	}
}

// Get是从pool中取出， Put是归还pool
func TestPool(t *testing.T) {
	pool := initPool()
	person := pool.Get().(*Person)
	fmt.Println("首次从sync.Pool中获取person：", person)
	person.Name = "Jack"
	person.Age = 23
	pool.Put(person)
	fmt.Println("设置的对象Name: ", person.Name)
	fmt.Println("设置的对象Age: ", person.Age)
	fmt.Println("Pool 中有一个对象，调用Get方法获取：", pool.Get().(*Person))
	fmt.Println("Pool 中没有对象了，再次调用Get方法：", pool.Get().(*Person))
}

// https://www.cnblogs.com/zqwlai/p/15269629.html
// sync.pool 命中会受协程的影响
// put时，如果P的private没有值，则保存到private，然后放到P的shared
// get时，如果P的private有值，则返回该值，否则从P的shared获取，如果shared没有从其他P的shared获取
// 所以一个程序的两个协程在两个P时，A放入自己P的private。则B获取不到
func TestPoolGC(t *testing.T) {
	runtime.GOMAXPROCS(2)
	var pool = sync.Pool{
		New: func() interface{} {
			return 0
		},
	}
	go func() {
		pool.Put(1) // 放到私有对象里
		// pool.Put(2) // 放到共享池里
		// pool.Put(3) // 追加到共享池里
	}()

	time.Sleep(1 * time.Second)

	go func() {
		// 如果本地私有对象和共享池里都没有，从其他协程对应P的共享池中获取
		t.Log(pool.Get().(int))
	}()
	time.Sleep(1 * time.Second)
}
