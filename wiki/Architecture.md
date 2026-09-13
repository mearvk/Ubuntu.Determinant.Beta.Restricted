# Architecture — Three-Tier Model

Ubuntu Determinant is organized as three logically-aware tiers that retain
**separate authority**. Each tier establishes or mediates a different kind of
fact, and evidence flows upward under provenance and policy.

```text
                 TOP
        SecureJDK 28 / Graal
        managed semantics
                │
        authenticated evidence
                ▼
              MIDDLE
               Total
        native moderation
                │
        kernel / OS evidence
                ▼
              GROUND
       Linux kernel / hardware
```

| Tier | Owner | Responsibility |
|---|---|---|
| **Ground** | Linux kernel / hardware | Establishes operating-system facts. |
| **Middle** | **Total** (native C/C++) | Mediates evidence, resource policy, provenance, and service behavior. |
| **Top** | SecureJDK 28 / Graal | Supplies managed-runtime and application semantics. |

The tiers are aware of one another but do not absorb each other's authority:
Ground states facts, Total mediates them, Top interprets them.

## Evidence pipeline

All inputs move through a fixed pipeline before they can influence behavior:

```text
input → normalization → provenance → validation → policy
      → action → observation → retained evidence
```

**The existence of an input is not proof of truth.** Provenance, validation,
authorization, and policy determine what an input may influence — the same
discipline the project applies to historical, legal, and economic data.

## Related pages

- **[Total (Native Moderator)](Total)** — the Middle tier in detail.
- **[SecureJDK 28 & Graal](SecureJDK-and-Graal)** — the Top tier.
- **[Proffer & Core Vocabulary](Proffer-and-Vocabulary)** — the reasoning model behind the tiers.

*Authoritative source: `README.md` (§ Total: Three-Tier Native Moderator) and
`UBUNTU_OS.md` (firmware → boot → kernel → userspace → desktop lineage).*
