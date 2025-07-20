#!/usr/bin/env bash

rm -f libBareMetal.asm
if [ -x "$(command -v curl)" ]; then
	curl -s -o libBareMetal.asm https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.asm
else
	wget -q https://raw.githubusercontent.com/ReturnInfinity/BareMetal/master/api/libBareMetal.asm
fi

nasm hello.asm -o hello.app
