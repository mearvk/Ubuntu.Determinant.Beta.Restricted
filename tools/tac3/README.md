# tools/tac3 — `tac3ctl` userspace diagnostic

`tac3ctl` is the userspace companion to the in-kernel **TAC3** filesystem
(`fs/tac3`, built into the ISO kernel via `CONFIG_TAC3=m`). It links the
portable C++ engine (`tac3.cpp` / `tac3.hpp`, the same port kept in
`file-systems/tac3/`) so it runs with or without the kernel module loaded.

## 33-stat contextual file model

TAC3 includes `tac3_context.hpp` / `tac3_context.cpp`, the medium-granularity
contextual identity layer defined by `TAC3_CONTEXTUAL_IDENTITY.md`.

Every contextual file record carries **33 vital statistics** covering identity,
context, provenance, filesystem/storage state, activity, pressure, wear, health,
integrity, administration, security, relationships, and operational state.

The model deliberately permits files to have the **same name**, the **same primary
ID**, and the **same context area**. Those attributes are not individually treated
as globally unique. `same_vital_state()` compares the complete 33-stat state, while
`contextual_signature()` provides a derived 64-bit diagnostic/indexing signature.
The signature is derived from the 33 statistics and is **not** a 34th statistic.

## Regular installation profile

TAC3 is an **optional, regularly installable system component**. The recommended
installation order is to install and successfully boot the standard/base operating
system first, verify that it works, then allocate an administrator-selected amount
of storage for a TAC3 partition.

See `TAC3_INSTALLATION_PROFILE.md` for installation and removal rules.

## Boot partition and recovery

TAC3 has a documented **boot-capable recovery option**. The initial architecture
retains a small firmware-compatible EFI System Partition for the firmware-facing
handoff, while TAC3 provides the protected boot and recovery environment behind it.

See `TAC3_BOOT_RECOVERY.md` for the boot/recovery contract and startup integrity policy.

## USB backup and logical layer selection

`tac3-usb-dump` creates a source-read-only backup bundle on an already-mounted USB
directory. It preserves the complete TAC3 image and the authoritative FILE,
HEALTH, ADMIN, and RECOVERY extents, plus selected logical-layer metadata.

Examples:

```sh
tac3-usb-dump --source /dev/sdX2 --usb /media/usb --layers 10
tac3-usb-dump --source /dev/sdX2 --usb /media/usb --layer 3
```

`--layers N` selects layers `0..N-1`; `--layer N` selects one logical layer. If
`--layers` is omitted, all layers in the TAC3 superblock's `multitude` are selected.

The current persistent format has shared metadata extents rather than authoritative
physical extents for individual layers. The utility therefore preserves the whole
TAC3 image and records the selected layer set instead of inventing a block mapping.
See `TAC3_USB_DUMP.md` for the backup layout and safety contract.

## Commands

```text
tac3ctl info                Print the TAC3 model summary.
tac3ctl simulate [opts]     Offline wear/pressure/health simulation.
tac3ctl grid [opts]         Show the per-layer SQUARE linkage model.
tac3ctl help                Usage.
```

When the kernel module is loaded, authoritative live per-mount numbers are at
`/proc/tac3/{status,health,admin}`; `tac3ctl` complements that with an offline
model view and simulation.

## Build & install

```sh
make -C tools/tac3
make -C tools/tac3 install
```

The build produces:

- `tac3ctl`
- `tac3-usb-dump`

Both are installed by the TAC3 tools Makefile. `tac3-usb-dump` is also registered
in `installer/install-manifest.txt`.

## Integration points

- **ISO / kernel:** the TAC3 filesystem module ships in the ISO through the kernel
  build (`CONFIG_TAC3=m`).
- **Binary installer:** `tac3ctl` and `tac3-usb-dump` are registered in the native
  installer manifest.
- **Tools chain:** the TAC3 Makefile builds and installs both userspace utilities.
- **Context layer:** `tac3_context.cpp` provides the 33-stat comparison/signature
  primitives.
- **Startup integrity:** `tac3_boot.cpp` provides the conservative boot-integrity
  state evaluator.
