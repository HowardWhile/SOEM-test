#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include <sys/syscall.h> //get PID TID

#include <termios.h> //keyboardPISOX中定义的标准接口

#include "arc_console.hpp"
#include "arc_rt_tool.hpp"

// ----------------------------------------------------------------
// ----------------------------------------------------------------
#define CPU_ID 7                    // 指定運行的CPU編號
#define PERIOD_NS (1 * 1000 * 1000) // 1ms rt任務週期
// ----------------------------------------------------------------
bool receivedCtrlC = false; // 控制台收到Crtl-C信号

// ----------------------------------------------------------------
// realtime 性能計算與顯示
// ----------------------------------------------------------------
int64_t ns_dt = 0; // nanosecond
int64_t cyc_count = 0;
int64_t max_dt = LLONG_MIN;
int64_t min_dt = LLONG_MAX;
int64_t sum_dt = 0;
void update_dt(int64_t dt_ns)
{
    ns_dt = dt_ns;
    if (ns_dt > max_dt)
        max_dt = ns_dt;

    if (ns_dt < min_dt)
        min_dt = ns_dt;

    sum_dt += ns_dt;
    cyc_count++;
}
void displayRealTimeInfo()
{
    console("cyc_count: %ld, Latency:(act, min, max, avg)us = (%-4ld, %ld, %ld, %.2f) ****",
            cyc_count,
            ns_dt / 1000,
            min_dt / 1000,
            max_dt / 1000,
            (double)sum_dt / cyc_count / 1000);
    MOVEUP(1);
}

// ----------------------------------------------------------------
// 主要的迴圈
// ----------------------------------------------------------------
void *bgRealtimeDoWork(void *arg)
{
    pid_t tid = syscall(SYS_gettid);
    console("bgRealtimeDoWork start, " LIGHT_GREEN "Thread ID: %d" RESET, tid);

    // config thread to realtime
    if (
        setLatencyTarget() == 0 &&        // 消除系統時間偏移的函數
        setThreadAffinity(CPU_ID) == 0 && // 指定cpu
        setThreadPriority(99) == 0 &&     // 指定優先級PRI 99 = RT
        setThreadNiceness(-20) == 0)      // 指定優先級NI -20
    {
        console("Starting RT task with dt=%u ns", PERIOD_NS);

        struct timespec wakeup_time,tnow;

        //  當前的精準時間
        if (clock_gettime(CLOCK_MONOTONIC, &wakeup_time) == -1) //
        {
            console("clock_gettime " RED "%s" RESET, strerror(errno));
            return NULL;
        }

        int64_t dt;

        while (true)
        {
            // sleep直到指定的時間點
            addTimespec(&wakeup_time, PERIOD_NS);
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
            int64_t dt = calcTimeDiffInNs(tnow, wakeup_time);
            update_dt(dt);

            
            EXEC_INTERVAL(30)
            {
                // 每30ms顯示一次實時統計的訊息
                displayRealTimeInfo();
            }
            EXEC_INTERVAL_END

            if (receivedCtrlC)
                break;
        }
    }
    else
    {
        console("setThreadPriority or setThreadNiceness fail");        
    }

    console("bgRealtimeDoWork thread exit");
    return NULL;
}

// ----------------------------------------------------------------
// 鍵盤控制
// ----------------------------------------------------------------
// Ctrl+C 訊號
void handleCtrlC(int sig)
{
    console("Ctrl+C handle\n");
    // 重新註冊信號處理函數，以繼續攔截 Ctrl+C 事件
    signal(SIGINT, handleCtrlC);
    receivedCtrlC = true;
}

void *bgKeyboardDoWork(void *arg)
{
    pid_t tid = syscall(SYS_gettid);
    console("bgKeyboardDoWork start, " LIGHT_GREEN "Thread ID: %d" RESET, tid);

    // 攔截ctrl+c事件
    signal(SIGINT, handleCtrlC);

    struct termios new_settings;
    struct termios stored_settings;

    tcgetattr(0, &stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON); // 屏蔽整行缓存
    new_settings.c_cc[VTIME] = 0;

    int tcgetattr(int fd, struct termios *termios_p);
    tcgetattr(0, &stored_settings);
    new_settings.c_cc[VMIN] = 1;

    int ch;
    while (ch != 'q')
    {
        tcsetattr(0, TCSANOW, &new_settings);
        ch = getchar();
        tcsetattr(0, TCSANOW, &stored_settings);

        switch (ch)
        {
        case 'r':
            max_dt = LLONG_MIN;
            min_dt = LLONG_MAX;
            sum_dt = 0;
            cyc_count = 0;
            break;
        case 'q':
            printf("\r\n");
            receivedCtrlC = 1;
            break;

        default:
            break;
        }

        if (receivedCtrlC)
            break;
    }

    return NULL;
}
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int main()
{
    console("delta_rs SOEM (Simple Open EtherCAT Master) Start...");

    // 鍵盤
    pthread_t bg_keyboard;
    if (pthread_create(&bg_keyboard, NULL, bgKeyboardDoWork, NULL) != 0)
    {
        fprintf(stderr, "Failed to create bg_rt thread\n");
        return -1;
    }
    usleep(100);

    // 開始執行背景執行
    pthread_t bg_rt;
    if (pthread_create(&bg_rt, NULL, bgRealtimeDoWork, NULL) != 0)
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