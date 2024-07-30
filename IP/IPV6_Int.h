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

File    : IPV6_Int.h
Purpose : IPv6 header file.
*/
#ifndef IPV6_INT_H
#define IPV6_INT_H                // Avoid multiple/recursive inclusion

#include "IP_Int.h"
#include "IPV6_IPv6.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef IPV6_MAX_PREFIX_LEN
  #define IPV6_MAX_PREFIX_LEN       (8)
#endif

#ifndef IPV6_ADDR_LIFETIME_MAX
  #define IPV6_ADDR_LIFETIME_MAX    (0xFFFFFFFFu)
#endif

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/

/*********************************************************************
*
*       Neighbor Discovery protocol
*/

#define IPV6_NDP_ROUTER_SOLICITATION                                       (133)
#define IPV6_NDP_ROUTER_ADVERTISEMENT                                      (134)
#define IPV6_NDP_NEIGHBOR_SOLICITATION                                     (135)
#define IPV6_NDP_NEIGHBOR_ADVERTISEMENT                                    (136)
#define IPV6_NDP_REDIRECT                                                  (137)

#define IPV6_NDP_REACHABILITY_STATE_INCOMPLETE                               (1)   // Address resolution is in progress and the link-layer address
                                                                                   // of the neighbor has not yet been determined.
#define IPV6_NDP_REACHABILITY_STATE_REACHABLE                                (2)   // Roughly speaking, the neighbor is known to have been reachable
                                                                                   // recently (within tens of seconds ago).
#define IPV6_NDP_REACHABILITY_STATE_STALE                                    (3)   // The neighbor is no longer known to be reachable but until traffic
                                                                                   // is sent to the neighbor, no attempt should be made to verify its
                                                                                   // reachability.
#define IPV6_NDP_REACHABILITY_STATE_DELAY                                    (4)   // The neighbor is no longer known to be reachable, and
                                                                                   // traffic has recently been sent to the neighbor.
                                                                                   // Rather than probe the neighbor immediately, however,
                                                                                   // delay sending probes for a short while in order to
                                                                                   // give upper-layer protocols a chance to provide
                                                                                   // reachability confirmation.
#define IPV6_NDP_REACHABILITY_STATE_PROBE                                    (5)   // The neighbor is no longer known to be reachable, and
                                                                                   // unicast Neighbor Solicitation probes are being sent to
                                                                                   // verify reachability.

//
// Autoconfiguration address states
//
#define IPV6_NDP_AUTOCONFIG_STATE_NOT_USED                                   (0)
#define IPV6_NDP_AUTOCONFIG_STATE_TENTATIVE                                  (1)   // Address whose uniqueness on a link is being verified, prior to its assignment to an interface.
#define IPV6_NDP_AUTOCONFIG_STATE_PREFERRED                                  (2)   // Address assigned to an interface whose use by upper-layer protocols is unrestricted.
#define IPV6_NDP_AUTOCONFIG_STATE_DEPRECATED                                 (3)   // Address assigned to an interface whose use is discouraged, but not forbidden.
#define IPV6_NDP_AUTOCONFIG_STATE_VALID                                      (4)   // A preferred or deprecated address.
#define IPV6_NDP_AUTOCONFIG_STATE_INVALID                                    (5)   // Address that is not assigned to any interface.

//
//   Node constants:
//
#define IPV6_NDP_MAX_MULTICAST_SOLICIT                                         3   // 3 transmissions
#define IPV6_NDP_MAX_UNICAST_SOLICIT                                           3   // 3 transmissions
#define IPV6_NDP_MAX_NEIGHBOR_ADVERTISEMENT                                    3   // 3 transmissions
#define IPV6_NDP_REACHABLE_TIME                                            30000   // 30,000 milliseconds
#define IPV6_NDP_RETRANS_TIMER                                              1000   // 1,000 milliseconds

