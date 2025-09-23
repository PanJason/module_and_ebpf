#include <linux/bpf.h>
#include <linux/ptrace.h>
#include <linux/types.h>

#include <bpf/bpf_helpers.h>

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 1);
	__type(key, __u32);
	__type(value, __u64);
} exec_counter SEC(".maps");

SEC("tracepoint/syscalls/sys_enter_execve")
// ctx contains all the event payload
// helper functions such as BPF_CORE_READ can be used to read fields from ctx
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
