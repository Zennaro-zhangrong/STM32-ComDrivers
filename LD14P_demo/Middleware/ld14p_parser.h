/** @file ld14p_parser.h
 * @brief LD14P最多56包的批次解析与8米网格转换，验证CRC并清理无效尾部。
 */
#ifndef COMDRIVERS_LD14P_PARSER_H
#define COMDRIVERS_LD14P_PARSER_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
enum { LD14P_PACKET_BYTES=47, LD14P_BATCH_PACKETS=56, LD14P_BATCH_POINTS=672,
       LD14P_BATCH_BYTES=2632, LD14P_POLAR_BYTES=2688, LD14P_GRID_BYTES=1344 };
/** @brief 仅检查包头；bytes必须为47的正整数倍且不超过2632，不代表CRC通过。 */
uint8_t Ld14p_CheckHeaders(const uint8_t *packets, size_t bytes);
/** @brief CRC-8，初值0、多项式0x4D、不反射、无最终异或。 */
uint8_t Ld14p_Crc8(const uint8_t *data, size_t length);
/** @brief 返回有效点数；逐包检查头部/CRC/角度，坏包跳过，polar尾部清零。
 * @note bytes为47的正整数倍且<=2632；capacity至少bytes/47*48；输出不可与输入重叠。
 */
uint16_t Ld14p_ParseBatch(const uint8_t *packets, size_t bytes, uint8_t *polar, size_t capacity);
/** @brief 按有效点数转换；polar至少count*4字节，grid至少count*2字节，尾部清零。
 * @param angle_offset 单位0.01度，正负均可，内部归一化到[0,36000)。
 */
bool Ld14p_ConvertGrid(const uint8_t *polar, size_t count, uint8_t *grid, size_t capacity, int32_t angle_offset);
#ifdef __cplusplus
}
#endif
#endif
