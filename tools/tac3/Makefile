# SPDX-License-Identifier: GPL-2.0
#
# tools/tac3 — userspace TAC3 diagnostics and backup utilities.

CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -I.
PREFIX   ?= /usr/local/bin

TARGETS := tac3ctl tac3-usb-dump
TAC3CTL_SRCS := tac3ctl.cpp tac3.cpp tac3_context.cpp tac3_boot.cpp
TAC3CTL_HDRS := tac3.hpp tac3_context.hpp tac3_boot.hpp

.PHONY: all install uninstall clean

all: $(TARGETS)

tac3ctl: $(TAC3CTL_SRCS) $(TAC3CTL_HDRS)
	$(CXX) $(CXXFLAGS) -o $@ $(TAC3CTL_SRCS)

tac3-usb-dump: tac3-usb-dump.cpp tac3_format.hpp ../../file-systems/tac3/tac3_format.h
	$(CXX) $(CXXFLAGS) -o $@ tac3-usb-dump.cpp

install: all
	install -d $(PREFIX)
	install -m 0755 tac3ctl $(PREFIX)/tac3ctl
	install -m 0755 tac3-usb-dump $(PREFIX)/tac3-usb-dump
	@echo "TAC3 userspace tools installed to $(PREFIX)"

uninstall:
	rm -f $(PREFIX)/tac3ctl $(PREFIX)/tac3-usb-dump

clean:
	rm -f $(TARGETS)
