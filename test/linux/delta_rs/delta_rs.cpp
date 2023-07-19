#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>

#include <termios.h> //keyboardPISOX中定义的标准接口

#include "arc_ipc_tool.hpp"
#include "arc_ec_tool.hpp"
#include "arc_rt_tool.hpp"
#include "arc_console.hpp"
#include "ec_config_delta.hpp"

// ----------------------------------------------------------------
#define EC_CH_NAME "eno1"           // 通訊用的eth設備名稱
#define CPU_ID 3                    // 指定運行的CPU編號
#define PERIOD_NS (1 * 1000 * 1000) // 1ms rt任務週期
#define EC_TIMEOUTMON 500

// Slave的站號
#define ASDA_I3_E_AXIS_6 0

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
bool needlf = false; // 顯示換行用
int line_count = 0;
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
int displayRealTimeInfo()
{
    console("cyc_count: %ld, Latency:(act, min, max, avg)us = (%-4ld, %ld, %ld, %.2f) ****",
            cyc_count,
            ns_dt / 1000,
            min_dt / 1000,
            max_dt / 1000,
            (double)sum_dt / cyc_count / 1000);

    return 1;
}
// ----------------------------------------------------------------
// 伺服馬達控制
// ----------------------------------------------------------------
bool ctrl_word[16] = {0};
bool status_word[16] = {0};
// servo on
bool request_servo_on = false;
bool request_servo_off = false;
int servo_on_step = 0;
int servo_on_work()
{
    switch (servo_on_step++)
    {

    // 故障復位
    case 0:
        ctrl_word[7] = false;
        break;
    case 1:
        ctrl_word[7] = true;
        break;
    case 2:
        ctrl_word[7] = false;
        break;

    // ref: delta ASDA-A3.pdf 使驅動器的狀態機進入準備狀態
    case 3: // 關閉
        ctrl_word[0] = false;
        ctrl_word[1] = true;
        ctrl_word[2] = true;
        ctrl_word[3] = false;
        ctrl_word[4] = false;
        break;

    case 4: // 準備使能
        ctrl_word[0] = true;
        ctrl_word[1] = true;
        ctrl_word[2] = true;
        ctrl_word[3] = false;
        ctrl_word[4] = false;
        break;

    case 5: // 始能
        ctrl_word[0] = true;
        ctrl_word[1] = true;
        ctrl_word[2] = true;
        ctrl_word[3] = true;
        ctrl_word[4] = false;
        break;

    default:
        return 0;
    }

    return servo_on_step;
}

bool request_trigger = false;
int servo_pp_trigger()
{
    //ref:
    // ASDA-A3.pdf 13-62(p915)
    // ASDA-A3.pdf (p880)
    static int step = 0;
    switch (step)
    {
    case 0:
        ctrl_word[4] = false; // RST 觸發命令
        ctrl_word[5] = true;  // 命令立即生效模式
        step++;
        break;

    case 1:
        if (status_word[12] == false) // 確認伺服收到命令訊號位元RST
            step++;
        break;
    case 2:
        ctrl_word[4] = true; // 觸發命令
        step++;
        break;
    case 3:
        if (status_word[12] == true) // 確認伺服收到命令訊號
        {
            ctrl_word[4] = false;    // RST 觸發命令
            step++;
        }
        break;
    default:
        step = 0;
        return 0;
    }
    return step;
}

int clamp(int number, int minValue, int maxValue)
{
    if (number < minValue)
    {
        return minValue;
    }
    else if (number > maxValue)
    {
        return maxValue;
    }
    else
    {
        return number;
    }
}
int position = 0;
int speed = 0;

int displayServoInfo(Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t *TxPDO, Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *RxPDO)
{
    int line_count = 0;
    console("ctrlword: 0x%X, (0x%X)*****", RxPDO->ControlWord, TxPDO->StatusWord);
    line_count++;
    console("position: [%f], %f, (%f)*****", (float)position / 10000, (float)RxPDO->TargetPosition / 10000, (float)TxPDO->ActualPosition / 10000);
    line_count++;
    console("speed   : [%d], %d, (%d)*****", speed, RxPDO->TargetVelocity, TxPDO->ActualVelocity);
    line_count++;
    console("torque  : %d, (%d)*****", RxPDO->TargetTorque, TxPDO->ActualTorque);
    line_count++;

    return line_count;
}
// ----------------------------------------------------------------
// ethercat
// ----------------------------------------------------------------
char IOmap[4096];
int wkc = 0;
int expected_wkc = 0;
bool inOP = false;
int currentgroup = 0;
Share_Memeber_t* share_memeber = NULL;

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
            printf("  Vendor ID: 0x%04X\n", ec_slave[i].eep_man);
            printf("  Product Code: 0x%04X\n", ec_slave[i].eep_id);
            // 添加其他從站信息
        }
    }

    ec_slave[ASDA_I3_E_AXIS_6].PO2SOconfig = setupDelta_ASDA_I3_E;

    // 將本地記憶體與slave記憶體建立映射
    ec_config_map(&IOmap);

    // 配置 EtherCAT 主站的時基
    ec_configdc();

    // 切換 EtherCAT 主站至SAFE_OP模式
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

