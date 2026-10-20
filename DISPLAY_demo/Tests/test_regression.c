/** @file test_regression.c @brief 队列所有权、满屏分块、失败恢复与触摸快照测试。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>
#include "../Service/display_transfer_service.h"
#include "../Service/ili9341_service.h"
#include "../Service/ft6336g_service.h"
typedef struct { unsigned successes, failures; } Owner;
typedef struct { bool fail; unsigned starts; size_t bytes; } Queue;
static bool Start(void *context,const DisplayTransfer *transfer)
{
    Queue *queue=context;++queue->starts;queue->bytes=transfer->byte_count;return !queue->fail;
}
static void Complete(void *context,bool success)
{
    Owner *owner=context;if(success)++owner->successes;else ++owner->failures;
}
typedef struct { bool selected,fail_dma,fail_command; unsigned chunks; size_t total; uint8_t touch[11]; unsigned reads; bool fail_read; } Device;
static void Pin(void *context,bool high) { ((Device *)context)->selected=!high; }
static void NoPin(void *context,bool high) { (void)context;(void)high; }
static DisplayMappingStatus Write(void *context,const uint8_t *data,size_t length)
{
    (void)data;(void)length;return ((Device *)context)->fail_command?DISPLAY_MAPPING_ERROR:DISPLAY_MAPPING_OK;
}
static DisplayMappingStatus Dma(void *context,const uint8_t *data,uint16_t length)
{
    Device *device=context;(void)data;assert(length>0 && length%2==0 && device->selected);
    if(device->fail_dma)return DISPLAY_MAPPING_ERROR;
    ++device->chunks;device->total+=length;return DISPLAY_MAPPING_OK;
}
static DisplayMappingStatus ReadTouch(void *context,uint8_t address,uint8_t reg,uint8_t *data,size_t length)
{
    Device *device=context;(void)address;assert(reg==2 && length==11);++device->reads;
    if(device->fail_read)return DISPLAY_MAPPING_ERROR;
    memcpy(data,device->touch,11);return DISPLAY_MAPPING_OK;
}
int main(void)
{
    static uint8_t pixels[320*240*2];Queue queue={0};Owner a={0},b={0},c={0};
    DisplayTransferService service={.context=&queue,.start=Start,.complete=Complete};
    assert(DisplayTransfer_Enqueue(&service,&a,0,0,319,239,pixels) && queue.bytes==153600);
    assert(DisplayTransfer_Enqueue(&service,&b,0,0,1,1,pixels));
    assert(!DisplayTransfer_Enqueue(&service,&c,0,0,1,1,pixels));
    DisplayTransfer_OnComplete(&service);assert(a.successes==1 && b.successes==0 && service.count==1);
    DisplayTransfer_OnError(&service);assert(b.failures==1 && service.count==0);
    DisplayTransfer_OnComplete(&service);assert(a.successes==1 && b.failures==1);
    queue.fail=true;assert(DisplayTransfer_Enqueue(&service,&c,0,0,1,1,pixels));
    assert(c.failures==1 && service.count==0 && !service.active);
    assert(!DisplayTransfer_Enqueue(&service,&c,2,0,1,1,pixels));
    Device device={0};Ili9341Service screen={.width=320,.height=240,.writer={&device,Write},
        .chip_select={.context=&device,.write=Pin},.data_command={.write=NoPin},.pixel_context=&device,.start_pixels=Dma};
    assert(Ili9341_SetWindow(&screen,0,0,319,239)==DISPLAY_MAPPING_OK);
    assert(Ili9341_StartPixels(&screen,pixels,sizeof(pixels))==DISPLAY_MAPPING_OK);
    assert(Ili9341_StartPixels(&screen,pixels,2)==DISPLAY_MAPPING_BUSY);
    assert(Ili9341_OnPixelsComplete(&screen)==DISPLAY_MAPPING_BUSY);
    assert(Ili9341_OnPixelsComplete(&screen)==DISPLAY_MAPPING_BUSY);
    assert(Ili9341_OnPixelsComplete(&screen)==DISPLAY_MAPPING_OK);
    assert(device.chunks==3 && device.total==sizeof(pixels) && !device.selected);
    device.fail_dma=true;assert(Ili9341_StartPixels(&screen,pixels,2)==DISPLAY_MAPPING_ERROR);
    assert(!device.selected && !screen.pixels_active);
    device.fail_dma=false;assert(Ili9341_SetWindow(&screen,0,0,1,1)==DISPLAY_MAPPING_OK);
    assert(Ili9341_StartPixels(&screen,pixels,sizeof(pixels))==DISPLAY_MAPPING_OK);
    device.fail_dma=true;assert(Ili9341_OnPixelsComplete(&screen)==DISPLAY_MAPPING_ERROR && !device.selected);
    device.fail_command=true;assert(Ili9341_SetWindow(&screen,0,0,1,1)==DISPLAY_MAPPING_ERROR);
    assert(Ili9341_StartPixels(&screen,pixels,2)==DISPLAY_MAPPING_ERROR);
    Ft6336gService touch={.bus={.context=&device,.read=ReadTouch},.width=320,.height=240};
    device.touch[0]=1;assert(Ft6336g_ReadCoordinates(&touch)==1 && touch.x[0]==319 && touch.y[0]==0 && device.reads==1);
    device.touch[0]=2;device.touch[7]=0;device.touch[8]=239;device.touch[9]=1;device.touch[10]=63;
    assert(Ft6336g_ReadCoordinates(&touch)==2 && touch.x[1]==0 && touch.y[1]==239);
    device.fail_read=true;assert(Ft6336g_ReadCoordinates(&touch)==255 && touch.x[0]==319);
    device.fail_read=false;device.touch[0]=0;assert(Ft6336g_ReadCoordinates(&touch)==0 && touch.x[0]==319);
    device.touch[0]=3;assert(Ft6336g_ReadCoordinates(&touch)==255);
    return 0;
}
