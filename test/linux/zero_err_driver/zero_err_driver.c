#define _GNU_SOURCE
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>

#include <sys/time.h>

// cpu latency
#include <sys/stat.h>
#include <fcntl.h>

// set priorty
#include <unistd.h>
#include <sys/resource.h>

// 線程 cpu 榜定
#include <sched.h>
#include <stdio.h>

// keyboard
#include <termios.h> //PISOX中定义的标准接口
#include <ctype.h>

#include "ethercat.h"
#include "arc_console.hpp"

#define EC_TIMEOUTMON 500

// -------------------------------------
// 設備定義
// -------------------------------------
#define ETH_CH_NAME "eno1" // 通訊用的eth設備名稱
// Slave的站號
#define R2_EC0902 1
#define ZeroErr_Driver_1 2
#define NUMBER_OF_SLAVES 2
// -------------------------------------
// -------------------------------------

#define CPU_ID 7 // 指定運行的CPU編號

// RT Loop的週期
#define PERIOD_NS (1 * 1000 * 1000)

boolean bg_cancel = 0;
OSAL_THREAD_HANDLE bg_ecatcheck;
OSAL_THREAD_HANDLE bg_keyboard;

boolean dynamicY = FALSE;

int usedmem;
char IOmap[4096];
boolean DO[32];
boolean CtrlWord[16];

int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

int display_move = 5;   


int64 last_cktime = 0;
int64 max_dt = LLONG_MIN;
int64 min_dt = LLONG_MAX;
int64 sum_dt = 0;
int64 cyc_count = 0;

//-----------------------------------
// Driver_Outputs
//  0x1600      "R0PDO"     [ARRAY  maxsub(0x11 / 17)]
//     0x00      "S"        [UNSIGNED8        RWRWRW]      0x03 / 3
//     0x01      "S"        [UNSIGNED32       RWRWRW]      0x607a0020 / 1618608160
//     0x02      "S"        [UNSIGNED32       RWRWRW]      0x60fe0020 / 1627258912
//     0x03      "S"        [UNSIGNED32       RWRWRW]      0x60400010 / 1614807056
//
//  0x607a      "Target Position"       [VAR]
//     0x00      "Target Position"      [INTEGER32        RWRWRW]      0x00000000 / 0
//  0x60fe      "Digital outputs"       [VAR]
//     0x00      "Digital outputs"      [UNSIGNED32       RWRWRW]      0x00000000 / 0
//  0x6040      "Control Word"          [VAR]
//     0x00      "Control Word"         [UNSIGNED16       RWRWRW]      0x0000 / 0
typedef struct 
{
    int32_t Position;
    uint32_t DigitalOutputs;
    uint16_t CtrlWord;
    /* data */
}Driver_Outputs;

// end of Driver_Outputs
//-----------------------------------
// Driver_Inputs
// 0x1a00      "T0PDO"      [ARRAY  maxsub(0x11 / 17)]
//     0x00      "S"        [UNSIGNED8        RWRWRW]      0x03 / 3
//     0x01      "S"        [UNSIGNED32       RWRWRW]      0x60640020 / 1617166368
//     0x02      "S"        [UNSIGNED32       RWRWRW]      0x60fd0020 / 1627193376
//     0x03      "S"        [UNSIGNED32       RWRWRW]      0x60410010 / 1614872592
// 0x6064      "Position Actual Value"          [VAR]
//     0x00      "Position Actual Value"        [INTEGER32        R_R_R_]      0x0003bb05 / 244485
// 0x60fd      "Digital inputs"                 [VAR]
//     0x00      "Digital inputs"               [UNSIGNED32       R_R_R_]      0x00000000 / 0
// 0x6041      "Status Word"                    [VAR]
//     0x00      "Status Word"                  [UNSIGNED16       R_R_R_]      0x1208 / 4616

typedef struct 
{
    int32_t Position;
    uint32_t DigitalInputs;
    uint16_t StatWord;
    /* data */
}Driver_Inputs;
// end of Driver_Inputs
//-----------------------------------

typedef enum{
    Mode_Position = 0,
    Mode_Velocity
}Driver_Modes;
Driver_Modes driver_mode = Mode_Position;
int32_t pos_target = 0;
int32_t pos_feedback = 0;
int32_t max_speed = 10;

int32_t direct = 1;
int32_t temp_speed = 0;

