package gotestify

import (
	"fmt"
	"testing"

	. "github.com/smartystreets/goconvey/convey"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"github.com/stretchr/testify/require"
)

func Test_assert(t *testing.T) {
	a := 2
	b := 3
	assert.Equal(t, a+b, 5, "They should be equal")
}

func Test_require(t *testing.T) {
	var name = "dazuo"
	var age = 24
	require.Equal(t, "dazuo", name, "they should be equal")
	require.Equal(t, 24, age, "they should be equal")
}

type Storage interface {
	Store(key, value string) (int, error)
	Load(key string) (string, error)
}

// 测试用例，当真实对象不可用时，使用mock对象代替
type mockStorage struct {
	mock.Mock
}

func (ms *mockStorage) Store(key, value string) (int, error) {
	args := ms.Called(key, value)
	return args.Int(0), args.Error(1)
}
func (ms *mockStorage) Load(key string) (string, error) {
	args := ms.Called(key)
	return args.String(0), args.Error(1)
}
func Test_mock(t *testing.T) {
	mockS := &mockStorage{}
	mockS.On("Store", "name", "dazuo").Return(20, nil).Once()
	var storage Storage = mockS
	i, e := storage.Store("name", "dazuo")
	if e != nil {
		panic(e)
	}
	fmt.Println(i)
}

func TestHttpGetWithTimeOut(t *testing.T) {

	Convey("TestHttpGetWithTimeOut", t, func() {
		Convey("TestHttpGetWithTimeOut normal", func() {
			// ts := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			// 	w.WriteHeader(http.StatusOK)
			// 	w.Write([]byte("TestHttpGetWithTimeOut success!!"))

			// 	if r.Method != "GET" {
			// 		t.Errorf("Except 'Get' got '%s'", r.Method)
			// 	}

			// 	if r.URL.EscapedPath() != "/要访问的url" {
			// 		t.Errorf("Expected request to '/要访问的url', got '%s'", r.URL.EscapedPath())
			// 	}
			// }))

			// api := ts.URL
			// defer ts.Close()
			// var header = make(map[string]string)
			// HttpGetWithTimeOut(api, header, 30)

		})
	})
}
