# macOS CMD Desktop Integration

The macOS desktop layer treats `.cmd` as a native application container.

- Packaging supplies the Mach-O launcher.
- The embedded icon is the application icon source.
- Launching must use the embedded payload rather than a neighboring class/JAR.
- Payload integrity is checked before Java execution.

Signing, quarantine, Launch Services registration, and application-bundle policy remain packaging responsibilities.