//
// NDP Options
//
#define IPV6_NDP_OPT_SOURCE_LINK_LAYER_ADDR                                  (1)   // Source Link-Layer Address                    1
#define IPV6_NDP_OPT_TARGET_LINK_LAYER_ADDR                                  (2)   // Target Link-Layer Address                    2
#define IPV6_NDP_OPT_PREFIX_INFORMATION                                      (3)   // Prefix Information                           3
#define IPV6_NDP_OPT_REDIRECT_HEADER                                         (4)   // Redirected Header                            4
#define IPV6_NDP_OPT_MTU                                                     (5)   // MTU                                          5
#define IPV6_NDP_OPT_RDNSS                                                   (25)  // Recursive DNS Server Option                 25
#define IPV6_NDP_OPT_DNSSL                                                   (31)  // DNS Search List Option                      31

//
// IPv6 extension header types
//
#define IPV6_EXT_HEADER_HOP_BY_HOP                                           (0)
#define IPV6_EXT_HEADER_ROUTING                                             (43)
#define IPV6_EXT_HEADER_FRAGMENT                                            (44)
#define IPV6_EXT_HEADER_NO_NEXT                                             (59)
#define IPV6_EXT_HEADER_DESTINATION_OPT                                     (60)

//
// Hop-by-Hop extension headers related defines
//
#define HOP_BY_HOP_OPT_PAD1                                                  (0)
#define HOP_BY_HOP_OPT_PADN                                                  (1)
#define HOP_BY_HOP_OPT_ROUTER_ALERT                                          (5)
//
// Router Alert options
//
#define ROUTER_ALERT_MLD_MSG                                                 (0)   // Multicast Listener Discovery message [RFC-2710].
#define ROUTER_ALERT_RSVP_MSG                                                (1)   // Datagram contains RSVP message.
#define ROUTER_ALERT_ACTIVE_NETWORK_MSG                                      (2)   // Datagram contains an Active Networks message.

//
// ICMPv6 related defines. Parameter problem error codes
//
#define ICMPV6_PARA_PROB_CODE_ERR_HEADER                                     (0)   // Erroneous header field encountered
#define ICMPV6_PARA_PROB_CODE_UNRECOGNIZED_NEXT_HEADER                       (1)   // Unrecognized Next Header type encountered
#define ICMPV6_PARA_PROB_CODE_UNRECOGNIZED_OPT                               (2)   // Unrecognized IPv6 option encountered


#define IP_ICMPV6_MLD_QUERY                                                (130)
#define IP_ICMPV6_MLD_REPORT                                               (131)
#define IP_ICMPV6_MLD_DONE                                                 (132)


/*********************************************************************
*
*       Types
*
**********************************************************************
*/

/*********************************************************************
*
*       IPV6_HEADER
*
*
*  Excerpt from RFC2460:
*
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |Version| Traffic Class |           Flow Label                  |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |         Payload Length        |  Next Header  |   Hop Limit   |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                         Source Address                        +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                      Destination Address                      +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef struct IPV6_HEADER {
  U32        VersionClassLabel;
  U16        PayloadLen;
  U8         NextHeader;
  U8         HopLimit;
  IPV6_ADDR  SrcAddr;
  IPV6_ADDR  DestAddr;
} IPV6_HEADER;

/*********************************************************************
*
*       IPV6_PSEUDO_HEADER
*
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                         Source Address                        +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +                      Destination Address                      +
*   |                                                               |
*   +                                                               +
*   |                                                               |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |                         Payload Length                        |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*   |         Reserved                              |  Next Header  |
*   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
typedef struct {
  IPV6_ADDR SrcAddr;
  IPV6_ADDR DestAddr;
  U32       PayloadLen;
  U32       NextHeader;
} IPV6_PSEUDO_HEADER;

//
// NDP table entry structure
//
typedef struct NDP_ENTRY {
  struct NDP_ENTRY* pNext;
  struct NDP_ENTRY* pPrev;
  //
  //
  //
  IPV6_ADDR        IPv6Addr;                   // Neighbors IPv6 address
  U8               HWAddr[6];                  // Physical address
  U8               State;                      // A Neighbor Cache entry can be in one of five states: INCOMPLETE, REACHABLE, STALE, DELAY, PROBE
  U8               IsRouter;                   // Flag indicating whether the neighbor is a router or a host.
  unsigned         tExpire;                    // Expiration time stamp, [ms].
  unsigned         SolicitationCnt;            // The number of unanswered probes.
  //
  //
  //
  IFACE*           pIFace;
  IP_PACKET*       pPending;                   // Packets waiting for resolution of this entry
  U8               NumPending;                 // Number of pending packets for this entry.
  //
  //
  //
  U8               UsedAfterRefreshCnt;        // Has this entry been used for sending since the last refresh from an incomimg packet ? Acts as counter for executions per seconds as well.
} NDP_ENTRY;


/*********************************************************************
*
*       Prefix handling related structures
*/
typedef struct {
  IP_DLIST_ITEM Link;
  U8            aPrefix[IPV6_MAX_PREFIX_LEN];  // By default, the maximum prefix length is 8 bytes.
  U8            PrefixLen;                     // Length of the prefix in bits
  U8            Onlink;
} IPV6_PREFIX;

