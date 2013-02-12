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
 * $Id: tok.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

#ifndef __NMEA_TOK_H__
#define __NMEA_TOK_H__

#include "config.h"
#include "stdlib.h"

#ifdef  __cplusplus
extern "C"
{
#endif

  int     nmea_calc_crc( const char *buff, int buff_sz );
  int     nmea_atoi( const char *str, size_t str_sz, int radix );
  double  nmea_atof( const char *str, int str_sz );
  int     nmea_printf( char *buff, int buff_sz, const char *format, ... );
  int     nmea_scanf( const char *buff, int buff_sz, const char *format, ... );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TOK_H__ */
