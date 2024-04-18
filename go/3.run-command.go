package main

import (
	"fmt"
	"os/exec"
)

var err error
func main()  {
	var resCmd []byte
	//Run       阻塞执行
    //Output    里面调用的是Run       不知道为什么调用Run无法执行
	//resCmd, err = exec.Command("bash", "-c", "echo '1' > /proc/sys/vm/drop_caches").Output()
	resCmd, err = exec.Command("bash", "-c", "echo '1' > ./test.tmp").Output()
	resCmd, err = exec.Command("ls", "-al").Output()
	fmt.Println(string(resCmd))

	fmt.Println(err)
}
