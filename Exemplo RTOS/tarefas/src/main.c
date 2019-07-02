#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "cmsis_os2.h" // CMSIS-RTOS
#define BUFFER_SIZE 15

osThreadId_t threadProdutor_id,threadConsumidor_id;
osSemaphoreId_t semaphoreVazio_id, semaphoreCheio_id;
uint8_t buffer[BUFFER_SIZE];

void threadProdutor(void *arg){
  uint8_t count, index = 0;
  while(1){
    osSemaphoreAcquire(semaphoreVazio_id,osWaitForever);
    if(index >= BUFFER_SIZE){
      index = 0;
    }
    buffer[index] = count;
    count++;
    count &= 0x0F;
    osSemaphoreRelease(semaphoreCheio_id);
    osDelay(1000);
  } // while
} // threadLED

void threadConsumidor(void *arg){
  uint8_t index,state = 0;
  while(1){
    osSemaphoreAcquire(semaphoreCheio_id,osWaitForever);
    index = osSemaphoreGetCount(semaphoreCheio_id);
    state = buffer[index];
    LEDWrite(LED4|LED3|LED2|LED1,state);
    osSemaphoreRelease(semaphoreVazio_id);
    osDelay(250);
  } // while
} // threadLED

void main(void){
  SystemInit();
  LEDInit(LED1|LED2|LED3|LED4);

  osKernelInitialize();

  threadProdutor_id = osThreadNew(threadProdutor, NULL, NULL);
  threadConsumidor_id = osThreadNew(threadConsumidor, NULL, NULL);
  
  semaphoreCheio_id = osSemaphoreNew(BUFFER_SIZE,0,NULL);
  semaphoreVazio_id = osSemaphoreNew(BUFFER_SIZE,BUFFER_SIZE,NULL);
    
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
