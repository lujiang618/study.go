package basic

import (
	"testing"
	"time"
)

// ticker 无论是start还是reset都不是立即执行，而是从当前时间开始计时。它的计时从<-t.C开始。
// 根据如上原理，如果当某一次执行时间>=ticker的interval时，这次执行后没有wait的时间，反之有wait的时间。
// 所以程序耗时比较久时，ticker就不是固定的频率执行
func TestTicker(t *testing.T) {
	tk := time.NewTicker(5 * time.Second)
	go func() {
		t.Log("1", time.Now())

		for {
			<-tk.C
			t.Log("2", time.Now())
			time.Sleep(2 * time.Second)
			t.Log("3", time.Now())
		}
	}()

	time.Sleep(2 * time.Second)
	t.Log("0", time.Now())
	tk.Reset(1 * time.Second)

	time.Sleep(5 * time.Second)
}
