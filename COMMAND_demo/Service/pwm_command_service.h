/** @file pwm_command_service.h
 * @brief 原 FOC_demo 两条命令的服务化接口；连接测试与PWM输出均由调用方注入。
 * @note command=0 触发连接测试，command=1 接收两字节大端比较值，按 maximum_duty 限幅。
 */
#ifndef COMDRIVERS_PWM_COMMAND_SERVICE_H
#define COMDRIVERS_PWM_COMMAND_SERVICE_H
#include "../Middleware/command_dispatcher.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    void *context;
    void (*on_connectivity_test)(void *context);
    void (*set_duty)(void *context, uint16_t duty);
    uint16_t maximum_duty;
    CommandTable table;
} PwmCommandService;
/** @brief 先填写回调、context、maximum_duty，再调用此函数绑定实例命令表。
 * @note 初始化后的对象不可直接复制到别处，因为 table.context 指向对象本身。
 */
void PwmCommand_Initialize(PwmCommandService *service);
#ifdef __cplusplus
}
#endif
#endif
