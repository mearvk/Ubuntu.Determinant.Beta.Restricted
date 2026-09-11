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
