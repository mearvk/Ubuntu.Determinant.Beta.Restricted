# TAC3 — Contextual File Identity and the 33 Vital Statistics

## Purpose

This document is the reference caveat for the TAC3 medium-granularity file model.
TAC3 does **not** require filenames, primary IDs, or context areas to be globally unique.

A TAC3 record is understood through its complete contextual state rather than through a
filename or identifier considered in isolation.

> **Same Name is permitted. Same ID is permitted. Same Context is permitted.**

The filename is a descriptive attribute. The File ID is a reference attribute. The
Context ID and Context Area establish the surrounding domain. The complete 33-stat record
provides the medium-granularity state needed to interpret the file.

## The 33 Vital Statistics

1. **File Name** — human-readable name.
2. **File ID** — primary reference identifier.
3. **Context ID** — identifier for the immediate contextual domain.
4. **Context Area** — logical or physical area containing the file.
5. **Parent ID** — immediate parent or containing relationship.
6. **Version** — logical version.
7. **Revision** — revision within the version.
8. **Record Type** — TAC3 record classification.
9. **Creation Timestamp** — creation time.
10. **Modification Timestamp** — most recent modification time.
11. **Owner** — ownership association.
12. **Source** — originating source or system.
13. **Source ID** — identifier assigned by the source.
14. **Location** — physical, logical, or filesystem location.
15. **Device Class** — applicable device classification.
16. **Filesystem Type** — filesystem under which the record exists.
17. **Mount Point** — associated mount context.
18. **Capacity** — relevant total storage capacity.
19. **Allocated Space** — allocated storage.
20. **Available Space** — relevant available storage.
21. **Read Count** — recorded read activity.
22. **Write Count** — recorded write activity.
23. **Read Pressure** — read-related operational pressure.
24. **Write Pressure** — write-related operational pressure.
25. **Wear Level** — applicable storage wear measurement.
26. **Health Level** — current health assessment.
27. **Jarring / Shock Level** — physical or simulated disturbance.
28. **Access Frequency** — frequency of access or interaction.
29. **Integrity Status** — current integrity assessment.
30. **Administrative Status** — administrative condition.
31. **Security / Trust Status** — security, trust, or validation condition.
32. **Parent / Relationship Context** — relationships to other records or structures.
33. **Operational State** — current operational condition.

## Contextual Identity Rule

Multiple records may legitimately share:

- the same File Name;
- the same File ID;
- the same Context ID;
- the same Context Area;
- the same parent domain; or
- combinations of these attributes.

None of those attributes, individually or in a partial combination, shall be treated as
proof that two records are the same operational object.

The TAC3 implementation therefore uses the **complete 33-stat record as the medium state
contract**. A derived contextual signature may be calculated from those statistics for
comparison, indexing, or diagnostics, but the signature is not an additional vital
statistic.

If two records are identical across the complete applicable state and the system must
nevertheless preserve them as separate objects, an explicit relationship/instance
mechanism is required. TAC3 must not manufacture uniqueness merely by changing a filename.

## Design Principle

> **A file is more than its name.**
>
> **An identifier is more than a uniqueness claim.**
>
> **A context is more than a location.**
>
> **The complete state is the meaningful record.**

This is the governing identity principle for the TAC3 medium-granularity model.
