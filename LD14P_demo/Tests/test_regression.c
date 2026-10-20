/** @file test_regression.c @brief 修复后的边界与错误路径回归测试；不访问真实硬件。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "../Middleware/ld14p_parser.h"
static void Put(uint8_t *p,uint16_t value) { p[0]=(uint8_t)value; p[1]=(uint8_t)(value>>8); }
static uint16_t Get(const uint8_t *p) { return (uint16_t)(p[0]|((uint16_t)p[1]<<8)); }
int main(void)
{
    assert(Ld14p_Crc8((const uint8_t *)"123456789",9)==0xC3);
    uint8_t packet[47]={0x54,0x2c},polar[48],grid[24];
    Put(packet+4,35900); Put(packet+42,100);
    for(unsigned i=0;i<12;++i) { Put(packet+6+i*3,1000); packet[8+i*3]=20; }
    packet[46]=Ld14p_Crc8(packet,46);
    assert(Ld14p_CheckHeaders(packet,47)); assert(!Ld14p_CheckHeaders(packet,46));
    assert(Ld14p_ParseBatch(packet,47,polar,48)==12);
    assert(Get(polar+2)==35900 && Get(polar+46)==100);
    assert(Ld14p_ConvertGrid(polar,12,grid,24,0));
    assert(grid[0]>128 && grid[1]>=128 && grid[22]>128 && grid[23]<=128);
    packet[8]=0; packet[46]=Ld14p_Crc8(packet,46);
    assert(Ld14p_ParseBatch(packet,47,polar,48)==11);
    for(unsigned i=44;i<48;++i) assert(polar[i]==0);
    assert(Ld14p_ConvertGrid(polar,11,grid,24,INT32_MAX)); assert(grid[22]==0 && grid[23]==0);
    packet[46]^=1; assert(Ld14p_ParseBatch(packet,47,polar,48)==0);
    for(unsigned i=0;i<48;++i) assert(polar[i]==0);
    assert(!Ld14p_ConvertGrid(polar,12,grid,23,0));
    return 0;
}
