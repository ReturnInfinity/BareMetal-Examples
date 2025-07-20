# Porting an existing app to BareMetal

This document only refers to existing programs that run on Linux/Unix.

BareMetal is a *very* minimal kernel and only provides a simple API to the hardware. Many of the layers of abstraction that are commonplace in most operating systems are not present here. Not including these abstractions allow the programmer to use what would be most efficient for their use case.

Some differences are:

- Only one running application
- All free memory in the system is pre-allocated for the application
- No threading (but SMP is supported for multi-core workloads)

Advantages:

- Faster than Linux on the same hardware (virtual or physical)
- More secure than conventional operating systems (no concept of users)
- BareMetal uses only 2 MiB of RAM for itself

Disadvantages:

- You can't just take a Linux app and compile it to BareMetal (at the moment).

A `syscall` library is in the works to make this easier.


## Example

[llama2.c](https://github.com/karpathy/llama2.c) is a program written in C that allows for Llama 2 model inference. It was recently ported to BareMetal [here](https://github.com/IanSeyler/llama2.c)

Newlib was used for the base C library. Several adjustments needed to be made for BareMetal:

- Removed the use of `mmap()` and the extra headers to go with it
- Removed the command line options
- Included the data as part of the program executable


// EOF