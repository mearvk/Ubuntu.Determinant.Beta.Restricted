# TAC3 On-Disk Format

This document defines TAC3's initial persistent on-disk contract. TAC3 is its own filesystem; it does not require ext4, ext5, or another filesystem underneath its logical format.

## Initial format

- Magic: `TAC3FS1\0`
- Major/minor: `1.0`
- Block size: 4096 bytes
- Superblock: block 0
- Endianness: little-endian
- Integrity: CRC32C

The format is explicitly versioned. Unsupported major versions must be rejected rather than guessed.

## Persistent regions

```text
block 0       TAC3 superblock
following     FILE table
following     HEALTH table
following     ADMIN table
following     recovery metadata
following     data / future allocation
```

The superblock records the actual extents, so later versions do not depend on implicit offsets.

## Superblock fields

The persistent superblock contains:

- format major/minor
- block size
- total block count
- filesystem UUID
- generation
- multitude
- device class
- FILE table extent
- HEALTH table extent
- ADMIN table extent
- recovery metadata extent
- data start
- clean/active/recovery-required state
- checksum algorithm
- checksum

The checksum is calculated with the checksum field zeroed. The formatter reads the superblock back after writing it and verifies both content and checksum.

## Tables

**FILE** stores persistent file/object records and placement information.

**HEALTH** stores persistent wear, pressure, quality, error, and health information needed to reconstruct TAC3's storage state after reboot.

**ADMIN** stores administrative state and operator-supplied opaque values. It must not infer personal characteristics or judgments.

The 33-stat contextual identity model remains the identity contract. Storage of those fields does not create an additional statistic.

SUDO 1 through SUDO 8, USER, GROUP, OWNER, TRUSTED, and GENIUS remain administrative notation and are not silently converted into identity statistics.

## Recovery metadata

Recovery metadata is separately addressable and is intended to contain known-good generation information, recovery manifest identity, boot/recovery state, rollback availability, validation state, and transaction markers.

Recovery updates must be transactional. An incomplete recovery record must never be treated as confirmed known-good state.

## `mkfs.tac3`

The formatter is being introduced as:

```text
mkfs.tac3 [--multitude N] [--device-class N] [--force] [--dry-run] DEVICE
```

It must refuse mounted targets. Formatting requires explicit `--force`; `--force` does not bypass structural validation. Dry-run performs no writes.

The first formatter writes and verifies the TAC3 superblock and establishes the initial table/recovery extents. It does not fall back to an ext-family formatter.

## No ext5 dependency

There is no `ext5` requirement. A standard filesystem may remain the base OS filesystem, while TAC3 occupies its own administrator-selected partition. The two are independent filesystem roles.

## Next compatibility requirement

The kernel TAC3 implementation and `mkfs.tac3` must ultimately consume the same authoritative format definitions. The shared `tac3_format.hpp` is the first userspace definition; kernel-side shared constants and persistent mount/read support should be added before declaring the on-disk format production-ready.

## Governing principle

> TAC3 owns its own format, tables, recovery state, and integrity contract. The standard filesystem may coexist with TAC3, but it does not define TAC3.

Copyright (C) 2026 MEARVK LLC
