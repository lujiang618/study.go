package main

import (
	"bufio"
	"os"

	"github.com/apolloconfig/agollo/v4/utils/parse/yaml"
)

type Parser struct {
	yaml.Parser
}

// Parse 内存内容=>yml文件转换器
func (d *Parser) Parse(configContent interface{}) (map[string]interface{}, error) {
	// fmt.Println(configContent)

	f, err := os.OpenFile("./abc.yaml", os.O_WRONLY|os.O_APPEND|os.O_CREATE, 0666)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	//写入文件时，使用带缓存的 *Writer
	write := bufio.NewWriter(f)
	content, ok := configContent.(string)
	if !ok {
		return nil, nil
	}
	write.WriteString(content)

	write.Flush()

	return d.Parser.Parse(configContent)
}
