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

File    : IP_OS_embOS.c
Purpose : Kernel abstraction for embOS. Do not modify to allow easy updates!
*/

#include "IP.h"
#include "RTOS.h"

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

//
// The default tick is expected to be 1ms. For a finer tick
// like 1us a multiplicator has to be configured. The tick
// should match the OS tick.
// Examples:
//   - 1ms   = 1
//   - 100us = 10
//   - 10us  = 100
//
#define TICK_MULTIPLICATOR  1  // Default, 1 = 1ms.

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef struct TCP_WAIT {
  struct TCP_WAIT* pNext;
  struct TCP_WAIT* pPrev;
  void*            pWaitItem;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  OS_TASK*         pTask;
#endif
  OS_EVENT         Event;
} TCP_WAIT;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static TCP_WAIT*            volatile _pTCPWait;  // Head of List. One entry per waiting task
static char                          _IsInited;
#if (OS_VERSION >= 38606)                        // OS_AddOnTerminateHook() is supported since embOS v3.86f .
static OS_ON_TERMINATE_HOOK          _OnTerminateTaskHook;
#endif
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
static IP_ON_EXIT_CB                 _OnExitCB;
#endif

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/

OS_RSEMA IP_OS_RSema;      // Public only to allow inlining (direct call from IP-Stack).
OS_EVENT IP_OS_EventRx;    // Public only to allow inlining (direct call from IP-Stack).
OS_EVENT IP_OS_EventWiFi;  // Public only to allow inlining (direct call from IP-Stack).
OS_TASK* IP_OS_pIPTask;    // Public only to allow inlining (direct call from IP-Stack).

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _DLIST_RemoveDelete()
*
*  Function description
*    Removes a waitable object from the doubly linked list and deletes
*    its wait object from embOS lists.
*
*  Parameters
*    pTCPWait: Item to remove.
*
*  Additional information
*    Function is called from IP_OS_WaitItemTimed() and _OnTerminateTask().
*    Calling functions have to make sure that it is not called recursive
*    by disabling task switch before calling this routine.
*/
static void _DLIST_RemoveDelete(TCP_WAIT* pTCPWait) {
  //
  // Remove entry from doubly linked list.
  //
  if (pTCPWait->pPrev) {
    pTCPWait->pPrev->pNext = pTCPWait->pNext;
  } else {
    _pTCPWait = pTCPWait->pNext;
  }
  if (pTCPWait->pNext) {
    pTCPWait->pNext->pPrev = pTCPWait->pPrev;
  }
  //
  // Delete the event object.
  //
  OS_EVENT_Set(&pTCPWait->Event);  // Set event to prevent error on removing an unsignalled event.
  OS_EVENT_Delete(&pTCPWait->Event);
}

#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
/*********************************************************************
*
*       _OnTerminateTask()
*
*  Function description
*    This routine removes the registered wait objects from the doubly
*    linked list of wait objects upon termination of its task. This
*    is necessary due to the fact that the element is publically known
*    due to a doubly linked list but is stored on a task stack. In case
*    this task gets terminated we need to gracefully remove the element
*    from all resources and even remove it from any embOS list.
*
*  Parameters
*    pTask: Task handle of task that will be terminated.
*
*  Additional information
*    Function is called from an application task via OS hook with
*    task switching disabled.
*/
static void _OnTerminateTask(OS_CONST_PTR OS_TASK* pTask) {
  TCP_WAIT* pTCPWait;

  for (pTCPWait = _pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pTask == pTask) {
      //
      // Prior to deleting an event object it needs to be set to be unused
      // (no task waiting for it). Setting the EVENT object is safe as in
      // all cases only one the task that created the object on its stack
      // is waiting for the event and task switching is disabled. Therefore
      // we will stay in this routine and finish our work.
      //
      OS_EVENT_Set(&pTCPWait->Event);
      _DLIST_RemoveDelete(pTCPWait);
      break;
    }
  }
}
#endif

