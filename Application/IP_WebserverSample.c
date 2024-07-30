/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2007 - 2022    SEGGER Microcontroller GmbH               *
*                                                                    *
*       www.segger.com     Support: www.segger.com/ticket            *
*                                                                    *
**********************************************************************
*                                                                    *
*       emNet * TCP/IP stack for embedded applications               *
*                                                                    *
*                                                                    *
*       Please note:                                                 *
*                                                                    *
*       Knowledge of this file may under no circumstances            *
*       be used to write a similar product for in-house use.         *
*                                                                    *
*       Thank you for your fairness !                                *
*                                                                    *
**********************************************************************
*                                                                    *
*       emNet version: V3.42.6                                       *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : IP_WebserverSample.c
Purpose : Sample program for embOS & TCP/IP
*/

#include <stdio.h>
#include <stdlib.h>
#include "RTOS.h"
#include "BSP.h"
#include "IP.h"
#include "IP_Webserver.h"
#include "WEBS_Conf.h"        // Stack size depends on configuration

/*********************************************************************
*
*       Local defines, configurable
*
**********************************************************************
*/

#define USE_RX_TASK   0  // 0: Packets are read in ISR, 1: Packets are read in a task of its own.

//
// Web server and IP stack
//
#define MAX_CONNECTIONS           2
#define BACK_LOG                 20
#define IDLE_TIMEOUT           1000  // Timeout [ms] after which the connection will be closed if no new data is received.
#define SERVER_PORT              80
#define CHILD_ALLOC_SIZE       2560  // NumBytes required from memory pool for one connection. Should be fine tuned according
                                     // to your configuration using IP_WEBS_CountRequiredMem() .

//
// Task priorities.
//
enum {
   TASK_PRIO_WEBS_CHILD = 150
  ,TASK_PRIO_WEBS_PARENT
  ,TASK_PRIO_IP_TASK           // Priority should be higher than all IP application tasks.
#if USE_RX_TASK
  ,TASK_PRIO_IP_RX_TASK        // Must be the highest priority of all IP related tasks.
#endif
};

//
// Task stack sizes that might not fit for some interfaces (multiples of sizeof(int)).
//
#ifndef   APP_TASK_STACK_OVERHEAD
  #define APP_TASK_STACK_OVERHEAD     0
#endif

#ifndef   STACK_SIZE_SERVER
  #define STACK_SIZE_SERVER           (2304 + APP_TASK_STACK_OVERHEAD)
#endif

//
// UDP discover
//
#define ETH_UDP_DISCOVER_PORT    50020
#define PACKET_SIZE              0x80

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static U32 _aPool[(CHILD_ALLOC_SIZE * MAX_CONNECTIONS) / sizeof(int)];  // Memory pool for the Web server child tasks.

static IP_HOOK_ON_STATE_CHANGE _StateChangeHook;
static int                     _IFaceId;
static int                     _ConnectCnt;

//
// Task stacks and Task-Control-Blocks.
//
static OS_STACKPTR int _IPStack[TASK_STACK_SIZE_IP_TASK/sizeof(int)];       // Stack of the IP_Task.
static OS_TASK         _IPTCB;                                              // Task-Control-Block of the IP_Task.

#if USE_RX_TASK
static OS_STACKPTR int _IPRxStack[TASK_STACK_SIZE_IP_RX_TASK/sizeof(int)];  // Stack of the IP_RxTask.
static OS_TASK         _IPRxTCB;                                            // Task-Control-Block of the IP_RxTask.
#endif

//
// Webserver TCBs and stacks
//
static OS_TASK         _aWebTasks[MAX_CONNECTIONS];
static OS_STACKPTR int _aWebStacks[MAX_CONNECTIONS][STACK_SIZE_SERVER/sizeof(int)];

