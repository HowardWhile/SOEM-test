#ifndef arc_ipc_tool_h__
#define arc_ipc_tool_h__

#include "arc_console.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdint.h>

typedef struct
{
    int64_t feedback_update_time_ns;
    int64_t feedback_update_time_s;
    int32_t ActualPosition[6]; 
    int32_t ActualVelocity[6]; 
    int16_t ActualTorque[6];

    int64_t control_update_time_ms;
    char control_key;

} Share_Memeber_t;

static int inline createShareMem(key_t share_key)
{
    int shmId = shmget(share_key, sizeof(Share_Memeber_t), IPC_CREAT | 0666);
    if (shmId == -1)
    {
        console("Failed to create shared memory");
        return -1;
    }
    return shmId;
}

static int inline getShareMem(key_t share_key)
{
    int shmId = shmget(share_key, sizeof(Share_Memeber_t), 0666);
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

static int inline detachShareMem(Share_Memeber_t *mem)
{
    if (shmdt(mem) == -1)
    {
        console("Failed to detach shared memory");
        return -1;
    }
    return 0;
}

static int inline removeShareMem(int share_mem_id)
{
    if (shmctl(share_mem_id, IPC_RMID, NULL) == -1)
    {
        console("Failed to remove shared memory");
        return -1;
    }
    return 0;
}

#endif