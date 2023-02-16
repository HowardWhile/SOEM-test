#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>

#include <sys/time.h>

// set priorty
#include <unistd.h>
#include <sys/resource.h>

// keyboard
#include <curses.h>
#include <ctype.h>

#include "ethercat.h"
#include "arc_console.hpp"

// RT Loop的週期
#define PERIOD_NS (1000000) // 1k hz
//#define PERIOD_NS (100000) // 10k hz
#define NSEC_PER_SEC (1000000000)

boolean bg_cancel = 0;
OSAL_THREAD_HANDLE thread1;
OSAL_THREAD_HANDLE bg_keyboard;

int64 last_cktime = 0;
int64 max_dt = LLONG_MIN;
int64 min_dt = LLONG_MAX;
int64 sum_dt = 0;
int64 cyc_count = 0;

void cyclic_task()
{  
    //int64 dc_time = ec_DCtime;
    int64 dc_time = clock_ns();
    int64 dt = dc_time - last_cktime;
    cyc_count++;
    sum_dt += dt;

    if (dt < min_dt)
        min_dt = dt;

    if (dt > max_dt)
        max_dt = dt;

    last_cktime = dc_time;

    // 顯示
    EXEC_INTERVAL(100)
    {
        console("cyc_count: %ld, (min, max, avg)us = (%ld, %ld, %.2f) T:%ldns ****",
                 cyc_count,
                 min_dt / 1000, max_dt / 1000, (double)sum_dt / cyc_count / 1000,
                 dc_time);
    }
    EXEC_INTERVAL_END
}

OSAL_THREAD_FUNC keyboard(void *ptr)
{
    (void)ptr; /* Not used */
    int ch;
    while (ch != 'q')
    {
        ch = getch();
        switch (ch)
        {
        case ' ':
            break;
        case 'r':
            max_dt = LLONG_MIN;
            min_dt = LLONG_MAX;
            sum_dt = 0;
            cyc_count = 0;
            break;

        default:
            break;
        }

        // printf("[keyboard] press (%c) (%d)\r\r\n", ch, ch);
    }
    bg_cancel = 1;
    return;
}

struct timespec timespec_add(struct timespec time1, struct timespec time2)
{
    struct timespec result;

    if ((time1.tv_nsec + time2.tv_nsec) >= NSEC_PER_SEC)
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec + 1;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec - NSEC_PER_SEC;
    }
    else
    {
        result.tv_sec = time1.tv_sec + time2.tv_sec;
        result.tv_nsec = time1.tv_nsec + time2.tv_nsec;
    }

    return result;
}

void signal_handler(int signum)
{
    // 用來攔截 Ctrl + C 訊號
    printf("signal_handler: caught signal %d\r\n", signum);
    if (signum == SIGINT)
    {
        printf("[ctrl + c] ethercat release\r\n");
        bg_cancel = 1;
    }
}

// int main(int argc, char *argv[])
int main(void)
{
    initscr();
    noecho();
    osal_thread_create(&bg_keyboard, 1024, &keyboard, (void *)&ctime);
    // 攔截 ctrl + C 事件
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        printf("Failed to caught signal\r\n");

    // ----------------------------------------------------
    // 設定程式的兩種優先權
    // ----------------------------------------------------
    // Set PR (Priority) to RT(-99) 最高優先權
    printf("Set priority ...\r\n");
    struct sched_param param = {};
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    printf("Using priority %i.", param.sched_priority);
    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
        printf("sched_setscheduler failed\r\n"); // 錯誤還是可以跑

    // Set NI (Niceness) to -19 最高優先權
    pid_t pid = getpid(); // 獲得進程PID
    printf("PID = %d\r\n", pid);
    if (setpriority(PRIO_PROCESS, pid, -19))                                // 設置進程優先順序
        printf("Warning: Failed to set priority: %s\r\n", strerror(errno)); // 錯誤還是可以跑

    struct timespec wakeup_time;
    
    printf("Starting RT task with dt=%u ns.\r\n", PERIOD_NS);
    clock_gettime(CLOCK_MONOTONIC, &wakeup_time);
    wakeup_time.tv_nsec += PERIOD_NS;

    last_cktime = clock_ns();
    while (!bg_cancel)
    {
        //console_fps("loop");
        const struct timespec cycletime = {0, PERIOD_NS};
        wakeup_time = timespec_add(wakeup_time, cycletime);
        int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
        if (ret)
        {
            printf("clock_nanosleep(): %s\n", strerror(ret));
            break;
        }

        //static int64 rt_check_time = 0;
        //if (rt_check_time != clock_ms())
        {
            //rt_check_time = clock_ms();
            // --------------------------------------------
            cyclic_task();
            // --------------------------------------------
        }
    }

    printf("End program\r\n");
    endwin();

    return (0);
}
