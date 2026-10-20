/** @file navigation_protocol.c @brief 严格校验RMC字段；航点帧完整接收后才公开。 */
#include "navigation_protocol.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static int HexDigit(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    return -1;
}
static bool Coordinate(const char *text, unsigned degree_digits, int32_t *value)
{
    size_t digits = 0;
    while (text[digits] >= '0' && text[digits] <= '9') ++digits;
    if (digits != degree_digits + 2) return false;
    size_t index = digits;
    if (text[index] == '.') {
        ++index;
        if (text[index] < '0' || text[index] > '9') return false;
        while (text[index] >= '0' && text[index] <= '9') ++index;
    }
    if (text[index] != '\0') return false;
    double number = strtod(text, NULL);
    unsigned degrees = (unsigned)(number / 100);
    double minutes = number - degrees * 100;
    unsigned limit = degree_digits == 2 ? 90 : 180;
    if (!isfinite(number) || degrees > limit || minutes >= 60 || (degrees == limit && minutes != 0)) return false;
    /* 固定点舍入不得把59.999999分钟进位为无效的60分钟。 */
    int32_t minute_units = (int32_t)llround(minutes * 100000.0);
    if (minute_units == 6000000) { ++degrees; minute_units = 0; }
    *value = (int32_t)(degrees * 10000000u) + minute_units;
    return true;
}
uint8_t Navigation_ParseRmc(const char *line, NavigationPosition *position)
{
    if (position == NULL) return 0;
    *position = (NavigationPosition){0};
    if (line == NULL || (strncmp(line, "$GNRMC,", 7) != 0 && strncmp(line, "$GPRMC,", 7) != 0)) return 0;
    size_t length = 0;
    while (length < 128 && line[length] != '\0') ++length;
    if (length >= 128) return 0;
    const char *star = strchr(line, '*');
    if (star == NULL || star - line + 3 > (ptrdiff_t)length) return 0;
    int high = HexDigit(star[1]), low = HexDigit(star[2]);
    if (high < 0 || low < 0) return 0;
    for (const char *tail = star + 3; *tail != '\0'; ++tail) if (*tail != '\r' && *tail != '\n') return 0;
    uint8_t checksum = 0;
    for (const char *cursor = line + 1; cursor < star; ++cursor) checksum ^= (uint8_t)*cursor;
    if (checksum != (uint8_t)((high << 4) | low)) return 0;
    char copy[128];
    size_t body_length = (size_t)(star - line);
    memcpy(copy, line, body_length); copy[body_length] = '\0';
    char *fields[16] = {copy};
    unsigned count = 1;
    for (size_t index = 0; index < body_length; ++index) {
        if (copy[index] == ',') {
            copy[index] = '\0';
            if (count >= 16) return 0;
            fields[count++] = copy + index + 1;
        }
    }
    if (count < 10 || strcmp(fields[2], "A") != 0 ||
        (strcmp(fields[4], "N") != 0 && strcmp(fields[4], "S") != 0) ||
        (strcmp(fields[6], "E") != 0 && strcmp(fields[6], "W") != 0)) return 0;
    NavigationPosition result;
    if (!Coordinate(fields[3], 2, &result.latitude) || !Coordinate(fields[5], 3, &result.longitude)) return 0;
    if (fields[4][0] == 'S') result.latitude = -result.latitude;
    if (fields[6][0] == 'W') result.longitude = -result.longitude;
    *position = result;
    return 1;
}
void WaypointDecoder_Initialize(WaypointDecoder *decoder, NavigationPosition *storage, uint16_t capacity)
{
    *decoder = (WaypointDecoder){.waypoints=storage, .capacity=storage != NULL ? capacity : 0};
}
void WaypointDecoder_ResetFrame(WaypointDecoder *decoder)
{
    decoder->state = 0;
    decoder->received_length = 0;
    decoder->payload_length = 0;
    decoder->staged_index = decoder->write_index;
}
static int32_t ReadSigned(const uint8_t *bytes)
{
    uint32_t bits = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
    return (int32_t)(bits <= INT32_MAX ? (int64_t)bits : (int64_t)bits - INT64_C(4294967296));
}
void WaypointDecoder_Feed(WaypointDecoder *decoder, uint8_t value, uint32_t milliseconds)
{
    if (decoder->state != 0 && (uint32_t)(milliseconds - decoder->previous_byte_milliseconds) >= 500)
        WaypointDecoder_ResetFrame(decoder);
    decoder->previous_byte_milliseconds = milliseconds;
    switch (decoder->state) {
    case 0:
        decoder->command = value; decoder->payload_length = 0;
        decoder->received_length = 0; decoder->state = 1; break;
    case 1:
        decoder->payload_length = (uint16_t)value << 8; decoder->state = 2; break;
    case 2:
        decoder->payload_length |= value;
        if (decoder->payload_length == 0) {
            if (decoder->command == 0x0E) decoder->write_index = decoder->read_index = 0;
            if (decoder->command >= 0x0B && decoder->command <= 0x0E && decoder->on_command != NULL)
                decoder->on_command(decoder->event_context, decoder->command);
            WaypointDecoder_ResetFrame(decoder);
        } else {
            decoder->staged_index = decoder->write_index;
            decoder->accept_payload = decoder->command == 0x0A && decoder->payload_length % 8 == 0 &&
                decoder->payload_length / 8 <= decoder->capacity - decoder->write_index;
            decoder->state = 3;
        }
        break;
    case 3:
        if (decoder->accept_payload) {
            unsigned byte = decoder->received_length % 8;
            decoder->coordinate_bytes[byte] = value;
            if (byte == 7) decoder->waypoints[decoder->staged_index++] = (NavigationPosition){
                ReadSigned(decoder->coordinate_bytes), ReadSigned(decoder->coordinate_bytes + 4)};
        }
        if (++decoder->received_length == decoder->payload_length) {
            if (decoder->accept_payload) decoder->write_index = decoder->staged_index;
            WaypointDecoder_ResetFrame(decoder);
        }
        break;
    default: WaypointDecoder_ResetFrame(decoder); break;
    }
}
uint8_t WaypointDecoder_Read(WaypointDecoder *decoder, NavigationPosition *position)
{
    if (position == NULL || decoder->read_index >= decoder->write_index) return 0;
    *position = decoder->waypoints[decoder->read_index++];
    return 1;
}
