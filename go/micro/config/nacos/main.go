package main

import (
	"bufio"
	"fmt"
	"os"

	"github.com/nacos-group/nacos-sdk-go/clients"
	"github.com/nacos-group/nacos-sdk-go/common/constant"
	"github.com/nacos-group/nacos-sdk-go/vo"
)

const (
	ServerIpAddr = "127.0.0.1"
	ServerPort   = 9999
)

// 不同环境，应用以下参数会有不同
// NamespaceId， Username， Password
func main() {
	sc := []constant.ServerConfig{
		{
			IpAddr: ServerIpAddr,
			Port:   ServerPort,
		},
	}
	//or a more graceful way to create ServerConfig
	// _ = []constant.ServerConfig{
	// 	*constant.NewServerConfig(
	// 		"127.0.0.1",
	// 		9999,
	// 		constant.WithScheme("http"),
	// 		constant.WithContextPath("/nacos")),
	// }

	cc := constant.ClientConfig{
		NamespaceId:         os.Getenv("NACOS_NAMESPACEID"), //namespace id
		TimeoutMs:           5000,
		NotLoadCacheAtStart: true,
		LogDir:              "./log",
		CacheDir:            "./cache",
		RotateTime:          "1h",
		MaxAge:              3,
		LogLevel:            "debug",
		Username:            os.Getenv("NACOS_USERNAME"),
		Password:            os.Getenv("NACOS_PASSWORD"),
	}
	//or a more graceful way to create ClientConfig
	// _ = *constant.NewClientConfig(
	// 	constant.WithNamespaceId("74ab6cb4-2503-4c82-b966-75043f8ea730"),
	// 	constant.WithTimeoutMs(5000),
	// 	constant.WithNotLoadCacheAtStart(true),
	// 	constant.WithLogDir("/tmp/nacos/log"),
	// 	constant.WithCacheDir("/tmp/nacos/cache"),
	// 	constant.WithRotateTime("1h"),
	// 	constant.WithMaxAge(3),
	// 	constant.WithLogLevel("debug"),
	// )

	// a more graceful way to create config client
	client, err := clients.NewConfigClient(
		vo.NacosClientParam{
			ClientConfig:  &cc,
			ServerConfigs: sc,
		},
	)

	if err != nil {
		panic(err)
	}

	//publish config
	// config key=dataId+group+namespaceId
	_, err = client.PublishConfig(vo.ConfigParam{
		DataId:  "test-data",
		Group:   "test-group",
		Content: "hello world!",
	})
	// _, err = client.PublishConfig(vo.ConfigParam{
	// 	DataId:  "test-data-2",
	// 	Group:   "test-group",
	// 	Content: "hello world!",
	// })
	// if err != nil {
	// 	fmt.Printf("PublishConfig err:%+v \n", err)
	// }

	// //get config
	content, err := client.GetConfig(vo.ConfigParam{
		DataId: "test-data",
		Group:  "test-group",
	})
	fmt.Println("GetConfig,config :" + content)

	//Listen config change,key=dataId+group+namespaceId.
	// err = client.ListenConfig(vo.ConfigParam{
	// 	DataId: "test-data",
	// 	Group:  "test-group",
	// 	OnChange: func(namespace, group, dataId, data string) {
	// 		fmt.Println("config changed group:" + group + ", dataId:" + dataId + ", content:" + data)
	// 	},
	// })

	err = client.ListenConfig(vo.ConfigParam{
		DataId: "test-data-2",
		Group:  "test-group",
		OnChange: func(namespace, group, dataId, data string) {
			fmt.Println("config changed group:" + group + ", dataId:" + dataId + ", content:" + data)
		},
	})

	// _, err = client.PublishConfig(vo.ConfigParam{
	// 	DataId:  "test-data",
	// 	Group:   "test-group",
	// 	Content: "test-listen",
	// })

	// time.Sleep(2 * time.Second)

	// _, err = client.PublishConfig(vo.ConfigParam{
	// 	DataId:  "test-data-2",
	// 	Group:   "test-group",
	// 	Content: "test-listen",
	// })

	// time.Sleep(2 * time.Second)

	// //cancel config change
	// err = client.CancelListenConfig(vo.ConfigParam{
	// 	DataId: "test-data",
	// 	Group:  "test-group",
	// })

	// time.Sleep(2 * time.Second)
	_, err = client.DeleteConfig(vo.ConfigParam{
		DataId: "test-data",
		Group:  "test-group",
	})
	// time.Sleep(5 * time.Second)

	searchPage, _ := client.SearchConfig(vo.SearchConfigParam{
		Search:   "blur",
		DataId:   "admin.api",
		Group:    "",
		PageNo:   1,
		PageSize: 10,
	})
	fmt.Printf("Search config:%+v \n", searchPage)
	f, err := os.OpenFile("./abc.yaml", os.O_WRONLY|os.O_APPEND|os.O_CREATE, 0666)
	if err != nil {
		return
	}
	defer f.Close()
	for _, item := range searchPage.PageItems {
		//写入文件时，使用带缓存的 *Writer
		write := bufio.NewWriter(f)
		write.WriteString(item.Content)

		write.Flush()
	}

	signChan := make(chan struct{})
	<-signChan
}
