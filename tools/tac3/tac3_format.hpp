#ifndef TAC3_FORMAT_HPP
#define TAC3_FORMAT_HPP

#include <cstddef>
#include <cstdint>

extern "C" {
#include "../../file-systems/tac3/tac3_format.h"
}

namespace tac3 { namespace format {
constexpr std::size_t block_size = TAC3_DISK_BLOCK_SIZE;
constexpr std::size_t magic_size = TAC3_DISK_MAGIC_SIZE;
constexpr char magic[magic_size] = {'T','A','C','3','F','S','1','\0'};
constexpr std::uint16_t major = TAC3_DISK_FORMAT_MAJOR;
constexpr std::uint16_t minor = TAC3_DISK_FORMAT_MINOR;
constexpr std::uint32_t checksum_crc32c = TAC3_DISK_CHECKSUM_CRC32C;
constexpr std::uint32_t state_clean = TAC3_DISK_STATE_CLEAN;
constexpr std::uint32_t state_active = TAC3_DISK_STATE_ACTIVE;
constexpr std::uint32_t state_recovery_required = TAC3_DISK_STATE_RECOVERY_REQUIRED;
constexpr std::uint64_t minimum_blocks = TAC3_DISK_MIN_BLOCKS;
constexpr std::uint32_t default_multitude = 10;
constexpr std::uint32_t default_device_class = 5;
}}

#endif
