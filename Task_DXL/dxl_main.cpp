/*
 * dxl_main.cpp
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

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
Serial serial1;
Serial serial2;
Serial serial3;
void Serial_Init(void);

DXL_Manager dxlManager;
void DXL_Manager_Init(void);

#define GROUPID 1

int32_t targetRPMcnt = 20;
void setSpeed(int32_t rpm){
	float temp = 0;


	temp = (float)rpm / 60; //rpm을 초당회전으로 변환
	temp *= 360;
	temp /= 100; // 초당 회전을 10ms당 회전으로 변경
	temp /= 0.087890625;//분해능

	//최대값 제한
	if(temp > 20)
		temp = 20;
	if(temp <= 1)
		temp = 1;

	targetRPMcnt = temp;
}

int32_t targetCnt = 0;
void setWalk(int32_t walkCnt){
	if(walkCnt > 5)
		walkCnt = 5;
	if(walkCnt < -5)
		walkCnt = -5;

	float angle = walkCnt * 180;
	angle /= 0.087890625;
	targetCnt = angle;
}

int currentCnt = 0;
void moveTarget_Process(void){
	static uint8_t okToggle = 1;

	if((targetCnt-targetRPMcnt-5) < currentCnt && currentCnt < (targetCnt+targetRPMcnt+5)){
		//위치 확정
		uint16_t msg = 1;
		if(okToggle == 1){
			okToggle = 0;
			osMessageQueuePut(okQueueHandle, &msg, 0U, 0U);
		}
	}else{
		okToggle = 1;
		if(currentCnt < targetCnt){
			currentCnt += targetRPMcnt;
		}
		else{
			currentCnt -= targetRPMcnt;
		}

		dxlManager.setRawPosition(1, currentCnt);
		dxlManager.setRawPosition(2, -currentCnt);
	}
}

move_typedef moveTemp;
void dxl_main(void *argument) {
	/* USER CODE BEGIN dxl_main */

	Serial_Init();
	DXL_Manager_Init();
	dxlManager.initializeAll_MultiTrun();

	dxlManager.setSettingInfo(
			GROUPID,
			1,
			DXL_ROT_CW,
			360,
			0,
			0
			);
	dxlManager.setSettingData_op(
			GROUPID,
			1,
			0,
			0
			);
	dxlManager.setSettingInfo(
			GROUPID,
			2,
			DXL_ROT_CW,
			360,
			0,
			0
			);
	dxlManager.setSettingData_op(
			GROUPID,
			2,
			0,
			0
			);

	//dxlManager.setDefaultPosi_Ready(GROUPID, 1);
	//dxlManager.setDefaultPosi_Ready(GROUPID, 2);

	dxlManager.setRawPosition(1, 0);
	dxlManager.setRawPosition(2, 0);
	osDelay(5000);
	setSpeed(0);
	setWalk(0);

	/* Infinite loop */
	for (;;) {
		osDelay(10);

		moveTarget_Process();
		dxlManager.allMotorProcess();
		serial1.rxLed_Check();
		serial2.rxLed_Check();
		serial3.rxLed_Check();

		osStatus_t status = osStatusReserved;
	    do {
	    	status = osMessageQueueGet(moveQueueHandle, &moveTemp, NULL, 0U); // wait for message
	        if (status == osOK) {
	        	setSpeed(moveTemp.rpm);
	        	setWalk(moveTemp.walkCnt);
			}
	    } while (status == osOK); // 큐가 비어있지 않는 동안 계속 반복
	}
	/* USER CODE END dxl_main */
}
void DXL_Manager_Init(void){
	uint8_t gID = GROUPID;

	for(int i = 1; i <= 10; i++)
		dxlManager.addDXLObject(gID, i, Motor::Robotis_Type, &serial1);
	for(int i = 11; i <= 20; i++)
		dxlManager.addDXLObject(gID, i, Motor::Robotis_Type, &serial2);
	for(int i = 21; i <= 30; i++)
		dxlManager.addDXLObject(gID, i, Motor::Robotis_Type, &serial3);
}
void Serial_Init(void) {
	/* serial1 [uart2] 1~10 */
	serial1.init(&huart2, USART2_IRQn);
	serial1.init_txLed(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
	serial1.init_rxLed(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	serial1.init_rs485(USART2_EN_GPIO_Port, USART2_EN_Pin);

	/* serial2 [uart3] 11~20 */
	serial2.init(&huart3, USART3_IRQn);
	serial2.init_txLed(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
	serial2.init_rxLed(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	serial2.init_rs485(USART3_EN_GPIO_Port, USART3_EN_Pin);

	/* serial3 [uart1] 21~30 */
	serial3.init(&huart1, USART1_IRQn);
	serial3.init_txLed(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
	serial3.init_rxLed(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	serial3.init_rs485(USART1_EN_GPIO_Port, USART1_EN_Pin);
}
/* HAL Driver Callback */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	serial1.TxCpltCallback(huart);
	serial2.TxCpltCallback(huart);
	serial3.TxCpltCallback(huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	serial1.RxCpltCallback(huart);
	serial2.RxCpltCallback(huart);
	serial3.RxCpltCallback(huart);
}

