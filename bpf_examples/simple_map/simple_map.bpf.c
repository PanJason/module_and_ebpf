#include <linux/bpf.h>
#include <linux/ptrace.h>
#include <linux/types.h>

#include "bpf_helpers.h"

struct bpf_map_def SEC("maps") exec_counter = {
    .type = BPF_MAP_TYPE_HASH,
    .key_size = sizeof(__u32),
    .value_size = sizeof(__u64),
    .max_entries = 1,
};

SEC("tracepoint/syscalls/sys_enter_execve")
int count_exec(void *ctx)
{
    __u32 key = 0;
    __u64 init = 1;
    __u64 *val;

    val = bpf_map_lookup_elem(&exec_counter, &key);
    if (!val) {
        bpf_map_update_elem(&exec_counter, &key, &init, BPF_ANY);
    } else {
        __sync_fetch_and_add(val, 1);
    }

    return 0;
}

char LICENSE[] SEC("license") = "GPL";
