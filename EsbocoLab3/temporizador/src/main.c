#include "system_tm4c1294.h"      // CMSIS-Core
#include <stdbool.h>              // Defines boolean variables
#include <stdint.h>               // Specifies int variables sizes
#include "driverlib/sysctl.h"     // SystemControl API definitions
#include "inc/hw_memmap.h"        // Memory map for each peripheral
#include "inc/hw_types.h"         // Hardware access macros (direct & bit-band)
#include "driverlib/timer.h"      // Timer API definitions
#include "inc/hw_timer.h"         // Timer Direct Access Adresses
#include "driverlib/interrupt.h"  // NVIC API definitions
#include "inc/hw_ints.h"
#include "driverleds.h"           // device drivers
#include "cmsis_os2.h"            // CMSIS-RTOS
#include "driverlib/pin_map.h"    // Peripheral pin's map
#include "driverlib/gpio.h"       // GPIO API definitions

uint8_t state = 0;
osTimerId_t timer1_id;
osThreadId_t app_main_id, app_sw_monitor_id, get_counter_id;
uint32_t borderOverflow = 0;
bool isHertz = true;

void callback(void *arg){
  osTimerStop(timer1_id);
  
  if(isHertz){
    osTimerStart(timer1_id,100000);
  }
  else {
    osTimerStart(timer1_id,100);
  }
  osThreadFlagsSet(get_counter_id, 0x0001);   // LEVANTA FLAG
} // callback

/** TIMER 0 **/
void hardwareOverflow(void){
  borderOverflow++;
  TimerIntClear(TIMER0_BASE,TIMER_CAPA_MATCH);
}

void app_main(void *arg){
  osTimerStart(timer1_id, 100000);
  osDelay(osWaitForever);
} // app_main

void app_sw_monitor(void *arg){
  uint8_t counter = 0;
  // Enables GPIO J and waits for confirmation before proceed
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ));
  // SW1 (PJ0) set as input in order to change the current timebase (freq. scale)
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0,
  GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  
  while(1){
    // Test SW1 (PJ0 pull-up), to toogle scale
    counter ++;
    if (counter == 10){
      counter = 0;
      if (!GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == GPIO_PIN_0) {
        isHertz = !isHertz;
      }
    }
  }
} // sw_setup

void get_counter(void *arg){
  while (1) {
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
  
    state ^= LED1;
    LEDWrite(LED1, state);
  }
} // get_counter

void setup_counter () {
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
  TimerEnable(TIMER0_BASE, TIMER_A);
}


void main(void){
  SystemInit();
  LEDInit(LED1);
  
  osKernelInitialize();
  app_sw_monitor_id = osThreadNew(app_sw_monitor,NULL,NULL);
  app_main_id = osThreadNew(app_main, NULL, NULL);
  get_counter_id = osThreadNew(get_counter, NULL, NULL);
  timer1_id = osTimerNew(callback, osTimerPeriodic, NULL, NULL);
  setup_counter();
  
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main

