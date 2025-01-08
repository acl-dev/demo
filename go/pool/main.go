package main

import (
	"flag"
	"fmt"
	"runtime"
	"sync"
	"sync/atomic"
	"time"
)

func main() {
	mergeLen := flag.Int("m", 1, "merge length")
	bufLen := flag.Int("b", 10000, "buffer length")
	delay := flag.Int("d", 10, "waiting delay seconds")
	count := flag.Int64("n", 100, "count of tasks")
	consumers := flag.Int("c", 10, "number of consumers")
	producers := flag.Int("p", 1, "number of producers")
	numCPU := flag.Int("k", runtime.NumCPU(), "number of CPU")
	chanDirect := flag.Bool("D", false, "Read from channel directly, or timed read from channel by select")

	flag.Parse()

	runtime.GOMAXPROCS(*numCPU)

	var result int64
	manager := NewTaskManager(*consumers, *bufLen, *mergeLen, *delay, *chanDirect)

	if *producers <= 0 {
		*producers = 1
	}

	n := *count / int64(*producers)
	*count = n * int64(*producers)

	fmt.Printf("count: %d, n: %d, procuders: %d\r\n", *count, n, *producers)

	var wg sync.WaitGroup

	begin := time.Now()

	task := func() {
		handleTask(&result, &wg)
	}

	for j := 0; j < *producers; j++ {
		wg.Add(1)
		go func() {
			for i := int64(0); i < n; i++ {
				wg.Add(1)
				manager.AddTask(task)
			}

			fmt.Printf("Send tasks over, count: %d\r\n", n)
			wg.Done()
		}()
	}

	wg.Wait()

	end := time.Now()
	tc := end.Sub(begin).Milliseconds()
	speed := (float64(*count) * 1000.00) / float64(tc)
	fmt.Println("Time cost:", tc, "ms, speed:", speed)

	manager.Stop()
}

func handleTask(result *int64, wg *sync.WaitGroup) {
	atomic.AddInt64(result, 1)
	wg.Done()
}
