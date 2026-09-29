<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# X11 Visual Core & Desktop Icons

Source tarballs for the X Window System display server, core libraries, and icon themes
needed to run a graphical desktop on Ubuntu Determinant Alpha RS.

## Contents

### Display Server
| File | Version | Size | License | Purpose |
|------|---------|------|---------|---------|
| xorg-server-21.1.24.tar.xz | 21.1.24 | 4.9 MB | MIT | X11 display server (Xorg) |
| xinit-1.4.2.tar.xz | 1.4.2 | 154 KB | MIT | startx / xinitrc startup |

### Core Libraries (build order)
| File | Version | Size | License | Purpose |
|------|---------|------|---------|---------|
| xorgproto-2024.1.tar.xz | 2024.1 | 743 KB | MIT | X11 protocol headers (build first) |
| xtrans-1.5.0.tar.xz | 1.5.0 | 167 KB | MIT | X transport abstraction |
| libxcb-1.17.0.tar.xz | 1.17.0 | 445 KB | MIT | X C Bindings (required by libX11) |
| libX11-1.8.13.tar.xz | 1.8.13 | 1.8 MB | MIT | Core X11 client library |
| libXext-1.3.6.tar.xz | 1.3.6 | 334 KB | MIT | X11 protocol extensions |
| libXrender-0.9.11.tar.xz | 0.9.11 | 296 KB | MIT | X Render extension (antialiasing) |

### Desktop Programs
| File | Version | Size | License | Purpose |
|------|---------|------|---------|---------|
| twm-1.0.12.tar.xz | 1.0.12 | 257 KB | MIT | Tab Window Manager (basic WM) |
| xterm-394.tgz | 394 | 1.6 MB | MIT | X11 terminal emulator |

### Icon Themes
| File | Version | Size | License | Purpose |
|------|---------|------|---------|---------|
| humanity-icon-theme_0.6.16.tar.xz | 0.6.16 | 1.7 MB | GPL-2.0 | Ubuntu's native icon theme |
| adwaita-icon-theme-46.2.tar.xz | 46.2 | 4.4 MB | LGPL-3.0/CC-BY-SA-3.0 | GNOME/GTK default icons (fallback) |

**Total: ~17 MB compressed**

## Build Order

```bash
# 1. Protocol headers (no deps)
tar xf xorgproto-2024.1.tar.xz && cd xorgproto-2024.1
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 2. Transport layer
tar xf xtrans-1.5.0.tar.xz && cd xtrans-1.5.0
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 3. XCB (X C Bindings)
tar xf libxcb-1.17.0.tar.xz && cd libxcb-1.17.0
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 4. libX11
tar xf libX11-1.8.13.tar.xz && cd libX11-1.8.13
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 5. Extensions
tar xf libXext-1.3.6.tar.xz && cd libXext-1.3.6
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

tar xf libXrender-0.9.11.tar.xz && cd libXrender-0.9.11
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 6. X server
tar xf xorg-server-21.1.24.tar.xz && cd xorg-server-21.1.24
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

# 7. Applications
tar xf xinit-1.4.2.tar.xz && cd xinit-1.4.2
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

tar xf twm-1.0.12.tar.xz && cd twm-1.0.12
meson setup build --prefix=/usr && ninja -C build && ninja -C build install

tar xzf xterm-394.tgz && cd xterm-394
./configure --prefix=/usr && make && make install

# 8. Icons
tar xf humanity-icon-theme_0.6.16.tar.xz
cp -r humanity-icon-theme-0.6.16/Humanity /usr/share/icons/
cp -r humanity-icon-theme-0.6.16/Humanity-Dark /usr/share/icons/

tar xf adwaita-icon-theme-46.2.tar.xz && cd adwaita-icon-theme-46.2
meson setup build --prefix=/usr && ninja -C build && ninja -C build install
```

## What This Gives You

After building and installing:
- `startx` — launches X11 session
- `twm` — basic tiling/floating window manager
- `xterm` — terminal emulator for X11
- Humanity icons — Ubuntu's warm orange/dark icon set
- Adwaita icons — GTK application fallback icons

## Additional Notes

- These are source tarballs; you also need build tools: `meson`, `ninja`, `gcc`, `pkg-config`
- Additional driver packages (xf86-video-*, xf86-input-*) may be needed for specific hardware
- For a fuller desktop, consider adding: mesa (3D), cairo, pango, GTK, a display manager (lightdm)
- The X server runs as root or via rootless mode with appropriate capabilities

## Sources

- X.Org Foundation: https://www.x.org/releases/individual/
- GNOME: https://download.gnome.org/sources/adwaita-icon-theme/
- Ubuntu: http://mirror.mit.edu/ubuntu/pool/universe/h/humanity-icon-theme/
- xterm: https://invisible-island.net/xterm/
