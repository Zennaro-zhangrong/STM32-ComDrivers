/** @file pid_controller.h
 * @brief 位置式与增量式 PID；输出使用浮点数，避免整数截断与窄化溢出。
 * @note 固定周期调用，采样周期已隐含在积分/微分增益中。不包含日志或时基。
 */
#ifndef COMDRIVERS_PID_CONTROLLER_H
#define COMDRIVERS_PID_CONTROLLER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    float proportional_gain, integral_gain, derivative_gain; /**< P、I、D 增益。 */
    float target, feedback;       /**< 目标值、反馈值，单位必须相同。 */
    float error, previous_error, accumulated_error;
    float maximum_output;        /**< 对称限幅，须为非负有限值。 */
    float output;                 /**< 限幅后的浮点输出。 */
} PositionPidController;
typedef struct {
    float proportional_gain, integral_gain, derivative_gain;
    float previous_output, output;
    float maximum_output, minimum_output;
    float target, feedback;
    float previous_error, second_previous_error;
} IncrementalPidController;
/** @brief 清除位置式 PID 的误差历史和输出，保留目标、反馈及配置。 */
void PositionPid_Reset(PositionPidController *controller);
/** @brief 根据 target/feedback 更新 output；零积分增益清积累，非零按增益绝对值约束积分项。 */
void PositionPid_Update(PositionPidController *controller);
/** @brief 增量式 PID，返回限幅后的输出。对象在首次使用前应全部清零并设置增益、上下限。 */
void IncrementalPid_Reset(IncrementalPidController *controller);
float IncrementalPid_Update(IncrementalPidController *controller, float target, float feedback);
#ifdef __cplusplus
}
#endif
#endif
