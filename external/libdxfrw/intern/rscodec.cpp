/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2014 J.F. Soriano (Rallaz), rallazz@gmail.com         **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

/**
 * Reed-Solomon codec
 * Reed Solomon code lifted from encoder/decoder for Reed-Solomon written by Simon Rockliff
 *
 * Original code:
 * This program may be freely modified and/or given to whoever wants it.
 *  A condition of such distribution is that the author's contribution be
 *  acknowledged by his name being left in the comments heading the program,
 *  however no responsibility is accepted for any financial or other loss which
 *  may result from some unforseen errors or malfunctioning of the program
 *  during use.
 *                                Simon Rockliff, 26th June 1991
 */


#include "rscodec.h"
#include <new>          // std::nothrow
#include <fstream>
#include <climits>

RScodec::RScodec( unsigned int pp, int mm, int tt )
{
  this->mm = mm;
  this->tt = tt;
  nn = ( 1 << mm ) - 1; //mm==8 nn=255
  kk = nn - ( tt * 2 );
  isOk = true;

  alpha_to = new ( std::nothrow ) int[nn + 1];
  index_of = new ( std::nothrow ) unsigned int[nn + 1];
  gg = new ( std::nothrow ) int[nn - kk + 1];

  RSgenerate_gf( pp ) ;
  /* compute the generator polynomial for this RS code */
  RSgen_poly() ;
}

RScodec::~RScodec()
{
  delete[] alpha_to;
  delete[] index_of;
  delete[] gg;
}


/* generate GF(2^mm) from the irreducible polynomial p(X) in pp[0]..pp[mm]
   lookup tables:  index->polynomial form   alpha_to[] contains j=alpha**i;
                   polynomial form -> index form  index_of[j=alpha**i] = i
   alpha=2 is the primitive element of GF(2^mm)
*/
void RScodec::RSgenerate_gf( unsigned int pp )
{
  int i, mask ;
  int pb;

  mask = 1 ;
  alpha_to[mm] = 0 ;
  for ( i = 0; i < mm; i++ )
  {
    alpha_to[i] = mask ;
    index_of[alpha_to[i]] = i ;
    pb = ( pp >> ( mm - 1 - i ) ) & 1;
    if ( pb != 0 )
    {
      alpha_to[mm] ^= mask;
    }
    mask <<= 1 ;
  }
  index_of[alpha_to[mm]] = mm ;
  mask >>= 1 ;
  for ( i = mm + 1; i < nn; i++ )
  {
    if ( alpha_to[i - 1] >= mask )
    {
      alpha_to[i] = alpha_to[mm] ^ ( ( alpha_to[i - 1] ^ mask ) << 1 ) ;
    }
    else alpha_to[i] = alpha_to[i - 1] << 1 ;
    index_of[alpha_to[i]] = i ;
  }
  index_of[0] = UINT_MAX;
}


/* Obtain the generator polynomial of the tt-error correcting, length
  nn=(2^mm -1) Reed Solomon code  from the product of (X+alpha**i), i=1..2*tt
*/
void RScodec::RSgen_poly()
{
  int i, j ;
  int tmp;
  int bb = nn - kk;; //nn-kk length of parity data

  gg[0] = 2 ;    /* primitive element alpha = 2  for GF(2**mm)  */
  gg[1] = 1 ;    /* g(x) = (X+alpha) initially */
  for ( i = 2; i <= bb; i++ )
  {
    gg[i] = 1 ;
    for ( j = i - 1; j > 0; j-- )
      if ( gg[j] != 0 )
      {
        if ( gg[j] < 0 ) { isOk = false; return; }
        tmp = ( index_of[gg[j]] + i ) % nn;
        if ( tmp < 0 ) { isOk = false; return; }
        gg[j] = gg[j - 1] ^ alpha_to[tmp] ;
      }
      else
      {
        gg[j] = gg[j - 1] ;
      }
    gg[0] = alpha_to[( index_of[gg[0]] + i ) % nn] ;     /* gg[0] can never be zero */
  }
  /* convert gg[] to index form for quicker encoding */
  for ( i = 0; i <= bb; i++ )  gg[i] = index_of[gg[i]] ;
}

