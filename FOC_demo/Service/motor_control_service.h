/** @file motor_control_service.h
 * @brief 电流环和速度环服务，原 Motor.c 的算法编排与板级采样/中断分离。
 * @note 调用方设置 PID、采样增益和 maximum_duty；各更新入口不得并发访问同一对象。
 */
#ifndef COMDRIVERS_MOTOR_CONTROL_SERVICE_H
#define COMDRIVERS_MOTOR_CONTROL_SERVICE_H
#include "../Middleware/incremental_pid.h"
#include "../Middleware/space_vector_pwm.h"
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { MOTOR_SPEED_CONTROL, MOTOR_CURRENT_CONTROL, MOTOR_BRAKE } MotorControlMode;
typedef struct {
    MotorControlMode mode, previous_mode;
    uint8_t mode_initialized;
    float maximum_current_target; /**< 电流目标的独立对称限幅；必须设置为正有限值。 */
    FocIncrementalPidController direct_current, quadrature_current, speed;
    SpaceVectorPwm modulator;
    FocMappingThreePhasePwm output;
    float current_phase_a, current_phase_b, current_phase_c;
    float electrical_angle_degrees;
    float speed_feedback;         /**< 与速度目标同尺度；霍尔估算现为机械转数/秒，必须配置真实时基和极对数。 */
    float current_bias_b, current_bias_c;
    float current_gain_b, current_gain_c;
} MotorControlService;
/** @brief 切换模式时清误差历史和旧目标；随后再设置新目标。无效模式转为制动。 */
void MotorControl_SetMode(MotorControlService *service, MotorControlMode mode);
/** @brief 将两相 ADC 原始采样换算为三相电流，保持浮点采样精度。 */
void MotorControl_SetSamples(MotorControlService *service, int16_t sample_b, int16_t sample_c);
/** @brief 执行一次电流环；必须先更新采样和电角度，输出回调必须有效。 */
void MotorControl_UpdateCurrent(MotorControlService *service);
/** @brief 执行一次速度环；前馈后再次限幅，停机及模式切换清历史。 */
void MotorControl_UpdateSpeed(MotorControlService *service);
/** @brief 将原遥控前进/转向值映射到左右电机目标，电流模式使用maximum_current_target独立限幅。 */
void MotorControl_SetDifferentialTarget(MotorControlService *left, MotorControlService *right,
                                        MotorControlMode mode, int16_t forward, int16_t turn);
#ifdef __cplusplus
}
#endif
#endif
