/*****************************************************************************/

/*
 *      gendf9icfilt.cc  --  Compute DF9IC Hardware Modem Filter Curves.
 *
 *      Copyright (C) 2003
 *        Thomas Sailer (t.sailer@alumni.ethz.ch)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/*****************************************************************************/

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <complex>
#include <ostream>
#include <iostream>

using namespace std;

/* --------------------------------------------------------------------- */

template<typename T> inline T par(const T& a)
{
	return a;
}

template<typename T> inline T par(const T& a, const T& b)
{
	return a * b / (a + b);
}

template<typename T> inline T par(const T& a, const T& b, const T& c)
{
	return a * b * c / (b * c + a * c + a * b);
}

template<typename T> inline T par(const T& a, const T& b, const T& c, const T& d)
{
	return a * b * c * d / (b * c * d + a * c * d + a * b * d + a * b * c);
}

template<typename T> complex<T> rxfilter(T freq)
{
	complex<T> g1(T(1)/100e3,0);
	complex<T> g2(T(1)/82e3,0);
	complex<T> g3(T(1)/39e3,0);
	complex<T> g4(T(1)/27e3,0);
	freq *= 2 * M_PI * 9600;
	complex<T> y1(0,freq*470e-9);
	complex<T> y2(0,freq*1e-9);
	complex<T> y3(0,freq*220e-12);
	complex<T> y4(0,freq*1e-9);

	return y1 * g2 * g3 * g4 / 
		(((g3 + y3) * g2 * (y1 + g1) + y3 * (y2 + g3) * (y1 + g1 + g2)) * (g4 + y4));
}

template<typename T> complex<T> txfilter(T freq)
{
	complex<T> g1(T(1)/100,0);
	complex<T> g2(T(1)/10e3,0);
	complex<T> g3(T(1)/100e3,0);
	complex<T> g4(T(1)/100e3,0);
	complex<T> g5(T(1)/56e3,0);
	complex<T> g6(T(1)/8.2e3,0);
	complex<T> g7(T(1)/12e3,0);
	freq *= 2 * M_PI * 9600;
	complex<T> y1(0,freq*3.3e-9);
	complex<T> y2(0,freq*470e-12);
	complex<T> y3(0,freq*100e-12);
	complex<T> y4(0,freq*1e-9);

	return g1 * g4 * g5 * g6 /
		(((g1 + g2 + g3 + y1) * (g5 + y3) * g4 + 
		  (g1 + g2 + g3 + y1 + g4) * y3 * (y2 + g5)) * (y4 + g7));
}

/* ---------------------------------------------------------------------- */

static void printtransferfunc(ostream& os, unsigned int nr, double over)
{
	over /= nr;
       	os << "# name: rxf\n"
		"# type: complex matrix\n"
		"# rows: " << nr << "\n"
		"# columns: 1\n";
	for (unsigned int i = 0; i < nr; i++) {
		complex<double> tf = rxfilter(i * over);
		os << "(" << tf.real() << "," << tf.imag() << ")\n";
	}
       	os << "# name: txf\n"
		"# type: complex matrix\n"
		"# rows: " << nr << "\n"
		"# columns: 1\n";
	for (unsigned int i = 0; i < nr; i++) {
		complex<double> tf = txfilter(i * over);
		os << "(" << tf.real() << "," << tf.imag() << ")\n";
	}
	os.flush();
}

/* ---------------------------------------------------------------------- */

/*
 * This fft routine is from ~gabriel/src/filters/fft/fft.c;
 * I am unsure of the original source.  The file contains no
 * copyright notice or description.
 * The declaration is changed to the prototype form but the
 * function body is unchanged.  (J. T. Buck)
 */

/*
 * Replace data by its discrete Fourier transform, if isign is
 * input as 1, or by its inverse discrete Fourier transform, if
 * "isign" is input as -1.  "data'"is a complex array of length "nn".
 * "nn" MUST be an integer power of 2 (this is not checked for!?)
 */

template<typename T> static void fft_rif(complex<T> *data, unsigned int nn, int isign)
{
        for (unsigned int i = 0, j = 0; i < nn; i++) {
                if (j > i) {
			complex<T> temp = data[j];
			data[j] = data[i];
			data[i] = temp;
                }
                unsigned int m = nn >> 1;
                while (m > 0 && (int)j >= (int)m) {
                        j -= m;
                        m >>= 1;
                }
                j += m;
        }
        unsigned int mmax = 1;
	T theta = -6.28318530717959 * 0.5;
	if (isign < 0)
		theta = -theta;
	T sintheta = sin(theta);
        while (nn > mmax) {
		T oldsintheta = sintheta;
		theta *= 0.5;
		sintheta = sin(theta);
		complex<T> wp(-2.0 * sintheta * sintheta, oldsintheta); /* -2.0 * sin(0.5*theta)^2 = cos(theta)-1 */
		complex<T> w(1,0);
                for (unsigned int m = 0; m < mmax; m++) {
                        for (unsigned int i = m; i < nn; i += 2*mmax) {
                                unsigned int j = i + mmax;
				complex<T> temp = w * data[j];
				data[j] = data[i] - temp;
				data[i] += temp;
                        }
			w += w * wp;
                }
                mmax <<= 1;
        }
}

/* ---------------------------------------------------------------------- */

static void printfcoeff(ostream& os, unsigned int fftsz, double over)
{
	complex<double> fftb[fftsz];

	over /= fftsz;
	for (unsigned int i = 1; i < fftsz/2; i++)
		fftb[i] = rxfilter(i * over);
	fftb[0] = complex<double>(fftb[1].real(), 0);
	fftb[fftsz/2] = 0;
	for (unsigned int i = 1; i < fftsz/2; i++)
		fftb[fftsz-i] = conj(fftb[i]);

       	os << "# name: f\n"
		"# type: complex matrix\n"
		"# rows: " << fftsz << "\n"
		"# columns: 1\n";
	for (unsigned int i = 0; i < fftsz; i++)
		os << "(" << fftb[i].real() << "," << fftb[i].imag() << ")\n";



	fft_rif(fftb, fftsz, -1);
       	os << "# name: rx\n"
		"# type: complex matrix\n"
		"# rows: " << fftsz << "\n"
		"# columns: 1\n";
	for (unsigned int i = 0; i < fftsz; i++)
		os << "(" << fftb[i].real() << "," << fftb[i].imag() << ")\n";
}

/* ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
	printtransferfunc(cout, 4096, 16);
	printfcoeff(cout, 2048, 16);
        return 0;
}
