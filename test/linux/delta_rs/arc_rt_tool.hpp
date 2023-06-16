#ifndef arc_rt_tool_h__
#define arc_rt_tool_h__

#include "arc_console.hpp"

// set cpu latency
#include <sys/stat.h>
#include <fcntl.h>

// set process priorty
#include <unistd.h>
#include <sys/resource.h>

// thread assign to cpu
#include <sched.h>
#include <stdio.h>

#include <sys/syscall.h>

/**
 * @brief setLatencyTarget
 *
 * 消除系統時間偏移的函數
 * 取自cyclic_test
 * https://github.com/jlelli/rt-tests/blob/master/src/cyclictest/cyclictest.c
 *
 * 如果檔案 /dev/cpu_dma_latency 存在，
 * 開啟它並將零寫入其中。這將告訴電源管理系統不要轉換到高 cstate
 * （事實上，系統的行為就像是 idle=poll）。
 * 當關閉對 /dev/cpu_dma_latency 的文件描述符時，
 * 行為將回到系統的預設值。
 *
 * @return 成功設定延遲目標時返回 0，否則返回 -1
 */
static int setLatencyTarget(void)
{
    struct stat s;
    int ret;

    if (stat("/dev/cpu_dma_latency", &s) != 0)
    {
        console("# /dev/cpu_dma_latency does not exist.");
        return -1;
    }

    int latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    if (latency_target_fd == -1)
    {
        console("# Failed to open /dev/cpu_dma_latency: %s", strerror(errno));
        return -1;
    }

    int32_t latency_target_value = 0;
    ret = write(latency_target_fd, &latency_target_value, sizeof(latency_target_value));
    if (ret == 0)
    {
        console("# error setting cpu_dma_latency to %d!: %s", latency_target_value, strerror(errno));
        close(latency_target_fd);
        return -1;
    }

    console("# /dev/cpu_dma_latency set to %dus", latency_target_value);

    return 0;
}

/**
 * @brief 取得目前執行序的 Thread ID
 *
 * @return int 目前執行序的 Thread ID
 */
static inline int getThreadID()
{
    return syscall(SYS_gettid);
}

/**
 * @brief 取得目前進程的 Process ID
 *
 * @return int 目前進程的 Process ID
 */
static inline int getProcessID()
{
    return getpid();
}

/**
 * @brief 將當前執行緒綁定到指定的 CPU
 *
 * @param cpu_id 目標 CPU 的 ID，指定執行緒運行的 CPU，4核心對應ID 0~3
 * @return 成功時返回 0，失敗時返回非零錯誤碼
 */
static int setThreadAffinity(int cpu_id)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);

    pthread_t thread = pthread_self();
    int ret = pthread_setaffinity_np(thread, sizeof(mask), &mask);
    if (ret != 0)
    {
        // 設置 CPU Affinity 失敗
        console(RED "setThreadAffinity(%d) cpu_id = %d %s" RESET, ret, cpu_id, strerror(errno));
    }

    return ret;
}

/**
 * @brief 設置執行緒的優先權
 *
 * @param Priority 優先權值，較高的值表示較高的優先級 99 = RT
 * @return 成功時返回 0，失敗時返回非零錯誤碼
 */
static int setThreadPriority(int priority)
{
    pthread_t thread = pthread_self();
    struct sched_param schedp;
    schedp.sched_priority = priority;

    int ret = pthread_setschedparam(thread, SCHED_FIFO, &schedp);
    if (ret != 0)
    {
        // 設置優先權失敗
        console(RED "setThreadPriority = %d %s" RESET, priority, strerror(errno));
        return ret;
    }

    return ret;
}

/**
 * @brief 設置進程的優先級（Niceness）
 *
 * @param[in] niceness 優先級值，較小的值表示較高的優先級，範圍是介於 -20 到 +19 之間
 * @return 成功時返回 0，失敗時返回非零錯誤碼
 */
int setThreadNiceness(int niceness)
{
    int ret = 0;
    ret = setpriority(PRIO_PROCESS, getThreadID(), niceness);
    if (ret != 0)
    {
        console(RED "setThreadNiceness = (%d) %s" RESET, niceness, strerror(errno));
    }

    return ret;
}

/**
 * @brief 計算兩個 timespec 結構體之間的時間差值（以納秒為單位）
 *
 * @param t1 第一個 timespec 結構體
 * @param t2 第二個 timespec 結構體
 * @return int64_t 兩個 timespec 結構體之間的時間差值（以納秒為單位）
 */
static int64_t calcTimeDiffInNs(struct timespec t1, struct timespec t2)
{
    int64_t tdiff;
    tdiff = NSEC_PER_SEC * (int64_t)((int)t1.tv_sec - (int)t2.tv_sec); // 計算秒數差值（乘以每秒的納秒數）
    tdiff += ((int)t1.tv_nsec - (int)t2.tv_nsec);                      // 加上納秒數差值
    return tdiff;
}

/**
 * @brief 將指定的時間（以納秒為單位）添加到 timespec 結構體中
 *
 * @param ts 指向 timespec 結構體的指針
 * @param addtime 要添加的時間（以納秒為單位）
 */
static void addTimespec(struct timespec *ts, int64_t addtime)
{
    int64_t sec, nsec;

    nsec = addtime % NSEC_PER_SEC;         // 取得納秒數的差值
    sec = (addtime - nsec) / NSEC_PER_SEC; // 取得秒數的差值
    ts->tv_sec += sec;                     // 添加秒數差值到 timespec 結構體的 tv_sec 成員
    ts->tv_nsec += nsec;                   // 添加納秒數差值到 timespec 結構體的 tv_nsec 成員
    if (ts->tv_nsec > NSEC_PER_SEC)
    {
        nsec = ts->tv_nsec % NSEC_PER_SEC;                 // 取得多餘的納秒數
        ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC; // 將多餘的納秒數轉換為秒數並添加到 tv_sec
        ts->tv_nsec = nsec;                                // 將剩餘的納秒數存回 tv_nsec
    }
}


#endif // rt_tool_h__