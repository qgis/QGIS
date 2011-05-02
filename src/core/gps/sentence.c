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
