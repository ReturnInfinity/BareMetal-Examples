## 01-hello-world

Compile:
```sh
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o hello.o hello.c
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o libBareMetal.o libBareMetal.c
ld -T c.ld -o hello.app hello.o libBareMetal.o
```