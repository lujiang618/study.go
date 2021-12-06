package main

type ConfigFile struct{}

func (t *ConfigFile) Load(fileName string, unmarshal func([]byte) (interface{}, error)) (interface{}, error) {
	vp.SetConfigFile(fileName)
	vp.ReadInConfig()

	return nil, nil
}

func (t *ConfigFile) Write(content interface{}, configPath string) error {
	return nil
}
