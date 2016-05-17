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
 * $Id: parse.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

#ifndef NMEA_PARSE_H
#define NMEA_PARSE_H

#include "sentence.h"

#ifdef  __cplusplus
extern "C"
{
#endif

  int nmea_pack_type( const char *buff, int buff_sz );
  int nmea_find_tail( const char *buff, int buff_sz, int *res_crc );

  int nmea_parse_GPGGA( const char *buff, int buff_sz, nmeaGPGGA *pack );
  int nmea_parse_GPGSA( const char *buff, int buff_sz, nmeaGPGSA *pack );
  int nmea_parse_GPGSV( const char *buff, int buff_sz, nmeaGPGSV *pack );
  int nmea_parse_GPRMC( const char *buff, int buff_sz, nmeaGPRMC *pack );
  int nmea_parse_GPVTG( const char *buff, int buff_sz, nmeaGPVTG *pack );

  void nmea_GPGGA2info( nmeaGPGGA *pack, nmeaINFO *info );
  void nmea_GPGSA2info( nmeaGPGSA *pack, nmeaINFO *info );
  void nmea_GPGSV2info( nmeaGPGSV *pack, nmeaINFO *info );
  void nmea_GPRMC2info( nmeaGPRMC *pack, nmeaINFO *info );
  void nmea_GPVTG2info( nmeaGPVTG *pack, nmeaINFO *info );

#ifdef  __cplusplus
}
#endif

#endif /* NMEA_PARSE_H */
