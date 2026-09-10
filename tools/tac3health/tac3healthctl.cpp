// SPDX-License-Identifier: GPL-2.0
//
// tac3healthctl.cpp — userspace TAC3 health monitor.
//
// tac3healthctl installs on the OS and monitors a live TAC3 filesystem for
// HEALTH, ERRORS, and ALTERATIONS by reading the module's authoritative
// interface at /proc/tac3/{status,health,admin}. It is the always-on companion
// to tac3ctl (the offline model/simulation diagnostic): tac3ctl reasons about
// the model without a disk; tac3healthctl watches the real mount.
//
// What it checks
//   * HEALTH     — file-table health, per-layer disk_health, layer state
//                  (green/white/yellow, no red-alarm — matching fs/tac3).
//   * ERRORS     — per-layer error indications and non-GREEN states; pressure
//                  crossing a configurable attention threshold.
//   * ALTERATION — drift from a saved baseline: admin_table_revision,
//                  multitude, device class, tech_id, and the set of layers.
//                  An unexpected change to these is reported as an alteration.
//
// Modes
//   tac3healthctl [check]         one-shot assessment; exit code reflects worst
//                                 state (0 green, 1 white, 2 yellow, 3 no mount).
//   tac3healthctl watch [--interval S]   poll forever, print on state change.
//   tac3healthctl baseline        snapshot the current admin/topology facts to
//                                 the baseline file (for alteration detection).
//   tac3healthctl help
//
// Options
//   --proc DIR         proc directory (default /proc/tac3).
//   --baseline FILE    baseline path (default /var/lib/tac3/baseline).
//   --pressure P       attention threshold for avg pressure, per-mille (def 800).
//   --interval S       poll seconds for watch mode (default 5).
//   --once             in watch mode, print the first assessment then exit.
//
// No person data. TAC3 Table 3 holds administrative facts + opaque operator
// values only; this tool reports file-system health, never any human attribute.
//
// Copyright (C) 2026 MEARVK LLC
// Author: Maximilian Eric Alexander Rupplin von Keffikon
//
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace {

// Worst-state ordering mirrors the fs/tac3 green/white/yellow model, plus a
// "no mount" sentinel above yellow for scripting clarity.
enum class Level { Green = 0, White = 1, Yellow = 2, NoMount = 3 };

const char* level_name(Level l) {
    switch (l) {
    case Level::Green:   return "GREEN";
    case Level::White:   return "WHITE";
    case Level::Yellow:  return "YELLOW";
    default:             return "NO-MOUNT";
    }
}
Level worst(Level a, Level b) { return (int)a >= (int)b ? a : b; }

Level parse_state(const std::string& s) {
    if (s == "GREEN")  return Level::Green;
    if (s == "WHITE")  return Level::White;
    if (s == "YELLOW") return Level::Yellow;
    return Level::White; // unknown token -> informational
}

struct Options {
    std::string proc_dir  = "/proc/tac3";
    std::string baseline  = "/var/lib/tac3/baseline";
    unsigned    pressure  = 800;   // per-mille attention threshold
    unsigned    interval  = 5;     // watch poll seconds
    bool        once      = false;
};

// ---- one layer parsed from /proc/tac3/health -----------------------------
struct Layer {
    unsigned    index = 0;
    Level       state = Level::Green;
    unsigned long long reads=0, writes=0, read_heat=0, write_wear=0, jarring=0;
    unsigned    quality = 1000, pressure = 0, health = 1000;
};

// ---- topology/admin facts used for alteration detection ------------------
struct Facts {
    long long tech_id      = -1;
    int       multitude    = -1;
    int       device_class = -1;
    long long admin_rev    = -1;
    int       layer_count  = -1;
    bool ok() const { return multitude >= 0; }
    std::string serialize() const {
        std::ostringstream o;
        o << "tech_id="      << tech_id      << "\n"
          << "multitude="    << multitude    << "\n"
          << "device_class=" << device_class << "\n"
          << "admin_rev="    << admin_rev    << "\n"
          << "layer_count="  << layer_count  << "\n";
        return o.str();
    }
};

std::string read_file(const std::string& path, bool* ok) {
    std::ifstream f(path);
    if (!f) { if (ok) *ok = false; return {}; }
    std::ostringstream ss; ss << f.rdbuf();
    if (ok) *ok = true;
    return ss.str();
}

// value after the last ':' on a "label : value" line, trimmed.
std::string after_colon(const std::string& line) {
    auto p = line.rfind(':');
    if (p == std::string::npos) return {};
    std::string v = line.substr(p + 1);
    size_t a = v.find_first_not_of(" \t");
    size_t b = v.find_last_not_of(" \t\r\n");
    if (a == std::string::npos) return {};
    return v.substr(a, b - a + 1);
}

// "123/1000 (GREEN)" or "123/1000" -> integer 123.
unsigned first_uint(const std::string& s) {
    unsigned v = 0; bool seen = false;
    for (char c : s) {
        if (c >= '0' && c <= '9') { v = v*10 + (c - '0'); seen = true; }
        else if (seen) break;
    }
    return v;
}

// ---- parsers -------------------------------------------------------------
bool parse_status_admin(const Options& o, Facts& f, bool& mounted) {
    bool ok = false;
    std::string status = read_file(o.proc_dir + "/status", &ok);
    if (!ok) return false;
    mounted = status.find("(none mounted)") == std::string::npos;
    if (!mounted) return true;

    std::istringstream in(status); std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("multitude", 0) == 0)
            f.multitude = (int)first_uint(after_colon(line));
        else if (line.rfind("device speed ceiling", 0) == 0) {
            auto v = after_colon(line);            // "14000 MB/s (class 6)"
            auto p = v.rfind("class ");
            if (p != std::string::npos) f.device_class = (int)first_uint(v.substr(p));
        }
    }
    // admin file for tech_id + revision (best-effort; may be root-only)
    bool aok = false;
    std::string admin = read_file(o.proc_dir + "/admin", &aok);
    if (aok) {
        std::istringstream ain(admin); std::string al;
        while (std::getline(ain, al)) {
            if (al.rfind("tech_id", 0) == 0)
                f.tech_id = (long long)first_uint(after_colon(al));
            else if (al.rfind("admin_table_revision", 0) == 0)
                f.admin_rev = (long long)first_uint(after_colon(al));
            else if (al.rfind("table_multitude", 0) == 0 && f.multitude < 0)
                f.multitude = (int)first_uint(after_colon(al));
        }
    }
    return true;
}

