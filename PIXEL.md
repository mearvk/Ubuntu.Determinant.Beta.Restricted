# PIXEL.md

## Purpose

This document provides the project-level PIXEL view of Ubuntu.Determinant.Beta.Restricted.

PIXEL answers two questions:
1. **What's Made** — the major systems, models, tools, and architectural capabilities represented by the repository.
2. **What's Included** — the source, scripts, data, specifications, experiments, workflows, and documentation that make those capabilities inspectable.

The repository is intentionally experimental and broad. PIXEL describes architectural scope without turning every prototype or specification into a claim of production certification.

# 1. What's Made

## 1.1 Proffer Systems Model

The project develops a common vocabulary for secure-runtime and systems reasoning across SecureJDK, Graal, native C/C++, Linux kernel state, memory, processes, time, geometry, provenance, policy, integrity, and historical/economic data.

The central Proffer framing asks:

```text
state → origin → transition → capability → authorization → next evidence
```

Provenance, validation, authorization, and policy remain distinct concepts.

## 1.2 Total — Three-Tier Native Moderator

Total is the native C/C++ moderator layer under /total/.

```text
TOP     SecureJDK 28 / Graal
  ↓
MIDDLE  Total
  ↓
GROUND  Linux kernel / hardware
```

Ground establishes operating-system facts. Total mediates evidence, resource policy, provenance, and service behavior. Top supplies managed-runtime and application semantics.

Total is deliberately conservative: it observes Linux memory state and maintains controlled admission accounting without replacing Linux virtual memory, malloc/free, or JVM garbage collection.

## 1.3 Evidence Pipeline

The documented evidence flow is:

```text
input → normalization → provenance → validation → policy
      → action → observation → retained evidence
```

Potential evidence includes process/thread state, memory pressure, executable/library descriptors, package metadata, JVM/Graal events, signed configuration, filesystem provenance, service lifecycle events, cgroup/PSI observations, integrity measurements, resource requests, and diagnostics.

An input is evidence to be evaluated, not automatically proof of truth.

## 1.4 Domain-Service Architecture

The three-tier mechanism is designed to support domain adapters where provenance, authorization, auditability, and controlled resource policy matter. Documented examples include banking, hospitality, regulated adult services, and other regulated commerce.

The common domain sequence is:

```text
identify → describe → authorize → transact → observe → retain → audit
```

The application remains the business authority. Total supplies infrastructure for evidence, provenance, resource policy, and controlled mediation.

## 1.5 JSpec Pixel Format — JPIX

The repository develops an experimental JSpec Pixel Format (.jpix) centered on the Pixel Map.

> The Pixel Map is the object; a rectangle is only a storage or rendering envelope when required.

JPIX models mapped pixels, transparency, boundaries, topology, orientation, transformations, and deterministic rendering. PNG and JPEG remain useful raster/export formats.

## 1.6 UTF-4088 Character and Graph System

/utf-4088/ contains an experimental character and graph system involving a 16,606-symbol front end, 8×12 glyph representations, historical language seeds, directed concept graphs, historical context priors, sampling, and neural relaying.

The documented language tuple is American English ↔ Korean ↔ Germanic.

Historical evidence and derived glyph representations are kept conceptually distinct.

## 1.7 Sampling and Semantic Modeling

UTF-4088 includes experimental normal-distributive sampling across time, cause, and primer density, along with semantic branches for cognition, psychology, emotion, development, relationships, nutrition, social concepts, ethics, environment, temporal/causal relationships, and meta-graphs.

Recorded sampling coefficients and candidate counts are experimental model results, not universal linguistic, physical, or intelligence constants.

## 1.8 Time and Opportunity Norming

The UTF-4088 algebra layer includes experimental outputs such as scatter, norm_cost, fine_cost, and opportunity_cost.

The repository describes these as accounting and observability models, not mechanisms for inferring human worth or imposing automatic penalties based on identity, nationality, language, ethnicity, role, or historical association.

## 1.9 SecureJDK 28 / Graal Architecture

The intended runtime layering is:

```text
Application
    ↓
SecureJDK 28
    ↓
Graal runtime / compiler
    ↓
Proffer + Total assistance
    ↓
Linux
```

The direction is to add provenance, policy, resource, and integrity hooks while preserving ordinary Java compatibility where practical.

## 1.10 Native Git Operation Intelligence

The repository includes a native policy layer under /tools/git/ covering add, commit, push, merge, rebase, restage, autocheck, and moral.

Documented planning contracts include 100 MiB logical add blocks, 50 MiB logical commit units, and 200 MiB conservative push accounting. These are policy/planning contracts; Git's actual object graph and transport behavior remain authoritative.

## 1.11 Moral Command

The repository includes a ceremonial moral operation with documented mana accounting. It is explicitly separated from security authorization, identity assessment, correctness, access control, and human worth.

## 1.12 Historical, Economic, and Geographic Data

