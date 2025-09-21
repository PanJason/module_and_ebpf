# simple_map eBPF example

This example shows how to use an eBPF hash map to count how many times the
`execve(2)` system call is invoked. The program keeps a single counter in a map
and updates it any time the kernel hits the `sys_enter_execve` tracepoint.

## Building the BPF object

```
cd $(git rev-parse --show-toplevel)/bpf_examples/simple_map
make
```

The Makefile produces `simple_map.bpf.o`, which can be loaded with common
userspace tools such as `bpftool`.

## Loading with bpftool

```
sudo bpftool prog load simple_map.bpf.o /sys/fs/bpf/simple_map_prog \
  type tracepoint
sudo bpftool prog attach pinned /sys/fs/bpf/simple_map_prog \
  tracepoint syscalls:sys_enter_execve
```

`bpftool` automatically pins any maps under the same directory. After running
some commands that trigger `execve`, you can inspect the counter:

```
sudo bpftool map dump pinned /sys/fs/bpf/simple_map_prog_map_0
```

Finally, detach and remove the program when you are done:

```
sudo bpftool prog detach pinned /sys/fs/bpf/simple_map_prog \
  tracepoint syscalls:sys_enter_execve
sudo rm /sys/fs/bpf/simple_map_prog
```

## Notes

- The example depends only on the kernel headers shipped with your
  distribution; no userspace helper library is required.
- If your system mounts bpffs elsewhere, adjust the pin paths accordingly.
