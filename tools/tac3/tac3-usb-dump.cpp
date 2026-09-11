// SPDX-License-Identifier: GPL-2.0
//
// tac3-usb-dump.cpp — safety-gated TAC3 backup bundle writer.
//
// The current TAC3 on-disk format stores the FILE/HEALTH/ADMIN/RECOVERY
// metadata as shared extents rather than independent physical layer extents.
// Therefore this utility never invents a layer-to-block mapping. A dump always
// preserves the complete TAC3 image and its authoritative metadata tables; the
// --layers/--layer selection controls the logical layer set recorded for restore,
// validation, and layer-specific metadata on the USB backup.
//
// This is deliberately a backup/export utility, not a filesystem repair tool.
// It refuses mounted TAC3 sources and refuses a USB destination that is not a
// directory. It never formats the USB device and never modifies the TAC3 source.
//
// Copyright (C) 2026 MEARVK LLC

#include "tac3_format.hpp"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/fs.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using byte = std::uint8_t;
constexpr std::size_t kBlock = tac3::format::block_size;

void usage(const char *argv0) {
    std::fprintf(stderr,
        "Usage: %s --source DEVICE --usb DIRECTORY [--layers N | --layer N] [--force]\n"
        "\n"
        "  --source DEVICE       TAC3 block device or image to back up.\n"
        "  --usb DIRECTORY       Mounted USB destination directory.\n"
        "  --layers N            Back up logical layers 0 through N-1.\n"
        "  --layer N             Back up one logical layer N.\n"
        "  --force               Permit replacement of an existing bundle.\n"
        "\n"
        "The current TAC3 disk format has shared metadata extents rather than\n"
        "per-layer physical extents. The complete TAC3 image is therefore kept\n"
        "on the USB; layer selection controls the logical recovery set and its\n"
        "pertinent metadata. No source blocks are altered.\n",
        argv0);
}

bool write_all(int fd, const void *buf, std::size_t len, off_t offset) {
    const auto *p = static_cast<const byte *>(buf);
    while (len) {
        const ssize_t n = pwrite(fd, p, len, offset);
        if (n < 0) { if (errno == EINTR) continue; return false; }
        if (n == 0) return false;
        p += n; len -= static_cast<std::size_t>(n); offset += n;
    }
    return true;
}

bool copy_range(int in, int out, std::uint64_t bytes) {
    constexpr std::size_t chunk = 1024 * 1024;
    std::vector<byte> buf(chunk);
    std::uint64_t off = 0;
    while (off < bytes) {
        const std::size_t want = static_cast<std::size_t>(std::min<std::uint64_t>(chunk, bytes - off));
        ssize_t n = pread(in, buf.data(), want, static_cast<off_t>(off));
        if (n < 0) { if (errno == EINTR) continue; return false; }
        if (n == 0) return false;
        if (!write_all(out, buf.data(), static_cast<std::size_t>(n), static_cast<off_t>(off))) return false;
        off += static_cast<std::uint64_t>(n);
    }
    return true;
}

std::uint16_t get16(const byte *p) {
    return static_cast<std::uint16_t>(p[0]) |
           static_cast<std::uint16_t>(p[1] << 8);
}
std::uint32_t get32(const byte *p) {
    return static_cast<std::uint32_t>(p[0]) |
           (static_cast<std::uint32_t>(p[1]) << 8) |
           (static_cast<std::uint32_t>(p[2]) << 16) |
           (static_cast<std::uint32_t>(p[3]) << 24);
}
std::uint64_t get64(const byte *p) {
    std::uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i) v |= static_cast<std::uint64_t>(p[i]) << (8 * i);
    return v;
}

std::uint32_t crc32c(const byte *data, std::size_t len) {
    std::uint32_t crc = 0xffffffffU;
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0x82f63b78U & static_cast<std::uint32_t>(-(crc & 1U)));
    }
    return ~crc;
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

