#!/usr/bin/env bash
# Install tac3healthctl (userspace TAC3 health monitor). Mirrors tools/tac3/install.sh.
set -euo pipefail
PREFIX="${PREFIX:-/usr/local/bin}"
CXX="${CXX:-c++}"
HERE="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$PREFIX"
"$CXX" -O2 -Wall -Wextra -std=c++17 -o "$HERE/tac3healthctl" \
  "$HERE/tac3healthctl.cpp"
install -m 0755 "$HERE/tac3healthctl" "$PREFIX/tac3healthctl"
echo "tac3healthctl installed to $PREFIX/tac3healthctl"

# State dir for the alteration baseline (best-effort).
mkdir -p /var/lib/tac3 2>/dev/null || true
echo "run 'tac3healthctl baseline' once a TAC3 volume is mounted to enable"
echo "alteration detection; see tac3health.service for always-on monitoring."
