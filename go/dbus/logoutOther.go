package main

import (
	"fmt"
	"github.com/godbus/dbus"
)

func main()  {
	//调用dbus接口
	var resDBus interface{}
	conn, err := dbus.SystemBus()
	
	curNameValue, err := conn.Object("org.freedesktop.login1", "/org/freedesktop/login1/user/self").GetProperty("org.freedesktop.login1.User.Name")
	fmt.Println(err)
	curName := curNameValue.Value().(string)

	logindbus := conn.Object("org.freedesktop.login1", "/org/freedesktop/login1")
	err = logindbus.Call("org.freedesktop.login1.Manager.ListUsers",0).Store(&resDBus)
	fmt.Println(resDBus)

	for _,v := range resDBus.([][]interface{}) {
		uid := v[0] 
		uname := v[1].(string) 

		if uname == curName || uname == "lightdm" {
			continue
		}
		fmt.Println("log out ",uname, uid )
	}
}
