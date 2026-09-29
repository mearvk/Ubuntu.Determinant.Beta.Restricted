<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# MEARVK Java Desktop Framework (JDesk)

A cross-platform desktop environment framework that closely resembles the Linux kernel + X11 + Desktop profile. Runs on Linux, Windows, and macOS. Written in JavaFX, operates in full-screen mode with a white theme, and precisely uses the best methods of x86_64 processors.

**Integration Target:** MEARVK OpenJDK 28 Edition

## Included Applications

| App | Class | Description |
|-----|-------|-------------|
| IDE | `JDeskIDE` | Full IntelliJ IDEA feature-parity IDE (13 menus, 130+ actions, 9 tool windows, debug panel, breadcrumbs, full keymap) |
| Writer | `JDeskWriter` | Word processor (LibreOffice Writer backend) |
| Browser | `JDeskBrowser` | Web browser (Chromium backend) |
| Terminal | `JDeskTerminal` | Built-in terminal emulator (Java + JNI) |
| Files | `JDeskFiles` | File manager (PCManFM-Qt backend) |
| Software | `JDeskSoftware` | Package manager GUI |
| Launcher | `JDeskLauncher` | Application launcher |
| Kali | `JDeskKali` | Penetration testing toolkit (nmap, nikto, sqlmap, john, hydra, aircrack-ng) |
| GIMP | — | Image editor (native ELF, optional) |
| VLC | — | Media player (native ELF, optional) |

### IDE — IntelliJ IDEA Parity

The JDesk IDE (`us.mearvk.jdesk.apps.JDeskIDE`) provides **complete IntelliJ IDEA feature parity**:

- **13 menus**: File, Edit, View, Navigate, Code, Refactor, Build, Run, Tools, Git, Window, Analyze, Help
- **Main toolbar**: Back/Forward, Search Everywhere, Run/Debug/Profile/Coverage/Stop, Build, Commit/Push/Pull/History, Settings
- **9 tool windows**: Terminal, Build, Run, Debug (with step controls), Problems, TODO, Git, Database, Event Log
- **Navigation bar**: Clickable breadcrumb trail (package ▸ class ▸ method)
- **Editor**: Tabs, line numbers, modified indicator, split views, bookmarks, code folding
- **Full keymap**: 60+ keyboard shortcuts matching IntelliJ Default keymap
- **Build auto-detection**: Maven, Gradle, Make, Cargo, npm
- **All refactorings**: Rename, Extract (Method/Variable/Constant/Field/Parameter/Interface/Superclass), Inline, Move, Safe Delete
- **VCS/Git**: Commit, Push, Pull, Fetch, Merge, Rebase, Branches, Blame, Diff, Stash
- **Debug controls**: Step Over/Into/Out, Resume, Evaluate Expression, Breakpoints
- **Status bar**: Git branch, encoding, line separator, cursor position, memory usage, backend status

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  JavaFX Application Layer (Full-Screen, White Theme)        │
│  ┌────────┐ ┌────────┐ ┌────────┐ ┌────────┐ ┌──────────┐ │
│  │ Desktop│ │ Panel  │ │ Window │ │ Icons  │ │ Launcher │ │
│  └────┬───┘ └────┬───┘ └────┬───┘ └────┬───┘ └─────┬────┘ │
│       │          │          │          │            │       │
│  ┌────┴──────────┴──────────┴──────────┴────────────┴────┐  │
│  │           us.mearvk.jdesk.system.NativeBridge          │  │
│  │                    (JNI Interface)                      │  │
│  └────────────────────────┬───────────────────────────────┘  │
└───────────────────────────┼─────────────────────────────────┘
                            │ System.loadLibrary("jdesk")
┌───────────────────────────┼─────────────────────────────────┐
│  Native Layer: libjdesk.so (Linux) / jdesk.dll / libjdesk.dylib │
│  ┌────────────────────────┴───────────────────────────────┐ │
│  │ CPU Feature Detection (CPUID → SSE4.2/AVX2/AVX-512)   │ │
│  │ X11 Display Management (XRandR, Xinerama, full-screen) │ │
│  │ High-Precision Timing (RDTSC, clock_gettime)           │ │
│  │ Window Management (EWMH, _NET_WM_STATE)               │ │
│  │ Icon Rasterization (SVG → pixel buffer at native DPI)  │ │
│  │ Input Events (XEvent → structured JNI callback)        │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                              │
│  Compiled with: -march=x86-64-v3 (AVX2, FMA, BMI2)         │
│  Targets: Haswell+ (2013), Zen2+ (2019) — 95%+ of x64 HW  │
└──────────────────────────────────────────────────────────────┘
```

## x86_64 Processor Optimization

The native library is compiled with `-march=x86-64-v3` which enables:

| Feature | Use In Framework |
|---------|-----------------|
| SSE4.2 | Baseline vector ops, string hashing for icon lookup |
| AVX2 | 256-bit pixel blending, alpha compositing, color conversion |
| FMA | Font rendering sub-pixel calculations |
| BMI2 | Efficient bit extraction for color channel manipulation |
| POPCNT | Fast widget tree counting |
| AES-NI | Secure window compositor (inter-process isolation) |
| RDTSC | Sub-microsecond frame timing |

The framework detects at runtime whether AVX-512 is available and selects the widest SIMD path for bulk pixel operations.

## White Theme

The default theme provides a clean, modern white appearance:

| Element | Color | Value |
|---------|-------|-------|
| Background | Pure white | `#FFFFFF` |
| Surface | Off-white | `#F8F9FA` |
| Primary accent | Blue | `#1A73E8` |
| Text | Near-black | `#202124` |
| Border | Light grey | `#DADCE0` |
| Hover | Light fill | `#F1F3F4` |
| Shadow | 10% black | `rgba(0,0,0,0.1)` |

