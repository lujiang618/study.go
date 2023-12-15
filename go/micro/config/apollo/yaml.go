package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"sync"

	"github.com/apolloconfig/agollo/v4/env/config"
)

//Suffix 默认文件保存类型
const Suffix = ".yaml"

var (
	yamlFileConfig = &ConfigFile{}
	//configFileMap 存取namespace文件地址
	configFileMap = make(map[string]string, 1)
	//configFileMapLock configFileMap的读取锁
	configFileMapLock sync.Mutex
)

type FileHandler struct{}

func (fileHandler *FileHandler) WriteConfigFile(config *config.ApolloConfig, configPath string) error {
	return yamlFileConfig.Write(config, fileHandler.GetConfigFile(configPath, config.AppID, config.NamespaceName))
}

func (fileHandler *FileHandler) GetConfigFile(configDir string, appID string, namespace string) string {
	key := fmt.Sprintf("%s-%s", appID, namespace)
	configFileMapLock.Lock()
	defer configFileMapLock.Unlock()
	fullPath := configFileMap[key]
	if fullPath == "" {
		filePath := fmt.Sprintf("%s%s", key, Suffix)
		if configDir != "" {
			configFileMap[namespace] = fmt.Sprintf("%s/%s", configDir, filePath)
		} else {
			configFileMap[namespace] = filePath
		}
	}
	return configFileMap[namespace]
}

func (fileHandler *FileHandler) LoadConfigFile(configDir string, appID string, namespace string) (*config.ApolloConfig, error) {
	configFilePath := fileHandler.GetConfigFile(configDir, appID, namespace)
	fmt.Println("load config file from :", configFilePath)
	c, e := yamlFileConfig.Load(configFilePath, func(b []byte) (interface{}, error) {
		config := &config.ApolloConfig{}
		e := json.NewDecoder(bytes.NewBuffer(b)).Decode(config)
		return config, e
	})

	if c == nil || e != nil {
		fmt.Errorf("loadConfigFile fail,error:", e)
		return nil, e
	}

	return c.(*config.ApolloConfig), e
}
