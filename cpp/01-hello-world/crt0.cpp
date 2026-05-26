extern "C" {
	extern int main();

	extern char __bss_start;
	extern char __bss_stop;
	extern void (*__init_array_start[])();
	extern void (*__init_array_end[])();
	extern void (*__fini_array_start[])();
	extern void (*__fini_array_end[])();
}

static void zero_bss();
static void call_constructors();
static void call_destructors();

extern "C" int _start()
{
	zero_bss();
	call_constructors();
	int retval = main();
	call_destructors();
	return retval;
}

static void zero_bss()
{
	for (char *c = &__bss_start; c < &__bss_stop; c++)
		*c = 0;
}

static void call_constructors()
{
	for (auto f = __init_array_start; f < __init_array_end; f++)
		(*f)();
}

static void call_destructors()
{
	for (auto f = __fini_array_start; f < __fini_array_end; f++)
		(*f)();
}
