#!/usr/bin/env bash

gcc -c hello.c -mno-red-zone -mcmodel=large -fomit-frame-pointer -g -fno-stack-protector -I include
ld -T app.ld -z max-page-size=0x1000 -L lib -o hello lib/crt0.o hello.o -lc
objcopy -O binary hello hello.app
