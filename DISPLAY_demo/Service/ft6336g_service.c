/** @file ft6336g_service.c @brief 单次连续寄存器读取，区分总线失败与无触点。 */
#include "ft6336g_service.h"
uint8_t Ft6336g_Initialize(Ft6336gService *service)
{
    uint8_t identity;
    if (service->bus.read == NULL || service->bus.write == NULL || service->width == 0 || service->height == 0) return 0;
    service->last_status = service->bus.read(service->bus.context, service->address, 0xA8, &identity, 1);
    if (service->last_status != DISPLAY_MAPPING_OK || identity != 0x11) return 0;
    static const uint8_t configuration[][2] = {
        {0x00,0x00},{0xA4,0x00},{0xA5,0x00},{0x80,0x18},{0x85,0x25},
        {0x86,0x01},{0x87,0x20},{0xBC,0x01},{0x88,0x3C},{0x89,0x28}
    };
    for (unsigned index = 0; index < sizeof(configuration) / sizeof(configuration[0]); ++index) {
        service->last_status = service->bus.write(service->bus.context, service->address, configuration[index][0], &configuration[index][1], 1);
        if (service->last_status != DISPLAY_MAPPING_OK) return 0;
    }
    return 1;
}
uint8_t Ft6336g_ReadCoordinates(Ft6336gService *service)
{
    if (service->bus.read == NULL || service->width == 0 || service->height == 0) {
        service->last_status = DISPLAY_MAPPING_ERROR; return FT6336G_READ_ERROR;
    }
    uint8_t data[11];
    service->last_status = service->bus.read(service->bus.context, service->address, 2, data, sizeof(data));
    if (service->last_status != DISPLAY_MAPPING_OK) return FT6336G_READ_ERROR;
    unsigned count = data[0] & 15u;
    if (count > 2) { service->last_status = DISPLAY_MAPPING_ERROR; return FT6336G_READ_ERROR; }
    uint16_t x[2], y[2];
    for (unsigned point = 0; point < count; ++point) {
        unsigned start = 1 + point * 6;
        uint16_t raw_x = (uint16_t)(((data[start] & 15u) << 8) | data[start + 1]);
        uint16_t raw_y = (uint16_t)(((data[start + 2] & 15u) << 8) | data[start + 3]);
        if (raw_x >= service->height || raw_y >= service->width) {
            service->last_status = DISPLAY_MAPPING_ERROR; return FT6336G_READ_ERROR;
        }
        x[point] = service->width - 1 - raw_y;
        y[point] = raw_x;
    }
    for (unsigned point = 0; point < count; ++point) { service->x[point] = x[point]; service->y[point] = y[point]; }
    return (uint8_t)count;
}
