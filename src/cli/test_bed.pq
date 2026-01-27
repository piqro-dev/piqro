define put(a, b) { print(a); print(b); }

var x = 0;
var y = 0;
var w = 10;
var h = 10;

var i = 0; 

repeat w
{
  var j = 0;

  repeat h
  {
    put(i, j);

    j += 1;
  }

  i += 1;
}

//
//define check_array(array, n)
//{
//	var s = true
//
//	var i = 0 repeat until i != n
//	{
//		s = s && array[i] == i; 
//		
//		i += 1
//	}
//
//	return s
//}
//
//// ================================== //
//
//var a = 10
//
//test(a == 10, 'top level assignment')
//
//// ================================== //
//
//var b = 0
//
//repeat 5 
//{
//	b += 1
//}
//
//test(b == 5, 'top level repeat')
//
//// ================================== //
//
//var success = true
//var c[10]
//
//var i = 0 
//
//repeat 10
//{
//	c[i] = i
//	i += 1
//}
//
//test(check_array(c, 10), 'top level array assignment by top level repeat loop')
//
//// ================================== //
//
//define do_math(n) 
//{
//	return n * 2
//}
//
//var d = do_math(50)
//
//test(d == 100, 'top level assignment by procedure')
//
//// ================================== //
//
//define factorial(n)
//{
//	if n == 1
//	{
//		return 1
//	}
//
//	return n * factorial(n - 1); 
//}
//
//var e = factorial(5)
//
//test(e == 120, 'top level assignment by recursive procedure')
//
//// ================================== //
//
//var another_array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
//
//test(check_array(another_array, 10), 'top level array with initializer list check using procedure')
//
//// ================================== //
//
//var unknown_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }
//
//test(check_array(unknown_array, 10), 'top level unknown size array with initializer list check using procedure')
