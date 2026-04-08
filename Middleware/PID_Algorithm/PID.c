/**
 * @名称：PID.c
 * @功能：实现PID控制算法封装
 * @作者：张荣
 */

#include "PID.h"
#include "../../DriverLayer/Serial_port_log.h"
#include "stm32f1xx_hal.h"  // 根据芯片修改

#define PID_debug_log  1  // 开启后打印详细参数，用于PID调试。   0：关闭，1：开启

/**
 * @功能：设置PID参数
 * @参数：PID对象指针, P, I, D
 */
void PID_paramet_set(PID_Obj *pobj, float P, float I, float D)
{
    pobj->P = P;
    pobj->I = I;
    pobj->D = D;
}

/**
 * @功能：PID复位（清除积分、误差）
 */
void PID_reset(PID_Obj *pobj)
{
    pobj->Error_value = 0;
    pobj->Last_error_value = 0;
    pobj->Sum_error = 0;
    pobj->PID_output = 0;
}

/**
 * @功能：位置式PID计算
 * @注意：调用前必须更新 Actual_value，此函数无Delay，可放在中断执行。
 * @参数：要运算的PID对象
 */
void PID_operation(PID_Obj *pobj)
{
    // ====================== 误差计算======================
    pobj->Error_value = pobj->Expected_value - pobj->Actual_value;

    // ====================== 积分限幅（抗积分饱和）======================
    pobj->Sum_error += pobj->Error_value;
    float integral_max = pobj->Maximum_output / (pobj->I > 0 ? pobj->I : 0.001f);
    if(pobj->Sum_error > integral_max)  pobj->Sum_error = integral_max;
    if(pobj->Sum_error < -integral_max) pobj->Sum_error = -integral_max;

    // ====================== PID 输出计算 ======================
    float out = pobj->P * pobj->Error_value +
                pobj->I * pobj->Sum_error +
                pobj->D * (pobj->Error_value - pobj->Last_error_value);

    // 输出限幅
    if(out > pobj->Maximum_output)        out = pobj->Maximum_output;
    else if(out < -pobj->Maximum_output)  out = -pobj->Maximum_output;

    // 赋值给输出
    pobj->PID_output = (short)out;

    // ====================== 保存上次误差 ======================
    pobj->Last_error_value = pobj->Error_value;

    // 日志打印
    #if PID_debug_log
        log_printf("[%lu]%s: PID[%s] | P=%.2f I=%.2f D=%.2f | 期望=%.2f 实际=%.2f 输出=%d\r\n",
                    HAL_GetTick(), __func__, pobj->PID_name,
                    pobj->P, pobj->I, pobj->D,
                    pobj->Expected_value, pobj->Actual_value,
                    pobj->PID_output);
    #endif
}
