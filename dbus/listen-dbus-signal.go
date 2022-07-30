package main

import (
	"github.com/godbus/dbus"
	"github.com/linuxdeepin/go-lib/log"
	"time"
)

var logger = log.NewLogger("")

func listenDbusSignal(){
	/* golang dbus 教程
	https://www.codeplayer.org/Wiki/go/%E5%9C%A8go%E4%B8%AD%E4%BD%BF%E7%94%A8dbus%E5%92%8Cgsettings.html
	*/

	// 监听信号
	sessionBus, err := dbus.SessionBus()
	err = sessionBus.BusObject().AddMatchSignal("org.freedesktop.DBus", "NameOwnerChanged",
		dbus.WithMatchObjectPath("/org/freedesktop/DBus")).Err

	signalCh := make(chan  *dbus.Signal, 10)
	sessionBus.Signal(signalCh)
	go func() {
		for {
			select {
			case sig := <-signalCh:

				if sig.Path == "/org/freedesktop/DBus" &&
					sig.Name == "org.freedesktop.DBus.NameOwnerChanged" {
					logger.Info("sig: ", sig)
				}
			}
		}
	}()

	time.Sleep(1000*time.Second)
	err = sessionBus.BusObject().RemoveMatchSignal("org.freedesktop.DBus", "NameOwnerChanged",
		dbus.WithMatchObjectPath("/org/freedesktop/Bus")).Err
	sessionBus.RemoveSignal(signalCh)

	logger.Info(err)
}

func main()  {
	go listenDbusSignal()

	time.Sleep(time.Second * 1000)
}