/*********************************************************************
*
*       _OnExit()
*
*  Function description
*    Called in case of a de-initialization of the stack.
*/
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
static void _OnExit(void) {
  OS_EVENT_Set(&IP_OS_EventRx);
  OS_EVENT_Delete(&IP_OS_EventRx);
  OS_EVENT_Set(&IP_OS_EventWiFi);
  OS_EVENT_Delete(&IP_OS_EventWiFi);
  OS_DeleteRSema(&IP_OS_RSema);
  OS_RemoveTerminateHook(&_OnTerminateTaskHook);
  IP_OS_pIPTask = NULL;
  _pTCPWait     = NULL;
  _IsInited     = 0;
}
#endif

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IP_OS_Init()
*
*  Function description
*    Initialize (create) all objects required for task synchronization.
*    These are 3 events (for IP_Task, IP_RxTask and WiFi task)
*    and one semaphore for protection of critical code which may not
*    be executed from multiple tasks at the same time.
*/
void IP_OS_Init(void) {
  if (_IsInited == 0) {
    OS_CREATERSEMA(&IP_OS_RSema);
    OS_EVENT_Create(&IP_OS_EventRx);
    OS_EVENT_Create(&IP_OS_EventWiFi);
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
    OS_AddOnTerminateHook(&_OnTerminateTaskHook, _OnTerminateTask);  // Has been renamed to OS_AddTerminateHook() with embOS v4.30 using a compatibility macro.
#endif
#if (IP_ALLOW_DEINIT && (OS_VERSION >= 43000))
    //
    // OS_RemoveTerminateHook() is supported since embOS v4.30 .
    // Only add an OnExit hook if we are able to remove the OS hook
    // as well as otherwise we would use OS_AddTerminateHook()
    // twice due to _IsInited != 0 .
    //
    IP_AddOnExitHandler(&_OnExitCB, _OnExit);
#endif
    _IsInited = 1;
  }
}

/*********************************************************************
*
*       IP_OS_DisableInterrupt()
*
*  Function description
*    Disables interrupts to lock against calls from interrupt routines.
*/
void IP_OS_DisableInterrupt(void) {
  OS_IncDI();
}

/*********************************************************************
*
*       IP_OS_EnableInterrupt()
*
*  Function description
*    Enables interrupts that have previously been disabled.
*/
void IP_OS_EnableInterrupt(void) {
  OS_DecRI();
}

/*********************************************************************
*
*       IP_OS_GetTime32()
*
*  Function description
*    Return the current system time in ms;
*    The value will wrap around after approximately 49.7 days; This is taken
*    into account by the stack.
*
*  Return value
*    U32 timestamp in system ticks (typically 1ms).
*/
U32 IP_OS_GetTime32(void) {
  return OS_GetTime32();
}

/*********************************************************************
*
*       IP_OS_Delay()
*
*  Function description
*    Blocks the calling task for a given time.
*
*  Parameters
*    ms: Time to block in system ticks (typically 1ms).
*/
void IP_OS_Delay(unsigned ms) {
#if (TICK_MULTIPLICATOR != 1)
  ms = ms * TICK_MULTIPLICATOR;
#endif
  OS_Delay(ms + 1);
}

/*********************************************************************
*
*       IP_OS_WaitNetEvent()
*
*  Function description
*    Called from IP_Task() only;
*    Blocks until the timeout expires or a NET-event occurs,
*    meaning IP_OS_SignalNetEvent() is called from an other task or
*    ISR.
*
*  Parameters
*    ms: Timeout for waiting in system ticks (typically 1ms).
*
*  Additional information
*    For the IP_Task() task events are used as they are using less
*    overhead and are slightly faster using event objects. However
*    as the task control block needs to be known the following rare
*    case might occur:
*
*      - IP_Init() is done and Rx interrupts are enabled.
*
*      - IP_Task() which runs a loop of IP_Exec(); IP_OS_WaitNetEvent()
*        gets interrupted before IP_OS_WaitNetEvent() can set
*        IP_OS_pIPTask .
*
*      - The Rx interrupt now calls IP_OS_SignalNetEvent() but
*        actually does not signal anything as IP_OS_pIPTask is
*        still NULL.
*
*      - One interrupt might get lost which is not a problem
*        for the IP_Task() as it will get woken up regularly
*        anyhow or at least with the second Rx event.
*/
void IP_OS_WaitNetEvent(unsigned ms) {
#if (TICK_MULTIPLICATOR != 1)
  ms = ms * TICK_MULTIPLICATOR;
#endif
  IP_OS_pIPTask = OS_GetpCurrentTask();
  (void)OS_WaitEventTimed(1, ms);
}

