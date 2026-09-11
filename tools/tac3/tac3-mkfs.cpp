#include "tac3_format.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <random>
#include <string>

namespace {

void usage(const char *argv0) {
    std::fprintf(stderr,
        "Usage: %s [--multitude N] [--device-class N] [--force] [--dry-run] DEVICE\n",
        argv0);
}

bool write_all(int fd, const void *buf, std::size_t len, off_t offset) {
    const auto *p = static_cast<const std::uint8_t *>(buf);
    while (len) {
        const ssize_t n = pwrite(fd, p, len, offset);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;
        p += n;
        len -= static_cast<std::size_t>(n);
        offset += n;
    }
    return true;
}

bool read_all(int fd, void *buf, std::size_t len, off_t offset) {
    auto *p = static_cast<std::uint8_t *>(buf);
    while (len) {
        const ssize_t n = pread(fd, p, len, offset);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;
        p += n;
        len -= static_cast<std::size_t>(n);
        offset += n;
    }
    return true;
}

std::uint32_t crc32c(const std::uint8_t *data, std::size_t len) {
    std::uint32_t crc = 0xffffffffU;
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0x82f63b78U & static_cast<std::uint32_t>(-(crc & 1U)));
    }
    return ~crc;
}

void put16(std::uint8_t *p, std::uint16_t v) {
    p[0] = static_cast<std::uint8_t>(v);
    p[1] = static_cast<std::uint8_t>(v >> 8);
}
void put32(std::uint8_t *p, std::uint32_t v) {
    for (unsigned i = 0; i < 4; ++i) p[i] = static_cast<std::uint8_t>(v >> (8 * i));
}
void put64(std::uint8_t *p, std::uint64_t v) {
    for (unsigned i = 0; i < 8; ++i) p[i] = static_cast<std::uint8_t>(v >> (8 * i));
}

std::uint32_t get32(const std::uint8_t *p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}

std::uint64_t blocks_for_bytes(std::uint64_t bytes) {
    return (bytes + tac3::format::block_size - 1) / tac3::format::block_size;
}

bool mounted_device(const char *device) {
    FILE *f = std::fopen("/proc/self/mountinfo", "r");
    if (!f) return false;
    char line[8192];
    bool found = false;
    while (std::fgets(line, sizeof(line), f)) {
        if (std::strstr(line, device)) { found = true; break; }
    }
    std::fclose(f);
    return found;
}

bool build_superblock(std::array<std::uint8_t, tac3::format::block_size> &out,
                      std::uint64_t total_blocks,
                      std::uint32_t multitude,
                      std::uint32_t device_class) {
    out.fill(0);
    std::memcpy(out.data(), tac3::format::magic, tac3::format::magic_size);
    std::size_t o = 8;
    put16(out.data() + o, tac3::format::major); o += 2;
    put16(out.data() + o, tac3::format::minor); o += 2;
    put32(out.data() + o, tac3::format::block_size); o += 4;
    put64(out.data() + o, total_blocks); o += 8;

    std::array<std::uint8_t, 16> uuid{};
    std::random_device rd;
    for (auto &b : uuid) b = static_cast<std::uint8_t>(rd());
    std::memcpy(out.data() + o, uuid.data(), uuid.size()); o += uuid.size();

    put64(out.data() + o, 1); o += 8;                 // generation
    put32(out.data() + o, multitude); o += 4;
    put32(out.data() + o, device_class); o += 4;

    /*
     * Initial conservative allocation. These regions are metadata contracts;
     * later kernel work will define their complete record geometry.
     */
    constexpr std::uint64_t file_blocks = 8;
    constexpr std::uint64_t health_blocks = 8;
    constexpr std::uint64_t admin_blocks = 4;
    constexpr std::uint64_t recovery_blocks = 4;
    constexpr std::uint64_t first_table = 1;

    const std::uint64_t file_start = first_table;
    const std::uint64_t health_start = file_start + file_blocks;
    const std::uint64_t admin_start = health_start + health_blocks;
    const std::uint64_t recovery_start = admin_start + admin_blocks;
    const std::uint64_t data_start = recovery_start + recovery_blocks;

    put64(out.data() + o, file_start); o += 8;
    put64(out.data() + o, file_blocks); o += 8;
    put64(out.data() + o, health_start); o += 8;
    put64(out.data() + o, health_blocks); o += 8;
    put64(out.data() + o, admin_start); o += 8;
    put64(out.data() + o, admin_blocks); o += 8;
    put64(out.data() + o, recovery_start); o += 8;
    put64(out.data() + o, recovery_blocks); o += 8;
    put64(out.data() + o, data_start); o += 8;
    put32(out.data() + o, tac3::format::state_clean); o += 4;
    put32(out.data() + o, tac3::format::checksum_crc32c); o += 4;

    /* checksum is stored in the final four bytes of the 4096-byte block */
    const std::uint32_t sum = crc32c(out.data(), out.size() - 4);
    put32(out.data() + out.size() - 4, sum);
    return true;
}

} // namespace

