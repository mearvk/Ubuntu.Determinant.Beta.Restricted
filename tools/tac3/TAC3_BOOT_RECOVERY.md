# TAC3 Boot and Recovery Contract

## Purpose

TAC3 may be used as a boot-capable recovery filesystem so that the health of the
ordinary/base filesystem is not a prerequisite for system recovery.

The initial design retains a small firmware-compatible EFI System Partition (ESP)
for the firmware-facing boot handoff. TAC3 then provides the protected boot and
recovery environment behind that handoff.

## Boot architecture

```text
UEFI / firmware
      |
      v
minimal bootloader / EFI System Partition
      |
      v
TAC3 boot and recovery environment
      |
      +-- kernel
      +-- initramfs / recovery environment
      +-- TAC3 filesystem support
      +-- trusted recovery manifest
      +-- known-good recovery state
      +-- recovery tools
      |
      v
standard/base filesystem
```

TAC3 is therefore not required to replace every firmware-facing component in the
initial implementation. The architecture leaves the firmware handoff small and
stable while giving TAC3 responsibility for the protected recovery environment.

## Startup system integrity and health scan

A TAC3-capable startup performs a non-destructive system integrity and health scan
before normal operation is authorized.

The preferred sequence is:

```text
DETECT
  -> ASSESS
  -> PRESERVE
  -> VERIFY
  -> AUTHORIZE
  -> BOOT / RECOVER
  -> VERIFY AGAIN
```

The startup scan should inspect, at minimum:

- boot environment availability;
- TAC3 filesystem availability;
- TAC3 metadata and contextual tables;
- known-good recovery manifest;
- required kernel and initramfs objects;
- integrity status of protected recovery objects;
- TAC3 storage health, wear, pressure, and operational state;
- availability of required backing storage;
- base filesystem discoverability;
- base filesystem integrity where a non-destructive check is available;
- boot and recovery generation consistency;
- recovery rollback availability.

The scan is diagnostic first. It must not silently modify a damaged filesystem.

## Base filesystem failure

If the base filesystem is healthy, normal boot continues.

If the base filesystem is degraded, TAC3 should prefer read-only assessment and
controlled repair rather than immediate modification.

If the base filesystem is severely damaged, TAC3 may enter recovery mode and use the
protected TAC3 recovery state to repair or restore the base filesystem, subject to
administrative authorization and successful integrity validation.

The governing recovery sequence is:

```text
READ-ONLY ASSESSMENT
        |
        v
PRESERVE AVAILABLE STATE
        |
        v
VERIFY KNOWN-GOOD RECOVERY STATE
        |
        v
AUTHORIZE RECOVERY
        |
        v
RECOVER / RESTORE
        |
        v
VERIFY RESTORED STATE
        |
        +---- failure ---> ROLLBACK / REMAIN IN RECOVERY
        |
        v
NORMAL BOOT
```

## Safety requirements

1. TAC3 must not destroy the last known-good bootable recovery state during repair.
2. Recovery operations should be transactional wherever the underlying storage
   permits it.
3. A failed recovery attempt must leave the system in a known recovery state rather
   than falsely declaring a successful boot.
4. A damaged base filesystem must not be treated as authoritative merely because it
   is the normal root filesystem.
5. TAC3 recovery state must remain independently meaningful from the base filesystem.
6. Integrity verification is required before activating restored boot/root state.
7. Rollback must remain available until post-recovery verification succeeds.
8. Startup health scanning must not silently redefine TAC3 file identity or its
   contextual identity model.
9. SUDO 1 through SUDO 8, USER, GROUP, OWNER, TRUSTED, and GENIUS remain established
   authority/reference notation. TAC3 must consume those names as supplied rather
   than inventing a proprietary replacement naming scheme.
10. `/swap` remains a backing mechanism under the TAC3 memory policy; it is not the
    canonical source of TAC3 filesystem truth.

## Relationship to the 33-stat model

The boot/recovery contract does not create a new identity statistic. The existing
33 vital statistics remain the contextual file identity model. Startup health,
recovery generation, boot authorization, and recovery state are operational controls
around that model.

A contextual signature remains a derived diagnostic/indexing value and is not promoted
to a 34th statistic.

## Recovery independence principle

> TAC3 is capable of recovering the standard filesystem without requiring the
> standard filesystem to be healthy.

This does not mean that destruction of the physical storage containing TAC3 can be
ignored. Persistent TAC3 state remains dependent upon its own underlying storage and
upon whatever independent recovery copies are intentionally maintained.

## Design objective

The strongest safe architecture is therefore:

- minimal firmware-facing boot dependency;
- protected TAC3 recovery environment;
- startup integrity and health assessment;
- read-only-first failure handling;
- explicit recovery authorization;
- transactional restoration where possible;
- post-recovery verification;
- protected rollback state;
- no silent destruction of the last known-good boot path.

The purpose of this contract is resilience: the normal filesystem should be allowed
to fail without making the recovery mechanism fail with it.
