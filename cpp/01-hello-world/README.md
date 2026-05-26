## 01-hello-world

### Description:

A simple C++ example that calls the kernel output function directly.

### Prerequisites:

- `libBareMetal.c` and `libBareMetal.h`. The `build.sh` script will download them.

### Compile:
```sh
g++ -c -m64 -nostdlib -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -std=c++17 -o crt0.o crt0.cpp
g++ -c -m64 -nostdlib -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -std=c++17 -o hello.o hello.cpp
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o libBareMetal.o libBareMetal.c
ld -T cpp.ld -o hello.app crt0.o hello.o libBareMetal.o
```
