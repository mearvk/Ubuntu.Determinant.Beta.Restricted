# CMD native desktop launcher toolchain
CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11
PREFIX ?= /usr/local
LIBEXECDIR ?= $(PREFIX)/lib/cmd
BINDIR ?= $(PREFIX)/bin
APPLICATIONSDIR ?= $(PREFIX)/share/applications
ICONDIR ?= $(PREFIX)/share/icons/hicolor/24x24/apps
DESKTOP_DIR ?= $(HOME)/Desktop
DESKTOP_FILE = CMD.desktop
DESKTOP_TEMPLATE = desktop/linux/cmd.desktop
DESKTOP_INSTALLED = $(APPLICATIONSDIR)/cmd.desktop
DESKTOP_ICON = $(ICONDIR)/cmd.bmp

COMMON = format/cmd-format.h format/cmd-validate.h
TOOLS = cmdlink cmdlink-original cmd-icon-gen cmd-inspect
LINUX_LAUNCHER = launcher/linux/cmd-launch-linux

.PHONY: all clean test verify-sha native-launcher install install-desktop windows-launcher macos-launcher

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
	printf abc | sha256sum | grep -q '^ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad  -$$'; \
	echo 'SHA-256 self-test passed.'

# Native ELF smoke test plus integrity-failure test. No shell-script fallback is accepted.
test: all verify-sha
	@set -eu; \
	tmp=$$(mktemp -d); trap 'rm -rf "$$tmp"' EXIT; \
	printf '%s\n' 'public class HelloCmd { public static void main(String[] a) { System.out.println("Hello from native CMD"); } }' > "$$tmp/HelloCmd.java"; \
	javac "$$tmp/HelloCmd.java"; \
	./cmdlink "$$tmp/HelloCmd.class" --main=HelloCmd --headless --no-pin --launcher=$(LINUX_LAUNCHER) -o "$$tmp/HelloCmd.cmd"; \
	file "$$tmp/HelloCmd.cmd"; \
	./cmd-inspect --verify --manifest "$$tmp/HelloCmd.cmd"; \
	"$$tmp/HelloCmd.cmd"; \
	cp "$$tmp/HelloCmd.cmd" "$$tmp/tampered.cmd"; \
	python3 -c 'import struct,sys; p=sys.argv[1]; b=bytearray(open(p,"rb").read()); h=b.find(struct.pack("<I",0x434D4428)); assert h>=0; o=struct.unpack_from("<I",b,h+0x18)[0]; n=struct.unpack_from("<I",b,h+0x1c)[0]; assert n>0; b[h+o+n//2]^=1; open(p,"wb").write(b)' "$$tmp/tampered.cmd"; \
	if "$$tmp/tampered.cmd" >/dev/null 2>&1; then echo 'ERROR: tampered CMD executed' >&2; exit 1; fi; \
	if ./cmd-inspect --verify "$$tmp/tampered.cmd" >/dev/null 2>&1; then echo 'ERROR: tampered CMD verified' >&2; exit 1; fi; \
	echo 'CMD native and integrity-failure tests passed.'

clean:
	rm -f $(TOOLS) $(LINUX_LAUNCHER)
	rm -f test-icon.bmp

# Install the generated CMD icon, a system desktop entry, and (for a normal
# user installation) a clickable CMD shortcut on the user's Desktop.
install: all install-desktop
	install -d $(DESTDIR)$(BINDIR) $(DESTDIR)$(LIBEXECDIR)
	install -m 755 cmdlink $(DESTDIR)$(BINDIR)/cmdlink
	install -m 755 cmd-icon-gen $(DESTDIR)$(BINDIR)/cmd-icon-gen
	install -m 755 cmd-inspect $(DESTDIR)$(BINDIR)/cmd-inspect
	install -m 755 $(LINUX_LAUNCHER) $(DESTDIR)$(LIBEXECDIR)/cmd-launch-linux
	@echo "Installed CMD tools, native Linux launcher, icon, and desktop integration."

install-desktop: cmd-icon-gen
	@set -eu; \
	install -d "$(DESTDIR)$(ICONDIR)" "$(DESTDIR)$(APPLICATIONSDIR)"; \
	./cmd-icon-gen -o "$(DESTDIR)$(DESKTOP_ICON)"; \
	sed -e 's|%CMDLINK%|$(BINDIR)/cmdlink|g' -e 's|%CMD_ICON%|$(DESKTOP_ICON)|g' \
		"$(DESKTOP_TEMPLATE)" > "$(DESTDIR)$(DESKTOP_INSTALLED)"; \
	chmod 755 "$(DESTDIR)$(DESKTOP_INSTALLED)"; \
	if [ -n "$(DESKTOP_DIR)" ]; then \
		install -d "$(DESKTOP_DIR)"; \
		sed -e 's|%CMDLINK%|$(BINDIR)/cmdlink|g' -e 's|%CMD_ICON%|$(DESKTOP_ICON)|g' \
			"$(DESKTOP_TEMPLATE)" > "$(DESKTOP_DIR)/$(DESKTOP_FILE)"; \
		chmod 755 "$(DESKTOP_DIR)/$(DESKTOP_FILE)"; \
		echo "Installed CMD desktop icon shortcut: $(DESKTOP_DIR)/$(DESKTOP_FILE)"; \
	fi

windows-launcher:
	$(MAKE) -C launcher/windows

macos-launcher:
	$(MAKE) -C launcher/macos
