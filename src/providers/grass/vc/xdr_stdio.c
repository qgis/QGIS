#ifdef HAVE_CONFIG_H
#include <config.h>
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

/* @(#)xdr_stdio.c	2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)xdr_stdio.c 1.16 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_stdio.c, XDR implementation on standard i/o file.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * This set of routines implements a XDR on a stdio stream.
 * XDR_ENCODE serializes onto the stream, XDR_DECODE de-serializes
 * from the stream.
 */

#include <rpc/types.h>
#include <stdio.h>
#include <rpc/xdr.h>

static bool_t	xdrstdio_getlong();
static bool_t	xdrstdio_putlong();
static bool_t	xdrstdio_getbytes();
static bool_t	xdrstdio_putbytes();
static u_int	xdrstdio_getpos();
static bool_t	xdrstdio_setpos();
static long *	xdrstdio_inline();
static void	xdrstdio_destroy();

/*
 * Ops vector for stdio type XDR
 */
static struct xdr_ops	xdrstdio_ops = {
	xdrstdio_getlong,	/* deseraialize a long int */
	xdrstdio_putlong,	/* seraialize a long int */
	xdrstdio_getbytes,	/* deserialize counted bytes */
	xdrstdio_putbytes,	/* serialize counted bytes */
	xdrstdio_getpos,	/* get offset in the stream */
	xdrstdio_setpos,	/* set offset in the stream */
	xdrstdio_inline,	/* prime stream for inline macros */
	xdrstdio_destroy	/* destroy stream */
};

/*
 * Initialize a stdio xdr stream.
 * Sets the xdr stream handle xdrs for use on the stream file.
 * Operation flag is set to op.
 */
void
xdrstdio_create(xdrs, file, op)
	register XDR *xdrs;
	FILE *file;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrstdio_ops;
	xdrs->x_private = (caddr_t)file;
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
}

/*
 * Destroy a stdio xdr stream.
 * Cleans up the xdr stream handle xdrs previously set up by xdrstdio_create.
 */
static void
xdrstdio_destroy(xdrs)
	register XDR *xdrs;
{
	if (xdrs->x_op == XDR_ENCODE) fflush((FILE *)xdrs->x_private);
	/* xx should we close the file ?? */
}

static bool_t
xdrstdio_getlong(xdrs, lp)
	XDR *xdrs;
	register long *lp;
{

	if (fread((caddr_t)lp, sizeof(long), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
#ifndef mc68000
	*lp = ntohl(*lp);
#endif
	return (TRUE);
}

static bool_t
xdrstdio_putlong(xdrs, lp)
	XDR *xdrs;
	long *lp;
{

#ifndef mc68000
	long mycopy = htonl(*lp);
	lp = &mycopy;
#endif
	if (fwrite((caddr_t)lp, sizeof(long), 1, (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_getbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{

	if ((len != 0) && (fread(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_putbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	u_int len;
{

	if ((len != 0) && (fwrite(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static u_int
xdrstdio_getpos(xdrs)
	XDR *xdrs;
{

	return ((u_int) ftell((FILE *)xdrs->x_private));
}

static bool_t
xdrstdio_setpos(xdrs, pos) 
	XDR *xdrs;
	u_int pos;
{ 

	return ((fseek((FILE *)xdrs->x_private, (long)pos, 0) < 0) ?
		FALSE : TRUE);
}

static long *
xdrstdio_inline(xdrs, len)
	XDR *xdrs;
	u_int len;
{

	/*
	 * Must do some work to implement this: must insure
	 * enough data in the underlying stdio buffer,
	 * that the buffer is aligned so that we can indirect through a
	 * long *, and stuff this pointer in xdrs->x_buf.  Doing
	 * a fread or fwrite to a scratch buffer would defeat
	 * most of the gains to be had here and require storage
	 * management on this buffer, so we don't do this.
	 */
	return (NULL);
}
