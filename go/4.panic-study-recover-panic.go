// json.go
package main

import (
	"fmt"
	"runtime"
	"time"
)



// 崩溃时需要传递的上下文信息
type panicContext struct {
	function string // 所在函数
}
// 保护方式允许一个函数
func ProtectRun(entry func() string) string{
	// 延迟处理的函数
	defer func() {
		// 发生宕机时，获取panic传递的上下文并打印
		err := recover()
		switch err.(type) {
		case runtime.Error: // 运行时错误
			fmt.Println("runtime error:", err)
		default: // 非运行时错误
			fmt.Println("error:", err)
		}
	}()
	return entry()
}

/*  说明：用于定位崩溃位置可能没什么作用
	即使没有使用recover函数，段错误仍然会抛出异常
*/

func main() {
	fmt.Println("运行前")
	//允许一段手动触发的错误
	ProtectRun(func() string{
		fmt.Println("手动宕机前")
		// 使用panic传递上下文
		panic(&panicContext{
			"手动触发panic",
		})
		fmt.Println("手动宕机后")
		return "hello"
	})
	//故意造成空指针访问错误

	res := ProtectRun(func() string{
		fmt.Println("赋值宕机前(野指针?)")
		var a *int
		*a = 1
		fmt.Println("赋值宕机后")
		return "没有崩溃的情况下，可以取得返回值"
	})
	fmt.Println("运行后")
	time.Sleep(time.Second)
	fmt.Println("运行后")
	fmt.Println(res)
}