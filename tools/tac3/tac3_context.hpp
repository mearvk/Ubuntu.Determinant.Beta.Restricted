// SPDX-License-Identifier: GPL-2.0
//
// tac3_context.hpp — TAC3 medium-granularity contextual file record.
//
// The 33 fields below are the vital statistics contract. A derived contextual
// signature is available for comparison/indexing, but is not a 34th statistic.
// Same names, IDs, and context areas are explicitly permitted.
//
// Copyright (C) 2026 MEARVK LLC

#pragma once

#include <cstdint>
#include <string>

namespace tac3 {

struct Tac3VitalStats {
    //  1-8: identity and context
    std::string file_name;                 // 1
    std::string file_id;                   // 2
    std::string context_id;                // 3
    std::string context_area;              // 4
    std::string parent_id;                 // 5
    std::uint64_t version = 0;             // 6
    std::uint64_t revision = 0;            // 7
    std::string record_type;               // 8

    //  9-14: time and provenance
    std::uint64_t creation_timestamp = 0;  // 9
    std::uint64_t modification_timestamp = 0; // 10
    std::string owner;                     // 11
    std::string source;                    // 12
    std::string source_id;                 // 13
    std::string location;                  // 14

    // 15-20: filesystem and storage
    std::string device_class;              // 15
    std::string filesystem_type;           // 16
    std::string mount_point;              // 17
    std::uint64_t capacity = 0;            // 18
    std::uint64_t allocated_space = 0;     // 19
    std::uint64_t available_space = 0;     // 20

    // 21-28: activity and pressure
    std::uint64_t read_count = 0;          // 21
    std::uint64_t write_count = 0;         // 22
    std::uint32_t read_pressure = 0;       // 23
    std::uint32_t write_pressure = 0;      // 24
    std::uint32_t wear_level = 0;          // 25
    std::uint32_t health_level = 0;        // 26
    std::uint32_t jarring_shock_level = 0; // 27
    std::uint64_t access_frequency = 0;    // 28

    // 29-33: integrity, administration, relationship, state
    std::string integrity_status;          // 29
    std::string administrative_status;     // 30
    std::string security_trust_status;     // 31
    std::string relationship_context;      // 32
    std::string operational_state;         // 33
};

// Compares the complete 33-stat state. This is intentionally stronger than
// comparing filename or File ID alone.
bool same_vital_state(const Tac3VitalStats& a,
                      const Tac3VitalStats& b) noexcept;

// Stable, derived diagnostic/indexing signature of the 33-stat state.
// It is NOT an additional vital statistic and must not be used as a claim that
// File ID or filename must be globally unique.
std::uint64_t contextual_signature(const Tac3VitalStats& stats) noexcept;

} // namespace tac3
