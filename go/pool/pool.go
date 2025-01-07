package main

import (
	"context"
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

type Task struct {
	Fn   func(args ...interface{})
	Args []interface{}
}

type TaskManager struct {
	concurrency int
	mergeLen    int
	delay       int
	chanDirect  bool
	chs         []chan *Task
	chi         uint32
	cancels     []context.CancelFunc
	wg          sync.WaitGroup
}

func NewTaskManager(concurrency, bufLen, mergeLen, delay int, chanDirect bool) *TaskManager {
	if concurrency <= 0 {
		panic("concurrency must be positive")
	}

	m := &TaskManager{concurrency: concurrency, mergeLen: mergeLen,
		delay: delay, chanDirect: chanDirect}

	for i := 0; i < concurrency; i++ {
		ch := make(chan *Task, bufLen)
		m.chs = append(m.chs, ch)

		m.wg.Add(1)
		if chanDirect {
			go m.chanWait(ch)
		} else {
			ctx, cancel := context.WithCancel(context.Background())
			m.cancels = append(m.cancels, cancel)
			go m.timedWait(ch, ctx)
		}
	}

	return m
}

func (m *TaskManager) chanWait(ch chan *Task) {
	for {
		t := <-ch
		if t == nil {
			fmt.Println("Got nil and the coroutine is exiting")
			m.wg.Done()
			break
		}
		t.Fn(t.Args...)
	}
}

func (m *TaskManager) timedWait(ch chan *Task, ctx context.Context) {
	var tasks []*Task
	var count int64
	var delay, lastDelay int
	delay = m.delay
	lastDelay = delay
	timer := time.NewTimer(time.Duration(delay) * time.Second)
	defer timer.Stop()

	for {
		if delay != lastDelay {
			timer.Reset(time.Duration(delay) * time.Second)
			lastDelay = delay
		}
		select {
		case <-ctx.Done():
			fmt.Println("The coroutine is exiting, count:", count)
			m.wg.Done()
			return
		case t := <-ch:
			tasks = append(tasks, t)
			count++
			if len(tasks) < m.mergeLen {
				delay = 0
				continue
			}
		case <-timer.C:
			if len(tasks) == 0 {
				delay = m.delay
				continue
			}
		default:
			continue
		}

		for _, t := range tasks {
			t.Fn(t.Args...)
		}
		tasks = tasks[:0]
		delay = m.delay
	}
}

func (m *TaskManager) AddTask(fn func(args ...interface{}),
	args ...interface{}) *Task {
	t := &Task{Fn: fn, Args: args}
	i := atomic.AddUint32(&m.chi, 1) - 1
	m.chs[i%uint32(len(m.chs))] <- t
	return t
}

func (m *TaskManager) Stop() {
	if m.chanDirect {
		for i := 0; i < len(m.chs); i++ {
			m.chs[i] <- nil
		}
	} else {
		for _, cancel := range m.cancels {
			cancel()
		}
	}
	m.wg.Wait()
}
