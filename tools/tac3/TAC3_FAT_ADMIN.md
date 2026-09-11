# TAC3 FAT Administration and Replicated File Allocation Tables

**Status:** Architecture and administration specification  
**Component:** TAC3  
**Path:** `tools/tac3`  
**Copyright:** MEARVK LLC 2026

## 1. Purpose

TAC3 may operate with a modern FAT-compatible administrative layer when a conventional, portable, independently duplicable file-allocation table is useful. This layer is an **option**, not a requirement for TAC3.

The design permits one person, administrator, installation, or fileset to maintain multiple FAT instances for the same logical fileset. The initial supported administrative maximum is **five FAT instances per fileset**.

The purpose is straightforward:

- provide a familiar and portable allocation/index structure;
- allow the same logical fileset to have several independently stored FAT copies;
- permit an administrator to create or duplicate a FAT from the terminal;
- reduce unnecessary object copying when a safe reference is sufficient;
- keep TAC3's native storage mode independent of FAT;
- preserve TAC3's own contextual identity, integrity, recovery, and authority model.

A FAT instance is therefore an administrative representation of allocation and file relationships. It is not, by itself, the authoritative TAC3 filesystem identity.

## 2. Native TAC3 Is Still Valid Without FAT

TAC3 can be constructed and operated directly on TAC3-owned storage without FAT, ext4, ext5, or another conventional filesystem.

The FAT layer is a **carrier/administrative option**:

```text
                         TAC3
                           |
             +-------------+-------------+
             |                           |
        Native TAC3                 FAT-backed option
             |                           |
      TAC3-owned format          Conventional carrier
             |                           |
             +-------------+-------------+
                           |
                    FILE / DATA layer
```

The presence of FAT must never become a hidden prerequisite for boot, recovery, contextual identity, or TAC3 persistence.

## 3. One Fileset, Up to Five FAT Instances

A logical fileset may have one or more FAT instances.

The initial administrative limit is:

```text
FAT-1
FAT-2
FAT-3
FAT-4
FAT-5
```

Five is an administrative maximum for this design, not a filesystem-wide mathematical limitation. A future format revision may raise the limit through an explicit versioned policy change.

Each FAT instance should have its own stable instance identity and should identify the logical fileset to which it belongs.

Conceptually:

```text
                    Logical Fileset F
                           |
        +------------------+------------------+
        |         |         |         |        |
      FAT-1     FAT-2     FAT-3     FAT-4    FAT-5
        |         |         |         |        |
      copy      copy      copy      copy     copy
```

The copies are not assumed to be byte-for-byte identical. They may be independently generated, transported, repaired, or refreshed while continuing to describe the same logical fileset.

## 4. FAT Instance Identity

Every TAC3-managed FAT instance should carry, directly or through its administrative record:

1. FAT instance identifier;
2. logical fileset identifier;
3. generation number;
4. creation timestamp;
5. last administrative update timestamp;
6. source/reference identifier when duplicated;
7. integrity/checksum information;
8. allocation-table format/version;
9. carrier filesystem type when applicable;
10. administrator/authority metadata;
11. relationship to other FAT instances;
12. current administrative state.

This prevents the system from treating five independent tables as one undifferentiated object.

## 5. Terminal Duplication

The administrative interface should support a direct terminal operation for duplicating a FAT instance.

Proposed command form:

```text
`tac3ctl fat duplicate --fileset FILESET_ID --source FAT_ID --target FAT_ID`
```

Example:

```text
`tac3ctl fat duplicate --fileset 7f3a --source FAT-1 --target FAT-2`
```

The command should:

1. authenticate the requesting authority;
2. locate the source FAT;
3. verify that the source belongs to the requested fileset;
4. verify that the target instance number is within 1–5;
5. refuse to overwrite an existing target unless an explicit replacement mode is supplied;
6. copy or reproduce the FAT representation;
7. establish the target's new instance identity;
8. retain source relationship metadata;
9. verify the resulting target;
10. record the administrative action.

A future implementation may also provide:

```text
`tac3ctl fat list --fileset FILESET_ID`
`tac3ctl fat verify --fileset FILESET_ID --fat FAT-2`
`tac3ctl fat promote --fileset FILESET_ID --fat FAT-2`
`tac3ctl fat retire --fileset FILESET_ID --fat FAT-5`
```

These command forms are proposed administrative interfaces until implemented in `tac3ctl`.

## 6. Duplicate the Table, Not Necessarily the Data

The central optimization is that duplicating a FAT does **not** automatically require duplicating every file's data.

For example:

```text
FAT-1 ----+
FAT-2 ----+----> same underlying object
FAT-3 ----+
```

If three FAT instances safely identify the same immutable or reference-safe object, TAC3 may retain a reference to that object rather than creating three physical data copies.

This is particularly useful for:

- large immutable files;
- installation media;
- deployment packages;
- archives;
- shared read-only resources;
- boot/recovery resources;
- files already present on a trusted carrier filesystem.

The governing distinction is:

> **Duplicate the allocation/reference information when that is sufficient; duplicate the underlying data only when ownership or correctness requires it.**

## 7. Reference Is Not Ownership

A FAT reference must never be interpreted as proof that TAC3 owns the underlying data.

Before retaining a reference instead of copying data, TAC3 should be able to establish sufficient information about:

- source filesystem;
- source device or carrier;
- source object identity;
- location or extent information;
- object size;
- generation/version where available;
- integrity information;
- access permissions;
- relationship to the logical fileset;
- authority/trust state;
- availability requirements.

If the reference cannot be safely reconstructed or verified, TAC3 should fall back to an owned-data representation when permitted.