bool parse_health(const Options& o, std::vector<Layer>& layers) {
    bool ok = false;
    std::string health = read_file(o.proc_dir + "/health", &ok);
    if (!ok) return false;
    if (health.find("no tac3 instance") != std::string::npos) return true;

    std::istringstream in(health); std::string line;
    while (std::getline(in, line)) {
        // skip header / empties
        if (line.empty() || line.rfind("layer", 0) == 0) continue;
        std::istringstream ls(line);
        Layer L; std::string state;
        if (!(ls >> L.index >> state >> L.reads >> L.writes >> L.read_heat
                 >> L.write_wear >> L.jarring >> L.quality >> L.pressure
                 >> L.health))
            continue;
        L.state = parse_state(state);
        layers.push_back(L);
    }
    return true;
}

// ---- baseline persistence -----------------------------------------------
Facts load_baseline(const std::string& path, bool* found) {
    Facts f; bool ok = false;
    std::string s = read_file(path, &ok);
    if (!ok) { if (found) *found = false; return f; }
    std::istringstream in(s); std::string line;
    while (std::getline(in, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq), v = line.substr(eq + 1);
        long long iv = std::atoll(v.c_str());
        if (k == "tech_id") f.tech_id = iv;
        else if (k == "multitude") f.multitude = (int)iv;
        else if (k == "device_class") f.device_class = (int)iv;
        else if (k == "admin_rev") f.admin_rev = iv;
        else if (k == "layer_count") f.layer_count = (int)iv;
    }
    if (found) *found = true;
    return f;
}

