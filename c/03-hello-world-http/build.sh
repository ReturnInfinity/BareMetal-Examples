#!/usr/bin/env bash

CFLAGS="-c -m64 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -falign-functions=16 -fomit-frame-pointer -mno-red-zone -fno-builtin -fno-stack-protector -Os -ffunction-sections -fdata-sections"

rm -f libBareMetal.*
if [ -x "$(command -v curl)" ]; then
	curl -s -o libBareMetal.c https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	curl -s -o libBareMetal.h https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
else
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
fi

gcc $CFLAGS -o crt0.o crt0.c
gcc $CFLAGS -o hello_http.o hello_http.c
gcc $CFLAGS -o libBareMetal.o libBareMetal.c
ld -T c.ld -o http.app crt0.o hello_http.o libBareMetal.o
