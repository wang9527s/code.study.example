package main

import (
	"fmt"
	"strconv"
)

func main()  {
	var a int = 669
	fmt.Println(int32(a))
	fmt.Println(float64(a))

	// string to int
	intNum, _ := strconv.Atoi("45656")
	fmt.Println(" to int ", intNum)

	// num to string
	fmt.Println(strconv.FormatUint(uint64(uint32(32)),10))
	fmt.Println(strconv.FormatInt(int64(int32(222)),10))
	fmt.Println(strconv.Itoa(int(58)))

	// string to byte
	fmt.Println([]byte("字符串转byte"))
	// byte to string
	data := []byte("我是字节数组123")
	fmt.Println(string(data))
}