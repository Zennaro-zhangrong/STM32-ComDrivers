/** @file command_dispatcher.h
 * @brief 只读命令表查找；由调用方持有表和业务上下文，不保存全局注册表。
 * @note 该接口迁移替代 CMD_Table_Bind；表生命周期必须覆盖所有派发调用。
 */
#ifndef COMDRIVERS_COMMAND_DISPATCHER_H
#define COMDRIVERS_COMMAND_DISPATCHER_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*CommandHandler)(void *context, uint8_t length, const uint8_t *payload);
typedef struct { uint8_t code; CommandHandler handler; } CommandEntry;
typedef struct {
    const CommandEntry *entries;
    uint8_t count;             /**< 实际上限为 255，修正原“256 条”注释。 */
    void *context;
} CommandTable;
/** @return 已调用有效处理函数返回1；未知命令、空处理函数或无效载荷返回0。 */
uint8_t CommandDispatcher_Run(const CommandTable *table, uint8_t code,
                              uint8_t length, const uint8_t *payload);
#ifdef __cplusplus
}
#endif
#endif
