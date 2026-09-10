// SPDX-License-Identifier: GPL-2.0
//
// tac3ctl.cpp — userspace TAC3 control/diagnostic tool.
//
// tac3ctl is the userspace companion to the in-kernel TAC3 filesystem
// (fs/tac3, CONFIG_TAC3). The kernel module owns the mountable filesystem and
// exposes live per-mount state at /proc/tac3/{status,health,admin}; tac3ctl is
// a small, self-contained diagnostic that:
//
//   * prints the TAC3 model summary (redundancy multitude, device-class speed
//     ceiling, the three-table layout), and
//   * runs an offline wear/pressure/health SIMULATION using the portable C++
//     engine (tac3.cpp/tac3.hpp) so an operator can reason about how access
//     patterns translate into pressure/quality/health WITHOUT touching a disk.
//
// It links the portable engine directly, so it does not require the kernel
// module to be loaded. When the module IS loaded, the authoritative live
// numbers are in /proc/tac3 — this tool complements, not replaces, that.
//
// Copyright (C) 2026 MEARVK LLC
// Author: Maximilian Eric Alexander Rupplin von Keffikon
//
#include "tac3.hpp"
#include "tac3_grid.hpp"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

using namespace tac3;

static void print_pmille(const char *label, std::uint32_t v) {
    std::printf("%-26s: %u.%u%% (%u/1000)\n", label, v / 10, v % 10, v);
}

static DeviceClass parse_class(const char *s) {
    if (!s) return DeviceClass::NvmeGen4;
    std::string x(s);
    if (x == "ide-hdd")   return DeviceClass::IdeHdd;
    if (x == "sata-hdd")  return DeviceClass::SataHdd;
    if (x == "sas-hdd")   return DeviceClass::SasHdd;
    if (x == "sata-ssd")  return DeviceClass::SataSsd;
    if (x == "nvme3")     return DeviceClass::NvmeGen3;
    if (x == "nvme4")     return DeviceClass::NvmeGen4;
    if (x == "nvme5")     return DeviceClass::NvmeGen5;
    if (x == "usb2")      return DeviceClass::Usb2;
    if (x == "usb3")      return DeviceClass::Usb3;
    if (x == "usb4")      return DeviceClass::Usb4;
    return DeviceClass::NvmeGen4;
}

static void usage(const char *argv0) {
    std::printf(
        "tac3ctl — userspace TAC3 diagnostic (companion to the fs/tac3 module)\n\n"
        "Usage: %s [command] [options]\n\n"
        "Commands:\n"
        "  info                 Print the TAC3 model summary (default).\n"
        "  simulate             Run an offline wear/pressure/health simulation.\n"
        "  grid                 Show the per-layer SQUARE linkage model and\n"
        "                       simulate ordinary neighbor wiring over a block.\n"
        "  help                 Show this help.\n\n"
        "Options (for simulate):\n"
        "  --multitude <n>      Redundancy layers (1..%u, default %u).\n"
        "  --class <c>          Device class: ide-hdd sata-hdd sas-hdd sata-ssd\n"
        "                       nvme3 nvme4 nvme5 usb2 usb3 usb4 (default nvme4).\n"
        "  --reads <n>          Simulated reads to region 0 (default 8).\n"
        "  --writes <n>         Simulated writes to region 0 (default 4).\n"
        "  --jarring <n>        Simulated jarring accesses (default 0).\n\n"
        "Options (for grid):\n"
        "  --conn <4|8>         Neighbor connectivity: 4 orthogonal or 8 with\n"
        "                       diagonals (default 8).\n"
        "  --block <n>          Wire an n x n block from the origin (default 8).\n\n"
        "When the kernel module is loaded, live per-mount numbers are in\n"
        "/proc/tac3/{status,health,admin}; this tool works offline.\n",
        argv0, kMultMax, kMultDefault);
}

