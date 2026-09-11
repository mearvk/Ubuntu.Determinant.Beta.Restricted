#ifndef TAC3_BOOT_HPP
#define TAC3_BOOT_HPP

#include <cstdint>
#include <string>

namespace tac3 {

// Startup integrity states. These are operational states, not new file-identity
// statistics.
enum class BootIntegrityState : std::uint8_t {
    unknown = 0,
    healthy,
    degraded,
    recovery_required,
    recovery_failed,
};

struct BootIntegrityReport {
    BootIntegrityState state = BootIntegrityState::unknown;
    bool boot_environment_available = false;
    bool tac3_available = false;
    bool recovery_manifest_valid = false;
    bool recovery_state_valid = false;
    bool base_filesystem_discovered = false;
    bool base_filesystem_healthy = false;
    bool rollback_available = false;
    std::string detail;
};

// Conservative startup policy: healthy only when the protected TAC3 recovery
// state is available and the base filesystem has either been verified healthy or
// has not yet been selected as the active target. A failed recovery attempt must
// remain visibly failed and must not be converted into a healthy result.
BootIntegrityState evaluate_boot_integrity(const BootIntegrityReport& report);

const char* boot_integrity_state_name(BootIntegrityState state);

} // namespace tac3

#endif // TAC3_BOOT_HPP
