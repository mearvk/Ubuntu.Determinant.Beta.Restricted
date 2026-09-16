#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT"

make clean >/dev/null 2>&1 || true
make all

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"; make clean >/dev/null 2>&1 || true' EXIT

cat > "$TMP/HelloCmd.java" <<'JAVA'
public class HelloCmd {
    public static void main(String[] args) {
        System.out.println("Hello from native CMD");
    }
}
JAVA

javac "$TMP/HelloCmd.java"
./cmdlink "$TMP/HelloCmd.class" --main=HelloCmd --headless --no-pin --launcher="$ROOT/launcher/linux/cmd-launch-linux" -o "$TMP/HelloCmd.cmd" 2>/dev/null

./cmd-inspect --verify "$TMP/HelloCmd.cmd"
OUTPUT=$("$TMP/HelloCmd.cmd")
[ "$OUTPUT" = "Hello from native CMD" ]

cp "$TMP/HelloCmd.cmd" "$TMP/tampered.cmd"
printf X | dd of="$TMP/tampered.cmd" bs=1 seek=120 conv=notrunc status=none

if "$TMP/tampered.cmd" >/dev/null 2>&1; then
    echo "tampered CMD unexpectedly executed" >&2
    exit 1
fi
if ./cmd-inspect --verify "$TMP/tampered.cmd" >/dev/null 2>&1; then
    echo "tampered CMD unexpectedly verified" >&2
    exit 1
fi

echo "CMD regression test passed."
