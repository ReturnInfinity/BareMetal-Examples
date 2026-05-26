// hello.cpp -- Output a 'Hello, world!' message

extern "C" {
#include "libBareMetal.h"
}

int main()
{
	b_output("Hello, world! - from C++", 24);
	return 0;
}
