/** @file navigation_math.h @brief 近距平面导航、航向归一化与标定插值。 */
#ifndef COMDRIVERS_NAVIGATION_MATH_H
#define COMDRIVERS_NAVIGATION_MATH_H
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/** @brief NMEA 度分定点值转十进制度。 */
double Navigation_DegreesFromFixed(int32_t coordinate);
/** @brief 近距距离和方位角；经纬度单位为度，距离为米，方位北为 0、顺时针为正。 */
void Navigation_DistanceBearing(double latitude, double longitude, double target_latitude,
                                 double target_longitude, float *distance, float *bearing);
/** @brief 有限角度归一化到 [-180,180]；不接受无穷值。 */
float Navigation_NormalizeError(float angle_degrees);
/** @brief 环形标定表插值；angles 必须严格递增且位于 [0,360)，count>=2。 */
float Navigation_CalibrateHeading(float heading, const float *angles, const float *corrections, size_t count);
#ifdef __cplusplus
}
#endif
#endif
