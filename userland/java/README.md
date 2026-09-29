<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# OpenJDK 28 — Default Java Runtime

## Overview

OpenJDK 28 Early Access source code is included in this distribution.
The full buildable source is in `openjdk-28-src/`.

- **Version:** JDK 28 EA Build 8 (2026/07/23)
- **Architecture:** Linux x86_64
- **License:** GPL-2.0 with Classpath Exception (fully open source)
- **Source:** https://github.com/openjdk/jdk (tag: jdk-28+8)
- **Source size:** 451 MB, 25,230 files (all individually < 50 MB)
- **Trimmed:** test/ removed (saves 426 MB, not needed for build)

## Two Install Methods

### 1. Quick Install (download prebuilt binary, ~227 MB)

```bash
make fetch          # Downloads binary tarball
make install DESTDIR=build/rootfs
```

### 2. Build from Source (compile from included tree)

Requires a "boot JDK" (JDK N-1, i.e. JDK 26 or 27):

```bash
make build-from-source BOOT_JDK=/usr/lib/jvm/jdk-27
make install-from-source DESTDIR=build/rootfs
```

Build takes 30-60 minutes depending on CPU cores.

## Installation Path

```
/usr/lib/jvm/jdk-28/         ← JDK home
/usr/bin/java                 ← symlink
/usr/bin/javac                ← symlink
/etc/profile.d/java.sh        ← sets JAVA_HOME
```

## Source Tree Layout

```
openjdk-28-src/
├── src/          ← 369 MB - Java + C/C++ source code
│   ├── java.base/     - Core library (java.lang, java.util, etc.)
│   ├── java.desktop/  - AWT, Swing, 2D
│   ├── hotspot/       - JVM (C++ runtime, JIT compiler, GC)
│   └── ...
├── make/         ← 82 MB - Build system (GNU Make + autoconf)
├── bin/          ← 64 KB - Build scripts
├── doc/          ← 576 KB - Build documentation
└── SOURCE_INFO.md
```

## GitHub Compatibility

All 25,230 source files are individually under 50 MB (largest is 6.8 MB).
No git-lfs needed. No file slicing needed. Safe for standard GitHub push.
