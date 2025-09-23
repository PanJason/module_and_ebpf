# simple_map eBPF example

This example shows how to use an eBPF hash map to count how many times the
`execve(2)` system call is invoked. The program keeps a single counter in a map
and updates it any time the kernel hits the `sys_enter_execve` tracepoint.

## Building the artifacts

```
cd $(git rev-parse --show-toplevel)/bpf_examples/simple_map
make
```

The Makefile produces the BPF object (`simple_map.bpf.o`) alongside two
libbpf-based loader binaries (`simple_map_user` and `simple_map_use_skel`). You will need the libbpf
development headers and libraries installed; distributions usually ship them as
`libbpf-dev`, `libbpf-devel`, or similar. The build uses `pkg-config` when it is
available to locate the headers and libraries.

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

## Loading with libbpf

Instead of relying on `bpftool`, you can use the provided `simple_map_user`
binary. It opens the compiled object file with libbpf, loads and verifies the
program, attaches it to the `syscalls:sys_enter_execve` tracepoint, and
periodically reads the `exec_counter` map.

```
sudo ./simple_map_user
```

The loader prints the current execve count once per second. A second variant,
`simple_map_use_skel`, demonstrates the auto-generated bpftool skeleton:

```
sudo ./simple_map_use_skel
```

Both helpers behave the same—trigger a few
`execve` calls (for instance by running `ls`, `sleep`, or another short-lived
program in a different terminal) and watch the counter increase. Press `Ctrl+C`
to detach the tracepoint hook and exit cleanly.

## Notes

- The example depends only on the kernel headers shipped with your
  distribution for the BPF program itself, but building `simple_map_user`
  requires libbpf headers and libraries.
- If your system mounts bpffs elsewhere, adjust the pin paths accordingly.
