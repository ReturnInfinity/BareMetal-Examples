LD=ld
RUSTC=rustc

all: rust.app

.SUFFIXES: .o .rs

.PHONY: clean

.rs.o:
	$(RUSTC) -O --crate-type lib -o $@ --emit obj $<

rust.app: rust.o
	$(LD) -T app.ld -o rust.app rust.o
clean:
	rm -f *.bin *.o *.img *.sys
