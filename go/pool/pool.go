package main

import (
	"context"
	"flag"
	"fmt"
	"runtime"
	"sync"
	"sync/atomic"
	"time"
)

type Task struct {
	Fn   func(end bool, args ...interface{})
	Args []interface{}
}

type TaskManager struct {
	concurrency int
	mergeLen    int
	delay       int
	chs         []chan *Task
	chi         uint32
	cancels     []context.CancelFunc
	wg          sync.WaitGroup
}

func NewTaskManager(concurrency, mergeLen, delay int) *TaskManager {
	if concurrency <= 0 {
		panic("concurrency must be positive")
	}

	m := &TaskManager{concurrency: concurrency, mergeLen: mergeLen,
		delay: delay}

	for i := 0; i < concurrency; i++ {
		ctx, cancel := context.WithCancel(context.Background())
		m.cancels = append(m.cancels, cancel)

		ch := make(chan *Task, 100000)
		m.chs = append(m.chs, ch)

		m.wg.Add(1)
		go m.waitAndHandle(ch, ctx)
	}

	return m
}

func (m *TaskManager) waitAndHandle(ch chan *Task, ctx context.Context) {
	timeout := time.NewTimer(time.Duration(m.delay) * time.Second)
	defer timeout.Stop()
	var tasks []*Task
	var count int64

	for {
		select {
		case <-ctx.Done():
			fmt.Println("The coroutine is exiting, count:", count)
			m.wg.Done()
			return
		case t := <-ch:
			tasks = append(tasks, t)
			count++
			if len(tasks) < m.mergeLen {
				continue
			}
		case <-timeout.C:
			if len(tasks) == 0 {
				continue
			}
		default:
			continue
		}

		for _, t := range tasks {
			t.Fn(false, t.Args...)
		}
		for _, t := range tasks {
			t.Fn(true, nil)
		}
		tasks = tasks[:0]
	}
}

func (m *TaskManager) AddTask(fn func(end bool, args ...interface{}),
	args ...interface{}) *Task {
	t := &Task{Fn: fn, Args: args}
	i := atomic.AddUint32(&m.chi, 1) - 1
	m.chs[i%uint32(len(m.chs))] <- t
	return t
}

func (m *TaskManager) Stop() {
	for _, cancel := range m.cancels {
		cancel()
	}
	m.wg.Wait()
}

///////////////////////////////////////////////////////////////////////////////

func main() {
	concurrency := flag.Int("c", 10, "concurrency number")
	mergeLen := flag.Int("m", 1, "merge length")
	delay := flag.Int("d", 10, "delay seconds")
	count := flag.Int64("n", 100, "count of tasks")
	producer := flag.Int("p", 1, "number of producers")
	numCPU := flag.Int("k", runtime.NumCPU(), "number of CPU")
	flag.Parse()

	runtime.GOMAXPROCS(*numCPU)

	var result int64
	begin := time.Now()
	manager := NewTaskManager(*concurrency, *mergeLen, *delay)

	if *producer <= 0 {
		*producer = 1
	}

	n := *count / int64(*producer)
	*count = n * int64(*producer)

	fmt.Printf("count: %d, n: %d, procuder: %d\r\n", *count, n, *producer)

	for j := 0; j < *producer; j++ {
		go func() {
			for i := int64(0); i < n; i++ {
				manager.AddTask(handleTask, &result)
			}

			fmt.Printf("Send tasks over, count: %d\r\n", n)
		}()
	}

loop:
	for {
		time.Sleep(time.Duration(10) * time.Millisecond)
		n := atomic.LoadInt64(&result)
		if n >= *count {
			fmt.Printf("over, count=%d, result=%d\n", *count, n)
			manager.Stop()
			break loop
		}
	}

	end := time.Now()
	tc := end.Sub(begin).Milliseconds()
	speed := (float64(*count) * 1000.00) / float64(tc)
	fmt.Println("Time cost:", tc, "ms, speed:", speed)

	manager.Stop()
}

func handleTask(end bool, args ...interface{}) {
	if end {
		return
	}

	if len(args) == 0 {
		panic("args empty")
	}

	result := args[0].(*int64)
	atomic.AddInt64(result, 1)
}
