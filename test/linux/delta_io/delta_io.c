#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>

#include "ethercat.h"
#include "arc_console.hpp"

#define EC_TIMEOUTMON 500

#define ETH_CH_NAME "eno1"
#define EC_SLAVE_ID 1

char IOmap[4096];
OSAL_THREAD_HANDLE thread1;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;

static int sdo_write8(uint16 slave, uint16 index, uint8 subindex, uint8 value)
{
   return ecx_SDOwrite(&ecx_context, slave, index, subindex, FALSE, sizeof(uint8), &value, EC_TIMEOUTRXM);
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

void simpletest(char *ifname)
{
   int i, j, oloop, iloop, chk;
   needlf = FALSE;
   inOP = FALSE;

   uint64_t start_time = clock_ms();

   printf("Starting simple test\n");

   /* initialise SOEM, bind socket to ifname */
   if (ec_init(ifname))
   {
      printf("ec_init on %s succeeded.\n", ifname);

      /* request INIT state for all slaves */
      ec_writestate(0);

      /* find and auto-config slaves */

      if (ec_config_init(FALSE) > 0)
      {
         printf("%d slaves found and configured.\n", ec_slavecount);

         setupDeltaIO(); // initial io module

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
            /* cyclic loop */
            for (i = 1; i <= 10000; i++)
            {

               static int blink = 0;
               if ((clock_ms() - start_time) >= 1000)
               {
                  // printf("[%ld]1 second has passed.\r\n", clock_ms());
                  blink = !blink;

                  for (j = 0; j < oloop; j++)
                  {
                     //*(ec_slave[0].outputs + j) = blink ? 0xFF : 0x00;
                     ec_slave[0].outputs[j] = blink ? 0x01 : 0x00;
                  }
                  //*ec_slave[0].outputs = blink ? 0xFF : 0x00;

                  // uint8 outputs = blink ? 0xFF : 0x00;
                  // ec_SDOwrite(0, 0x6200, 0x01, FALSE, sizeof(uint8), &outputs, EC_TIMEOUTRXM);

                  start_time = clock_ms();
               }

               ec_send_processdata();
               wkc = ec_receive_processdata(EC_TIMEOUTRET);

               if (wkc >= expectedWKC)
               {
                  printf("Processdata cycle %4d, WKC %d , O:", i, wkc);

                  for (j = 0; j < oloop; j++)
                  {
                     printf(" %2.2x", *(ec_slave[0].outputs + j));
                  }

                  printf(" I:");
                  for (j = 0; j < iloop; j++)
                  {
                     printf(" %2.2x", *(ec_slave[0].inputs + j));
                  }
                  printf(" T:%" PRId64 "\r", ec_DCtime);
                  needlf = TRUE;
               }
               osal_usleep(100);
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

   while (1)
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
}

// int main(int argc, char *argv[])
int main(void)
{

   printf("SOEM (Simple Open EtherCAT Master)\nSimple test\n");
   /* create thread to handle slave error handling in OP */
   //      pthread_create( &thread1, NULL, (void *) &ecatcheck, (void*) &ctime);
   osal_thread_create(&thread1, 128000, &ecatcheck, (void *)&ctime);
   /* start cyclic part */
   simpletest(ETH_CH_NAME);

   printf("End program\n");
   return (0);
}
