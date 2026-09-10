# tools/tac3health — `tac3healthctl` health monitor

`tac3healthctl` is the userspace **health monitor** for the in-kernel TAC3
filesystem (`fs/tac3`, `CONFIG_TAC3=m`). It installs on the OS and watches a
live TAC3 mount for **health, errors, and alterations** by reading the module's
authoritative interface at `/proc/tac3/{status,health,admin}`.

It is the always-on companion to `tac3ctl` (`tools/tac3/`): `tac3ctl` reasons
about the model offline (info + simulation, no disk needed); `tac3healthctl`
observes the real mount.

## What it monitors

- **Health** — file-table health, per-layer `disk_health`, and each layer's
  green/white/yellow state (no red-alarm — matching `fs/tac3`).
- **Errors** — layers reporting a non-GREEN state / attention, and average
  read *pressure* crossing a configurable threshold.
- **Alterations** — drift from a saved baseline: `admin_table_revision`
  (a backward regression), `multitude`, `device_class`, `tech_id`, and the
  number of layers. An unexpected change is reported as an alteration.

## Commands

```text
tac3healthctl check                One-shot assessment (default).
tac3healthctl watch [--interval S]  Poll continuously; print on state change.
tac3healthctl baseline              Snapshot admin/topology facts for alteration
                                    detection (run once after mounting).
tac3healthctl help                  Usage.
```

Options: `--proc DIR` (default `/proc/tac3`), `--baseline FILE`
(default `/var/lib/tac3/baseline`), `--pressure P` (per-mille attention
threshold, default 800), `--interval S` (watch seconds, default 5), `--once`.

**Exit codes** (`check`): `0` GREEN · `1` WHITE · `2` YELLOW · `3` NO-MOUNT /
unreadable. Suitable for scripts, cron, and monitoring hooks.

## Build & install

```sh
tools/tac3health/build.sh                 # or: make -C tools/tac3health
tools/tac3health/install.sh               # or: make -C tools/tac3health install
make -C tools/tac3health install-service  # + systemd unit (optional)
```

Then, once a TAC3 volume is mounted:

```sh
tac3healthctl baseline                    # record the trusted topology/admin facts
systemctl enable --now tac3health.service # always-on journal monitoring
```

## Integration points

- **Kernel interface:** reads `/proc/tac3/{status,health,admin}` published by the
  module in `fs/tac3` (reference copy in `file-systems/tac3/`).
- **Binary installer:** register in `installer/install-manifest.txt` the same way
  as `tac3ctl`, e.g. `tac3healthctl|tools/tac3health|tac3healthctl|1`, so the
  `package-installer` installs it into `/user/bin` and `/deck/bin`.
- **Tools chain:** add to `tools/Makefile` (`all`/`install`/`clean`).

## Ethics

TAC3 Table 3 holds administrative facts and operator-supplied *opaque* values
only. `tac3healthctl` reports **file-system** health, errors, and alterations —
never any human attribute.