static void cmd_info() {
    std::printf("TAC3 — n-tuple redundant File System (userspace view)\n");
    std::printf("=====================================================\n\n");
    std::printf("Tables:\n");
    std::printf("  Table 1 FILE    canonical n-way redundant file entries\n");
    std::printf("  Table 2 HEALTH  per-layer/per-region wear + disk health\n");
    std::printf("  Table 3 ADMIN   facts + opaque operator slots (no person data)\n\n");
    std::printf("Redundancy multitude : default %u (min %u, max %u)\n",
                kMultDefault, kMultMin, kMultMax);
    std::printf("Wear-tracked regions : %u per layer\n\n", kMaxRegions);
    std::printf("Device-class speed ceilings (MB/s):\n");
    const DeviceClass classes[] = {
        DeviceClass::IdeHdd, DeviceClass::SataHdd, DeviceClass::SasHdd,
        DeviceClass::SataSsd, DeviceClass::NvmeGen3, DeviceClass::NvmeGen4,
        DeviceClass::NvmeGen5, DeviceClass::Usb2, DeviceClass::Usb3,
        DeviceClass::Usb4,
    };
    for (DeviceClass c : classes)
        std::printf("  %-10s %u\n", to_string(c), Tac3Engine::speed_ceiling(c));
}

static void cmd_simulate(std::uint32_t mult, DeviceClass cls,
                         unsigned reads, unsigned writes, unsigned jarring) {
    Tac3Config cfg;
    cfg.multitude = mult;
    cfg.device_class = cls;
    Tac3Engine e(cfg);

    std::uint32_t ceil = Tac3Engine::speed_ceiling(cls);
    (void)e.allocate_entry();

    for (unsigned i = 0; i < reads; ++i)
        e.record_access(0, 0, Tac3Engine::AccessOp::Read, ceil, false);
    for (unsigned i = 0; i < writes; ++i)
        e.record_access(0, 0, Tac3Engine::AccessOp::Write, 0, false);
    for (unsigned i = 0; i < jarring; ++i)
        e.record_access(0, 0, Tac3Engine::AccessOp::Write, 0, true);

    const Tac3LayerHealth &L = e.layer(0);
    std::printf("TAC3 simulation\n");
    std::printf("---------------\n");
    std::printf("%-26s: %u layers\n", "multitude (redundancy)", e.multitude());
    std::printf("%-26s: %s (%u MB/s)\n", "device class", to_string(cls), ceil);
    std::printf("%-26s: reads=%u writes=%u jarring=%u\n",
                "applied to region 0", reads, writes, jarring);
    std::printf("\nLayer 0 after simulation:\n");
    std::printf("%-26s: %llu\n", "total reads",  (unsigned long long)L.total_reads);
    std::printf("%-26s: %llu\n", "total writes", (unsigned long long)L.total_writes);
    std::printf("%-26s: %llu\n", "total jarring (this layer)",
                (unsigned long long)L.total_jarring);
    print_pmille("avg read quality", L.avg_quality);
    print_pmille("avg read pressure", L.avg_pressure);
    print_pmille("layer disk health", L.disk_health);
    std::printf("%-26s: %s\n", "layer health state", to_string(L.state));
    print_pmille("file-table health (min)", e.file_table_health());
    std::printf("\nNote: a jarring access spreads extra impact across all %u layers.\n",
                e.multitude());
}

