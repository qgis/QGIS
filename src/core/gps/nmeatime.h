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
 * $Id: time.h 4 2007-08-27 13:11:03Z xtimor $
 *
 */

/** \file */

#ifndef __NMEA_TIME_H__
#define __NMEA_TIME_H__

#include "config.h"

#ifdef  __cplusplus
extern "C"
{
#endif

  /**
   * Date and time data
   * @see nmea_time_now
   */
  typedef struct _nmeaTIME
  {
    int     year;       /**< Years since 1900 */
    int     mon;        /**< Months since January - [0,11] */
    int     day;        /**< Day of the month - [1,31] */
    int     hour;       /**< Hours since midnight - [0,23] */
    int     min;        /**< Minutes after the hour - [0,59] */
    int     sec;        /**< Seconds after the minute - [0,59] */
    int     msec;       /**< Thousandths part of second - [0,999] */

  } nmeaTIME;

  /**
   * \brief Get time now to nmeaTIME structure
   */
  void nmea_time_now( nmeaTIME *t );

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_TIME_H__ */
