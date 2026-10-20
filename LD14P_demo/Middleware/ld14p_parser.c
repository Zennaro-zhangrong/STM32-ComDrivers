/** @file ld14p_parser.c @brief 来源：CarMainControlBoard/ld14p.c。 */
#include "ld14p_parser.h"
#include <math.h>
#include <string.h>
static uint16_t ReadLittleEndian(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}
uint8_t Ld14p_Crc8(const uint8_t *data, size_t length)
{
    uint8_t crc = 0;
    for (size_t index = 0; index < length; ++index) {
        crc ^= data[index];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (uint8_t)((crc << 1) ^ ((crc & 0x80u) ? 0x4Du : 0));
    }
    return crc;
}
uint8_t Ld14p_CheckHeaders(const uint8_t *packets, size_t bytes)
{
    if (packets == NULL || bytes == 0 || bytes % 47 != 0 || bytes > LD14P_BATCH_BYTES) return 0;
    for (size_t index = 0; index < bytes; index += 47)
        if (packets[index] != 0x54 || packets[index + 1] != 0x2C) return 0;
    return 1;
}
uint16_t Ld14p_ParseBatch(const uint8_t *packets, size_t bytes, uint8_t *polar, size_t capacity)
{
    if (polar == NULL) return 0;
    memset(polar, 0, capacity);
    if (packets == NULL || bytes == 0 || bytes % 47 != 0 || bytes > LD14P_BATCH_BYTES || capacity < bytes / 47 * 48) return 0;
    uint16_t cursor = 0;
    for (size_t packet = 0; packet < bytes / 47; ++packet) {
        const uint8_t *data = &packets[packet * 47];
        if (data[0] != 0x54 || data[1] != 0x2C || Ld14p_Crc8(data, 46) != data[46]) continue;
        uint16_t start = ReadLittleEndian(&data[4]), end = ReadLittleEndian(&data[42]);
        if (start >= 36000 || end >= 36000) continue;
        uint32_t difference = ((uint32_t)end + 36000 - start) % 36000;
        for (int point = 0; point < 12; ++point) {
            if (data[6 + point * 3 + 2] < 10) continue;
            uint16_t distance = ReadLittleEndian(&data[6 + point * 3]);
            uint16_t angle = (uint16_t)((start + difference * (unsigned)point / 11) % 36000);
            polar[cursor++] = (uint8_t)distance;
            polar[cursor++] = (uint8_t)(distance >> 8);
            polar[cursor++] = (uint8_t)angle;
            polar[cursor++] = (uint8_t)(angle >> 8);
        }
    }
    return cursor / 4;
}
bool Ld14p_ConvertGrid(const uint8_t *polar, size_t count, uint8_t *grid, size_t capacity, int32_t angle_offset)
{
    if (grid == NULL) return false;
    memset(grid, 0, capacity);
    if (polar == NULL || count > LD14P_BATCH_POINTS || capacity < count * 2) return false;
    for (size_t index = 0; index < count; ++index) {
        uint16_t distance = ReadLittleEndian(&polar[index * 4]);
        uint16_t angle = ReadLittleEndian(&polar[index * 4 + 2]);
        if (distance == 0) { grid[index * 2] = grid[index * 2 + 1] = 0; continue; }
        if (distance > 8000) distance = 8000;
        int32_t effective_angle = (angle + angle_offset % 36000) % 36000;
        if (effective_angle < 0) effective_angle += 36000;
        float radians = -(float)effective_angle * (3.14159265f / 18000.0f);
        int16_t x_mm = (int16_t)((float)distance * cosf(radians));
        int16_t y_mm = (int16_t)((float)distance * sinf(radians));
        int16_t x_grid = (int16_t)((float)x_mm * 128.0f / 8000.0f) + 128;
        int16_t y_grid = (int16_t)((float)y_mm * 128.0f / 8000.0f) + 128;
        if (x_grid < 0) x_grid = 0;
        if (x_grid > 255) x_grid = 255;
        if (y_grid < 0) y_grid = 0;
        if (y_grid > 255) y_grid = 255;
        grid[index * 2] = (uint8_t)x_grid;
        grid[index * 2 + 1] = (uint8_t)y_grid;
    }
    return true;
}
