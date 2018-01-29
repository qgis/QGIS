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
 * $Id: sentence.c 17 2008-03-11 11:56:11Z xtimor $
 *
 */

#include "sentence.h"

#include <string.h>

void nmea_zero_GPGGA( nmeaGPGGA *pack )
{
  memset( pack, 0, sizeof( nmeaGPGGA ) );
  nmea_time_now( &pack->utc );
  pack->ns = 'N';
  pack->ew = 'E';
  pack->elv_units = 'M';
  pack->diff_units = 'M';
}

void nmea_zero_GPGSA( nmeaGPGSA *pack )
{
  memset( pack, 0, sizeof( nmeaGPGSA ) );
  pack->fix_mode = 'A';
  pack->fix_type = NMEA_FIX_BAD;
}

void nmea_zero_GPGSV( nmeaGPGSV *pack )
{
  memset( pack, 0, sizeof( nmeaGPGSV ) );
}

void nmea_zero_GPRMC( nmeaGPRMC *pack )
{
  memset( pack, 0, sizeof( nmeaGPRMC ) );
  nmea_time_now( &pack->utc );
  pack->status = 'V';
  pack->ns = 'N';
  pack->ew = 'E';
  pack->declin_ew = 'E';
}

void nmea_zero_GPVTG( nmeaGPVTG *pack )
{
  memset( pack, 0, sizeof( nmeaGPVTG ) );
  pack->dir_t = 'T';
  pack->dec_m = 'M';
  pack->spn_n = 'N';
  pack->spk_k = 'K';
}
