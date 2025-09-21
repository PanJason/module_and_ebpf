#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *file_name = "test";
const char *file_content = "Hello, World!\n";

static void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [file_name] [file_content]\n", prog);
	fprintf(stderr,
		"  file_name    Name of the file to create/overwrite (default: %s)\n"
		"  file_content Content written to the file         (default: %s)\n",
		file_name, file_content);
}

int main(int argc, char **argv)
{
	const char *name = file_name;
	const char *content = file_content;
	size_t len;
	ssize_t written;
	int fd;

	if (argc > 3) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (argc >= 2)
		name = argv[1];
	if (argc == 3)
		content = argv[2];

	fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", name, strerror(errno));
		return EXIT_FAILURE;
	}

	len = strlen(content);
	written = write(fd, content, len);
	if (written != (ssize_t)len) {
		fprintf(stderr, "Failed to write to %s: %s\n", name, strerror(errno));
		close(fd);
		return EXIT_FAILURE;
	}

	if (close(fd) < 0) {
		fprintf(stderr, "Failed to close %s: %s\n", name, strerror(errno));
		return EXIT_FAILURE;
	}

	printf("Wrote %zu bytes to %s\n", len, name);
	return EXIT_SUCCESS;
}
