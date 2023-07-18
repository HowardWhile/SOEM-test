#ifndef arc_ipc_tool_h__
#define arc_ipc_tool_h__

#include "arc_console.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdint.h>

typedef struct
{
    int32_t ActualPosition[6]; 
    int32_t ActualVelocity[6]; 
    int16_t ActualTorque[6];   
} Share_Memeber_t;

static int inline createShareMem()
{
    int shmId = shmget(IPC_PRIVATE, sizeof(Share_Memeber_t), IPC_CREAT | 0666);
    if (shmId == -1)
    {
        console("Failed to create shared memory");
        return -1;
    }
    return shmId;
}

static int inline getShareMem()
{
    int shmId = shmget(IPC_PRIVATE, sizeof(Share_Memeber_t), 0666);
    if (shmId == -1)
    {
        console("Failed to get shared memory");
        return -1;
    }
    return shmId;
}

static int inline attachShareMem(int id, Share_Memeber_t **o_mem)
{
    Share_Memeber_t *mem = (Share_Memeber_t *)shmat(id, NULL, 0);
    if (mem == (Share_Memeber_t *)-1)
    {
        console("Failed to attach shared memory");
        return -1;
    }
    *o_mem = mem;
    return 0;
}

#endif