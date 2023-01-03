#ifndef BASE64_H
#define BASE64_H

/*
 * 字符串 Base64 编码
 * @param src 源字符串
 * @param srclen 源字符串长度
 * @param dest 编码后的字符串缓冲区
 * @return 编码后的字符串
 */
char *base64_encode(const char *src, unsigned int srclen, char *dest);

/*
 * Base64 编码后的字符串解码
 * @param src 编码后的字符串
 * @param srclen 编码后的字符串长度
 * @param dest 解码后的字符串缓冲区
 * @return 解码后的字符串
 */
char *base64_decode(const char *src, unsigned int srclen, char *dest);
#endif  // BASE64_H