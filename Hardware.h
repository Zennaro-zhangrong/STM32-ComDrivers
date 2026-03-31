/*
 * Test.h
 *
 *  Created on: Mar 27, 2026
 *      Author: 吴梦佳
 */

#ifndef TEST_H_
#define TEST_H_
#include "main.h"

typedef enum {
	LED_ON = 0,
	LED_OFF = 1
}LED_State;

typedef struct{
	uint16_t Duty_Cycle;
	uint8_t Direction;
	uint32_t CHANNEL;
}Electric_Drive;

typedef struct{
	uint8_t CMD;
	uint8_t Data_Len;
	uint8_t Data_Buffer[36];
	uint8_t Buffer_Status;
	uint8_t Buffer_Cursor;
}SPI_Data_Buffer;


void LED_Set(LED_State P);
void PWM_Init(void);
void PWM_Updata(void);
void UART_Printf(const char *fmt, ...);
void SPI_RxCallback(SPI_HandleTypeDef *hspi);


#endif /* TEST_H_ */
