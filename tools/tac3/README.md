# tools/tac3 — `tac3ctl` userspace diagnostic

`tac3ctl` is the userspace companion to the in-kernel **TAC3** filesystem
(`fs/tac3`, built into the ISO kernel via `CONFIG_TAC3=m`). It links the
portable C++ engine (`tac3.cpp` / `tac3.hpp`, the same port kept in
`file-systems/tac3/`) so it runs with or without the kernel module loaded.

## 33-stat contextual file model

TAC3 now includes `tac3_context.hpp` / `tac3_context.cpp`, the medium-granularity
contextual identity layer defined by `TAC3_CONTEXTUAL_IDENTITY.md`.

Every contextual file record carries **33 vital statistics** covering identity,
context, provenance, filesystem/storage state, activity, pressure, wear, health,
integrity, administration, security, relationships, and operational state.

The model deliberately permits files to have the **same name**, the **same primary
ID**, and the **same context area**. Those attributes are not individually treated
as globally unique. `same_vital_state()` compares the complete 33-stat state, while
`contextual_signature()` provides a derived 64-bit diagnostic/indexing signature.
The signature is derived from the 33 statistics and is **not a 34th statistic**.

If two records are identical across the complete applicable state but must still be
preserved as distinct objects, an explicit relationship/instance mechanism is needed;
TAC3 does not manufacture uniqueness merely by changing a filename.

See `TAC3_CONTEXTUAL_IDENTITY.md` for the normative reference and field-by-field list.

## Regular installation profile

TAC3 is an **optional, regularly installable system component**. The recommended
installation order is to install and successfully boot the standard/base operating
system first, verify that it works, then allocate an administrator-selected amount
of storage for a TAC3 partition.

See `TAC3_INSTALLATION_PROFILE.md` for the installation sequence, partition-sizing
rules, safety contract, validation steps, and removal rules.

The TAC3 partition size is a deployment parameter, not an identity statistic. The
installer must not silently destroy a known-good base system when creating TAC3.

## Boot partition and recovery

TAC3 also has a documented **boot-capable recovery option**. The initial architecture
retains a small firmware-compatible EFI System Partition for the firmware-facing
handoff, while TAC3 provides the protected boot and recovery environment behind it.

See `TAC3_BOOT_RECOVERY.md` for the boot/recovery contract, failure handling,
rollback requirements, and startup integrity policy.

The startup design includes a **system integrity and health scan** before normal
operation is authorized. The scan is diagnostic first and read-only-first: it checks
the boot environment, TAC3 availability, recovery manifest/state, base filesystem
discoverability and health, and rollback availability. A failed recovery attempt
must remain visibly failed and must not destroy the last known-good recovery state.

`tac3_boot.hpp` / `tac3_boot.cpp` provide the initial startup integrity state model
and conservative evaluation function. These are operational controls around the
existing 33-stat model, not additional identity statistics.

## Commands

```text
tac3ctl info                Print the TAC3 model summary (tables, multitude,
                            device-class speed ceilings). This is the default.
tac3ctl simulate [opts]     Offline wear/pressure/health simulation.
tac3ctl help                Usage.
```

`simulate` options: `--multitude <n>` (1..16), `--class <ide-hdd|sata-hdd|
sas-hdd|sata-ssd|nvme3|nvme4|nvme5|usb2|usb3|usb4>`, `--reads <n>`,
`--writes <n>`, `--jarring <n>`.

When the kernel module is loaded, authoritative live per-mount numbers are at
`/proc/tac3/{status,health,admin}`; `tac3ctl` complements that with an offline
model view and simulation.

## Build & install

```sh
tools/tac3/build.sh          # or: make -C tools/tac3
tools/tac3/install.sh        # or: make -C tools/tac3 install   (PREFIX=/usr/local/bin)
```

## Integration points

- **ISO / kernel:** the TAC3 *filesystem module* ships in the ISO through the
  kernel build (`CONFIG_TAC3=m` in `arch/x86/configs/galactic_cherry_defconfig`
  and the checked-in `.config`), so `modules_install` folds `tac3.ko` into the
  rootfs and `gen-iso.sh`'s squashfs. `tac3ctl` is not the module.
- **Binary installer:** `tac3ctl` is registered in
  `installer/install-manifest.txt` (`tac3ctl|tools/tac3|tac3ctl|1`), so the
  `package-installer` binary installs it into `/user/bin` and `/deck/bin`
  without any recompile of the installer.
- **Tools chain:** wired into `tools/Makefile` (`all`/`install`/`clean`).
- **Context layer:** `tac3_context.cpp` is compiled into `tac3ctl` by the local
  `tools/tac3/Makefile` and provides the 33-stat comparison/signature primitives.
- **Startup integrity:** `tac3_boot.cpp` is compiled into `tac3ctl` and provides
  the conservative boot-integrity state evaluator.
