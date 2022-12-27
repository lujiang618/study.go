package slice

import "testing"

func TestRead(t *testing.T) {
	var abc = []int{0, 1, 2, 3, 4, 5}

	t.Log(abc[0:])
}
