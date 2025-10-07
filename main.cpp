#include "sdk/config_def.h"
#include "sdk/includes.h"

int main()
{
	while (true)
	{
		__builtin_printf("USB: Hello, world!\n");
		sleep_ms(1000);
	}
}
