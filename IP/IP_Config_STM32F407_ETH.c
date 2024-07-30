/*********************************************************************
*                SEGGER MICROCONTROLLER GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 2003-2014     SEGGER Microcontroller GmbH & Co KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

----------------------------------------------------------------------
File        : IP_Config_STM32F746.c
Purpose     : Configuration file for TCP/IP with STM32F746
---------------------------END-OF-HEADER------------------------------
*/

#include "IP.h"
#include "IP_NI_SYNOPSYS.h"
#include "BSP_IP.h"
#include "stm32f4xx.h"  // For the cache handling functions.

#if IP_SUPPORT_PTP
#include "IP_NI_SYNOPSYS_PTP.h"
#endif

/*********************************************************************
*
*       Configuration
*
**********************************************************************
*/

#define ALLOC_SIZE   0x6000                      // Size of memory dedicated to the stack in bytes.
#define DRIVER       &IP_Driver_STM32F4          // Driver used for target.
#define TARGET_NAME  "STM32F407"                 // Target name used for DHCP client.
#define HW_ADDR      "\x00\x22\xC7\xFF\xFF\xFF"  // MAC addr. used for target.
#define USE_DHCP     0                           // Use DHCP client or static IP configuration.

//
// The following parameters are only used when the DHCP client is not active.
//
#define IP_ADDR      IP_BYTES2ADDR(177, 152,   1, 128)
#define SUBNET_MASK  IP_BYTES2ADDR(255, 255, 255,   0)
#define GW_ADDR      IP_BYTES2ADDR(177, 152,   1,   1)
#define DNS_ADDR     IP_BYTES2ADDR(177, 152,   1,   1)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*
static void _CleanDCache(void* p, unsigned long NumBytes) {
  SCB_CleanDCache_by_Addr(p, NumBytes);
}
static void _InvalidateDCache(void* p, unsigned long NumBytes) {
  SCB_InvalidateDCache_by_Addr(p, NumBytes);
}

