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

#include <curses.h>

#include "ethercat.h"
#include "arc_console.hpp"

#define EC_TIMEOUTMON 500

// 通訊用的eth設備名稱
#define ETH_CH_NAME "eno1"

// Slave的站號
#define EC_SLAVE_ID 1

// RT Loop的週期
#define PERIOD_NS (1000000)
#define NSEC_PER_SEC (1000000000)

boolean bg_cancel = 0;
OSAL_THREAD_HANDLE thread1;
OSAL_THREAD_HANDLE thread2;

boolean dynamicY = FALSE;

char IOmap[4096];
boolean DO[32];

int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

static int sdo_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(uint8), &value, EC_TIMEOUTRXM);
}

int setupDeltaIO(void)

{
    int slave = EC_SLAVE_ID;
    int wkc = 0;
    printf("[slave:%d] DELTA RC-EC0902 setup\r\n", slave);

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
    wkc += sdo_write8(EC_SLAVE_ID, 0x2001, 1, 0xFF);
    wkc += sdo_write8(EC_SLAVE_ID, 0x2001, 2, 0xFF);
    wkc += sdo_write8(EC_SLAVE_ID, 0x2001, 3, 0xFF);
    wkc += sdo_write8(EC_SLAVE_ID, 0x2001, 4, 0xFF);

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
    wkc += sdo_write8(EC_SLAVE_ID, 0x6206, 1, 0x0);
    wkc += sdo_write8(EC_SLAVE_ID, 0x6206, 2, 0x0);
    wkc += sdo_write8(EC_SLAVE_ID, 0x6206, 3, 0x0);
    wkc += sdo_write8(EC_SLAVE_ID, 0x6206, 4, 0x0);

    // strncpy(ec_slave[slave].name, "IO", EC_MAXNAME);

    if (wkc != 8)
    {
        printf("[slave:%d] setup failed\nwkc: %d\n", slave, wkc);
        return -1;
    }
    else
    {
        printf("[slave:%d] DELTA RC-EC0902 setup succeed.\r\n", slave);
        return 0;
    }
}

void modifyBit(uint8 *value, int p, boolean bit)
{
    int mask = 1 << p;
    *value = ((*value & ~mask) | (bit << p));
}

