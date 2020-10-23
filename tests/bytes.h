#pragma once

#include <stdint.h>

static inline uint16_t be16(uint16_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[2]; uint16_t b; } u = {
        .a = { (u8)(v >> 8), (u8)(v) }
    };
    return u.b;
}

static inline uint16_t le16(uint16_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[2]; uint16_t b; } u = {
        .a = { (u8)(v), (u8)(v >> 8) }
    };
    return u.b;
}

static inline uint32_t be32(uint32_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[4]; uint32_t b; } u = {
        .a = { (u8)(v >> 24), (u8)(v >> 16), (u8)(v >> 8), (u8)(v) }
    };
    return u.b;
}

static inline uint32_t le32(uint32_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[4]; uint32_t b; } u = {
        .a = { (u8)(v), (u8)(v >> 8), (u8)(v >> 16), (u8)(v >> 24) }
    };
    return u.b;
}

static inline uint64_t be64(uint64_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[8]; uint64_t b; } u = {
        .a = { (u8)(v >> 56), (u8)(v >> 48), (u8)(v >> 40), (u8)(v >> 32),
               (u8)(v >> 24), (u8)(v >> 16), (u8)(v >> 8), (u8)(v) }
    };
    return u.b;
}

static inline uint64_t le64(uint64_t v)
{
    typedef uint8_t u8;
    union { uint8_t a[8]; uint64_t b; } u = {
        .a = { (u8)(v), (u8)(v >> 8), (u8)(v >> 16), (u8)(v >> 24),
               (u8)(v >> 32), (u8)(v >> 40), (u8)(v >> 48), (u8)(v >> 56) }
    };
    return u.b;
}

#define htobe16(x) be16(x)
#define htole16(x) le16(x)
#define be16toh(x) be16(x)
#define le16toh(x) le16(x)

#define htobe32(x) be32(x)
#define htole32(x) le32(x)
#define be32toh(x) be32(x)
#define le32toh(x) le32(x)

#define htobe64(x) be64(x)
#define htole64(x) le64(x)
#define be64toh(x) be64(x)
#define le64toh(x) le64(x)
