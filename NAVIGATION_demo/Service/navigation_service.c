/** @file navigation_service.c @brief 显式定位/目标有效状态，及时更新目标方位。 */
#include "navigation_service.h"
#include "../Middleware/navigation_math.h"
#include <string.h>
#include <math.h>
uint8_t NavigationService_Feed(NavigationService *service, uint8_t value)
{
    if (value == '\r') return 0;
    if (value == '\n') {
        service->line[service->line_length] = '\0';
        service->line_length = 0;
        if (service->line_overflow) {
            service->line_overflow = false; service->position_valid = false;
            service->position = (NavigationPosition){0}; return 0;
        }
        if (strncmp(service->line, "$GNRMC,", 7) != 0 && strncmp(service->line, "$GPRMC,", 7) != 0) return 0;
        service->position_valid = Navigation_ParseRmc(service->line, &service->position) != 0;
        return service->position_valid;
    }
    if (service->line_length < sizeof(service->line) - 1) service->line[service->line_length++] = (char)value;
    else service->line_overflow = true;
    return 0;
}
static void WriteLittleEndian(uint8_t *destination, int32_t value)
{
    uint32_t bits = (uint32_t)value;
    for (unsigned index = 0; index < 4; ++index) destination[index] = (uint8_t)(bits >> (index * 8));
}
void NavigationService_Upload(NavigationService *service, uint32_t milliseconds)
{
    if (!service->position_valid || service->uplink.write == NULL ||
        (uint32_t)(milliseconds - service->previous_upload_milliseconds) < 3000) return;
    uint8_t frame[11] = {0x01,0x00,0x08};
    WriteLittleEndian(frame + 3, service->position.longitude);
    WriteLittleEndian(frame + 7, service->position.latitude);
    if (service->uplink.write(service->uplink.context, frame, sizeof(frame)) == NAVIGATION_MAPPING_OK)
        service->previous_upload_milliseconds = milliseconds;
}
void NavigationService_RequestRoute(NavigationService *service)
{
    uint8_t frame[3] = {0x1A,0,0};
    service->target = (NavigationPosition){0}; service->target_valid = false;
    service->target_distance_meters = service->target_bearing_degrees = 0;
    if (service->uplink.write != NULL) (void)service->uplink.write(service->uplink.context, frame, sizeof(frame));
}
uint8_t NavigationService_Advance(NavigationService *service, WaypointDecoder *route, float arrival_meters)
{
    if (!service->position_valid || !isfinite(arrival_meters) || arrival_meters < 0) return 0;
    for (;;) {
        if (!service->target_valid) service->target_valid = WaypointDecoder_Read(route, &service->target) != 0;
        if (!service->target_valid) {
            service->target = (NavigationPosition){0};
            service->target_distance_meters = service->target_bearing_degrees = 0;
            return 0;
        }
        Navigation_DistanceBearing(Navigation_DegreesFromFixed(service->position.latitude),
            Navigation_DegreesFromFixed(service->position.longitude),
            Navigation_DegreesFromFixed(service->target.latitude), Navigation_DegreesFromFixed(service->target.longitude),
            &service->target_distance_meters, &service->target_bearing_degrees);
        if (service->target_distance_meters > arrival_meters) return 1;
        service->target_valid = false;
    }
}