typedef struct {
  union {
    IP_DLIST_HEAD   Head;
    IPV6_PREFIX*    pFirstAddr;
  } List;
  U16                   Limit;
  U16                   Cnt;
} IPV6_PREFIX_LIST;


/*********************************************************************
*
*       Router handling related structures
*/
typedef struct {
  IP_DLIST_ITEM Link;
  IPV6_ADDR     IPAddr;
  U32           PreferredLifeTime;
  U32           ValidLifetime;
  U16           State;
} IPV6_ROUTER_ADDR;

typedef struct {
  union {
    IP_DLIST_HEAD     Head;
    IPV6_ROUTER_ADDR* pFirstAddr;
  } List;
} IPV6_DEF_ROUTER_LIST;


/*********************************************************************
*
*       DNS related structures
*/
typedef struct {
  IP_DLIST_ITEM Link;
  IPV6_ADDR     IPAddr;
  U32           ValidLifetime;
} IPV6_DNS_SERVER_ADDR;

typedef struct {
  union {
    IP_DLIST_HEAD     Head;
    IPV6_ROUTER_ADDR* pFirstAddr;
  } List;
  U32                 NumDNSServer;
} IPV6_DNS_SERVER_LIST;


/*********************************************************************
*
*       Interface related structures
*/
typedef struct {
  IP_DLIST_ITEM     Link;
  IPV6_ADDR         IPAddr;
  U32               PreferredLifeTime;         // The length of time that a valid address is preferred (i.e., the time until deprecation).  When the preferred lifetime expires, the address becomes deprecated.
  U32               ValidLifetime;             // The length of time an address remains in the valid state (i.e., the time until invalidation).  The valid lifetime must be greater than or equal to the preferred lifetime.  When the valid lifetime expires, the address becomes invalid.
  U32               DADTransmitCnt;
  IPV6_ROUTER_ADDR* pRouter;
  IPV6_PREFIX*      pPrefix;
  U16               State;
} IPV6_ADDR_DESC;

typedef struct {
  IP_DLIST_ITEM Link;
  IPV6_ADDR     IPAddr;
  unsigned      tDelay;
} IPV6_MULTICAST_ADDR;

typedef struct {
  struct {
    union {
      IP_DLIST_HEAD     Head;
      IPV6_ADDR_DESC*   pFirstAddr;
    } List;
    U16                 Limit;
    U16                 Cnt;
  } UnicastAddr;
  struct {
    union {
      IP_DLIST_HEAD     Head;
      IPV6_ADDR_DESC*   pFirstAddr;
    } List;
    U16                 Limit;
    U16                 Cnt;
  } MulticastAddr;
  IPV6_DEF_ROUTER_LIST* pDefRouterList;
  IPV6_DNS_SERVER_LIST* pDNSServerList;
  IPV6_PREFIX_LIST*     pPrefixList;
  U8                    DefHopLimit;
  U32                   DefaultConfMask;
} IPV6_LINK_DESC;

