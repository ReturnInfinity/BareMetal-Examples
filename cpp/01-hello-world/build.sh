#!/usr/bin/env bash

CFLAGS="-c -m64 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -falign-functions=16 -fomit-frame-pointer -mno-red-zone -fno-builtin -fno-stack-protector -fno-exceptions -fno-rtti -std=c++17"

rm -f libBareMetal.*
if [ -x "$(command -v curl)" ]; then
	curl -s -o libBareMetal.c https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	curl -s -o libBareMetal.h https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
else
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
fi

g++ $CFLAGS -o crt0.o crt0.cpp
g++ $CFLAGS -o hello.o hello.cpp
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -ffreestanding -fomit-frame-pointer -mno-red-zone -fno-builtin -fno-stack-protector -o libBareMetal.o libBareMetal.c
ld -T cpp.ld -o hello.app crt0.o hello.o libBareMetal.o
