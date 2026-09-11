# TAC3 System Pointers, FAT Co-Linking, and Method Ties

**Status:** Architecture and administration specification  
**Component:** TAC3  
**Path:** `tools/tac3`  
**Copyright:** MEARVK LLC 2026

## 1. Purpose

TAC3 may associate multiple FAT instances with a common **system pointer** when those FAT instances are intended to participate in one logical fileset, system presumption, administrative method, recovery method, or other explicitly defined relationship.

The system pointer is a relationship mechanism. It is not a replacement for the FAT identity, FILE identity, contextual identity, or underlying object reference.

This permits a fileset to have up to five FAT instances while allowing the system to reason about them as a coordinated set.

## 2. Co-Linked FAT Model

The relationship may be represented as:

```text
                         SYSTEM POINTER
                              |
                   +----------+----------+
                   |                     |
              FILESET / METHOD       POINTER STATE
                   |
       +-----------+-----------+-----------+-----------+
       |           |           |           |           |
     FAT-1       FAT-2       FAT-3       FAT-4       FAT-5
       |           |           |           |           |
       +-----------+-----------+-----------+-----------+
                              |
                      FILE / DATA LAYER
                              |
                 +------------+------------+
                 |                         |
             REFERENCES                 OWNED DATA
```

The pointer can therefore identify the **relationship between representations** without requiring the representations themselves to be physically merged.

## 3. What the System Pointer Means

A TAC3 system pointer should be capable of pointing to a logical relationship such as:

- one logical fileset;
- one selected FAT instance;
- a set of co-linked FAT instances;
- an administrative method;
- a recovery method;
- a deployment method;
- a system-presumed relationship;
- a verified relationship;
- a generation or version of the relationship.

The pointer is therefore closer to a **system relationship handle** than to an ordinary filesystem pathname.

## 4. System Presumption and Verified Relationship

The design should distinguish between a relationship that the system is permitted to **presume** and one that has been **verified**.

For example:

```text
PRESUMED
  FAT-1 <-> FAT-2 <-> FAT-3

VERIFIED
  FAT-1 <-> FAT-2
  FAT-3 pending verification
```

A presumption may permit discovery, planning, or administrative presentation, but it must not be treated as proof of integrity, ownership, or data equivalence.

Before a relationship is promoted to verified status, TAC3 should validate the applicable identities, generations, references, integrity information, and authority conditions.

## 5. Method Tie

A system pointer may also establish a **method tie**.

A method tie says, in effect:

> These FAT instances are to be considered together under a specified TAC3 method or administrative procedure.

Examples include:

- common deployment method;
- common recovery method;
- common synchronization method;
- common archival method;
- common reference-resolution method;
- common administrative policy.

The method tie does not make the FAT instances identical. It tells the system how their relationship is to be interpreted.

## 6. Pointer Target Model

A pointer should resolve through stable identifiers rather than relying only on filenames or mutable paths.

A conceptual pointer record is:

```text
SYSTEM POINTER
    Pointer ID
    Fileset ID
    Method ID / Method Type
    Primary FAT ID
    Co-linked FAT IDs [0..4]
    Generation
    Relationship State
    Presumption State
    Verification State
    Authority Metadata
    Integrity Metadata
    Recovery Metadata
```

The exact binary representation remains a future persistent-format implementation concern.

The pointer itself is **not** one of TAC3's 33 contextual identity statistics.

## 7. Pointer-to-FAT Relationship

A pointer may identify one FAT as primary for a particular method while retaining links to additional FAT instances.

For example:

```text
Pointer P-17
    |
    +-- primary: FAT-2
    +-- linked:  FAT-1
    +-- linked:  FAT-3
    +-- linked:  FAT-4
    +-- linked:  FAT-5
```

This allows an administrator or system process to select the appropriate representation without losing awareness of the other co-linked instances.

A primary designation is contextual to the pointer/method. It does not permanently make one FAT universally authoritative over all other TAC3 state.

## 8. Pointer-to-Object Relationship

A system pointer may ultimately resolve through FAT metadata to an object reference.

For example:

```text
SYSTEM POINTER
      |
    FAT-2
      |
   FILE RECORD
      |
  OBJECT REFERENCE
      |
  Carrier / Device / Extent
```

The pointer should not bypass the FILE record when TAC3 identity or contextual state is required.

The FILE layer remains responsible for the contextual identity of the object.

## 9. Reference Preservation

Co-linking makes the earlier no-copy optimization stronger.

If several FAT instances point through the system relationship to the same verified object, TAC3 should be able to preserve one underlying object reference rather than manufacture separate physical copies merely because multiple FAT instances exist.

```text
FAT-1 --+
FAT-2 --+
FAT-3 --+--> SYSTEM POINTER --> OBJECT A
FAT-4 --+
FAT-5 --+
```

