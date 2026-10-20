/** @file space_vector_pwm.h
 * @brief 七段式空间矢量调制；浮点角度与幅值、零矢量分配及过调制缩放。
 * @note 不访问定时器，不补偿死区。源代码 DeadTime 字段未参与计算，故不暴露虚假功能。
 */
#ifndef COMDRIVERS_SPACE_VECTOR_PWM_H
#define COMDRIVERS_SPACE_VECTOR_PWM_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    uint8_t sector;             /**< 扇区 1～6。 */
    float angle_degrees;     /**< 归一化后的电角度。 */
    float alpha, beta;            /**< 静止坐标电压，沿用原计数尺度。 */
    int duty_a, duty_b, duty_c; /**< 三相比较值，单位：定时器计数。 */
    uint16_t maximum_duty;      /**< 由板级定时器配置提供。 */
} SpaceVectorPwm;
/** @brief 从 alpha/beta 计算三相比较值；非有限输入输出三相0。 */
void SpaceVectorPwm_Update(SpaceVectorPwm *modulator);
/** @brief 极坐标入口；amplitude 为计数幅值，angle_degrees 单位为度。 */
void SpaceVectorPwm_FromPolar(SpaceVectorPwm *modulator, float amplitude, float angle_degrees);
#ifdef __cplusplus
}
#endif
#endif