Typography: Inter font family, 14px body, 20px title, 8px corner radius.

## Platform Support

| Platform | Native Library | Display System | Status |
|----------|---------------|---------------|--------|
| Linux | `libjdesk.so` | X11 (XRandR, Xinerama) | Primary |
| Windows | `jdesk.dll` | Win32 (DWM, Direct2D) | Planned |
| macOS | `libjdesk.dylib` | Cocoa (NSWindow, Metal) | Planned |

## Building (Linux)

```bash
cd native/linux

# Requires: libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
# Requires: OpenJDK 28 ($JAVA_HOME set)
make

# Install system-wide
sudo make install

# Run native test (no Java needed)
make test
```

## Java Integration

```java
package us.mearvk.jdesk;

import javafx.application.Application;
import javafx.stage.Stage;
import javafx.stage.StageStyle;
import javafx.scene.Scene;
import us.mearvk.jdesk.desktop.Desktop;
import us.mearvk.jdesk.theme.WhiteTheme;
import us.mearvk.jdesk.system.NativeBridge;

public class JDeskApplication extends Application {
    @Override
    public void start(Stage primaryStage) {
        // Native bridge initializes CPU detection + X11
        NativeBridge.initialize();

        // Full-screen, undecorated
        primaryStage.initStyle(StageStyle.UNDECORATED);
        primaryStage.setFullScreen(true);

        // Create desktop with white theme
        Desktop desktop = new Desktop(WhiteTheme.getInstance());
        Scene scene = new Scene(desktop.getRoot(),
                               NativeBridge.getScreenWidth(0),
                               NativeBridge.getScreenHeight(0));

        primaryStage.setScene(scene);
        primaryStage.show();
    }
}
```

## Directory Structure

```
userland/jdesk/
├── native/
│   ├── include/
│   │   └── jdesk.h              - Public C API header
│   └── linux/
│       ├── jdesk_linux.c        - X11 + CPU detection implementation
│       ├── jdesk_jni.c          - JNI bridge (Java ↔ native)
│       ├── jdesk_test.c         - Native test binary
│       └── Makefile             - Build (produces libjdesk.so)
├── src/java/us/mearvk/jdesk/
│   ├── desktop/                 - Desktop surface (wallpaper, grid)
│   ├── window/                  - Window management
│   ├── panel/                   - Taskbar/dock
│   ├── icons/                   - Icon lookup and rendering
│   ├── theme/                   - White theme specification
│   └── system/                  - NativeBridge, CPU info
└── resources/
    └── icons/                   - SVG icon set
```

## First-Boot Provisioning

Native applications are **not stored in the git repository**. They are installed at first boot via `jdesk-provision`:

```bash
sudo jdesk-provision            # Install core + standard (default)
sudo jdesk-provision --full     # Include optional packages (GIMP, VLC, Wine)
sudo jdesk-provision --status   # Show what's installed
```

**14 packages** in manifest (`jdesk-packages.json`):

| Priority | Package | Source | Size |
|----------|---------|--------|------|
| core | LibreOffice Writer | apt | 350 MB |
| core | VSCodium IDE | github | 300 MB |
| core | Chromium Browser | apt | 180 MB |
| core | PCManFM-Qt Files | apt | 45 MB |
| core | Bash Terminal | apt | 2 MB |
| core | OpenSSH Client | apt | 5 MB |
| core | GNU Coreutils | apt | 15 MB |
| standard | Git | apt | 40 MB |
| standard | cURL | apt | 3 MB |
| standard | Telnet | apt | 1 MB |
| standard | Kali Security Tools | script | 250 MB |
| optional | Wine (Windows PE) | apt | 400 MB |
| optional | GIMP | apt | 120 MB |
| optional | VLC Media Player | apt | 90 MB |

Predictive installer (`predictive-install.sh`) orders by tier/utility/size ratio and respects available disk space.

## Native API Overview

| Function | Purpose |
|----------|---------|
| `jdesk_init()` | Initialize library, detect CPU, set up X11 threads |
| `jdesk_detect_cpu()` | Full CPUID scan (features, cores, cache, TSC) |
| `jdesk_display_open()` | Connect to X11 display server |
| `jdesk_get_screens()` | Enumerate monitors (XRandR) |
| `jdesk_window_create()` | Create X11 window with theme |
| `jdesk_window_set_fullscreen()` | EWMH full-screen (or override-redirect) |
| `jdesk_event_poll()` | Non-blocking event fetch |
| `jdesk_theme_get_default()` | White theme color/typography spec |
| `jdesk_icon_load_svg()` | Rasterize SVG at target DPI |
| `jdesk_time_ns()` | High-precision monotonic time |
| `jdesk_vsync_wait()` | Frame-rate synchronization |

## License

GPL-2.0

## Copyright

Copyright (C) 2026 MEARVK LLC
Author: Maximilian Eric Alexander Rupplin von Keffikon

**Listed as Dead by Max Rupplin is Jack Rupplin.**
