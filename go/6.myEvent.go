// json.go
package main

import (
	"fmt"
	"sync"
	"time"
)

type plugEvent struct {
	mu       sync.Mutex
	funcs    map[string]func(string2 string)
	interval time.Duration
	timer    *time.Timer
	quit     chan struct{}
}

func newPlugEvent() *plugEvent {
	interval := 1 * time.Second
	timer := time.NewTimer(interval)
	timer.Stop()

	l := &plugEvent{
		timer:    timer,
		interval: interval,
	}
	l.funcs = make(map[string]func(string2 string))
	l.quit = make(chan struct{})
	go l.loopCheck()
	return l
}

func (pe *plugEvent) loopCheck() {
	for {
		select {
		case <-pe.timer.C:
			pe.mu.Lock()
			funcs := pe.funcs
			pe.funcs = make(map[string]func(string2 string))
			pe.mu.Unlock()

			for k,v := range funcs {
				v(k)
			}

		case <-pe.quit:
			return
		}
	}
}

func (pe *plugEvent) addEvent(k string,v func(string)()) {
	pe.mu.Lock()
	pe.funcs[k] = v
	pe.timer.Reset(pe.interval)
	pe.mu.Unlock()
}

func main() {
	event := newPlugEvent()

	for _,str:=range []string{"1","2","3"} {
		fmt.Println("add", str)
		event.addEvent(str,func(string2 string) {fmt.Println("event:",string2)})
		time.Sleep(200*time.Millisecond)
	}

	time.Sleep(2*time.Second)
	fmt.Println("add", "333333333")
	event.addEvent("333333333",func(string2 string) {fmt.Println("event:",string2)})
	time.Sleep(10*time.Second)
}