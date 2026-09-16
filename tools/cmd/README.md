# cmd — Java Command Executable Format

**Version:** 1.0.0  
**Edition:** Galactic Cherry Marvell 98  
**Target:** SecureJDK 28 (.cmd format)  
**License:** GPL-2.0 WITH Classpath-exception-2.0

## Overview

`.cmd` is a native executable container for Java class/JAR payloads. The current Linux implementation uses a real ELF launcher prefix, followed by a fixed 96-byte CMD header and embedded application sections. The native launcher reads the executable itself, validates the container, verifies the embedded payload SHA-256, discovers a compatible Java runtime, extracts the payload to a private temporary directory, and only then launches Java.

The historical `tools/cmd/cmdlink.c` is intentionally retained. It is built as `cmdlink-original` for compatibility/reference purposes. The native toolchain uses `linker/cmdlink-native.c` as the authoritative linker.

## File Structure

```text
native launcher prefix
CMD header (96 bytes)
icon section
manifest section
embedded class/JAR
security section
optional native-image metadata
```

The CMD header contains magic, version, flags, section offsets/sizes, SHA-256, minimum JDK version, native-image hint information, and reserved space. Section offsets are relative to the CMD header rather than the beginning of the native prefix.

## Header

| Offset | Size | Field |
|---|---:|---|
| 0x00 | 4 | magic `0x434D4428` (`CMD(`) |
| 0x04 | 2 | format version |
| 0x06 | 2 | flags |
| 0x08 | 4 | icon offset |
| 0x0C | 4 | icon size |
| 0x10 | 4 | manifest offset |
| 0x14 | 4 | manifest size |
| 0x18 | 4 | class/JAR offset |
| 0x1C | 4 | class/JAR size |
| 0x20 | 4 | security offset |
| 0x24 | 4 | security size |
| 0x28 | 32 | SHA-256 of class/JAR payload |
| 0x48 | 4 | minimum JDK version |
| 0x4C | 4 | native-image hint offset |
| 0x50 | 4 | native-image hint size |
| 0x54 | 12 | reserved |

Total: **96 bytes**.

## Toolchain

### Native linker

```bash
make
./cmdlink MyApp.class --main=MyApp -o MyApp.cmd
./cmdlink MyApp.jar --main=com.example.Main -o App.cmd
```

The authoritative linker is `linker/cmdlink-native.c`. It uses a self-contained, standard SHA-256 implementation and embeds the resulting digest in both the header and security metadata.

### Original linker

The original `tools/cmd/cmdlink.c` remains in the repository and is compiled as:

```bash
make cmdlink-original
```

It is retained rather than deleted so the historical implementation remains available for comparison and compatibility work.

### Inspector

```bash
./cmd-inspect --verify MyApp.cmd
./cmd-inspect --manifest MyApp.cmd
./cmd-inspect --security MyApp.cmd
./cmd-inspect --icon MyApp.cmd
```

The inspector performs structural section-bound checks before reading sections and independently recomputes the embedded payload SHA-256.

## Integrity and Validation

`format/cmd-validate.h` centralizes range validation and payload-flag validation. It rejects integer-wrap conditions, out-of-file sections, malformed native-image ranges, unsupported header versions, and invalid payload flag combinations.

The Linux launcher verifies the embedded payload before execution. A modified class/JAR payload therefore fails before Java is started. The native smoke test also deliberately tampers with a generated `.cmd` and requires both execution and inspection verification to fail.

The SHA-256 implementation is tested against the standard `SHA-256("abc")` digest:

```text
ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
```

## Platform Launchers

```text
launcher/linux/cmd-launch-linux.c
launcher/windows/cmd-launch-windows.c
launcher/macos/cmd-launch-macos.c
```

Linux has the executable runtime template used by the current native smoke test. Windows and macOS now have native entry-point source and build targets; their packaging/runtime integration remains platform-specific and is not represented as complete until those native toolchains are exercised.

## Desktop Integration

Platform-specific desktop contracts are separated from linking:

```text
desktop/linux/
desktop/windows/
desktop/macos/
```

This keeps file association, icon registration, signing, quarantine, and desktop-cache policy out of the core linker. The application payload itself remains self-contained.

## Build and Test

```bash
make
make test
make verify-sha
make windows-launcher   # Windows host/toolchain
make macos-launcher     # macOS host/toolchain
make install
```

`make test` requires a working C compiler and `javac`. It builds a real ELF-prefixed `.cmd`, inspects it, executes it, modifies the payload, and confirms the modified executable is rejected.

## Relationship to SecureJDK 28

The format records a minimum JDK version of 28 and advertises SecureJDK 28 as the preferred runtime. The launcher still performs runtime discovery rather than assuming one fixed installation path.

## Source Layout

```text
tools/cmd/
├── README.md
├── Makefile
├── cmdlink.c                    # historical/original implementation
├── cmd-icon-gen.c
├── format/
│   ├── cmd-format.h
│   └── cmd-validate.h
├── linker/
│   ├── cmdlink.c                # existing native linker implementation
│   └── cmdlink-native.c         # authoritative hardened linker
├── inspect/
│   └── cmd-inspect.c
├── launcher/
│   ├── linux/
│   ├── windows/
│   └── macos/
├── desktop/
│   ├── linux/
│   ├── windows/
│   └── macos/
└── tests/                       # native/integrity regression tests
```

---

*Copyright (C) 2026 MEARVK LLC*  
*Author: Maximilian Eric Alexander Rupplin von Keffikon*
