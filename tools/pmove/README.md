<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# `pmove` — Parallel Move

`pmove` is the explicit move-mode command in the repository's parallel file-transfer tools.

## Source design

The authoritative implementation is `tools/pcopy/pcopy.c`. `tools/pmove/pmove.c` is a deliberate source-level entry point built from that implementation:

- `pmove.c` includes the established `pcopy.c` implementation.
- Its `main()` entry point is renamed during inclusion so there is no duplicate symbol.
- The resulting executable is named `pmove`.
- The shared implementation detects the `pmove` invocation name and enables move semantics.
- Move mode sets `PCOPY_F_MOVE` and `PCOPY_F_CROSS_DEVICE`.

This gives `/tools` a clear, inspectable `pmove` source without maintaining a second copy of the parallel I/O engine.

## Build

From this directory:

```bash
make
```

Install:

```bash
sudo make install
```

The resulting command is installed as:

```text
/usr/local/bin/pmove
```

## Usage

```text
pmove [options] <source...> <destination>
```

Examples:

```bash
pmove -p old/ new/
pmove -j 8 -p source/ /mnt/backup/
pmove --status
pmove -n source/ destination/
```

Supported options include:

| Option | Detail |
|---|---|
| `-j N` | Force the number of parallel channels |
| `-c SIZE` | Set chunk size in KB |
| `-s` | Sync after each file (`fsync`) |
| `-p` | Preserve permissions and timestamps |
| `-f` | Force overwrite |
| `-v` | Verbose per-file status |
| `-n` | Dry run |
| `--status` | Display hardware and channel status |
| `--help` | Display command help |

## Relationship to `pcopy`

`pcopy` and `pmove` intentionally share the same kernel-facing implementation. The existing implementation can select copy or move behavior from the executable name, while this directory makes `pmove` independently visible as source, documentation, and build target under `/tools`.

The parallel path is designed around the repository's `/dev/pcopy` interface, hardware queue information, PCIe lane awareness, CPU availability, and automatic channel/chunk selection, with the established fallback behavior retained in `pcopy.c`.

## Files

```text
tools/pmove/
├── Makefile
├── README.md
└── pmove.c
```

Copyright (C) 2026 MEARVK LLC  
License: GPL-2.0
