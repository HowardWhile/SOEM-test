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

// 通訊用的eth設備名稱
#define ETH_CH_NAME "eno1"

// Slave的站號
#define EC_SLAVE_ID_DELTA_IO 1

// 指定運行的CPU編號
#define CPU_ID 7

// RT Loop的週期
#define PERIOD_NS (1 * 1000 * 1000)

boolean bg_cancel = 0;
OSAL_THREAD_HANDLE bg_ecatcheck;
OSAL_THREAD_HANDLE bg_keyboard;

boolean dynamicY = FALSE;

char IOmap[4096];
boolean DO[32];

int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

int64 last_cktime = 0;
int64 max_dt = LLONG_MIN;
int64 min_dt = LLONG_MAX;
int64 sum_dt = 0;
int64 cyc_count = 0;

static int sdo_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(uint8), &value, EC_TIMEOUTRXM);
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
    //printf("[setPRICPUx] Priority= %d, cpu_id= %d\r\n", Priority, cpu_id);

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

int setupDeltaIO(int slave)
{
    int wkc = 0;
    console("[slave:%d] DELTA RC-EC0902 setup", slave);

    // Active all DO port ----------------------------------------------------------
    // 此物件可以設定輸出通道是否允許變更(8 個輸出通道為一組)。0 代表不允許改變狀態，1 代表允許改變狀態。
    //    Index: 2001 Datatype: 002a Objectcode: 09 Name: Active DO Enable
    //    Sub: 00 Datatype: 0005 Bitlength: 0008 Obj.access: 0007 Name: SubIndex 000
    //             Value :0x04 4
    //    Sub: 01 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Active Port2 DO CH0~7 Enable
    //             Value :0x00 0
    //    Sub: 02 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Active Port2 DO CH8~15 Enable
    //             Value :0x00 0
    //    Sub: 03 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Active Port3 DO CH0~7 Enable
    //             Value :0x00 0
    //    Sub: 04 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Active Port3 DO CH8~15 Enable
    //             Value :0x00 0
    wkc += sdo_write8(slave, 0x2001, 1, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 2, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 3, 0xFF);
    wkc += sdo_write8(slave, 0x2001, 4, 0xFF);

    // Error Mode disable ----------------------------------------------------------
    // 0 代表維持原本輸出值，1 代表參考Error Mode Output Value(6207h)的設定值。
    //    Index: 6206 Datatype: 002a Objectcode: 09 Name: DO Error Mode Enable
    //    Sub: 00 Datatype: 0005 Bitlength: 0008 Obj.access: 0007 Name: SubIndex 000
    //             Value :0x04 4
    //    Sub: 01 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Port2 DO Ch0~7 Error Mode Enable
    //             Value :0xff 255
    //    Sub: 02 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Port2 DO Ch8~15 Error Mode Enable
    //             Value :0xff 255
    //    Sub: 03 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Port3 DO Ch0~7 Error Mode Enable
    //             Value :0xff 255
    //    Sub: 04 Datatype: 0005 Bitlength: 0008 Obj.access: 003f Name: Port3 DO Ch8~15 Error Mode Enable
    //             Value :0xff 255
    wkc += sdo_write8(slave, 0x6206, 1, 0x0);
    wkc += sdo_write8(slave, 0x6206, 2, 0x0);
    wkc += sdo_write8(slave, 0x6206, 3, 0x0);
    wkc += sdo_write8(slave, 0x6206, 4, 0x0);

    // strncpy(ec_slave[slave].name, "IO", EC_MAXNAME);

    if (wkc != 8)
    {
        console("[slave:%d] setup failed\r\nwkc: %d", slave, wkc);
        return -1;
    }
    else
    {
        console("[slave:%d] DELTA RC-EC0902 setup succeed.", slave);
        return 0;
    }
}

void modifyBit(uint8 *value, int p, boolean bit)
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
    console("[%ld ms] Starting RT task with dt=%u ns.", clock_ms(), PERIOD_NS);
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

