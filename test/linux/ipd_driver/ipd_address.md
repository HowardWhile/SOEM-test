SOEM (Simple Open EtherCAT Master)
Slaveinfo
Starting slaveinfo
ec_init on enp2s0 succeeded.
3 slaves found and configured.
Calculated workcounter 9

Slave:1
 Name:IPD EtherCAT Slave
 Output size: 544bits
 Input size: 544bits
 State: 4
 Delay: 0[ns]
 Has DC: 1
 DCParentport:0
 Activeports:1.1.0.0
 Configured address: 1001
 Man: 0000071a ID: 00009252 Rev: 00000001
 SM0 A:1000 L: 128 F:00010026 Type:1
 SM1 A:1080 L: 128 F:00010022 Type:2
 SM2 A:1100 L:  68 F:00010064 Type:3
 SM3 A:1400 L:  68 F:00010020 Type:4
 FMMU0 Ls:00000000 Ll:  68 Lsb:0 Leb:7 Ps:1100 Psb:0 Ty:02 Act:01
 FMMU1 Ls:000000cc Ll:  68 Lsb:0 Leb:7 Ps:1400 Psb:0 Ty:01 Act:01
 FMMUfunc 0:1 1:2 2:3 3:0
 MBX length wr: 128 rd: 128 MBX protocols : 0e
 CoE details: 23 FoE details: 01 EoE details: 01 SoE details: 00
 Ebus current: 0[mA]
 only LRD/LWR:0
 CoE Object Description found, 20 entries.
0x1000      "Device type"                                 [VAR]
    0x00      "Device type"                                 [UNSIGNED32       R_R_R_]      0x00000192 / 402
0x1001      "Error register"                              [VAR]
    0x00      "Error register"                              [UNSIGNED8        R_R_R_]      0x00 / 0
0x1008      "Device name"                                 [VAR]
    0x00      "Device name"                                 [VISIBLE_STR(144) R_R_R_]      "IPD EtherCAT Slave"
0x1009      "Hardware version"                            [VAR]
    0x00      "Hardware version"                            [VISIBLE_STR(24)  R_R_R_]      "0.7"
0x100a      "Software version"                            [VAR]
    0x00      "Software version"                            [VISIBLE_STR(24)  R_R_R_]      "1.1"
