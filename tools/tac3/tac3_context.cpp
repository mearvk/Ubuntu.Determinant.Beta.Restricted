// SPDX-License-Identifier: GPL-2.0
//
// tac3_context.cpp — TAC3 medium-granularity contextual identity.
//
// Copyright (C) 2026 MEARVK LLC

#include "tac3_context.hpp"

#include <string_view>

namespace tac3 {
namespace {

class Fnv1a64 {
public:
    void add(std::string_view s) noexcept {
        for (unsigned char c : s) {
            value_ ^= static_cast<std::uint64_t>(c);
            value_ *= 1099511628211ULL;
        }
    }

    void add_u64(std::uint64_t v) noexcept {
        for (unsigned i = 0; i < 8; ++i) {
            value_ ^= (v >> (i * 8)) & 0xffULL;
            value_ *= 1099511628211ULL;
        }
    }

    void add_u32(std::uint32_t v) noexcept {
        add_u64(static_cast<std::uint64_t>(v));
    }

    std::uint64_t value() const noexcept { return value_; }

private:
    std::uint64_t value_ = 14695981039346656037ULL;
};

bool equal(const Tac3VitalStats& a, const Tac3VitalStats& b) noexcept {
    return a.file_name == b.file_name &&
           a.file_id == b.file_id &&
           a.context_id == b.context_id &&
           a.context_area == b.context_area &&
           a.parent_id == b.parent_id &&
           a.version == b.version &&
           a.revision == b.revision &&
           a.record_type == b.record_type &&
           a.creation_timestamp == b.creation_timestamp &&
           a.modification_timestamp == b.modification_timestamp &&
           a.owner == b.owner &&
           a.source == b.source &&
           a.source_id == b.source_id &&
           a.location == b.location &&
           a.device_class == b.device_class &&
           a.filesystem_type == b.filesystem_type &&
           a.mount_point == b.mount_point &&
           a.capacity == b.capacity &&
           a.allocated_space == b.allocated_space &&
           a.available_space == b.available_space &&
           a.read_count == b.read_count &&
           a.write_count == b.write_count &&
           a.read_pressure == b.read_pressure &&
           a.write_pressure == b.write_pressure &&
           a.wear_level == b.wear_level &&
           a.health_level == b.health_level &&
           a.jarring_shock_level == b.jarring_shock_level &&
           a.access_frequency == b.access_frequency &&
           a.integrity_status == b.integrity_status &&
           a.administrative_status == b.administrative_status &&
           a.security_trust_status == b.security_trust_status &&
           a.relationship_context == b.relationship_context &&
           a.operational_state == b.operational_state;
}

void add_field(Fnv1a64& h, std::string_view value) noexcept {
    h.add_u64(static_cast<std::uint64_t>(value.size()));
    h.add(value);
}

} // namespace

bool same_vital_state(const Tac3VitalStats& a,
                      const Tac3VitalStats& b) noexcept {
    return equal(a, b);
}

std::uint64_t contextual_signature(const Tac3VitalStats& s) noexcept {
    Fnv1a64 h;

    // Delimit every field with its type/length so concatenation cannot make
    // ambiguous byte streams. All 33 vital statistics participate.
    add_field(h, s.file_name);                 // 1
    add_field(h, s.file_id);                   // 2
    add_field(h, s.context_id);                // 3
    add_field(h, s.context_area);              // 4
    add_field(h, s.parent_id);                 // 5
    h.add_u64(s.version);                      // 6
    h.add_u64(s.revision);                     // 7
    add_field(h, s.record_type);               // 8
    h.add_u64(s.creation_timestamp);           // 9
    h.add_u64(s.modification_timestamp);       // 10
    add_field(h, s.owner);                     // 11
    add_field(h, s.source);                    // 12
    add_field(h, s.source_id);                 // 13
    add_field(h, s.location);                   // 14
    add_field(h, s.device_class);              // 15
    add_field(h, s.filesystem_type);           // 16
    add_field(h, s.mount_point);               // 17
    h.add_u64(s.capacity);                     // 18
    h.add_u64(s.allocated_space);              // 19
    h.add_u64(s.available_space);              // 20
    h.add_u64(s.read_count);                   // 21
    h.add_u64(s.write_count);                  // 22
    h.add_u32(s.read_pressure);                // 23
    h.add_u32(s.write_pressure);               // 24
    h.add_u32(s.wear_level);                   // 25
    h.add_u32(s.health_level);                 // 26
    h.add_u32(s.jarring_shock_level);          // 27
    h.add_u64(s.access_frequency);             // 28
    add_field(h, s.integrity_status);          // 29
    add_field(h, s.administrative_status);     // 30
    add_field(h, s.security_trust_status);     // 31
    add_field(h, s.relationship_context);      // 32
    add_field(h, s.operational_state);         // 33

    return h.value();
}

} // namespace tac3
