#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include <termios.h> //keyboardPISOX中定义的标准接口

#include "ethercat.h" // SOEM

#include "arc_console.hpp"
#include "arc_rt_tool.hpp"

// ----------------------------------------------------------------
#define EC_CH_NAME "eno1"           // 通訊用的eth設備名稱
#define CPU_ID 3                    // 指定運行的CPU編號
#define PERIOD_NS (1 * 1000 * 1000) // 1ms rt任務週期
// ----------------------------------------------------------------
bool execExit = false; // 開始解建構程式
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
// ethercat
// ----------------------------------------------------------------
char IOmap[4096];

int checkSlaveConfig(void)
{
    // 檢查 EtherCAT 主站配置是否正確
    if (ec_config_init(FALSE) <= 0)
    {
        console("Failed to initialize EtherCAT configuration");
        return -1;
    }

    // 檢查 EtherCAT 從站的數量
    if (ec_slavecount <= 0)
    {
        console("No EtherCAT slaves found");
        return -1;
    }
    else
    {
        console("Found %d EtherCAT slaves:", ec_slavecount);
        for (int i = 1; i <= ec_slavecount; i++)
        {
            printf("Slave %d:\n", i);
            printf("  Vendor ID: 0x%04X\n", ec_slave[i].eep_id);
            printf("  Product Code: 0x%04X\n", ec_slave[i].eep_man);
            // 添加其他從站信息
        }
    }

    // 將本地記憶體與slave記憶體建立映射
    ec_config_map(&IOmap);

    // 配置 EtherCAT 主站的時基
    // ec_configdc();

    // 啟動 EtherCAT 主站
    if (ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE) != EC_STATE_SAFE_OP)
    {
        console("Slaves mapped, state to SAFE_OP... " RED "Failed." RESET);
        return -1;
    }
    console("Slaves mapped, state to SAFE_OP... " LIGHT_GREEN "succeeded" RESET);

    // Read slave states and list slave information
    ec_readstate();
    console("---- Slave Information ----");
    for (int i = 1; i <= ec_slavecount; i++)
    {
        printf("Slave %d:\n", i);
        printf("  Name: %s\n", ec_slave[i].name);
        printf("  Output Size: %3d bits\n", ec_slave[i].Obits);
        printf("  Input Size: %3d bits\n", ec_slave[i].Ibits);
        printf("  State: %2d\n", ec_slave[i].state);
        printf("  Delay: %d.%d\n", (int)ec_slave[i].pdelay, ec_slave[i].hasdc);
        // Add other slave information as needed
    }

    return 0;
}

int checkOperational(void)
{
    // 切換到OP模式
    console("Request operational state for all slaves");
    ec_slave[0].state = EC_STATE_OPERATIONAL; // 配置
    ec_writestate(0);

    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET); // 執行收發一次

    // wait for all slaves to reach OP state
    const int64_t k_timeout = 3000; // 3s;
    int64_t ck_time = clock_ms();
    while (!execExit)
    {
        if (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTRET) == EC_STATE_OPERATIONAL)
        {
            console(LIGHT_GREEN "Operational state reached for all slaves." RESET);
            break;
        }

        int64_t dt = clock_ms() - ck_time;
        if (dt > k_timeout)
        {
            console(RED "Timeout" RESET);
            return -1;
        }
        else
        {
            console("wait for all slaves to reach OP state (%.1fs)...", (float32)(k_timeout - dt) / 1000);
        }
    }
    return 0;
}

// ----------------------------------------------------------------
// 鍵盤控制
// ----------------------------------------------------------------
// Ctrl+C 訊號
void handleCtrlC(int sig)
{
    console("Ctrl+C handle");
    // 重新註冊信號處理函數，以繼續攔截 Ctrl+C 事件
    signal(SIGINT, handleCtrlC);
    execExit = true;
}

void setupTerminal()
{
    termios new_settings;
    tcgetattr(STDIN_FILENO, &new_settings);          // 儲存目前的終端設定
    new_settings.c_lflag &= (~ICANON | ECHO);        // 將 ICANON 標誌位設為 0，以屏蔽整行緩衝
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings); // 設定終端屬性
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);        // 將終端檔案描述符設置為非阻塞模式
}

