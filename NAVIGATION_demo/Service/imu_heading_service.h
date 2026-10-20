/** @file imu_heading_service.h @brief 0x23 地址模块的欧拉航向读取与外部标定表接口。 */
#ifndef COMDRIVERS_IMU_HEADING_SERVICE_H
#define COMDRIVERS_IMU_HEADING_SERVICE_H
#include "../Mapping/mapping.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    NavigationMappingRegisterBus bus;
    uint8_t address;
    const float *calibration_angles, *calibration_corrections;
    size_t calibration_count; /**< 非零时必须 >=2，数组等长且角度严格递增。 */
} ImuHeadingService;
/** @brief 返回校准航向（度）；总线或数值错误返回NAN。 */
float ImuHeading_Read(ImuHeadingService *service);
/** @brief 成功返回true并写入航向；失败保持调用方输出不变。 */
bool ImuHeading_TryRead(ImuHeadingService *service, float *heading_degrees);
#ifdef __cplusplus
}
#endif
#endif
