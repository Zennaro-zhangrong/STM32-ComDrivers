/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Middleware/command_dispatcher.h"
#include "../Middleware/spi_frame_receiver.h"
static void Handler(void *context,uint8_t length,const uint8_t *data) { (void)length;(void)data; ++*(unsigned *)context; }
static void Feed(SpiFrameReceiver *receiver,uint8_t value,bool complete)
{
    uint8_t *address=SpiFrameReceiver_NextAddress(receiver); assert(address!=NULL);
    assert(address==SpiFrameReceiver_NextAddress(receiver));
    *address=value; assert(SpiFrameReceiver_CommitByte(receiver)==complete);
}
int main(void)
{
    unsigned calls=0; CommandEntry entries[]={{1,NULL},{2,Handler}}; CommandTable table={entries,2,&calls};
    assert(!CommandDispatcher_Run(&table,1,0,NULL));
    assert(!CommandDispatcher_Run(&table,2,1,NULL));
    assert(CommandDispatcher_Run(&table,2,0,NULL) && calls==1);
    SpiFrameReceiver receiver; SpiFrameReceiver_Initialize(&receiver);
    Feed(&receiver,1,false); Feed(&receiver,2,false); assert(SpiFrameReceiver_Acquire(&receiver)==NULL);
    Feed(&receiver,10,false); Feed(&receiver,3,false); Feed(&receiver,99,true);
    assert(!SpiFrameReceiver_CommitByte(&receiver));
    const SpiReceiveFrame *first=SpiFrameReceiver_Acquire(&receiver); assert(first && first->bytes[2]==99);
    Feed(&receiver,20,false); Feed(&receiver,3,false); Feed(&receiver,88,true);
    assert(SpiFrameReceiver_NextAddress(&receiver)==NULL);
    SpiFrameReceiver_Release(&receiver,first);
    Feed(&receiver,30,false); Feed(&receiver,3,false); Feed(&receiver,77,true);
    const SpiReceiveFrame *second=SpiFrameReceiver_Acquire(&receiver); assert(second && second->bytes[0]==20);
    SpiFrameReceiver_Release(&receiver,second);
    const SpiReceiveFrame *third=SpiFrameReceiver_Acquire(&receiver); assert(third && third->bytes[0]==30);
    SpiFrameReceiver_Release(&receiver,third);
    Feed(&receiver,10,false); SpiFrameReceiver_Abort(&receiver);
    assert(SpiFrameReceiver_Acquire(&receiver)==NULL);
    Feed(&receiver,10,false); Feed(&receiver,36,false);
    for(unsigned i=2;i<36;++i) Feed(&receiver,(uint8_t)i,i==35);
    assert(SpiFrameReceiver_Acquire(&receiver)->cursor==36);
    return 0;
}
