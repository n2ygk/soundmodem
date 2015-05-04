#ifndef _COMPLEX_H
#define _COMPLEX_H

#include <math.h>

typedef struct {
	float re, im;
#ifdef __ia64__
	int dummy;
#endif
} complex;

#if __GNUC__ < 5
#define only_inline extern inline
#else
#define only_inline inline
#endif

/*
 * Complex multiplication.
 */
only_inline complex cmul(complex x, complex y)
{
	complex z;

	z.re = x.re * y.re - x.im * y.im;
	z.im = x.re * y.im + x.im * y.re;

	return z;
}

/*
 * Complex ... yeah, what??? Returns a complex number that has the
 * properties: |z| = |x| * |y|  and  arg(z) = arg(y) - arg(x)
 */
only_inline complex ccor(complex x, complex y)
{
	complex z;

	z.re = x.re * y.re + x.im * y.im;
	z.im = x.re * y.im - x.im * y.re;

	return z;
}

/*
 * Real part of the complex ???
 */
only_inline float ccorI(complex x, complex y)
{
	return x.re * y.re + x.im * y.im;
}

/*
 * Imaginary part of the complex ???
 */
only_inline float ccorQ(complex x, complex y)
{
	return x.re * y.im - x.im * y.re;
}

/*
 * Modulo (absolute value) of a complex number.
 */
only_inline float cmod(complex x)
{
	return sqrt(x.re * x.re + x.im * x.im);
}

/*
 * Square of the absolute value (power).
 */
only_inline float cpwr(complex x)
{
	return (x.re * x.re + x.im * x.im);
}

/*
 * Argument of a complex number.
 */
only_inline float carg(complex x)
{
	return atan2(x.im, x.re);
}

#endif
