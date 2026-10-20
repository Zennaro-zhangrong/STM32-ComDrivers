/** @file nrf24l01_service.h
 * @brief 固定32字节无线帧，区分请求受理、最终送达和错误。
 * @note 所有接口在同一任务执行；IRQ仅通知任务。回调期间不可重入服务。
 */
#ifndef COMDRIVERS_NRF24L01_SERVICE_H
#define COMDRIVERS_NRF24L01_SERVICE_H
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { NRF24L01_OK, NRF24L01_INVALID, NRF24L01_BUSY, NRF24L01_IO_ERROR } Nrf24l01Result;
typedef enum { NRF24L01_TX_IDLE, NRF24L01_TX_PENDING, NRF24L01_TX_DELIVERED,
    NRF24L01_TX_RETRY_EXHAUSTED, NRF24L01_TX_IO_ERROR, NRF24L01_TX_TIMEOUT } Nrf24l01TransmitState;
typedef struct {
    Nrf24l01MappingSpiBus bus;
    Nrf24l01MappingDigitalPin chip_select, chip_enable;
    Nrf24l01MappingClock clock;
    uint8_t address[5], radio_channel, radio_setup;
    bool transmitting, initialized;
    void *receive_context;
    void (*on_receive)(void *context, uint8_t command, uint8_t length, const uint8_t *payload);
    Nrf24l01MappingStatus last_bus_status;
    Nrf24l01TransmitState transmit_state;
    uint32_t transmit_started_milliseconds, transmit_timeout_milliseconds;
    uint32_t rejected_frames;
} Nrf24l01Service;
/** @brief 填好总线、GPIO、毫秒时钟与延时回调后初始化；超时为0时采用100ms。 */
Nrf24l01Result Nrf24l01_Initialize(Nrf24l01Service *service);
/** @brief length<=30；OK只表示已受理，最终结果查询transmit_state；忙时不覆盖原请求。 */
Nrf24l01Result Nrf24l01_Send(Nrf24l01Service *service, uint8_t command, uint8_t length, const uint8_t *payload);
/** @brief 处理独立IRQ标志并排空最多3帧；可周期调用以处理IRQ清除期间到达的帧。 */
void Nrf24l01_ProcessInterrupt(Nrf24l01Service *service);
/** @brief 周期检查无IRQ的发送超时；时间差支持32位回绕，需在半个计数范围内调用。 */
void Nrf24l01_Supervise(Nrf24l01Service *service, uint32_t milliseconds);
#ifdef __cplusplus
}
#endif
#endif
