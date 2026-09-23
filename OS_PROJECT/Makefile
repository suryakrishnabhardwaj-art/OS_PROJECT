CC = gcc
CFLAGS = -Wall -Wextra

all: os_manager process exec_module ipc monitor file_manager

os_manager:
	$(CC) $(CFLAGS) src/main.c -o output/os_manager

process:
	$(CC) $(CFLAGS) src/process.c -o output/process

exec_module:
	$(CC) $(CFLAGS) src/exec_module.c -o output/exec_module

ipc:
	$(CC) $(CFLAGS) src/ipc.c -o output/ipc

monitor:
	$(CC) $(CFLAGS) src/monitor.c -o output/monitor

file_manager:
	$(CC) $(CFLAGS) src/file_manager.c -o output/file_manager

clean:
	rm -f output/os_manager output/process output/exec_module output/ipc output/monitor output/file_manager
