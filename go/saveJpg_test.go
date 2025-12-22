package main

import (
	"bufio"
	"fmt"
	"image"
	"image/jpeg"
	"os"
	"testing"
	"time"

	"github.com/tal-tech/go-zero/core/logx"
)

func SaveImg(sequence int32, pixels []byte, peerID string, width, height int) error {
	if len(pixels) == 0 {
		return nil
	}

	image := image.NewRGBA(image.Rectangle{Max: image.Point{X: width, Y: height}})
	copy(image.Pix, pixels)

	filePath := fmt.Sprintf("%s/", "./")

	f, err := os.Create(fmt.Sprintf("%s/%d.%d.jpg", filePath, time.Now().UnixMilli(), sequence))
	if err != nil {
		logx.Errorf("create image err:%s", err.Error())
	}

	b := bufio.NewWriter(f)
	if err := jpeg.Encode(b, image, &jpeg.Options{Quality: 90}); err != nil {
		logx.Errorf("encode image err:%s", err.Error())
	}

	b.Flush()
	f.Close()

	return nil
}

func ReadPixelsFromFile(filename string) ([]byte, error) {
	file, err := os.Open(filename)
	if err != nil {
		return nil, fmt.Errorf("打开文件失败: %v", err)
	}
	defer file.Close()

	// 读取文件内容
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, fmt.Errorf("读取文件失败: %v", err)
	}

	return data, nil
}

func TestPixels(t *testing.T) {
	pixels, err := ReadPixelsFromFile("1.txt")
	if err != nil {
		logx.Errorf("读取pixels失败: %v", err)
		return
	}

	t.Logf("读取pixels成功, 长度: %d", len(pixels))

	SaveImg(1, pixels, "1", 1920, 960)
}