The repository contains substantial dated datasets and research material supporting experiments involving time, geography, historical context, economics, policy modeling, sampling, and provenance.

# 2. What's Included

## 2.1 Native Source

Native C/C++ source is present for Total, Git policy tooling, UTF-4088 algebra, and supporting utilities.

## 2.2 SecureJDK / Graal Material

The repository contains runtime architecture, security concepts, provenance material, and supporting research for the SecureJDK/Graal direction.

## 2.3 UTF-4088

The /utf-4088/ tree contains character-system source, glyph material, historical language data, algebra, sampling experiments, semantic models, graph structures, results, methodology, and derived research artifacts.

## 2.4 JPIX Material

The repository contains specifications and supporting material for Pixel Maps, .jpix, boundaries, transparency, rendering, trimming, color-depth models, deterministic rasterization, and integrity concepts.

## 2.5 Git Policy Tooling

The /tools/git/ area includes operation logic, native policy companions, metadata concepts, add/commit/push accounting, merge/rebase policy, restage, autocheck, and moral policy.

## 2.6 Data and Research Material

Datasets include annual country/region material, historical data, economic material, language data, geographic material, CSV research data, model outputs, and methodology/results documents.

## 2.7 Markdown Specifications

The /markdown/ area provides major architectural specifications covering three-tier architecture, domain services, progress, security, SecureJDK/Graal, Git policy, UTF-4088, JPIX, research methodologies, and OS/native architecture.

## 2.8 Automation and CI

The .github/workflows/ tree contains automation for multiple data and build tasks. Workflow definitions are evidence of specified automation; they are not by themselves proof that every workflow currently succeeds.

## 2.9 Agent and Task Material

The .agents/tasks/ tree records engineering tasks, reviews, contexts, and feature definitions, providing an inspectable engineering trail.

# 3. How to Write This Kind of Stuff

## 3.1 Define the System Boundary

Start with one clear description of the repository before listing individual experiments.

## 3.2 Separate Made From Included

**What's Made** describes capabilities and architecture. **What's Included** describes repository objects such as source, data, scripts, specifications, and workflows.

## 3.3 Give Research Claims Their Experimental Status

For experimental numbers, record sample count, model/version, inputs, method, result, and limitations. Do not turn one coefficient into a universal law.

## 3.4 Distinguish Specification From Implementation

A Markdown design can specify future architecture; source implements behavior; tests demonstrate behavior. PIXEL should keep those categories distinct.

## 3.5 Preserve Provenance

For data and generated outputs, maintain the chain:

```text
source → date → transformation → model/version → output
```

## 3.6 Describe Security Concretely

Describe hashes, signatures, authorization, policy, scanning, or isolation mechanisms directly rather than converting them into absolute claims of security.

## 3.7 Keep Human-Related Models Bounded

Computational categories must not be silently transformed into unsupported conclusions about a real person's identity, worth, consent, morality, intelligence, or status.

# 4. Recommended PIXEL Pattern

```text
Purpose
1. What's Made
2. What's Included
3. How to Write This Kind of Stuff
4. Recommended PIXEL Pattern
5. Current Project Snapshot
6. Maintenance Rule
```

# 5. Current Project Snapshot

| Area | Repository representation |
|---|---|
| Project | Ubuntu.Determinant.Beta.Restricted |
| Primary direction | Experimental secure systems / research platform |
| Runtime direction | SecureJDK 28 / Graal |
| Native moderator | Total |
| OS foundation | Linux / kernel / hardware |
| Evidence model | Proffer |
| Character system | UTF-4088 |
| Pixel format | JPIX / .jpix |
| Semantic modeling | Directed graphs / historical priors / experimental samplers |
| Native Git policy | add / commit / push / merge / rebase / restage / autocheck / moral |
| Data | Historical, geographic, economic, language, and experimental datasets |
| Main documentation | /markdown/ |
| Automation | .github/workflows/ |
| Engineering records | .agents/tasks/ |
| Status | Experimental |

The README describes the project as experimental and specifically cautions that Total, UTF-4088, JPIX, SecureJDK/Graal integration, domain-service adapters, and native Git policy should not be treated as production-certified merely because their architecture is documented.

# 6. Maintenance Rule

**PIXEL follows the repository.**

When the project changes materially:

1. Keep implementation source authoritative.
2. Keep specifications synchronized with implementation.
3. Record experimental inputs, methods, and outputs.
4. Preserve provenance for data and generated artifacts.
5. Keep format versions explicit.
6. Keep security descriptions limited to demonstrated mechanisms.
7. Keep CI claims tied to actual workflow results.
8. Distinguish planned architecture from implemented behavior.
9. Keep research measurements labeled as experimental where appropriate.
10. Update PIXEL when the high-level architecture materially changes.

> **Make an unusually large systems repository understandable without flattening its experiments, specifications, data, or engineering claims into something they are not.**