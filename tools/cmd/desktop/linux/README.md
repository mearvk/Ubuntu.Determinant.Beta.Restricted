# Linux CMD Desktop Integration

This directory defines the desktop-facing contract for `.cmd` applications and for installation of the CMD package's desktop assets.

- MIME type: `application/x-cmd`
- Executable bit is required for native `.cmd` launchers and desktop launchers.
- The embedded 24x24 BMP is the source icon for the CMD desktop layer.
- Launching must execute the native Linux CMD launcher; it must not fall back to a companion `.class` or `.jar` file.
- Integrity verification occurs before Java execution.
- Package installation installs the CMD icon into the system icon directory and installs a desktop entry for the CMD toolchain.
- When `DESKTOP_DIR` is available (normally `$(HOME)/Desktop`), installation also places an executable CMD desktop shortcut there.
- `DESKTOP_DIR=/path/to/Desktop` may be supplied by a package manager or administrator to select the target user's desktop explicitly.

The linker remains responsible for producing native `.cmd` applications. Desktop registration, MIME association, icon-cache updates, and per-user desktop placement are installation concerns and are therefore handled by the Makefile's `install` target rather than by the linker.
