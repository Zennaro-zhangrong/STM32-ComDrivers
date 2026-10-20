/** @file coordinate_transform.h
 * @brief Clarke/Park 正逆变换，输入输出均为浮点数，保留小数精度。
 * @note 输入值须在 float 范围内，角度单位为度；各对象独立，可用于多电机。
 */
#ifndef COMDRIVERS_COORDINATE_TRANSFORM_H
#define COMDRIVERS_COORDINATE_TRANSFORM_H
#ifdef __cplusplus
extern "C" {
#endif
typedef struct { float phase_a, phase_b, phase_c, alpha, beta; } ClarkeTransform;
typedef struct { float alpha, beta, direct, quadrature, angle_degrees; } ParkTransform;
/** @brief 三相 abc 转静止坐标 alpha/beta，结果写回对象。 */
void Clarke_Forward(ClarkeTransform *transform);
/** @brief 静止坐标转三相；不重建零序分量。 */
void Clarke_Inverse(ClarkeTransform *transform);
/** @brief 静止坐标转旋转坐标 direct/quadrature。 */
void Park_Forward(ParkTransform *transform);
/** @brief 旋转坐标转静止坐标，角度先按360度约简。 */
void Park_Inverse(ParkTransform *transform);
#ifdef __cplusplus
}
#endif
#endif
