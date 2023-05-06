#ifndef BASE64_H
#define BASE64_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 字符串 Base64 编码
 * @param dest [out] 编码后的字符串缓冲区
 * @param src [in] 源字符串
 * @param srclen [in] 源字符串长度
 * @return 编码后的字符串长度
 */
size_t base64_encode(char *dest, const char *src, size_t srclen);

/*
 * Base64 编码后的字符串解码
 * @param dest [out] 解码后的字符串缓冲区
 * @param src [in] 编码后的字符串
 * @param srclen [in] 编码后的字符串长度
 * @return 解码后的字符串长度
 */
size_t base64_decode(char *dest, const char *src, size_t srclen);

#ifdef __cplusplus
}
#endif

#endif  // BASE64_H
