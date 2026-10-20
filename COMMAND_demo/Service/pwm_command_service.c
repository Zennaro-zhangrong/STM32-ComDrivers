/** @file pwm_command_service.c @brief 来源：目标库 spi_cmd_table.c。 */
#include "pwm_command_service.h"
#include <stddef.h>
static void ConnectivityTest(void *context, uint8_t length, const uint8_t *payload)
{
    PwmCommandService *service = context;
    (void)length; (void)payload;
    if (service->on_connectivity_test != NULL) service->on_connectivity_test(service->context);
}
static void SetDuty(void *context, uint8_t length, const uint8_t *payload)
{
    PwmCommandService *service = context;
    if (length < 2 || payload == NULL) return;
    uint16_t duty = ((uint16_t)payload[0] << 8) | payload[1];
    if (duty > service->maximum_duty) duty = service->maximum_duty;
    if (service->set_duty != NULL) service->set_duty(service->context, duty);
}
void PwmCommand_Initialize(PwmCommandService *service)
{
    static const CommandEntry entries[] = {{0, ConnectivityTest}, {1, SetDuty}};
    service->table = (CommandTable){entries, 2, service};
}
