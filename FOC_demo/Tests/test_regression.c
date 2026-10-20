/** @file test_regression.c @brief 霍尔、调制、协议超时及控制模式边界测试。 */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <math.h>
#include <limits.h>
#include "../Middleware/hall_estimator.h"
#include "../Middleware/motor_link_protocol.h"
#include "../Middleware/coordinate_transform.h"
#include "../Service/motor_control_service.h"
typedef struct { unsigned calls; uint16_t a,b,c; } Output;
static void Write(void *context,uint16_t a,uint16_t b,uint16_t c)
{
    Output *out=context; ++out->calls; out->a=a;out->b=b;out->c=c;
}
typedef struct { unsigned calls; int16_t forward,turn; } Frame;
static void Received(void *context,uint32_t state,int16_t forward,int16_t turn)
{
    Frame *frame=context; assert(state==1); ++frame->calls; frame->forward=forward;frame->turn=turn;
}
static void SendFrame(MotorLinkDecoder *decoder,unsigned bits,uint32_t now)
{
    uint32_t joystick=MotorLink_PackJoystick(-32768,32767);
    for(unsigned bit=0;bit<bits;++bit)
        MotorLink_OnClock(decoder,false,bit<32 && (1u&(UINT32_C(0x80000000)>>bit)),
            bit<32 && (joystick&(UINT32_C(0x80000000)>>bit)),now);
    MotorLink_OnClock(decoder,true,false,false,now);
}
int main(void)
{
    float history[3]; HallEstimator hall;
    const uint8_t bits[]={6,4,5,1,3,2,6};
    HallEstimator_Initialize(&hall,history,3,bits[0],UINT32_MAX-99);
    assert(HallEstimator_ConfigureTimebase(&hall,6000,2));
    uint32_t tick=UINT32_MAX-99;
    for(unsigned i=1;i<7;++i) { tick+=100; HallEstimator_OnEdge(&hall,bits[i],tick); assert(hall.direction==1); }
    assert(fabs(hall.frequency-5)<0.001f);
    HallEstimator_OnEdge(&hall,2,tick+100); assert(hall.direction==-1 && hall.frequency<0);
    for(unsigned i=0;i<1000;++i) assert(HallEstimator_Angle(&hall,tick+i)>=0 && HallEstimator_Angle(&hall,tick+i)<360);
    HallEstimator_Supervise(&hall,tick+301); assert(hall.frequency==0 && hall.pulse_period==0);
    for(unsigned i=0;i<3;++i) assert(history[i]==0);
    HallEstimator_OnEdge(&hall,0,tick+400); assert(hall.frequency==0);
    HallEstimator_Initialize(&hall,NULL,0,6,0); HallEstimator_OnEdge(&hall,4,100); assert(hall.frequency==0);
    HallEstimator_Initialize(&hall,history,3,6,0); HallEstimator_OnEdge(&hall,4,100000); assert(hall.pulse_period==100000);

    SpaceVectorPwm pwm={.maximum_duty=1679};
    const float angles[]={0,0.5f,-0.5f,-1e-30f,60,359.9f,1e30f,-1e30f};
    for(unsigned i=0;i<sizeof(angles)/sizeof(angles[0]);++i) {
        SpaceVectorPwm_FromPolar(&pwm,1e30f,angles[i]);
        assert(pwm.sector>=1 && pwm.sector<=6);
        assert(pwm.duty_a>=0 && pwm.duty_a<=1679 && pwm.duty_b>=0 && pwm.duty_b<=1679 && pwm.duty_c>=0 && pwm.duty_c<=1679);
    }
    SpaceVectorPwm_FromPolar(&pwm,100,0.5f); assert(pwm.angle_degrees==0.5f);
    SpaceVectorPwm_FromPolar(&pwm,NAN,0); assert(pwm.duty_a==0 && pwm.duty_b==0 && pwm.duty_c==0);
    pwm.alpha=(float)INT_MAX;pwm.beta=(float)INT_MAX;SpaceVectorPwm_Update(&pwm); assert(pwm.duty_a<=1679);
    ParkTransform park={.alpha=0.25f,.beta=-0.5f,.angle_degrees=37};
    Park_Forward(&park);Park_Inverse(&park); assert(fabs(park.alpha-0.25f)<0.00001f && fabs(park.beta+0.5f)<0.00001f);

    Frame frame={0}; MotorLinkDecoder decoder={.context=&frame,.on_frame=Received};
    assert(MotorLink_HasTimedOut(&decoder,0));
    SendFrame(&decoder,31,10);SendFrame(&decoder,33,20);assert(frame.calls==0);
    SendFrame(&decoder,32,UINT32_MAX-99); assert(frame.calls==1 && frame.forward==INT16_MIN && frame.turn==INT16_MAX);
    assert(!MotorLink_HasTimedOut(&decoder,399)); assert(MotorLink_HasTimedOut(&decoder,400));
    SendFrame(&decoder,32,450); assert(!MotorLink_HasTimedOut(&decoder,500));

    Output output={0}; MotorControlService motor={.mode=MOTOR_SPEED_CONTROL,.maximum_current_target=20};
    motor.output=(FocMappingThreePhasePwm){&output,Write};motor.modulator.maximum_duty=1679;
    motor.speed=(FocIncrementalPidController){.proportional_gain=1,.maximum_output=100,.minimum_output=-100,.target=100};
    MotorControl_UpdateSpeed(&motor); assert(motor.speed.output<=100);
    motor.mode=MOTOR_BRAKE; motor.direct_current.previous_error=17;motor.quadrature_current.second_previous_error=12;
    MotorControl_UpdateCurrent(&motor);assert(output.a==0 && output.b==0 && output.c==0);
    assert(motor.direct_current.previous_error==0 && motor.quadrature_current.second_previous_error==0);
    MotorControlService second={.maximum_current_target=30};
    MotorControl_SetDifferentialTarget(&motor,&second,MOTOR_CURRENT_CONTROL,100,50);
    assert(motor.quadrature_current.target==-20 && second.quadrature_current.target==30);
    MotorControl_SetMode(&motor,MOTOR_SPEED_CONTROL);
    assert(motor.quadrature_current.target==0 && motor.speed.target==0);
    motor.speed.target=NAN;MotorControl_UpdateSpeed(&motor);assert(motor.quadrature_current.target==0);
    motor.current_phase_a=NAN;MotorControl_UpdateCurrent(&motor);
    assert(output.a==0 && output.b==0 && output.c==0);
    return 0;
}