0x1018      "Identity"                                    [RECORD  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "Vendor ID"                                   [UNSIGNED32       R_R_R_]      0x0000071a / 1818
    0x02      "Product code"                                [UNSIGNED32       R_R_R_]      0x00009252 / 37458
    0x03      "Revision"                                    [UNSIGNED32       R_R_R_]      0x00000001 / 1
    0x04      "Serial number"                               [UNSIGNED32       R_R_R_]      0x00000001 / 1
0x10f1      "Error Settings"                              [RECORD  maxsub(0x02 / 2)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x01      "Local Error Reaction"                        [UNSIGNED32       RWRWRW]      0x00000001 / 1
    0x02      "Sync Error Counter Limit"                    [UNSIGNED16       RWRWRW]      0x0004 / 4
0x1600      "Output mapping 0"                            [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x70000020 / 1879048224
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x70010120 / 1879114016
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x70010210 / 1879114256
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x70010310 / 1879114512
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x70010410 / 1879114768
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x70010510 / 1879115024
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x70010620 / 1879115296
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x70010720 / 1879115552
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x70010810 / 1879115792
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x70010910 / 1879116048
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x70010a10 / 1879116304
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x70010b10 / 1879116560
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x70010c20 / 1879116832
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x70020120 / 1879179552
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x70020210 / 1879179792
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x70020310 / 1879180048
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x70020410 / 1879180304
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x70020510 / 1879180560
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x70020620 / 1879180832
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x70020720 / 1879181088
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x70020810 / 1879181328
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x70020910 / 1879181584
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x70020a10 / 1879181840
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x70020b10 / 1879182096
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x70020c20 / 1879182368
0x1a00      "Input mapping 0"                             [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x60000020 / 1610612768
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x60010120 / 1610678560
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x60010210 / 1610678800
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x60010310 / 1610679056
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x60010410 / 1610679312
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x60010510 / 1610679568
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x60010620 / 1610679840
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x60010720 / 1610680096
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x60010810 / 1610680336
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x60010910 / 1610680592
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x60010a10 / 1610680848
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x60010b10 / 1610681104
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x60010c20 / 1610681376
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x60020120 / 1610744096
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x60020210 / 1610744336
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x60020310 / 1610744592
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x60020410 / 1610744848
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x60020510 / 1610745104
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x60020620 / 1610745376
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x60020720 / 1610745632
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x60020810 / 1610745872
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x60020910 / 1610746128
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x60020a10 / 1610746384
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x60020b10 / 1610746640
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x60020c20 / 1610746912
0x1c00      "Sync manager type"                           [ARRAY  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "SubIndex 001"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x02      "SubIndex 002"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x03      "SubIndex 003"                                [UNSIGNED8        R_R_R_]      0x03 / 3
    0x04      "SubIndex 004"                                [UNSIGNED8        R_R_R_]      0x04 / 4
0x1c12      "SyncManager 2 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1600 / 5632
0x1c13      "SyncManager 3 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1a00 / 6656
0x1c32      "SM output parameter"                         [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0001 / 1
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001dbbcf / 1948623
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0001 / 1
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x09f8 / 2552
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x1c33      "SM input parameter"                          [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0022 / 34
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001dbbcf / 1948623
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x09f8 / 2552
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x6000      "GPIO_INPUTS"                                 [VAR]
    0x00      "GPIO_INPUTS"                                 [UNSIGNED32       R_R_R_]      0x00000000 / 0
0x6001      "IPDFbkData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis1VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis1CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis1CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis1AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis1ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axi2PosFbk"                                  [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis2VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis2CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis2CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis2AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis2ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x6002      "IPDFbkData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis3VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis3CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis3CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis3AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis3ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axis4PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis4VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis4CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis4CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis4AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis4ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x7000      "GPIO_OUTPUTS"                                [VAR]
    0x00      "GPIO_OUTPUTS"                                [UNSIGNED32       RWRWRW]      0x00000000 / 0
0x7001      "IPDCmdData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis1VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis1CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis1CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis1AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis1ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis2PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis2VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis2CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis2CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis2AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis2ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
0x7002      "IPDCmdData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis3VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis3CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis3CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis3AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis3ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis4PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis4VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis4CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis4CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis4AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis4ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

Slave:2
 Name:IPD EtherCAT Slave
 Output size: 544bits
 Input size: 544bits
 State: 4
 Delay: 720[ns]
 Has DC: 1
 DCParentport:1
 Activeports:1.1.0.0
 Configured address: 1002
 Man: 0000071a ID: 00009252 Rev: 00000001
 SM0 A:1000 L: 128 F:00010026 Type:1
 SM1 A:1080 L: 128 F:00010022 Type:2
 SM2 A:1100 L:  68 F:00010064 Type:3
 SM3 A:1400 L:  68 F:00010020 Type:4
 FMMU0 Ls:00000044 Ll:  68 Lsb:0 Leb:7 Ps:1100 Psb:0 Ty:02 Act:01
 FMMU1 Ls:00000110 Ll:  68 Lsb:0 Leb:7 Ps:1400 Psb:0 Ty:01 Act:01
 FMMUfunc 0:1 1:2 2:3 3:0
 MBX length wr: 128 rd: 128 MBX protocols : 0e
 CoE details: 23 FoE details: 01 EoE details: 01 SoE details: 00
 Ebus current: 0[mA]
 only LRD/LWR:0
 CoE Object Description found, 20 entries.
0x1000      "Device type"                                 [VAR]
    0x00      "Device type"                                 [UNSIGNED32       R_R_R_]      0x00000192 / 402
0x1001      "Error register"                              [VAR]
    0x00      "Error register"                              [UNSIGNED8        R_R_R_]      0x00 / 0
0x1008      "Device name"                                 [VAR]
    0x00      "Device name"                                 [VISIBLE_STR(144) R_R_R_]      "IPD EtherCAT Slave"
0x1009      "Hardware version"                            [VAR]
    0x00      "Hardware version"                            [VISIBLE_STR(24)  R_R_R_]      "0.7"
0x100a      "Software version"                            [VAR]
    0x00      "Software version"                            [VISIBLE_STR(24)  R_R_R_]      "1.1"
0x1018      "Identity"                                    [RECORD  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "Vendor ID"                                   [UNSIGNED32       R_R_R_]      0x0000071a / 1818
    0x02      "Product code"                                [UNSIGNED32       R_R_R_]      0x00009252 / 37458
    0x03      "Revision"                                    [UNSIGNED32       R_R_R_]      0x00000001 / 1
    0x04      "Serial number"                               [UNSIGNED32       R_R_R_]      0x00000001 / 1
0x10f1      "Error Settings"                              [RECORD  maxsub(0x02 / 2)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x01      "Local Error Reaction"                        [UNSIGNED32       RWRWRW]      0x00000001 / 1
    0x02      "Sync Error Counter Limit"                    [UNSIGNED16       RWRWRW]      0x0004 / 4
0x1600      "Output mapping 0"                            [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x70000020 / 1879048224
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x70010120 / 1879114016
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x70010210 / 1879114256
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x70010310 / 1879114512
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x70010410 / 1879114768
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x70010510 / 1879115024
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x70010620 / 1879115296
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x70010720 / 1879115552
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x70010810 / 1879115792
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x70010910 / 1879116048
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x70010a10 / 1879116304
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x70010b10 / 1879116560
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x70010c20 / 1879116832
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x70020120 / 1879179552
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x70020210 / 1879179792
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x70020310 / 1879180048
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x70020410 / 1879180304
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x70020510 / 1879180560
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x70020620 / 1879180832
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x70020720 / 1879181088
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x70020810 / 1879181328
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x70020910 / 1879181584
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x70020a10 / 1879181840
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x70020b10 / 1879182096
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x70020c20 / 1879182368
0x1a00      "Input mapping 0"                             [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x60000020 / 1610612768
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x60010120 / 1610678560
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x60010210 / 1610678800
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x60010310 / 1610679056
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x60010410 / 1610679312
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x60010510 / 1610679568
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x60010620 / 1610679840
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x60010720 / 1610680096
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x60010810 / 1610680336
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x60010910 / 1610680592
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x60010a10 / 1610680848
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x60010b10 / 1610681104
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x60010c20 / 1610681376
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x60020120 / 1610744096
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x60020210 / 1610744336
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x60020310 / 1610744592
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x60020410 / 1610744848
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x60020510 / 1610745104
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x60020620 / 1610745376
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x60020720 / 1610745632
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x60020810 / 1610745872
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x60020910 / 1610746128
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x60020a10 / 1610746384
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x60020b10 / 1610746640
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x60020c20 / 1610746912
0x1c00      "Sync manager type"                           [ARRAY  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "SubIndex 001"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x02      "SubIndex 002"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x03      "SubIndex 003"                                [UNSIGNED8        R_R_R_]      0x03 / 3
    0x04      "SubIndex 004"                                [UNSIGNED8        R_R_R_]      0x04 / 4
0x1c12      "SyncManager 2 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1600 / 5632
0x1c13      "SyncManager 3 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1a00 / 6656
0x1c32      "SM output parameter"                         [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0001 / 1
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001e1486 / 1971334
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0001 / 1
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x7876 / 30838
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x1c33      "SM input parameter"                          [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0022 / 34
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001e1486 / 1971334
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x7876 / 30838
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x6000      "GPIO_INPUTS"                                 [VAR]
    0x00      "GPIO_INPUTS"                                 [UNSIGNED32       R_R_R_]      0x00000000 / 0
0x6001      "IPDFbkData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis1VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis1CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis1CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis1AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis1ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axi2PosFbk"                                  [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis2VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis2CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis2CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis2AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis2ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x6002      "IPDFbkData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis3VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis3CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis3CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis3AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis3ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axis4PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis4VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis4CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis4CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis4AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis4ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x7000      "GPIO_OUTPUTS"                                [VAR]
    0x00      "GPIO_OUTPUTS"                                [UNSIGNED32       RWRWRW]      0x00000000 / 0
0x7001      "IPDCmdData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis1VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis1CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis1CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis1AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis1ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis2PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis2VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis2CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis2CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis2AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis2ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
0x7002      "IPDCmdData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis3VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis3CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis3CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis3AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis3ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis4PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis4VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis4CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis4CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis4AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis4ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0

Slave:3
 Name:IPD EtherCAT Slave
 Output size: 544bits
 Input size: 544bits
 State: 4
 Delay: 1440[ns]
 Has DC: 1
 DCParentport:1
 Activeports:1.0.0.0
 Configured address: 1003
 Man: 0000071a ID: 00009252 Rev: 00000001
 SM0 A:1000 L: 128 F:00010026 Type:1
 SM1 A:1080 L: 128 F:00010022 Type:2
 SM2 A:1100 L:  68 F:00010064 Type:3
 SM3 A:1400 L:  68 F:00010020 Type:4
 FMMU0 Ls:00000088 Ll:  68 Lsb:0 Leb:7 Ps:1100 Psb:0 Ty:02 Act:01
 FMMU1 Ls:00000154 Ll:  68 Lsb:0 Leb:7 Ps:1400 Psb:0 Ty:01 Act:01
 FMMUfunc 0:1 1:2 2:3 3:0
 MBX length wr: 128 rd: 128 MBX protocols : 0e
 CoE details: 23 FoE details: 01 EoE details: 01 SoE details: 00
 Ebus current: 0[mA]
 only LRD/LWR:0
 CoE Object Description found, 20 entries.
0x1000      "Device type"                                 [VAR]
    0x00      "Device type"                                 [UNSIGNED32       R_R_R_]      0x00000192 / 402
0x1001      "Error register"                              [VAR]
    0x00      "Error register"                              [UNSIGNED8        R_R_R_]      0x00 / 0
0x1008      "Device name"                                 [VAR]
    0x00      "Device name"                                 [VISIBLE_STR(144) R_R_R_]      "IPD EtherCAT Slave"
0x1009      "Hardware version"                            [VAR]
    0x00      "Hardware version"                            [VISIBLE_STR(24)  R_R_R_]      "0.7"
0x100a      "Software version"                            [VAR]
    0x00      "Software version"                            [VISIBLE_STR(24)  R_R_R_]      "1.1"
0x1018      "Identity"                                    [RECORD  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "Vendor ID"                                   [UNSIGNED32       R_R_R_]      0x0000071a / 1818
    0x02      "Product code"                                [UNSIGNED32       R_R_R_]      0x00009252 / 37458
    0x03      "Revision"                                    [UNSIGNED32       R_R_R_]      0x00000001 / 1
    0x04      "Serial number"                               [UNSIGNED32       R_R_R_]      0x00000001 / 1
0x10f1      "Error Settings"                              [RECORD  maxsub(0x02 / 2)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x01      "Local Error Reaction"                        [UNSIGNED32       RWRWRW]      0x00000001 / 1
    0x02      "Sync Error Counter Limit"                    [UNSIGNED16       RWRWRW]      0x0004 / 4
0x1600      "Output mapping 0"                            [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x70000020 / 1879048224
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x70010120 / 1879114016
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x70010210 / 1879114256
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x70010310 / 1879114512
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x70010410 / 1879114768
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x70010510 / 1879115024
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x70010620 / 1879115296
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x70010720 / 1879115552
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x70010810 / 1879115792
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x70010910 / 1879116048
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x70010a10 / 1879116304
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x70010b10 / 1879116560
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x70010c20 / 1879116832
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x70020120 / 1879179552
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x70020210 / 1879179792
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x70020310 / 1879180048
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x70020410 / 1879180304
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x70020510 / 1879180560
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x70020620 / 1879180832
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x70020720 / 1879181088
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x70020810 / 1879181328
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x70020910 / 1879181584
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x70020a10 / 1879181840
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x70020b10 / 1879182096
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x70020c20 / 1879182368
0x1a00      "Input mapping 0"                             [RECORD  maxsub(0x19 / 25)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x19 / 25
    0x01      "SubIndex 001"                                [UNSIGNED32       R_R_R_]      0x60000020 / 1610612768
    0x02      "SubIndex 002"                                [UNSIGNED32       R_R_R_]      0x60010120 / 1610678560
    0x03      "SubIndex 003"                                [UNSIGNED32       R_R_R_]      0x60010210 / 1610678800
    0x04      "SubIndex 004"                                [UNSIGNED32       R_R_R_]      0x60010310 / 1610679056
    0x05      "SubIndex 005"                                [UNSIGNED32       R_R_R_]      0x60010410 / 1610679312
    0x06      "SubIndex 006"                                [UNSIGNED32       R_R_R_]      0x60010510 / 1610679568
    0x07      "SubIndex 007"                                [UNSIGNED32       R_R_R_]      0x60010620 / 1610679840
    0x08      "SubIndex 008"                                [UNSIGNED32       R_R_R_]      0x60010720 / 1610680096
    0x09      "SubIndex 009"                                [UNSIGNED32       R_R_R_]      0x60010810 / 1610680336
    0x0a      "SubIndex 010"                                [UNSIGNED32       R_R_R_]      0x60010910 / 1610680592
    0x0b      "SubIndex 011"                                [UNSIGNED32       R_R_R_]      0x60010a10 / 1610680848
    0x0c      "SubIndex 012"                                [UNSIGNED32       R_R_R_]      0x60010b10 / 1610681104
    0x0d      "SubIndex 013"                                [UNSIGNED32       R_R_R_]      0x60010c20 / 1610681376
    0x0e      "SubIndex 014"                                [UNSIGNED32       R_R_R_]      0x60020120 / 1610744096
    0x0f      "SubIndex 015"                                [UNSIGNED32       R_R_R_]      0x60020210 / 1610744336
    0x10      "SubIndex 016"                                [UNSIGNED32       R_R_R_]      0x60020310 / 1610744592
    0x11      "SubIndex 017"                                [UNSIGNED32       R_R_R_]      0x60020410 / 1610744848
    0x12      "SubIndex 018"                                [UNSIGNED32       R_R_R_]      0x60020510 / 1610745104
    0x13      "SubIndex 019"                                [UNSIGNED32       R_R_R_]      0x60020620 / 1610745376
    0x14      "SubIndex 020"                                [UNSIGNED32       R_R_R_]      0x60020720 / 1610745632
    0x15      "SubIndex 021"                                [UNSIGNED32       R_R_R_]      0x60020810 / 1610745872
    0x16      "SubIndex 022"                                [UNSIGNED32       R_R_R_]      0x60020910 / 1610746128
    0x17      "SubIndex 023"                                [UNSIGNED32       R_R_R_]      0x60020a10 / 1610746384
    0x18      "SubIndex 024"                                [UNSIGNED32       R_R_R_]      0x60020b10 / 1610746640
    0x19      "SubIndex 025"                                [UNSIGNED32       R_R_R_]      0x60020c20 / 1610746912
0x1c00      "Sync manager type"                           [ARRAY  maxsub(0x04 / 4)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x04 / 4
    0x01      "SubIndex 001"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x02      "SubIndex 002"                                [UNSIGNED8        R_R_R_]      0x02 / 2
    0x03      "SubIndex 003"                                [UNSIGNED8        R_R_R_]      0x03 / 3
    0x04      "SubIndex 004"                                [UNSIGNED8        R_R_R_]      0x04 / 4
0x1c12      "SyncManager 2 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1600 / 5632
0x1c13      "SyncManager 3 assignment"                    [ARRAY  maxsub(0x01 / 1)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x01 / 1
    0x01      "SubIndex 001"                                [UNSIGNED16       R_R_R_]      0x1a00 / 6656
0x1c32      "SM output parameter"                         [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0001 / 1
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001ea70c / 2008844
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0001 / 1
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x2d72 / 11634
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x1c33      "SM input parameter"                          [RECORD  maxsub(0x20 / 32)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x20 / 32
    0x01      "Synchronization Type"                        [UNSIGNED16       RWR_R_]      0x0022 / 34
    0x02      "Cycle Time"                                  [UNSIGNED32       R_R_R_]      0x001ea70c / 2008844
    0x04      "Synchronization Types supported"             [UNSIGNED16       R_R_R_]      0x401f / 16415
    0x05      "Minimum Cycle Time"                          [UNSIGNED32       R_R_R_]      0x000186a0 / 100000
    0x06      "Calc and Copy Time"                          [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x08      "Get Cycle Time"                              [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x09      "Delay Time"                                  [UNSIGNED32       R_R_R_]      0x00000000 / 0
    0x0a      "Sync0 Cycle Time"                            [UNSIGNED32       RWRWRW]      0x00000000 / 0
    0x0b      "SM-Event Missed"                             [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Cycle Time Too Small"                        [UNSIGNED16       R_R_R_]      0x2d72 / 11634
    0x20      "Sync Error"                                  [BOOLEAN          R_R_R_]      FALSE
0x6000      "GPIO_INPUTS"                                 [VAR]
    0x00      "GPIO_INPUTS"                                 [UNSIGNED32       R_R_R_]      0x00000000 / 0
0x6001      "IPDFbkData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis1VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis1CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis1CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis1AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis1ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axi2PosFbk"                                  [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis2VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis2CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis2CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis2AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis2ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x6002      "IPDFbkData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x02      "Axis3VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x03      "Axis3CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x04      "Axis3CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x05      "Axis3AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x06      "Axis3ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x07      "Axis4PosFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
    0x08      "Axis4VelFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x09      "Axis4CurFbk"                                 [INTEGER16        R_R_R_]      0x0000 / 0
    0x0a      "Axis4CtlFbk"                                 [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0b      "Axis4AdrsFbk"                                [UNSIGNED16       R_R_R_]      0x0000 / 0
    0x0c      "Axis4ParFbk"                                 [INTEGER32        R_R_R_]      0x00000000 / 0
0x7000      "GPIO_OUTPUTS"                                [VAR]
    0x00      "GPIO_OUTPUTS"                                [UNSIGNED32       RWRWRW]      0x00000000 / 0
0x7001      "IPDCmdData1"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis1PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis1VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis1CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis1CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis1AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis1ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis2PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis2VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis2CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis2CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis2AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis2ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
0x7002      "IPDCmdData2"                                 [RECORD  maxsub(0x0c / 12)]
    0x00      "SubIndex 000"                                [UNSIGNED8        R_R_R_]      0x0c / 12
    0x01      "Axis3PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x02      "Axis3VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x03      "Axis3CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x04      "Axis3CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x05      "Axis3AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x06      "Axis3ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x07      "Axis4PosCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
    0x08      "Axis4VelCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x09      "Axis4CurCmd"                                 [INTEGER16        RWRWRW]      0x0000 / 0
    0x0a      "Axis4CtlCmd"                                 [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0b      "Axis4AdrsCmd"                                [UNSIGNED16       RWRWRW]      0x0000 / 0
    0x0c      "Axis4ParCmd"                                 [INTEGER32        RWRWRW]      0x00000000 / 0
End slaveinfo, close socket
End program

