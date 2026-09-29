<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# xmc — XML Metaclass Compiler

**Version:** 2.01  
**Edition:** Galactic Cherry Marvell 98  
**Target:** SecureJDK 28 (.xclass format)  
**License:** GPL-2.0 WITH Classpath-exception-2.0

## Versioning

XMC uses a two-part version counter: **MAJOR.MINOR**.

- **Starting version:** `2.01`
- **Minor code change:** increment the minor number (`2.01` → `2.02` → `2.03`).
- **Major architectural or compatibility change:** increment the major number and reset the minor sequence as appropriate (`2.99` → `3.01`).
- The version is defined centrally in `xmc-version.h` and is exposed by the integrated `xmc` binary with `xmc --version` or `xmc -V`.

Every future XMC source revision should update this counter according to the rule above when the revision constitutes a released code change.

## Overview

`xmc` compiles Java (`.java`) or Python (`.py`) source files into the SecureJDK 28 XML class file format (`.xclass`). The `.xclass` format carries rich metadata beyond binary `.class`: provenance, design intent, security grades, dependencies, quality assessments, and contracts.

## Unified XMC + ASYSMA output

The integrated `xmc` binary is now the developer-facing combined path:

```text
SOURCE.java
    │
    ├── xmc-core ────→ SOURCE.xclass
    │
    └── ASYSMA pack ─→ SOURCE.asysma
                         │
                         └── SOURCE.asysma.desktop
```

Run:

```bash
xmc SOURCE.java
```

The generated `.xclass`, `.asysma`, and desktop launcher are deliberately **localized beside the source file**. This makes the compiler's output location deterministic and easy to inspect or archive.

The `.asysma` file is an application/container artifact, not an OS-native executable by itself. Linux, Windows, and macOS still require their own native executable/loader when a package contains native code. The ASYSMA manifest records the declared entry mode and payloads.

## Desktop launcher

The integrated compiler composes the XMC application icon identity into ASYSMA and emits a per-source desktop launcher. The launcher invokes the ASYSMA runtime adapter rather than falsely treating the package bytes as ELF, PE/COFF, or Mach-O.

For Linux desktop installation of a source-specific application entry point, the repository also provides:

```bash
xmc-install-desktop.sh /path/to/MyClass.java
```

## What xmc Produces

For each class found in source, xmc generates an `.xclass` file containing:

1. **Identity** — structural position (name, superclass, interfaces, methods, fields, weight)
2. **Quality Democracy** — democratic structural assessment (voice, representation, participation, transparency, accountability)
3. **Quality Woman** — feminine excellence assessment (care, integrity, nurture, resolve, grace)
4. **Design** — INT tier (1-4) and wet structure (module, demange, demart, artistry)
5. **Security** — trust grade and classload grade
6. **Conduct Frame** — international law basis for the compilation
7. **Provenance** — compiler identity, installer tech signature, SHA-256 integrity

## Relationship to javac

`xmc` studies the same structural concerns as `javac` from OpenJDK 28 (`src/jdk.compiler/share/classes/com/sun/tools/javac/main/`):

- Source parsing and structural extraction (parse phase)
- Class declaration recognition (enter phase)
- Security: bounded input, no symlink following, no hostile constructs
- No code execution or side effects during compilation

Unlike `javac`, which emits bytecode (`.class`), `xmc` emits metadata (`.xclass`). The `.xclass` output feeds directly into the SecureJDK 28's `jvmINTLoader` for structural placement at runtime.

## Conduct Frames

The xmc binary adjusts quality scoring based on international law or conduct frame:

| Frame | Code | Emphasis | Legal Basis |
|-------|------|----------|-------------|
| US Standard | `us` | Voice, participation, liberty | US Constitution, Bill of Rights |
| European Union | `eu` | Transparency, accountability, data protection | EU Charter, GDPR, Treaty of Lisbon |
| International | `intl` | Balanced universal assessment | UN Charter, Universal Declaration |
| Commonwealth | `cw` | Representation, accountability, precedent | Common Law, Magna Carta, Westminster |

## Usage

```bash
xmc --version
xmc MyClass.java
xmc --frame=eu UserService.java
xmc --frame=intl --verbose App.py
xmc-build --verbose MyClass.java
xmc-install-desktop.sh MyClass.java
```

## Process rehearsal

The repository includes representative inputs under `testing-inputs/` for persistent manual testing and under `tools/xmc/tests/` for the automated rehearsal:

- `testing-inputs/java/XmcDesktopProbe.java` — simple Java application entry.
- `testing-inputs/java/XmcControlFlowProbe.java` — deterministic Java control-flow input.
- `testing-inputs/native/xmc_native_probe.c` — C native payload.
- `testing-inputs/native/xmc_native_probe.cpp` — C++ native payload.
- `tools/xmc/tests/run-xmc-process.sh` — builds native probes, runs XMC on Java inputs, verifies localized `.xclass`/`.asysma`, packages a native+Java ASYSMA, and verifies desktop launcher installation.

Run the complete rehearsal with:

```bash
make process-test
```

## Build and install

```bash
make
make test
make process-test
make install
```

## Security Concerns

| Concern | Mitigation |
|---------|-----------|
| Hostile source constructs | Parse only structure; no expression evaluation |
| Symlink attacks on input | `O_NOFOLLOW` + `lstat()` check |
| Symlink attacks on output | `O_NOFOLLOW` on output fd |
| Unbounded input | 16 MB source file cap |
| Memory exhaustion | Bounded class/method/field counts |
| Output tampering | SHA-256 signature in provenance block |
| DTD/ENTITY injection | Output contains no DTD/ENTITY/SYSTEM |
| Path traversal | Source/output paths validated |

## Files

```text
tools/xmc/xmc.c
tools/xmc/xmc-driver.c
tools/xmc/xmc-version.h
tools/xmc/asysma_pack.c
tools/xmc/xmc-build
tools/xmc/xmc-install-desktop.sh
tools/xmc/Makefile
tools/xmc/tests/java/
tools/xmc/tests/native/
tools/xmc/tests/run-xmc-process.sh
testing-inputs/java/
testing-inputs/native/
```

## Integration

Output `.xclass` files are consumed by the repository's managed-runtime integration. ASYSMA is the packaging layer for Java/XMC metadata and explicitly declared native payloads; it does not replace the host operating system's executable loader.

## Copyright

Copyright (C) 2026 MEARVK LLC  
Author: Maximilian Eric Alexander Rupplin von Keffikon  
Installer Tech: mearvk - Installer Tech 2
