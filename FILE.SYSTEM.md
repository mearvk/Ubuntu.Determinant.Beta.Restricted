# FILE.SYSTEM.md — TAC3 and the Square Grid Linkage

This document describes **TAC3**, the n-tuple redundant filesystem developed for
the Ubuntu White Edition, and the **square per-layer grid linkage** — the
"subject medium" model in which files are treated as *assailable and firm* and
linked to their neighbors with a logarithmic falloff cost.

- **Authoritative filesystem source:** `mearvk/Ubuntu.Determinant.Beta.Restricted`
  — kernel module at `kernels/linux-5.15.204/.../fs/tac3/`, reference copies at
  `file-systems/tac3/`, userspace tooling at `tools/tac3/`.
- **This file** is a shared write-up kept in both that repository and the
  Sleela repository for reference.

Nothing in TAC3 computes, infers, or judges any human attribute. Table 3 stores
administrative facts and operator-supplied *opaque* values only; that ethics
boundary is preserved throughout.

---

## 1. What TAC3 is

**TAC3** (Tripartite Addressable Cache, 3-Table Edition) is an **N-way
redundant filesystem** whose reads and writes are serviced only through its own
kernel-call handles (the VFS operation tables). Every serviced I/O feeds a
wear / pressure / health engine. It maintains three coordinated tables over an
N-layer (**"multitude"**) redundant file table:

| Table | Name | Holds |
|-------|------|-------|
| **Table 1** | FILE | Canonical per-file entries, replicated across N layers (`layer_mask`, `primary_layer`, per-layer `present[]`). |
| **Table 2** | HEALTH / WEAR | Per-layer, per-region read/write/read-heat/write-wear/jarring counters, plus derived `quality`, `pressure`, `disk_health`, and a green/white/yellow state. |
| **Table 3** | ADMIN / STATE | Administrative facts (tech id, monitor health, table multitude, timestamps, permits) plus opaque operator reference slots the engine assigns **no** meaning to. |

### Key constants

| Constant | Value | Meaning |
|----------|-------|---------|
| `TAC3_MULT_MIN` | 1 | Minimum redundancy. |
| `TAC3_MULT_DEFAULT` | 10 | Default "10× of file redundancy". |
| `TAC3_MULT_MAX` | 16 | Hard ceiling on layers. |
| `TAC3_MAX_REGIONS` | 4096 | Wear-tracked regions per layer. |

### The wear / pressure / health math (identical in kernel C and the C++ port)

- `quality (‰)  = min(1000, observed_mbps * 1000 / device_speed_ceiling)`
- `pressure (‰) = +50 per doubling of a region's read-heat, capped at 1000`
- `disk_health  = 1000 − (write_wear>>6 + jarring>>2 + errors*20)`, floored at 0
- A **jarring** access spreads extra impact across **all N layers**.
- Device-class speed ceilings (MB/s): IDE-HDD 80 · SATA-HDD 150 · SAS-HDD 200 ·
  SATA-SSD 550 · NVMe-Gen3 3500 · NVMe-Gen4 7000 · NVMe-Gen5 14000 · USB2 35 ·
  USB3 400 · USB4 3000.

### The portable engine

`tac3.hpp` / `tac3.cpp` are a standalone C++ port of the pure engine (Tables
1/2/3 and the derivations), for simulation, testing, and tooling. The C kernel
module and the C++ port are kept as parallel expressions of the same model. The
mountable VFS glue lives only in the kernel module.

---

## 2. The subject medium: assailable and firm

The design goal is a *subject medium* in which files are:

- **Assailable** — a file's region may be worn, attacked, or degraded by heavy
  or jarring access; the engine records that impact honestly.
- **Firm** — a file nonetheless remains recoverable, because it is backed by
  (a) TAC3's existing **N-way layer redundancy** and (b) a new **neighbor
  linkage** in which each file's low-cost neighbors act as additional recovery
  and verification sources.