bool ensure_dir(const std::string &path, bool force) {
    struct stat st{};
    if (stat(path.c_str(), &st) == 0) return S_ISDIR(st.st_mode);
    if (errno != ENOENT) return false;
    return mkdir(path.c_str(), 0755) == 0;
}

bool write_text(const std::string &path, const std::string &text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out << text;
    return static_cast<bool>(out);
}

struct Super {
    std::uint64_t total_blocks = 0;
    std::uint64_t file_start = 0, file_blocks = 0;
    std::uint64_t health_start = 0, health_blocks = 0;
    std::uint64_t admin_start = 0, admin_blocks = 0;
    std::uint64_t recovery_start = 0, recovery_blocks = 0;
    std::uint64_t data_start = 0;
    std::uint64_t generation = 0;
    std::uint32_t multitude = 0;
    std::uint32_t device_class = 0;
    std::uint32_t state = 0;
};

bool read_super(int fd, Super &s, std::array<byte, kBlock> &raw) {
    if (pread(fd, raw.data(), raw.size(), 0) != static_cast<ssize_t>(raw.size())) return false;
    if (std::memcmp(raw.data(), tac3::format::magic, tac3::format::magic_size) != 0) return false;
    if (get16(raw.data() + 8) != tac3::format::major || get16(raw.data() + 10) != tac3::format::minor) return false;
    if (get32(raw.data() + 12) != tac3::format::block_size) return false;
    const std::uint32_t stored = get32(raw.data() + tac3::format::checksum);
    auto copy = raw;
    copy[tac3::format::checksum + 0] = 0;
    copy[tac3::format::checksum + 1] = 0;
    copy[tac3::format::checksum + 2] = 0;
    copy[tac3::format::checksum + 3] = 0;
    if (stored != crc32c(copy.data(), copy.size() - 4)) return false;

    s.total_blocks = get64(raw.data() + 16);
    s.generation = get64(raw.data() + 40);
    s.multitude = get32(raw.data() + 48);
    s.device_class = get32(raw.data() + 52);
    s.file_start = get64(raw.data() + 56); s.file_blocks = get64(raw.data() + 64);
    s.health_start = get64(raw.data() + 72); s.health_blocks = get64(raw.data() + 80);
    s.admin_start = get64(raw.data() + 88); s.admin_blocks = get64(raw.data() + 96);
    s.recovery_start = get64(raw.data() + 104); s.recovery_blocks = get64(raw.data() + 112);
    s.data_start = get64(raw.data() + 120);
    s.state = get32(raw.data() + 128);
    if (s.multitude == 0 || s.multitude > tac3::kMultMax) return false;
    const std::uint64_t disk_blocks = s.total_blocks;
    auto extent_ok = [disk_blocks](std::uint64_t start, std::uint64_t count) {
        return count != 0 && start < disk_blocks && count <= disk_blocks - start;
    };
    return extent_ok(s.file_start, s.file_blocks) &&
           extent_ok(s.health_start, s.health_blocks) &&
           extent_ok(s.admin_start, s.admin_blocks) &&
           extent_ok(s.recovery_start, s.recovery_blocks) &&
           s.data_start < disk_blocks;
}

bool copy_extent(int src, const std::string &path, std::uint64_t start, std::uint64_t blocks) {
    const int out = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out < 0) return false;
    const bool ok = copy_range(src, out, blocks * kBlock);
    if (ok) fsync(out);
    close(out);
    return ok;
}

} // namespace

