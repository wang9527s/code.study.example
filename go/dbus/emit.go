package main

import (
	"fmt"
	"github.com/godbus/dbus"
	"time"
)

func main()  {
	//调用dbus接口
	sessionBus, err := dbus.SessionBus()
	for {
		err = sessionBus.Emit("/wangbin", "com.wangbin.daemon.interface.sigProgress", 23,"arg1")
		fmt.Println("send signal",err)
		time.Sleep(2*time.Second)
	}
}
