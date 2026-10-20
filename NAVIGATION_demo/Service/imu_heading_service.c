/** @file imu_heading_service.c @brief 显式解码线上的小端IEEE754数值，错误不会伪装为零度。 */
#include "imu_heading_service.h"
#include "../Middleware/navigation_math.h"
#include <math.h>
bool ImuHeading_TryRead(ImuHeadingService *service, float *heading_degrees)
{
    if (heading_degrees == NULL || service->bus.read == NULL) return false;
    uint8_t data[12];
    if (service->bus.read(service->bus.context, service->address, 0x26, data, sizeof(data)) != NAVIGATION_MAPPING_OK) return false;
    uint32_t bits = (uint32_t)data[8] | ((uint32_t)data[9] << 8) | ((uint32_t)data[10] << 16) | ((uint32_t)data[11] << 24);
    unsigned exponent = (bits >> 23) & 255;
    if (exponent == 255) return false;
    uint32_t fraction = bits & UINT32_C(0x7FFFFF);
    float yaw = exponent == 0 ? ldexpf((float)fraction, -149) : ldexpf((float)(fraction | UINT32_C(0x800000)), (int)exponent - 150);
    if (bits >> 31) yaw = -yaw;
    /* 先约简弧度再换算，避免有限大输入乘法溢出。 */
    yaw = fmodf(-yaw, 6.2831853071795864769f) * 57.29577951308232f;
    if (yaw < 0) yaw += 360;
    if (yaw >= 360) yaw = 0;
    if (service->calibration_count != 0)
        yaw = Navigation_CalibrateHeading(yaw, service->calibration_angles, service->calibration_corrections, service->calibration_count);
    if (!isfinite(yaw)) return false;
    *heading_degrees = yaw;
    return true;
}
float ImuHeading_Read(ImuHeadingService *service)
{
    float heading;
    return ImuHeading_TryRead(service, &heading) ? heading : NAN;
}
