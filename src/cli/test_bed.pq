//////////////////////////////////////////

var a = 10

test(a == 10, 'top level assignment')

//////////////////////////////////////////

var b = 0

repeat 5 
{
	b += 1
}

test(b == 5, 'top level repeat')

//////////////////////////////////////////

var success = true
var c[10]

var i = 0 

repeat 10
{
	c[i] = i
	i += 1
}

i = 0

repeat 10
{
	success = success && c[i] == i
	i += 1 
}

test(success, 'top level array assignment by top level repeat loop')

//////////////////////////////////////

define do_math(n) 
{
	return n * 2
}

var d = do_math(50)

test(d == 100, 'top level assignment by procedure')

//////////////////////////////////////

define factorial(n)
{
	if n == 1
	{
		return 1
	}

	return n * factorial(n - 1); 
}

var e = factorial(5)

test(e == 120, 'recursive factorial procedure')

////////////////////////////////////

define check_array(ac, n)
{
	var s = true

	var ii = 0; repeat n 
	{
		s = s && ac[ii] == ii
		ii += 1
	}

	return s
}

var aaa[10]

var ii = 0; repeat 10
{
	aaa[ii] = ii
	ii += 1
}

test(check_array(aaa, 10), 'top level array check using procedure')