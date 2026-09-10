# SPDX-License-Identifier: GPL-2.0
#
# tools/tac3health — userspace TAC3 health monitor (tac3healthctl). Reads the
# in-kernel module's /proc/tac3/{status,health,admin} interface; standalone,
# no engine link required.

CXX      ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2
PREFIX   ?= /usr/local/bin

TARGET := tac3healthctl
SRCS   := tac3healthctl.cpp

.PHONY: all install uninstall clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

install: all
	install -d $(PREFIX)
	install -m 0755 $(TARGET) $(PREFIX)/$(TARGET)
	@echo "tac3healthctl installed to $(PREFIX)/$(TARGET)"
	@echo "optional: install the service unit with 'make install-service'"

# Install the systemd watch service + a state dir for the baseline.
install-service: install
	install -d /var/lib/tac3
	install -m 0644 tac3health.service /etc/systemd/system/tac3health.service
	@echo "service unit installed; enable with:"
	@echo "  systemctl daemon-reload && systemctl enable --now tac3health.service"

uninstall:
	rm -f $(PREFIX)/$(TARGET)

clean:
	rm -f $(TARGET)
