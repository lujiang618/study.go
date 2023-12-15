package monkey

import (
	"bufio"
	"fmt"
	"os"
	"strings"
	"testing"

	"github.com/agiledragon/gomonkey"
	"github.com/smartystreets/goconvey/convey"
	"github.com/stretchr/testify/assert"
)

// 假设networkFunc是一个网络调用
func networkFunc(a, b int) int {
	return a + b
}

// 本地单测一般不会进行网络调用，所以要mock住networkFunc
func Test_MockNetworkFunc(t *testing.T) {
	convey.Convey("123", t, func() {
		p := gomonkey.NewPatches()
		defer p.Reset()
		p.ApplyFunc(networkFunc, func(a, b int) int {
			fmt.Println("in mock function")
			return a + b
		})
		result := networkFunc(10, 20)
		t.Log(result)
	})
}

func TestProcessFirstLineWithMock(t *testing.T) {
	patch := gomonkey.NewPatches()
	patch.ApplyFunc(ReadFirstLine, func() string {
		return "line110"
	})
	defer patch.Reset()
	line := ProcessFirstLine()
	assert.Equal(t, "line000", line)
}

func ReadFirstLine() string {
	open, err := os.Open("log")
	defer open.Close()
	if err != nil {
		return ""
	}
	scanner := bufio.NewScanner(open)
	for scanner.Scan() {
		return scanner.Text()
	}
	return ""
}

func ProcessFirstLine() string {
	line := ReadFirstLine()
	destLine := strings.ReplaceAll(line, "11", "00")
	return destLine
}
