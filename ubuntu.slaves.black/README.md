<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# Ubuntu Source Archive — Slaves Black

Complete Ubuntu 22.04.3 LTS source package archive, split across 4 discs for Git-compatible storage. Provides GPL source compliance for the Ubuntu Determinant Alpha RS distribution.

**Origin:** Ubuntu 22.04.3 LTS (Jammy Jellyfish) official source ISOs  
**Total source:** ~19 GB across 4 discs  
**Packages:** ~2,500 source packages (≤ 50 MB each extracted)  
**Format:** Split chunks (20–21 MB) → reassemble → ISO → extract

---

## Directory Structure

```
ubuntu.slaves.black/
├── 1/                          — Disc 1 (221 chunks, ~4.5 GB)
│   ├── ubuntu_1_aa ... _im    — Split chunks
│   └── packages/              — Extracted source packages (683 packages)
├── 2/                          — Disc 2 (221 chunks, ~4.6 GB)
│   ├── ubuntu_2_aa ... _im    — Split chunks
│   └── packages/              — Extracted source packages (1496 packages)
├── 3/                          — Disc 3 (62 chunks, ~1.3 GB)
│   ├── ubuntu_3_aa ... _cj    — Split chunks
│   └── packages/              — Extracted source packages (162 packages)
├── 4/                          — Disc 4 (71 chunks, ~1.4 GB)
│   ├── ubuntu_4_aa ... _cs    — Split chunks
│   └── packages/              — Extracted source packages (221 packages)
├── 5/                          — Reserved (placeholder)
│   └── quit.git               — Marker file
├── jars/                       — Java connector packages
│   ├── mysql-connector-j-9.7.0.tar.gz
│   └── mysql-connector-j_9.7.0-1ubuntu25.10_all.deb
├── manifest.txt                — Full package inventory (disc/package/size/status)
├── skipped.txt                 — Packages > 50 MB (not extracted)
├── reassemble-source-all.sh    — Rebuild all ISOs from chunks
├── reassemble-source-iso-1.sh  — Rebuild disc 1 ISO
├── reassemble-source-iso-2.sh  — Rebuild disc 2 ISO
├── reassemble-source-iso-3.sh  — Rebuild disc 3 ISO
├── reassemble-source-iso-4.sh  — Rebuild disc 4 ISO
├── extract-source-packages.sh  — Extract packages from reassembled ISOs
├── extract-small-packages.sh   — Extract packages ≤ 50 MB only
├── check-compiled-artifacts.sh — Audit compiled/oversized files
├── delete-full-isos.sh         — Remove reassembled ISOs (keeps chunks)
├── sparse-checkout.sh          — Selective fetch by module + grade
└── README.md                   — This file
```

---

## Disc Contents

| Disc | Chunks | Size | Packages | Primary Contents |
|------|--------|------|----------|------------------|
| 1 | 221 | ~4.5 GB | 683 | Core system: glibc, gcc, binutils, bash, dpkg, apt, systemd, grub2, python, ruby, fonts, gtk, gnome, kernel tools |
| 2 | 221 | ~4.6 GB | 1496 | Libraries & languages: boost, cairo, icu, mesa, qt, kde, java, perl modules, language packs, eclipse, maven |
| 3 | 62 | ~1.3 GB | 162 | Runtime & desktop: perl, python, pipewire, qt5, protobuf, openexr, openjfx, openmpi |
| 4 | 71 | ~1.4 GB | 221 | System services: openssl, openssh, neutron, nova, horizon, linux-meta, nvidia-settings, lintian, X11 libs |
| 5 | — | — | — | Reserved |

---

## Workflow

### Full Rebuild (chunks → ISO → packages)

```bash
# 1. Reassemble ISOs from split chunks
./reassemble-source-all.sh

# 2. Extract individual source packages from the ISOs
./extract-source-packages.sh --all ./output/

# 3. Or extract a specific package
./extract-source-packages.sh --package linux ./kernel-src/
```

### Sparse Checkout (fetch only what you need)

```bash
# Fetch disc 1, essential packages only (~1.5 GB)
./sparse-checkout.sh 1 1

# Fetch disc 2, standard set (~3 GB)
./sparse-checkout.sh 2 2

# Fetch disc 4, everything (~1.4 GB)
./sparse-checkout.sh 4 3
```

**Grade levels:**

| Grade | Name | Fetch | Criteria |
|-------|------|-------|----------|
| 1 | Essential | ~33% | Core system, stabil accue, high structure/importance |
| 2 | Standard | ~66% | Normal relevance, moderate size, standard accue |
| 3 | Complete | 100% | Full disc including unstabil, large, supplementary |

Grade selection evaluates: **size**, **importance**, **relevance**, **structure**, **accue** (stabil/unstabil), and **normal**.

### Maintenance

```bash
# Check for oversized compiled artifacts
./check-compiled-artifacts.sh

# Remove reassembled ISOs (chunks remain for rebuild)
./delete-full-isos.sh

# Or with no prompt:
./delete-full-isos.sh -y
```

---

## Skipped Packages (> 50 MB)

These packages exceed the 50 MB extraction threshold and are available only from the full ISOs:

| Package | Size | Disc |
|---------|------|------|
| fonts-noto | 862 MB | 1 |
| libreoffice | 881 MB | 4 |
| nvidia-graphics-drivers-525 | 656 MB | 3 |
| nvidia-graphics-drivers-535 | 592 MB | 3 |
| nvidia-graphics-drivers-510 | 516 MB | 2 |
| mysql-8.0 | 418 MB | 2 |
| budgie-wallpapers | 285 MB | 1 |
| nvidia-graphics-drivers-470 | 261 MB | 2 |
| fonts-noto-cjk | 222 MB | 1 |
| linux (kernel) | 195 MB | 4 |
| linux-lowlatency | 197 MB | 4 |
| efl | 161 MB | 1 |
| fonts-noto-color-emoji | 136 MB | 1 |
| fluid-soundfont | 129 MB | 1 |
| llvm-toolchain-15 | 132 MB | 1 |
| llvm-toolchain-14 | 124 MB | 1 |
| mono | 112 MB | 1 |

Full list in `skipped.txt`.

---

## Storage Considerations

| State | Size | Notes |
|-------|------|-------|
| Chunks only | ~11.8 GB | Git-trackable (each chunk ≤ 21 MB) |
| Chunks + ISOs | ~19 GB | Redundant (ISOs rebuilt from chunks) |
| Chunks + ISOs + packages | ~30+ GB | Full working set |
| Sparse grade 1 (1 disc) | ~1.5 GB | Minimal checkout |

**Recommendation:** Keep chunks in git, delete ISOs after extraction. Use `sparse-checkout.sh` for partial clones.

---

## Online Source

The source packages originate from the Ubuntu archive and can also be fetched individually:

- **Package pool:** `https://archive.ubuntu.com/ubuntu/pool/main/`
- **Source index:** `https://archive.ubuntu.com/ubuntu/dists/jammy/main/source/Sources.gz`
- **Package search:** `https://packages.ubuntu.com/source/jammy/`

---

## License

All source packages retain their original upstream licenses. The majority are GPL-2.0, LGPL-2.1, MIT, BSD, or Apache-2.0. Individual package licenses are declared in their respective `.dsc` and `debian/copyright` files.

## Copyright

Archive assembly and tooling: Copyright (C) 2026 MEARVK LLC  
Ubuntu source packages: Copyright their respective authors (Canonical Ltd, upstream maintainers)
