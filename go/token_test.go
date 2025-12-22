package main

import (
	"encoding/base64"
	"fmt"
	"net/url"
	"strings"
	"testing"
)

func TestToken(t *testing.T) {
	data := "QmVhcmVyIGV5SmhiR2NpT2lKSVV6STFOaUlzSW5SNWNDSTZJa3BYVkNKOS5leUpoZFdRaU9pSmlNV0poTldWaU5qWTRPV0kwT0dOaE9EVTNOMlExTXpobE5Ea3dNbU5sTWlJc0ltVjRjQ0k2TVRZMk5qSTBNekF5TVN3aWMzVmlJam9pTVRVNEluMC42UHRtdHZNSTJtTHJUcXNRZHY5czZHSVoyRXJ3NjRfRzBMUEpFM3ZVWW5V&0&cGFhcy1zdG9yYWdlL3BlcnNvbi83MDAwVy5hc20%3D&&cGM%3D&MTkyMA%3D%3D&OTU5"

	arr := strings.Split(data, "&")

	for _, v := range arr {
		fmt.Println(v)
	}

	token, _ := ProcessWebsocketParams(arr[0])

	fmt.Println(string(token))

	path, _ := ProcessWebsocketParams(strings.TrimSpace(arr[2]))

	fmt.Println(path)
}

func ProcessWebsocketParams(param string) (string, error) {
	param, err := url.QueryUnescape(param)
	if err != nil {
		return "", err
	}

	paramByte, err := base64.StdEncoding.DecodeString(param)
	if err != nil {
		return "", err
	}

	return string(paramByte), nil
}
