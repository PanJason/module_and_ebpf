#ifndef __SIMPLE_MAP_BPF_HELPERS_H__
#define __SIMPLE_MAP_BPF_HELPERS_H__

#include <linux/bpf.h>
#include <linux/types.h>

#define SEC(NAME) __attribute__((section(NAME), used))

static void *(*bpf_map_lookup_elem)(void *map, const void *key) =
    (void *)BPF_FUNC_map_lookup_elem;
static int (*bpf_map_update_elem)(void *map, const void *key, const void *value,
                                  __u64 flags) =
    (void *)BPF_FUNC_map_update_elem;
static long (*bpf_trace_printk)(const char *fmt, __u32 fmt_size, ...) =
    (void *)BPF_FUNC_trace_printk;

#endif // __SIMPLE_MAP_BPF_HELPERS_H__
