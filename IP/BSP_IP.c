/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File    : BSP_IP.c
Purpose : IP related BSP routines for ST STM32F40G-EVAL .
*/

#include "BSP_IP.h"
#include "SEGGER.h"
#include "RTOS.h"

#include "stm32f4xx.h"

/*********************************************************************
*
*       Types, local
*
**********************************************************************
*/

typedef void ETH_ISR_HANDLER(void);

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static ETH_ISR_HANDLER* _pfEthISRHandler;

/*********************************************************************
*
*       Prototypes
*
*  Declare ISR handler here to avoid "no prototype" warning.
*  They are not declared in any CMSIS header.
*
**********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

void ETH_IRQHandler(void);

#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Local functions
*
**********************************************************************
*/

/*********************************************************************
*
*       _cbInit()
*
*  Function description
*    This routine is called from the Ethernet driver to initialize
*    port pins and enable clocks required for Ethernet.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*
*  Notes:
*    (1) RMII pin configuration
*          Port    Pin function
*            PA1     RMII_REF_CLK
*            PA2     RMII_MDIO
*            PA7     RMII_CRS_DV
*            PC1     RMII_MDC
*            PC4     RMII_RXD0
*            PC5     RMII_RXD1
*            PB10    RMII_RXER
*            PB11    RMII_TX_EN
*            PB12    RMII_TXD0
*            PB13    RMII_TXD1
*/
static void _cbInit(unsigned IFaceId) {
  U32 v;

  SEGGER_USE_PARA(IFaceId);

  //
  // Enable I/O clocks
  //
  RCC->AHB1ENR |= (0uL
                   | (1uL << 1)  // GPIOAEN: IO port B clock enable.
                   | (1uL << 2)  // GPIOAEN: IO port C clock enable.
                   | (1uL << 0)  // GPIOAEN: IO port A clock enable.
                  );
  //
  // Set PA1 (RMII_REF_CLK) as alternate function.
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 1u));
  v             |=  (0x2uL << (2u * 1u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 1u));
  v             |=  (0xBuL << (4u * 1u));
  GPIOA->AFR[0]  = v;
  //
  // Set PA2 (RMII_MDIO) as alternate function.
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 2u));
  v             |=  (0x2uL << (2u * 2u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 2u));
  v             |=  (0xBuL << (4u * 2u));
  GPIOA->AFR[0]  = v;
  //
  // Set PA7 (RMII_CRS_DV) as alternate function.
  //
  v              = GPIOA->MODER;
  v             &= ~(0x3uL << (2u * 7u));
  v             |=  (0x2uL << (2u * 7u));
  GPIOA->MODER   = v;
  v              = GPIOA->AFR[0];
  v             &= ~(0xFuL << (4u * 7u));
  v             |=  (0xBuL << (4u * 7u));
  GPIOA->AFR[0]  = v;
  //
  // Set PC1 (RMII_MDC) as alternate function.
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 1u));
  v             |=  (0x2uL << (2u * 1u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 1u));
  v             |=  (0xBuL << (4u * 1u));
  GPIOC->AFR[0]  = v;
  //
  // Set PC4 (RMII_RXD0) as alternate function.
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 4u));
  v             |=  (0x2uL << (2u * 4u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 4u));
  v             |=  (0xBuL << (4u * 4u));
  GPIOC->AFR[0]  = v;
  //
  // Set PC5 (RMII_RXD1) as alternate function.
  //
  v              = GPIOC->MODER;
  v             &= ~(0x3uL << (2u * 5u));
  v             |=  (0x2uL << (2u * 5u));
  GPIOC->MODER   = v;
  v              = GPIOC->AFR[0];
  v             &= ~(0xFuL << (4u * 5u));
  v             |=  (0xBuL << (4u * 5u));
  GPIOC->AFR[0]  = v;
  //
  // Set PB10 (RMII_RXER) as alternate function.
  //
  v              = GPIOB->MODER;
  v             &= ~(0x3uL << (2u * 10u));
  v             |=  (0x2uL << (2u * 10u));
  GPIOB->MODER   = v;
  v              = GPIOB->AFR[1];
  v             &= ~(0xFuL << (4u * 2u));
  v             |=  (0xBuL << (4u * 2u));
  GPIOB->AFR[1]  = v;
  //
  // Set PB11 (RMII_TX_EN) as alternate function, speed 100MHz.
  //
  v               = GPIOB->MODER;
  v              &= ~(0x3uL << (2 * 11));
  v              |=  (0x2uL << (2 * 11));
  GPIOB->MODER    = v;
  GPIOB->OSPEEDR |= (0x3uL << (2 * 11));
  v               = GPIOB->AFR[1];
  v              &= ~(0xFuL << (4 * 3));
  v              |=  (0xBuL << (4 * 3));
  GPIOB->AFR[1]   = v;
  //
  // Set PB12 (RMII_TXD0) as alternate function, speed 100MHz.
  //
  v               = GPIOB->MODER;
  v              &= ~(0x3uL << (2 * 12));
  v              |=  (0x2uL << (2 * 12));
  GPIOB->MODER    = v;
  GPIOB->OSPEEDR |= (0x3uL << (2 * 12));
  v               = GPIOB->AFR[1];
  v              &= ~(0xFuL << (4 * 4));
  v              |=  (0xBuL << (4 * 4));
  GPIOB->AFR[1]   = v;
  //
  // Set PB13 (ETH_RMII_TXD1) as alternate function, speed 100MHz.
  //
  v               = GPIOB->MODER;
  v              &= ~(0x3uL << (2 * 13));
  v              |=  (0x2uL << (2 * 13));
  GPIOB->MODER    = v;
  GPIOB->OSPEEDR |= (0x3uL << (2 * 13));
  v               = GPIOB->AFR[1];
  v              &= ~(0xFuL << (4 * 5));
  v              |=  (0xBuL << (4 * 5));
  GPIOB->AFR[1]   = v;
}

