/*
* Copyright Tim (xtimor@gmail.com)
*
* NMEA library is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: tok.c 17 2008-03-11 11:56:11Z xtimor $
 *
 */

/** \file tok.h */

#include "tok.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <locale.h>

#define NMEA_TOKS_COMPARE   (1)
#define NMEA_TOKS_PERCENT   (2)
#define NMEA_TOKS_WIDTH     (3)
#define NMEA_TOKS_TYPE      (4)

/**
 * \brief Calculate control sum of binary buffer
 */
int nmea_calc_crc( const char *buff, int buff_sz )
{
  int chsum = 0,
              it;

  for ( it = 0; it < buff_sz; ++it )
    chsum ^= ( int )buff[it];

  return chsum;
}

/**
 * \brief Convert string to number
 */
int nmea_atoi( const char *str, size_t str_sz, int radix )
{
  char *tmp_ptr;
  char buff[NMEA_CONVSTR_BUF];
  int res = 0;

  if ( str_sz < NMEA_CONVSTR_BUF )
  {
    memcpy( &buff[0], str, str_sz );
    buff[str_sz] = '\0';
    res = strtol( &buff[0], &tmp_ptr, radix );
  }

  return res;
}

/**
 * \brief Convert string to fraction number
 */
double nmea_atof( const char *str, int str_sz )
{
  char *tmp_ptr;
  char buff[NMEA_CONVSTR_BUF];
  double res = 0;

  if ( str_sz < NMEA_CONVSTR_BUF )
  {
    const char *oldlocale = setlocale( LC_NUMERIC, NULL );

    memcpy( &buff[0], str, str_sz );
    buff[str_sz] = '\0';
    setlocale( LC_NUMERIC, "C" );
    res = strtod( &buff[0], &tmp_ptr );
    setlocale( LC_NUMERIC, oldlocale );
  }

  return res;
}

/**
 * \brief Formating string (like standard printf) with CRC tail (*CRC)
 */
int nmea_printf( char *buff, int buff_sz, const char *format, ... )
{
  int retval, add = 0;
  va_list arg_ptr;

  if ( buff_sz <= 0 )
    return 0;

  va_start( arg_ptr, format );

  retval = NMEA_POSIX( vsnprintf )( buff, buff_sz, format, arg_ptr );

  if ( retval > 0 )
  {
    add = NMEA_POSIX( snprintf )(
            buff + retval, buff_sz - retval, "*%02x\r\n",
            nmea_calc_crc( buff + 1, retval - 1 ) );
  }

  retval += add;

  if ( retval < 0 || retval > buff_sz )
  {
    memset( buff, ' ', buff_sz );
    retval = buff_sz;
  }

  va_end( arg_ptr );

  return retval;
}

/**
 * \brief Analyse string (specificate for NMEA sentences)
 */
int nmea_scanf( const char *buff, int buff_sz, const char *format, ... )
{
  const char *beg_tok;
  const char *end_buf = buff + buff_sz;

  va_list arg_ptr;
  int tok_type = NMEA_TOKS_COMPARE;
  int width = 0;
  const char *beg_fmt = 0;
  int snum = 0, unum = 0;

  int tok_count = 0;
  void *parg_target;

  va_start( arg_ptr, format );

  for ( ; *format && buff < end_buf; ++format )
  {
    switch ( tok_type )
    {
      case NMEA_TOKS_COMPARE:
        if ( '%' == *format )
          tok_type = NMEA_TOKS_PERCENT;
        else if ( *buff++ != *format )
          goto fail;
        break;
      case NMEA_TOKS_PERCENT:
        width = 0;
        beg_fmt = format;
        tok_type = NMEA_TOKS_WIDTH;
        //intentional fall-through
      case NMEA_TOKS_WIDTH:
        if ( isdigit( *format ) )
          break;
        {
          tok_type = NMEA_TOKS_TYPE;
          if ( format > beg_fmt )
            width = nmea_atoi( beg_fmt, ( int )( format - beg_fmt ), 10 );
        }
      case NMEA_TOKS_TYPE:
        beg_tok = buff;

        if ( !width && ( 'c' == *format || 'C' == *format ) && *buff != format[1] )
          width = 1;

        if ( width )
        {
          if ( buff + width <= end_buf )
            buff += width;
          else
            goto fail;
        }
        else
        {
          if ( !format[1] || ( 0 == ( buff = ( char * )memchr( buff, format[1], end_buf - buff ) ) ) )
            buff = end_buf;
        }

        if ( buff > end_buf )
          goto fail;

        tok_type = NMEA_TOKS_COMPARE;
        tok_count++;

        parg_target = 0; width = ( int )( buff - beg_tok );

        switch ( *format )
        {
          case 'c':
          case 'C':
            parg_target = ( void * )va_arg( arg_ptr, char * );
            if ( width && 0 != ( parg_target ) )
              *(( char * )parg_target ) = *beg_tok;
            break;
          case 's':
          case 'S':
            parg_target = ( void * )va_arg( arg_ptr, char * );
            if ( width && 0 != ( parg_target ) )
            {
              memcpy( parg_target, beg_tok, width );
              (( char * )parg_target )[width] = '\0';
            }
            break;
          case 'f':
          case 'g':
          case 'G':
          case 'e':
          case 'E':
            parg_target = ( void * )va_arg( arg_ptr, double * );
            if ( width && 0 != ( parg_target ) )
              *(( double * )parg_target ) = nmea_atof( beg_tok, width );
            break;
        };

        if ( parg_target )
          break;
        if ( 0 == ( parg_target = ( void * )va_arg( arg_ptr, int * ) ) )
          break;
        if ( !width )
          break;

        switch ( *format )
        {
          case 'd':
          case 'i':
            snum = nmea_atoi( beg_tok, width, 10 );
            memcpy( parg_target, &snum, sizeof( int ) );
            break;
          case 'u':
            unum = nmea_atoi( beg_tok, width, 10 );
            memcpy( parg_target, &unum, sizeof( unsigned int ) );
            break;
          case 'x':
          case 'X':
            unum = nmea_atoi( beg_tok, width, 16 );
            memcpy( parg_target, &unum, sizeof( unsigned int ) );
            break;
          case 'o':
            unum = nmea_atoi( beg_tok, width, 8 );
            memcpy( parg_target, &unum, sizeof( unsigned int ) );
            break;
          default:
            goto fail;
        };

        break;
    };
  }

fail:

  va_end( arg_ptr );

  return tok_count;
}
