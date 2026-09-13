# Ubuntu Determinant — Project Wiki

**Project:** Ubuntu.Determinant.Beta.Restricted · **Edition:** Ubuntu White Edition
**Attention:** Max Rupplin — MEARVK LLC — 2026 · **Default branch:** `main`

> *A modern secure runtime should know not only what it is doing, but where the
> state came from, what transition produced it, what capability it exercises,
> why the transition is permitted, and what should be examined next.*
> — the **Proffer** proposition

Ubuntu Determinant is an experimental systems project exploring a common
security and modeling vocabulary across **SecureJDK 28, Graal, native C/C++,
operating-system state, memory, process, geometry, time, provenance, and
hardened historical/economic data**. This wiki is the navigable map of the
project so far.

> **Reader's note.** Several project terms — *300 IQ*, *color*, *fall*, *scape*,
> *net universe* — are deliberate software-modeling **metaphors**. They are not
> measurements of human or machine intelligence, worth, or physical reality.

## Start here

- **[Architecture](Architecture)** — the three-tier model (Ground → Total → Top).
- **[Total (Native Moderator)](Total)** — the C/C++ moderation layer and evidence surface.
- **[Proffer & Core Vocabulary](Proffer-and-Vocabulary)** — Proffer, Fielter, Exact/Call Fall, 300-IQ framing.
- **[JPIX Pixel Format](JPIX-Pixel-Format)** — the `.jpix` deterministic pixel/image model.
- **[UTF-4088](UTF-4088)** — the project's character/semantic system.
- **[SecureJDK 28 & Graal](SecureJDK-and-Graal)** — managed-runtime integration with Total.
- **[Repository Map](Repository-Map)** — what lives in each top-level directory.
- **[Legal & Provenance](Legal-and-Provenance)** — headings, modifications, signing, ICC materials.
- **[Glossary](Glossary)** — master term index (`DEFINITIONS.md`).

## Project at a glance

| Aspect | Summary |
|---|---|
| Central idea | **Proffer** — evidence-aware, provenance-carrying secure runtime state |
| Native layer | **Total** — three-tier moderator between kernel and managed runtime |
| Managed layer | **SecureJDK 28 / Graal** supplying application semantics |
| Data model | `.jpix` pixel maps, **UTF-4088** semantics, hardened historical/economic data |
| Governance | Per-file **HEADINGS**, **MODIFICATIONS** record, **SIGNING**/SHA256 provenance |

## Status

This is a **beta, restricted** research repository. Contents, vocabulary, and
interfaces are evolving. Where a term or component is specified in full inside
the repository, this wiki links to that authoritative source rather than
restating it.

---
*This wiki was sketched from the repository's own documentation
(`README.md`, `DEFINITIONS.md`, `UBUNTU_OS.md`, and the `total/`, `legal/`,
`countries/`, `international-criminal-court/` trees). It summarizes; the
in-repo documents remain authoritative.*
