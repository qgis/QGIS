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
 * $Id: parser.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__

#include "info.h"

#ifdef  __cplusplus
extern "C"
{
#endif

  /*
   * high level
   */

  typedef struct _nmeaPARSER
  {
    void *top_node;
    void *end_node;
    unsigned char *buffer;
    int buff_size;
    int buff_use;

  } nmeaPARSER;

  int     nmea_parser_init( nmeaPARSER *parser );
  void    nmea_parser_destroy( nmeaPARSER *parser );

  int     nmea_parse(
    nmeaPARSER *parser,
    const char *buff, int buff_sz,
    nmeaINFO *info
  );

  /*
   * low level
   */

  int     nmea_parser_push( nmeaPARSER *parser, const char *buff, int buff_sz );
  int     nmea_parser_top( nmeaPARSER *parser );
  int     nmea_parser_pop( nmeaPARSER *parser, void **pack_ptr );
  int     nmea_parser_peek( nmeaPARSER *parser, void **pack_ptr );
  int     nmea_parser_drop( nmeaPARSER *parser );
  int     nmea_parser_buff_clear( nmeaPARSER *parser );
  int     nmea_parser_queue_clear( nmeaPARSER *parser );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_PARSER_H__ */
