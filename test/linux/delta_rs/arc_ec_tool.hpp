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

static inline int drive_read8(uint16 slave, uint16 index, uint8 subindex, uint8 &o_value)
{
    uint8 result;
    int result_len = sizeof(result);
    int ret = ec_SDOread(slave, index, subindex, FALSE, &result_len, &result, EC_TIMEOUTRXM);
    if (ret > 0) {
        o_value = result;
        return 0; // Success
    } else {
        return ret; // Error occurred
    }
}

static inline int drive_read16(uint16 slave, uint16 index, uint8 subindex, uint16 &o_value)
{
    uint16 result;
    int result_len = sizeof(result);
    int ret = ec_SDOread(slave, index, subindex, FALSE, &result_len, &result, EC_TIMEOUTRXM);
    if (ret > 0) {
        o_value = result;
        return 0; // Success
    } else {
        return ret; // Error occurred
    }
}

static inline int drive_read32(uint16 slave, uint16 index, uint8 subindex, uint32 &o_value)
{
    uint32 result;
    int result_len = sizeof(result);
    int ret = ec_SDOread(slave, index, subindex, FALSE, &result_len, &result, EC_TIMEOUTRXM);
    if (ret > 0) {
        o_value = result;
        return 0; // Success
    } else {
        return ret; // Error occurred
    }
}


#endif