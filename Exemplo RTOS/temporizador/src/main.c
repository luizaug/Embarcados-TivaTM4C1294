#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS

uint8_t stateLED1=0;
uint8_t stateLED2=0;
uint8_t stateLED3=0;
uint8_t stateLED4=0;
uint8_t timeMultiplier;

osTimerId_t timer1_id;
osThreadId_t app_main_id;
osMutexId_t mutexLED;

const osMutexAttr_t MutexLED_attr = {"MutexLED", osMutexRecursive | osMutexPrioInherit, NULL, 0U};

void callback(void *arg){
  if(timeMultiplier%2==0){
    stateLED1^=LED1;
    LEDWrite(LED1,stateLED1);
  }
  if (timeMultiplier%4==0){
    stateLED2^=LED2;
    LEDWrite(LED2,stateLED2);
  }
  if (timeMultiplier%8==0){
    stateLED3^=LED3;
    LEDWrite(LED3,stateLED3);
  }
  if (timeMultiplier%16==0){
    stateLED4^=LED4;
    LEDWrite(LED4,stateLED4);
  }
  timeMultiplier++;
} // callback

void app_main(void *arg){
  osTimerStart(timer1_id, 50);
  osDelay(osWaitForever);
} // app_main

void main(void){
  SystemInit();
  LEDInit(LED1);
  LEDInit(LED2);
  LEDInit(LED3);
  LEDInit(LED4);

  osKernelInitialize();

  app_main_id = osThreadNew(app_main, NULL, NULL);
  timer1_id = osTimerNew(callback, osTimerPeriodic, NULL, NULL);
  mutexLED = osMutexNew(&MutexLED_attr);

  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
