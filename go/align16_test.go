package main

import (
	"fmt"
	"testing"
)

func align(v int32, a int32) int32 {
	return (v + a - 1) & ^(a - 1)
}

func align16(x int32) int32 {
	return align(x, 16)
}

// go test align16_test.go -v
func TestAlign16(t *testing.T) {
	w := align(1080, 16)
	h := align(720, 8)

	fmt.Printf("%d %d \n", w, h)

	w = align(1872, 64)
	h = align(720*4, 256)
	fmt.Printf("%d %d \n", w, h)


}
