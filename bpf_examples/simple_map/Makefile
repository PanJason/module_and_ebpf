BPF_CLANG ?= clang
BPF_CFLAGS ?= -O2 -g -target bpf -D__BPF_TRACING__ -Wall

BPF_OBJ := simple_map.bpf.o

.PHONY: all clean

all: $(BPF_OBJ)

$(BPF_OBJ): simple_map.bpf.c bpf_helpers.h
	$(BPF_CLANG) $(BPF_CFLAGS) -I. -c $< -o $@

clean:
	rm -f $(BPF_OBJ)
