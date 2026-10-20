/** @file navigation_math.c @brief 来源：CarMainControlBoard/Motion.c、IMU.c。 */
#include "navigation_math.h"
#include <math.h>
#define NAVIGATION_PI 3.14159265358979323846
double Navigation_DegreesFromFixed(int32_t coordinate)
{
    double value = (double)coordinate / 100000.0;
    int degrees = (int)(value / 100.0);
    return (double)degrees + (value - degrees * 100.0) / 60.0;
}
void Navigation_DistanceBearing(double latitude, double longitude, double target_latitude,
                                 double target_longitude, float *distance, float *bearing)
{
    double north = (target_latitude - latitude) * 111320.0;
    double east = (target_longitude - longitude) * 111320.0 * cos(latitude * NAVIGATION_PI / 180.0);
    *distance = (float)sqrt(north * north + east * east);
    *bearing = (float)(atan2(east, north) * 180.0 / NAVIGATION_PI);
    if (*bearing < 0) *bearing += 360.0f;
}
float Navigation_NormalizeError(float angle_degrees)
{
    if (!isfinite(angle_degrees)) return NAN;
    angle_degrees = fmodf(angle_degrees, 360.0f);
    if (angle_degrees > 180) angle_degrees -= 360;
    if (angle_degrees < -180) angle_degrees += 360;
    return angle_degrees;
}
float Navigation_CalibrateHeading(float heading, const float *angles, const float *corrections, size_t count)
{
    if (!isfinite(heading) || angles == NULL || corrections == NULL || count < 2) return NAN;
    for (size_t index = 0; index < count; ++index) {
        if (!isfinite(angles[index]) || !isfinite(corrections[index]) || angles[index] < 0 || angles[index] >= 360 ||
            (index != 0 && angles[index] <= angles[index - 1])) return NAN;
    }
    heading = fmodf(heading, 360.0f);
    if (heading < 0) heading += 360;
    if (heading >= 360) heading = 0;
    float first_angle, second_angle, first_correction, second_correction;
    if (heading < angles[0]) {
        first_angle = angles[count - 1] - 360; second_angle = angles[0];
        first_correction = corrections[count - 1]; second_correction = corrections[0];
    } else if (heading >= angles[count - 1]) {
        first_angle = angles[count - 1]; second_angle = angles[0] + 360;
        first_correction = corrections[count - 1]; second_correction = corrections[0];
    } else {
        size_t index = 0;
        while (index < count - 1 && !(heading >= angles[index] && heading < angles[index + 1])) ++index;
        first_angle = angles[index]; second_angle = angles[index + 1];
        first_correction = corrections[index]; second_correction = corrections[index + 1];
    }
    float ratio = (heading - first_angle) / (second_angle - first_angle);
    float result = heading + first_correction + ratio * (second_correction - first_correction);
    if (!isfinite(result)) return NAN;
    result = fmodf(result, 360.0f);
    if (result < 0) result += 360;
    if (result >= 360) result = 0;
    return result;
}
