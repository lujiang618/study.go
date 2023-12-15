package ps

import (
	"testing"

	"github.com/shirou/gopsutil/v3/mem"
)

func TestVirtualMemory(t *testing.T) {
	v, _ := mem.VirtualMemory()
	t.Log(v.Available/1024/1024, v.Free/1024/1024, int(v.UsedPercent))
}
