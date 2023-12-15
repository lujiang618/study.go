package main

import (
	"fmt"

	"github.com/prashantv/gostub"
)

// 1.为全局变量打桩
var counter = 100

func stubGlobalVariable() {
	stubs := gostub.Stub(&counter, 200)
	defer stubs.Reset()
	fmt.Println("Counter:", counter)
}

// 2.为函数打桩
var Exec = func() {
	fmt.Println("Exec")
}

func stubFunc() {
	stubs := gostub.Stub(&Exec, func() {
		fmt.Println("Stub Exec")
	})
	Exec()
	defer stubs.Reset()
}

// 3.为过程打桩（当一个函数没有返回值时，该函数我们一般称为过程。很多时候，我们将资源清理类函数定义为过程。）
var CleanUp = cleanUp

func cleanUp(val string) {
	fmt.Println(val)
}
func stubPath() {
	stubs := gostub.StubFunc(&CleanUp)
	defer stubs.Reset()
	CleanUp("Hello go")
}
func main() {
	stubGlobalVariable()
	stubFunc()
	stubPath()
}
