#!/usr/bin/env bash
# Build tac3healthctl — the userspace TAC3 health monitor.
# Mirrors tools/tac3/build.sh: prefer the repo's own GCC if present, else cc/c++.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRCDIR="$ROOT/tools/tac3health"
OUT="$SRCDIR/tac3healthctl"

CXX="${MEARVK_GXX:-}"
if [[ -z "$CXX" ]]; then
  for candidate in \
    "$ROOT/tools/gcc/build/gcc/xg++" \
    "$ROOT/tools/gcc/build/gcc/g++" \
    "$ROOT/tools/gcc/gcc-16.2.0/build/gcc/xg++" \
    "$ROOT/tools/gcc/gcc-16.2.0/build/gcc/g++"; do
    if [[ -x "$candidate" ]]; then CXX="$candidate"; break; fi
  done
fi
if [[ -z "$CXX" ]]; then CXX="${CXX:-c++}"; fi

CXXFLAGS=(-std=c++17 -Wall -Wextra -O2)
"$CXX" "${CXXFLAGS[@]}" -o "$OUT" "$SRCDIR/tac3healthctl.cpp"
printf 'built %s with %s\n' "$OUT" "$CXX"