int RScodec::calcDecode( unsigned char *data, int *recd, int **elp, int *d, int *l, int *u_lu, int *s, int *root, int *loc, int *z, int *err, int *reg, int bb )
{
  if ( !isOk ) return -1;
  int count = 0;
  int syn_error = 0;
  int i, j, u, q;

  //    for (int i=0; i<nn; i++)
  //       recd[i] = index_of[recd[i]] ;          /* put recd[i] into index form */
  for ( int i = 0, j = bb; i < kk; i++, j++ )
    recd[j] = index_of[data[j]];          /* put data in recd[i] into index form */
  for ( int i = kk, j = 0; i < nn; i++, j++ )
    recd[j] = index_of[data[j]];          /* put data in recd[i] into index form */

  /* first form the syndromes */
  for ( i = 1; i <= bb; i++ )
  {
    s[i] = 0;
    for ( j = 0; j < nn; j++ )
    {
      if ( recd[j] != -1 )
      {
        s[i] ^= alpha_to[( recd[j] + i * j ) % nn];  /* recd[j] in index form */
      }
    }
    /* convert syndrome from polynomial form to index form  */
    if ( s[i] != 0 )  syn_error = 1;      /* set flag if non-zero syndrome => error */
    s[i] = index_of[s[i]];
  }

  if ( !syn_error )      /* if no errors, ends */
  {
    /* no non-zero syndromes => no errors: output is received codeword */
    return 0;
  }

  /* errors are present, try and correct */
  /* compute the error location polynomial via the Berlekamp iterative algorithm,
  following the terminology of Lin and Costello :   d[u] is the 'mu'th
  discrepancy, where u='mu'+1 and 'mu' (the Greek letter!) is the step number
  ranging from -1 to 2*tt (see L&C),  l[u] is the
  degree of the elp at that step, and u_l[u] is the difference between the
  step number and the degree of the elp.
  */
  /* initialize table entries */
  d[0] = 0;           /* index form */
  d[1] = s[1];        /* index form */
  elp[0][0] = 0;      /* index form */
  elp[1][0] = 1;      /* polynomial form */
  for ( i = 1; i < bb; i++ )
  {
    elp[0][i] = -1;   /* index form */
    elp[1][i] = 0;   /* polynomial form */
  }
  l[0] = 0;
  l[1] = 0;
  u_lu[0] = -1;
  u_lu[1] = 0;
  u = 0;

  do
  {
    u++;
    if ( d[u] == -1 )
    {
      l[u + 1] = l[u];
      for ( i = 0; i <= l[u]; i++ )
      {
        elp[u + 1][i] = elp[u][i];
        elp[u][i] = index_of[elp[u][i]];
      }
    }
    else
    {
      /* search for words with greatest u_lu[q] for which d[q]!=0 */
      q = u - 1;
      while ( ( d[q] == -1 ) && ( q > 0 ) ) q--;
      /* have found first non-zero d[q]  */
      if ( q > 0 )
      {
        j = q;
        do
        {
          j--;
          if ( ( d[j] != -1 ) && ( u_lu[q] < u_lu[j] ) )
            q = j;
        }
        while ( j > 0 );
      }

      /* have now found q such that d[u]!=0 and u_lu[q] is maximum */
      /* store degree of new elp polynomial */
      if ( l[u] > l[q] + u - q )
      {
        l[u + 1] = l[u];
      }
      else
      {
        l[u + 1] = l[q] + u - q;
      }

      /* form new elp(x) */
      for ( i = 0; i < bb; i++ )    elp[u + 1][i] = 0;
      for ( i = 0; i <= l[q]; i++ )
      {
        if ( elp[q][i] != -1 )
        {
          elp[u + 1][i + u - q] = alpha_to[( d[u] + nn - d[q] + elp[q][i] ) % nn];
        }
      }
      for ( i = 0; i <= l[u]; i++ )
      {
        elp[u + 1][i] ^= elp[u][i];
        elp[u][i] = index_of[elp[u][i]];  /*convert old elp value to index*/
      }
    }
    u_lu[u + 1] = u - l[u + 1];

    /* form (u+1)th discrepancy */
    if ( u < bb ) /* no discrepancy computed on last iteration */
    {
      if ( s[u + 1] != -1 )
      {
        d[u + 1] = alpha_to[s[u + 1]];
      }
      else
      {
        d[u + 1] = 0;
      }
      for ( i = 1; i <= l[u + 1]; i++ )
      {
        if ( ( s[u + 1 - i] != -1 ) && ( elp[u + 1][i] != 0 ) )
        {
          d[u + 1] ^= alpha_to[( s[u + 1 - i] + index_of[elp[u + 1][i]] ) % nn];
        }
      }
      d[u + 1] = index_of[d[u + 1]];    /* put d[u+1] into index form */
    }
  }
  while ( ( u < bb ) && ( l[u + 1] <= tt ) );

  u++;
  if ( l[u] > tt )            /* elp has degree has degree >tt hence cannot solve */
  {
    return -1;              /* just output is received codeword as is */
  }

  /* can correct error */
  /* put elp into index form */
  for ( i = 0; i <= l[u]; i++ )   elp[u][i] = index_of[elp[u][i]];

  /* find roots of the error location polynomial */
  for ( i = 1; i <= l[u]; i++ )
  {
    reg[i] = elp[u][i];
  }
  count = 0;
  for ( i = 1; i <= nn; i++ )
  {
    q = 1;
    for ( j = 1; j <= l[u]; j++ )
    {
      if ( reg[j] != -1 )
      {
        reg[j] = ( reg[j] + j ) % nn;
        q ^= alpha_to[reg[j]];
      }
    }
    if ( !q )       /* store root and error location number indices */
    {
      root[count] = i;
      loc[count] = nn - i;
      count++;
    }
  }

  if ( count != l[u] )   /* no. roots != degree of elp => >tt errors and cannot solve */
  {
    return -1;        /* just output is received codeword as is */
  }

  /* no. roots = degree of elp hence <= tt errors */
  /* form polynomial z(x) */
  for ( i = 1; i <= l[u]; i++ )       /* Z[0] = 1 always - do not need */
  {
    if ( ( s[i] != -1 ) && ( elp[u][i] != -1 ) )
    {
      z[i] = alpha_to[s[i]] ^ alpha_to[elp[u][i]];
    }
    else if ( ( s[i] != -1 ) && ( elp[u][i] == -1 ) )
    {
      z[i] = alpha_to[s[i]];
    }
    else if ( ( s[i] == -1 ) && ( elp[u][i] != -1 ) )
    {
      z[i] = alpha_to[elp[u][i]];
    }
    else
    {
      z[i] = 0;
    }
    for ( j = 1; j < i; j++ )
    {
      if ( ( s[j] != -1 ) && ( elp[u][i - j] != -1 ) )
      {
        z[i] ^= alpha_to[( elp[u][i - j] + s[j] ) % nn];
      }
    }
    z[i] = index_of[z[i]];         /* put into index form */
  }

  /* evaluate errors at locations given by error location numbers loc[i] */
  for ( i = 0; i < nn; i++ ) err[i] = 0;
  for ( i = 0; i < l[u]; i++ ) /* compute numerator of error term first */
  {
    err[loc[i]] = 1;       /* accounts for z[0] */
    for ( j = 1; j <= l[u]; j++ )
    {
      if ( z[j] != -1 )
      {
        err[loc[i]] ^= alpha_to[( z[j] + j * root[i] ) % nn];
      }
    }
    if ( err[loc[i]] != 0 )
    {
      err[loc[i]] = index_of[err[loc[i]]];
      q = 0;     /* form denominator of error term */
      for ( j = 0; j < l[u]; j++ )
      {
        if ( j != i )
        {
          q += index_of[1 ^ alpha_to[( loc[j] + root[i] ) % nn]];
        }
      }
      q = q % nn;
      err[loc[i]] = alpha_to[( err[loc[i]] - q + nn ) % nn];
      data[loc[i]] ^= err[loc[i]];  /*change errors by correct data, in polynomial form */
    }
  }
  return count;
}

