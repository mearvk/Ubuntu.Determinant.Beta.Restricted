# TAC3 USB Dump and Layer Backup

## Purpose

`tac3-usb-dump` is the TAC3 backup/export utility for saving a TAC3 filesystem to an already-mounted USB destination.

The utility preserves:

- the complete TAC3 image;
- the TAC3 superblock;
- the FILE table extent;
- the HEALTH table extent;
- the ADMIN table extent;
- the recovery metadata extent;
- selected logical-layer metadata;
- available live `/proc/tac3/{status,health,admin}` information.

The source is read-only. The utility does not format the USB device, resize partitions, repair TAC3, or alter the source filesystem.

## Layer selection

TAC3 redundancy is represented by its `multitude`. The backup command can select a logical recovery set:

```text
tac3-usb-dump --source /dev/DEVICE --usb /media/USB --layers 10
```

This records layers `0` through `9` as the selected ten-layer set.

A single layer can be selected:

```text
tac3-usb-dump --source /dev/DEVICE --usb /media/USB --layer 4
```

The selected layer metadata is written beneath the backup's `layers/` directory. The bundle also records the TAC3 generation, multitude, table references, and complete-image relationship.

## Why the complete image is preserved

The current TAC3 disk format defines shared FILE, HEALTH, ADMIN, and RECOVERY extents. It does **not** currently define independent physical block extents for each redundancy layer.

The utility therefore does not pretend that layer 4, for example, occupies a known contiguous range of blocks. A false physical mapping would make a backup appear recoverable when it was not.

Instead:

```text
USB backup
├── MANIFEST.txt
├── tac3-image.bin
├── tables/
│   ├── superblock.bin
│   ├── file.bin
│   ├── health.bin
│   ├── admin.bin
│   └── recovery.bin
├── metadata/
│   ├── selected-layers.txt
│   └── proc-{status,health,admin}.txt   (when available)
└── layers/
    ├── layer-0.meta
    ├── layer-1.meta
    └── ...
```

The complete image is the authoritative recovery object. Layer selection is the logical recovery/administrative selection until the on-disk format gains explicit per-layer physical extents.

## Safety contract

1. The TAC3 source must not be mounted.
2. The USB destination must already exist as a directory.
3. The utility never formats the USB device.
4. The utility validates the TAC3 magic, format version, block size, checksum, multitude, and table extents before copying.
5. Existing backup bundles are never silently replaced.
6. The source is opened read-only.
7. The complete image is copied before the backup is reported successful.
8. The authoritative table extents are copied separately for inspection and recovery tooling.
9. Live `/proc/tac3` metadata is supplementary; the on-disk tables remain authoritative.

## Installation

The utility is built with:

```sh
make -C tools/tac3
```

and installed with:

```sh
make -C tools/tac3 install
```

It is registered as `tac3-usb-dump` in the native installer manifest.

## Examples

Back up all layers:

```sh
tac3-usb-dump --source /dev/sdX2 --usb /media/usb --layers 10
```

Back up one logical layer:

```sh
tac3-usb-dump --source /dev/sdX2 --usb /media/usb --layer 3
```

If `--layers` is omitted, all layers in the TAC3 superblock's `multitude` are selected. Layer numbering is zero-based.

## Future physical-layer extraction

When the TAC3 persistent format defines authoritative per-layer extents, this utility can be extended so that `--layer N` copies only the physical blocks belonging to that layer. That extension must consume the authoritative on-disk mapping; it must not infer one from filenames, device position, or the current multitude alone.

The governing principle is:

> Preserve the whole TAC3 truth first; select logical layers explicitly; never manufacture a physical layer mapping that the filesystem format does not establish.
