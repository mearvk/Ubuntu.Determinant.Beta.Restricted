#!/usr/bin/env bash
# TAC3 partition installer.
#
# Default mode is a read-only plan. --execute is required for partition writes.
# The script only uses unallocated space reported by parted and refuses to run
# against a disk with mounted child partitions. TAC3 formatting is delegated to
# mkfs.tac3; no substitute filesystem is silently created.
set -euo pipefail

PROGRAM="tac3-partition-installer"
DISK=""
SIZE=""
EXECUTE=0

usage() {
    cat <<'USAGE'
Usage: tac3-partition-installer --disk /dev/sdX --size 20G [--execute]

Default: read-only plan. No partition or filesystem changes occur.

Options:
  --disk PATH     Whole disk device, for example /dev/sda or /dev/nvme0n1.
  --size SIZE     TAC3 partition size, for example 8G, 20G, 100G.
  --execute       Perform the reviewed partition and TAC3 filesystem creation.
  --help          Show this help.

Safety requirements:
  - Linux with lsblk and parted.
  - The target must be a whole disk.
  - No child partition of the target disk may be mounted.
  - mkfs.tac3 must exist before --execute is accepted.
  - The requested size must fit entirely inside a reported free extent.
USAGE
}

fail() { echo "$PROGRAM: $*" >&2; exit 1; }

while (($#)); do
    case "$1" in
        --disk) [[ $# -ge 2 ]] || fail "--disk requires a value"; DISK="$2"; shift 2 ;;
        --size) [[ $# -ge 2 ]] || fail "--size requires a value"; SIZE="$2"; shift 2 ;;
        --execute) EXECUTE=1; shift ;;
        --help|-h) usage; exit 0 ;;
        *) fail "unknown option: $1" ;;
    esac
done

[[ -n "$DISK" ]] || fail "--disk is required"
[[ -n "$SIZE" ]] || fail "--size is required"
[[ -b "$DISK" ]] || fail "$DISK is not a block device"
command -v lsblk >/dev/null 2>&1 || fail "lsblk is required"
command -v parted >/dev/null 2>&1 || fail "parted is required"

TYPE="$(lsblk -dnro TYPE "$DISK")"
[[ "$TYPE" == "disk" ]] || fail "$DISK must identify a whole disk (reported type: $TYPE)"

mapfile -t MOUNTED < <(lsblk -nrpo NAME,MOUNTPOINT "$DISK" | awk 'NF >= 2 && $2 != "" {print $1 " -> " $2}')
if ((${#MOUNTED[@]})); then
    printf '%s\n' "Mounted child partitions were found:" >&2
    printf '  %s\n' "${MOUNTED[@]}" >&2
    fail "run from an environment where the target disk has no mounted child partitions"
fi

if (( EXECUTE )); then
    [[ "$(id -u)" -eq 0 ]] || fail "--execute requires root"
    command -v mkfs.tac3 >/dev/null 2>&1 || fail "mkfs.tac3 is required before partition creation"
fi

# Ask parted for free extents in MiB. We deliberately select only a Free Space
# row; existing partitions are never resized, moved, or reformatted.
FREE_ROWS="$(parted -m -s "$DISK" unit MiB print free 2>/dev/null | awk -F: '$5 == "free" {print $2 ":" $3}')"
[[ -n "$FREE_ROWS" ]] || fail "no unallocated space was reported on $DISK"

# Convert the requested size to bytes with numfmt, then to MiB. GNU numfmt is
# used only for validation/planning; parted performs the final alignment.
command -v numfmt >/dev/null 2>&1 || fail "numfmt is required"
SIZE_BYTES="$(numfmt --from=iec "$SIZE" 2>/dev/null)" || fail "invalid size: $SIZE"
(( SIZE_BYTES > 0 )) || fail "size must be greater than zero"
SIZE_MIB=$(( (SIZE_BYTES + 1048575) / 1048576 ))

SELECTED_START=""
SELECTED_END=""
SELECTED_MIB=""
while IFS=: read -r START END; do
    [[ -n "$START" && -n "$END" ]] || continue
    START_N="${START%MiB}"
    END_N="${END%MiB}"
    AVAILABLE_MIB=$(( ${END_N%.*} - ${START_N%.*} ))
    if (( AVAILABLE_MIB >= SIZE_MIB )); then
        SELECTED_START="$START_N"
        SELECTED_END="$(( ${START_N%.*} + SIZE_MIB ))"
        SELECTED_MIB="$SIZE_MIB"
        break
    fi
done <<< "$FREE_ROWS"

[[ -n "$SELECTED_START" ]] || fail "no single unallocated extent is large enough for $SIZE"

cat <<PLAN
TAC3 partition plan
-------------------
Disk:       $DISK
Requested:  $SIZE
Extent:     ${SELECTED_START}MiB -> ${SELECTED_END}MiB
Allocation: ${SELECTED_MIB}MiB
Operation:  create one TAC3 partition in unallocated space only
Formatting: mkfs.tac3
Execution:  $( (( EXECUTE )) && echo AUTHORIZED || echo DRY-RUN )

The base filesystem and existing partitions are not selected for formatting.
PLAN

if (( ! EXECUTE )); then
    exit 0
fi

BEFORE="$(lsblk -nrpo NAME,TYPE "$DISK" | awk '$2 == "part" {print $1}' | sort)"

parted -s "$DISK" unit MiB mkpart primary "$SELECTED_START" "$SELECTED_END"
partprobe "$DISK" 2>/dev/null || true
udevadm settle 2>/dev/null || true

AFTER="$(lsblk -nrpo NAME,TYPE "$DISK" | awk '$2 == "part" {print $1}' | sort)"
NEW_PART="$(comm -13 <(printf '%s\n' "$BEFORE") <(printf '%s\n' "$AFTER") | head -n 1)"
[[ -n "$NEW_PART" ]] || fail "partition was created but its device node could not be identified"

# Never guess a filesystem tool. TAC3 must provide the TAC3 formatter explicitly.
mkfs.tac3 "$NEW_PART"

cat <<DONE
TAC3 partition created and initialized.
Partition: $NEW_PART
Size:      $SIZE
Filesystem: TAC3

Next stages remain explicit: install TAC3 recovery state, run the startup
integrity/health scan, and register the recovery/boot target.
DONE
