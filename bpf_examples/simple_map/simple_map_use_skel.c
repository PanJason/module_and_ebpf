#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "simple_map.skel.h"

static volatile sig_atomic_t exiting;

static void handle_signal(int sig)
{
    exiting = 1;
}

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
    if (level == LIBBPF_DEBUG)
        return 0;

    return vfprintf(stderr, format, args);
}

int main(void)
{
    struct simple_map_bpf *skel = NULL;
    int map_fd = -1;
    __u32 key = 0;
    __u64 value = 0;
    int err = 0;

    libbpf_set_strict_mode(LIBBPF_STRICT_NONE);
    libbpf_set_print(libbpf_print_fn);

    skel = simple_map_bpf__open();
    if (!skel) {
        fprintf(stderr, "failed to open simple_map skeleton\n");
        return 1;
    }

    err = simple_map_bpf__load(skel);
    if (err) {
        fprintf(stderr, "failed to load simple_map skeleton: %s\n", strerror(-err));
        goto cleanup;
    }

    err = simple_map_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "failed to attach simple_map programs: %s\n", strerror(-err));
        goto cleanup;
    }

    map_fd = bpf_map__fd(skel->maps.exec_counter);
    if (map_fd < 0) {
        fprintf(stderr, "failed to get exec_counter map fd: %s\n", strerror(-map_fd));
        err = map_fd;
        goto cleanup;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Loaded simple_map via skeleton and attached tracepoint syscalls:sys_enter_execve.\n");
    printf("Press Ctrl+C to exit.\n");

    while (!exiting) {
        int lookup_err = bpf_map_lookup_elem(map_fd, &key, &value);

        if (lookup_err && errno != ENOENT) {
            fprintf(stderr, "map lookup failed: %s\n", strerror(errno));
            err = lookup_err;
            break;
        }

        if (!lookup_err) {
            printf("execve count: %llu\n", (unsigned long long)value);
        } else {
            printf("execve count: 0\n");
            err = 0;
        }

        fflush(stdout);
        sleep(1);
    }

cleanup:
    simple_map_bpf__destroy(skel);
    return err ? 1 : 0;
}