/*********************************************************************
*
*       IP_OS_SignalNetEvent()
*
*  Function description
*    Wakes the IP_Task if it is waiting for a NET-event or timeout in
*    the function IP_OS_WaitNetEvent().
*
*  Additional information
*    See IP_OS_WaitNetEvent() regarding why we are using task events.
*/
void IP_OS_SignalNetEvent(void) {
  if (IP_OS_pIPTask != NULL) {
    OS_SignalEvent(1, IP_OS_pIPTask);
  }
}

/*********************************************************************
*
*       IP_OS_WaitRxEvent()
*
*  Function description
*    Called by the IP_RxTask() whenever idle (no more packets in the
*    In-FIFO).
*
*  Additional information
*    See IP_OS_WaitNetEvent() regarding problems that might occur
*    when using task events. The same problematic applies to the
*    IP_RxTask() but as this is treated like an interrupt we do not
*    use a fallback activation each x seconds. Therefore we use event
*    objects in all other cases which are always valid after
*    IP_Init() and can be signaled before being waited on. In this
*    case the wait will simply run through this time.
*/
void IP_OS_WaitRxEvent(void) {
  OS_EVENT_Wait(&IP_OS_EventRx);
}

/*********************************************************************
*
*       IP_OS_SignalRxEvent()
*
*  Function description
*    Called by an interrupt to signal that the IP_RxTask() needs
*    to check for new received packets.
*
*  Additional information
*    See IP_OS_WaitRxEvent() regarding task event vs. event object
*    usage.
*/
void IP_OS_SignalRxEvent(void) {
  OS_EVENT_Set(&IP_OS_EventRx);
}

/*********************************************************************
*
*       IP_OS_WaitWiFiEventTimed()
*
*  Function description
*    Called by the IP_WIFI_IsrTask() whenever idle (no more events
*    to handle).
*
*  Parameters
*    Timeout: Time [ms] to wait for an event to be signaled. 0 means
*             infinite wait.
*
*  Additional information
*    See IP_OS_WaitRxEvent() regarding task event vs. event object
*    usage.
*/
void IP_OS_WaitWiFiEventTimed(unsigned Timeout) {
  if (Timeout == 0) {
    OS_EVENT_Wait(&IP_OS_EventWiFi);
  } else {
    (void)OS_EVENT_WaitTimed(&IP_OS_EventWiFi, Timeout);
  }
}

/*********************************************************************
*
*       IP_OS_SignalWiFiEvent()
*
*  Function description
*    Called by an interrupt from an external WiFi module to signal
*    that the IP_WIFI_IsrTask() needs to handle a WiFi event.
*
*  Additional information
*    See IP_OS_WaitRxEvent() regarding task event vs. event object
*    usage.
*/
void IP_OS_SignalWiFiEvent(void) {
  OS_EVENT_Set(&IP_OS_EventWiFi);
}

/*********************************************************************
*
*       IP_OS_Lock()
*
*  Function description
*    The stack requires a single lock, typically a resource semaphore
*    or mutex; This function locks this object, guarding sections of
*    the stack code against other threads;
*    If the entire stack executes from a single task, no
*    functionality is required here.
*/
void IP_OS_Lock(void) {
  (void)OS_Use(&IP_OS_RSema);
}

/*********************************************************************
*
*       IP_OS_Unlock()
*
*  Function description
*    Unlocks the single lock, locked by a previous call to IP_OS_Lock()
*    and signals the IP_Task() if a packet has been freed.
*/
void IP_OS_Unlock(void) {
  int Status;

  //
  // Read the current lock count before unlocking to prevent
  // directly being locked again by a higher priority task.
  //
  Status = OS_GetSemaValue(&IP_OS_RSema);
  OS_Unuse(&IP_OS_RSema);
  //
  // If this would have been the last unlock we check if we need to
  // to signal the IP_Task().
  //
  if ((Status - 1) == 0) {
    IP_SignalIfPacketFreeUsed();
  }
}

/*********************************************************************
*
*       IP_OS_AssertLock()
*
*  Function description
*    Makes sure that the lock is in use. Called in debug builds only.
*/
void IP_OS_AssertLock(void) {
  if (IP_OS_RSema.UseCnt == 0) {
    for (;;) {
      // Allows setting a breakpoint here.
    }
  } else {
    if (IP_OS_RSema.pTask != OS_GetpCurrentTask()) {
      for (;;) {
        // Allows setting a breakpoint here.
      }
    }
  }
}

