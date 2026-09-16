#ifndef CMD_VALIDATE_H
#define CMD_VALIDATE_H

#include <stdint.h>
#include "cmd-format.h"

/* Validate a section relative to the CMD header without allowing integer wrap. */
static inline int cmd_section_valid(uint64_t file_size, uint64_t header_offset,
                                    uint32_t offset, uint32_t size)
{
    uint64_t start = header_offset + (uint64_t)offset;
    uint64_t end = start + (uint64_t)size;
    if (start < header_offset || end < start)
        return 0;
    return end <= file_size;
}

static inline int cmd_header_valid(const cmd_header_t *h, uint64_t file_size,
                                   uint64_t header_offset)
{
    if (!h || h->magic != CMD_MAGIC || h->version != CMD_VERSION)
        return 0;
    if (h->jdk_min_version == 0 || h->jdk_min_version > 0xFFFFu)
        return 0;
    if (!cmd_section_valid(file_size, header_offset, h->icon_offset, h->icon_size) ||
        !cmd_section_valid(file_size, header_offset, h->manifest_offset, h->manifest_size) ||
        !cmd_section_valid(file_size, header_offset, h->class_offset, h->class_size) ||
        !cmd_section_valid(file_size, header_offset, h->security_offset, h->security_size))
        return 0;
    if (h->native_hint_size &&
        !cmd_section_valid(file_size, header_offset,
                           h->native_hint_offset, h->native_hint_size))
        return 0;
    return 1;
}

static inline int cmd_payload_flags_valid(uint16_t flags)
{
    const unsigned int payload = flags &
        (CMD_FLAG_EMBEDDED_JAR | CMD_FLAG_EMBEDDED_CLASS | CMD_FLAG_EXTERNAL_REF);
    return payload == CMD_FLAG_EMBEDDED_JAR || payload == CMD_FLAG_EMBEDDED_CLASS ||
           payload == CMD_FLAG_EXTERNAL_REF;
}

#endif /* CMD_VALIDATE_H */