void cyclic_task()
{
    static int cyc_count = 0;

    // printf("[%ld]\r\n",clock_ms());

    if (dynamicY && cyc_count % 100 == 0)
    {
        for (size_t idx = 0; idx < 32; idx++)
        {
            DO[idx] = !DO[idx];
        }
    }

    for (size_t idx_bit = 0; idx_bit < 8; idx_bit++)
    {
        modifyBit(&ec_slave[EC_SLAVE_ID].outputs[0], idx_bit, DO[0 + idx_bit]);
        modifyBit(&ec_slave[EC_SLAVE_ID].outputs[1], idx_bit, DO[8 + idx_bit]);
        modifyBit(&ec_slave[EC_SLAVE_ID].outputs[2], idx_bit, DO[16 + idx_bit]);
        modifyBit(&ec_slave[EC_SLAVE_ID].outputs[3], idx_bit, DO[24 + idx_bit]);
    }

    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
    cyc_count++;

    //
    if (wkc >= expectedWKC)
    {
        // printf("Processdata cycle %4d, WKC %d , O:", cyc_count++, wkc);

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

void simpletest(char *ifname)
{
    int i, oloop, iloop, chk;
    needlf = FALSE;
    inOP = FALSE;

    printf("Starting simple test\n");
    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n", ifname);
        /* find and auto-config slaves */
        if (ec_config_init(FALSE) > 0)
        {
            printf("%d slaves found and configured.\n", ec_slavecount);

            while (setupDeltaIO())
                usleep(100);

            memset(DO, 0, sizeof(DO));

            ec_config_map(&IOmap);
            ec_configdc();

            printf("Slaves mapped, state to SAFE_OP.\n");
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

            printf("segments : %d : %d %d %d %d\n", ec_group[0].nsegments, ec_group[0].IOsegment[0], ec_group[0].IOsegment[1], ec_group[0].IOsegment[2], ec_group[0].IOsegment[3]);

            printf("Request operational state for all slaves\n");
            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* request OP state for all slaves */
            ec_writestate(0);

            chk = 200;
            /* wait for all slaves to reach OP state */
            do
            {
                ec_send_processdata();
                ec_receive_processdata(EC_TIMEOUTRET);
                ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
            } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

            if (ec_slave[0].state == EC_STATE_OPERATIONAL)
            {
                printf("Operational state reached for all slaves.\n");
                inOP = TRUE;

                // ----------------------------------------------------
                // 設定程式的兩種優先權
                // ----------------------------------------------------
                // Set PR (Priority) to RT(-99) 最高優先權
                printf("Set priority ...\n");
                struct sched_param param = {};
                param.sched_priority = sched_get_priority_max(SCHED_FIFO);
                printf("Using priority %i.", param.sched_priority);
                if (sched_setscheduler(0, SCHED_FIFO, &param) == -1)
                    printf("sched_setscheduler failed\r\n"); // 錯誤還是可以跑

                // Set NI (Niceness) to -19 最高優先權
                pid_t pid = getpid(); // 獲得進程PID
                printf("PID = %d\r\n", pid);
                if (setpriority(PRIO_PROCESS, pid, -19))                              // 設置進程優先順序
                    printf("Warning: Failed to set priority: %s\n", strerror(errno)); // 錯誤還是可以跑
                // ----------------------------------------------------
                // real-time 定時器
                // ----------------------------------------------------
                printf("[%ld ms] Starting RT task with dt=%u ns.\n", clock_ms(), PERIOD_NS);
                struct timespec wakeup_time;
                if (clock_gettime(CLOCK_REALTIME, &wakeup_time) == -1) // 當前的精準時間
                {
                    printf("clock_gettime failed\n");
                    return;
                }
                // wakeup_time.tv_sec += 1; /* start in future */
                // wakeup_time.tv_nsec = 0;
                while (!bg_cancel)
                {
                    // sleep直到指定的時間點
                    int ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &wakeup_time, NULL);
                    if (ret)
                    {
                        // sleep錯誤處理
                        printf("clock_nanosleep(): %s\n", strerror(ret));
                        if (ret == EINTR)
                            printf("Interrupted by signal handler\n");
                        else
                            printf("clock_nanosleep");
                        break;
                    }

                    // --------------------------------------------
                    console_fps("cyclic_task");
                    cyclic_task();
                    // --------------------------------------------

                    // 指定下次睡醒的時間點
                    wakeup_time.tv_nsec += PERIOD_NS;
                    while (wakeup_time.tv_nsec >= NSEC_PER_SEC)
                    {
                        wakeup_time.tv_nsec -= NSEC_PER_SEC;
                        wakeup_time.tv_sec++;
                    }
                }
                inOP = FALSE;
            }
            else
            {
                printf("Not all slaves reached operational state.\n");
                ec_readstate();
                for (i = 1; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_OPERATIONAL)
                    {
                        printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n",
                               i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                    }
                }
            }
            printf("\nRequest init state for all slaves\n");
            ec_slave[0].state = EC_STATE_INIT;
            /* request INIT state for all slaves */
            ec_writestate(0);
        }
        else
        {
            printf("No slaves found!\n");
        }
        printf("End simple test, close socket\n");
        /* stop SOEM, close socket */
        ec_close();
    }
    else
    {
        printf("No socket connection on %s\nExcecute as root\n", ifname);
    }
}

OSAL_THREAD_FUNC ecatcheck(void *ptr)
{
    int slave;
    (void)ptr; /* Not used */

    while (!bg_cancel)
    {
        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
                needlf = FALSE;
                printf("\n");
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
        osal_usleep(10000);
    }

    printf("exit ecatcheck\r\n");
}

OSAL_THREAD_FUNC keyboard(void *ptr)
{
    (void)ptr; /* Not used */

    int ch;
    initscr();
    noecho();

    while (ch != 'q')
    {
        ch = getch();
        switch (ch)
        {
        case '0':

            dynamicY = !dynamicY;
            break;
        case '1':
            DO[1] = !DO[1];
            break;
        case '2':
            DO[2] = !DO[2];
            break;
        case '3':
            DO[3] = !DO[3];
            break;
        case '4':
            DO[4] = !DO[4];
            break;

        default:
            break;
        }

        // printf("[keyboard] press (%c)(%d)\r\n", ch, ch);
    }
    endwin();
    bg_cancel = 1;
    return;
}

void signal_handler(int signum)
{
    // 用來攔截 Ctrl + C 訊號
    printf("signal_handler: caught signal %d\n", signum);
    if (signum == SIGINT)
    {
        printf("[ctrl + c] ethercat release\n");
        bg_cancel = 1;
    }
}

// int main(int argc, char *argv[])
int main(void)
{

    printf("SOEM (Simple Open EtherCAT Master)\ndelta io\n");
    /* create thread to handle slave error handling in OP */
    //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);

    osal_thread_create(&thread1, 128000, &ecatcheck, (void *)&ctime);
    osal_usleep(10000);

    osal_thread_create(&thread2, 128000, &keyboard, (void *)&ctime);
    osal_usleep(10000);

    // 攔截 ctrl + C 事件
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        printf("Failed to caught signal\n");

    /* start cyclic part */
    simpletest(ETH_CH_NAME);

    printf("End program\n");
    return (0);
}
