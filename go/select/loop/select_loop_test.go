package loop

import "testing"

func TestLoop(t *testing.T) {
	counter := make(chan int, 1)

	for i := 0; i < 5; i++ {
		select {
		case <-counter:
			t.Log("read ....")
		default:
			t.Log("default")
		}

		t.Log("i-->", i)
	}

	t.Log("-----------------------------------------------------------------------")
	for j := 0; j < 5; j++ {
		select {
		case counter <- j:
			t.Log("write ....")
		default:
			t.Log("default")
		}

		t.Log("j-->", j)
	}
}
