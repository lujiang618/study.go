package structs

import (
	"fmt"
	"testing"
)

func TestP(t *testing.T) {

	dept1 := Dept{
		name:     "MySohu",
		building: "Internet",
		floor:    7,
	}

	// 假设T是struct，那么Go里面遵循下面几个原则：
	//     1. T的方法集仅拥有 T Receiver （方法中的接受者）方法。
	//     2. *T 方法集则包含全部方法 (T + *T)。
	switch v := interface{}(dept1).(type) {
	case DeptModeB:
		fmt.Printf("The dept1 is a DeptModeB.\n")
	case DeptModeA:
		fmt.Printf("The dept1 is a DeptModeA.\n")
	default:
		fmt.Printf("The type of dept1 is %v\n", v)
	}

	t.Log("***************************************************************************")

	// 结构体方法副本传参与指针传参的区别
	// SetName修改了结构体的变量，但是只是在这个方法生效，方法外没有生效，而且两次调用发现 调用SetName的dept指针不同
	// 这说明每一次Test1的调用，都是传入的结构体b的一个副本(拷贝)，当在Test1中对内部变量的任何改动，都将会失效(因为下一次访问的时候传入的是b结构体新的副本)。
	t.Logf("dept addr:%p", &dept1)
	dept1.SetName("a")
	dept1.SetName("b")

	t.Logf("dept1:%+v", dept1)

	t.Log("***************************************************************************")

	// 副本接受者调用指针接受者的方法，实际传入的是它的指针
	dept1.Relocate("c", 88)
	t.Logf("dept1:%+v", dept1)

	t.Log("***************************************************************************")

	// 传入指针，接受者的地址没有变化
	dept := &Dept{
		name:     "MySohu",
		building: "Internet",
		floor:    7,
	}
	t.Logf("dept addr:%p", dept)
	dept.Relocate("d", 99)
	t.Logf("dept1:%+v", dept1)

	t.Log("***************************************************************************")

	dept.SetName("e")
	t.Logf("dept:%+v", dept)
}
