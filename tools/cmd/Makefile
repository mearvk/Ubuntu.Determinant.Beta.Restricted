# CMD native desktop launcher toolchain
CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11
PREFIX ?= /usr/local
LIBEXECDIR ?= $(PREFIX)/lib/cmd
BINDIR ?= $(PREFIX)/bin

COMMON = format/cmd-format.h
TOOLS = cmdlink cmd-icon-gen cmd-inspect
LINUX_LAUNCHER = launcher/linux/cmd-launch-linux

.PHONY: all clean test install native-launcher

all: native-launcher $(TOOLS)

native-launcher: $(LINUX_LAUNCHER)

$(LINUX_LAUNCHER): launcher/linux/cmd-launch-linux.c $(COMMON)
	mkdir -p launcher/linux
	$(CC) $(CFLAGS) -I. -o $@ $<

cmdlink: linker/cmdlink.c $(COMMON)
	$(CC) $(CFLAGS) -I. -o $@ $<

cmd-inspect: inspect/cmd-inspect.c $(COMMON)
	$(CC) $(CFLAGS) -I. -o $@ $<

cmd-icon-gen: cmd-icon-gen.c
	$(CC) $(CFLAGS) -o $@ $< -lm

# Build and exercise a real ELF-prefixed .cmd. The test does not silently
# fall back to a shell script: failure to build the native template fails here.
test: all
	@set -eu; \
	tmp=$$(mktemp -d); trap 'rm -rf "$$tmp"' EXIT; \
	printf '%s\n' 'public class HelloCmd { public static void main(String[] a) { System.out.println("Hello from native CMD"); } }' > "$$tmp/HelloCmd.java"; \
	javac "$$tmp/HelloCmd.java"; \
	./cmdlink "$$tmp/HelloCmd.class" --main=HelloCmd --headless --no-pin --launcher=$(LINUX_LAUNCHER) -o "$$tmp/HelloCmd.cmd"; \
	file "$$tmp/HelloCmd.cmd"; \
	./cmd-inspect --verify --manifest "$$tmp/HelloCmd.cmd"; \
	"$$tmp/HelloCmd.cmd"; \
	echo 'CMD native test passed.'

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

# Cross-platform source targets. These are compiled on their native hosts/toolchains;
# cmdlink embeds the resulting PE or Mach-O launcher template exactly as it embeds ELF.
windows-launcher:
	$(MAKE) -C launcher/windows

macos-launcher:
	$(MAKE) -C launcher/macos