//
// File system info
//
static const IP_FS_API *_pFS_API;

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
void MainTask(void);
#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _OnStateChange()
*
* Function description
*   Callback that will be notified once the state of an interface
*   changes.
*
* Parameters
*   IFaceId   : Zero-based interface index.
*   AdminState: Is this interface enabled ?
*   HWState   : Is this interface physically ready ?
*/
static void _OnStateChange(unsigned IFaceId, U8 AdminState, U8 HWState) {
  //
  // Check if this is a disconnect from the peer or a link down.
  // In this case call IP_Disconnect() to get into a known state.
  //
  if (((AdminState == IP_ADMIN_STATE_DOWN) && (HWState == 1)) ||  // Typical for dial-up connection e.g. PPP when closed from peer. Link up but app. closed.
      ((AdminState == IP_ADMIN_STATE_UP)   && (HWState == 0))) {  // Typical for any Ethernet connection e.g. PPPoE. App. opened but link down.
    IP_Disconnect(IFaceId);                                       // Disconnect the interface to a clean state.
  }
}

/*********************************************************************
*
*       _closesocket()
*
*  Function description
*    Wrapper for closesocket()
*/
static int _closesocket(long pConnectionInfo) {
  int r;
  struct linger Linger;

  Linger.l_onoff  = 1;  // Enable linger for this socket to verify that all data is send.
  Linger.l_linger = 1;  // Linger timeout in seconds
  setsockopt((long)pConnectionInfo, SOL_SOCKET, SO_LINGER, &Linger, sizeof(Linger));
  r = closesocket((long)pConnectionInfo);
  return r;
}

