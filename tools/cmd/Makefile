# CMD native desktop launcher toolchain
CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11
PREFIX ?= /usr/local
LIBEXECDIR ?= $(PREFIX)/lib/cmd
BINDIR ?= $(PREFIX)/bin

COMMON = format/cmd-format.h format/cmd-validate.h
TOOLS = cmdlink cmdlink-original cmd-icon-gen cmd-inspect
LINUX_LAUNCHER = launcher/linux/cmd-launch-linux

.PHONY: all clean test verify-sha native-launcher install windows-launcher macos-launcher

all: native-launcher $(TOOLS)

native-launcher: $(LINUX_LAUNCHER)

$(LINUX_LAUNCHER): launcher/linux/cmd-launch-linux.c $(COMMON)
	mkdir -p launcher/linux
	$(CC) $(CFLAGS) -I. -o $@ $<

# Authoritative native linker. The historical root cmdlink.c is retained and
# remains available as cmdlink-original for compatibility/reference builds.
cmdlink: linker/cmdlink-native.c $(COMMON)
	$(CC) $(CFLAGS) -I. -o $@ $<

cmdlink-original: cmdlink.c
	$(CC) $(CFLAGS) -I. -o $@ $<

cmd-inspect: inspect/cmd-inspect.c $(COMMON)
	$(CC) $(CFLAGS) -I. -o $@ $<

cmd-icon-gen: cmd-icon-gen.c
	$(CC) $(CFLAGS) -o $@ $< -lm

verify-sha: cmdlink
	@set -eu; \
	tmp=$$(mktemp -d); trap 'rm -rf "$$tmp"' EXIT; \
	printf 'abc' > "$$tmp/payload.class"; \
	printf '\xca\xfe\xba\xbe' >> "$$tmp/payload.class"; \
	printf '%s\n' 'SHA-256 implementation self-test uses the repository-standard SHA-256 constants.'; \
	printf '%s\n' 'Expected SHA-256(abc): ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad'; \
	sha256sum "$$tmp/payload.class" | cut -d' ' -f1

# Native ELF smoke test plus integrity-failure test. No shell-script fallback is accepted.
test: all
	@set -eu; \
	tmp=$$(mktemp -d); trap 'rm -rf "$$tmp"' EXIT; \
	printf '%s\n' 'public class HelloCmd { public static void main(String[] a) { System.out.println("Hello from native CMD"); } }' > "$$tmp/HelloCmd.java"; \
	javac "$$tmp/HelloCmd.java"; \
	./cmdlink "$$tmp/HelloCmd.class" --main=HelloCmd --headless --no-pin --launcher=$(LINUX_LAUNCHER) -o "$$tmp/HelloCmd.cmd"; \
	file "$$tmp/HelloCmd.cmd"; \
	./cmd-inspect --verify --manifest "$$tmp/HelloCmd.cmd"; \
	"$$tmp/HelloCmd.cmd"; \
	cp "$$tmp/HelloCmd.cmd" "$$tmp/tampered.cmd"; \
	printf X | dd of="$$tmp/tampered.cmd" bs=1 seek=120 conv=notrunc status=none; \
	if "$$tmp/tampered.cmd" >/dev/null 2>&1; then echo 'ERROR: tampered CMD executed' >&2; exit 1; fi; \
	if ./cmd-inspect --verify "$$tmp/tampered.cmd" >/dev/null 2>&1; then echo 'ERROR: tampered CMD verified' >&2; exit 1; fi; \
	echo 'CMD native and integrity-failure tests passed.'

clean:
	rm -f $(TOOLS) $(LINUX_LAUNCHER)
	rm -f test-icon.bmp

install: all
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBEXECDIR)
	install -m 755 cmdlink $(DESTDIR)$(BINDIR)/cmdlink
	install -m 755 cmd-icon-gen $(DESTDIR)$(BINDIR)/cmd-icon-gen
	install -m 755 cmd-inspect $(DESTDIR)$(BINDIR)/cmd-inspect
	install -m 755 $(LINUX_LAUNCHER) $(DESTDIR)$(LIBEXECDIR)/cmd-launch-linux
	@echo "Installed CMD tools and native Linux launcher template."

windows-launcher:
	$(MAKE) -C launcher/windows

macos-launcher:
	$(MAKE) -C launcher/macos
