package main

import (
	"fmt"
	"reflect"
	"sync"

	"github.com/apolloconfig/agollo/v4/storage"
	"github.com/davecgh/go-spew/spew"
)

type CustomChangeListener struct {
	wg sync.WaitGroup
}

func (c *CustomChangeListener) OnChange(changeEvent *storage.ChangeEvent) {
	//write your code here
	fmt.Println(changeEvent.Changes)
	for key, value := range changeEvent.Changes {
		fmt.Println("change key : ", key, ", value :", value)
		// v, err := GetStructStringField(cf, "NAME")
		v := reflect.ValueOf(&cf).Elem()
		if reflect.ValueOf(value.NewValue).Type() == v.FieldByName(key).Type() {
			v.FieldByName(key).Set(reflect.ValueOf(value.NewValue))
		}

		fmtCf()
	}
	fmt.Println(changeEvent.Namespace)
	c.wg.Done()
}

func (c *CustomChangeListener) OnNewestChange(event *storage.FullChangeEvent) {
	//write your code here
	fmt.Println("*****************************************************************")
	spew.Dump(event.Changes)
}
