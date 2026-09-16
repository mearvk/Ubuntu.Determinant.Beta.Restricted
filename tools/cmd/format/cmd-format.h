#ifndef CMD_FORMAT_H
#define CMD_FORMAT_H

#include <stdint.h>
#include <stddef.h>

#define CMD_MAGIC       UINT32_C(0x434D4428) /* "CMD(" */
#define CMD_VERSION     UINT16_C(0x0100)
#define CMD_HEADER_SIZE 96U
#define CMD_MIN_JDK     28U

#define CMD_FLAG_EMBEDDED_JAR    UINT16_C(1 << 0)
#define CMD_FLAG_EMBEDDED_CLASS  UINT16_C(1 << 1)
#define CMD_FLAG_EXTERNAL_REF    UINT16_C(1 << 2)
#define CMD_FLAG_NATIVE_IMAGE    UINT16_C(1 << 3)
#define CMD_FLAG_PINNABLE        UINT16_C(1 << 4)
#define CMD_FLAG_HEADLESS        UINT16_C(1 << 5)
#define CMD_FLAG_NEGAMANE        UINT16_C(1 << 6)
#define CMD_FLAG_GRAIN_AWARE     UINT16_C(1 << 7)

#if defined(_MSC_VER)
#pragma pack(push, 1)
#endif
typedef struct
#if !defined(_MSC_VER)
__attribute__((packed))
#endif
{
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
    uint32_t icon_offset;
    uint32_t icon_size;
    uint32_t manifest_offset;
    uint32_t manifest_size;
    uint32_t class_offset;
    uint32_t class_size;
    uint32_t security_offset;
    uint32_t security_size;
    uint8_t  sha256[32];
    uint32_t jdk_min_version;
    uint32_t native_hint_offset;
    uint32_t native_hint_size;
    uint8_t  reserved[12];
} cmd_header_t;
#if defined(_MSC_VER)
#pragma pack(pop)
#endif

_Static_assert(sizeof(cmd_header_t) == CMD_HEADER_SIZE, "CMD header must be 96 bytes");

#endif /* CMD_FORMAT_H */
