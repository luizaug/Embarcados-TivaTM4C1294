#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS


//#include <stdbool.h>
//#include "inc/hw_memmap.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/gpio.h"


extern void mydelay(uint32_t mstime);

osThreadId_t thread1_id, thread2_id;

void thread1(void *argument){
  while(1){
    LEDOn(LED1);
//        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
    mydelay(100);
    LEDOff(LED1);
//        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
    mydelay(100);
  } // while
} // thread1

void thread2(void *argument){
  while(1){
    LEDOn(LED2);
//        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_PIN_5);
    mydelay(100);
    LEDOff(LED2);
//        GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
    mydelay(100);
  } // while
} // thread1

void main(void){
  SystemInit();
  LEDInit(LED1|LED2);


//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // Habilita GPIO C
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)); // Aguarda final da habilitação
//    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4); // PC4 como saída
//    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, 0);
//    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_5); // PC5 como saída
//    GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_5, 0);
 
  
  osKernelInitialize(); // Initialize CMSIS-RTOS

  thread1_id = osThreadNew(thread1, NULL, NULL); // Create application main thread
  thread2_id = osThreadNew(thread2, NULL, NULL); // Create application main thread

  if(osKernelGetState() == osKernelReady)
    osKernelStart(); // Start thread execution

  while(1); // Execution should never get here
} // main
