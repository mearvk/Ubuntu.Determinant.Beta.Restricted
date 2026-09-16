# Windows CMD Desktop Integration

The Windows desktop layer treats `.cmd` as a native application container.

- File association is installed by the Windows packaging layer.
- The application icon is sourced from the embedded CMD icon section.
- Execution must use the PE launcher and embedded payload.
- The runtime must verify the embedded payload before Java execution.

No companion class/JAR is required at launch time.
