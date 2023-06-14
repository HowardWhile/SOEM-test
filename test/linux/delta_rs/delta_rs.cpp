#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include "arc_console.hpp"
#include "arc_rt_tool.hpp"


// 全域變數表示是否收到訊號
bool receivedCtrlC = false;
// 自定義的信號處理函數
void handleCtrlC(int sig)
{
    console("Ctrl+C handle\n");
    // 重新註冊信號處理函數，以繼續攔截 Ctrl+C 事件
    signal(SIGINT, handleCtrlC);
    receivedCtrlC = true;
}

void *threadFunc(void *arg)
{
    int tid = *(int *)arg;
    printf("Thread %d: Hello, World!\n", tid);
    while (!receivedCtrlC)
    {
        /* code */
    }
    
    return NULL;
}


int main()
{
    console("delta_rs SOEM (Simple Open EtherCAT Master) Start...");

    // 消除系統時間偏移的函數
    if (set_latency_target())
    {
        console("[error]" RED "%s" RESET, strerror(errno));
        return -1;
    }

    // 攔截ctrl+c事件
    signal(SIGINT, handleCtrlC);

    pthread_t thread;
    int tid = 123;
    int ret = pthread_create(&thread, NULL, threadFunc, &tid);
    if (ret != 0) {
        fprintf(stderr, "Failed to create thread\n");
        return 1;
    }

    // 等待執行緒結束
    ret = pthread_join(thread, NULL);
    if (ret != 0) {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }


    return 0;
}