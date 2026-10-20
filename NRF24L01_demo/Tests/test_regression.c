/** @file test_regression.c @brief 寄存器/FIFO模拟，覆盖并发状态位、忙、SPI错误和超时。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
#include "../Service/nrf24l01_service.h"
#include "../Middleware/remote_control_protocol.h"
typedef struct {
    uint8_t registers[32],frames[3][32],command;
    unsigned fifo_count,received,rx_flushes,position;
    int fail_after;
    bool selected,enabled;
    uint32_t milliseconds;
} Radio;
static void Select(void *context,bool high)
{
    Radio *radio=context;radio->selected=!high;if(!high)radio->position=0;
}
static void Enable(void *context,bool high) { ((Radio *)context)->enabled=high; }
static void Delay(void *context,uint32_t delay) { ((Radio *)context)->milliseconds+=delay; }
static uint32_t Time(void *context) { return ((Radio *)context)->milliseconds; }
static Nrf24l01MappingStatus Exchange(void *context,const uint8_t *out,uint8_t *in,size_t count)
{
    Radio *radio=context;assert(radio->selected);
    if(radio->fail_after==0) return NRF24L01_MAPPING_ERROR;
    if(radio->fail_after>0)--radio->fail_after;
    for(size_t i=0;i<count;++i) {
        in[i]=0;
        if(radio->position++==0) {
            radio->command=out[i];in[i]=radio->registers[7];
            if(out[i]==0xE2) { radio->fifo_count=0;++radio->rx_flushes; }
        } else if(radio->command<=0x1F) {
            in[i]=radio->command==0x17?(radio->fifo_count==0?1:0):radio->registers[radio->command];
        } else if((radio->command&0xE0)==0x20) {
            unsigned address=radio->command&31;
            if(address==7)radio->registers[7]&=(uint8_t)~out[i];
            else radio->registers[address]=out[i];
        } else if(radio->command==0x61) {
            assert(radio->fifo_count>0);
            unsigned index=radio->position-2;in[i]=radio->frames[0][index];
            if(index==31) {
                --radio->fifo_count;memmove(radio->frames[0],radio->frames[1],sizeof(radio->frames[0])*radio->fifo_count);
            }
        }
    }
    return NRF24L01_MAPPING_OK;
}
static void Receive(void *context,uint8_t command,uint8_t length,const uint8_t *payload)
{
    Radio *radio=context;assert(command==3 && length==1 && payload[0]==7);++radio->received;
}
int main(void)
{
    Radio radio={.fail_after=-1};Nrf24l01Service service={
        .bus={&radio,Exchange},.chip_select={.context=&radio,.write=Select},.chip_enable={.context=&radio,.write=Enable},
        .clock={&radio,Time,Delay},.radio_channel=76,.radio_setup=14,.receive_context=&radio,.on_receive=Receive};
    assert(Nrf24l01_Initialize(&service)==NRF24L01_OK && radio.enabled);
    unsigned initial_flushes=radio.rx_flushes;uint8_t payload[30]={0};
    assert(Nrf24l01_Send(&service,3,30,payload)==NRF24L01_OK);
    assert(Nrf24l01_Send(&service,3,1,payload)==NRF24L01_BUSY);
    radio.registers[7]=0x60;radio.fifo_count=3;
    for(unsigned i=0;i<3;++i) { radio.frames[i][0]=3;radio.frames[i][1]=1;radio.frames[i][2]=7; }
    Nrf24l01_ProcessInterrupt(&service);
    assert(radio.received==3 && radio.fifo_count==0 && radio.rx_flushes==initial_flushes);
    assert(service.transmit_state==NRF24L01_TX_DELIVERED && !service.transmitting);
    assert(Nrf24l01_Send(&service,3,1,payload)==NRF24L01_OK);
    radio.registers[7]=0x70;radio.fifo_count=1;radio.frames[0][1]=31;
    Nrf24l01_ProcessInterrupt(&service);assert(service.rejected_frames==1 && service.transmit_state==NRF24L01_TX_RETRY_EXHAUSTED);
    radio.fail_after=0;assert(Nrf24l01_Send(&service,3,1,payload)==NRF24L01_IO_ERROR);
    assert(!service.transmitting && !radio.enabled);
    radio.fail_after=-1;radio.milliseconds=UINT32_MAX-49;
    assert(Nrf24l01_Send(&service,3,1,payload)==NRF24L01_OK);
    Nrf24l01_Supervise(&service,50);assert(service.transmit_state==NRF24L01_TX_TIMEOUT && !service.transmitting);
    RemoteControlInput input={.left_x=-32768,.left_y=32767,.right_x=-1,.right_y=2,.switches=4},decoded={0};uint8_t remote[9];
    RemoteControl_Encode(&input,remote);
    assert(!RemoteControl_DecodeFrame(&decoded,2,3,remote,9));
    assert(!RemoteControl_DecodeFrame(&decoded,3,3,remote,8));
    assert(RemoteControl_DecodeFrame(&decoded,3,3,remote,9));
    assert(decoded.left_x==-32768 && decoded.left_y==32767 && decoded.right_x==-1);
    return 0;
}
