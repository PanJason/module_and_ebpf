#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int main(int argc, char **argv)
{
    const char *obj_path = "simple_map.bpf.o";
    const char *prog_name = "count_exec";
    const char *tp_category = "syscalls";
    const char *tp_name = "sys_enter_execve";
    struct bpf_object *obj = NULL;
    struct bpf_program *prog = NULL;
    struct bpf_link *link = NULL;
    struct bpf_map *map = NULL;
    int map_fd = -1;
    __u32 key = 0;
    __u64 value = 0;
    int err = 0;

    if (argc > 1)
        obj_path = argv[1];

    libbpf_set_strict_mode(LIBBPF_STRICT_NONE);
    libbpf_set_print(libbpf_print_fn);

    obj = bpf_object__open_file(obj_path, NULL);
    err = libbpf_get_error(obj);
    if (err) {
        fprintf(stderr, "failed to open BPF object %s: %s\n", obj_path, strerror(-err));
        return 1;
    }

    err = bpf_object__load(obj);
    if (err) {
        fprintf(stderr, "failed to load BPF object: %s\n", strerror(-err));
        goto cleanup;
    }

    prog = bpf_object__find_program_by_name(obj, prog_name);
    if (!prog) {
        fprintf(stderr, "cannot find program '%s' in object\n", prog_name);
        goto cleanup;
    }

    link = bpf_program__attach_tracepoint(prog, tp_category, tp_name);
    err = libbpf_get_error(link);
    if (err) {
        link = NULL;
        fprintf(stderr, "failed to attach tracepoint %s:%s: %s\n", tp_category, tp_name, strerror(-err));
        goto cleanup;
    }

    map = bpf_object__find_map_by_name(obj, "exec_counter");
    if (!map) {
        fprintf(stderr, "cannot find map 'exec_counter' in object\n");
        goto cleanup;
    }

    map_fd = bpf_map__fd(map);
    if (map_fd < 0) {
        fprintf(stderr, "failed to get map fd: %s\n", strerror(-map_fd));
        goto cleanup;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Loaded simple_map program via libbpf and attached to %s:%s.\n", tp_category, tp_name);
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
    if (link)
        bpf_link__destroy(link);
    if (obj)
        bpf_object__close(obj);

    return err ? 1 : 0;
}
