# Linux CMD Desktop Integration

This directory defines the desktop-facing contract for `.cmd` applications.

- MIME type: `application/x-cmd`
- Executable bit is required.
- The embedded 24x24 BMP is the source icon for the CMD desktop layer.
- Launching must execute the native Linux CMD launcher; it must not fall back to a companion `.class` or `.jar` file.
- Integrity verification occurs before Java execution.

Desktop registration and icon-cache installation are intentionally kept separate from the linker so packaging systems can own installation policy.