## 8. When TAC3 Should Copy

TAC3 should create an owned data representation when one or more of the following apply:

- the source object can disappear independently of TAC3;
- TAC3 requires independent durability;
- the source object may be modified without TAC3 knowing;
- the source integrity cannot be verified;
- recovery requires a self-contained copy;
- security or trust policy prohibits external references;
- the target operation changes the object;
- the source filesystem cannot preserve the required metadata;
- the administrator explicitly requests an owned copy.

Copy-on-write is preferred where an object can remain safely shared until a modification actually requires independent storage.

## 9. When TAC3 Should Prefer a Reference

A reference is preferable when:

- the object is safely immutable;
- the source remains available for the intended lifetime;
- integrity can be verified;
- the reference contains enough information for deterministic rediscovery;
- multiple FAT instances would otherwise create redundant copies;
- the administrator has authorized reference-backed operation;
- recovery semantics remain valid without a second physical copy.

This allows TAC3 to save time and storage during duplication operations that would otherwise be pure data copies.

## 10. FAT and Conventional Filesystems

A FAT-backed TAC3 deployment may use a modern FAT-family filesystem or another conventional carrier with an appropriate adapter.

The carrier is not required to expose itself as the TAC3 filesystem. Instead, TAC3 may use the carrier as a storage and interchange environment while maintaining its own logical identity and administrative records.

The initial design should therefore distinguish:

```text
Carrier filesystem
    |
    +-- FAT instance
    +-- TAC3 administrative metadata
    +-- reference-backed objects
    +-- owned/copied objects
```

TAC3 must not silently treat a carrier's filename alone as sufficient identity. The 33-stat contextual identity model remains authoritative for TAC3 records.

## 11. Administrative Authority

FAT creation, duplication, promotion, replacement, and retirement are administrative operations.

They should use the established authority notation already recognized by TAC3:

- `SUDO 1` through `SUDO 8`;
- `USER`;
- `GROUP`;
- `OWNER`;
- `TRUSTED`;
- `GENIUS`.

TAC3 should consume these established authority names rather than inventing a second authority naming scheme.

Authority metadata is administrative metadata. It does not silently become an additional TAC3 identity statistic.

## 12. Administrator Workflow

A practical administrative sequence is:

```text
1. Identify fileset.
2. Inspect existing FAT instances.
3. Select source FAT.
4. Select unused target FAT slot (1–5).
5. Verify authority.
6. Duplicate FAT representation.
7. Preserve safe references where possible.
8. Copy only data that requires independent ownership.
9. Verify allocation and references.
10. Record generation and administrative state.
11. Verify the new FAT.
```

The administrator should be able to inspect all five slots with one command and determine which are active, valid, stale, unavailable, or retired.

## 13. Failure and Recovery

If a FAT instance becomes invalid, TAC3 should not automatically destroy the remaining instances.

For example:

```text
FAT-1  VALID
FAT-2  VALID
FAT-3  INVALID
FAT-4  VALID
FAT-5  RETIRED
```

An administrator may then reconstruct FAT-3 from an authoritative valid instance if policy permits.

A duplicated FAT is therefore useful as an administrative recovery resource, but it must not be described as a complete backup unless its referenced data is independently recoverable.

## 14. Relationship to TAC3 Recovery

FAT duplication must integrate with the TAC3 recovery model:

```text
DETECT
  -> ASSESS
  -> PRESERVE
  -> VERIFY
  -> AUTHORIZE
  -> RECOVER
  -> VERIFY AGAIN
  -> BOOT
```

A FAT instance may help TAC3 locate or reconstruct objects, but FAT duplication must never bypass integrity checks or transactional recovery rules.

## 15. Relationship to the Read-Only FILE Layer

The Read-Only FILE Layer may expose FAT administration state for inspection, including:

- fileset identity;
- FAT instance count;
- FAT instance identifiers;
- generation;
- carrier type;
- reference-backed status;
- owned-data status;
- integrity state;
- administrative state.

These views are diagnostic/administrative views unless and until the corresponding state is durably implemented in the TAC3 on-disk format.

## 16. Implementation Phases

The intended implementation sequence is:

### Phase 1 — Specification

Define FAT instance identity, fileset relationship, five-instance limit, reference semantics, and administrative state.

### Phase 2 — Native FAT Adapter

Implement an adapter capable of reading/writing the selected FAT representation without making FAT mandatory for native TAC3 storage.

### Phase 3 — FAT Administration

Add `tac3ctl fat list`, `duplicate`, `verify`, and related administrative operations.

### Phase 4 — Reference-backed FILE Records

Add persistent FILE metadata capable of identifying externally stored objects without unnecessary data duplication.

### Phase 5 — Owned Data / Copy-on-Write

Add TAC3-owned data placement and copy-on-write behavior where references are no longer sufficient.

### Phase 6 — Recovery Integration

Allow valid FAT instances to participate in reconstruction and recovery while preserving TAC3's transactional and integrity rules.

## 17. Governing Principle

> **TAC3 may use FAT; TAC3 does not depend on FAT.**
>
> **A fileset may have up to five administratively managed FAT instances.**
>
> **Duplicating a FAT should not imply duplicating every underlying file.**
>
> **Where a verified reference is sufficient, preserve the reference. Where independent ownership is required, copy or materialize the data.**
>
> **The administrator controls the operation; TAC3 preserves identity, integrity, authority, and recovery semantics.**

This architecture gives TAC3 a practical conventional-filesystem option while retaining the central design goal: avoid unnecessary copying without weakening ownership, integrity, recovery, or contextual identity.
