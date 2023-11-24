package nvidia

import (
	"fmt"
	"log"
	"testing"
	"time"

	"github.com/NVIDIA/go-nvml/pkg/nvml"
)

func TestABC(t *testing.T) {
	ret := nvml.Init()
	if ret != nvml.SUCCESS {
		log.Fatalf("Unable to initialize NVML: %v", nvml.ErrorString(ret))
	}
	defer func() {
		ret := nvml.Shutdown()
		if ret != nvml.SUCCESS {
			log.Fatalf("Unable to shutdown NVML: %v", nvml.ErrorString(ret))
		}
	}()

	count, ret := nvml.DeviceGetCount()
	if ret != nvml.SUCCESS {
		log.Fatalf("Unable to get device count: %v", nvml.ErrorString(ret))
	}

	for i := 0; i < count; i++ {
		device, ret := nvml.DeviceGetHandleByIndex(i)
		if ret != nvml.SUCCESS {
			log.Fatalf("Unable to get device at index %d: %v", i, nvml.ErrorString(ret))
		}

		uuid, ret := device.GetUUID()
		if ret != nvml.SUCCESS {
			log.Fatalf("Unable to get uuid of device at index %d: %v", i, nvml.ErrorString(ret))
		}

		fmt.Printf("%v\n", uuid)

		memory, err := device.GetMemoryInfo()
		if err != nvml.SUCCESS {
			fmt.Printf("Failed to get memory info for device %d: %v\n", i, err)
			continue
		}

		fmt.Printf("memory %d %d %d \n", memory.Total, memory.Free, memory.Used)
	}

	time.Sleep(2 * time.Minute)
}
