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

		res, ret := device.GetSupportedEventTypes()
		fmt.Printf("event type %v %v", res, ret)
	}
	// time.Sleep(2 * time.Minute)
}

func TestEvent(t *testing.T) {
	// // 初始化NVML
	// ret := nvml.Init()
	// if ret != nvml.SUCCESS {
	// 	fmt.Println("Failed to initialize NVML:", ret)
	// 	return
	// }

	// // 获取第一个GPU的句柄
	// device, ret := nvml.DeviceGetHandleByIndex(0)
	// if ret != nvml.SUCCESS {
	// 	fmt.Println("Failed to get device handle:", ret)
	// 	return
	// }

	// // 创建事件集合
	// eventSet, ret := nvml.EventSetCreate()
	// if ret != nvml.SUCCESS {
	// 	fmt.Println("Failed to create event set:", ret)
	// 	return
	// }

	// // 注册内存变化事件
	// ret = nvml.DeviceRegisterEvents(device, nvml.Memory)
	// if ret != nvml.SUCCESS {
	// 	fmt.Println("Failed to register events:", ret)
	// 	return
	// }

	// device.RegisterEvents()
	// // 设置事件回调函数
	// ret = nvml.EventSetRegisterCallback(eventSet, nvmlCallback, nil)
	// if ret != nvml.SUCCESS {
	// 	fmt.Println("Failed to register callback:", ret)
	// 	return
	// }

	// 启动一个循环，等待事件的发生
	for {
		// 这里可以做一些其他的事情，或者让程序休眠一段时间
		time.Sleep(1 * time.Second)
	}
}

// 定义事件回调函数
// func nvmlCallback(handle nvml.Device, infoType nvml.EventType, eventData uint64, userdata interface{}) {
// 	if infoType == nvml.Memory {
// 		fmt.Println("Memory usage changed")
// 	}
// }
