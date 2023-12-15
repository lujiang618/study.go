package vote

import (
	"testing"
	"time"
)

func TestRun(t *testing.T) {
	err := run()
	if err != nil {
		t.Logf("err:%+v", err)
	}

	time.Sleep(1 * time.Minute)
}

func TestVote(t *testing.T) {
	err := Vote()
	if err != nil {
		t.Logf("err:%+v", err)
	}

	time.Sleep(1 * time.Minute)
}