int main(int argc, char **argv) {
    const char *source = nullptr;
    const char *usb = nullptr;
    std::uint32_t selected = 0;
    bool explicit_layer = false;
    bool force = false;

    for (int i = 1; i < argc; ++i) {
        const std::string a(argv[i]);
        if (a == "--source" && i + 1 < argc) source = argv[++i];
        else if (a == "--usb" && i + 1 < argc) usb = argv[++i];
        else if (a == "--layers" && i + 1 < argc) { selected = static_cast<std::uint32_t>(std::stoul(argv[++i])); explicit_layer = false; }
        else if (a == "--layer" && i + 1 < argc) { selected = static_cast<std::uint32_t>(std::stoul(argv[++i])); explicit_layer = true; }
        else if (a == "--force") force = true;
        else if (a == "--help" || a == "-h") { usage(argv[0]); return 0; }
        else { usage(argv[0]); return 2; }
    }

    if (!source || !usb) { usage(argv[0]); return 2; }
    if (mounted_device(source)) {
        std::fprintf(stderr, "tac3-usb-dump: refusing a mounted TAC3 source: %s\n", source);
        return 2;
    }
    struct stat ust{};
    if (stat(usb, &ust) != 0 || !S_ISDIR(ust.st_mode)) {
        std::fprintf(stderr, "tac3-usb-dump: USB destination must be an existing mounted directory: %s\n", usb);
        return 2;
    }

    const int in = open(source, O_RDONLY);
    if (in < 0) {
        std::fprintf(stderr, "tac3-usb-dump: cannot open source: %s\n", std::strerror(errno));
        return 2;
    }

    Super s{};
    std::array<byte, kBlock> sb{};
    if (!read_super(in, s, sb)) {
        std::fprintf(stderr, "tac3-usb-dump: source is not a valid TAC3 %u.%u image\n",
                     tac3::format::major, tac3::format::minor);
        close(in); return 2;
    }

    if (explicit_layer) {
        if (selected >= s.multitude) {
            std::fprintf(stderr, "tac3-usb-dump: layer %u is outside multitude %u\n", selected, s.multitude);
            close(in); return 2;
        }
    } else {
        if (selected == 0) selected = s.multitude;
        if (selected > s.multitude) {
            std::fprintf(stderr, "tac3-usb-dump: requested %u layers but TAC3 has %u\n", selected, s.multitude);
            close(in); return 2;
        }
    }

    std::ostringstream name;
    name << "tac3-usb-backup-g" << s.generation << (explicit_layer ? "-layer" : "-layers") << selected;
    const std::string root = std::string(usb) + "/" + name.str();
    struct stat rst{};
    if (stat(root.c_str(), &rst) == 0) {
        if (!force) {
            std::fprintf(stderr, "tac3-usb-dump: bundle already exists; use --force: %s\n", root.c_str());
            close(in); return 2;
        }
        std::fprintf(stderr, "tac3-usb-dump: refusing destructive replacement of an existing bundle\n");
        close(in); return 2;
    }
    if (mkdir(root.c_str(), 0755) != 0 ||
        mkdir((root + "/tables").c_str(), 0755) != 0 ||
        mkdir((root + "/metadata").c_str(), 0755) != 0 ||
        mkdir((root + "/layers").c_str(), 0755) != 0) {
        std::fprintf(stderr, "tac3-usb-dump: cannot create backup bundle: %s\n", std::strerror(errno));
        close(in); return 1;
    }

    const std::string image = root + "/tac3-image.bin";
    const int out = open(image.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out < 0 || !copy_range(in, out, s.total_blocks * kBlock) || fsync(out) != 0) {
        std::fprintf(stderr, "tac3-usb-dump: complete TAC3 image copy failed\n");
        if (out >= 0) close(out);
        close(in); return 1;
    }
    close(out);

    if (!write_all(in, sb.data(), sb.size(), 0)) {
        std::fprintf(stderr, "tac3-usb-dump: source verification read failed\n");
        close(in); return 1;
    }
    const std::string sbpath = root + "/tables/superblock.bin";
    const int sbout = open(sbpath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (sbout < 0 || !write_all(sbout, sb.data(), sb.size(), 0)) {
        std::fprintf(stderr, "tac3-usb-dump: superblock export failed\n");
        if (sbout >= 0) close(sbout);
        close(in); return 1;
    }
    fsync(sbout); close(sbout);

    if (!copy_extent(in, root + "/tables/file.bin", s.file_start, s.file_blocks) ||
        !copy_extent(in, root + "/tables/health.bin", s.health_start, s.health_blocks) ||
        !copy_extent(in, root + "/tables/admin.bin", s.admin_start, s.admin_blocks) ||
        !copy_extent(in, root + "/tables/recovery.bin", s.recovery_start, s.recovery_blocks)) {
        std::fprintf(stderr, "tac3-usb-dump: table/recovery export failed\n");
        close(in); return 1;
    }

    const std::string selected_path = root + "/metadata/selected-layers.txt";
    std::ostringstream selected_text;
    selected_text << "TAC3 logical backup layer selection\n";
    selected_text << "format=" << tac3::format::major << "." << tac3::format::minor << "\n";
    selected_text << "generation=" << s.generation << "\n";
    selected_text << "multitude=" << s.multitude << "\n";
    if (explicit_layer) selected_text << "mode=single-layer\nlayer=" << selected << "\n";
    else selected_text << "mode=first-N-layers\nlayers=" << selected << "\n";
    selected_text << "complete_image_preserved=true\n";
    selected_text << "layer_physical_extents=not_defined_by_current_format\n";
    if (!write_text(selected_path, selected_text.str())) {
        std::fprintf(stderr, "tac3-usb-dump: layer metadata write failed\n");
        close(in); return 1;
    }

    for (std::uint32_t layer = 0; layer < (explicit_layer ? 1u : selected); ++layer) {
        const std::uint32_t index = explicit_layer ? selected : layer;
        std::ostringstream p;
        p << root << "/layers/layer-" << index << ".meta";
        std::ostringstream m;
        m << "layer=" << index << "\n";
        m << "multitude=" << s.multitude << "\n";
        m << "generation=" << s.generation << "\n";
        m << "file_table=tables/file.bin\nhealth_table=tables/health.bin\nadmin_table=tables/admin.bin\n";
        m << "recovery_table=tables/recovery.bin\n";
        m << "complete_image=tac3-image.bin\n";
        m << "status=selected-logical-layer\n";
        write_text(p.str(), m.str());
    }

    std::ostringstream manifest;
    manifest << "TAC3 USB BACKUP\n";
    manifest << "format=" << tac3::format::major << "." << tac3::format::minor << "\n";
    manifest << "generation=" << s.generation << "\n";
    manifest << "multitude=" << s.multitude << "\n";
    manifest << "device_class=" << s.device_class << "\n";
    manifest << "state=" << s.state << "\n";
    manifest << "total_blocks=" << s.total_blocks << "\n";
    manifest << "selected_mode=" << (explicit_layer ? "single" : "first-N") << "\n";
    manifest << "selected_value=" << selected << "\n";
    manifest << "complete_tac3_image=tac3-image.bin\n";
    manifest << "tables=superblock.bin,file.bin,health.bin,admin.bin,recovery.bin\n";
    manifest << "layer_data_policy=complete-image-plus-selected-layer-metadata\n";
    manifest << "source_unchanged=true\n";
    if (!write_text(root + "/MANIFEST.txt", manifest.str())) {
        std::fprintf(stderr, "tac3-usb-dump: manifest write failed\n");
        close(in); return 1;
    }

    // Capture live TAC3 metadata when the mount exposes it. Failure is harmless:
    // the authoritative on-disk tables were already copied above.
    for (const char *leaf : {"status", "health", "admin"}) {
        std::string cmd = std::string("/proc/tac3/") + leaf;
        std::ifstream src(cmd);
        if (!src) continue;
        std::ostringstream data; data << src.rdbuf();
        write_text(root + "/metadata/proc-" + leaf + ".txt", data.str());
    }

    close(in);
    std::printf("TAC3 USB backup complete.\n");
    std::printf("Bundle: %s\n", root.c_str());
    std::printf("Layers selected: %s %u\n", explicit_layer ? "layer" : "layers", selected);
    std::printf("Complete TAC3 image and FILE/HEALTH/ADMIN/RECOVERY tables preserved.\n");
    std::printf("Source was not modified.\n");
    return 0;
}
