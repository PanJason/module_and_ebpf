CC := gcc
CFLAGS := -Wall -Werror -std=gnu99
all: cat_nonblock

cat_nonblock: cat_nonblock.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f cat_nonblock