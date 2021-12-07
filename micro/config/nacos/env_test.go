package main

import (
	"os"
	"testing"
)

// echo 'export NACOS_NAMESPACEID="74ab6cb4-2503-4c82-b966-75043f8ea730"' >> ~/.profile
// echo 'export NACOS_USERNAME="dev"' >> ~/.profile
// echo 'export NACOS_PASSWORD="nacos"' >> ~/.profile
func TestEnv(t *testing.T) {
	t.Log("namespace:", os.Getenv("NACOS_NAMESPACEID"))
	t.Log("username:", os.Getenv("NACOS_USERNAME"))
	t.Log("password:", os.Getenv("NACOS_PASSWORD"))
}
