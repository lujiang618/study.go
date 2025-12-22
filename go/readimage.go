package main

import (
	"fmt"
	"image"
	"image/draw"
	_ "image/jpeg"
	_ "image/png"
	"os"
)

func main() {
	file, err := os.Open("./log.jpg")
	if err != nil {
		panic(err)
	}

	defer file.Close()

	img, _, err := image.Decode(file)
	if err != nil {
		panic(err)
	}

	// 打印图片的宽高

	rgbaImage := image.NewRGBA(img.Bounds())
	draw.Draw(rgbaImage, img.Bounds(), img, img.Bounds().Min, draw.Src)

	fmt.Printf("Width: %d, Height: %d\n", rgbaImage.Rect.Dx(), rgbaImage.Rect.Dy())

	outFile, err := os.Create("pixels.txt")
	if err != nil {
		panic(err)
	}
	defer outFile.Close()

	outFile.Write(rgbaImage.Pix)
}
