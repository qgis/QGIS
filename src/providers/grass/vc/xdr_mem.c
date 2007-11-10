#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#if !defined(HAVE_UINTPTR_T) && !defined(uintptr_t)
 typedef unsigned long uintptr_t;
#endif

#if SIZEOF_LONG > 4
#error This XDR implementation assumes 4-byte longs
#endif

/* Local mod: assumes WIN32 is i386 and little-endian generic is 32-bit */
#if defined(WIN32) || defined(__CYGWIN__)
static unsigned long int
ntohl(unsigned long int x)
{ /* could write VC++ inline assembler, but not worth it for now */
#ifdef _MSC_VER
  return((x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24));
#else
  __asm__("xchgb %b0,%h0\n\t"	/* swap lower bytes	*/
	  "rorl $16,%0\n\t"	/* swap words		*/
	  "xchgb %b0,%h0"       /* swap higher bytes	*/
	  :"=q" (x)
	  : "0" (x));
  return x;
#endif 
}
#else /* net is big-endian: little-endian hosts need byte-swap code */
#ifndef WORDS_BIGENDIAN
/* #ifdef LITTLE_ENDIAN */
static unsigned long int 
htonl (unsigned long int x)
{
  return((x << 24) | ((x & 0xff00) << 8) | ((x & 0xff0000) >> 8) | (x >> 24));
}
#else
#define ntohl(x) (x)
#endif
#endif
#define htonl ntohl

/*********************************************************************
 * RPC for the Windows NT Operating System
 * 1993 by Martin F. Gergeleit
 * Users may use, copy or modify Sun RPC for the Windows NT Operating 
 * System according to the Sun copyright below.
 *
 * RPC for the Windows NT Operating System COMES WITH ABSOLUTELY NO 
 * WARRANTY, NOR WILL I BE LIABLE FOR ANY DAMAGES INCURRED FROM THE 
 * USE OF. USE ENTIRELY AT YOUR OWN RISK!!!
 *********************************************************************/

/* @(#)xdr_mem.c	2.1 88/07/29 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)xdr_mem.c 1.19 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#include <string.h> /* for memcpy */
#include <rpc/types.h>
#include <rpc/xdr.h>

/* The use of 'long' here would be problematic if it were ever to be
   used on a 64-bit system */

static bool_t	xdrmem_getlong();
static bool_t	xdrmem_putlong();
static bool_t	xdrmem_getbytes();
static bool_t	xdrmem_putbytes();
static u_int	xdrmem_getpos();
static bool_t	xdrmem_setpos();
static long *	xdrmem_inline();
static void	xdrmem_destroy();

static struct	xdr_ops xdrmem_ops = {
	xdrmem_getlong,
	xdrmem_putlong,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline,
	xdrmem_destroy
};

/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.  
 */
void
xdrmem_create(xdrs, addr, size, op)
	register XDR *xdrs;
	caddr_t addr;
	u_int size;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmem_ops;
	xdrs->x_private = xdrs->x_base = addr;
	xdrs->x_handy = size;
}

static void
xdrmem_destroy(/*xdrs*/)
	/*XDR *xdrs;*/
{
}

static bool_t
xdrmem_getlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0)
		return (FALSE);
	*lp = (long)ntohl((u_long)(*((long *)(xdrs->x_private))));
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

static bool_t
xdrmem_putlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0)
		return (FALSE);
	*(long *)xdrs->x_private = (long)htonl((u_long)(*lp));
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

static bool_t
xdrmem_getbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	memcpy(addr, xdrs->x_private, len);
	xdrs->x_private += len;
	return (TRUE);
}

static bool_t
xdrmem_putbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	memcpy(xdrs->x_private, addr, len);
	xdrs->x_private += len;
	return (TRUE);
}

static u_int
xdrmem_getpos(xdrs)
	register XDR *xdrs;
{

	return ((uintptr_t)xdrs->x_private - (uintptr_t)xdrs->x_base);
}

static bool_t
xdrmem_setpos(xdrs, pos)
	register XDR *xdrs;
	u_int pos;
{
	register caddr_t newaddr = xdrs->x_base + pos;
	register caddr_t lastaddr = xdrs->x_private + xdrs->x_handy;

	if ((uintptr_t)newaddr > (uintptr_t)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (uintptr_t)lastaddr - (uintptr_t)newaddr;
	return (TRUE);
}

static long *
xdrmem_inline(xdrs, len)
	register XDR *xdrs;
	int len;
{
	long *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (long *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
