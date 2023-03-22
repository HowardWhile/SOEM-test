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
#define IPD_1 1
#define IPD_2 2
#define IPD_3 3
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
// 0x1600      "Output mapping 0"                            [RECORD  maxsub(0x19 / 25)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
//     0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x70000020 / 1879048224
//     0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x70010120 / 1879114016
//     0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x70010210 / 1879114256
//     0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x70010310 / 1879114512
//     0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x70010410 / 1879114768
//     0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x70010510 / 1879115024
//     0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x70010620 / 1879115296
//     0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x70010720 / 1879115552
//     0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x70010810 / 1879115792
//     0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x70010910 / 1879116048
//     0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x70010a10 / 1879116304
//     0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x70010b10 / 1879116560
//     0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x70010c20 / 1879116832
//     0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x70020120 / 1879179552
//     0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x70020210 / 1879179792
//     0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x70020310 / 1879180048
//     0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x70020410 / 1879180304
//     0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x70020510 / 1879180560
//     0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x70020620 / 1879180832
//     0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x70020720 / 1879181088
//     0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x70020810 / 1879181328
//     0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x70020910 / 1879181584
//     0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x70020a10 / 1879181840
//     0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x70020b10 / 1879182096
//     0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x70020c20 / 1879182368
//
// 0x7001      "IPDCmdData1"                                 [RECORD  maxsub(0x0c / 12)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12

//     0x01      "Axis1PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
//     0x02      "Axis1VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x03      "Axis1CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x04      "Axis1CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x05      "Axis1AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x06      "Axis1ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

//     0x07      "Axis2PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
//     0x08      "Axis2VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x09      "Axis2CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x0a      "Axis2CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x0b      "Axis2AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x0c      "Axis2ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

// 0x7002      "IPDCmdData2"                                 [RECORD  maxsub(0x0c / 12)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12

//     0x01      "Axis3PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
//     0x02      "Axis3VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x03      "Axis3CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x04      "Axis3CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x05      "Axis3AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x06      "Axis3ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

//     0x07      "Axis4PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
//     0x08      "Axis4VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x09      "Axis4CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
//     0x0a      "Axis4CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x0b      "Axis4AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
//     0x0c      "Axis4ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

typedef struct 
{
    int32_t  Axis1PosCmd;
    int16_t  Axis1VelCmd;
    int16_t  Axis1CurCmd;
    uint16_t Axis1CtlCmd;
    uint16_t Axis1AdrsCmd;
    int32_t  Axis1ParCmd;

    int32_t  Axis2PosCmd;
    int16_t  Axis2VelCmd;
    int16_t  Axis2CurCmd;
    uint16_t Axis2CtlCmd;
    uint16_t Axis2AdrsCmd;
    int32_t  Axis2ParCmd;

    int32_t  Axis3PosCmd;
    int16_t  Axis3VelCmd;
    int16_t  Axis3CurCmd;
    uint16_t Axis3CtlCmd;
    uint16_t Axis3AdrsCmd;
    int32_t  Axis3ParCmd;

    int32_t  Axis4PosCmd;
    int16_t  Axis4VelCmd;
    int16_t  Axis4CurCmd;
    uint16_t Axis4CtlCmd;
    uint16_t Axis4AdrsCmd;
    int32_t  Axis4ParCmd;
}Driver_Outputs;

//-----------------------------------
// Driver_Inputs
// 0x1a00      "Input mapping 0"                             [RECORD  maxsub(0x19 / 25)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
//     0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x60000020 / 1610612768
//     0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x60010120 / 1610678560
//     0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x60010210 / 1610678800
//     0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x60010310 / 1610679056
//     0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x60010410 / 1610679312
//     0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x60010510 / 1610679568
//     0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x60010620 / 1610679840
//     0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x60010720 / 1610680096
//     0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x60010810 / 1610680336
//     0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x60010910 / 1610680592
//     0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x60010a10 / 1610680848
//     0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x60010b10 / 1610681104
//     0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x60010c20 / 1610681376
//     0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x60020120 / 1610744096
//     0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x60020210 / 1610744336
//     0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x60020310 / 1610744592
//     0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x60020410 / 1610744848
//     0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x60020510 / 1610745104
//     0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x60020620 / 1610745376
//     0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x60020720 / 1610745632
//     0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x60020810 / 1610745872
//     0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x60020910 / 1610746128
//     0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x60020a10 / 1610746384
//     0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x60020b10 / 1610746640
//     0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x60020c20 / 1610746912