This is a reference relationship, not an assertion that every FAT contains identical physical data.

## 10. Divergence Is Permitted

Co-linked FAT instances may diverge.

For example:

```text
FAT-1 -> Object A
FAT-2 -> Object A
FAT-3 -> Object B
FAT-4 -> Object A
FAT-5 -> retired
```

The system pointer may still retain the relationship while recording that the representations have different object mappings.

The method governing the pointer determines whether divergence is acceptable, requires reconciliation, or requires administrator intervention.

## 11. Generation and Staleness

Every system pointer relationship should be evaluated against generation information.

A pointer may become stale when:

- the fileset generation changes;
- a FAT is replaced;
- an object reference changes;
- the carrier changes;
- the method changes;
- integrity verification fails;
- authority is revoked;
- recovery state changes.

A stale pointer should not silently be treated as current.

## 12. Administrator Operations

A future `tac3ctl` interface should support operations such as:

```text
`tac3ctl pointer list --fileset FILESET_ID`
`tac3ctl pointer show --pointer POINTER_ID`
`tac3ctl pointer create --fileset FILESET_ID --method METHOD_ID`
`tac3ctl pointer link --pointer POINTER_ID --fat FAT-2`
`tac3ctl pointer unlink --pointer POINTER_ID --fat FAT-4`
`tac3ctl pointer verify --pointer POINTER_ID`
`tac3ctl pointer promote --pointer POINTER_ID`
```

These are proposed command forms until implemented.

The `link`, `unlink`, `verify`, and `promote` operations should be treated as administrative state changes and should therefore record authority and generation information.

## 13. Safety Rules

The system pointer must not:

- turn a filename into an identity;
- bypass the TAC3 33-stat contextual identity model;
- imply ownership merely because a FAT is linked;
- imply data equivalence merely because FATs are co-linked;
- suppress integrity verification;
- make a presumed relationship appear verified;
- silently redirect recovery to an unverified target;
- overwrite a valid relationship without an authorized state transition.

The pointer is a coordination mechanism, not an integrity shortcut.

## 14. Relationship to the Five-FAT Limit

The existing five-instance policy remains:

```text
FAT-1
FAT-2
FAT-3
FAT-4
FAT-5
```

A system pointer may link any subset of those five instances.

Examples:

```text
Pointer A -> FAT-1, FAT-2
Pointer B -> FAT-2, FAT-3, FAT-4
Pointer C -> FAT-1, FAT-2, FAT-3, FAT-4, FAT-5
```

This means that five FAT instances do not require five independent system pointers, and one FAT may participate in more than one explicitly authorized method relationship.

## 15. Multiple People and Administrative Context

The fileset may have multiple administrators or users operating under the established TAC3 authority model.

A system pointer should therefore preserve administrative context rather than assuming that the person who created a pointer is the permanent owner of every linked object.

Authority remains governed by the existing TAC3 notation:

- `SUDO 1` through `SUDO 8`;
- `USER`;
- `GROUP`;
- `OWNER`;
- `TRUSTED`;
- `GENIUS`.

The pointer records the administrative relationship; it does not redefine these authorities.

## 16. Recovery Use

During recovery, a valid system pointer may provide a compact route to a group of FAT instances.

The recovery process should nevertheless continue to follow:

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

A pointer may accelerate discovery, but it cannot turn an unverified FAT into a trusted recovery source.

## 17. Read-Only FILE Layer

The Read-Only FILE Layer may expose system-pointer information such as:

- pointer ID;
- fileset ID;
- linked FAT IDs;
- primary FAT for the selected method;
- method type;
- relationship state;
- presumption state;
- verification state;
- generation;
- stale/current state.

These are administrative and relationship views. They are not additions to the 33-stat identity record.

## 18. Persistent Format Direction

When persistent pointer records are implemented, the TAC3 on-disk format should preserve enough information to reconstruct the relationship without relying solely on mutable paths.

At minimum, the persistent representation should support:

1. stable pointer identity;
2. logical fileset identity;
3. linked FAT instance identities;
4. method identity/type;
5. generation;
6. relationship state;
7. presumption/verification state;
8. integrity information;
9. authority information;
10. recovery relationship.

The persistent implementation must be transactional and must not claim durable pointer semantics until the corresponding records are actually written and recoverable.

## 19. Governing Principle

> **A FAT may be co-linked without being physically merged.**
>
> **A system pointer points to the relationship and method, not merely to a filename.**
>
> **A system presumption is not proof; verification promotes a relationship to trusted operational state.**
>
> **Co-linking permits efficient reference preservation while maintaining separate FAT identities.**
>
> **The pointer coordinates the system; the FILE layer establishes contextual identity; the underlying object establishes the data relationship.**

This gives TAC3 a clean mechanism for coordinated FAT instances, system-level assumptions, method ties, and reference-efficient administration without weakening the underlying identity and recovery architecture.
