package main_test

import (
	"testing"

	"github.com/spf13/viper"
)

func TestViper(t *testing.T) {
	viper.SetConfigName("admin") // name of config file (without extension)
	viper.SetConfigType("yaml")
	viper.AddConfigPath("./")
	err := viper.ReadInConfig() // Find and read the config file
	if err != nil {             // Handle errors reading the config file
		t.Log(err)
	}
}

func TestAbc(t *testing.T) {
	t.Log("1111")
}
