CC ?= cl
CFLAGS ?= /O2 /W4
OUT ?= cmd-launch-windows.exe

all:
	$(CC) $(CFLAGS) cmd-launch-windows.c /Fe:$(OUT)

clean:
	del /Q $(OUT) 2>NUL || exit 0
