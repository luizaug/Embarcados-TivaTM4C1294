#include <stdbool.h>              // Defines boolean variables
#include <stdint.h>               // Specifies int variables sizes
#include <math.h>                 // 
#include "inc/tm4c1294ncpdt.h"    // Register definitions for TM4C1294NCPDT
#include "inc/hw_memmap.h"        // Memory map for each peripheral
#include "inc/hw_types.h"         // Hardware access macros (direct & bit-band)
#include "driverlib/pin_map.h"    // Peripheral pin's map
#include "driverlib/uart.h"       // UART specific definitions and macros
#include "driverlib/sysctl.h"     // SystemControl API definitions
#include "driverlib/gpio.h"       // GPIO API definitions
#include "utils/uartstdio.h"      // UART STDIO API (printf mandatory)
#include "driverlib/timer.h"      // Timer API definitions
#include "inc/hw_timer.h"         // Timer Direct Access Adresses
#include "driverlib/interrupt.h"  // NVIC API definitions
#include "driverlib/systick.h"    // SysTick API definitions

/****************************** GLOBAL VARIABLES ******************************/
uint32_t regBorderDetector = 0;
uint32_t bufferBorderDetector = 0;
uint32_t count = 0;
uint32_t borderOverflow = 0;
uint32_t systickCounter = 0;

bool isSysTick = false;
bool isHertz = true;

/***************************** INTERRUPT HANDLERS *****************************/
/** SYSTICK **/
void SysTick_Handler(void){
  // Allways get a snapshot of the current counting register, to avoid
  // counting blindness while handling SysTick interrupt. It may loose some
  // counts because of stacking process before entering the SysTick Handler
  bufferBorderDetector = TimerValueGet(TIMER0_BASE,TIMER_A);
  if (++systickCounter == 10) {
    count = borderOverflow;
    regBorderDetector = bufferBorderDetector;
    isSysTick = true;
    HWREG(TIMER0_BASE + TIMER_O_TAV) = 0;
    HWREG(TIMER0_BASE + TIMER_O_TAR) = 0;
    borderOverflow = 0;
    systickCounter = 0;
  }
}

/** TIMER 0 **/
void hardwareOverflow(void){
  borderOverflow++;
  TimerIntClear(TIMER0_BASE,TIMER_CAPA_MATCH);
}

/*********************************** MAIN *************************************/
void main(void){
  // PLL at 2*(2^24) MHz, make possible to SysTick count up to 500ms.
  // A edge detection needs at least 2 clock cycles, so Fmax =~ 16.77 MHz
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              120000000);

  /** LOCAL VARIABLES **/
  // this variable is used to count the numbers of rising-edges detected
  uint32_t edgeCount = 0;

  /** SWITCH SETUP **/
  // Enables GPIO J and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
  // SW1 (PJ0) set as input in order to change the current timebase (freq. scale)
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


  /** TIMER AND SYSTICK INITIALIZATION **/
  SysTickEnable();
  // Tsystick = 100ms (max 16_777_216)
  SysTickPeriodSet(12000000);
  // Timer 0 uses pin L4 as capture port (T0CCP0).
  // This enables GPIO L, waits for confirmation and set special function to L4
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOL));
  GPIOPinConfigure(GPIO_PL4_T0CCP0);
  GPIOPinTypeTimer(GPIO_PORTL_BASE, GPIO_PIN_4);
  // Enable Timer 0 and wait for confirmation.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
  // Timer 0 configuration: Positive up-counting edges.
  TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP));
  TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
  // Timer A Preescale Register
  //HWREG(TIMER1_BASE+TIMER_O_TAPR) = 0x01;
  TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0x01);
  // Timer A Interval Load Register
  //HWREG(TIMER1_BASE+TIMER_O_TAILR) = 0xFFFF;
  TimerLoadSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFF);
  // Timer A Match Register
  //HWREG(TIMER1_BASE+TIMER_O_TAMATCHR) = 0xFFFFFFFE;
  TimerMatchSet(TIMER0_BASE, TIMER_A, 0xFFFFFFFE);
  // Timer A Preescale Match
  //HWREG(TIMER1_BASE+TIMER_O_TAPMR) = 0x00000000;
  TimerPrescaleMatchSet(TIMER0_BASE, TIMER_A, 0x00);
  // Interrupt enable
  TimerIntEnable(TIMER0_BASE,TIMER_CAPA_MATCH);
  IntEnable(INT_TIMER0A);
  IntMasterEnable();
  SysTickIntEnable();
  TimerEnable(TIMER0_BASE, TIMER_A);

/** ENDLESS LOOP **/
  while(1){
    // Systick flag pooling
    if(isSysTick){
      // Counting consolidation
      edgeCount = count*(pow(2,16)-1);
      edgeCount += regBorderDetector;

      // Serial printing (with correct scale label)
      UARTprintf("Frequencia: %d ", isHertz ? edgeCount : edgeCount/1000);
      if(isHertz){
        UARTprintf("[Hz]\n");
      }
      else{
        UARTprintf("[kHz]\n");
      }

      // Test SW1 (PJ0 pull-up), to toogle scale
      if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0) {
        isHertz = !isHertz;
      }

      isSysTick = false;
    }
  }
} // main