/*********************************************************************
*
*       IP_OS_WaitItemTimed()
*
*  Function description
*    Suspend a task which needs to wait for a object;
*    This object is identified by a pointer to it and can be of any
*    type, for example a socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*    Timeout  : Timeout for waiting in system ticks (typically 1ms).
*
*  Additional information
*    Function is called from an application task and is locked in
*    every case.
*/
void IP_OS_WaitItemTimed(void* pWaitItem, unsigned Timeout) {
  TCP_WAIT TCPWait;

#if (TICK_MULTIPLICATOR != 1)
  Timeout = Timeout * TICK_MULTIPLICATOR;
#endif
  //
  // Create the wait object which contains the OS-Event object.
  //
  TCPWait.pWaitItem = pWaitItem;
  OS_EVENT_Create(&TCPWait.Event);
  //
  // Add to beginning of doubly-linked list.
  //
  TCPWait.pPrev = NULL;
#if (OS_VERSION >= 38606)  // OS_AddOnTerminateHook() is supported since embOS v3.86f .
  TCPWait.pTask = OS_GetpCurrentTask();
  OS_EnterRegion();        // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
#endif
  TCPWait.pNext = _pTCPWait;
  _pTCPWait     = &TCPWait;
  if (TCPWait.pNext) {
    TCPWait.pNext->pPrev = &TCPWait;
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
  //
  // Unlock mutex.
  //
  IP_OS_UNLOCK();
  //
  //  Suspend this task.
  //
  if (Timeout == 0) {
    OS_EVENT_Wait(&TCPWait.Event);
  } else {
    (void)OS_EVENT_WaitTimed(&TCPWait.Event, Timeout);
  }
  //
  // Lock the mutex again.
  //
  IP_OS_LOCK();  //lint !e534 unused return value.
  //
  // Remove it from doubly linked list and delete event object.
  //
#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  _DLIST_RemoveDelete(&TCPWait);
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
}

/*********************************************************************
*
*       IP_OS_WaitItem()
*
*  Function description
*    Suspend a task which needs to wait for an object;
*    This object is identified by a pointer to it and can be of any
*    type, for example socket.
*
*  Parameters
*    pWaitItem: Item to wait for.
*
*  Additional information
*    Function is called from an application task.
*/
void IP_OS_WaitItem(void* pWaitItem) {
  IP_OS_WaitItemTimed(pWaitItem, 0);
}

/*********************************************************************
*
*       IP_OS_SignalItem()
*
*  Function description
*    Sets an object to signaled state, or resumes tasks which are
*    waiting at the event object.
*
*  Parameters
*    pWaitItem: Item to signal.
*
*  Additional information
*    Function is called from a task, not an ISR and is locked in
*    every case.
*/
void IP_OS_SignalItem(void* pWaitItem) {
  TCP_WAIT* pTCPWait;

#if (OS_VERSION >= 38606)  // Disable task switching to prevent being preempted by a task being killed while modifying the linked list.
  OS_EnterRegion();
#endif
  for (pTCPWait = _pTCPWait; pTCPWait; pTCPWait = pTCPWait->pNext) {
    if (pTCPWait->pWaitItem == pWaitItem) {
      OS_EVENT_Set(&pTCPWait->Event);
    }
  }
#if (OS_VERSION >= 38606)
  OS_LeaveRegion();
#endif
}

/*********************************************************************
*
*       IP_OS_AddTickHook()
*
*  Function description
*    Add tick hook. This is a function which is called from the tick
*    handler, typically because the driver's interrupt handler is not
*    called via it's own hardware ISR. (We poll 1000 times per second)
*
*  Parameters
*    pfHook: Callback to be called on every tick.
*
*  Additional information
*    Function is called from a task, not an ISR.
*/
void IP_OS_AddTickHook(void (*pfHook)(void)) {
#if (OS_VERSION >= 36000)
  static OS_TICK_HOOK _cb;
  OS_AddTickHook(&_cb, pfHook);
#else
  IP_PANIC("IP_OS_AddTickHook() requires an OS version >= 3.60");  // This requires a newer version of the OS.
#endif
}

/*********************************************************************
*
*       IP_OS_GetTaskName()
*
*  Function description
*    Retrieves the task name (if available from the OS and not in
*    interrupt) for the currently active task.
*
*  Parameters
*    pTask: Pointer to a task identifier such as a task control block.
*
*  Return value
*    Terminated string with task name.
*/
const char* IP_OS_GetTaskName(void* pTask) {
  return OS_GetTaskName((OS_TASK*)pTask);
}

/*************************** End of file ****************************/
