<img align="right" width="75" height="75" src="https://github.com/mearvk/SLeeLa/blob/master/images/debian-logo.png" alt="Debian logo">

# Psych-ID — Network Intelligence & Web Analysis Module

**Port Scanner | Banner Collector | Suggestion Lobotomy | Insect Trimming | Search Prescription Engine**

Part of Ubuntu Determinant Alpha RS — Galactic Cherry Marvell Edition 98

---

## Overview

Psych-ID is a network intelligence daemon that scans standard service ports for MOTD banner messages, builds a ~300 MB database of suspects and web-of-information, and applies a two-stage analyzer engine:

1. **Suggestion Lobotomy** — carry analysis toward center reference while maintaining 3rd and 4th dimension, with respect to law and law again, and with respect to maintenance of science
2. **Insect Trimming** — cut off impossible or dead-end varios (variations) from the analysis web

The module prescribes search engine queries for further information hinting, building an actionable intelligence web from passive port observation.

---

## Scan Ports

| Port | Protocol | Service |
|------|----------|---------|
| 20 | TCP | FTP Data |
| 21 | TCP | FTP Control |
| 22 | TCP | SSH |
| 80 | TCP | HTTP |
| 443 | TCP/TLS | HTTPS |
| 8080 | TCP | HTTP Alternate |
| 8443 | TCP/TLS | HTTPS Alternate |

---

## Analyzer Engine

### Suggestion Lobotomy (7 Stages)

| Stage | Name | Action |
|-------|------|--------|
| 0 | None | Raw, unprocessed data |
| 1 | Center | Carry toward center reference — normalize outliers |
| 2 | Dim3 | Maintain 3rd dimension — spatial/relational context |
| 3 | Dim4 | Maintain 4th dimension — temporal/causal context |
| 4 | Law | Respect to law — legal compliance verification |
| 5 | Law Again | Reinforced — double-check legal boundary |
| 6 | Science | Maintenance of science — reproducibility verification |
| 7 | Full | All stages complete — fully centered and verified |

### Insect Trimming (6 Types)

| Type | Name | Meaning |
|------|------|---------|
| 0 | Impossible | Logically cannot be true |
| 1 | Dead End | Path terminates without yield |
| 2 | Circular | Path loops back to origin |
| 3 | Contradictory | Contradicts established fact |
| 4 | Expired | Temporal window has closed |
| 5 | Superceded | Better information exists |

---

## Feed Modes

| Mode | Description | Cron-Friendly |
|------|-------------|:---:|
| `daily` | Auto-feed at configured hour (default 3:00 AM) | ✓ |
| `command` | Feed only when explicitly invoked | ✓ |
| `feed-update` | Feed AND run full analysis on command | ✓ |
| `reminder` | Random interval between 8-36 hours | ~ |

---

## Database

- **Engine:** SQLite (WAL mode, 256 MB mmap)
- **Max Size:** 300 MB (auto-prune when exceeded)
- **Location:** `/var/lib/psych-id/suspects.db`
- **Tables:** banners, suspects, prescriptions, web_nodes, feed_log
- **Retention:** Prunes oldest records when size limit reached

---

## Usage

```bash
# Start daemon (background)
psych-id daemon

# Scan all configured targets
psych-id feed

# Scan + full analysis + prescriptions
psych-id feed-update

# Scan a single host
psych-id scan 192.168.1.1

# Show status (MOTD banner)
psych-id status

# List pending search prescriptions
psych-id prescriptions

# List tracked suspects
psych-id suspects

# Prune database
psych-id prune

# Stop running daemon
psych-id stop
```

### Options

| Flag | Description |
|------|-------------|
| `-c <file>` | Configuration file path |
| `-m <mode>` | Override feed mode |
| `-v` | Verbose output |
| `-q` | Quiet output |
| `-h` | Help |

### Signals

| Signal | Action |
|--------|--------|
| `SIGUSR1` | Trigger immediate feed (scan only) |
| `SIGUSR2` | Trigger feed + update |
| `SIGTERM` | Graceful shutdown |

---

## Cron Integration

Compatible with the cronie callback extension:

```crontab
# Daily feed + analysis at 3am
0 3 * * * /usr/local/bin/psych-id feed-update @callback {
    expect: "Feed + Update complete"
    retry: 2
    retry_delay: 120s
    on_fail: escalate
    notify: "chat:ops-team"
    timeout: 300s
}

# Hourly quick scan (no full analysis)
0 * * * * /usr/local/bin/psych-id feed @callback {
    expect: "Scanned"
    timeout: 60s
}

# Weekly database prune
0 4 * * 0 /usr/local/bin/psych-id prune @callback {
    expect: "pruned"
    notify: "chat:ops-team"
}
```

Standard cron (without callback extension) works identically:

```crontab
0 3 * * * /usr/local/bin/psych-id feed-update >> /var/log/psych-id-cron.log 2>&1
```

---

## Search Prescription Engine

When the analyzer identifies a suspect of sufficient interest, it generates search engine queries designed to yield further intelligence:

1. **Vulnerability search** — known CVEs, advisories for the service fingerprint
2. **Hardening guide** — CIS benchmarks, vendor best practices
3. **Direct CVE lookup** — site-specific searches on cve.mitre.org, nvd.nist.gov

Prescriptions include:
- The query string (ready to paste into a search engine)
- The recommended engine (duckduckgo, google, bing)
- A hint explaining what to look for in results
- Priority rating (1-10)
- Which lobotomy stage generated the prescription

---

## Configuration

**File:** `/etc/psych-id/psych-id.conf`