/*********************************************************************
*
*       IPv6 internal constants
*/
#define IPV6_UNSPECIFIED_ADDR_INIT          { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
#define IPV6_MULTICAST_SOLICIDED_NODE_INIT  { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0x00, 0x00, 0x00 }
#define IPV6_MULTICAST_ALL_NODES_INIT       { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }
#define IPV6_MULTICAST_ALL_ROUTERS_INIT     { 0xFF, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }

extern const IPV6_ADDR IP_IPV6_UnspecifiedAddr;
extern const IPV6_ADDR IP_IPV6_MulticastSolicidedNode;
extern const IPV6_ADDR IP_IPV6_MulticastAllNodes;
extern const IPV6_ADDR IP_IPV6_MulticastAllRouters;

/*********************************************************************
*
*       IPv6 address helper macros
*/
#define IP_IP6_ADDR_IS_LINKLOCAL(pIPv6Addr)             (((pIPv6Addr)->Union.aU8[0]         == 0xFE)   \
                                                     && (((pIPv6Addr)->Union.aU8[1] & 0xC0) == 0x80))


#define IP_IP6_ADDR_IS_UNICAST_LINKLOCAL(pIPv6Addr)    (((pIPv6Addr)->Union.aU8[0]          == 0xFC)   \
                                                     || ((pIPv6Addr)->Union.aU8[0]          == 0xFD))

#define IP_IP6_ADDR_IS_MULTICAST(pIPv6Addr)             ((pIPv6Addr)->Union.aU8[0] == 0xFF)

#define IP_IP6_ADDR_IS_UNSPECIFIED(pIPv6Addr)           (((pIPv6Addr)->Union.aU32[0] == 0) &&         \
                                                        ((pIPv6Addr)->Union.aU32[1] == 0)  &&         \
                                                        ((pIPv6Addr)->Union.aU32[2] == 0)  &&         \
                                                        ((pIPv6Addr)->Union.aU32[3] == 0))

#define IP_IP6_ADDR_IS_GLOBAL(pIPv6Addr)                (((pIPv6Addr)->Union.aU8[0] & 0xF0) == 0x20)


#define IP_IP6_ADDR_SET_UNSPECIFIED(pIPv6Addr)          ((pIPv6Addr)->Union.aU32[0] = 0);              \
                                                        ((pIPv6Addr)->Union.aU32[1] = 0);              \
                                                        ((pIPv6Addr)->Union.aU32[2] = 0);              \
                                                        ((pIPv6Addr)->Union.aU32[3] = 0)


/*********************************************************************
*
*       API functions
*
**********************************************************************
*/

/*********************************************************************
*
*       IPv6 internal API functions
*/

//
// Network Discovery Protocol (NDP)
//
void             IP_ICMPV6_NDP_InitModule                      (void);
NDP_ENTRY*       IP_ICMPV6_NDP_FindNDPEntryByIP                (IPV6_ADDR* pIPv6Addr);
NDP_ENTRY*       IP_ICMPV6_NDP_GetFreeNDPEntry                 (void);
void             IP_ICMPV6_NDP_AddToUseList                    (NDP_ENTRY* pEntry);
void             IP_ICMPV6_NDP_Timer                           (void* PContext);
void             IP_ICMPV6_NDP_OnRx                            (U8 Type, IP_PACKET* pPacket);
void             IP_ICMPV6_NDP_AddEntryToFreeList              (NDP_ENTRY* pEntry);
void             IP_ICMPV6_NDP_RemoveEntryFromUseList          (NDP_ENTRY* pEntry);
int              IP_ICMPV6_NDP_SendNeighborSolicitationPacket  (unsigned IFaceId, IPV6_ADDR* pDestIP);

