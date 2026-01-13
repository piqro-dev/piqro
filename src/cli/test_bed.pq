var i = 0

print('calling native sin:')

repeat until i >= 3
{
	print(sin(i))
	
	i += 1
}

define sine_wave(x) 
{
	return sin(x)
}

print('calling sine_wave (wrapper around sin):')

i = 0

repeat until i >= 3
{
	print(sine_wave(i))
	
	i += 1
}