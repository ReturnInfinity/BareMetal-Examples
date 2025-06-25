#!/usr/bin/env bash

rm libBareMetal.*
if [ -x "$(command -v curl)" ]; then
	curl -s -o libBareMetal.c https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	curl -s -o libBareMetal.h https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
else
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.c
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.h
fi

gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o hello.o hello.c
gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o libBareMetal.o libBareMetal.c
ld -T c.ld -o hello.app hello.o libBareMetal.o