//
// Multicast Listener Discovery (MLD) protocol
//
int              IP_ICMPV6_MLD_AddMulticastAddr_NoLock         (unsigned IFaceId, IPV6_ADDR* pMultiCAddr);
void             IP_ICMPV6_MLD_ConfIFace                       (unsigned IFaceId, U8 UpdateHWFilter);
void             IP_ICMPV6_MLD_InitModule                      (void);
void             IP_ICMPV6_MLD_Timer                           (void* pContext);
void             IP_ICMPV6_MLD_OnRx                            (U8 Type, IP_PACKET* pPacket);
void             IP_ICMPV6_MLD_Query_OnRx                      (IP_PACKET* pPacket);
int              IP_ICMPV6_MLD_RemoveMulticastAddr_NoLock      (unsigned IFaceId, IPV6_ADDR* pIPv6Addr);
int              IP_ICMPV6_MLD_GenAndAddSolicitedNodeAddr      (unsigned IFaceId, IPV6_ADDR* pIPv6Addr);
int              IP_ICMPV6_MLD_GenAndAddSolicitedNodeAddrEx    (unsigned IFaceId, IPV6_ADDR* pIPv6Addr, unsigned UpdateFilters);

//
// Internet Control Message Protocol version 6 (ICMPv6)
//
void             IP_ICMPV6_Add                                 (void);
void             IP_ICMPV6_OnRx                                (IP_PACKET* pPacket);
int              IP_ICMPV6_Send                                (IP_PACKET* pPacket, IPV6_ADDR* pDestAddr);
void             IP_ICMPV6_GenCheckSum                         (IP_PACKET* pPacket, IPV6_ADDR* pSrcAddr, IPV6_ADDR* pDestAddr);
int              IP_ICMPV6_SendParaProbMsg                     (unsigned IFaceId, IP_PACKET *pProbPacket, U8 Code, U32 Offset);

//
// Internet Protocol version 6 (IPv6)
//
IPV6_PREFIX*          IP_IPV6_AllocAddrPrefixZeroed            (unsigned IFaceId);
IPV6_DNS_SERVER_ADDR* IP_IPV6_AllocDNSServerAddr               (unsigned IFaceId);
IPV6_ROUTER_ADDR*     IP_IPV6_AllocRouterAddr                  (unsigned IFaceId);
void                  IP_IPV6_FreeDNSServerAddr                (unsigned IFaceId, IPV6_DNS_SERVER_ADDR* pDNSServerAddr);
void                  IP_IPV6_OnRx                             (IP_PACKET* pPacket);
int                   IP_IPV6_Write                            (IP_PACKET* pPacket, U8 NextHeader, U8 HopLimit, IPV6_ADDR* pDestIP);
void                  IP_IPV6_AddExtHeader                     (IP_PACKET* pPacket, U8 ExtHeader, U8 NextHeader, U8 Type, U16 Value);
IPV6_ADDR_DESC*       IP_IPV6_GenUnicastAddr                   (unsigned IFaceId, U32 ValidLifetime, U32 PreferredLifetime, const IPV6_PREFIX* pPrefix, const IPV6_ROUTER_ADDR* pRouterAddr);
IFACE*                IP_IPV6_LAddr2IFace                      (U32 PFamily, void* pLHost);
IPV6_ADDR_DESC*       IP_IPV6_SelectSourceAddr                 (IFACE* pIFace, IPV6_ADDR* pDest, U8* pRouterReq);
IFACE*                IP_IPV6_GetIFaceForHost                  (IPV6_ADDR* pAddr);
void                  IP_IPV6_Timer                            (void);
int                   IP_IPV6_RegenerateUnicastAddresses       (unsigned IFaceId);
//
// UDPv6
//
IP_UDP_CONNECTION* IP_IPV6_UDP_OpenEx_NoLock(IPV6_ADDR* pFAddr, U16 FPort, IPV6_ADDR* pLAddr, U16 LPort, int (*handler)(IP_PACKET* pPacket, void* pContext), void* pContext);

//
// UDP function related defines
// (To avoid code doubling and present a clean API with convenient name prefix.)
//
#define IP_IPV6_UDP_Close_NoLock  IP_UDP_Close_NoLock

#if defined(__cplusplus)
  }      // Make sure we have C-declarations in C++ programs
#endif

#endif   // Avoid multiple inclusion

/****** End Of File *************************************************/