static const SEGGER_CACHE_CONFIG _CacheConfig = {
  32,                            // CacheLineSize of CPU
  NULL,                          // pfDMB
  _CleanDCache,                  // pfClean
  _InvalidateDCache              // pfInvalidate
};
*/
//static U32 _aPool[ALLOC_SIZE / 4];
static U32 _aPool[ALLOC_SIZE / 4] __attribute__ (( section (".RAM1") ));  // RAM used by peripherals can NOT be located in ITCM or DTCM RAM.

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_X_Config()
*
* Function description
*   This function is called by the IP stack during IP_Init().
*
* Typical memory/buffer configurations:
*   Microcontroller system, minimum size optimized
*     #define ALLOC_SIZE 0x1000                         // 4KBytes RAM.
*     mtu = 576;                                        // 576 is minimum acc. to RFC, 1500 is max. for Ethernet.
*     IP_SetMTU(0, mtu);                                // Maximum Transmission Unit is 1500 for Ethernet by default.
*     IP_AddBuffers(4, 256);                            // Small buffers.
*     IP_AddBuffers(2, mtu + 16);                       // Big buffers. Size should be mtu + 16 byte for Ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding).
*     IP_ConfTCPSpace(2 * (mtu - 40), 1 * (mtu - 40));  // Define the TCP Tx and Rx window size. At least Tx space for 2*(mtu-40) for two full TCP packets is needed.
*
*   Microcontroller system, size optimized
*     #define ALLOC_SIZE 0x3000                         // 12KBytes RAM.
*     mtu = 576;                                        // 576 is minimum acc. to RFC, 1500 is max. for Ethernet.
*     IP_SetMTU(0, mtu);                                // Maximum Transmission Unit is 1500 for Ethernet by default.
*     IP_AddBuffers(8, 256);                            // Small buffers.
*     IP_AddBuffers(4, mtu + 16);                       // Big buffers. Size should be mtu + 16 byte for Ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding).
*     IP_ConfTCPSpace(2 * (mtu - 40), 2 * (mtu - 40));  // Define the TCP Tx and Rx window size. At least Tx space for 2*(mtu-40) for two full TCP packets is needed.
*
*   Microcontroller system, speed optimized or multiple connections
*     #define ALLOC_SIZE 0x6000                         // 24 KBytes RAM.
*     mtu = 1500;                                       // 576 is minimum acc. to RFC, 1500 is max. for Ethernet.
*     IP_SetMTU(0, mtu);                                // Maximum Transmission Unit is 1500 for Ethernet by default.
*     IP_AddBuffers(12, 256);                           // Small buffers.
*     IP_AddBuffers(6, mtu + 16);                       // Big buffers. Size should be mtu + 16 byte for Ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding).
*     IP_ConfTCPSpace(3 * (mtu - 40), 3 * (mtu - 40));  // Define the TCP Tx and Rx window size. At least Tx space for 2*(mtu-40) for two full TCP packets is needed.
*
*   System with lots of RAM
*     #define ALLOC_SIZE 0x20000                        // 128 KBytes RAM.
*     mtu = 1500;                                       // 576 is minimum acc. to RFC, 1500 is max. for Ethernet.
*     IP_SetMTU(0, mtu);                                // Maximum Transmission Unit is 1500 for Ethernet by default.
*     IP_AddBuffers(50, 256);                           // Small buffers.
*     IP_AddBuffers(50, mtu + 16);                      // Big buffers. Size should be mtu + 16 byte for Ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding).
*     IP_ConfTCPSpace(8 * (mtu - 40), 8 * (mtu - 40));  // Define the TCP Tx and Rx window size. At least Tx space for 2*(mtu-40) for two full TCP packets is needed.
*/
void IP_X_Config(void) {
  int mtu;
  int IFaceId;

  IP_AssignMemory(_aPool, sizeof(_aPool));                  // Assigning memory should be the first thing.
  //IP_CACHE_SetConfig(&_CacheConfig, sizeof(_CacheConfig));  // Set cache configuration for IP stack.
  IFaceId = IP_AddEtherInterface(DRIVER);                   // Add driver for your hardware.
  IP_BSP_SetAPI(IFaceId, &BSP_IP_Api);                      // Set BSP callbacks for hardware access.
  IP_SetHWAddrEx(IFaceId, (const U8*)HW_ADDR, 6);           // MAC addr.: Needs to be unique for production units.
#if IP_SUPPORT_PTP
  //
  // PTP configuration
  // Move this section beneath the "IFaceId = IP_AddEtherInterface()" part
  // that you want to enable PTP for.
  //
  IP_NI_AddPTPDriver(IFaceId, &IP_PTP_Driver_Synopsys, SystemCoreClock);
#endif
  //
  // Configure the PHY interface mode (optional):
  //   - IP_PHY_MODE_MII : MII, typically default if not explicitly configured.
  //   - IP_PHY_MODE_RMII: RMII
  // Can be set/overwritten from BSP_IP pfGetMiiMode() callback.
  //
//  IP_NI_ConfigPHYMode(IFaceId, IP_PHY_MODE_RMII);
  //
  // Use DHCP client or define IP address, subnet mask,
  // gateway address and DNS server according to the
  // requirements of your application.
  //
#if USE_DHCP
  IP_DHCPC_Activate(IFaceId, TARGET_NAME, NULL, NULL);  // Activate DHCP client.
#else
  IP_SetAddrMaskEx(IFaceId, IP_ADDR, SUBNET_MASK);      // Assign IP addr. and subnet mask.
  IP_SetGWAddr(IFaceId, GW_ADDR);                       // Set gateway addr.
  IP_DNS_SetServer(DNS_ADDR);                           // Set DNS server addr.
#endif
  //
  // Run-time configure buffers.
  // The default setup will do for most cases.
  //
  mtu = 1500;                                       // 576 is minimum acc. to RFC, 1500 is max. for Ethernet
  IP_SetMTU(IFaceId, mtu);                          // Maximum Transmission Unit is 1500 for ethernet by default
  IP_AddBuffers(12, 256);                           // Small buffers.
  IP_AddBuffers(6, mtu + 16);                       // Big buffers. Size should be mtu + 16 byte for ethernet header (2 bytes type, 2*6 bytes MAC, 2 bytes padding)
  IP_ConfTCPSpace(3 * (mtu - 40), 3 * (mtu - 40));  // Define the TCP Tx and Rx window size
  IP_SOCKET_SetDefaultOptions(0
//                              | SO_TIMESTAMP        // Send TCP timestamp to optimize the round trip time measurement. Normally not used in LAN.
                              | SO_KEEPALIVE        // Enable keepalives by default for TCP sockets.
                             );
  //
  // Define log and warn filter.
  // Note: The terminal I/O emulation might affect the timing of your
  //       application, since most debuggers need to stop the target
  //       for every terminal I/O output unless you use another
  //       implementation such as DCC or SWO.
  //
  IP_SetWarnFilter(0xFFFFFFFF);              // 0xFFFFFFFF: Do not filter: Output all warnings.
  IP_SetLogFilter(0
                  | IP_MTYPE_APPLICATION     // Output application messages.
                  | IP_MTYPE_INIT            // Output all messages from init.
                  | IP_MTYPE_LINK_CHANGE     // Output a message if link status changes.
                  | IP_MTYPE_PPP             // Output all PPP/PPPoE related messages.
                  | IP_MTYPE_DHCP            // Output general DHCP status messages.
#if IP_SUPPORT_IPV6
                  | IP_MTYPE_IPV6            // Output IPv6 address related messages
#endif
//                  | IP_MTYPE_DHCP_EXT        // Output additional DHCP messages.
//                  | IP_MTYPE_CORE            // Output log messages from core module.
//                  | IP_MTYPE_ALLOC           // Output log messages for memory allocation.
//                  | IP_MTYPE_DRIVER          // Output log messages from driver.
//                  | IP_MTYPE_ARP             // Output log messages from ARP layer.
//                  | IP_MTYPE_IP              // Output log messages from IP layer.
//                  | IP_MTYPE_TCP_CLOSE       // Output a log messages if a TCP connection has been closed.
//                  | IP_MTYPE_TCP_OPEN        // Output a log messages if a TCP connection has been opened.
//                  | IP_MTYPE_TCP_IN          // Output TCP input logs.
//                  | IP_MTYPE_TCP_OUT         // Output TCP output logs.
//                  | IP_MTYPE_TCP_RTT         // Output TCP round trip time (RTT) logs.
//                  | IP_MTYPE_TCP_RXWIN       // Output TCP RX window related log messages.
//                  | IP_MTYPE_TCP             // Output all TCP related log messages.
//                  | IP_MTYPE_UDP_IN          // Output UDP input logs.
//                  | IP_MTYPE_UDP_OUT         // Output UDP output logs.
//                  | IP_MTYPE_UDP             // Output all UDP related messages.
//                  | IP_MTYPE_ICMP            // Output ICMP related log messages.
//                  | IP_MTYPE_NET_IN          // Output network input related messages.
//                  | IP_MTYPE_NET_OUT         // Output network output related messages.
//                  | IP_MTYPE_DNS             // Output all DNS related messages.
//                  | IP_MTYPE_SOCKET_STATE    // Output socket status messages.
//                  | IP_MTYPE_SOCKET_READ     // Output socket read related messages.
//                  | IP_MTYPE_SOCKET_WRITE    // Output socket write related messages.
//                  | IP_MTYPE_SOCKET          // Output all socket related messages.
                 );
  //
  // Add protocols to the stack.
  //
  IP_TCP_Add();
  IP_UDP_Add();
  IP_ICMP_Add();
#if IP_SUPPORT_IPV6
  IP_IPV6_Add(IFaceId);
#endif
}

/*************************** End of file ****************************/
