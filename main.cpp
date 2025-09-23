#include <stdint.h>

static inline void set_u32(uint32_t address, uint32_t value)
{
	*(volatile uint32_t*)address = value;
}

static inline uint32_t get_u32(uint32_t address)
{
	return *(uint32_t*)address;
}

static inline void sleep(uint32_t time)
{
	for (volatile uint32_t i = 0; i < time; i++) {}
}

int main(void)
{
	set_u32(0x4000f000, (1 << 5));               // IO BANK

	while (get_u32(0x4000c008) & ((1 << 5) == 0)) {}  // Reset Done?

	set_u32(0x400140cc, 0x05);                     // IO PAD = FUNC 5 (GPIO)
	set_u32(0xd0000020, (1 << 25));              // GPIO_OE
	
	while (true)
	{
		set_u32( 0xd000001c, (1 << 25));      // XOR GPIO
		sleep(500'000);
		set_u32( 0xd000001c, (0 << 25));
		sleep(250'000);
		set_u32( 0xd000001c, (1 << 25));      // XOR GPIO
		sleep(500'000);
	}

	return 0;
}