| Setting | Default | Description |
|---------|---------|-------------|
| `feed_mode` | daily | Feed scheduling mode |
| `daily_hour` | 3 | Hour for daily feed (0-23) |
| `daily_minute` | 0 | Minute for daily feed (0-59) |
| `reminder_min_hours` | 8 | Minimum hours between reminder feeds |
| `reminder_max_hours` | 36 | Maximum hours between reminder feeds |
| `scan_timeout_ms` | 5000 | Per-port connection timeout |
| `max_concurrent_scans` | 4 | Parallel scan limit |
| `enable_tls` | 1 | TLS probing on 443/8443 |
| `enable_search` | 1 | Generate search prescriptions |
| `enable_web_fetch` | 0 | Auto-fetch prescribed URLs |
| `db_path` | /var/lib/psych-id/suspects.db | Database location |
| `verbose` | 1 | Log verbosity (0-2) |
| `targets_file` | /etc/psych-id/targets.txt | Scan target list |
| `search_engines` | duckduckgo,google,bing | Preferred engines |

---

## Build & Install

```bash
cd tools/psych-id
make
sudo make install
```

**Dependencies:** `libsqlite3-dev`, `libssl-dev`

---

## Files

```
tools/psych-id/psych_id.c              - Main daemon/CLI source (~900 lines)
tools/psych-id/psych_id.h              - Header (structures, API, constants)
tools/psych-id/congregation_sorter.c   - 3rd Order Congregation Sorter (~550 lines)
tools/psych-id/congregation_sorter.h   - Sorter header (axes, ethical table, congruences)
tools/psych-id/psych_id.conf           - Default configuration
tools/psych-id/Makefile                - Build/install rules
tools/psych-id/README.md               - This file
```

**Runtime:**
```
/usr/local/bin/psych-id          - Installed binary (includes Congregation Sorter)
/etc/psych-id/psych-id.conf      - Configuration
/etc/psych-id/targets.txt        - Scan targets
/var/lib/psych-id/suspects.db    - SQLite database (~300 MB, shared with sorter)
/var/log/psych-id.log            - Log file
/var/run/psych-id.pid            - PID file (daemon mode)
```

---

## 3rd Order Congregation Sorter

A binding reagent to the Psych-ID daemon. Sorts all gathered information through a three-axis congregation model derived from the centricities of **Jewish Law** (source quality) and **Mormonism** (relevance quality).

### Centricities

**Jewish Law (Halacha)** informs the SOURCE axis:
- Data has sanctity of provenance
- "Who said it?" matters as much as "what was said"
- Chain of transmission: witnessed → corroborated → documented → canonical
- Authority of the transmitter is weighed

**Mormonism (Restoration)** informs the RELEVANCE axis:
- Data has quality of living relevance
- Revelation is ongoing — what it means NOW matters
- Records are kept for the living, not the dead
- Dormant data can reawaken when new light arrives

### Three Axes

| Axis | Question | Levels (0–6) |
|------|----------|-------------|
| **Source Quality** | Who said it? | Unknown → Hearsay → Witnessed → Corroborated → Documented → Authoritative → Canonical |
| **Relevance Quality** | Is it alive? | Dead → Historical → Dormant → Peripheral → Active → Immediate → Revelation |
| **System Congruence** | Must it be central? | None → Peripheral → Supporting → Structural → Essential → Axiomatic |

### Quality of Ethical Individual & Entity Table

Data is produced by entities — servers, organizations, services. The ethical quality of the producing entity directly informs the Source axis. This is discernment, not surveillance.

| Dimension | Range | Meaning |
|-----------|-------|---------|
| Truthfulness | 0–100 | History of providing accurate data |
| Consistency | 0–100 | Stability, non-contradiction over time |
| Transparency | 0–100 | Openness about operations |
| Harmlessness | 0–100 | Absence of malicious behavior |
| Reliability | 0–100 | Uptime, responsiveness |
| **Composite** | 0–100 | Weighted ethical standing |

An entity starts at 50 (neutral). Each observation moves the score:
- Contradictions decrement truthfulness
- Confirmations increment truthfulness + consistency
- Uptime/availability increases reliability
- The composite flows into congregation records

### What Must Be Central (System Congruences)

| Level | What Belongs | Removal Consequence |
|-------|-------------|---------------------|
| **Axiomatic** | System identity, ethical posture, chain of authority | Coherence destroyed |
| **Essential** | Active high-concern suspects, lobotomy-verified facts | Function impaired |
| **Structural** | Topology, temporal patterns, cross-refs | Reasoning degraded |

### What Falls On Categories (Gravitational Settlement)

| Category | Natural Content |
|----------|----------------|
| Record | Permanent institutional knowledge |
| Warning | Active threat requiring attention |
| Pattern | Recurring behavior, trend |
| Identity | Attribution, fingerprinting |
| Law | Legal/compliance |
| Science | Verifiable technical fact |
| Ethics | Right/wrong |
| Commerce | Economic implication |
| Health | Wellness concern |
| Heritage | Cultural/ancestral |
| Revelation | New understanding (rare) |
| Nowhere | Floats — insect-trim candidate |

### Binding to Psych-ID

The Congregation Sorter hooks into Psych-ID's pipeline:
- Every new banner → auto-sorted (source+relevance inferred)
- Every new suspect → auto-sorted
- Every lobotomy completion → triggers re-sort (source quality rises)
- Periodic resort of unstable records (confidence < 75)

### Installation

Part of the basic userland install:
```bash
make tools           # Builds psych-id + congregation sorter
make tools-install   # Installs to rootfs
```

---

## License

GPL-2.0 — Copyright (C) 2026 MEARVK LLC