bool save_baseline(const std::string& path, const Facts& f) {
    // ensure parent dir exists (best-effort, mode 0755)
    auto slash = path.rfind('/');
    if (slash != std::string::npos && slash > 0) {
        std::string dir = path.substr(0, slash);
        std::string acc;
        std::istringstream ds(dir); std::string part;
        if (dir[0] == '/') acc = "";
        std::vector<std::string> parts;
        std::stringstream ss(dir); std::string p;
        while (std::getline(ss, p, '/')) if (!p.empty()) parts.push_back(p);
        std::string cur = (dir[0] == '/') ? "" : ".";
        for (auto& pp : parts) { cur += "/" + pp; mkdir(cur.c_str(), 0755); }
    }
    std::ofstream out(path);
    if (!out) return false;
    out << f.serialize();
    return true;
}

// compare facts; append human-readable alteration lines. returns true if any.
bool diff_facts(const Facts& base, const Facts& now, std::vector<std::string>& out) {
    bool changed = false;
    auto chk = [&](const char* name, long long b, long long n) {
        if (b >= 0 && b != n) {
            out.push_back(std::string("ALTERATION: ") + name + " changed "
                          + std::to_string(b) + " -> " + std::to_string(n));
            changed = true;
        }
    };
    chk("tech_id",      base.tech_id,      now.tech_id);
    chk("multitude",    base.multitude,    now.multitude);
    chk("device_class", base.device_class, now.device_class);
    chk("layer_count",  base.layer_count,  now.layer_count);
    // admin_table_revision going BACKWARD is an alteration; forward is normal.
    if (base.admin_rev >= 0 && now.admin_rev >= 0 && now.admin_rev < base.admin_rev) {
        out.push_back("ALTERATION: admin_table_revision regressed "
                      + std::to_string(base.admin_rev) + " -> "
                      + std::to_string(now.admin_rev));
        changed = true;
    }
    return changed;
}

// ---- one assessment ------------------------------------------------------
struct Report {
    Level level = Level::Green;
    bool  mounted = false;
    std::vector<std::string> notes;
};

Report assess(const Options& o) {
    Report r;
    Facts now;
    bool mounted = false;
    if (!parse_status_admin(o, now, mounted)) {
        r.level = Level::NoMount;
        r.notes.push_back("cannot read " + o.proc_dir + "/status "
                          "(module not loaded, or insufficient privilege)");
        return r;
    }
    r.mounted = mounted;
    if (!mounted) {
        r.level = Level::NoMount;
        r.notes.push_back("no TAC3 instance is currently mounted");
        return r;
    }

    std::vector<Layer> layers;
    parse_health(o, layers);
    now.layer_count = (int)layers.size();

    // HEALTH + ERRORS
    for (const auto& L : layers) {
        r.level = worst(r.level, L.state);
        if (L.state == Level::Yellow)
            r.notes.push_back("ERROR/ATTENTION: layer " + std::to_string(L.index)
                + " state YELLOW (health " + std::to_string(L.health) + "/1000)");
        else if (L.state == Level::White)
            r.notes.push_back("note: layer " + std::to_string(L.index)
                + " state WHITE (health " + std::to_string(L.health) + "/1000)");
        if (L.pressure >= o.pressure) {
            r.level = worst(r.level, Level::White);
            r.notes.push_back("pressure: layer " + std::to_string(L.index)
                + " avg pressure " + std::to_string(L.pressure)
                + "/1000 >= threshold " + std::to_string(o.pressure));
        }
    }

    // ALTERATION vs baseline
    bool have_base = false;
    Facts base = load_baseline(o.baseline, &have_base);
    if (have_base) {
        std::vector<std::string> alts;
        if (diff_facts(base, now, alts)) {
            r.level = worst(r.level, Level::Yellow); // alteration warrants attention
            for (auto& a : alts) r.notes.push_back(a);
        }
    } else {
        r.notes.push_back("no baseline saved; run 'tac3healthctl baseline' to "
                          "enable alteration detection");
    }

    if (r.notes.empty())
        r.notes.push_back("all layers GREEN; no errors or alterations detected");
    return r;
}

