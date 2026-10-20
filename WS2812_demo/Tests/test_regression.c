/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Middleware/ws2812_encoder.h"
static unsigned DecodeByte(const uint16_t *wave)
{
    unsigned value=0; for(unsigned i=0;i<8;++i) value=(value<<1)|(wave[i]==2); return value;
}
int main(void)
{
    uint16_t output[600]; uint32_t color=0x00FFFFFF;
    size_t reset=Ws2812_ResetCycles(300000,1250);
    assert(reset==240 && Ws2812_RequiredSamples(1,reset)==504);
    assert(Ws2812_RequiredSamples(SIZE_MAX,reset)==0);
    assert(Ws2812_ResetCycles(50,0)==0);
    output[0]=77; assert(Ws2812_Encode(output,503,&color,1,255,1,2,reset)==0 && output[0]==77);
    for(unsigned brightness=0;brightness<=255;++brightness) {
        assert(Ws2812_Encode(output,600,&color,1,(uint8_t)brightness,1,2,reset)==504);
        assert(DecodeByte(output+reset)==brightness);
        for(size_t i=0;i<reset;++i) assert(output[i]==0 && output[264+i]==0);
    }
    return 0;
}
