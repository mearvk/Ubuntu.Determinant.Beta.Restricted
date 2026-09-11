# TAC3 On-Disk Format

This document defines TAC3's initial persistent on-disk contract and the first kernel consumer of that contract. TAC3 is its own filesystem; it does not require ext4, ext5, or another filesystem underneath its logical format.

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

The kernel now consumes the same field offsets through `file-systems/tac3/tac3_format.h`.

## Kernel persistent mount — Phase 1

The authoritative `CONFIG_TAC3` build target now uses `tac3_persistent.c`.

The persistent mount path is:

```text
mount -t tac3 DEVICE MOUNTPOINT
        |
        v
get_tree_bdev()
        |
        v
read TAC3 block 0
        |
        v
validate magic/version/block size/checksum
        |
        v
validate total size and table extents
        |
        v
load UUID/generation/multitude/device class/state
        |
        v
construct TAC3 runtime state
        |
        v
read-only TAC3 mount
```

This is deliberately **fail closed**. A non-TAC3 device, unsupported format version, invalid checksum, invalid extent, invalid multitude, invalid device class, or inconsistent device size is rejected rather than interpreted as TAC3.

### Why Phase 1 is read-only

The previous TAC3 implementation keeps file payloads in the page cache. That behavior is not sufficient to claim durable file-data persistence on a block device. Therefore the new persistent mount layer explicitly forces the mounted filesystem read-only and returns `-EROFS` from the file write path.

This prevents a false durability guarantee.

Phase 1 therefore establishes:

- persistent TAC3 format recognition;
- persistent superblock verification;
- persistent filesystem identity;
- persistent generation/state/configuration discovery;
- persistent table extent discovery;
- persistent recovery extent discovery; and
- a safe read-only kernel mount.

It does **not** yet claim durable FILE payload writes or complete persistent directory/inode reconstruction.

## Tables

**FILE** stores persistent file/object records and placement information.

**HEALTH** stores persistent wear, pressure, quality, error, and health information needed to reconstruct TAC3's storage state after reboot.

**ADMIN** stores administrative state and operator-supplied opaque values. It must not infer personal characteristics or judgments.

The 33-stat contextual identity model remains the identity contract. Storage of those fields does not create an additional statistic.

SUDO 1 through SUDO 8, USER, GROUP, OWNER, TRUSTED, and GENIUS remain administrative notation and are not silently converted into identity statistics.

## Recovery metadata

Recovery metadata is separately addressable and is intended to contain known-good generation information, recovery manifest identity, boot/recovery state, rollback availability, validation state, and transaction markers.

Recovery updates must be transactional. An incomplete recovery record must never be treated as confirmed known-good state.

The Phase 1 kernel mount reads the recovery extent location and persistent state from the superblock but does not yet modify recovery metadata. That write path belongs to the transactional persistence phase.

## `mkfs.tac3`

The formatter is:

```text
mkfs.tac3 [--multitude N] [--device-class N] [--force] [--dry-run] DEVICE
```

It refuses mounted targets. Formatting requires explicit `--force`; `--force` does not bypass structural validation. Dry-run performs no writes.

The formatter writes and verifies the TAC3 superblock and establishes the initial table/recovery extents. It does not fall back to an ext-family formatter.

## No ext5 dependency

There is no `ext5` requirement. A standard filesystem may remain the base OS filesystem, while TAC3 occupies its own administrator-selected partition. The two are independent filesystem roles.

## Next persistence phase

The next kernel persistence phase is the durable FILE layer:

1. define the persistent FILE record encoding;
2. define persistent directory/name records;
3. reconstruct the root and inode namespace from the FILE table;
4. map file data into the declared data region;
5. implement transactional allocation and writeback;
6. persist HEALTH state;
7. persist ADMIN state; and
8. implement transactional recovery-generation updates.

Only after those phases are implemented should TAC3 be advertised as a fully read/write persistent filesystem.

## Governing principle

> TAC3 owns its own format, tables, recovery state, and integrity contract. The standard filesystem may coexist with TAC3, but it does not define TAC3. TAC3 must never claim durable behavior that its current on-disk implementation does not actually provide.

Copyright (C) 2026 MEARVK LLC