boolean request_servo_on = 0;
boolean request_servo_off = 0;




static int sdo_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(uint8), &value, EC_TIMEOUTRXM);
}

static inline void printBinary(uint16_t num)
{
    for (int i = 15; i >= 0; i--)
    {
        printf("%d", (num >> i) & 1);
        if (i % 4 == 0)
        {
            printf(" ");
        }
    }
    printf("b");
}

/* ref: cyclic_test */
void set_latency_target(void)
{
    struct stat s;
    int ret;

    if (stat("/dev/cpu_dma_latency", &s) == 0)
    {
        int latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
        if (latency_target_fd == -1)
            return;

        int32_t latency_target_value = 0;
        ret = write(latency_target_fd, &latency_target_value, 4);
        if (ret == 0)
        {
            printf("# error setting cpu_dma_latency to %d!: %s\r\n", latency_target_value, strerror(errno));
            close(latency_target_fd);
            return;
        }
        console("/dev/cpu_dma_latency set to %dus", latency_target_value);
    }
}

// Priority
int setPRICPUx(int Priority, int cpu_id)
{
    // printf("[setPRICPUx] Priority= %d, cpu_id= %d\r\n", Priority, cpu_id);

    int ret = 0;
    // 指定 程序運作的cpu_id
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    pthread_t thread = pthread_self();
    pthread_setaffinity_np(thread, sizeof(mask), &mask);

    // Set Priority, 99 = RT
    struct sched_param schedp;
    memset(&schedp, 0, sizeof(schedp));
    schedp.sched_priority = Priority;
    ret = sched_setscheduler(0, SCHED_FIFO, &schedp);
    if (ret)
    {
        console("[setPRICPUx] Warning: sched_setscheduler failed: %s", strerror(errno));
    }

    return ret;
}

int setNI(int Niceness)
{
    int ret = 0;
    pid_t pid = getpid(); // 獲得進程PID

    // Set NI (Niceness) -19 = 最高優先權
    ret = setpriority(PRIO_PROCESS, pid, Niceness);
    if (ret)
    {
        printf("Warning: setpriority failed: %s\r\n", strerror(errno));
        return ret;
    }
    return ret;
}

int setupDeltaIO(uint16 slave)
{
    int wkc = 0;
    console("[slave:%d] DELTA RC-EC0902 setup", slave);

    // Active all DO port ----------------------------------------------------------
    // 此物件可以設定輸出通道是否允許變更(8 個輸出通道為一組)。0 代表不允許改變狀態，1 代表允許改變狀態。
    // 0x2001      "Active DO Enable"                          [RECORD  maxsub(0x04 / 4)]
    //   0x00      "SubIndex 000"                              [UNSIGNED8        R_R_R_]      0x04 / 4
    //   0x01      "Active Port2 DO CH0~7 Enable"              [UNSIGNED8        RWRWRW]      0xff / 255
    //   0x02      "Active Port2 DO CH8~15 Enable"             [UNSIGNED8        RWRWRW]      0xff / 255
    //   0x03      "Active Port3 DO CH0~7 Enable"              [UNSIGNED8        RWRWRW]      0xff / 255
    //   0x04      "Active Port3 DO CH8~15 Enable"             [UNSIGNED8        RWRWRW]      0xff / 255
    wkc += sdo_write8(slave, 0x2001, 1, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 2, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 3, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 4, 0xFF);

    // Error Mode disable ----------------------------------------------------------
    // 0 代表維持原本輸出值，1 代表參考Error Mode Output Value(6207h)的設定值。
    // 0x6206      "DO Error Mode Enable"                        [RECORD  maxsub(0x04 / 4)]
    //   0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    //   0x01      "Port2 DO Ch0~7 Error Mode Enable"            [UNSIGNED8        RWRWRW]      0x00 / 0
    //   0x02      "Port2 DO Ch8~15 Error Mode Enable"           [UNSIGNED8        RWRWRW]      0x00 / 0
    //   0x03      "Port3 DO Ch0~7 Error Mode Enable"            [UNSIGNED8        RWRWRW]      0x00 / 0
    //   0x04      "Port3 DO Ch8~15 Error Mode Enable"           [UNSIGNED8        RWRWRW]      0x00 / 0
    wkc += sdo_write8(slave, 0x6206, 1, 0x0);
    wkc += sdo_write8(slave, 0x6206, 2, 0x0);
    wkc += sdo_write8(slave, 0x6206, 3, 0x0);
    wkc += sdo_write8(slave, 0x6206, 4, 0x0);

    // strncpy(ec_slave[slave].name, "IO", EC_MAXNAME);

    if (wkc != 8)
    {
        console("[slave:%d] DELTA RC-EC0902 setup failed. wkc: %d", slave, wkc);
        return -1;
    }
    else
    {
        console("[slave:%d] DELTA RC-EC0902 setup "LIGHT_GREEN"succeed."RESET, slave);
        return 0;
    }
}

