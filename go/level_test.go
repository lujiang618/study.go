package main

import "testing"

func CalcLevel(width, height, fps int) string {
	pixelsPerSecond := width * height * fps

	// 调整条件判断顺序，按像素数量从小到大判断
	if pixelsPerSecond <= 62208000 {
		return "Level 4.1"
	}

	if pixelsPerSecond <= 124416000 {
		return "Level 4.2"
	}

	if pixelsPerSecond <= 251658240 {
		return "Level 5.1"
	}

	if pixelsPerSecond <= 503316480 {
		return "Level 5.2"
	}

	return "Level 5.2"
}

func TestLevel(t *testing.T) {
	tests := []struct {
		width, height, fps int
		expected           string
	}{
		{1920, 1080, 30, "Level 4.1"},
		{1920, 1080, 60, "Level 4.2"},
		{3840, 2160, 30, "Level 5.1"},
		{3840, 2160, 60, "Level 5.2"},
	}

	for _, test := range tests {
		actual := CalcLevel(test.width, test.height, test.fps)
		if actual != test.expected {
			t.Errorf("%d %d %d %d Expected %s, got %s", test.width, test.height, test.fps, test.width*test.height*test.fps, test.expected, actual)
		}
	}
}
