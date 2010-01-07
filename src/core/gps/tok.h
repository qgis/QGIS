/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: tok.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#include "config.h"

#ifdef  __cplusplus
extern "C"
{
#endif

  int     nmea_calc_crc( const char *buff, int buff_sz );
  int     nmea_atoi( const char *str, int str_sz, int radix );
  double  nmea_atof( const char *str, int str_sz );
  int     nmea_printf( char *buff, int buff_sz, const char *format, ... );
  int     nmea_scanf( const char *buff, int buff_sz, const char *format, ... );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TOK_H__ */
