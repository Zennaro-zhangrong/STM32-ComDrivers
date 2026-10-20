/** @file nrf24l01_service.c @brief 状态位分别处理，RX FIFO按队列读取，不在正常接收时清空。 */
#include "nrf24l01_service.h"
#include <string.h>
static void Select(Nrf24l01Service *service, bool active) { service->chip_select.write(service->chip_select.context, !active); }
static void Enable(Nrf24l01Service *service, bool active) { service->chip_enable.write(service->chip_enable.context, active); }
static uint8_t Exchange(Nrf24l01Service *service, uint8_t value)
{
    uint8_t received = 0;
    if (service->last_bus_status == NRF24L01_MAPPING_OK)
        service->last_bus_status = service->bus.exchange(service->bus.context, &value, &received, 1);
    return received;
}
static void Command(Nrf24l01Service *service, uint8_t command)
{
    Select(service, true); Exchange(service, command); Select(service, false);
}
static uint8_t ReadRegister(Nrf24l01Service *service, uint8_t address)
{
    Select(service, true); Exchange(service, address & 31u);
    uint8_t result = Exchange(service, 255); Select(service, false); return result;
}
static void WriteRegister(Nrf24l01Service *service, uint8_t address, uint8_t value)
{
    Select(service, true); Exchange(service, 0x20 | (address & 31u)); Exchange(service, value); Select(service, false);
}
static void WriteAddress(Nrf24l01Service *service, uint8_t address)
{
    Select(service, true); Exchange(service, 0x20 | address);
    for (unsigned index = 0; index < 5; ++index) Exchange(service, service->address[index]);
    Select(service, false);
}
static void ReceiveMode(Nrf24l01Service *service)
{
    Enable(service, false);
    uint8_t configuration = ReadRegister(service, 0);
    WriteRegister(service, 0, configuration | 0x0B);
    if (service->last_bus_status == NRF24L01_MAPPING_OK) Enable(service, true);
    service->transmitting = false;
}
static void FailTransmission(Nrf24l01Service *service)
{
    Enable(service, false); Select(service, false);
    service->transmitting = false;
    service->transmit_state = NRF24L01_TX_IO_ERROR;
}
Nrf24l01Result Nrf24l01_Initialize(Nrf24l01Service *service)
{
    service->initialized = false;
    if (service->bus.exchange == NULL || service->chip_select.write == NULL || service->chip_enable.write == NULL ||
        service->clock.delay_milliseconds == NULL || service->clock.milliseconds == NULL || service->radio_channel > 125)
        return NRF24L01_INVALID;
    service->transmitting = false; service->transmit_state = NRF24L01_TX_IDLE;
    service->last_bus_status = NRF24L01_MAPPING_OK;
    service->rejected_frames = 0;
    if (service->transmit_timeout_milliseconds == 0) service->transmit_timeout_milliseconds = 100;
    if (service->transmit_timeout_milliseconds >= UINT32_C(0x80000000)) return NRF24L01_INVALID;
    Select(service, false); Enable(service, false);
    service->clock.delay_milliseconds(service->clock.context, 100);
    WriteRegister(service, 0, 0x0C);
    uint8_t configuration = ReadRegister(service, 0);
    if (service->last_bus_status != NRF24L01_MAPPING_OK) return NRF24L01_IO_ERROR;
    if (configuration != 0x0C) return NRF24L01_INVALID;
    WriteRegister(service, 0, 0);
    service->clock.delay_milliseconds(service->clock.context, 5);
    WriteRegister(service, 7, 0x70); Command(service, 0xE1); Command(service, 0xE2);
    WriteRegister(service, 1, 1); WriteRegister(service, 2, 1);
    WriteRegister(service, 3, 3); WriteRegister(service, 4, 0x1A);
    WriteRegister(service, 5, service->radio_channel); WriteRegister(service, 6, service->radio_setup);
    WriteAddress(service, 0x0A); WriteAddress(service, 0x10); WriteRegister(service, 0x11, 32);
    WriteRegister(service, 0, 0x0F);
    if (service->last_bus_status != NRF24L01_MAPPING_OK) return NRF24L01_IO_ERROR;
    service->clock.delay_milliseconds(service->clock.context, 5);
    Enable(service, true);
    service->initialized = true;
    return NRF24L01_OK;
}
Nrf24l01Result Nrf24l01_Send(Nrf24l01Service *service, uint8_t command, uint8_t length, const uint8_t *payload)
{
    if (!service->initialized || length > 30 || (length != 0 && payload == NULL)) return NRF24L01_INVALID;
    if (service->transmitting) return NRF24L01_BUSY;
    service->last_bus_status = NRF24L01_MAPPING_OK;
    uint8_t frame[32] = {command,length};
    if (length != 0) memcpy(frame + 2, payload, length);
    Enable(service, false);
    uint8_t configuration = ReadRegister(service, 0);
    WriteRegister(service, 0, (configuration & (uint8_t)~1u) | 0x0A);
    WriteRegister(service, 7, 0x30); Command(service, 0xE1);
    Select(service, true); Exchange(service, 0xA0);
    uint8_t ignored[32];
    if (service->last_bus_status == NRF24L01_MAPPING_OK)
        service->last_bus_status = service->bus.exchange(service->bus.context, frame, ignored, sizeof(frame));
    Select(service, false);
    if (service->last_bus_status != NRF24L01_MAPPING_OK) { FailTransmission(service); return NRF24L01_IO_ERROR; }
    service->transmitting = true; service->transmit_state = NRF24L01_TX_PENDING;
    service->transmit_started_milliseconds = service->clock.milliseconds(service->clock.context);
    Enable(service, true); service->clock.delay_milliseconds(service->clock.context, 1); Enable(service, false);
    return NRF24L01_OK;
}
void Nrf24l01_ProcessInterrupt(Nrf24l01Service *service)
{
    if (!service->initialized) return;
    service->last_bus_status = NRF24L01_MAPPING_OK;
    uint8_t status = ReadRegister(service, 7);
    if (service->last_bus_status != NRF24L01_MAPPING_OK) return;
    if ((status & 0x30) != 0) {
        if (service->transmitting) service->transmit_state = (status & 0x10) ? NRF24L01_TX_RETRY_EXHAUSTED : NRF24L01_TX_DELIVERED;
        if (status & 0x10) Command(service, 0xE1);
        WriteRegister(service, 7, status & 0x30);
        ReceiveMode(service);
    }
    /* 先清已观察到的RX_DR再检查FIFO，新到帧会重新置位；不清除其他中断。 */
    if (status & 0x40) WriteRegister(service, 7, 0x40);
    for (unsigned packet = 0; packet < 3 && service->last_bus_status == NRF24L01_MAPPING_OK; ++packet) {
        uint8_t fifo = ReadRegister(service, 0x17);
        if (service->last_bus_status != NRF24L01_MAPPING_OK || (fifo & 1)) break;
        uint8_t outgoing[32], incoming[32]; memset(outgoing, 255, sizeof(outgoing));
        Select(service, true); Exchange(service, 0x61);
        if (service->last_bus_status == NRF24L01_MAPPING_OK)
            service->last_bus_status = service->bus.exchange(service->bus.context, outgoing, incoming, sizeof(incoming));
        Select(service, false);
        if (service->last_bus_status != NRF24L01_MAPPING_OK) break;
        if (incoming[1] > 30) ++service->rejected_frames;
        else if (service->on_receive != NULL)
            service->on_receive(service->receive_context, incoming[0], incoming[1], incoming + 2);
    }
    if (service->last_bus_status != NRF24L01_MAPPING_OK && service->transmitting) FailTransmission(service);
}
void Nrf24l01_Supervise(Nrf24l01Service *service, uint32_t milliseconds)
{
    if (!service->initialized || !service->transmitting ||
        (uint32_t)(milliseconds - service->transmit_started_milliseconds) < service->transmit_timeout_milliseconds) return;
    service->last_bus_status = NRF24L01_MAPPING_OK;
    Enable(service, false); Command(service, 0xE1); WriteRegister(service, 7, 0x30);
    ReceiveMode(service);
    service->transmit_state = service->last_bus_status == NRF24L01_MAPPING_OK ? NRF24L01_TX_TIMEOUT : NRF24L01_TX_IO_ERROR;
}
