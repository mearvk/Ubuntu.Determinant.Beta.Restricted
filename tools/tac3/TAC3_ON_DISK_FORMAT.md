# TAC3 On-Disk Format

## Purpose

This document defines the first persistent on-disk contract for TAC3. It is deliberately conservative: the format is versioned, self-identifying, integrity-checked, and designed so that the kernel filesystem and `mkfs.tac3` can evolve together without pretending that TAC3 is an ext-family filesystem.

TAC3 is its own filesystem type. It does not require `ext4`, `ext5`, or another filesystem underneath its logical format.

## Design Principles

1. **TAC3 is TAC3.** The filesystem type and format identifier are TAC3-specific.
2. **Format before mount.** A TAC3 volume must be initialized by a TAC3 formatter before normal mounting.
3. **Version explicitly.** The on-disk format has a major and minor version.
4. **Verify before trust.** The superblock carries a checksum and structural validation fields.
5. **Preserve recovery state.** Boot/recovery metadata is distinct from ordinary file data.
6. **Do not silently reinterpret foreign media.** A non-TAC3 filesystem must be rejected by TAC3 mount/format logic.
7. **The 33-stat contextual identity model remains unchanged.** On-disk storage of those fields does not create additional identity statistics.
8. **Authority notation remains administrative metadata.** SUDO 1 through SUDO 8, USER, GROUP, OWNER, TRUSTED, and GENIUS are not silently converted into identity statistics.

## Initial Format Identity

The initial format uses these fixed values:

- Magic: `TAC3FS1\0`
- Format major: `1`
- Format minor: `0`
- Endianness: little-endian
- Block size: 4096 bytes
- Superblock location: byte offset `0`
- Superblock size: one 4096-byte block

The first implementation should reject unsupported major versions. Minor-version compatibility may be expanded later when the exact compatibility rules are defined.

## Layout

The first persistent layout is:

```text
+-------------------------------+  block 0
| TAC3 Superblock               |
+-------------------------------+  block 1
| FILE table metadata           |
+-------------------------------+
| FILE table                    |
|                               |
+-------------------------------+
| HEALTH table metadata         |
+-------------------------------+
| HEALTH table                  |
|                               |
+-------------------------------+
| ADMIN table metadata          |
+-------------------------------+
| ADMIN table                   |
|                               |
+-------------------------------+
| Recovery metadata             |
+-------------------------------+
| Data / future allocation      |
|                               |
+-------------------------------+
```

The exact allocation of table extents is stored in the superblock rather than inferred from fixed offsets. This permits future format revisions without requiring the kernel to guess where a table begins.

## Superblock Contract

The TAC3 superblock must contain, at minimum:

- magic
- format major
- format minor
- block size
- total block count
- filesystem UUID
- volume generation
- multitude
- device class
- FILE table start block
- FILE table block count
- HEALTH table start block
- HEALTH table block count
- ADMIN table start block
- ADMIN table block count
- recovery metadata start block
- recovery metadata block count
- data start block
- clean/unclean state
- checksum algorithm identifier
- superblock checksum

Fields are serialized in a fixed-width representation. Padding is reserved and must be zero on initial creation.

## Checksums

The initial formatter should use CRC32C for the superblock integrity check if the build environment and kernel implementation already provide the required primitive. The checksum field itself is treated as zero while calculating the checksum over the defined superblock bytes.

The formatter must read the superblock back after writing it and verify the checksum before reporting success.

Future authenticated or cryptographic integrity mechanisms may be added as a versioned extension. They must not change the meaning of the initial CRC32C field.

## Table Separation

### FILE

The FILE table stores persistent file/object records and their placement information. It is the persistent counterpart of TAC3's first coordinated table.

### HEALTH

The HEALTH table stores persistent storage-health accounting, including per-layer/per-region state required to reconstruct TAC3's wear, pressure, quality, and health view after a reboot.

### ADMIN

The ADMIN table stores administrative/state properties and operator-supplied opaque values. It must not be used by the filesystem to infer personal characteristics or judgments.

## Recovery Metadata

Recovery metadata is a separately addressable region. It is intended to support:

- known-good generation tracking
- recovery manifest identity
- boot/recovery state
- rollback availability
- validation state
- recovery transaction markers

Recovery metadata must be updated transactionally. A partially written recovery record must never be interpreted as a confirmed known-good state.

## Initialization State

A newly formatted TAC3 volume begins in an initialization state. `mkfs.tac3` must not claim that the volume is fully operational until:

1. the superblock is written;
2. the superblock is read back and verified;
3. table metadata is initialized;
4. recovery metadata is initialized;
5. required structural checks pass; and
6. the final initialization marker is written and verified.

The formatter must fail closed if any verification step fails.

## Multitude

The filesystem stores its configured `multitude` in the superblock. This is the persistent redundancy factor for TAC3's coordinated file representation.

The formatter must validate the requested multitude against the values supported by the kernel implementation. It must not silently substitute another value.

## Device Class

The selected TAC3 device class is recorded in the superblock so that runtime health calculations have a persistent configuration reference.

The formatter must use the established TAC3 device-class identifiers. It must not invent a second naming scheme for the same classes.

## UUID and Generation

Every newly formatted filesystem receives a filesystem UUID. The UUID identifies the filesystem instance and is not a file identity.

The volume generation identifies the current persistent state generation. Recovery transactions may advance the generation only after the new state has been verified.

## Clean / Unclean State

The filesystem must distinguish at least:

- clean
- mounted/active
- recovery-required

A shutdown or unmount path should return the volume to `clean` only after required persistent state has been committed and verified.

An interrupted transaction must leave enough information for the next mount to select the recovery path rather than assuming that the last operation completed.

## `mkfs.tac3` Contract

The formatter should eventually provide:

```text
mkfs.tac3 [options] DEVICE
```

Initial options should include only values required by the first stable implementation, such as:

- `--multitude N`
- `--device-class CLASS`
- `--force` for an explicit administrator-authorized format operation
- `--dry-run`
- `--help`
- `--version`

Formatting must require an explicit target device and must refuse mounted targets.

`--force` must never mean "format whatever was found." It means that the administrator explicitly authorized formatting of the already-validated TAC3 target.

## No ext5 Dependency

There is no `ext5` dependency in this design.

If an ordinary Linux filesystem is desired for the base OS, that filesystem remains separate. TAC3 is then allocated as its own partition and initialized as TAC3.

The installer must never use an ext-family formatter as a fallback for a missing `mkfs.tac3`.

## Compatibility Rule

The kernel TAC3 driver and `mkfs.tac3` must share the same authoritative format definitions. Duplicated magic numbers, field offsets, and structure sizes should be avoided.

The next implementation step should therefore introduce a small shared format-definition header usable by both the userspace formatter and the kernel implementation, followed by the formatter itself.

## Governing Principle

> TAC3 owns its own format, its own tables, its own recovery state, and its own integrity contract. The standard filesystem may coexist with TAC3, but it does not define TAC3.

Copyright (C) 2026 MEARVK LLC
