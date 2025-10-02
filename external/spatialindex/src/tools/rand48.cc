/*
* Copyright (c) 1993 Martin Birgmeier
* All rights reserved.
*
* You may redistribute unmodified or modified versions of this source
* code provided that the above copyright notice and this and the
* following conditions are retained.
*
* This software is provided ``as is'', and comes with no warranties
* of any kind. I shall in no event be liable for anything that happens
* to anyone/anything when using this software.
*/

#ifndef HAVE_SRAND48

#include <math.h>
#include <stdlib.h>
#include <spatialindex/tools/rand48.h>

#define	RAND48_SEED_0	(0x330e)
#define	RAND48_SEED_1	(0xabcd)
#define	RAND48_SEED_2	(0x1234)
#define	RAND48_MULT_0	(0xe66d)
#define	RAND48_MULT_1	(0xdeec)
#define	RAND48_MULT_2	(0x0005)
#define	RAND48_ADD	(0x000b)


 /* Internal function to compute next state of the generator.  */
static void _dorand48(unsigned short[3]);

/* Unfortunately, 3 __globals, which the exported functions must access */
unsigned short __rand48_Seed[3] = {
	RAND48_SEED_0,
	RAND48_SEED_1,
	RAND48_SEED_2
};
unsigned short __rand48_Mult[3] = {
	RAND48_MULT_0,
	RAND48_MULT_1,
	RAND48_MULT_2
};
unsigned short __rand48_Add = RAND48_ADD;

/* Internal function to compute next state of the generator.  */
	static void
_dorand48(unsigned short xseed[3])
{
	unsigned long accu;
	unsigned short temp[2];

	accu = (unsigned long) __rand48_Mult[0] * (unsigned long) xseed[0] +
	 (unsigned long) __rand48_Add;
	temp[0] = (unsigned short) accu;	/* lower 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += (unsigned long) __rand48_Mult[0] * (unsigned long) xseed[1] +
		 (unsigned long) __rand48_Mult[1] * (unsigned long) xseed[0];
	temp[1] = (unsigned short) accu;	/* middle 16 bits */
	accu >>= sizeof(unsigned short) * 8;
	accu += __rand48_Mult[0] * xseed[2] + __rand48_Mult[1] * xseed[1] +
			__rand48_Mult[2] * xseed[0];
	xseed[0] = temp[0];
	xseed[1] = temp[1];
	xseed[2] = (unsigned short) accu;
}

	extern void
srand48(long seed) __THROW
{
	__rand48_Seed[0] = RAND48_SEED_0;
	__rand48_Seed[1] = (unsigned short) seed;
	__rand48_Seed[2] = (unsigned short) (seed >> 16);
	__rand48_Mult[0] = RAND48_MULT_0;
	__rand48_Mult[1] = RAND48_MULT_1;
	__rand48_Mult[2] = RAND48_MULT_2;
	__rand48_Add = RAND48_ADD;
}

	extern unsigned short *
seed48(unsigned short xseed[3]) __THROW
{
	static unsigned short sseed[3];

	sseed[0] = __rand48_Seed[0];
	sseed[1] = __rand48_Seed[1];
	sseed[2] = __rand48_Seed[2];
	__rand48_Seed[0] = xseed[0];
	__rand48_Seed[1] = xseed[1];
	__rand48_Seed[2] = xseed[2];
	__rand48_Mult[0] = RAND48_MULT_0;
	__rand48_Mult[1] = RAND48_MULT_1;
	__rand48_Mult[2] = RAND48_MULT_2;
	__rand48_Add = RAND48_ADD;
	return sseed;
}
	
	extern long
nrand48(unsigned short xseed[3]) __THROW
{
	_dorand48(xseed);
	return ((long) xseed[2] << 15) + ((long) xseed[1] >> 1);
}
	extern long
mrand48(void) __THROW
{
	_dorand48(__rand48_Seed);
	return ((long) __rand48_Seed[2] << 16) + (long) __rand48_Seed[1];
}

	extern long
lrand48(void) __THROW
{
	_dorand48(__rand48_Seed);
	return ((long) __rand48_Seed[2] << 15) + ((long) __rand48_Seed[1] >> 1);
}

	extern void
lcong48(unsigned short p[7]) __THROW
{
	__rand48_Seed[0] = p[0];
	__rand48_Seed[1] = p[1];
	__rand48_Seed[2] = p[2];
	__rand48_Mult[0] = p[3];
	__rand48_Mult[1] = p[4];
	__rand48_Mult[2] = p[5];
	__rand48_Add = p[6];
}
	extern long
jrand48(unsigned short xseed[3]) __THROW
{
	_dorand48(xseed);
	return ((long) xseed[2] << 16) + (long) xseed[1];
}

	extern double
erand48(unsigned short xseed[3]) __THROW
{
	_dorand48(xseed);
	return ldexp((double) xseed[0], -48) +
	       ldexp((double) xseed[1], -32) +
	       ldexp((double) xseed[2], -16);
}

	extern double
drand48(void) __THROW
{
	return erand48(__rand48_Seed);
}

#endif
