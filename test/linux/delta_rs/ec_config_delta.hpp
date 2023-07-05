#ifndef _EC_CONFIG_DELTA_H_
#define _EC_CONFIG_DELTA_H_

#include "arc_ec_tool.hpp"
#include "arc_console.hpp"


static int setupDelta_ASDA_I3_E(uint16 slave)
{
    console_tag("Delta_ASDA_I3_E", "setup slave(%d)", slave);

    const int vandor_id = 0x1DD;
    const int producr_code = 0x1DD;

    // check Vendor ID and Product Code





    return 0;
}

#endif