/*********************************************************************
*
*       _Recv()
*
*  Function description
*    Wrapper for recv()
*/
static int _Recv(unsigned char *buf, int len, void *pConnectionInfo) {
  int r;

  r = recv((long)pConnectionInfo, (char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       _Send()
*
*  Function description
*    Wrapper for send()
*/
static int _Send(const unsigned char *buf, int len, void* pConnectionInfo) {
  int r;

  r = send((long)pConnectionInfo, (const char *)buf, len, 0);
  return r;
}

/*********************************************************************
*
*       WEBS_IP_API
*
*  Description
*   IP related function table
*/
static const WEBS_IP_API _Webs_IP_API = {
  _Send,
  _Recv
};

/*********************************************************************
*
*       _Alloc()
*
*  Function description
*    Wrapper for Alloc(). (emNet: IP_MEM_Alloc())
*/
static void * _Alloc(U32 NumBytesReq) {
  return IP_AllocEx(_aPool, NumBytesReq);
}

/*********************************************************************
*
*       _Free()
*
*  Function description
*    Wrapper for Alloc(). (emNet: IP_MEM_Alloc())
*/
static void _Free(void *p) {
  IP_Free(p);
}

/*********************************************************************
*
*       WEBS_SYS_API
*
*  Description
*   System related function table
*/
static const WEBS_SYS_API _Webs_SYS_API = {
  _Alloc,
  _Free
};

/*********************************************************************
*
*       _AddToConnectCnt
*/
static void _AddToConnectCnt(int Delta) {
  OS_IncDI();
  _ConnectCnt += Delta;
  OS_DecRI();
}

/*********************************************************************
*
*       _WebServerChildTask
*/
static void _WebServerChildTask(void *pContext) {
  WEBS_CONTEXT ChildContext;
  long hSock;
  int  Opt;
  int  r;

  hSock    = (long)pContext;
  Opt      = 1;
  setsockopt(hSock, SOL_SOCKET, SO_KEEPALIVE, &Opt, sizeof(Opt));
  IP_WEBS_Init(&ChildContext, &_Webs_IP_API, &_Webs_SYS_API, _pFS_API, &WebsSample_Application);  // Initialize the context of the child task.
  if (_ConnectCnt < MAX_CONNECTIONS) {
    r = IP_WEBS_ProcessEx(&ChildContext, pContext, NULL);
  } else {
    r = IP_WEBS_ProcessLastEx(&ChildContext, pContext, NULL);
  }
  if (r != WEBS_CONNECTION_DETACHED) {
    //
    // Only close the socket if it is still in web server context and has
    // not been detached to another handler like a WebSocket handler.
    //
    _closesocket(hSock);
  }
  OS_EnterRegion();
  _AddToConnectCnt(-1);
  OS_Terminate(0);
  OS_LeaveRegion();
}

/*********************************************************************
*
*       _WebServerParentTask
*/
static void _WebServerParentTask(void) {
  struct sockaddr_in InAddr;
  U32  Timeout;
  U32  NumBytes;
  long hSockListen;
  long hSock;
  int  i;
  int  t;
  int  t0;
  int  r;
  WEBS_BUFFER_SIZES BufferSizes;

  Timeout = IDLE_TIMEOUT;
  //
  // Assign file system
  //
  _pFS_API = &IP_FS_ReadOnly;  // To use a a real filesystem like emFile replace this line.
//  _pFS_API = &IP_FS_emFile;    // Use emFile
//  IP_WEBS_AddUpload();         // Enable upload
  //
  // Configure buffer size.
  //
  IP_MEMSET(&BufferSizes, 0, sizeof(BufferSizes));
  BufferSizes.NumBytesInBuf       = WEBS_IN_BUFFER_SIZE;
  BufferSizes.NumBytesOutBuf      = IP_TCP_GetMTU(_IFaceId) - 72;  // Use max. MTU configured for the last interface added minus worst case IPv4/TCP/VLAN headers.
                                                                   // Calculation for the memory pool is done under assumption of the best case headers with - 40 bytes.
  BufferSizes.NumBytesParaBuf     = WEBS_PARA_BUFFER_SIZE;
  BufferSizes.NumBytesFilenameBuf = WEBS_FILENAME_BUFFER_SIZE;
  BufferSizes.MaxRootPathLen      = WEBS_MAX_ROOT_PATH_LEN;
  //
  // Configure the size of the buffers used by the Webserver child tasks.
  //
  IP_WEBS_ConfigBufSizes(&BufferSizes);
  //
  // Check memory pool size.
  //
  NumBytes = IP_WEBS_CountRequiredMem(NULL);     // Get NumBytes for internals of one child thread.
  NumBytes = (NumBytes + 64) * MAX_CONNECTIONS;  // Calc. the total amount for x connections (+ some bytes for managing a memory pool).
  IP_Logf_Application("WEBS: Using a memory pool of %lu bytes for %lu connections.", sizeof(_aPool), MAX_CONNECTIONS);
  if (NumBytes > sizeof(_aPool)) {
    IP_Warnf_Application("WEBS: Memory pool should be at least %lu bytes.", NumBytes);
  }
  //
  // Give the stack some more memory to enable the dynamical memory allocation for the Web server child tasks
  //
  IP_AddMemory(_aPool, sizeof(_aPool));
  //
  // Get a socket into listening state
  //
  hSockListen = socket(AF_INET, SOCK_STREAM, 0);
  if (hSockListen == SOCKET_ERROR) {
    while(1); // This should never happen!
  }
  IP_MEMSET(&InAddr, 0, sizeof(InAddr));
  InAddr.sin_family      = AF_INET;
  InAddr.sin_port        = htons(SERVER_PORT);
  InAddr.sin_addr.s_addr = INADDR_ANY;
  bind(hSockListen, (struct sockaddr *)&InAddr, sizeof(InAddr));
  listen(hSockListen, BACK_LOG);
  //
  // Loop once per client and create a thread for the actual server
  //
  do {
Next:
    //
    // Wait for an incoming connection
    //
    hSock = 0;
    if ((hSock = accept(hSockListen, NULL, NULL)) == SOCKET_ERROR) {
      continue;    // Error
    }
    //
    // Create server thread to handle connection.
    // If connection limit is reached, keep trying for some time before giving up and outputting an error message
    //
    t0 = OS_GetTime32() + 1000;
    do {
      if (_ConnectCnt < MAX_CONNECTIONS) {
        for (i = 0; i < MAX_CONNECTIONS; i++) {
          r = OS_IsTask(&_aWebTasks[i]);
          if (r == 0) {
            setsockopt(hSock, SOL_SOCKET, SO_RCVTIMEO, &Timeout, sizeof(Timeout));  // Set receive timeout for the client.
            OS_CREATETASK_EX(&_aWebTasks[i], "Webserver Child", _WebServerChildTask, TASK_PRIO_WEBS_CHILD, _aWebStacks[i], (void *)hSock);
            _AddToConnectCnt(1);
            goto Next;
          }
        }
      }
      //
      // Check time out
      //
      t = OS_GetTime32();
      if ((t - t0) > 0) {
        IP_WEBS_OnConnectionLimit(_Send, _Recv, (void*)hSock);
        _closesocket(hSock);
        break;
      }
      OS_Delay(10);
    } while(1);
  }  while(1);
}

/*********************************************************************
*
*       _OnRx
*
*  Function description
*    Discover client UDP callback. Called from stack
*    whenever we get a discover request.
*
*  Return value
*    IP_RX_ERROR  if packet is invalid for some reason
*    IP_OK        if packet is valid
*/
#if ETH_UDP_DISCOVER_PORT
static int _OnRx(IP_PACKET *pInPacket, void *pContext) {
  char *      pInData;
  IP_PACKET * pOutPacket;
  char *      pOutData;
  U32         TargetAddr;
  U32         IPAddr;
  unsigned    IFaceId;

  (void)pContext;

  IFaceId = IP_UDP_GetIFIndex(pInPacket);  // Find out the interface that the packet came in.
  IPAddr  = htonl(IP_GetIPAddr(IFaceId));
  if (IPAddr == 0) {
    goto Done;
  }
  pInData = (char*)IP_UDP_GetDataPtr(pInPacket);
  if (memcmp(pInData, "Discover", 8)) {
    goto Done;
  }
  //
  // Alloc packet
  //
  pOutPacket = IP_UDP_AllocEx(IFaceId, PACKET_SIZE);
  if (pOutPacket == NULL) {
    goto Done;
  }
  //
  // Fill packet with data
  //
  pOutData = (char*)IP_UDP_GetDataPtr(pOutPacket);
  IP_UDP_GetSrcAddr(pInPacket, &TargetAddr, sizeof(TargetAddr));    // Send back Unicast
  memset(pOutData, 0, PACKET_SIZE);
  strcpy(pOutData + 0x00, "Found");
  IPAddr = htonl(IP_GetIPAddr(IFaceId));
  memcpy(pOutData + 0x20, (void*)&IPAddr, 4);      // 0x20: IP address
  IP_GetHWAddr(IFaceId, (U8*)pOutData + 0x30, 6);  // 0x30: MAC address
  //
  // Send packet
  //
  IP_UDP_SendAndFree(IFaceId, TargetAddr, ETH_UDP_DISCOVER_PORT, ETH_UDP_DISCOVER_PORT, pOutPacket);
Done:
  return IP_OK;
}
#endif

/*********************************************************************
*
*       MainTask
*/
void MainTask(void) {
  IP_Init();
  _IFaceId = IP_INFO_GetNumInterfaces() - 1;                                           // Get the last registered interface ID as this is most likely the interface we want to use in this sample.
  //
  // Start TCP/IP task
  //
  OS_CREATETASK(&_IPTCB,   "IP_Task",   IP_Task,   TASK_PRIO_IP_TASK,    _IPStack);
#if USE_RX_TASK
  OS_CREATETASK(&_IPRxTCB, "IP_RxTask", IP_RxTask, TASK_PRIO_IP_RX_TASK, _IPRxStack);  // Start the IP_RxTask, optional.
#endif
  IP_AddStateChangeHook(&_StateChangeHook, _OnStateChange);                            // Register hook to be notified on disconnects.
  IP_Connect(_IFaceId);                                                                // Connect the interface if necessary.
  //
  // IPv4 address configured ?
  //
  while (IP_IFaceIsReadyEx(_IFaceId) == 0) {
    BSP_ToggleLED(0);
    OS_Delay(200);
  }
  BSP_ClrLED(0);
  OS_SetPriority(OS_GetTaskID(), TASK_PRIO_WEBS_PARENT);                                  // This task has highest prio!
  OS_SetTaskName(OS_GetTaskID(), "IP_WebServer");
#if ETH_UDP_DISCOVER_PORT
  //
  // Open UDP port ETH_UDP_DISCOVER_PORT for emNet discover
  //
  IP_UDP_Open(0L /* any foreign host */,  ETH_UDP_DISCOVER_PORT, ETH_UDP_DISCOVER_PORT,  _OnRx, 0L);
#endif
  IP_WEBS_X_SampleConfig();  // Load a web server sample config that might add other resources like REST.
  _WebServerParentTask();
}

/*************************** End of file ****************************/