// 0x6000      "GPIO_INPUTS"                                 [VAR]
//     0x00      "GPIO_INPUTS"                                 [UNSIGNED32       R_R_R_]      0x00000000 / 0
// 0x6001      "IPDFbkData1"                                 [RECORD  maxsub(0x0c / 12)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12

//     0x01      "Axis1PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
//     0x02      "Axis1VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x03      "Axis1CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x04      "Axis1CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x05      "Axis1AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x06      "Axis1ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0

//     0x07      "Axi2PosFbk"                                  [INTEGER32        R_R_R_]      0x00000000 / 0
//     0x08      "Axis2VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x09      "Axis2CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x0a      "Axis2CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x0b      "Axis2AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x0c      "Axis2ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
// 0x6002      "IPDFbkData2"                                 [RECORD  maxsub(0x0c / 12)]
//     0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12

//     0x01      "Axis3PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
//     0x02      "Axis3VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x03      "Axis3CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x04      "Axis3CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x05      "Axis3AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x06      "Axis3ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0

//     0x07      "Axis4PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
//     0x08      "Axis4VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x09      "Axis4CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
//     0x0a      "Axis4CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x0b      "Axis4AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
//     0x0c      "Axis4ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0

typedef struct 
{
    int32_t  GPIO_INPUTS;

    int32_t  Axis1PosFbk;
    int16_t  Axis1VelFbk;
    int16_t  Axis1CurFbk;
    uint16_t Axis1CtlFbk;
    uint16_t Axis1AdrsFbk;
    int32_t  Axis1ParFbk;

    int32_t  Axis2PosFbk;
    int16_t  Axis2VelFbk;
    int16_t  Axis2CurFbk;
    uint16_t Axis2CtlFbk;
    uint16_t Axis2AdrsFbk;
    int32_t  Axis2ParFbk;

    int32_t  Axis3PosFbk;
    int16_t  Axis3VelFbk;
    int16_t  Axis3CurFbk;
    uint16_t Axis3CtlFbk;
    uint16_t Axis3AdrsFbk;
    int32_t  Axis3ParFbk;

    int32_t  Axis4PosFbk;
    int16_t  Axis4VelFbk;
    int16_t  Axis4CurFbk;
    uint16_t Axis4CtlFbk;
    uint16_t Axis4AdrsFbk;
    int32_t  Axis4ParFbk;
}Driver_Inputs;

//-----------------------------------
void print_driver_io(
    int32_t pos_cmd, int32_t pos_fbk,
    int16_t vel_cmd, int16_t vel_fbk,
    int16_t cur_cmd, int16_t cur_fbk,
    uint16_t ctl_cmd, uint16_t ctl_fbk,
    uint16_t adr_cmd, uint16_t adr_fbk,
    int32_t par_cmd, int32_t par_fbk)
{
    //console(" Pos Cmd: %d\tPos Fbk: %d", pos_cmd, pos_fbk);
    //console(" Vel Cmd: %d\tVel Fbk: %d", vel_cmd, vel_fbk);
    console(" Cur Cmd: %d\tCur Fbk: %d", cur_cmd, cur_fbk);
    //console(" Ctl Cmd: %d\tCtl Fbk: %d", ctl_cmd, ctl_fbk);
    console(" Adr Cmd: %d\tAdr Fbk: %d", adr_cmd, adr_fbk);
    console(" Par Cmd: %d\tPar Fbk: %d", par_cmd, par_fbk);

    return;
}
void print_driver_io_ptr(Driver_Inputs* in, Driver_Outputs* out)
{
    console(" Axis1");
    print_driver_io(
        out->Axis1PosCmd,  in->Axis1PosFbk,
        out->Axis1VelCmd,  in->Axis1VelFbk,
        out->Axis1CurCmd,  in->Axis1CurFbk,
        out->Axis1CtlCmd,  in->Axis1CtlFbk,
        out->Axis1AdrsCmd, in->Axis1AdrsFbk,
        out->Axis1ParCmd,  in->Axis1ParFbk );    

    console(" Axis2");
    print_driver_io(
        out->Axis2PosCmd,  in->Axis2PosFbk,
        out->Axis2VelCmd,  in->Axis2VelFbk,
        out->Axis2CurCmd,  in->Axis2CurFbk,
        out->Axis2CtlCmd,  in->Axis2CtlFbk,
        out->Axis2AdrsCmd, in->Axis2AdrsFbk,
        out->Axis2ParCmd,  in->Axis2ParFbk );  

    console(" Axis3");
    print_driver_io(
        out->Axis3PosCmd,  in->Axis3PosFbk,
        out->Axis3VelCmd,  in->Axis3VelFbk,
        out->Axis3CurCmd,  in->Axis3CurFbk,
        out->Axis3CtlCmd,  in->Axis3CtlFbk,
        out->Axis3AdrsCmd, in->Axis3AdrsFbk,
        out->Axis3ParCmd,  in->Axis3ParFbk );  

    console(" Axis4");
     print_driver_io(
        out->Axis4PosCmd,  in->Axis4PosFbk,
        out->Axis4VelCmd,  in->Axis4VelFbk,
        out->Axis4CurCmd,  in->Axis4CurFbk,
        out->Axis4CtlCmd,  in->Axis4CtlFbk,
        out->Axis4AdrsCmd, in->Axis4AdrsFbk,
        out->Axis4ParCmd,  in->Axis4ParFbk );         

}

