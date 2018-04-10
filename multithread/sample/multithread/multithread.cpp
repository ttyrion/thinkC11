// multithread.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <Windows.h>
//Attention: Even for a fundamental data type, such as int or bool, 
//the standard does not guarantee that a read or a write is atomic(In other words,
//    a read or write is NOT an exclusive noninterruptable data access).

class Task {
public:
    Task(int task) {
        task_ = task;
    }

    int operator() () {
        std::cout << task_ << std::endl;
        return task_;
    }

    int task_;
};

std::vector<Task> g_tasks;
std::mutex g_mutex;

void producer_thread() {
    int count = 0;
    for (int i = 1000; i >= 0 ; --i) {
        std::lock_guard<std::mutex> guard(g_mutex);
        g_tasks.push_back(Task(i));
    }
}

void consumer_thread() {
    for (int i = 0; i < 1000; i++) {
        std::lock_guard<std::mutex> guard(g_mutex);
        if (!g_tasks.empty()) {
            if ((*g_tasks.begin())() == 0) {
                return;
            }

            g_tasks.erase(g_tasks.begin());
        }
    }   
}


#define WM_TASK WM_USER + 1
#define WM_TASK_END WM_USER + 2

void producer_message_thread(DWORD id) {
    int count = 0;
    for (int i = 1000; i >= 0; --i) {
        Task *t = new Task(i);
        PostThreadMessage(id, i == 0 ? WM_TASK_END : WM_TASK, (WPARAM)t, 0);
    }
}

void consumer_message_thread() {
    // call PeekMessage to force the system to create the message queue.
    MSG msg;
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    while (true)
    {
        if (GetMessage(&msg, 0, 0, 0)) //get msg from message queue
        {
            switch (msg.message) {
            case WM_TASK: {
                Task * t = (Task *)msg.wParam;
                (*t)();
                delete t;
                break;
            }
            case WM_TASK_END: {
                return;
            }
            }
        };
    }
}

int main() {
    //用进程间发送消息的方式，就不需要自己管理全局的任务队列，不需要自己加锁
    std::thread consumer(consumer_message_thread);
    //std::thread::id cannot be cast into DWORD
    DWORD id = ::GetThreadId(consumer.native_handle());
    std::thread producer(producer_message_thread, id);
    producer.join();
    consumer.join();

    return 0;
}

