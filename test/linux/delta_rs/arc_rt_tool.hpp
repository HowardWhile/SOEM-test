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

/**
 * @brief set_latency_target
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
static int set_latency_target(void)
{
    struct stat s;
    int ret;

    if (stat("/dev/cpu_dma_latency", &s) != 0) {
        console("# /dev/cpu_dma_latency does not exist.");
        return -1;
    }

    int latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    if (latency_target_fd == -1) {
        console("# Failed to open /dev/cpu_dma_latency: %s", strerror(errno));
        return -1;
    }

    int32_t latency_target_value = 0;
    ret = write(latency_target_fd, &latency_target_value, sizeof(latency_target_value));
    if (ret == 0) {
        console("# error setting cpu_dma_latency to %d!: %s", latency_target_value, strerror(errno));
        close(latency_target_fd);
        return -1;
    }

    console("# /dev/cpu_dma_latency set to %dus", latency_target_value);
    close(latency_target_fd);

    return 0;
}


/**
 * @brief 將當前執行緒綁定到指定的 CPU
 *
 * @param cpu_id 目標 CPU 的 ID，指定執行緒運行的 CPU
 * @return 成功時返回 0，失敗時返回非零錯誤碼
 */
static int setThreadAffinity(int cpu_id)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);

    pthread_t thread = pthread_self();
    int ret = pthread_setaffinity_np(thread, sizeof(mask), &mask);
    if (ret != 0) {
        // 設置 CPU Affinity 失敗
        perror("pthread_setaffinity_np");
        return ret;
    }

    return ret;
}

/**
 * @brief 設置執行緒的優先權
 *
 * @param Priority 優先權值，較高的值表示較高的優先級
 * @return 成功時返回 0，失敗時返回非零錯誤碼
 */
static int setThreadPriority(int Priority)
{
    pthread_t thread = pthread_self();
    struct sched_param schedp;
    schedp.sched_priority = Priority;

    int ret = pthread_setschedparam(thread, SCHED_FIFO, &schedp);
    if (ret != 0) {
        // 設置優先權失敗
        perror("pthread_setschedparam");
        return ret;
    }

    return ret;
}

#endif // rt_tool_h__