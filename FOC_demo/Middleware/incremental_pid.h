/** @file incremental_pid.h
 * @brief FOC内部增量式PID，与PID_demo独立命名以支持同时链接。
 * @note 固定周期调用，增益隐含周期；首次使用前清零并设置增益及上下限。
 *       非有限输入或无效限幅会清除历史并返回零。
 */
#ifndef COMDRIVERS_FOC_INCREMENTAL_PID_H
#define COMDRIVERS_FOC_INCREMENTAL_PID_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    float proportional_gain, integral_gain, derivative_gain;
    float previous_output, output;
    float maximum_output, minimum_output;
    float target, feedback;
    float previous_error, second_previous_error;
} FocIncrementalPidController;
/** @brief 输入目标和反馈，更新误差历史并返回限幅后的输出。 */
void FocIncrementalPid_Reset(FocIncrementalPidController *controller);
float FocIncrementalPid_Update(FocIncrementalPidController *controller, float target, float feedback);
#ifdef __cplusplus
}
#endif
#endif