int setupZeroErrDriver(uint16 slave)
{
    int wkc = 0;
    const int check_wkc = 2;
    console("[slave:%d] ZeroErrDriver setup", slave);
    // 釋放煞車
    // 0x4602      "Release Brake"    [VAR]
    //   0x00      "Release Brake"    [UNSIGNED32       RWRWRW]      0x00000000 / 0
    //wkc += sdo_write8(slave, 0x4602, 0, 0x0);

    uint16 map_RxPDOassign[] = {0x0001, 0x1600}; // 0x1c12
    wkc += ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_RxPDOassign), &map_RxPDOassign, EC_TIMEOUTSAFE );

    uint16 map_TxPDOassign[] = {0x0001, 0x1A00}; // 0x1c13
    wkc += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_TxPDOassign), &map_TxPDOassign, EC_TIMEOUTSAFE );

    //wkc += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_TxPDOassign), &map_TxPDOassign, EC_TIMEOUTSAFE );

    //uint32 map_TxPDO[] = {0x0002, 0x60640020, 0x60FD0020};
    //wkc += ec_SDOwrite(slave, 0x1A00, 0x00, TRUE, sizeof(map_TxPDO), &map_TxPDO, EC_TIMEOUTSAFE );

    if (wkc != check_wkc)
    {
        console("[slave:%d] ZeroErrDriversetup "RED"failed."RESET" wkc: %d", slave, wkc);
        return -1;
    }
    else
    {
        console("[slave:%d] ZeroErrDriver setup "LIGHT_GREEN"succeed."RESET, slave);
        return 0;
    }
}

void modifyBit8(uint8 *value, int p, boolean bit)
{
    int mask = 1 << p;
    *value = ((*value & ~mask) | (bit << p));
}

void modifyBit16(uint16 *value, int p, boolean bit)
{
    int mask = 1 << p;
    *value = ((*value & ~mask) | (bit << p));
}

boolean getBit(uint8 *value, int p)
{
    return (*value >> p) & 1;
}

static inline int64_t calcdiff_ns(struct timespec t1, struct timespec t2)
{
    int64_t tdiff;
    tdiff = NSEC_PER_SEC * (int64_t)((int)t1.tv_sec - (int)t2.tv_sec);
    tdiff += ((int)t1.tv_nsec - (int)t2.tv_nsec);
    return tdiff;
}

/* add ns to timespec */
void add_timespec(struct timespec *ts, int64 addtime)
{
    int64 sec, nsec;

    nsec = addtime % NSEC_PER_SEC;
    sec = (addtime - nsec) / NSEC_PER_SEC;
    ts->tv_sec += sec;
    ts->tv_nsec += nsec;
    if (ts->tv_nsec > NSEC_PER_SEC)
    {
        nsec = ts->tv_nsec % NSEC_PER_SEC;
        ts->tv_sec += (ts->tv_nsec - nsec) / NSEC_PER_SEC;
        ts->tv_nsec = nsec;
    }
}

