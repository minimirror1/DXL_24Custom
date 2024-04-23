/*
 * schedule_main.cpp
 *
 *  Created on: Apr 23, 2024
 *      Author: minim
 */


#include "main.h"


#include "string.h"
/* RTOS ----------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

/* User Task -----------------------------------------------------------------*/
#include "schedule_main.h"
#include "dxl_main.h"

/* Component -----------------------------------------------------------------*/
#include "UART_Class.h"
#include "DXL_Class.h"
#include "DXL_Manager.h"
#include "cpp_tick.h"



extern osMessageQueueId_t moveQueueHandle;
extern osMessageQueueId_t okQueueHandle;

typedef struct _motionTypeDef{
	 int walkCnt;
	 uint8_t rpm;
	 uint32_t delay;
}motionTypeDef;

#define LIST_SIZE 100
uint8_t listCnt = 0;
motionTypeDef motionList[LIST_SIZE] = {0,};

void motionAdd(int walkCnt, uint8_t rpm, uint32_t delay){
	if(listCnt >= LIST_SIZE)
		return;

	motionList[listCnt].walkCnt = walkCnt;
	motionList[listCnt].rpm = rpm;
	motionList[listCnt].delay = delay;
	listCnt++;
}
void motionDelay(uint32_t delay){
	if(listCnt >= LIST_SIZE)
		return;

	motionList[listCnt].walkCnt = 0;
	motionList[listCnt].rpm = 0;
	motionList[listCnt].delay = delay;
	listCnt++;
}

void motionInit(){
	motionAdd(5,5,0);
	motionAdd(0,3,0);
	motionDelay(5000);
	motionAdd(1,2,0);
	motionAdd(2,5,0);
	motionAdd(3,10,0);
	motionDelay(5000);
	motionAdd(5,5,0);
	motionAdd(1,5,0);
}


uint8_t nextCnt = 0;
void nextMotion(void){


	if(motionList[nextCnt].delay != 0){
		osDelay(motionList[nextCnt].delay);
		nextCnt++;
			if(nextCnt >=listCnt) nextCnt = 0;
	}

	move_typedef moveTemp;
	moveTemp.rpm = motionList[nextCnt].rpm;
	moveTemp.walkCnt = motionList[nextCnt].walkCnt;
	osMessageQueuePut(moveQueueHandle, &moveTemp, 0U, 0U);

	nextCnt++;
	if(nextCnt >=listCnt) nextCnt = 0;
}

void schedule_main(void *argument)
{
  /* USER CODE BEGIN schedule_main */
	osDelay(8000);
	motionInit();
	nextMotion();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
	osStatus_t status = osStatusReserved;
    do {
    	uint16_t temp;
    	status = osMessageQueueGet(okQueueHandle, &temp, NULL, 0U); // wait for message
        if (status == osOK) {
        	osDelay(100);
        	nextMotion();

		}
    } while (status == osOK); // 큐가 비어있지 않는 동안 계속 반복
  }
  /* USER CODE END schedule_main */
}