Historically a layer was a **flat, unlinked array**: a file mapped to one wear
region with no relationship to its neighbors. The square grid linkage adds that
missing relationship.

---

## 3. Why a square, not a line

For the same region count, the arrangement determines how much "linking surface
area" each file has:

| Arrangement | Interior neighbors | Path length between two files | Failure behavior |
|-------------|--------------------|-------------------------------|------------------|
| **Line** (`FILEA → FILEB → …`) | 2 (left/right) | **O(N)** = up to 4096 | A single break can partition the chain. |
| **Square grid** (√N × √N) | **4** orthogonal (8 with diagonals) | **O(√N)** = up to 64 | Multiple independent paths around any single loss. |

Laying each layer's 4096 regions on a **64 × 64 grid** (64 × 64 = 4096 exactly)
roughly doubles the available linking surface per file and cuts the worst-case
distance between files from ~4096 to ~64. That is the "maximize linking surface
area / effectiveness" the medium calls for.

---

## 4. The data structure — careful on memory, careful on search

### 4.1 The grid is *implicit* (zero stored topology)

With `side = 64`, a **region id is its coordinate**:

```text
row = id >> 6        col = id & 63        id = (row << 6) | col
```

So neighbor positions are pure shift/mask — **O(1)** bit-math — and the grid
layout itself costs **zero stored bytes**. There is no adjacency matrix and no
stored coordinate table.

### 4.2 Only explicit links are stored — fixed, inline, allocation-free

Because "more than two linkings should be rare," each cell carries a **small
fixed-capacity** link record rather than a per-cell dynamic list (which would
mean thousands of heap allocations per layer):

```c++
struct Tac3Link      { uint16 neighbor; uint16 cost; };          //  4 bytes
struct Tac3CellLinks {
    uint8    degree;              // 0 .. hard cap
    uint8    connectivity;        // 4 or 8 (how last enumerated)
    Tac3Link link[TAC3_LINK_CAP]; // contiguous inline links (CAP = 4)
};                                // 18 bytes (measured)
```

- **Inline array, not adjacency list** → a cell's links sit contiguously with
  its degree in one cache line; lookup is a linear scan of ≤ 4 entries, no
  pointer chasing, no fragmentation.
- **Fixed per-layer footprint:** `4096 × 18 B ≈ 72 KiB per layer`, allocated
  once as a single contiguous block.

### 4.3 Search-speed summary

| Operation | Cost |
|-----------|------|
| id → (row, col) and back | O(1) bit-math |
| Enumerate neighbors (4- or 8-conn) | O(1), ≤ 8 ids |
| Look up / add a link on a cell | ≤ 4 inline compares |
| Grid distance between any two files | O(√N) = ≤ 64 |

---

## 5. Connectivity — both 4 and 8

Neighbor enumeration supports both, selectable per operation:

- **4-connectivity** — orthogonal only (N / S / E / W); natural degree ≤ 4.
- **8-connectivity** — adds the four diagonals (up to 8 neighbors).

Border and corner cells simply have fewer neighbors (a corner has 2 orthogonal /
3 with diagonals), handled by bounds checks in the enumerator.

---

## 6. Logarithmic falloff cost

The cost to link cell `a` to cell `b`:

```text
cost(a, b) = round( K · log2(1 + distance(a, b)) )        K = 100
```

- `distance` is **Chebyshev** for 8-connectivity (a diagonal is one step) and
  **Manhattan/orthogonal** for 4-connectivity.
- Adjacent cells (distance 1): `cost = 100 · log2(2) = 100` — cheap.
- The cost then grows **logarithmically** with distance, so far links are
  affordable but never free — a true falloff, not a cliff.
- `K = 100` follows TAC3's existing per-mille (0..1000) fixed-point convention.

| distance | log2(1+d) | base cost (K=100) |
|---------:|----------:|------------------:|
| 1 | 1 | 100 |
| 3 | 2 | 200 |
| 7 | 3 | 300 |
| 15 | 4 | 400 |
| 63 | 6 | 600 |

