# TAC3 Installation Profile

## Objective

TAC3 should be a regular, optional installation component rather than a requirement
for establishing a working operating system.

The recommended installation sequence is:

```text
1. Install the standard/base operating system.
2. Boot the standard/base operating system successfully.
3. Verify the installation and boot path.
4. Allocate a selected amount of storage for TAC3.
5. Create and initialize the TAC3 partition/filesystem.
6. Install TAC3 boot/recovery state.
7. Run the TAC3 startup integrity/health validation.
8. Register TAC3 as an available recovery/boot target.
9. Reboot and validate the TAC3 recovery path.
```

The base operating system therefore has a chance to establish a known-good working
state before TAC3 is introduced.

## Partition sizing

TAC3 should accept an administrator-selected partition size rather than requiring a
single fixed size.

The installer should validate:

- requested TAC3 size;
- available unallocated storage;
- partition alignment;
- filesystem/device compatibility;
- sufficient space for the TAC3 recovery environment;
- sufficient space for the intended TAC3 workload;
- preservation of the EFI System Partition and base operating system.

The TAC3 size is an installation parameter, not an identity statistic.

## Installation modes

### Standard installation

```text
BASE OS
  + EFI
  + ROOT / STANDARD FILESYSTEM
  + optional TAC3 partition
```

The system boots normally from the standard installation while TAC3 remains available
for storage, administration, recovery, and later boot use.

### Recovery-capable installation

```text
EFI
  |
  +--> BASE OS
  |
  +--> TAC3 RECOVERY
```

TAC3 is initialized with its protected recovery state and startup integrity scan.
The normal operating system remains the default boot target until the administrator
selects or recovery logic requires TAC3.

### TAC3 boot/recovery installation

The administrator may designate TAC3 as a boot/recovery target after the TAC3
partition has passed initialization and integrity validation. The firmware-facing
EFI handoff remains separate in the initial architecture.

## Safety contract

TAC3 installation must not require destruction of a known-good base operating system.

Before partitioning or initialization, the installer should establish:

- the current boot target;
- the base filesystem identity and mount information;
- available free/unallocated storage;
- the intended TAC3 partition boundary;
- a recoverable boot configuration;
- the TAC3 recovery requirements.

The installer must refuse an ambiguous or unsafe partition operation rather than
silently selecting a destructive alternative.

## Post-install validation

After TAC3 initialization, validation should include:

1. TAC3 metadata initialization check.
2. TAC3 contextual-table check.
3. TAC3 health/wear/pressure baseline.
4. Recovery manifest verification.
5. Boot/recovery object verification.
6. Base filesystem accessibility check.
7. Rollback-state availability check.
8. Startup integrity scan.

Only after successful validation should TAC3 be registered as a trusted recovery
resource.

## Removal

TAC3 should be removable without making the base operating system unusable, provided
that TAC3 is not currently the only active boot path and the administrator has not
explicitly designated it as the active recovery/boot target.

Removal must therefore first verify that the standard boot path remains valid.

## Design principle

> Install the base system first. Prove that it works. Add TAC3 as an independently
> sized, validated recovery and filesystem resource. Only then promote TAC3 to an
> optional boot/recovery role.

This makes TAC3 regular for ordinary installations while preserving the stronger
recovery architecture for systems that elect to use it.
