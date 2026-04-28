#ifndef PID_H_
#define PID_H_


#ifdef __cplusplus
extern "C" {
#endif

// PID 对象结构体
typedef struct {
    float P;                  // 比例系数
    float I;                  // 积分系数
    float D;                  // 微分系数
    float Expected_value;     // 目标期望值
    float Actual_value;       // 当前实际值
    float Error_value;        // 当前误差
    float Last_error_value;   // 上次误差
    float Sum_error;          // 积分累加值
    float Maximum_output;     // 最大输出限幅
    short PID_output;         // 实际输出
    char *PID_name;           // PID名称
} PID_Obj;

// 函数声明
void PID_paramet_set(PID_Obj *pobj, float P, float I, float D);
void PID_operation(PID_Obj *pobj);
void PID_reset(PID_Obj *pobj); 

#ifdef __cplusplus
}
#endif

#endif
