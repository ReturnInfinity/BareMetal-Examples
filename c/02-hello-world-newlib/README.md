## 02-hello-world-newlib

### Description:

A C example that outputs text via a standard C function.

### Prerequisites:

- Build [newlib4 for BareMetal](https://github.com/ReturnInfinity/BareMetal-newlib4/)
- Copy `include` and `lib` folders to this folder

### Compile:
```sh
currentdir=`pwd`
gcc -c hello.c -mno-red-zone -mcmodel=large -fomit-frame-pointer -g -fno-stack-protector -I $currentdir/include
ld -T $currentdir/c.ld -z max-page-size=0x1000 -L $currentdir/lib -o hello lib/crt0.o hello.o -lc
objcopy -O binary hello hello.app
```