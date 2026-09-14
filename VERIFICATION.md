# VERIFICATION

A point-in-time verification record for this repository: who authored it, what
it is built from, the repository identity, how complete it is, the work
invested, and its connections to other systems.

**Generated:** 2026-09-14 · **Branch of record:** `main` · **Edition:** Ubuntu White Edition

---

## Author

- **Attention / owner:** Max Rupplin — MEARVK LLC — 2026.
- **Authorship standard:** new code authored for this repository is the work of
  the owner (`HEADINGS.md`); vendored third-party trees retain their upstream
  authorship and license, with deviations recorded in `MODIFICATIONS.md`.
- **This record + the project wiki** (`wiki/`) were produced by the **Kiro** AI
  software agent working with the owner, committed to `main` under the owner's
  account.

## Source

- **Central idea:** **Proffer** — an evidence-, provenance-, and
  capability-aware secure runtime framing.
- **Own source:** the **Total** three-tier native C/C++ moderator (`total/`),
  **Proffer** sources (`proffer/`), the **UTF-4088** system (`utf-4088/`), the
  SecureJDK installer (`securejdk-installer/`), and the project documentation set
  (`README.md`, `DEFINITIONS.md`, `UBUNTU_OS.md`, governance docs).
- **Data:** a **world statistical archive** — per-country data under `national/`,
  tracked by `countries/MASTER.md`, each with a `DATA_SOURCES.md` recording
  provenance.
- **Vendored source:** managed-runtime and OS trees (`graal-latest/`, `java/`,
  `openjdk-8-semeru/`, `gnome-source/`, `kernels/`, `rust/`, etc.) are vendored
  reference material; provenance/deviations are recorded per tree.

## Repo

- **Repository:** `mearvk/Ubuntu.Determinant.Beta.Restricted` (GitHub) —
  *"NCIQ — National Professional Software Repository as Ubuntu."*
- **Default branch:** `main`. Status: **Beta, Restricted** research repository.
- **Scale:** very large (~20 GB), spanning systems source, vendored runtimes,
  visual assets, and the world data archive.
- **Config:** CI under `.github/workflows/`; agent/steering config under
  `.kiro/` and `.agents/`.

## Completeness

| Area | State |
|---|---|
| Documentation set (`README`, `DEFINITIONS`, `UBUNTU_OS`, governance) | Present and cross-referenced. |
| Total native moderator (`total/`) | Bootstrap source + tests present; foundation for future systemd/cgroup/PSI/eBPF/IPC modules. Conservative by design. |
| Proffer / UTF-4088 | Vocabulary defined; sources present under `proffer/`, `utf-4088/`. |
| SecureJDK installer | Maven project (`pom.xml`, `src/`, `packaging/`). |
| World data archive (`national/`) | Populating per `countries/MASTER.md` — mix of `✅ Pulled`, `🏗 Builder ready`, `⬜ Not started`. |
| Project wiki (`wiki/`) | Sketched: Home, Architecture, Total, Proffer, JPIX, UTF-4088, SecureJDK/Graal, Repository Map, Legal, Glossary. |

Overall: an **experimental, in-progress** systems + data project; components
range from documented-and-present to actively populating.

## Time Spent

Honest qualitative accounting (wall-clock not precisely measured) for the work
recorded here:

- Reviewing the repository structure and core documents via the GitHub API
  (the full tree is too large to clone in this environment) — small–moderate.
- Sketching the 11-page project wiki from the repository's own docs — moderate.
- Authoring this verification record — small.

Prior architecture, Total, data-archive, and installer work predates this
record and is not itemized here.

## Outward Connections

- **Submodule:** `graal-latest` → Oracle GraalVM
  (`https://github.com/oracle/graal.git`), declared in `.gitmodules`.
- **Sibling project:** `mearvk/SLeeLa` — shares the **SecureJDK 28** vocabulary;
  SLeeLa implements a Sleela↔Java 28 SecureJDK memory link, while this repo is
  the systems/OS side. Cross-referenced from both READMEs.
- **Runtimes referenced/vendored:** GraalVM, OpenJDK/Semeru, the Linux kernel and
  GNOME desktop lineage, Rust.
- **Data provenance:** the `national/` archive draws on external statistical
  sources (e.g. FAOSTAT, World Bank WDI, national statistics offices), recorded
  per country in `DATA_SOURCES.md`.

---

*This VERIFICATION.md is a descriptive project record. It does not assert any
external legal, certification, or institutional status — including any court,
governmental, university, or professional-license standing — which must be
established from the appropriate primary authority. Per project discipline
("the existence of an input is not proof of truth"), the presence of a document
in this repository is not itself proof of a claim.*
