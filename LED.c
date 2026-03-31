/*
 * LED.c
 *
 *  Created on: Mar 27, 2026
 *      Author: 吴梦佳
 */
#include "Hardware.h"
/*
 * 前提：PA1配置为推挽输出，低电平有效；
 */

/*
 * 功能：控制LED引脚高低电平
 * 输入：枚举类型
 * 输出：无
 */
void LED_Set(LED_State P){
	if(P == LED_ON){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	}else if(P == LED_OFF){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	}
}