/** Take the string of symbols in data[i], i=0..(k-1) and encode systematically
   to produce 2*tt parity symbols in bd[0]..bd[2*tt-1]
   data[] is input and bd[] is output in polynomial form.
   Encoding is done by using a feedback shift register with appropriate
   connections specified by the elements of gg[], which was generated above.
   Codeword is   c(X) = data(X)*X**(nn-kk)+ b(X)         */
bool RScodec::encode( unsigned char *data, unsigned char *parity )
{
  if ( !isOk ) return false;
  int i, j ;
  int feedback ;
  unsigned char *idata = data;
  unsigned char *bd = parity;
  int bb = nn - kk;; //nn-kk length of parity data

  for ( i = 0; i < bb; i++ )   bd[i] = 0 ;
  for ( i = kk - 1; i >= 0; i-- )
  {
    feedback = index_of[idata[i] ^ bd[bb - 1]] ;
    if ( feedback != -1 )
    {
      for ( j = bb - 1; j > 0; j-- )
        if ( gg[j] != -1 )
          bd[j] = bd[j - 1] ^ alpha_to[( gg[j] + feedback ) % nn] ;
        else
          bd[j] = bd[j - 1] ;
      bd[0] = alpha_to[( gg[0] + feedback ) % nn] ;
    }
    else
    {
      for ( j = bb - 1; j > 0; j-- )
        bd[j] = bd[j - 1] ;
      bd[0] = 0 ;
    }
  }
  return true;
}


