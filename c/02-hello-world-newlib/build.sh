#!/usr/bin/env bash

currentdir=`pwd`
gcc -c hello.c -mno-red-zone -mcmodel=large -fomit-frame-pointer -g -fno-stack-protector -I $currentdir/include
ld -T $currentdir/c.ld -z max-page-size=0x1000 -L $currentdir/lib -o hello lib/crt0.o hello.o -lc
objcopy -O binary hello hello.app
