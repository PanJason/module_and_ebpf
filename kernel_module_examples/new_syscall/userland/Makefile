CC = gcc
CFLAGS = -Wall -Werror -std=gnu99

all: invoke_new_syscall

invoke_new_syscall: invoke_new_syscall.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f invoke_new_syscall
