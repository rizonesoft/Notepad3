package main

import (
	"fmt"
	"math/rand"
)

func main() {
	channels := make([]chan bool, 6) //创建一个类型为chan bool的切片，每一项是能发送bool值的通道
	for i := range channels {        //通过range初始化切片
		channels[i] = make(chan bool)
	}

	go func() { //在其他goroutine中执行匿名函数
		for {
			channels[rand.Intn(6)] <- true //read.Intn(n, int)的用途是产生一个不大于n的随机数
		} // 发送数据到随机出现的通道
	}()

	for i := 0; i < 36; i++ {
		var x int
		select { // select语句当监听到哪个分支的通道未阻塞时就跳转到哪个分支
		case <-channels[0]:
			x = 1
		case <-channels[1]:
			x = 2
		case <-channels[2]:
			x = 3
		case <-channels[3]:
			x = 4
		case <-channels[4]:
			x = 5
		case <-channels[5]:
			x = 6
		}
		fmt.Printf("%d ", x)
	}
	fmt.Println()
}