// static int sdo_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
// {
//     return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(uint8), &value, EC_TIMEOUTRXM);
// }

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

int setupIPDDriver(uint16 slave)
{
    int wkc = 0;
    const int check_wkc = 2;
    console("[slave:%d] IPD Driver setup", slave);

    uint16 map_RxPDOassign[] = {0x0001, 0x1600}; // 0x1c12
    wkc += ec_SDOwrite(slave, 0x1c12, 0x00, TRUE, sizeof(map_RxPDOassign), &map_RxPDOassign, EC_TIMEOUTSAFE );

    uint16 map_TxPDOassign[] = {0x0001, 0x1A00}; // 0x1c13
    wkc += ec_SDOwrite(slave, 0x1c13, 0x00, TRUE, sizeof(map_TxPDOassign), &map_TxPDOassign, EC_TIMEOUTSAFE );


    if (wkc != check_wkc)
    {
        console("[slave:%d] IPD Driver setup "RED"failed."RESET" wkc: %d", slave, wkc);
        return -1;
    }
    else
    {
        console("[slave:%d] IPD Driver setup "LIGHT_GREEN"succeed."RESET, slave);
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

        // -------------------------------------
        // IPD pdo point mapping
        // -------------------------------------
        Driver_Inputs *iptr_ipd[3];
        iptr_ipd[0] = (Driver_Inputs*)ec_slave[IPD_1].inputs;
        iptr_ipd[1] = (Driver_Inputs*)ec_slave[IPD_2].inputs;
        iptr_ipd[2] = (Driver_Inputs*)ec_slave[IPD_3].inputs;

        Driver_Outputs *optr_ipd[3];
        optr_ipd[0] = (Driver_Outputs*)ec_slave[IPD_1].outputs;
        optr_ipd[1] = (Driver_Outputs*)ec_slave[IPD_2].outputs;
        optr_ipd[2] = (Driver_Outputs*)ec_slave[IPD_3].outputs;

        // -------------------------------------
        // renew inputs
        // -------------------------------------

        // -------------------------------------
        // logic
        // -------------------------------------      

        // -------------------------------------
        // update outputs
        // ------------------------------------

        // ------------------------------------
        // send ethercat cmd 
        // ------------------------------------
        ec_send_processdata();

        // ------------------------------------
        // latency 
        // ------------------------------------
        cyc_count++;
        sum_dt += dt;

        if (dt < min_dt)
            min_dt = dt;

        if (dt > max_dt)
            max_dt = dt;

        // ------------------------------------
        // 顯示
        // ------------------------------------
        EXEC_INTERVAL(100)        
        {
            for (size_t i = 0; i < 17; i++)
            {
                console("                                                       ");
            }
            MOVEUP(17);
            

            console("cyc_count: %ld, Latency:(min, max, avg)us = (%ld, %ld, %.2f) T:%ld+(%3ld)ns ****",
                    cyc_count,
                    min_dt / 1000, max_dt / 1000, (double)sum_dt / cyc_count / 1000,
                    ec_DCtime, toff);

            for (size_t idx = 0; idx < 1; idx++)
            {
                print_driver_io_ptr(iptr_ipd[idx], optr_ipd[idx]);
            }         

            fflush(stdout);
            MOVEUP(17);
        }
        EXEC_INTERVAL_END
    }
    
    MOVEDOWN(17);
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

            console("---- slave config ----");
            
            ec_slave[IPD_1].PO2SOconfig = setupIPDDriver;
            ec_slave[IPD_2].PO2SOconfig = setupIPDDriver;
            ec_slave[IPD_3].PO2SOconfig = setupIPDDriver;
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
            //int idx = ch - '0';
            //DO[idx] = !DO[idx];
            //CtrlWord[idx] = !CtrlWord[idx];

        }

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