/* assume we have received bits grouped into mm-bit symbols in recd[i],
   i=0..(nn-1),  and recd[i] is index form (ie as powers of alpha).
   We first compute the 2*tt syndromes by substituting alpha**i into rec(X) and
   evaluating, storing the syndromes in s[i], i=1..2tt (leave s[0] zero) .
   Then we use the Berlekamp iteration to find the error location polynomial
   elp[i].   If the degree of the elp is >tt, we cannot correct all the errors
   and hence just put out the information symbols unmodified. If the degree of
   elp is <=tt, we substitute alpha**i , i=1..n into the elp to get the roots,
   hence the inverse roots, the error location numbers. If the number of errors
   located does not equal the degree of the elp, we have more than tt errors
   and cannot correct them.  Otherwise, we then solve for the error value at
   the error location and correct the error.  The procedure is that found in
   Lin and Costello. For the cases where the number of errors is known to be too
   large to correct, the information symbols as received are output (the
   advantage of systematic encoding is that hopefully some of the information
   symbols will be okay and that if we are in luck, the errors are in the
   parity part of the transmitted codeword).  Of course, these insoluble cases
   can be returned as error flags to the calling routine if desired.   */
//! Return value: number of corrected errors or -1 if can't correct it
int RScodec::decode( unsigned char *data )
{
  if ( !isOk ) return -1;
  int bb = nn - kk;; //nn-kk length of parity data

  int *recd = new ( std::nothrow ) int[nn];
  int **elp = new int *[bb + 2];
  for ( int i = 0; i < bb + 2; ++i )
    elp[i] = new int[bb];
  int *d = new int[bb + 2];
  int *l = new int[bb + 2];
  int *u_lu = new int[bb + 2];
  int *s = new int[bb + 1];
  int *root = new int[tt];
  int *loc = new int[tt];
  int *z = new int[tt + 1];
  int *err = new int[nn];
  int *reg = new int[tt + 1];

  int res = calcDecode( data, recd, elp, d, l, u_lu, s, root, loc, z, err, reg, bb );

  delete[] recd;
  for ( int i = 0; i < bb + 2; ++i )
    delete[] elp[i];
  delete[] elp;
  delete[] d;
  delete[] l;
  delete[] u_lu;
  delete[] s;
  delete[] root;
  delete[] loc;
  delete[] z;
  delete[] err;
  delete[] reg;

  return res;
}