int main(int argc, char **argv) {
    std::uint32_t multitude = tac3::format::default_multitude;
    std::uint32_t device_class = tac3::format::default_device_class;
    bool force = false;
    bool dry_run = false;
    const char *device = nullptr;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if (arg == "--multitude" && i + 1 < argc) multitude = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        else if (arg == "--device-class" && i + 1 < argc) device_class = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        else if (arg == "--force") force = true;
        else if (arg == "--dry-run") dry_run = true;
        else if (arg == "--help") { usage(argv[0]); return 0; }
        else if (arg == "--version") { std::printf("mkfs.tac3 format %u.%u\n", tac3::format::major, tac3::format::minor); return 0; }
        else if (!device) device = argv[i];
        else { usage(argv[0]); return 2; }
    }

    if (!device) { usage(argv[0]); return 2; }
    if (multitude == 0 || multitude > 1024) {
        std::fprintf(stderr, "mkfs.tac3: invalid multitude\n"); return 2;
    }
    if (mounted_device(device)) {
        std::fprintf(stderr, "mkfs.tac3: refusing a mounted target: %s\n", device); return 2;
    }

    const int flags = dry_run ? O_RDONLY : (O_RDWR | (force ? 0 : O_EXCL));
    const int fd = open(device, flags);
    if (fd < 0) {
        std::fprintf(stderr, "mkfs.tac3: cannot open %s: %s\n", device, std::strerror(errno)); return 2;
    }

    std::uint64_t bytes = 0;
    if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
        struct stat st{};
        if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
            std::fprintf(stderr, "mkfs.tac3: cannot determine target size: %s\n", std::strerror(errno));
            close(fd); return 2;
        }
        bytes = static_cast<std::uint64_t>(st.st_size);
    }
    const std::uint64_t total_blocks = bytes / tac3::format::block_size;
    if (total_blocks < tac3::format::minimum_blocks) {
        std::fprintf(stderr, "mkfs.tac3: target is too small (minimum %llu blocks)\n",
                     static_cast<unsigned long long>(tac3::format::minimum_blocks));
        close(fd); return 2;
    }

    std::array<std::uint8_t, tac3::format::block_size> sb{};
    build_superblock(sb, total_blocks, multitude, device_class);

    std::printf("TAC3 format %u.%u\n", tac3::format::major, tac3::format::minor);
    std::printf("Device: %s\n", device);
    std::printf("Blocks: %llu\n", static_cast<unsigned long long>(total_blocks));
    std::printf("Block size: %zu\n", tac3::format::block_size);
    std::printf("Multitude: %u\n", multitude);
    std::printf("Device class: %u\n", device_class);
    if (dry_run) {
        std::printf("Dry run: no changes made.\n");
        close(fd); return 0;
    }

    if (!force) {
        std::fprintf(stderr, "mkfs.tac3: refusing to format without --force\n");
        close(fd); return 2;
    }

    if (!write_all(fd, sb.data(), sb.size(), 0)) {
        std::fprintf(stderr, "mkfs.tac3: superblock write failed: %s\n", std::strerror(errno));
        close(fd); return 1;
    }
    if (fsync(fd) != 0) {
        std::fprintf(stderr, "mkfs.tac3: fsync failed: %s\n", std::strerror(errno));
        close(fd); return 1;
    }

    std::array<std::uint8_t, tac3::format::block_size> verify{};
    if (!read_all(fd, verify.data(), verify.size(), 0) ||
        std::memcmp(verify.data(), sb.data(), sb.size()) != 0 ||
        get32(verify.data() + verify.size() - 4) != crc32c(verify.data(), verify.size() - 4)) {
        std::fprintf(stderr, "mkfs.tac3: superblock read-back verification failed\n");
        close(fd); return 1;
    }

    close(fd);
    std::printf("TAC3 superblock initialized and verified.\n");
    return 0;
}
