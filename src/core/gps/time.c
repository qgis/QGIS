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
 * $Id: time.c 4 2007-08-27 13:11:03Z xtimor $
 *
 */

/** \file nmeatime.h */

#include "nmeatime.h"

#ifdef NMEA_WIN
#   pragma warning(disable: 4201)
#   pragma warning(disable: 4214)
#   pragma warning(disable: 4115)
#   include <windows.h>
#   pragma warning(default: 4201)
#   pragma warning(default: 4214)
#   pragma warning(default: 4115)
#else
#   include <time.h>
#endif

#ifdef NMEA_WIN

void nmea_time_now( nmeaTIME *stm )
{
  SYSTEMTIME st;

  GetSystemTime( &st );

  stm->year = st.wYear - 1900;
  stm->mon = st.wMonth - 1;
  stm->day = st.wDay;
  stm->hour = st.wHour;
  stm->min = st.wMinute;
  stm->sec = st.wSecond;
  stm->msec = st.wMilliseconds;
}

#else /* NMEA_WIN */

void nmea_time_now( nmeaTIME *stm )
{
  time_t lt;
  struct tm *tt;

  time( &lt );
  tt = gmtime( &lt );

  stm->year = tt->tm_year;
  stm->mon = tt->tm_mon;
  stm->day = tt->tm_mday;
  stm->hour = tt->tm_hour;
  stm->min = tt->tm_min;
  stm->sec = tt->tm_sec;
  stm->msec = 0;
}

#endif
