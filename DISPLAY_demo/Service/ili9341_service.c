/** @file ili9341_service.c @brief 初始化字节按源 LCD.c 原顺序抽取，不更改寄存器值。 */
#include "ili9341_service.h"
static void SendByte(Ili9341Service *service, bool data, uint8_t value)
{
    if (service->last_status != DISPLAY_MAPPING_OK) return;
    service->data_command.write(service->data_command.context, data);
    service->chip_select.write(service->chip_select.context, false);
    service->last_status = service->writer.write(service->writer.context, &value, 1);
    service->chip_select.write(service->chip_select.context, true);
}
void Ili9341_Initialize(Ili9341Service *service)
{
    service->last_status = DISPLAY_MAPPING_OK;
    service->pixels_active = false;
    service->reset.write(service->reset.context, false);
    service->clock.delay_milliseconds(service->clock.context, 100);
    service->reset.write(service->reset.context, true);
    service->clock.delay_milliseconds(service->clock.context, 50);
    service->clock.delay_milliseconds(service->clock.context, 400);
    SendByte(service, false, 0xcf);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0xc1);
    SendByte(service, true, 0x30);
    SendByte(service, false, 0xed);
    SendByte(service, true, 0x64);
    SendByte(service, true, 0x03);
    SendByte(service, true, 0x12);
    SendByte(service, true, 0x81);
    SendByte(service, false, 0xe8);
    SendByte(service, true, 0x85);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0x78);
    SendByte(service, false, 0xcb);
    SendByte(service, true, 0x39);
    SendByte(service, true, 0x2c);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0x34);
    SendByte(service, true, 0x02);
    SendByte(service, false, 0xf7);
    SendByte(service, true, 0x20);
    SendByte(service, false, 0xea);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0x00);
    SendByte(service, false, 0xc0);
    SendByte(service, true, 0x13);
    SendByte(service, false, 0xc1);
    SendByte(service, true, 0x13);
    SendByte(service, false, 0xc5);
    SendByte(service, true, 0x22);
    SendByte(service, true, 0x35);
    SendByte(service, false, 0xc7);
    SendByte(service, true, 0xbd);
    SendByte(service, false, 0x21);
    SendByte(service, false, 0x36);
    SendByte(service, true, 0x08);
    SendByte(service, false, 0xb6);
    SendByte(service, true, 0x0a);
    SendByte(service, true, 0xa2);
    SendByte(service, false, 0x3a);
    SendByte(service, true, 0x55);
    SendByte(service, false, 0xf6);
    SendByte(service, true, 0x01);
    SendByte(service, true, 0x30);
    SendByte(service, false, 0xb1);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0x1b);
    SendByte(service, false, 0xf2);
    SendByte(service, true, 0x00);
    SendByte(service, false, 0x26);
    SendByte(service, true, 0x01);
    SendByte(service, false, 0xe0);
    SendByte(service, true, 0x0f);
    SendByte(service, true, 0x35);
    SendByte(service, true, 0x31);
    SendByte(service, true, 0x0b);
    SendByte(service, true, 0x0e);
    SendByte(service, true, 0x06);
    SendByte(service, true, 0x49);
    SendByte(service, true, 0xa7);
    SendByte(service, true, 0x33);
    SendByte(service, true, 0x07);
    SendByte(service, true, 0x0f);
    SendByte(service, true, 0x03);
    SendByte(service, true, 0x0c);
    SendByte(service, true, 0x0a);
    SendByte(service, true, 0x00);
    SendByte(service, false, 0xe1);
    SendByte(service, true, 0x00);
    SendByte(service, true, 0x0a);
    SendByte(service, true, 0x0f);
    SendByte(service, true, 0x04);
    SendByte(service, true, 0x11);
    SendByte(service, true, 0x08);
    SendByte(service, true, 0x36);
    SendByte(service, true, 0x58);
    SendByte(service, true, 0x4d);
    SendByte(service, true, 0x07);
    SendByte(service, true, 0x10);
    SendByte(service, true, 0x0c);
    SendByte(service, true, 0x32);
    SendByte(service, true, 0x34);
    SendByte(service, true, 0x0f);
    SendByte(service, false, 0x11);
    service->clock.delay_milliseconds(service->clock.context, 120);
    SendByte(service, false, 0x29);
    Ili9341_SetDirection(service, 3);
}
void Ili9341_SetDirection(Ili9341Service *service, uint8_t direction)
{
    static const uint8_t modes[4] = {0x08, 0x68, 0xC8, 0xA8};
    if (direction > 3) direction = 0;
    service->width = (direction == 1 || direction == 3) ? 320 : 240;
    service->height = (direction == 1 || direction == 3) ? 240 : 320;
    SendByte(service, false, 0x36);
    SendByte(service, true, modes[direction]);
}
static void SendRange(Ili9341Service *service, uint8_t command, uint16_t first, uint16_t last)
{
    SendByte(service, false, command);
    uint8_t bytes[4] = {(uint8_t)(first >> 8), (uint8_t)first, (uint8_t)(last >> 8), (uint8_t)last};
    service->data_command.write(service->data_command.context, true);
    service->chip_select.write(service->chip_select.context, false);
    if (service->last_status == DISPLAY_MAPPING_OK) service->last_status = service->writer.write(service->writer.context, bytes, 4);
    service->chip_select.write(service->chip_select.context, true);
}