void cyclic_task()
{
    // ----------------------------------------------------
    // real-time 定時器
    // ----------------------------------------------------
    printf("[%ld ms] Starting RT task with dt=%u ns.\r\n", clock_ms(), PERIOD_NS);
    const int64 cycletime = PERIOD_NS; /* cycletime in ns */

    struct timespec wakeup_time;
    if (clock_gettime(CLOCK_MONOTONIC, &wakeup_time) == -1) // 當前的精準時間
    {
        printf("clock_gettime failed\r\n");
        return;
    }

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

        // 開關所有的Y
        if (dynamicY && cyc_count % 100 == 0)
        {
            for (size_t idx = 0; idx < 32; idx++)
            {
                DO[idx] = !DO[idx];
            }
        }

        // ------------------------------------
        // update output
        // ------------------------------------
        for (size_t idx_bit = 0; idx_bit < 8; idx_bit++)
        {
            modifyBit(&ec_slave[EC_SLAVE_ID_DELTA_IO].outputs[0], idx_bit, DO[0 * 8 + idx_bit]);
            modifyBit(&ec_slave[EC_SLAVE_ID_DELTA_IO].outputs[1], idx_bit, DO[1 * 8 + idx_bit]);
            modifyBit(&ec_slave[EC_SLAVE_ID_DELTA_IO].outputs[2], idx_bit, DO[2 * 8 + idx_bit]);
            modifyBit(&ec_slave[EC_SLAVE_ID_DELTA_IO].outputs[3], idx_bit, DO[3 * 8 + idx_bit]);
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
            consoler("cyc_count: %ld, Latency:(min, max, avg)us = (%ld, %ld, %.2f) T:%ld+(%3ld)ns ****",
                     cyc_count,
                     min_dt / 1000, max_dt / 1000, (double)sum_dt / cyc_count / 1000,
                     ec_DCtime, toff);
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
            // needlf = TRUE;
        }
    }
}

void simpletest(char *ifname)
{
    int i, oloop, iloop;
    needlf = FALSE;
    inOP = FALSE;

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
            console("%d slaves found and configured.\r\n", ec_slavecount);

            while (setupDeltaIO(EC_SLAVE_ID_DELTA_IO))
                usleep(100);

            memset(DO, 0, sizeof(DO));

            ec_config_map(&IOmap);
            ec_configdc();

            printf("Slaves mapped, state to SAFE_OP.\r\n");
            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

            oloop = ec_slave[0].Obytes;
            if ((oloop == 0) && (ec_slave[0].Obits > 0))
                oloop = 1;
            if (oloop > 8)
                oloop = 8;
            iloop = ec_slave[0].Ibytes;
            if ((iloop == 0) && (ec_slave[0].Ibits > 0))
                iloop = 1;
            if (iloop > 8)
                iloop = 8;

            printf("segments : %d : %d %d %d %d\r\n", ec_group[0].nsegments, ec_group[0].IOsegment[0], ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);

            printf("Request operational state for all slaves\r\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\r\n", expectedWKC);
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* request OP state for all slaves */            
            ec_writestate(0);

            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* wait for all slaves to reach OP state */
            consoler("wait for all slaves to reach OP state");
            int64 ck_time = clock_ms();
            int64 k_timeout = 3000; // 3s;
            while ( !bg_cancel && (ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTRET) != EC_STATE_OPERATIONAL) )
            {
                int64 dt = clock_ms() - ck_time;
                if(dt > k_timeout)
                {
                    printf(RED"Timeout\r\n"RESET);
                    break;
                }
                consoler("wait for all slaves to reach OP state (%.1fs)...", (float32)(k_timeout - dt)/1000 );

            }
            printf("\r\n");

            // do
            // {
            //     ec_writestate(0);
            //     ec_send_processdata();
            //     ec_receive_processdata(EC_TIMEOUTRET);
            //     ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTRET*10);

            //     int64 dt = clock_ms() - ck_time;
            //     consoler("wait for all slaves to reach OP state %3ld ...", clock_ms() - ck_time );
            //     if(dt > k_timeout)
            //     {
            //         printf("timeout\r\n");
            //         break;
            //     }
            // } while (ec_slave[0].state != EC_STATE_OPERATIONAL && !bg_cancel);

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
                        console("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s",
                               i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
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
            DO[idx] = !DO[idx];
        }

        switch (ch)
        {
        case ' ':
            dynamicY = !dynamicY;
            break;
        case 'r':
            max_dt = LLONG_MIN;
            min_dt = LLONG_MAX;
            sum_dt = 0;
            cyc_count = 0;
            break;
        case 'q':
            printf("\r\n");
            bg_cancel = 1;
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


    printf("%s\r\n", clock_now());


    set_latency_target(); // 消除系统时钟偏移

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
