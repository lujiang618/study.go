package gopool

import (
	"fmt"
	"testing"
)

func TestSelect(t *testing.T) {

	task := make(chan int)
	sem := make(chan int)

	select {
	case task <- 2:
	case sem <- 1:
		fmt.Println(sem)
	}

	fmt.Println("task-->", task)
	fmt.Println("sem-->", sem)
}
