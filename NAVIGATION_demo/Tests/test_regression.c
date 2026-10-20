/** @file test_regression.c @brief 文本坏帧、二进制重同步、零坐标和设备失败测试。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "../Service/navigation_service.h"
#include "../Service/heading_control_service.h"
#include "../Service/imu_heading_service.h"
#include "../Middleware/navigation_math.h"
static void Sentence(char *output,const char *body)
{
    unsigned checksum=0;for(const char *p=body;*p;++p) checksum^=(unsigned char)*p;
    sprintf(output,"$%s*%02X",body,checksum);
}
static void Feed(WaypointDecoder *decoder,const uint8_t *data,size_t count,uint32_t time)
{
    for(size_t i=0;i<count;++i) WaypointDecoder_Feed(decoder,data[i],time+(uint32_t)i);
}
static NavigationMappingStatus ReadImu(void *context,uint8_t address,uint8_t reg,uint8_t *data,size_t count)
{
    (void)address;(void)reg; assert(count==12);
    if(*(int *)context) return NAVIGATION_MAPPING_ERROR;
    memset(data,0,count);data[8]=0xdb;data[9]=0x0f;data[10]=0xc9;data[11]=0x3f;return NAVIGATION_MAPPING_OK;
}
int main(void)
{
    char sentence[128];NavigationPosition position;
    Sentence(sentence,"GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W");
    assert(Navigation_ParseRmc(sentence,&position) && position.latitude==480703800 && position.longitude==113100000);
    sentence[strlen(sentence)-1]^=1;assert(!Navigation_ParseRmc(sentence,&position) && position.latitude==0);
    Sentence(sentence,"GNRMC,,A,,N,01131.000,E,,,230394,,");assert(!Navigation_ParseRmc(sentence,&position));
    Sentence(sentence,"GNRMC,0,A,9060.0,N,18100.0,E,,,230394,,");assert(!Navigation_ParseRmc(sentence,&position));
    Sentence(sentence,"GNRMC,0,A,0000.000,S,00000.000,W,,,230394,,");assert(Navigation_ParseRmc(sentence,&position));
    NavigationService service={0};
    for(size_t i=0;i<strlen(sentence);++i) NavigationService_Feed(&service,(uint8_t)sentence[i]);
    assert(NavigationService_Feed(&service,'\n') && service.position_valid);
    Sentence(sentence,"GNRMC,0,V,,,,,,,230394,,");
    for(size_t i=0;i<strlen(sentence);++i) NavigationService_Feed(&service,(uint8_t)sentence[i]);
    assert(!NavigationService_Feed(&service,'\n') && !service.position_valid);
    for(unsigned i=0;i<200;++i) NavigationService_Feed(&service,'x');
    assert(!NavigationService_Feed(&service,'\n') && service.line_length==0);

    NavigationPosition storage[4];WaypointDecoder decoder;WaypointDecoder_Initialize(&decoder,storage,4);
    const uint8_t unknown[]={0x55,0,2,1,2},malformed[]={0x0A,0,1,255};
    Feed(&decoder,unknown,sizeof(unknown),10);Feed(&decoder,malformed,sizeof(malformed),20);
    assert(decoder.state==0 && decoder.write_index==0);
    const uint8_t waypoint[]={0x0A,0,8,255,255,255,255,0,0,0,1};
    Feed(&decoder,waypoint,6,100);assert(decoder.write_index==0);
    Feed(&decoder,waypoint,sizeof(waypoint),700);assert(decoder.write_index==1);
    assert(WaypointDecoder_Read(&decoder,&position) && position.longitude==-1 && position.latitude==1);
    const uint8_t clear[]={0x0E,0,0};Feed(&decoder,clear,3,800);assert(decoder.write_index==0);
    /* 整帧未完成时，前一个已组装的航点也不得公开。 */
    const uint8_t partial[]={0x0A,0,16,0,0,0,0,0,0,0,0};Feed(&decoder,partial,sizeof(partial),900);
    assert(decoder.write_index==0);WaypointDecoder_ResetFrame(&decoder);
    storage[0]=(NavigationPosition){0,0};storage[1]=(NavigationPosition){100000,0};decoder.write_index=2;
    service.position_valid=true;service.position=(NavigationPosition){0,0};
    assert(NavigationService_Advance(&service,&decoder,1) && service.target_valid && service.target.longitude==100000);
    assert(service.target_bearing_degrees>89 && service.target_bearing_degrees<91);
    service.position=service.target;assert(!NavigationService_Advance(&service,&decoder,1) && !service.target_valid);

    HeadingControlService heading;HeadingControl_Initialize(&heading);
    for(unsigned i=0;i<100;++i) { HeadingControl_Update(&heading,true,180,0);assert(heading.turn<=156 && fabs(heading.integral)<=200); }
    assert(heading.integral==0); HeadingControl_Update(&heading,true,NAN,0);assert(heading.turn==0);
    assert(isnan(Navigation_NormalizeError(INFINITY)));
    const float angles[]={0,180},corrections[]={0,0};
    assert(fabs(Navigation_CalibrateHeading(-1,angles,corrections,2)-359)<0.001f);
    assert(Navigation_CalibrateHeading(-1e-30f,angles,corrections,2)==0);
    assert(isnan(Navigation_CalibrateHeading(0,angles,corrections,1)));
    int fail=0;ImuHeadingService imu={.bus={.context=&fail,.read=ReadImu}};float yaw=0;
    assert(ImuHeading_TryRead(&imu,&yaw) && fabs(yaw-270)<0.001f);
    fail=1;assert(!ImuHeading_TryRead(&imu,&yaw) && yaw>269);assert(isnan(ImuHeading_Read(&imu)));
    return 0;
}
