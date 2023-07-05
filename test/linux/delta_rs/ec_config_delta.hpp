#ifndef _EC_CONFIG_DELTA_H_
#define _EC_CONFIG_DELTA_H_

#include "arc_ec_tool.hpp"
#include "arc_console.hpp"

typedef struct
{

} Delta_ASDA_I3_E_1st_TxPDO_Mapping_t; // 0x1A00
typedef struct
{
    uint16_t StatusWord;            // 0x6041
    int32_t ActualPosition;         // 0x6064
    int32_t VelocityActualValue;    // 0x606c
    int16_t ActualTorque;           // 0x6077
    uint16_t TouchProbeStatus;      // 0x60b9
    int32_t TouchProbePos1PosValue; // 0x60ba
    uint32_t DigitalInputs;         // 0x60fd

} Delta_ASDA_I3_E_2nd_TxPDO_Mapping_t; // 0x1A01
typedef struct
{

} Delta_ASDA_I3_E_3rd_TxPDO_Mapping_t; // 0x1A02
typedef struct
{

} Delta_ASDA_I3_E_4th_TxPDO_Mapping_t; // 0x1A03

typedef struct
{

}Delta_ASDA_I3_E_1st_RxPDO_Mapping_t; // 0x1600
typedef struct
{
    uint16_t ControlWord;        // 0x6040
    int32_t TargetPosition;      // 0x607A
    int32_t TargetVelocity;      // 0x60FF
    int16_t TargetTorque;        // 0x6071
    uint16_t TouchProbeFunction; // 0x60B8

} Delta_ASDA_I3_E_2nd_RxPDO_Mapping_t; // 0x1601
typedef struct
{

}Delta_ASDA_I3_E_3rd_RxPDO_Mapping_t;// 0x1602
typedef struct
{

}Delta_ASDA_I3_E_4th_RxPDO_Mapping_t;// 0x1603


static int setupDelta_ASDA_I3_E(uint16 slave)
{
    console_tag("Delta_ASDA_I3_E", "setup slave(%d)", slave);

    const int vandor_id = 0x1DD;
    const int product_code = 0x10306081;

    if (ec_slave[slave].eep_man != vandor_id ||
        ec_slave[slave].eep_id != product_code)
    {
        console_tag("Delta_ASDA_I3_E", RED "slave(%d) product not compatible" RESET, slave);
        //return -1;
    }

    const int excepted_wkc = 22; // 預期寫入
    int wkc = 0;

    // 設定為0x0 停用配置
    wkc += drive_write8(slave, 0x1C12, 0, 0);
    wkc += drive_write8(slave, 0x1C13, 0, 0);

    // 0x1A00
    // ...

    // 0x1A01
    // 0x1a01      "�"                                           [RECORD  maxsub(0x08 / 8)]
    //     0x00      "SubIndex 000"                                [UNSIGNED8        RWR_R_]      0x07 / 7
    //     0x01      "�"                                           [UNSIGNED32       RWR_R_]      0x60410010 / 1614872592
    //     0x02      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60640020 / 1617166368
    //     0x03      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x606c0020 / 1617690656
    //     0x04      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60770010 / 1618411536
    //     0x05      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60b90010 / 1622736912
    //     0x06      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60ba0020 / 1622802464
    //     0x07      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60fd0020 / 1627193376

    wkc += drive_write8(slave, 0x1A01, 0, 0); // 設定為0x0停用PDO映射設定

    // 設定mapping entry
    wkc += drive_write32(slave, 0x1A01, 1, 0x60410010); // Status Word
    wkc += drive_write32(slave, 0x1A01, 2, 0x60640020); // Actual Position
    wkc += drive_write32(slave, 0x1A01, 3, 0x606c0020); // Velocity actual value
    wkc += drive_write32(slave, 0x1A01, 4, 0x60770010); // Actual Torque
    wkc += drive_write32(slave, 0x1A01, 5, 0x60b90010); // Touch Probe Status
    wkc += drive_write32(slave, 0x1A01, 6, 0x60ba0020); // Touch Probe Pos1 Pos Value
    wkc += drive_write32(slave, 0x1A01, 7, 0x60fd0020); // Digital inputs

    wkc += drive_write8(slave, 0x1A01, 0, 7); // 設定PDO映射中的映射數

    // 0x1A02
    // ...

    // 0x1A03
    // ...

    // 0x1600
    // ...

    // 0x1601      "�"                                           [RECORD  maxsub(0x08 / 8)]
    //     0x00      "SubIndex 000"                                [UNSIGNED8        RWR_R_]      0x05 / 5
    //     0x01      "�"                                           [UNSIGNED32       RWR_R_]      0x60400010 / 1614807056
    //     0x02      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x607a0020 / 1618608160
    //     0x03      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60ff0020 / 1627324448
    //     0x04      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60710010 / 1618018320
    //     0x05      "Swne 0"                                      [UNSIGNED32       RWR_R_]      0x60b80010 / 1622671376
    wkc += drive_write8(slave, 0x1601, 0, 0); // 設定為0x0停用PDO映射設定

    // 設定mapping entry
    wkc += drive_write32(slave, 0x1601, 1, 0x60400010); // Controlword
    wkc += drive_write32(slave, 0x1601, 2, 0x607a0020); // Target position
    wkc += drive_write32(slave, 0x1601, 3, 0x60ff0020); // Target velocity
    wkc += drive_write32(slave, 0x1601, 4, 0x60710010); // Target torque
    wkc += drive_write32(slave, 0x1601, 5, 0x60b80010); // Touch Probe Function

    wkc += drive_write8(slave, 0x1601, 0, 5); // 設定PDO映射中的映射數

    // 0x1602
    // ...

    // 0x1603
    // ...

    // 0x1604
    // ...

    // 設定為所選的PDO映射配置
    wkc += drive_write16(slave, 0x1C12, 1, 0x1601);
    wkc += drive_write16(slave, 0x1C13, 1, 0x1A01);

    // 設定為0x1啟用PDO配置
    wkc += drive_write8(slave, 0x1C12, 0, 1);
    wkc += drive_write8(slave, 0x1C13, 0, 1);

    // ref: ASDA A2 5.3.3, ASDA A3 13.3.5    
    // 設定週期同步位置模式 0x8
    wkc += drive_write8(slave, 0x6060, 0, 8);


    if (wkc != excepted_wkc)
    {
        console_tag("Delta_ASDA_I3_E", RED "slave(%d) config failed wkc(%d) != %d" RESET, slave, wkc, excepted_wkc);
        return -1;
    }
    else
        console_tag("Delta_ASDA_I3_E", "slave(%d) config " LIGHT_GREEN "succeed" RESET, slave);

    return 0;
}

#endif