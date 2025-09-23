BPF_CLANG ?= clang
USER_CC ?= cc
BPFTOOL ?= bpftool

# Target: BPF bytecode compiled by clang
# __BPF_TRACING__: for BPF tracing programs so tracepoint helpers are available
BPF_CFLAGS ?= -O2 -g -target bpf -D__BPF_TRACING__ -Wall

# Userspace loader settings (libbpf)
LIBBPF_CFLAGS ?= $(shell pkg-config --cflags libbpf 2>/dev/null)
LIBBPF_LDLIBS ?= $(shell pkg-config --libs libbpf 2>/dev/null)
USER_CFLAGS ?= -O2 -g -Wall -Wextra $(LIBBPF_CFLAGS)
USER_LDLIBS ?= $(if $(LIBBPF_LDLIBS),$(LIBBPF_LDLIBS),-lbpf -lelf -lz)

BPF_OBJ := simple_map.bpf.o
SKELETON := simple_map.skel.h
USER_BINS := simple_map_user simple_map_use_skel

.PHONY: all clean

all: $(BPF_OBJ) $(SKELETON) $(USER_BINS)


$(BPF_OBJ): simple_map.bpf.c
	$(BPF_CLANG) $(BPF_CFLAGS) -I. -c $< -o $@

$(SKELETON): $(BPF_OBJ)
	$(BPFTOOL) gen skeleton $< > $@

simple_map_user: simple_map_user.c
	$(USER_CC) $(USER_CFLAGS) $< -o $@ $(USER_LDLIBS)

simple_map_use_skel: simple_map_use_skel.c $(SKELETON)
	$(USER_CC) $(USER_CFLAGS) $< -o $@ $(USER_LDLIBS)

clean:
	rm -f $(BPF_OBJ) $(USER_BINS) $(SKELETON)