void print_report(const Report& r) {
    std::time_t t = std::time(nullptr);
    char ts[32]; std::strftime(ts, sizeof ts, "%Y-%m-%dT%H:%M:%S", std::gmtime(&t));
    std::printf("[%sZ] TAC3 health: %s\n", ts, level_name(r.level));
    for (const auto& n : r.notes) std::printf("  %s\n", n.c_str());
}

void usage() {
    std::printf(
      "tac3healthctl — TAC3 filesystem health monitor (reads /proc/tac3)\n\n"
      "Commands:\n"
      "  check                 One-shot health/error/alteration assessment (default).\n"
      "  watch                 Poll continuously; print on state change.\n"
      "  baseline              Snapshot admin/topology facts for alteration detection.\n"
      "  help                  This help.\n\n"
      "Options:\n"
      "  --proc DIR            proc directory (default /proc/tac3)\n"
      "  --baseline FILE       baseline path (default /var/lib/tac3/baseline)\n"
      "  --pressure P          avg-pressure attention threshold, per-mille (default 800)\n"
      "  --interval S          watch poll seconds (default 5)\n"
      "  --once                watch: print one assessment then exit\n\n"
      "Exit codes (check): 0 GREEN, 1 WHITE, 2 YELLOW, 3 NO-MOUNT/unreadable.\n");
}

} // namespace

int main(int argc, char** argv) {
    Options o;
    std::string cmd = (argc > 1 && argv[1][0] != '-') ? argv[1] : "check";
    int start = (argc > 1 && argv[1][0] != '-') ? 2 : 1;
    for (int i = start; i < argc; ++i) {
        std::string a = argv[i];
        const char* v = (i + 1 < argc) ? argv[i + 1] : nullptr;
        if (a == "--proc" && v)          { o.proc_dir = v; ++i; }
        else if (a == "--baseline" && v) { o.baseline = v; ++i; }
        else if (a == "--pressure" && v) { o.pressure = (unsigned)std::atoi(v); ++i; }
        else if (a == "--interval" && v) { o.interval = (unsigned)std::atoi(v); ++i; }
        else if (a == "--once")          { o.once = true; }
        else if (a == "help" || a == "-h" || a == "--help") { usage(); return 0; }
        else { std::fprintf(stderr, "tac3healthctl: unknown option '%s'\n", a.c_str()); return 4; }
    }

    if (cmd == "help") { usage(); return 0; }

    if (cmd == "baseline") {
        Facts now; bool mounted = false;
        if (!parse_status_admin(o, now, mounted) || !mounted) {
            std::fprintf(stderr, "tac3healthctl: no mounted TAC3 to baseline\n");
            return 3;
        }
        std::vector<Layer> layers; parse_health(o, layers);
        now.layer_count = (int)layers.size();
        if (!save_baseline(o.baseline, now)) {
            std::fprintf(stderr, "tac3healthctl: cannot write baseline '%s'\n",
                         o.baseline.c_str());
            return 4;
        }
        std::printf("baseline saved to %s (multitude=%d device_class=%d layers=%d rev=%lld)\n",
                    o.baseline.c_str(), now.multitude, now.device_class,
                    now.layer_count, now.admin_rev);
        return 0;
    }

    if (cmd == "watch") {
        Level last = (Level)-1;
        for (;;) {
            Report r = assess(o);
            if (r.level != last) { print_report(r); last = r.level; }
            if (o.once) return (int)r.level;
            sleep(o.interval ? o.interval : 5);
        }
    }

    if (cmd == "check") {
        Report r = assess(o);
        print_report(r);
        return (int)r.level;
    }

    std::fprintf(stderr, "tac3healthctl: unknown command '%s' (try 'help')\n", cmd.c_str());
    return 4;
}
