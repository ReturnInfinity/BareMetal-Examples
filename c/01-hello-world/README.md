## 01-hello-world

### Description:

A simple C example that calls the kernel output function directly.

### Prerequisites:

- `libBareMetal.c` and `libBareMetal.h`. The `build.sh` script will download them.

### Compile:
```sh
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o hello.o hello.c
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o libBareMetal.o libBareMetal.c
ld -T c.ld -o hello.app hello.o libBareMetal.o
```