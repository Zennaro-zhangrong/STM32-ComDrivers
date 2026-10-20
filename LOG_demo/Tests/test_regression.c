/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Service/log_service.h"
typedef struct { unsigned depth, enters, leaves; } Lock;
static uintptr_t Enter(void *context) { Lock *lock=context; ++lock->enters; return lock->depth++; }
static void Leave(void *context,uintptr_t previous) { Lock *lock=context; assert(lock->depth==previous+1); lock->depth=(unsigned)previous; ++lock->leaves; }
typedef struct { char bytes[512]; unsigned count; bool fail; Lock *lock; } Writer;
static LogMappingStatus Write(void *context,const uint8_t *data,size_t length)
{
    Writer *writer=context; assert(writer->lock->depth==0);
    if(writer->fail) return LOG_MAPPING_ERROR;
    memcpy(writer->bytes+writer->count,data,length); writer->count+=(unsigned)length; return LOG_MAPPING_OK;
}
int main(void)
{
    Lock lock={0}; Writer writer={.lock=&lock}; uint8_t storage[8]; LogService service;
    LogService_Initialize(&service,storage,sizeof(storage),(LogMappingByteWriter){&writer,Write});
    assert(!RingBuffer_SetCriticalSection(&service.buffer,(RingBufferCriticalSection){&lock,Enter,NULL}));
    assert(RingBuffer_SetCriticalSection(&service.buffer,(RingBufferCriticalSection){&lock,Enter,Leave}));
    assert(LogService_Print(&service,"abcdefgh")==LOG_PRINT_OK);
    assert(LogService_Print(&service,"x")==LOG_PRINT_QUEUE_FULL);
    writer.fail=true; assert(LogService_Process(&service)==0 && service.pending_valid);
    assert(LogService_Print(&service,"i")==LOG_PRINT_OK);
    writer.fail=false; assert(LogService_Process(&service)==9 && memcmp(writer.bytes,"abcdefghi",9)==0);
    char long_message[300]; memset(long_message,'x',299); long_message[299]=0;
    assert(LogService_Print(&service,"%s",long_message)==LOG_PRINT_TOO_LONG);
    assert(service.rejected_messages==2 && service.send_failures==1 && service.buffer.count==0);
    for(unsigned i=0;i<1000;++i) {
        uint8_t value;
        assert(RingBuffer_WriteByte(&service.buffer,(uint8_t)i));
        assert(RingBuffer_ReadByte(&service.buffer,&value) && value==(uint8_t)i);
    }
    assert(lock.depth==0 && lock.enters==lock.leaves);
    RingBuffer empty; RingBuffer_Initialize(&empty,NULL,0);
    assert(!RingBuffer_WriteByte(&empty,1));
    return 0;
}
