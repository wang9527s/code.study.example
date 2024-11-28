// json.go
package main

import (
	"fmt"
	"sync"
	"time"
)

type nmDeviceStatusObject struct {
	u_newState	uint32
	u_oldState	uint32
	u_reaseon	uint32
}

type DeviceStatusChangeEvent struct {
	mu       	sync.Mutex
	evMap		[]nmDeviceStatusObject
	doChan		chan bool
}

func newDeviceStatusChangeEvent() *DeviceStatusChangeEvent {
	l := &DeviceStatusChangeEvent{
		evMap:    make([]nmDeviceStatusObject,0),
		doChan: 	make(chan bool,20),
	}

	go l.loop()
	return l
}

func (pe *DeviceStatusChangeEvent) loop() {
	for {
		select {
		case <- pe.doChan:
			fmt.Println("chan")
			for  {
				if len(pe.evMap) == 0 {
					break
				}

				pe.mu.Lock()
				currentEvent := pe.evMap[0]
				pe.evMap = pe.evMap[1:]
				fmt.Println("do event size", len(pe.evMap))
				pe.mu.Unlock()
				fmt.Println("do ", currentEvent)

				time.Sleep(2*time.Second)
			}
		}
	}
}

func (pe *DeviceStatusChangeEvent)addEvent(newState,oldState,reaseon uint32)  {
	fmt.Println("")
	fmt.Println("add event",time.Now().Unix(), newState,oldState,reaseon)
	recvEvent := nmDeviceStatusObject{newState,oldState,reaseon}

	pe.mu.Lock()
	for i,ev := range pe.evMap {
		//logger.Info(i,ev)
		if ev == recvEvent {
			fmt.Println("ignore repeat")
			pe.evMap = append(pe.evMap[:i], pe.evMap[i+1:]...)
			break
		}
	}

	pe.evMap = append(pe.evMap, recvEvent)
	pe.mu.Unlock()


	fmt.Println("event count  ---------- ", len(pe.evMap))
}

func (pe *DeviceStatusChangeEvent)doEvent(){
	fmt.Println("send chan")
	//pe.mu.Lock()
	//if len(pe.evMap) == 1 {
		pe.doChan <- true
	//}
	//pe.mu.Unlock()
}


func main() {
	fmt.Println("event:","hello")
	ec := newDeviceStatusChangeEvent()
	ec.addEvent(1,20, 0)
	ec.doEvent()
	ec.addEvent(1,21, 0)
	ec.doEvent()
	ec.addEvent(1,22, 0)
	ec.doEvent()
	ec.addEvent(1,23, 0)
	ec.doEvent()
	ec.addEvent(1,23, 0)
	ec.doEvent()
	ec.addEvent(1,22, 0)
	ec.doEvent()

	time.Sleep(10*time.Second)

	ec.addEvent(2,20, 0)
	ec.doEvent()
	ec.addEvent(2,30, 0)
	ec.doEvent()


	time.Sleep(100*time.Second)
}
