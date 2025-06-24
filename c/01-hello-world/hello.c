// hello.c -- Output a 'Hello, world!' message

// gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o hello.o hello.c
// gcc -c -m64 -nostdlib -nostartfiles -nodefaultlibs -o libBareMetal.o libBareMetal.c
// ld -T c.ld -o hello.app hello.o libBareMetal.o

#include "libBareMetal.h"

int main(void)
{
	b_output("Hello, world!", 13);
	return 0;
}