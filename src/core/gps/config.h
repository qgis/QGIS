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
 * $Id: config.h 17 2008-03-11 11:56:11Z xtimor $
 *
 */

#ifndef __NMEA_CONFIG_H__
#define __NMEA_CONFIG_H__

#define NMEA_VERSION        ("0.5.3")
#define NMEA_VERSION_MAJOR  (0)
#define NMEA_VERSION_MINOR  (5)
#define NMEA_VERSION_PATCH  (3)

#define NMEA_CONVSTR_BUF    (256)
#define NMEA_TIMEPARSE_BUF  (256)

#if defined(WINCE) || defined(UNDER_CE)
#   define  NMEA_CE
#endif

#if defined(WIN32) || defined(NMEA_CE)
#   define  NMEA_WIN
#else
#   define  NMEA_UNI
#endif

#if defined(_MSC_VER)
# define NMEA_POSIX(x)  _##x
# define NMEA_INLINE    __inline
#else
# define NMEA_POSIX(x)  x
# define NMEA_INLINE    inline
#endif

#if !defined(NDEBUG) && !defined(NMEA_CE)
#   include <assert.h>
#   define NMEA_ASSERT(x)   assert(x)
#else
#   define NMEA_ASSERT(x)
#endif

#endif /* __NMEA_CONFIG_H__ */
