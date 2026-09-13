# SecureJDK 28 & Graal

The **Top** tier of the architecture supplies managed-runtime and application
semantics through **SecureJDK 28** and **Graal**. It cooperates with the native
[Total](Total) moderator under authenticated evidence rather than direct
control.

## Integration with Total

```text
SecureJDK 28 / Graal   (managed semantics)
        │  authenticated evidence  (JVM/Graal runtime events,
        │                           trusted software descriptors,
        ▼                           signed configuration)
      Total            (native moderation)
```

SecureJDK/Graal runtime events are one class of input on Total's
[evidence surface](Total#evidence-surface): they are admitted only after
normalization, provenance, validation, and policy — the managed runtime informs
moderation, it does not bypass it.

## Related components in the repository

- [`securejdk-installer/`](https://github.com/mearvk/Ubuntu.Determinant.Beta.Restricted/tree/main/securejdk-installer) — installer (Maven `pom.xml`, `src/`, `packaging/`).
- [`graal-latest/`](https://github.com/mearvk/Ubuntu.Determinant.Beta.Restricted/tree/main/graal-latest) — Graal materials.
- [`openjdk-8-semeru/`](https://github.com/mearvk/Ubuntu.Determinant.Beta.Restricted/tree/main/openjdk-8-semeru), [`java/`](https://github.com/mearvk/Ubuntu.Determinant.Beta.Restricted/tree/main/java) — JDK/Java trees.

## SecureJDK 28 elsewhere in the project

SecureJDK 28 also appears as the managed target in the sibling **SLeeLa**
project's integration work (a `.xclass` ingest format and a Java-28-SecureJDK
memory-model link). This repository is the systems/OS side of that shared
SecureJDK vocabulary.

## Related pages

- **[Architecture](Architecture)** · **[Total (Native Moderator)](Total)**

*Authoritative source: `README.md` (§ SecureJDK 28 and Graal, § Total
integration) and the `securejdk-installer/`, `graal-latest/` trees.*
