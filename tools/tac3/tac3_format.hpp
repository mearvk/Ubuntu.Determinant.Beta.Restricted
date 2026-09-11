#ifndef TAC3_FORMAT_HPP
#define TAC3_FORMAT_HPP

#include <cstdint>
#include <cstddef>

namespace tac3 {
namespace format {

constexpr std::size_t block_size = 4096;
constexpr std::size_t magic_size = 8;
constexpr char magic[magic_size] = {'T','A','C','3','F','S','1','\0'};
constexpr std::uint16_t major = 1;
constexpr std::uint16_t minor = 0;
constexpr std::uint32_t format_endianness_little = 1;
constexpr std::uint32_t checksum_crc32c = 1;

/*
 * Fixed-width persistent superblock contract.
 * All multi-byte fields are little-endian on disk. The in-memory C++
 * representation is intentionally not written directly to disk; mkfs.tac3
 * serializes each field explicitly so compiler padding cannot become format.
 */
struct SuperblockFields {
    std::uint16_t format_major = major;
    std::uint16_t format_minor = minor;
    std::uint32_t block_size = static_cast<std::uint32_t>(block_size);
    std::uint64_t total_blocks = 0;
    std::uint8_t uuid[16]{};
    std::uint64_t generation = 1;
    std::uint32_t multitude = 10;
    std::uint32_t device_class = 5;
    std::uint64_t file_table_start = 0;
    std::uint64_t file_table_blocks = 0;
    std::uint64_t health_table_start = 0;
    std::uint64_t health_table_blocks = 0;
    std::uint64_t admin_table_start = 0;
    std::uint64_t admin_table_blocks = 0;
    std::uint64_t recovery_start = 0;
    std::uint64_t recovery_blocks = 0;
    std::uint64_t data_start = 0;
    std::uint32_t state = 0; /* 0=clean, 1=active, 2=recovery-required */
    std::uint32_t checksum_algorithm = checksum_crc32c;
};

constexpr std::uint32_t state_clean = 0;
constexpr std::uint32_t state_active = 1;
constexpr std::uint32_t state_recovery_required = 2;

constexpr std::uint64_t minimum_blocks = 64;
constexpr std::uint32_t default_multitude = 10;
constexpr std::uint32_t default_device_class = 5; /* TAC3_DEV_NVME_GEN4 */

} // namespace format
} // namespace tac3

#endif
