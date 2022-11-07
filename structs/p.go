package structs

import "fmt"

type DeptModeA interface {
	Name() string
	SetName(name string)
}
type DeptModeB interface {
	Relocate(building string, floor uint8)
}
type Dept struct {
	name     string
	building string
	floor    uint8
	Key      string
}

func (r Dept) Name() string {
	return r.name
}
func (r Dept) SetName(name string) {
	r.name = name

	fmt.Printf("dep addr:%p\n", &r)
	fmt.Printf("dept1:%+v\n", r)

}
func (r *Dept) Relocate(building string, floor uint8) {
	r.building = building
	r.floor = floor

	fmt.Printf("dep addr:%p\n", r)
	fmt.Printf("dept1:%+v\n", r)
}
