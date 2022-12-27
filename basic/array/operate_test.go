package array

import "testing"

var number int

func TestUpdate(t *testing.T) {
	// arr := make([]int, 0)
	// add(arr)
	// t.Logf("arr:%+v", arr)

	arr := make([]int, 5, 5)
	for i := 0; i < 5; i++ {
		add(arr)
		t.Logf("arr:%+v", arr)
	}

}

func add(arr []int) {
	number++
	// arr = append(arr, number)
	arr[number] = number

}