// Show the per-layer SQUARE linkage model, then wire an n x n block with the
// "ordinary" neighbor path and report how the degree distribution stays within
// the soft cap (2) with degree>2 remaining rare, plus memory/search facts.
static void cmd_grid(Connectivity conn, unsigned block) {
    std::printf("TAC3 per-layer SQUARE linkage (subject medium)\n");
    std::printf("----------------------------------------------\n");
    std::printf("%-26s: %u x %u  (= %u regions)\n", "grid geometry",
                Tac3LayerGrid::side(), Tac3LayerGrid::side(),
                Tac3LayerGrid::regions());
    std::printf("%-26s: id = (row<<%u)|col  -> O(1) bit-math, 0 stored bytes\n",
                "coordinate mapping", kGridShift);
    std::printf("%-26s: %u-connectivity (%s)\n", "neighbors",
                (unsigned)conn, conn == Connectivity::Eight
                    ? "orthogonal + diagonal" : "orthogonal only");
    std::printf("%-26s: soft %u (ordinary), hard %u (fixed inline storage)\n",
                "degree caps", kLinkSoftCap, kLinkHardCap);
    std::printf("%-26s: round(%u * log2(1 + distance))%s\n", "link cost",
                kLinkCostK, " ; +surcharge on the rare 3rd/4th link");
    std::printf("%-26s: %zu bytes/cell -> %zu KiB per layer (contiguous, no per-cell alloc)\n",
                "memory", sizeof(Tac3CellLinks),
                (sizeof(Tac3CellLinks) * (std::size_t)Tac3LayerGrid::regions()) / 1024);

    if (block < 1) block = 1;
    if (block > Tac3LayerGrid::side()) block = Tac3LayerGrid::side();

    Tac3LayerGrid g;
    for (std::uint32_t r = 0; r < block; ++r)
        for (std::uint32_t c = 0; c < block; ++c)
            g.link_to_neighbors(grid_id(r, c), conn);

    Tac3LayerGrid::Stats s = g.stats();
    std::printf("\nOrdinary wiring of a %u x %u block:\n", block, block);
    std::printf("%-26s: %u\n", "linked cells", s.linked_cells);
    std::printf("%-26s: %llu\n", "undirected links",
                (unsigned long long)s.total_links);
    std::printf("%-26s: %llu (sum of log-falloff link costs)\n", "total cost",
                (unsigned long long)s.total_cost);
    std::printf("%-26s: deg0=%u deg1=%u deg2=%u deg3=%u deg4=%u\n",
                "degree histogram",
                s.degree_hist[0], s.degree_hist[1], s.degree_hist[2],
                s.degree_hist[3], s.degree_hist[4]);
    std::printf("%-26s: %u cell(s) above the soft cap", "rare (degree>2)",
                s.rare_cells);
    if (s.linked_cells)
        std::printf("  (%.1f%% of linked cells)",
                    100.0 * (double)s.rare_cells / (double)s.linked_cells);
    std::printf("\n\nFirmness: each cell's low-cost neighbors are recovery sources\n");
    std::printf("in addition to the N-way layer redundancy, so an assailed cell\n");
    std::printf("stays recoverable without a full-layer scan.\n");
}

int main(int argc, char **argv) {
    const char *cmd = (argc > 1) ? argv[1] : "info";

    if (std::strcmp(cmd, "help") == 0 ||
        std::strcmp(cmd, "-h") == 0 || std::strcmp(cmd, "--help") == 0) {
        usage(argv[0]);
        return 0;
    }
    if (std::strcmp(cmd, "info") == 0) {
        cmd_info();
        return 0;
    }
    if (std::strcmp(cmd, "simulate") == 0) {
        std::uint32_t mult = kMultDefault;
        DeviceClass cls = DeviceClass::NvmeGen4;
        unsigned reads = 8, writes = 4, jarring = 0;
        for (int i = 2; i < argc; ++i) {
            const char *a = argv[i];
            const char *v = (i + 1 < argc) ? argv[i + 1] : nullptr;
            if (std::strcmp(a, "--multitude") == 0 && v) { mult = (std::uint32_t)std::atoi(v); ++i; }
            else if (std::strcmp(a, "--class") == 0 && v) { cls = parse_class(v); ++i; }
            else if (std::strcmp(a, "--reads") == 0 && v) { reads = (unsigned)std::atoi(v); ++i; }
            else if (std::strcmp(a, "--writes") == 0 && v) { writes = (unsigned)std::atoi(v); ++i; }
            else if (std::strcmp(a, "--jarring") == 0 && v) { jarring = (unsigned)std::atoi(v); ++i; }
            else { std::fprintf(stderr, "tac3ctl: unknown option '%s'\n", a); return 2; }
        }
        cmd_simulate(mult, cls, reads, writes, jarring);
        return 0;
    }
    if (std::strcmp(cmd, "grid") == 0) {
        Connectivity conn = Connectivity::Eight;
        unsigned block = 8;
        for (int i = 2; i < argc; ++i) {
            const char *a = argv[i];
            const char *v = (i + 1 < argc) ? argv[i + 1] : nullptr;
            if (std::strcmp(a, "--conn") == 0 && v) {
                int n = std::atoi(v);
                if (n == 4) conn = Connectivity::Four;
                else if (n == 8) conn = Connectivity::Eight;
                else { std::fprintf(stderr, "tac3ctl: --conn must be 4 or 8\n"); return 2; }
                ++i;
            }
            else if (std::strcmp(a, "--block") == 0 && v) { block = (unsigned)std::atoi(v); ++i; }
            else { std::fprintf(stderr, "tac3ctl: unknown option '%s'\n", a); return 2; }
        }
        cmd_grid(conn, block);
        return 0;
    }

    std::fprintf(stderr, "tac3ctl: unknown command '%s' (try 'help')\n", cmd);
    return 2;
}
