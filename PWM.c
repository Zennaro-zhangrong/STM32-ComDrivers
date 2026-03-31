/*
 * PWM.c
 *
 *  Created on: Mar 27, 2026
 *      Author: 吴梦佳
 */
#include "Hardware.h"
extern TIM_HandleTypeDef htim4;

Electric_Drive ED_A = {0};
Electric_Drive ED_B = {0};
Electric_Drive ED_C = {0};
Electric_Drive ED_D = {0};

/*
 * 功能：用于PWM参数初始化，上电执行一次即可
 */
void PWM_Init(void){
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

	ED_A.Duty_Cycle = 0;
	ED_B.Duty_Cycle = 0;
	ED_C.Duty_Cycle = 0;
	ED_D.Duty_Cycle = 0;

	ED_A.Direction = 1;
	ED_B.Direction = 1;
	ED_C.Direction = 1;
	ED_D.Direction = 1;


	ED_A.CHANNEL = TIM_CHANNEL_1;//通道可以根据实际情况更改
	ED_B.CHANNEL = TIM_CHANNEL_2;
	ED_C.CHANNEL = TIM_CHANNEL_3;
	ED_D.CHANNEL = TIM_CHANNEL_4;
}


/*
 * 功能：实时更新PWM占空比参数，放到循环或者定时器中断，确保至少100ms执行一次
 */
void PWM_Updata(void){
	__HAL_TIM_SET_COMPARE(&htim4, ED_A.CHANNEL, ED_A.Duty_Cycle);
	__HAL_TIM_SET_COMPARE(&htim4, ED_B.CHANNEL, ED_B.Duty_Cycle);
	__HAL_TIM_SET_COMPARE(&htim4, ED_C.CHANNEL, ED_C.Duty_Cycle);
	__HAL_TIM_SET_COMPARE(&htim4, ED_D.CHANNEL, ED_D.Duty_Cycle);
	UART_Printf("Duty_Cycle: %d / 720 \r\n",ED_A.Duty_Cycle);
}
