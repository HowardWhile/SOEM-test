#ifndef arc_ec_tool_h__
#define arc_ec_tool_h__

#include "ethercat.h"

static inline int drive_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(value), &value, EC_TIMEOUTRXM);
}

static inline int drive_write16(uint16 slave, uint16 index, uint8 subindex, uint16 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(value), &value, EC_TIMEOUTRXM);
}

static inline int drive_write32(uint16 slave, uint16 index, uint8 subindex, int32 value)
{
    return ec_SDOwrite(slave, index, subindex, FALSE, sizeof(value), &value, EC_TIMEOUTRXM);
}

#endif