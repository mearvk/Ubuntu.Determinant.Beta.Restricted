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

## Read-Only FILE Layer

The next layer above the base TAC3 persistent format is the **FILE layer**. Its first implementation boundary is deliberately read-only.

The FILE layer is not simply a directory containing ordinary user files. It is TAC3's persistent identity, relationship, placement, and namespace metadata layer. It tells TAC3 that an object exists, identifies that object in context, records its relationship to other objects, and ultimately identifies where its data belongs.

The architectural relationship is:

```text
TAC3 base filesystem
        |
        v
Persistent superblock
        |
        v
FILE layer
        |
        +---- file/object identity
        +---- contextual identity
        +---- parent/relationship information
        +---- name and namespace information
        +---- allocation/placement information
        |
        v
HEALTH / ADMIN / RECOVERY
        |
        v
DATA layer
        |
        v
Actual persistent file payloads
```

The FILE layer therefore comes **before** durable user payload storage. A FILE record describes the persistent object; the DATA region eventually stores the object's actual contents.

### Read-only TAC3 administrative namespace

During the read-only phase, TAC3 should expose only a small reserved administrative namespace rather than presenting incomplete durable user-file semantics.

The proposed logical namespace is:

```text
/.tac3/
    superblock
    uuid
    generation
    state
    multitude
    device-class
    format
    file-table
    health
    admin
    recovery
    manifest
```

These entries are diagnostic/administrative views of persistent TAC3 state. They are not additional copies of the authoritative on-disk records and must not become a second persistence mechanism.

The initial implementation may expose only the entries whose underlying persistent state is actually implemented. In particular, `health`, `admin`, and `recovery` must not claim durable contents until their corresponding persistent write/reconstruction paths exist.

### Meaning of the read-only entries

| Entry | Purpose | Phase 1 status |
|---|---|---|
| `/.tac3/superblock` | Human-readable view of the validated superblock | Read-only |
| `/.tac3/uuid` | Persistent TAC3 filesystem UUID | Read-only |
| `/.tac3/generation` | Current filesystem generation | Read-only |
| `/.tac3/state` | `CLEAN`, `ACTIVE`, or `RECOVERY_REQUIRED` | Read-only |
| `/.tac3/multitude` | Persistent TAC3 multitude | Read-only |
| `/.tac3/device-class` | Persistent device class | Read-only |
| `/.tac3/format` | Format version and block-size information | Read-only |
| `/.tac3/file-table` | Read-only view of persistent FILE records | Planned/limited until FILE persistence is implemented |
| `/.tac3/health` | Persistent HEALTH state | Planned |
| `/.tac3/admin` | Persistent ADMIN state | Planned |
| `/.tac3/recovery` | Recovery metadata/state | Planned/limited |
| `/.tac3/manifest` | Persistent-region and capability description | Read-only/derived |

TAC3 must never fabricate an entry merely because the name has been defined. An entry is exposed only when its underlying state can be obtained consistently from the persistent format.

### FILE records versus ordinary files

A FILE record is not itself the user's file payload. It is the persistent description of that file/object.

Conceptually:

```text
FILE record
    |
    +-- File ID
    +-- Context ID
    +-- Parent ID
    +-- Name
    +-- Version
    +-- Revision
    +-- Record type
    +-- timestamps
    +-- owner/source/location
    +-- allocation information
    +-- relationship information
    +-- operational/integrity state
    |
    v
DATA allocation
    |
    v
file payload
```

The complete contextual identity model remains the governing identity contract. The FILE layer stores that identity information; it does not create an additional identity statistic merely by persisting it.

### Contextual identity

TAC3's identity model remains contextual rather than name-only. A filename is not sufficient to establish uniqueness.

The medium-granularity identity contract consists of the established 33 vital statistics:

1. File Name
2. File ID
3. Context ID
4. Context Area
5. Parent ID
6. Version
7. Revision
8. Record Type
9. Creation Timestamp
10. Modification Timestamp
11. Owner
12. Source
13. Source ID
14. Location
15. Device Class
16. Filesystem Type
17. Mount Point
18. Capacity
19. Allocated Space
20. Available Space
21. Read Count
22. Write Count
23. Read Pressure
24. Write Pressure
25. Wear Level
26. Health Level
27. Jarring / Shock Level
28. Access Frequency
29. Integrity Status
30. Administrative Status
31. Security / Trust Status
32. Parent / Relationship Context
33. Operational State

A derived contextual signature may be used for indexing or diagnostics, but it is not a 34th statistic.

If two records are otherwise identical but must remain distinct, TAC3 must use an explicit relationship or instance mechanism rather than changing the filename merely to manufacture uniqueness.

### Administrative authority notation

The established authority notation remains separate from the 33-stat identity model:

- SUDO 1 through SUDO 8
- USER
- GROUP
- OWNER
- TRUSTED
- GENIUS

These designations are administrative metadata. They must not silently become additional identity statistics, inferred scores, or replacement identity fields.

## Tables

**FILE** stores persistent file/object records, contextual identity information, namespace relationships, and placement information.

**HEALTH** stores persistent wear, pressure, quality, error, and health information needed to reconstruct TAC3's storage state after reboot.

**ADMIN** stores administrative state and operator-supplied opaque values. It must not infer personal characteristics or judgments.

The 33-stat contextual identity model remains the identity contract. Storage of those fields does not create an additional statistic.

SUDO 1 through SUDO 8, USER, GROUP, OWNER, TRUSTED, and GENIUS remain administrative notation and are not silently converted into identity statistics.

## Recovery metadata

Recovery metadata is separately addressable and is intended to contain known-good generation information, recovery manifest identity, boot/recovery state, rollback availability, validation state, and transaction markers.

Recovery updates must be transactional. An incomplete recovery record must never be treated as confirmed known-good state.

The Phase 1 kernel mount reads the recovery extent location and persistent state from the superblock but does not yet modify recovery metadata. That write path belongs to the transactional persistence phase.

## Read-only operating contract

The read-only FILE layer has a strict safety boundary:

1. Read the persistent TAC3 superblock.
2. Validate the persistent format.
3. Validate table and recovery extents.
4. Load the persistent filesystem identity and configuration.
5. Discover available FILE-layer state.
6. Reconstruct only namespace/state that can be proven from persistent records.
7. Expose validated information as read-only.
8. Reject writes with `-EROFS` until durable writeback exists.
9. Never represent page-cache state as durable storage.
10. Never silently repair or rewrite persistent state during ordinary read-only mounting.

This makes the read-only FILE layer an inspection and reconstruction layer rather than an incomplete read/write filesystem disguised as one.

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

The next implementation phase is to make the FILE layer genuinely persistent and reconstructable:

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