DisplayMappingStatus Ili9341_SetWindow(Ili9341Service *service, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom)
{
    if (service->pixels_active) return DISPLAY_MAPPING_BUSY;
    if (left > right || top > bottom || right >= service->width || bottom >= service->height) return DISPLAY_MAPPING_ERROR;
    service->last_status = DISPLAY_MAPPING_OK;
    SendRange(service, 0x2A, left, right);
    SendRange(service, 0x2B, top, bottom);
    return service->last_status;
}
void Ili9341_AbortPixels(Ili9341Service *service)
{
    service->chip_select.write(service->chip_select.context, true);
    service->pixels_active = false;
    service->remaining_bytes = 0;
    service->active_bytes = 0;
    service->pending_pixels = NULL;
}
static DisplayMappingStatus StartChunk(Ili9341Service *service)
{
    service->active_bytes = (uint16_t)(service->remaining_bytes > 65534 ? 65534 : service->remaining_bytes);
    service->last_status = service->start_pixels(service->pixel_context, service->pending_pixels, service->active_bytes);
    if (service->last_status != DISPLAY_MAPPING_OK) Ili9341_AbortPixels(service);
    return service->last_status;
}
DisplayMappingStatus Ili9341_StartPixels(Ili9341Service *service, const uint8_t *pixels, size_t byte_count)
{
    if (service->pixels_active) return DISPLAY_MAPPING_BUSY;
    if (pixels == NULL || byte_count == 0 || byte_count % 2 != 0 || service->start_pixels == NULL) return DISPLAY_MAPPING_ERROR;
    if (service->last_status != DISPLAY_MAPPING_OK) return service->last_status;
    SendByte(service, false, 0x2C);
    if (service->last_status != DISPLAY_MAPPING_OK) return service->last_status;
    service->chip_select.write(service->chip_select.context, false);
    service->data_command.write(service->data_command.context, true);
    service->pixels_active = true;
    service->pending_pixels = pixels;
    service->remaining_bytes = byte_count;
    return StartChunk(service);
}
DisplayMappingStatus Ili9341_OnPixelsComplete(Ili9341Service *service)
{
    if (!service->pixels_active) return DISPLAY_MAPPING_ERROR;
    service->remaining_bytes -= service->active_bytes;
    service->pending_pixels += service->active_bytes;
    if (service->remaining_bytes == 0) { Ili9341_AbortPixels(service); return DISPLAY_MAPPING_OK; }
    DisplayMappingStatus status = StartChunk(service);
    return status == DISPLAY_MAPPING_OK ? DISPLAY_MAPPING_BUSY : DISPLAY_MAPPING_ERROR;
}
void Ili9341_FinishPixels(Ili9341Service *service) { Ili9341_AbortPixels(service); }