/* get linux time synced to DC time */
void ec_sync(int64 reftime, int64 cycletime, int64 *offsettime)
{
    // 原來是用PI控制器的概念來同步時間呀...
    static int64 integral = 0;
    int64 delta;
    /* set linux sync point 50us later than DC sync, just as example */
    delta = (reftime - 50000) % cycletime;
    if (delta > (cycletime / 2))
    {
        delta = delta - cycletime;
    }
    if (delta > 0)
    {
        integral++;
    }
    if (delta < 0)
    {
        integral--;
    }
    *offsettime = -(delta / 100) - (integral / 20);
}

// 檢查EtherCAT 主站是否正常
void *bgEcatCheckDoWork(void *arg)
{
    // ref:
    // https://github.com/OpenEtherCATsociety/SOEM/blob/master/test/linux/simple_test/simple_test.c
    // ecatcheck()

    int slave;
    (void)arg; /* Not used */

    console("bgEcatCheckDoWork start, " LIGHT_GREEN "Thread ID: %d" RESET, getThreadID());

    while (1)
    {
        if (inOP && ((wkc < expected_wkc) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
                needlf = FALSE;
                MOVEDOWN(100);
            }
            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();
            for (slave = 1; slave <= ec_slavecount; slave++)
            {
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
                {
                    ec_group[currentgroup].docheckstate = TRUE;
                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                    {
                        printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state > EC_STATE_NONE)
                    {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d reconfigured\n", slave);
                        }
                    }
                    else if (!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            printf("ERROR : slave %d lost\n", slave);
                        }
                    }
                }
                if (ec_slave[slave].islost)
                {
                    if (ec_slave[slave].state == EC_STATE_NONE)
                    {
                        if (ec_recover_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d recovered\n", slave);
                        }
                    }
                    else
                    {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d found\n", slave);
                    }
                }
            }
            if (!ec_group[currentgroup].docheckstate)
                printf("OK : all slaves resumed OPERATIONAL.\n");
        }

        usleep(10000);
        if (execExit)
            break;
    }

    return NULL;
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

        // servo
        case 'o':
            request_servo_on = true;
            break;
        case 'p':
            request_servo_off = true;
            break;

        // position
        case 'w':
            position = 100 * 10000;
            break;
        case 's':
            position = 0;
            break;
        case 'x':
            position = -100 * 10000;
            break;

        // speed
        case 'e':
            speed += 10;
            if (speed > 1000)
                speed = 1000;
            drive_write32(ASDA_I3_E_AXIS_6, 0x6081, 0, speed * 10000);
            break;
        case 'd':
            speed = 0;
            drive_write32(ASDA_I3_E_AXIS_6, 0x6081, 0, speed * 10000);
            break;
        case 'c':
            speed -= 10;
            if (speed < 0)
                speed = 0;
            drive_write32(ASDA_I3_E_AXIS_6, 0x6081, 0, speed * 10000);
            break;
        case '3':
            
            break;
            
        case ' ':
            request_trigger = true;
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
        const int cycletime = PERIOD_NS;
        console("Starting RT task with dt=%u ns", cycletime);

        // --------------------------------------
        // 創建共享記憶體
        // --------------------------------------
        int shared_mem_id = createShareMem(1234);
        if (shared_mem_id != -1)
        {
            if(attachShareMem(shared_mem_id, &share_memeber) == 0)
            {
                console("create share memrory..." LIGHT_GREEN "succeeded" RESET);
            }
        }
        // --------------------------------------
        // --------------------------------------
        // --------------------------------------

        if (ec_init(EC_CH_NAME)) // 初始化 EtherCAT 主站
        {
            console("ec_init on [%s] " LIGHT_GREEN "succeeded." RESET, EC_CH_NAME);

            if (checkSlaveConfig() == 0) // 檢查連線的設備是否符合定義 切換至SAFE_OP模式
            {
                console("checkSlaveConfig..." LIGHT_GREEN "succeeded" RESET);

                // 計算預期的 work count
                expected_wkc = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;

                if (checkOperational() == 0)
                {
                    console(LIGHT_GREEN "Operational state reached for all slaves." RESET);
                    inOP = true;

                    struct timespec time_next_execution, time_now;
                    if (clock_gettime(CLOCK_MONOTONIC, &time_next_execution) == -1) // 當前的精準時間
                    {
                        console("clock_gettime " RED "%s" RESET, strerror(errno));
                        return NULL;
                    }

                    int64_t offset_time, dt;
                    while (!execExit)
                    {
                        // 計算下一個喚醒時間
                        addTimespec(&time_next_execution, cycletime + offset_time);

                        // wait to cycle start
                        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time_next_execution, NULL);

                        clock_gettime(CLOCK_MONOTONIC, &time_now); // for rt benchmark

                        if (ec_slave[0].hasdc)
                        {
                            // 計算offse_time以獲得讓Linux時間與同步DC時間
                            ec_sync(ec_DCtime, cycletime, &offset_time);
                        }

                        // -------------------------------------
                        // renew inputs
                        // -------------------------------------
                        wkc = ec_receive_processdata(EC_TIMEOUTRET);
                        Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t *iptr = (Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t *)ec_slave[ASDA_I3_E_AXIS_6].inputs;
                        for (int i = 0; i < 16; i++)
                        {
                            status_word[i] = (iptr->StatusWord >> i) & 1;
                        }

                        // -------------------------------------
                        // logic
                        // -------------------------------------
                        Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *optr = (Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *)ec_slave[ASDA_I3_E_AXIS_6].outputs;

                        if (request_servo_on)
                        {
                            if (servo_on_work() == 0)
                            {
                                request_servo_on = false;
                            }
                        }

                        if (request_servo_off)
                        {
                            request_servo_off = false;
                            servo_on_step = 0;
                            // ref ASDA-A3 p914
                            ctrl_word[0] = false;
                            ctrl_word[1] = true;
                            ctrl_word[2] = true;
                            ctrl_word[7] = false;
                            // console("request_servo_off");
                        }

                        if (request_trigger)
                        {
                            if (status_word[2] == false) // 還沒有ServoOn
                                request_trigger = false;
                            else
                            {
                                int ret = servo_pp_trigger();
                                if (ret == 0)
                                {
                                    request_trigger = false;
                                }
                            }
                        }

                        // -------------------------------------
                        // update outputs
                        // -------------------------------------
                        if (ec_slavecount >= ASDA_I3_E_AXIS_6)
                        {
                            Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *optr;
                            optr = (Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *)ec_slave[ASDA_I3_E_AXIS_6].outputs;

                            // int16 ControlWord = ctrl_word[16]
                            optr->ControlWord = 0;
                            for (int i = 0; i < 16; i++)
                                optr->ControlWord |= (ctrl_word[i] ? 1 : 0) << i;

                            // EXEC_INTERVAL(50)
                            // {
                            //     printBinary16(optr->ControlWord);
                            // }
                            // EXEC_INTERVAL_END

                            optr->TargetPosition = position;
                        }

                        ec_send_processdata();

                        // -------------------------------------
                        // rt benchmark
                        // 計算用於測定rt能力的時間差距
                        int64_t dt = calcTimeDiffInNs(time_now, time_next_execution);
                        update_dt(dt);
                        EXEC_INTERVAL(50)
                        {
                            Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t *iptr = (Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t *)ec_slave[ASDA_I3_E_AXIS_6].inputs;
                            Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *optr = (Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t *)ec_slave[ASDA_I3_E_AXIS_6].outputs;

                            // 每段時間顯示一次訊息
                            line_count += displayRealTimeInfo();
                            line_count += displayServoInfo(iptr, optr);

                            if (share_memeber != NULL)
                            {
                                for (int i = 0; i < 6; ++i)
                                    console("ActualPosition[%d]: %d", i, share_memeber->ActualPosition[i]);                                    
                                line_count += 6;

                                // for (int idx = 0; idx < 6; idx++)
                                // {
                                //     share_memeber->ActualPosition[idx] += idx; //test
                                // }
                                share_memeber->ActualPosition[0] = 0;
                                share_memeber->ActualPosition[1] = 0;
                                share_memeber->ActualPosition[2] = -2025000;
                                share_memeber->ActualPosition[3] = 2025000;
                                share_memeber->ActualPosition[4] = -2025000;
                                share_memeber->ActualPosition[5] = 0;
                            }

                            MOVEUP(line_count);
                            line_count = 0;
                            needlf = true;

                            // debug hexdump
                            // dumpHex(ec_slave[ASDA_I3_E_AXIS_6].inputs, 32);
                            // dumpHex(ec_slave[ASDA_I3_E_AXIS_6].outputs, 32);
      

                        }
                        EXEC_INTERVAL_END
                        // ---------------------------------------------------

                        if (execExit)
                            break;
                    }
                    inOP = false;
                }
                else
                {
                }
            }
            else
            {
                console("checkSlaveConfig " RED "Failed." RESET);
            }
            // 恢復成INIT
            console("[Request INIT state for all slaves");
            ec_slave[0].state = EC_STATE_INIT;
            ec_writestate(0);

            // 停止 EtherCAT 通信並關閉連接。
            console("ec_close");
            ec_close();
        }
        else
        {
            console("ec_init on [%s] " RED "Failed." RESET, EC_CH_NAME);
        }

        if (share_memeber != NULL)
        {
            // deinit share memory
            detachShareMem(share_memeber);
            removeShareMem(shared_mem_id);
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
    pthread_t bg_keyboard, bg_rt, bg_ecatcheck;

    pthread_create(&bg_keyboard, NULL, bgKeyboardDoWork, NULL); // 鍵盤輸入執行序
    usleep(1000);
    pthread_create(&bg_ecatcheck, NULL, bgEcatCheckDoWork, NULL); // EC狀態檢查執行序
    usleep(1000);
    pthread_create(&bg_rt, NULL, bgRealtimeDoWork, NULL); // RT執行序

    // 等待執行緒結束
    pthread_join(bg_rt, NULL);
    pthread_join(bg_ecatcheck, NULL);
    pthread_join(bg_keyboard, NULL);
    return 0;
}