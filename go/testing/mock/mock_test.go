package mock

import (
	"errors"
	"testing"

	"github.com/golang/mock/gomock"
)

func TestGetFromDB(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish() // 断言 DB.Get() 方法是否被调用
	m := NewMockDB(ctrl)
	// 打桩
	m.EXPECT().Get(gomock.Eq("Tom")).Return(0, errors.New("not exist"))
	if v := GetFromDB(m, "Tom"); v != -1 {
		t.Fatal("expected -1, but got", v)
	}
}
