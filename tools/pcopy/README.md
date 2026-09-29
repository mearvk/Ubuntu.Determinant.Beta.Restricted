<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# pcopy / pmove — Parallel Copy/Move

Hardware-aware parallel file copy and move operations that exploit NVMe multi-queue architecture, PCIe lane bandwidth, and multi-core CPUs to copy many files simultaneously.

## Why

Standard `cp` and `mv` operate sequentially — one file at a time, one read/write per chunk. Modern NVMe SSDs expose **multiple hardware submission queues** (typically one per CPU core) and sit on **PCIe Gen4 x4** links providing ~7 GB/s of bandwidth. A single-threaded copy barely uses one queue and achieves ~2-3 GB/s at best.

`pcopy` dispatches multiple files across multiple channels simultaneously. Each channel runs on its own CPU, submitting I/O to its own NVMe hardware queue. The NVMe controller processes all queues in parallel. Result: near-linear throughput scaling until the PCIe link saturates.

## Theory of Operation

```
                    Standard cp (sequential)
                    ════════════════════════
File 1: [████████████████████████████]
File 2:                               [████████████████████████████]
File 3:                                                             [████████]
         └────────────────── Time ──────────────────────────────────────────┘

                    pcopy (parallel, 3 channels)
                    ════════════════════════════
File 1: [████████████████████████████]
File 2: [████████████████████████████]           (concurrent)
File 3: [████████]                               (concurrent)
         └────── Time ──────┘

Speedup ≈ min(CPUs, HW_Queues, nr_files) × single_stream_throughput
           capped by PCIe bandwidth
```

## Hardware Detection

pcopy auto-detects:

| Parameter | Source | Example |
|-----------|--------|---------|
| Online CPUs | `num_online_cpus()` | 8 |
| NVMe HW Queues | `request_queue->nr_hw_queues` | 8 |
| PCIe Generation | PCI Express Link Status register | Gen4 |
| PCIe Lanes | PCI Express Link Status register | x4 |
| Total Bandwidth | Gen × Lanes × per-lane rate | 7876 MB/s |

## Channel Formula

```
channels = min(online_cpus, nr_hw_queues, nr_files)
```

- **More channels than CPUs** → context switch overhead exceeds benefit
- **More channels than HW queues** → software queuing only, no hardware parallelism
- **More channels than files** → idle channels, waste
- **PCIe bandwidth** → hard cap regardless of queue/CPU count

## Usage

```bash
# Copy all files in parallel (auto-detect channels)
pcopy *.log /backup/logs/

# Copy directories recursively (8 channels forced)
pcopy -j 8 -p data/ images/ /mnt/backup/

# Move with preserved attributes (cross-device OK)
pmove -p /old/location/ /new/location/

# Show hardware detection
pcopy --status

# Dry run (show plan)
pcopy -n project/ /backup/
```

## Options

| Flag | Description |
|------|-------------|
| `-j N` | Force N parallel channels (default: auto) |
| `-c SIZE` | Chunk size in KB (default: auto-tune) |
| `-s` | fsync after each file |
| `-p` | Preserve permissions and timestamps |
| `-f` | Force overwrite existing files |
| `-v` | Verbose (show per-file status) |
| `-n` | Dry run |
| `--status` | Show hardware status and exit |

## Kernel Module

The kernel module (`fs/pcopy/pcopy.c`) provides:

- `/proc/pcopy/status` — Hardware detection and recommendations
- `/proc/pcopy/config` — Runtime tunable parameters
- `/dev/pcopy` — ioctl interface for batch operations

```bash
# Load module
modprobe pcopy

# Check hardware detection
cat /proc/pcopy/status

# Configure (optional)
echo "channels=16" > /proc/pcopy/config
echo "chunk=8192" > /proc/pcopy/config  # 8MB chunks
```

## Build

```bash
# Userspace tool
cd tools/pcopy && make && sudo make install

# Kernel module (as part of kernel build)
# Enable CONFIG_PCOPY=m in .config
make modules
```

## Performance Example

8-core system, NVMe Gen4 x4, copying 100 × 100MB files:

| Method | Channels | Time | Throughput |
|--------|----------|------|------------|
| `cp` (sequential) | 1 | 33s | ~300 MB/s |
| `pcopy -j 2` | 2 | 17s | ~600 MB/s |
| `pcopy -j 4` | 4 | 9s | ~1.1 GB/s |
| `pcopy -j 8` | 8 | 5s | ~2.0 GB/s |
| `pcopy` (auto) | 8 | 5s | ~2.0 GB/s |

(Actual numbers depend on file sizes, caching, and device characteristics.)

## Cross-Device Move

When `pmove` encounters a cross-device situation (source and destination on different filesystems), it:

1. Attempts `rename()` (atomic, zero-copy, instant)
2. If `EXDEV`: copies data via splice, preserves attributes, then unlinks source
3. Cross-device copy is parallel too — multiple files copied simultaneously

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│  Userspace: pcopy / pmove                                           │
│  - Collects file list (recursive directory walk)                    │
│  - Opens /dev/pcopy                                                 │
│  - Submits pcopy_batch_request via ioctl                            │
└───────────────────────────────────┬─────────────────────────────────┘
                                    │ ioctl(PCOPY_IOC_COPY)
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Kernel: pcopy module (fs/pcopy/pcopy.c)                            │
│  - Detects NVMe HW queues from block device                        │
│  - Reads PCIe Link Status (gen + lanes)                             │
│  - Computes: channels = min(cpus, hw_queues, nr_files)              │
│  - Creates bounded workqueue (max_active = channels)                │
│  - Dispatches per-file work items                                   │
└───────────────────────────────────┬─────────────────────────────────┘
                                    │ WQ_UNBOUND workers
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Per-Channel Workers (running on distinct CPUs)                     │
│  - do_splice_direct() for zero-copy page transfer                   │
│  - Each CPU's I/O → that CPU's blk-mq software queue               │
│  - blk-mq maps software queue → NVMe hardware submission queue     │
└───────────────────────────────────┬─────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│  NVMe Controller (hardware)                                         │
│  - Processes N submission queues in parallel                        │
│  - DMA reads/writes to flash independently per queue                │
│  - All queues share PCIe x4 link (bandwidth limit)                  │
└─────────────────────────────────────────────────────────────────────┘
```

## Files

```
fs/pcopy/pcopy.c                    - Kernel module (~600 lines)
fs/pcopy/Kconfig                    - CONFIG_PCOPY
fs/pcopy/Makefile                   - Build rules
include/linux/pcopy.h               - Interface header
tools/pcopy/pcopy.c                 - Userspace tool (~500 lines)
tools/pcopy/Makefile                - Build/install
```

## License

GPL-2.0

## Copyright

Copyright (C) 2026 MEARVK LLC
Author: Maximilian Eric Alexander Rupplin von Keffikon
