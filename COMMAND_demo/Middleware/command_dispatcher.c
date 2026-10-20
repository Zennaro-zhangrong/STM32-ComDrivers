/** @file command_dispatcher.c @brief 不依赖日志或硬件的线性查表。 */
#include "command_dispatcher.h"
#include <stddef.h>
uint8_t CommandDispatcher_Run(const CommandTable *table, uint8_t code,
                              uint8_t length, const uint8_t *payload)
{
    if (table == NULL || table->entries == NULL || (length != 0 && payload == NULL)) return 0;
    for (uint8_t index = 0; index < table->count; ++index) {
        if (table->entries[index].code == code) {
            if (table->entries[index].handler == NULL) return 0;
            table->entries[index].handler(table->context, length, payload);
            return 1;
        }
    }
    return 0;
}