---

## 7. Degree policy — "more than two should be rare"

- **Soft cap = 2 (ordinary).** The ordinary wiring path links a cell to its
  cheapest available neighbors and **stops at 2**. It never exceeds the soft cap
  on its own.
- **Hard cap = 4 (fixed storage).** A 3rd or 4th link is *permitted* but pays a
  **rarity surcharge** (`+250`) on top of the log cost and is counted, so high
  degree stays rare **by construction**. The hard cap keeps per-cell storage a
  fixed inline array.
- Degree above the soft cap arises only from **inbound** links (a cell being
  chosen by several neighbors) — legitimate, and demonstrably uncommon.

### Demonstrated distribution

Wiring an 8 × 8 block with the ordinary path (from `tac3ctl grid`):

```text
linked cells     : 72
undirected links : 71
total cost       : 15700   (sum of log-falloff link costs)
degree histogram : deg0=4024 deg1=8 deg2=58 deg3=6 deg4=0
rare (degree>2)  : 6 cells  (8.3% of linked cells)
```

Only 6 of 72 linked cells exceed degree 2, and **none** reach degree 4 — degree
> 2 is rare, and the hard cap is never breached.

---

## 8. How firmness follows

Each cell's linked neighbors are **low-cost recovery/verification sources** in
addition to the N-way layer redundancy. Because neighbors sit at different grid
positions (and replicate across the N layers), an **assailed** cell — worn,
errored, or lost — can be reconstructed from a cheap neighbor rather than a
full-layer scan. The square layout guarantees multiple short, independent paths
around any single loss, which is what makes an assailable medium *firm*.

---

## 9. Tooling — `tac3ctl`

`tac3ctl` is the userspace diagnostic (companion to the in-kernel `fs/tac3`).
It links the portable engine, so it runs with or without the kernel module
loaded. The linkage model is exposed by the `grid` command:

```sh
tac3ctl grid                       # 8-connectivity, 8x8 block (defaults)
tac3ctl grid --conn 4 --block 16   # 4-connectivity, 16x16 block
tac3ctl grid --conn 8 --block 8    # orthogonal + diagonal
```

`grid` prints the geometry, coordinate mapping, degree caps, cost model, and the
per-layer memory footprint, then wires an *n × n* block and reports the degree
histogram and how rare degree > 2 is. The pre-existing `info` and `simulate`
commands are unchanged.

### Build

```sh
tools/tac3/build.sh        # or: make -C tools/tac3
```

Builds cleanly under `-Wall -Wextra -std=c++17`. `tac3_grid.hpp` is header-only
and included by `tac3ctl.cpp`, so no build wiring changes are required.

---

## 10. Files

| File | Role |
|------|------|
| `tools/tac3/tac3.hpp` / `tac3.cpp` | Portable engine (Tables 1/2/3 + wear/pressure/health). |
| `tools/tac3/tac3_grid.hpp` | **Square grid linkage** (this document's subject). Header-only. |
| `tools/tac3/tac3ctl.cpp` | Userspace diagnostic, incl. the `grid` command. |
| `file-systems/tac3/*` | Reference copies of the kernel + portable sources, kept in sync. |
| `kernels/linux-5.15.204/.../fs/tac3/` | Authoritative, build-wired kernel module. |

> **Scope note.** The grid linkage is implemented and verified in the portable
> C++ engine (the layer that builds and runs standalone) and mirrored into the
> `file-systems/tac3/` reference copy. Adding the corresponding kernel-side
> `struct` fields and `/proc/tac3` surfacing is a separate step and would be
> validated only when built in the kernel tree.

---

*TAC3 and the square grid linkage — Copyright (C) 2026 MEARVK LLC.
Author: Maximilian Eric Alexander Rupplin von Keffikon (Max Rupplin).*
