# tac3ctl — verified build record

`tac3ctl` is the userspace diagnostic for the in-kernel TAC3 filesystem
(`fs/tac3`, `CONFIG_TAC3=m`). It is built on demand from the portable C++
engine (`tac3.cpp`/`tac3.hpp`) via `tools/tac3/build.sh` or `make -C tools/tac3`;
the compiled binary is a build artifact and is intentionally **not** checked in
(see `tools/Makefile` and `installer/install-manifest.txt`).

This file records a verified, reproducible build + run of `tac3ctl` from the
committed sources, confirming the TAC3 filesystem premise (three tables,
N-way redundancy multitude, per-layer wear/pressure/health) tools cleanly.

## Build

```sh
tools/tac3/build.sh        # or: make -C tools/tac3
```

- Compiler: `c++ (GCC) 11.5.0 20240719 (Red Hat 11.5.0-5)`
- Command: `c++ -O2 -Wall -Wextra -std=c++17 -I. -o tac3ctl tac3ctl.cpp tac3.cpp`
- Result: clean build, no warnings; binary size 28088 bytes
- `sha256(tac3ctl)` for this toolchain: `8a9b718fce59b032461b69a05981eb94c3aff68715fc22919003567ef0f20112`
  (informational — a binary is toolchain-specific and not authoritative)

## Verified run — `tac3ctl info`

```text
TAC3 — n-tuple redundant File System (userspace view)
=====================================================

Tables:
  Table 1 FILE    canonical n-way redundant file entries
  Table 2 HEALTH  per-layer/per-region wear + disk health
  Table 3 ADMIN   facts + opaque operator slots (no person data)

Redundancy multitude : default 10 (min 1, max 16)
Wear-tracked regions : 4096 per layer

Device-class speed ceilings (MB/s):
  IDE_HDD    80
  SATA_HDD   150
  SAS_HDD    200
  SATA_SSD   550
  NVME_GEN3  3500
  NVME_GEN4  7000
  NVME_GEN5  14000
  USB2       35
  USB3       400
  USB4       3000
```

## Verified run — `tac3ctl simulate --multitude 10 --class nvme5 --reads 100 --writes 40 --jarring 5`

```text
TAC3 simulation
---------------
multitude (redundancy)    : 10 layers
device class              : NVME_GEN5 (14000 MB/s)
applied to region 0       : reads=100 writes=40 jarring=5

Layer 0 after simulation:
total reads               : 100
total writes              : 45
total jarring (this layer): 5
avg read quality          : 100.0% (1000/1000)
avg read pressure         : 34.3% (343/1000)
layer disk health         : 99.9% (999/1000)
layer health state        : GREEN
file-table health (min)   : 99.9% (999/1000)

Note: a jarring access spreads extra impact across all 10 layers.
```

_Build verified 2026-09-10 by the MEARVK build tooling. Copyright (C) 2026 MEARVK LLC._