/* PI calculation to get linux time synced to DC time */
void ec_sync(int64 reftime, int64 cycletime, int64 *offsettime)
{
    static int64 integral = 0;
    int64 delta;
    /* set linux sync point 50us later than DC sync, just as example */
    delta = (reftime - 50 * 1000) % cycletime;
    // delta = (reftime - 200*1000) % cycletime;
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

void cyclic_test()
{
    // ----------------------------------------------------
    // real-time 定時器
    // ----------------------------------------------------
    console("Starting RT task with dt=%u ns.", PERIOD_NS);
    const int64 cycletime = PERIOD_NS; /* cycletime in ns */

    struct timespec wakeup_time;
    if (clock_gettime(CLOCK_MONOTONIC, &wakeup_time) == -1) // 當前的精準時間
    {
        printf("clock_gettime failed\r\n");
        return;
    }

    int64 dt;
    struct timespec tnow;

    while (!bg_cancel)
    {
        // sleep直到指定的時間點
        add_timespec(&wakeup_time, cycletime);
        int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
        if (ret)
        {
            // sleep錯誤處理
            printf("clock_nanosleep(): %s\n", strerror(ret));
            // break;
        }

        clock_gettime(CLOCK_MONOTONIC, &tnow);
        dt = calcdiff_ns(tnow, wakeup_time);

        cyc_count++;
        sum_dt += dt;

        if (dt < min_dt)
            min_dt = dt;

        if (dt > max_dt)
            max_dt = dt;

        // 顯示
        EXEC_INTERVAL(100)
        {
            consoler("cyc_count: %ld, Latency:(min, max, avg)us = (%ld, %ld, %.2f) ****",
                     cyc_count,
                     min_dt / 1000, max_dt / 1000, (double)sum_dt / cyc_count / 1000);
        }
        EXEC_INTERVAL_END
    }
}

int servo_on_step = 0;
void servo_on_work()
{
    switch (servo_on_step++)
    {
    // 故障復位
    case 0: 
        CtrlWord[7] = FALSE;
        break;    
    case 1:
        CtrlWord[7] = TRUE;
        break;
    case 2:
        CtrlWord[7] = FALSE;
        break;

    case 3: // 關閉
        CtrlWord[0] = FALSE;
        CtrlWord[1] = TRUE;
        CtrlWord[2] = TRUE;
        CtrlWord[3] = FALSE;
        break;

    case 4: // 準備使能
        CtrlWord[0] = TRUE;
        CtrlWord[1] = TRUE;
        CtrlWord[2] = TRUE;
        CtrlWord[3] = FALSE;
        break;

    case 5: // 始能
        CtrlWord[0] = TRUE;
        CtrlWord[1] = TRUE;
        CtrlWord[2] = TRUE;
        CtrlWord[3] = TRUE;
        break;
    
    default:
        //servo_on_step = 0;
        break;
    } 
}

void cyclic_task()
{
    // ----------------------------------------------------
    // real-time 定時器
    // ----------------------------------------------------
    console("Starting RT task with dt=%u ns.", PERIOD_NS);
    const int64 cycletime = PERIOD_NS; /* cycletime in ns */

    struct timespec wakeup_time;
    if (clock_gettime(CLOCK_MONOTONIC, &wakeup_time) == -1) // 當前的精準時間
    {
        printf("clock_gettime failed\r\n");
        return;
    }

    pos_target = pos_feedback;

    // 初始統計時間
    last_cktime = ec_DCtime;

    //
    struct timespec tnow;
    int64 toff = 0;
    int64 dt;

    while (!bg_cancel)
    {
        // sleep直到指定的時間點
        add_timespec(&wakeup_time, cycletime + toff);
        int ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
        clock_gettime(CLOCK_MONOTONIC, &tnow);
        dt = calcdiff_ns(tnow, wakeup_time);

        if (ret)
        {
            // sleep錯誤處理
            printf("clock_nanosleep(): %s\n", strerror(ret));
            // break;
        }

        // calulate toff to get linux time and DC synced
        if (ec_slave[0].hasdc)
        {
            ec_sync(ec_DCtime, cycletime, &toff);
            // console("[debug] toff = %ld ns", toff);
        }

        wkc = ec_receive_processdata(EC_TIMEOUTRET);
        Driver_Inputs *iptr = (Driver_Inputs*)ec_slave[ZeroErr_Driver_1].inputs;
        Driver_Outputs *optr = (Driver_Outputs*)ec_slave[ZeroErr_Driver_1].outputs;

        // -------------------------------------
        // renew inputs
        // -------------------------------------
        pos_feedback = iptr->Position;

        // -------------------------------------
        // logic
        // -------------------------------------
        // 開關所有的Y
        if (dynamicY && cyc_count % 100 == 0)
        {        
            for (size_t idx = 0; idx < 32; idx++)
            {
                DO[idx] = !DO[idx];
            }
        }
        
        if(request_servo_on)
        {
            request_servo_on = FALSE;
            pos_target = pos_feedback;
            servo_on_work();
        }

        if(request_servo_off)
        {
            request_servo_off = FALSE;
            servo_on_step = 0;
            CtrlWord[0] = FALSE;     
            CtrlWord[1] = FALSE;
            CtrlWord[2] = FALSE;
            CtrlWord[3] = FALSE;       
        }

        if(driver_mode == Mode_Position)
        {

        }
        else if(driver_mode == Mode_Velocity)
        {
            //EXEC_INTERVAL(1)
            {
                int k_delta_speed = 1;
                if (direct == 1)
                {
                    temp_speed += k_delta_speed;
                    if (temp_speed > max_speed)
                        temp_speed = max_speed;
                }
                else if (direct == -1)
                {
                    temp_speed -= k_delta_speed;
                    if (temp_speed < -max_speed)
                        temp_speed = -max_speed;
                }
                else
                {
                    if (temp_speed > 0)
                        temp_speed -= k_delta_speed;
                    else if (temp_speed < 0)
                        temp_speed += k_delta_speed;
                }

                pos_target += temp_speed;
            }
            //EXEC_INTERVAL_END;

        }


        // -------------------------------------
        // update outputs
        // ------------------------------------
        optr->Position = pos_target;
        for (size_t idx_bit = 0; idx_bit < 8; idx_bit++)
        {
            modifyBit8(&ec_slave[R2_EC0902].outputs[0], idx_bit, DO[0 * 8 + idx_bit]);
            modifyBit8(&ec_slave[R2_EC0902].outputs[1], idx_bit, DO[1 * 8 + idx_bit]);
            modifyBit8(&ec_slave[R2_EC0902].outputs[2], idx_bit, DO[2 * 8 + idx_bit]);
            modifyBit8(&ec_slave[R2_EC0902].outputs[3], idx_bit, DO[3 * 8 + idx_bit]);
        }

        for (size_t idx_bit = 0; idx_bit < 16; idx_bit++)
        {
            modifyBit16(&optr->CtrlWord, idx_bit, CtrlWord[idx_bit]);
        }


        ec_send_processdata();

        // dt = ck_time2 - ck_time1; // ec rx 用時
        // dt = ck_time4 - ck_time3; // ec tx 用時
        // dt = ck_time4 - ck_time1; // 整體 用時

        cyc_count++;
        sum_dt += dt;

        if (dt < min_dt)
            min_dt = dt;

        if (dt > max_dt)
            max_dt = dt;

        // 顯示
        EXEC_INTERVAL(100)        
        {
            int NLcount = 0;
            console("cyc_count: %ld, Latency:(min, max, avg)us = (%ld, %ld, %.2f) T:%ld+(%3ld)ns ****",
                    cyc_count,
                    min_dt / 1000, max_dt / 1000, (double)sum_dt / cyc_count / 1000,
                    ec_DCtime, toff);
            NLcount++;

            console("---- IOmap infomation ----");
            NLcount++;
            for (int idx = 0; idx < usedmem; idx++)
            {
                printf("%02X ", (IOmap[idx]) & 0xFF);
            }
            printf("  ----  \r\n");
            NLcount++;

            console("---- driver infomation ----");
            NLcount++;
            printf("pose(output, input):\t %10d %10d, speed(max, tmp): %d, %d ---- \r\n", optr->Position, iptr->Position, max_speed, temp_speed);
            NLcount++;
            //printf("DIO(output, input): \t 0x%X 0x%X\r\n", optr->DigitalOutputs, iptr->DigitalInputs);
            printf("(CtrlWord, StatWord)):\t");
            printf("|%5d = " ,optr->CtrlWord);
            printBinary(optr->CtrlWord);
            printf("|%5d = " ,iptr->StatWord);
            printBinary(iptr->StatWord);
            printf(" ----\r\n");
            NLcount++;

            fflush(stdout);
            MOVEUP(NLcount);

            //printf("accPosition %d", iptr->Position);
            // for (int idx = 0; idx < (int)ec_slave[R2_EC0902].Obytes; idx++)
            // {
            //     printf("%02X ", ec_slave[R2_EC0902].outputs[idx]);
            // }    
        }
        EXEC_INTERVAL_END

        if (wkc >= expectedWKC)
        {
            // printf("\t\tProcessdata cycle %4d, WKC %d , O:", cyc_count++, wkc);

            // for (int j = 0; j < 4; j++)
            // {
            //     printf(" %2.2x", *(ec_slave[0].outputs + j));
            // }

            // printf(" I:");
            // for (int j = 0; j < 4; j++)
            // {
            //     printf(" %2.2x", *(ec_slave[0].inputs + j));
            // }
            // printf(" T:%" PRId64 "\r", ec_DCtime);
            // needlf = TRUE;f
        }
    }
    MOVEDOWN(display_move);

}

void print_ec_group(ec_groupt group)
{
    printf(" ---- print_ec_group: ---- \r\n");
    printf("logstartaddr: %" PRIu32 "\n", group.logstartaddr);
    printf("Obytes: %" PRIu32 "\n", group.Obytes);
    printf("outputs: %p\n", (void *)group.outputs);
    printf("Ibytes: %" PRIu32 "\n", group.Ibytes);
    printf("inputs: %p\n", (void *)group.inputs);
    printf("hasdc: %d\n", group.hasdc);
    printf("DCnext: %" PRIu16 "\n", group.DCnext);
    printf("Ebuscurrent: %" PRId16 "\n", group.Ebuscurrent);
    printf("blockLRW: %" PRIu8 "\n", group.blockLRW);
    printf("nsegments: %" PRIu16 "\n", group.nsegments);
    printf("Isegment: %" PRIu16 "\n", group.Isegment);
    printf("Ioffset: %" PRIu16 "\n", group.Ioffset);
    printf("outputsWKC: %" PRIu16 "\n", group.outputsWKC);
    printf("inputsWKC: %" PRIu16 "\n", group.inputsWKC);
    printf("docheckstate: %d\n", group.docheckstate);
    printf("IOsegment: ");
    for (int i = 0; i < EC_MAXIOSEGMENTS; i++)
    {
        printf("%" PRIu32 " ", group.IOsegment[i]);
    }
    printf("\n");
}
void print_ec_slave(struct ec_slave slave)
{
    printf(" ---- print_ec_slave: [%s] ---- \r\n", slave.name);
    printf("state: %u\n", slave.state);
    printf("ALstatuscode: %u\n", slave.ALstatuscode);
    printf("configadr: %u\n", slave.configadr);
    printf("aliasadr: %u\n", slave.aliasadr);
    printf("eep_man: %u\n", slave.eep_man);
    printf("eep_id: %u\n", slave.eep_id);
    printf("eep_rev: %u\n", slave.eep_rev);
    printf("Itype: %u\n", slave.Itype);
    printf("Dtype: %u\n", slave.Dtype);
    printf("Obits: %u\n", slave.Obits);
    printf("Obytes: %u\n", slave.Obytes);
    printf("Ostartbit: %u\n", slave.Ostartbit);
    printf("Ibits: %u\n", slave.Ibits);
    printf("Ibytes: %u\n", slave.Ibytes);
    printf("Istartbit: %u\n", slave.Istartbit);
    printf("mbx_l: %u\n", slave.mbx_l);
    printf("mbx_wo: %u\n", slave.mbx_wo);
    printf("mbx_rl: %u\n", slave.mbx_rl);
    printf("mbx_ro: %u\n", slave.mbx_ro);
    printf("mbx_proto: %u\n", slave.mbx_proto);
    printf("mbx_cnt: %u\n", slave.mbx_cnt);
    printf("hasdc: %d\n", slave.hasdc);
    printf("ptype: %u\n", slave.ptype);
    printf("topology: %u\n", slave.topology);
    printf("activeports: %u\n", slave.activeports);
    printf("consumedports: %u\n", slave.consumedports);
    printf("parent: %u\n", slave.parent);
    printf("parentport: %u\n", slave.parentport);
    printf("entryport: %u\n", slave.entryport);
    printf("DCrtA: %d\n", slave.DCrtA);
    printf("DCrtB: %d\n", slave.DCrtB);
    printf("DCrtC: %d\n", slave.DCrtC);
    printf("DCrtD: %d\n", slave.DCrtD);
    printf("pdelay: %d\n", slave.pdelay);
    printf("DCnext: %u\n", slave.DCnext);
    printf("DCprevious: %u\n", slave.DCprevious);
    printf("DCcycle: %d\n", slave.DCcycle);
    printf("DCshift: %d\n", slave.DCshift);
    printf("DCactive: %u\n", slave.DCactive);
    printf("configindex: %u\n", slave.configindex);
    printf("SIIindex: %u\n", slave.SIIindex);
    printf("eep_8byte: %u\n", slave.eep_8byte);
    printf("eep_pdi: %u\n", slave.eep_pdi);
    printf("CoEdetails: %u\n", slave.CoEdetails);
    printf("FoEdetails: %u\n", slave.FoEdetails);
    printf("EoEdetails: %u\n", slave.EoEdetails);
    printf("SoEdetails: %u\n", slave.SoEdetails);
    printf("CoE details: %d\n", slave.CoEdetails);
    printf("FoE details: %d\n", slave.FoEdetails);
    printf("EoE details: %d\n", slave.EoEdetails);
    printf("SoE details: %d\n", slave.SoEdetails);
    printf("E-bus current: %d\n", slave.Ebuscurrent);
    printf("Block LRW: %d\n", slave.blockLRW);
    printf("Group: %d\n", slave.group);
    printf("First unused FMMU: %d\n", slave.FMMUunused);
    printf("Is lost: %d\n", slave.islost);
}
void simpletest(char *ifname)
{
    int i;
    needlf = FALSE;
    inOP = FALSE;

    int64 ck_time = clock_ms();
    int64 k_timeout = 3000; // 3s;

    console("main work start...");

    // 設定程式搶佔優先權到最高
    console("Priority= RT, NI=-20 cpu_id= %d", CPU_ID);
    setPRICPUx(99, CPU_ID); // 99=RT
    setNI(-20);

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        console("ec_init on %s succeeded.", ifname);
        /* find and auto-config slaves */
        if (ec_config_init(FALSE) > 0)
        {
            console("%d slaves found and configured.", ec_slavecount);

            // list all slave name
            console("---- slave name ----");
            for (int slave_id = 1; slave_id <= ec_slavecount; slave_id++)
            {
                console("[slave:%d] name: %s", slave_id, ec_slave[slave_id].name);
            }

            memset(DO, 0, sizeof(DO));

            console("---- slave config ----");
            
            ec_slave[R2_EC0902].PO2SOconfig = setupDeltaIO;
            ec_slave[ZeroErr_Driver_1].PO2SOconfig = setupZeroErrDriver;
            usedmem = ec_config_map(&IOmap);
            console("IOmap address %p used memsize %d", IOmap, usedmem);
            console("Slaves mapped state to SAFE_OP.");



            ec_configdc();
            // for (int slave_id = 1; slave_id <= ec_slavecount; slave_id++)
            // {
            //      ec_dcsync0(slave_id, TRUE, PERIOD_NS, 20000U);
            // }

            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

            /* read indevidual slave state and store in ec_slave[] */
            ec_readstate();

            // list all slave infomation
            console("---- slave infomation ----");
            for (int cnt = 1; cnt <= ec_slavecount; cnt++)
            {
                console("Slave:%d Name:%s Output size:%3dbits Input size:%3dbits State:%2d delay:%d.%d",
                        cnt, ec_slave[cnt].name,
                        ec_slave[cnt].Obits,
                        ec_slave[cnt].Ibits,
                        ec_slave[cnt].state,
                        (int)ec_slave[cnt].pdelay,
                        ec_slave[cnt].hasdc);

                //print_ec_slave(ec_slave[cnt]);
            }
            //print_ec_group(ec_group[0]);
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            console("Calculated workcounter %d", expectedWKC);    

            console("Request operational state for all slaves");
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* request OP state for all slaves */
            ec_writestate(0);

            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* wait for all slaves to reach OP state */
            console("wait for all slaves to reach OP state");
            
            ck_time = clock_ms();
            while (!bg_cancel && (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTRET) != EC_STATE_OPERATIONAL))
            {
                int64 dt = clock_ms() - ck_time;
                if (dt > k_timeout)
                {
                    printf(RED "Timeout" RESET);
                    break;
                }
                consoler("wait for all slaves to reach OP state (%.1fs)...", (float32)(k_timeout - dt) / 1000);
            }

            if (ec_slave[0].state == EC_STATE_OPERATIONAL)
            {
                console("Operational state reached for all slaves.");
                inOP = TRUE;

                cyclic_task();
                // cyclic_test();

                inOP = FALSE;
            }
            else
            {
                console("Not all slaves reached operational state.");
                ec_readstate();
                for (i = 1; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_OPERATIONAL)
                    {
                        console("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s", i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                    }
                }
            }
            console("[Exit] Request init state for all slaves");
            ec_slave[0].state = EC_STATE_INIT;
            /* request INIT state for all slaves */
            ec_writestate(0);
        }
        else
        {
            console("No slaves found!\r\n");
        }

        console("[Exit] close socket\r\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
        printf("No socket connection on %s\r\nExcecute as root\r\n", ifname);
    }
}