void *bgKeyboardDoWork(void *arg)
{
    console("bgKeyboardDoWork start, " LIGHT_GREEN "Thread ID: %d" RESET, getThreadID());

    // 攔截ctrl+c事件
    signal(SIGINT, handleCtrlC);

    // 儲存目前的終端設定
    termios stored_settings;
    tcgetattr(0, &stored_settings);
    setupTerminal();
    int ch;
    while (ch != 'q')
    {
        if (execExit)
            break;

        ch = getchar();
        if (ch == EOF)
        {
            // 若 getchar() 返回 EOF，表示沒有輸入可用
            usleep(1000); // 等待 1 毫秒
            continue;
        }

        switch (ch)
        {
        case 'r':
            max_dt = LLONG_MIN;
            min_dt = LLONG_MAX;
            sum_dt = 0;
            cyc_count = 0;
            break;
        case 'q':
            MOVEDOWN(100); // 游標移到最後
            execExit = 1;
            break;

        default:
            break;
        }
    }

    tcsetattr(0, TCSANOW, &stored_settings); // 還原設定
    console("bgKeyboardDoWork thread exit");
    return NULL;
}

// ----------------------------------------------------------------
// 主要的迴圈
// ----------------------------------------------------------------
void *bgRealtimeDoWork(void *arg)
{
    console("bgRealtimeDoWork start, " LIGHT_GREEN "Thread ID: %d" RESET, getThreadID());

    // config thread to realtime
    if (
        setLatencyTarget() == 0 &&        // 消除系統時間偏移的函數
        setThreadAffinity(CPU_ID) == 0 && // 指定cpu
        setThreadPriority(99) == 0 &&     // 指定優先級PRI 99 = RT
        setThreadNiceness(-20) == 0)      // 指定優先級NI -20
    {
        console("Starting RT task with dt=%u ns", PERIOD_NS);

        if (ec_init(EC_CH_NAME)) // 初始化 EtherCAT 主站
        {
            console("ec_init on [%s] " LIGHT_GREEN "succeeded." RESET, EC_CH_NAME);

            if (checkSlaveConfig() == 0) // 檢查連線的設備是否符合定義
            {
                console("checkSlaveConfig..." LIGHT_GREEN "succeeded" RESET);

                if (checkOperational() == 0)
                {
                }
                else
                {
                }

                struct timespec time_next_execution, time_now;
                //  當前的精準時間
                if (clock_gettime(CLOCK_MONOTONIC, &time_next_execution) == -1) //
                {
                    console("clock_gettime " RED "%s" RESET, strerror(errno));
                    return NULL;
                }

                int64_t dt;
                while (!execExit)
                {
                    // sleep直到指定的時間點
                    addTimespec(&time_next_execution, PERIOD_NS);
                    int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time_next_execution, NULL);
                    if (ret)
                    {
                        // sleep錯誤處理
                        printf("clock_nanosleep(): %s\n", strerror(ret));
                        // break;
                    }

                    // 取得當前的精準時間
                    clock_gettime(CLOCK_MONOTONIC, &time_now);

                    // ec_send_processdata();
                    // ec_receive_processdata(EC_TIMEOUTRET); // 執行收發一次

                    // 計算時間差距
                    int64_t dt = calcTimeDiffInNs(time_now, time_next_execution);
                    update_dt(dt);

                    EXEC_INTERVAL(30)
                    {
                        // 每30ms顯示一次實時統計的訊息
                        displayRealTimeInfo();
                    }
                    EXEC_INTERVAL_END

                    if (execExit)
                        break;
                }
            }
            else
            {
                console("checkSlaveConfig " RED "Failed." RESET);
            }

            console("ec_close");
            ec_close(); // 停止 EtherCAT 通信並關閉連接。
        }
        else
        {
            console("ec_init on [%s] " RED "Failed." RESET, EC_CH_NAME);
        }
    }
    else
    {
        console("config thread to realtime " RED "failed." RESET);
    }

    console("bgRealtimeDoWork thread exit");
    execExit = true;
    return NULL;
}
// ----------------------------------------------------------------
// ----------------------------------------------------------------

int main()
{
    console("delta_rs SOEM (Simple Open EtherCAT Master) Start... " LIGHT_GREEN "Process ID: %d" RESET, getProcessID());
    pthread_t bg_keyboard, bg_rt;

    pthread_create(&bg_keyboard, NULL, bgKeyboardDoWork, NULL); // 鍵盤執行序
    usleep(1000);
    pthread_create(&bg_rt, NULL, bgRealtimeDoWork, NULL); // RT執行序

    // 等待執行緒結束
    pthread_join(bg_rt, NULL);
    pthread_join(bg_keyboard, NULL);
    return 0;
}