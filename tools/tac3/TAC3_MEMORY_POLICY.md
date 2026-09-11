# TAC3 — Dynamic Memory and Swap Policy

## Purpose

TAC3 shall treat its relevant tables as **usage-loaded state** rather than assuming that every table must remain resident in RAM at all times.

The preferred operating model is:

1. Load actively used TAC3 tables into memory.
2. Retain hot tables and recently accessed contextual records in RAM.
3. Allow cold or inactive table material to become eligible for dynamic displacement.
4. When TAC3's memory working set becomes material relative to system RAM, dynamically use the configured `/swap` backing store for eligible cold state.
5. Restore displaced state when usage makes it relevant again.

## Initial Memory Threshold

The initial TAC3 policy target is **12% of total system RAM** as the threshold at which TAC3 should begin treating its table working set as a candidate for dynamic swap management.

This is a **policy threshold, not a hard allocation of 12% of RAM**. TAC3 should not reserve 12% of RAM merely because the machine has that amount available.

The threshold means that when the TAC3 resident working set exceeds approximately 12% of total system RAM, TAC3 may begin moving eligible cold table state toward the `/swap` backing store, subject to system memory pressure and the availability of swap.

## Dynamic Behavior

TAC3 should maintain a distinction between:

- **Hot state** — actively used tables and records that should remain resident when practical.
- **Warm state** — recently used state that may remain resident until memory pressure increases.
- **Cold state** — infrequently used state eligible for displacement.
- **Required state** — state that must remain resident because active operations depend upon it.

The memory manager should prefer cold state for displacement before warm state and should avoid displacing required state merely because the TAC3 threshold has been crossed.

## Hysteresis

The 12% threshold should not be implemented as a single on/off switch.

TAC3 should use hysteresis so that repeated crossing of the threshold does not cause constant swapping. A lower-watermark policy should determine when displacement activity can relax after the working set falls below the pressure threshold.

The exact lower watermark should remain configurable and should be established during implementation testing.

## Swap Is a Backing Mechanism, Not the Canonical Record

The `/swap` file is a backing mechanism for eligible memory-resident TAC3 state. It is not the authoritative filesystem record.

Canonical TAC3 state remains associated with the filesystem's persistent representation. Swap may temporarily contain displaced representations of tables or working-state pages.

TAC3 must therefore tolerate:

- swap unavailable;
- swap disabled;
- swap insufficient;
- swap temporarily full;
- memory pressure before the TAC3 threshold;
- memory pressure above the TAC3 threshold; and
- restoration of previously displaced state.

TAC3 must remain functionally correct when no swap is available. The 12% policy must therefore be treated as an optimization and pressure-management policy rather than a correctness dependency.

## Interaction With the 33 Vital Statistics

The TAC3 33-stat record remains the medium-granularity state contract. Memory residency is an implementation property and does not become an additional statistic.

A record may move between resident and displaced state without changing its identity merely because its memory location changes.

In particular:

> **Memory residency does not define file identity.**

The same filename, File ID, Context ID, and Context Area may continue to identify contextual records regardless of whether their associated tables are currently resident in RAM or temporarily backed by `/swap`.

## Important Implementation Considerations

Before making this policy authoritative in the kernel implementation, TAC3 should establish:

1. whether the 12% measurement is based on total physical RAM or currently usable RAM;
2. whether kernel-reserved memory is excluded from the denominator;
3. the exact lower watermark for hysteresis;
4. maximum TAC3 RAM consumption under heavy workload;
5. maximum acceptable swap latency;
6. whether table pages or serialized table records are the swap unit;
7. locking and synchronization during displacement and restoration;
8. behavior when swap is unavailable or exhausted;
9. protection against swap thrashing;
10. accounting for multiple TAC3 mounts;
11. interaction with ordinary kernel memory reclaim; and
12. whether administrators may configure or override the threshold.

## Governing Principle

> **TAC3 loads what is used, retains what is hot, displaces what is cold, and never makes swap a requirement for correctness.**

The initial design target is a **12% of system RAM working-set threshold** for dynamic swap consideration, with hysteresis and workload-aware reclamation to be defined before kernel-level enforcement.