/*********************************************************************
*
*       _cbDeInit()
*
*  Function description
*    This routine is called from the Ethernet driver to de-initialize
*    the hardware upon calling IP_DeInit() .
*
*  Parameters
*    IFaceId: Zero-based interface index.
*
*  Additional information
*    A typical de-initialization needs to remove its previously
*    installed interrupts and should reset any kind of guard
*    variables to their reset state.
*    Resetting pin configurations is typically optional as it is
*    assumed that a de-init in most cases is part of a re-init anyhow.
*/
static void _cbDeInit(unsigned IFaceId) {
  SEGGER_USE_PARA(IFaceId);

  NVIC_DisableIRQ(ETH_IRQn);
  _pfEthISRHandler = NULL;
}

/*********************************************************************
*
*       _cbInstallISR()
*
*  Function description
*    Installs the IP stack interrupt handler for Ethernet.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*    pPara  : Parameters for the interrupt handler to install:
*               - pfISR   : Pointer to the interrupt handler of the
*                           IP stack.
*               - ISRIndex: Default interrupt index as far as known by
*                           the driver.
*                           For drivers that fit multiple devices this
*                           might not always be the correct index and
*                           should then be replaced by the correct
*                           index within this function.
*                - Prio   : Suggested priority by the IP stack. This
*                           is only a suggestion and should be
*                           replaced with the desired priority within
*                           this function.
*/
static void _cbInstallISR(unsigned IFaceId, BSP_IP_INSTALL_ISR_PARA* pPara) {
  SEGGER_USE_PARA(IFaceId);

  _pfEthISRHandler = pPara->pfISR;
  NVIC_SetPriority(ETH_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
  NVIC_EnableIRQ(ETH_IRQn);
}

/*********************************************************************
*
*       _cbGetMiiMode()
*
*  Function description
*    This routine is called from the Ethernet driver to ask for the
*    MII mode (MII or RMII) to use.
*
*  Parameters
*    IFaceId: Zero-based interface index.
*
*  Return value
*    0: MII  mode.
*    1: RMII mode.
*/
static unsigned _cbGetMiiMode(unsigned IFaceId) {
  SEGGER_USE_PARA(IFaceId);

  return 1u;  // RMII mode.
}

/*********************************************************************
*
*       _cbGetEthClock()
*
*  Function description
*    This routine is called from the Ethernet driver to ask for the
*    clock frequency [Hz] used by the Ethernet peripheral.
*
*  Return value
*    Ethernet peripheral clock [Hz].
*
*  Additional information
*    Typically the Ethernet clock is equal to the system clock but
*    it does not have to be.
*/
static unsigned long _cbGetEthClock(void) {
  return SystemCoreClock;
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/

/*********************************************************************
*
*       ETH_IRQHandler()
*
*  Function description
*    Ethernet interrupt handler.
*
*  Additional information
*    Needs to inform the OS that we are now in interrupt context.
*/
void ETH_IRQHandler(void) {
  OS_EnterInterrupt();
  if (_pfEthISRHandler) {
    (_pfEthISRHandler)();
  }
  OS_LeaveInterrupt();
}

/*********************************************************************
*
*       Public API structures
*
**********************************************************************
*/

const BSP_IP_API BSP_IP_Api = {
  _cbInit,        // pfInit
  _cbDeInit,      // pfDeInit
  _cbInstallISR,  // pfInstallISR
  _cbGetMiiMode,  // pfGetMiiMode
  _cbGetEthClock  // pfGetEthClock
};

/*************************** End of file ****************************/
