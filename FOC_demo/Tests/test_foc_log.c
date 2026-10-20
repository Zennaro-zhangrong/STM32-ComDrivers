/** @file test_foc_log.c
 * @brief 只复制FOC_demo和LOG_demo时的组合回归入口，不访问真实硬件。
 * @note 测试两个电机状态、PWM回调和日志实例；不把已知算法缺陷视为已修复。
 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
#include "FOC_demo/Service/motor_control_service.h"
#include "LOG_demo/Service/log_service.h"

typedef struct {
    uint16_t phase_a, phase_b, phase_c;
    unsigned writes;
} PwmCapture;

typedef struct {
    char text[128];
    size_t length;
} LogCapture;

static void CapturePwm(void *context, uint16_t phase_a, uint16_t phase_b, uint16_t phase_c)
{
    PwmCapture *capture = context;
    capture->phase_a = phase_a;
    capture->phase_b = phase_b;
    capture->phase_c = phase_c;
    ++capture->writes;
}

static LogMappingStatus CaptureLog(void *context, const uint8_t *data, size_t length)
{
    LogCapture *capture = context;
    assert(capture->length + length < sizeof(capture->text));
    memcpy(capture->text + capture->length, data, length);
    capture->length += length;
    capture->text[capture->length] = '\0';
    return LOG_MAPPING_OK;
}

int main(void)
{
    PwmCapture first_output = {0}, second_output = {0};
    MotorControlService first = {0}, second = {0};
    first.output = (FocMappingThreePhasePwm){&first_output, CapturePwm};
    second.output = (FocMappingThreePhasePwm){&second_output, CapturePwm};
    first.modulator.maximum_duty = second.modulator.maximum_duty = 1679;
    first.mode = MOTOR_CURRENT_CONTROL;
    second.mode = MOTOR_BRAKE;
    first.quadrature_current = (FocIncrementalPidController){
        .proportional_gain=1, .integral_gain=0.05f, .minimum_output=-300,
        .maximum_output=300, .target=20
    };
    first.direct_current.minimum_output = -300;
    first.direct_current.maximum_output = 300;
    MotorControl_UpdateCurrent(&first);
    MotorControl_UpdateCurrent(&second);
    assert(first_output.writes == 1 && second_output.writes == 1);
    assert(first.quadrature_current.output > 0 && second.quadrature_current.output == 0);
    assert(second_output.phase_a == 0 && second_output.phase_b == 0 && second_output.phase_c == 0);
    assert(first_output.phase_a <= 1679 && first_output.phase_b <= 1679 && first_output.phase_c <= 1679);

    uint8_t storage[128];
    LogCapture capture = {0};
    LogService log;
    LogService_Initialize(&log, storage, sizeof(storage),
        (LogMappingByteWriter){&capture, CaptureLog});
    LogService_Print(&log, "FOC outputs: %u/%u\n", first_output.writes, second_output.writes);
    assert(capture.length == 0); /* 电机控制和日志传输分别调度。 */
    assert(LogService_Process(&log) > 0);
    assert(strcmp(capture.text, "FOC outputs: 1/1\n") == 0);
    return 0;
}
