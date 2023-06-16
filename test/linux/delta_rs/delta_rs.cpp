#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include "arc_console.hpp"
#include "arc_rt_tool.hpp"

// ----------------------------------------------------------------
// ----------------------------------------------------------------
#define CPU_ID 7 // 指定運行的CPU編號

#define PERIOD_NS (1 * 1000 * 1000) // 1ms rt任務週期
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// realtime 時間測試
int64_t max_dt = LLONG_MIN;
int64_t min_dt = LLONG_MAX;
int64_t sum_dt = 0;
int64_t cyc_count = 0;
void update_dt(int64_t dt)
{
    if (dt > max_dt)
        max_dt = dt;

    if (dt < min_dt)
        min_dt = dt;

    sum_dt += dt;
    cyc_count++;
}

// ----------------------------------------------------------------
// Ctrl+C 訊號
// ----------------------------------------------------------------
bool receivedCtrlC = false;
void handleCtrlC(int sig)
{
    console("Ctrl+C handle\n");
    // 重新註冊信號處理函數，以繼續攔截 Ctrl+C 事件
    signal(SIGINT, handleCtrlC);
    receivedCtrlC = true;
}

// ----------------------------------------------------------------
// 主要的迴圈
// ----------------------------------------------------------------
void *bg_rt_dowork(void *arg)
{
    console("bg_rt_dowork thread start");

    // 消除系統時間偏移的函數
    if (set_latency_target())
    {
        console(RED "%s" RESET, strerror(errno));
    }

    // 指定cpu
    setThreadAffinity(CPU_ID);

    // config thread to realtime 
    if (setThreadPriority(99) != 0 || setThreadNiceness(-20) != 0)
    {
        console("setThreadPriority or setThreadNiceness fail");
    }
    else
    {
        console("Starting RT task with dt=%u ns", PERIOD_NS);
        
        // cycletime in ns
        const int64_t cycletime = PERIOD_NS; 

        //  當前的精準時間
        struct timespec wakeup_time;
        if (clock_gettime(CLOCK_MONOTONIC, &wakeup_time) == -1) //f
        {
            console("clock_gettime " RED "%s" RESET, strerror(errno));
            return NULL;
        }

        int64_t dt;
        struct timespec tnow;

        while (true)
        {
            // sleep直到指定的時間點
            addTimespec(&wakeup_time, cycletime);
            int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
            if (ret)
            {
                // sleep錯誤處理
                printf("clock_nanosleep(): %s\n", strerror(ret));
                // break;
            }

            // 取得當前的精準時間
            clock_gettime(CLOCK_MONOTONIC, &tnow);

            // 計算時間差距
            int64_t dt = calcDiff_ns(tnow, wakeup_time);
            update_dt(dt);

            // 顯示
            EXEC_INTERVAL(30)
            {
                consoler("cyc_count: %ld, Latency:(act, min, max, avg)us = (%-8ld, %ld, %ld, %.2f) ****",
                         cyc_count,
                         dt / 1000,
                         min_dt / 1000,
                         max_dt / 1000,
                         (double)sum_dt / cyc_count / 1000);
            }
            EXEC_INTERVAL_END

            if (receivedCtrlC)
                break;
        }
    }

    console("bg_rt_dowork thread exit");
    return NULL;
}

int main()
{
    console("delta_rs SOEM (Simple Open EtherCAT Master) Start...");

    // 攔截ctrl+c事件
    signal(SIGINT, handleCtrlC);

    // 開始執行背景執行
    pthread_t bg_rt;
    if (pthread_create(&bg_rt, NULL, bg_rt_dowork, NULL) != 0)
    {
        fprintf(stderr, "Failed to create bg_rt thread\n");
        return -1;
    }

    // 等待執行緒結束
    if (pthread_join(bg_rt, NULL) != 0)
    {
        fprintf(stderr, "Failed to join bg_rt thread\n");
        return -1;
    }

    return 0;
}