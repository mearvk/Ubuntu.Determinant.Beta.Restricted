#include "tac3_boot.hpp"

namespace tac3 {

BootIntegrityState evaluate_boot_integrity(const BootIntegrityReport& report) {
    if (report.state == BootIntegrityState::recovery_failed)
        return BootIntegrityState::recovery_failed;

    if (!report.boot_environment_available || !report.tac3_available ||
        !report.recovery_manifest_valid || !report.recovery_state_valid)
        return BootIntegrityState::recovery_required;

    if (report.base_filesystem_discovered && !report.base_filesystem_healthy) {
        if (report.rollback_available)
            return BootIntegrityState::recovery_required;
        return BootIntegrityState::degraded;
    }

    return BootIntegrityState::healthy;
}

const char* boot_integrity_state_name(BootIntegrityState state) {
    switch (state) {
    case BootIntegrityState::healthy:
        return "healthy";
    case BootIntegrityState::degraded:
        return "degraded";
    case BootIntegrityState::recovery_required:
        return "recovery-required";
    case BootIntegrityState::recovery_failed:
        return "recovery-failed";
    case BootIntegrityState::unknown:
    default:
        return "unknown";
    }
}

} // namespace tac3
