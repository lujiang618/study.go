package main

import (
	"fmt"
	"os"
	"strings"

	"github.com/apolloconfig/agollo/v4"
	"github.com/apolloconfig/agollo/v4/constant"
	"github.com/apolloconfig/agollo/v4/env/config"
	"github.com/apolloconfig/agollo/v4/extension"
	"github.com/davecgh/go-spew/spew"
	"github.com/spf13/viper"
)

// TODO:
// 热发布时，无法修改config结构体。而且会在备份的配置文件中追加内容

var vp = viper.New()
var cf Config

func init() {
	vp.SetConfigType("yml")
}
func main() {
	if err := os.Remove(".abc.yaml"); err != nil {
		fmt.Printf("remove err:%v\n", err)
	}

	c := &config.AppConfig{
		AppID:          "dxyp",
		Cluster:        "DEV",
		IP:             "http://127.0.0.1:8080",
		NamespaceName:  "adminApi.yaml",
		IsBackupConfig: false,
		Secret:         "ff4cb4e21dfe4076bfe20e9c723de087",
	}
	c2 := &CustomChangeListener{}
	c2.wg.Add(1)

	agollo.SetLogger(&DefaultLogger{})
	// fileHandler := &FileHandler{}
	// extension.SetFileHandler(fileHandler)
	extension.AddFormatParser(constant.YAML, &Parser{})

	client, err := agollo.StartWithConfig(func() (*config.AppConfig, error) {
		return c, nil
	})

	client.AddChangeListener(c2)
	fmt.Println("connect success..........................................................")
	if err != nil {
		fmt.Println("err:", err)
		panic(err)
	}

	for _, n := range strings.Split(c.NamespaceName, ",") {
		cache := client.GetConfigCache(n)
		cache.Range(func(key, value interface{}) bool {
			fmt.Println("key : ", key, ", value :", value)
			return true
		})
		name, err := cache.Get("name")
		if err != nil {
			fmt.Printf("get Err:%v\n", err)
		}
		if name != nil {
			fmt.Printf("Name:%s........................................\n", name.(string))
		}

	}

	config := client.GetConfig("adminApi.yaml")
	host := config.GetStringValue("host", "none")
	fmt.Println("host--------------------->", host)
	// vp.SetConfigFile("./abc.yaml")

	// if err := vp.ReadInConfig(); err != nil {
	// 	panic(fmt.Errorf("Fatal error config file: %s \n", err))
	// }

	// if err := vp.Unmarshal(&cf); err != nil {
	// 	fmt.Println("unmarshal err:", err)
	// }

	// viper.OnConfigChange(func(e fsnotify.Event) {
	// 	fmt.Println("Config file changed:", e.Name)
	// })
	// viper.WatchConfig()

	c2.wg.Wait()
	// writeConfig(c.NamespaceName, client)
}

func writeConfig(namespace string, client agollo.Client) {
	cache := client.GetConfigCache(namespace)
	cache.Range(func(key, value interface{}) bool {
		fmt.Println("key1 : ", key, ", value :", value)
		return true
	})
}

func fmtCf() {
	spew.Dump(cf)
}
