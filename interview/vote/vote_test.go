package vote

import "testing"

func TestVote(t *testing.T) {
	err := run()
	if err != nil {
		t.Logf("err:%+v", err)
	}
}
