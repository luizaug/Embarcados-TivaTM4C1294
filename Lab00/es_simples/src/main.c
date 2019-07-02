#include <stdbool.h>              // Defines boolean variables
#include <stdint.h>               // Specifies int variables sizes
#include "inc/tm4c1294ncpdt.h"    // Register definitions for TM4C1294NCPDT
#include "inc/hw_memmap.h"        // Memory map for each peripheral
#include "inc/hw_types.h"         // Hardware access macros (direct & bit-band)
#include "driverlib/pin_map.h"    // Peripheral pin's map
#include "driverlib/uart.h"       // UART specific definitions and macros
#include "driverlib/sysctl.h"     // SystemControl API definitions
#include "driverlib/gpio.h"       // GPIO API definitions
#include "utils/uartstdio.h"      // UART STDIO API (printf mandatory)

// Values determined via simulation and analisys of CYCLE COUNTER register
// for the average duration of a acquisition cycle. (923073 w/o debounce)
#define HzValue 923073
#define kHzValue 923


/************************************ MAIN ************************************/
void main(void){
  /** SYSTEM CLOCK SETUP **/
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              24000000);  // PLL set at 24MHz
  /** INITIAL VARIABLE SETUP **/
  // Default frequency scale: Hz
  uint32_t scaleValue = HzValue;
  // This variable will be used to control the acquisition loop duration
  uint32_t loopCount;
  // This variable is used to count events on a single acquisition loop.
  uint32_t eventCounter = 0;
  // These variables are used to detect a rising-edge event on the externalPin
  uint8_t newSample = 0;
  uint8_t oldSample = 0;
 // this variable is used to debounce SW1
  bool SWlock = false; 
  
  /** GPIO SETUP **/
  // Enables GPIO N and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION));

  // externalPin (PN3) set as input in order to measure external frequencies
  GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_3);
  GPIOPadConfigSet(GPIO_PORTN_BASE, GPIO_PIN_3,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  // Enables GPIO J and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));

  // SW1 (PJ0) set as input in order to chnge the current timebase (freq. scale)
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0,
                   GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  /** UART SETUP **/
  // Enables GPIO A (UART0) and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

  // Enables UART0 peripheral and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

  // Enables PA1 as TX pin for UART0
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_1);

  // General configurations for UART0 (baud rate & clock source)
  // Note that the Precision Internal Oscilator runs at 16 MHz (Datasheet p.69)
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTStdioConfig(0, 115200, 16000000);

  /** ENDLESS LOOP **/
  while(1){
    for(loopCount=HzValue;loopCount>0;loopCount--){
      // Acquire the externalPin (PN3)
      newSample = GPIOPinRead(GPIO_PORTN_BASE,GPIO_PIN_3);
      // Test a risging-edge event
      if(newSample > oldSample){
        // Rising-edge detected, so we register an event
        eventCounter++;
      }
      // As the acquisition cycle is over, the sample becomes old
      oldSample = newSample;
    }
    
    // The acquisition loop is over, time to print results
    UARTprintf("Frequencia: %d ", scaleValue==HzValue?eventCounter:eventCounter/1000);
    
    // This block prints the correct scale unit
    if(scaleValue == HzValue){
      UARTprintf("[Hz]\n");
    }
    else{
      UARTprintf("[kHz]\n");
    }
    
    // Test SW1 (PJ0 pull-up), to change scaleValue
    if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0 && 
        SWlock == false){
      // Toogle scaleValue.
      if(scaleValue == HzValue)
        scaleValue = kHzValue;
      else
        scaleValue = HzValue;    
      SWlock=true;
    }
    else if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0)
       SWlock=false;
    eventCounter = 0;
  } // while
} // main

/*
PERGUNTAS:
1 - Como adicionar a biblioteca utils/uartstdio da mesma forma como as demais 
    bibliotecas (arquivo .a)
2 - Por que o simulador não funciona com a biblioteca uartstdio (trava se não 
    comentar a linha UARTStdioConfig(0, 115200, 16000000);
3 - Por que adicionar um pseudo-debounce dentro de um if que nunca executa (por
    exemplo dentro da verificação de uma chave que nunca foi apertada) pode 
    alterar a temporização e interferir na contagem de eventos? [compilador ?]
4 - Como fazer o debounce de SW1 ? De forma geral,  
*/