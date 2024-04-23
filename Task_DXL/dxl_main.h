/*
 * dxl_main.h
 *
 *  Created on: Apr 23, 2024
 *      Author: minim
 */

#ifndef DXL_MAIN_H_
#define DXL_MAIN_H_


typedef struct {
	int walkCnt;
	uint8_t rpm;
}move_typedef;

#ifdef __cplusplus
extern "C" {
#endif

void dxl_main(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* DXL_MAIN_H_ */
