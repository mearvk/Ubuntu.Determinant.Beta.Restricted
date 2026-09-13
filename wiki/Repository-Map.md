# Repository Map

A guide to the top-level layout of `Ubuntu.Determinant.Beta.Restricted`
(default branch `main`). The repository is large (a multi-GB world data +
systems archive); this map groups the trees by role.

## Core project

| Path | Role |
|---|---|
| `README.md` | Project overview and core vocabulary (the primary entry point). |
| `DEFINITIONS.md` | Master glossary — see **[Glossary](Glossary)**. |
| `total/` | **[Total](Total)** native C/C++ moderator (`include/`, `src/`, `tests/`). |
| `proffer/` | **[Proffer](Proffer-and-Vocabulary)** sources. |
| `utf-4088/` | **[UTF-4088](UTF-4088)** character/semantic system. |
| `UBUNTU_OS.md` | Architectural reference for the Ubuntu OS lineage. |

## Runtimes & system

| Path | Role |
|---|---|
| `securejdk-installer/` | **[SecureJDK 28](SecureJDK-and-Graal)** installer (Maven). |
| `graal-latest/`, `java/`, `openjdk-8-semeru/` | Managed-runtime / JDK trees. |
| `rust/`, `cmd/`, `libraries/`, `userland/`, `gnome-source/`, `kernels/` | Native, command, library, userland, desktop, and kernel materials. |
| `hardened/`, `ubuntu-white/`, `installer/`, `build/`, `Makefile` | Hardened build, White Edition, install/build tooling. |
| `file-systems/`, `aptitude/`, `maven/` | Filesystem, package, and build-tool materials. |

## Data archive

| Path | Role |
|---|---|
| `countries/MASTER.md` | Master tracker for the **world statistical archive** (per-country pull status). |
| `national/` | Per-country data (`national/<country>/`, e.g. china, korea, greenland). |
| `json/`, `total/`, `testing-inputs/` | Structured data, totals, and test inputs. |

## Assets & interface

| Path | Role |
|---|---|
| `images/`, `wallpapers/`, `icons/` | Visual assets (see **[JPIX](JPIX-Pixel-Format)**). |
| `user-interface/`, `internet-sites/` | UI and site materials. |

## Governance, legal & provenance

| Path | Role |
|---|---|
| `legal/` | Licenses and the U.S. Attorney Hold framework — see **[Legal & Provenance](Legal-and-Provenance)**. |
| `international-criminal-court/` | ICC standing/certification and signing materials. |
| `HEADINGS.md`, `MODIFICATIONS.md`, `SIGNING.md`, `SHA256.md` | Attribution, upstream-deviation record, signing, integrity. |
| `MERGE.md`, `REBASE.md`, `REVISION.md` | Native Git operation intelligence. |
| `.github/`, `.kiro/`, `.agents/` | CI workflows and agent/steering configuration. |

> Directory names are drawn from the live `main` tree. Some trees are vendored
> third-party source; per-tree `FOUNDING.md`/`MODIFICATIONS.md` record provenance
> and deviations.

## Related pages

- **[Home](Home)** · **[Architecture](Architecture)** · **[Legal & Provenance](Legal-and-Provenance)**