OSAL_THREAD_FUNC ecatcheck(void *ptr)
{
    console("[Thread] ecatcheck start");
    int slave;
    (void)ptr; /* Not used */
    // 設定程式優先權
    setPRICPUx(20, CPU_ID); //

    while (!bg_cancel)
    {
        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
                needlf = FALSE;
                printf("\r\n");
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
                        printf("\r\n ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\r\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        printf("\r\n WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\r\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state > EC_STATE_NONE)
                    {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("\r\n MESSAGE : slave %d reconfigured\r\n", slave);
                        }
                    }
                    else if (!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);
                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            printf("\r\n ERROR : slave %d lost\r\n", slave);
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
                            printf("\r\n MESSAGE : slave %d recovered\r\n", slave);
                        }
                    }
                    else
                    {
                        ec_slave[slave].islost = FALSE;
                        printf("\r\n MESSAGE : slave %d found\r\n", slave);
                    }
                }
            }
            if (!ec_group[currentgroup].docheckstate)
                printf("\r\n OK : all slaves resumed OPERATIONAL.\r\n");
        }
        osal_usleep(10000);
    }

    printf("exit ecatcheck\r\r\n");
}

OSAL_THREAD_FUNC keyboard(void *ptr)
{
    console("[Thread] keyboard start");

    (void)ptr; /* Not used */

    // 設定程式優先權
    setPRICPUx(21, CPU_ID); //

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

        if (isdigit(ch))
        {
            int idx = ch - '0';
            //DO[idx] = !DO[idx];
            CtrlWord[idx] = !CtrlWord[idx];
        }

        switch (ch)
        {
        case ' ':
            dynamicY = !dynamicY;
            break;
        case 'i':
            max_dt = LLONG_MIN;
            min_dt = LLONG_MAX;
            sum_dt = 0;
            cyc_count = 0;
            break;
        case 'q':
            printf("\r\n");
            bg_cancel = 1;
            break;

        // case 'a':
        //     sdo_write8(ZeroErr_Driver_1, 0x4602, 0, 0x0);
        //     break;
        // case 'd':
        //     sdo_write8(ZeroErr_Driver_1, 0x4602, 0, 0x1);
        //     break;

        // 設定位置
        case 'w':
            driver_mode = Mode_Position;
            pos_target += max_speed;
            break;
        case 's':
            driver_mode = Mode_Position;
            pos_target = pos_feedback;
            break;
        case 'x':
            driver_mode = Mode_Position;
            pos_target -= max_speed;
            break;

        // 設定速度
        case 'e':
            max_speed += 10;
            break;
        case 'd':
            max_speed = 10;
            break;
        case 'c':
            max_speed -= 10;
            if(max_speed < 1)
                max_speed = 1;
            break;

        // 持續旋轉
        case 'r':
            driver_mode = Mode_Velocity;
            direct = 1;
            break;
        case 'f':
            driver_mode = Mode_Velocity;
            direct = 0;
            break;
        case 'v':
            driver_mode = Mode_Velocity;
            direct = -1;
            break;

        // servo
        case 'o':
        request_servo_on = TRUE;
        break;
        case 'p':
        request_servo_off = TRUE;
        break;

        default:
            break;
        }

        // printf("[keyboard] press (%c) (%d)\r\r\n", ch, ch);
        osal_usleep(10000); 
    }

    return;
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

void signal_init()
{
    // 攔截 ctrl + C 事件
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        printf("Failed to caught signal\r\n");
}

// int main(int argc, char *argv[])
int main(void)
{
    console("SOEM (Simple Open EtherCAT Master)");
    console("zero_err_driver start");
    /* create thread to handle slave error handling in OP */
    //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);

    set_latency_target(); // 消除系統時鐘的偏移

    osal_thread_create(&bg_keyboard, 2048, &keyboard, (void *)&ctime);
    usleep(100);

    osal_thread_create(&bg_ecatcheck, 128000, &ecatcheck, (void *)&ctime);
    usleep(100);

    signal_init(); // 攔截 ctrl + C 事件

    /* start cyclic part */
    usleep(100);
    simpletest(ETH_CH_NAME);

    console("End program");

    return (0);
}
