#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"

uint32_t SystemCoreClock;

void SystemInit(void){
  SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              24000000); // 24MHz
} // SystemInit

void SystemCoreClockUpdate(void){
  // Not implemented
} // SystemCoreClockUpdate
