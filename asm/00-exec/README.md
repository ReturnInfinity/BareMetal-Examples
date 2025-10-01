## 00-exec

### Description:

A simple assembly program that executes a small program directly after it.

### Compile:
```sh
nasm exec.asm -o exec.app
```

### Usage:

Copy exec.app and your <8KiB program to the `BareMetal-OS/sys` folder

```sh
cat sys/exec.bin sys/YOUR_PROGRAM.bin > sys/execprogram.bin
./baremetal.sh build execprogram.bin
./baremetal.sh install
./baremetal.sh run
```
