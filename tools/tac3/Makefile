# SPDX-License-Identifier: GPL-2.0
#
# tools/tac3 — userspace TAC3 diagnostic (tac3ctl), built from the portable
# C++ engine plus the 33-stat contextual identity layer. Companion to the
# in-kernel fs/tac3 module.

CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -I.
PREFIX   ?= /usr/local/bin

TARGET := tac3ctl
SRCS   := tac3ctl.cpp tac3.cpp tac3_context.cpp
HDRS   := tac3.hpp tac3_context.hpp

.PHONY: all install uninstall clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

install: all
	install -d $(PREFIX)
	install -m 0755 $(TARGET) $(PREFIX)/$(TARGET)
	@echo "tac3ctl installed to $(PREFIX)/$(TARGET)"

uninstall:
	rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f $(TARGET)
