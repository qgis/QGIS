/*
    freespa
    Copyright (C) 2023  B. E. Pieters,
    IEK-5 Photovoltaik, Forschunszentrum Juelich

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public
    License along with this program.  If not, see
    <http://www.gnu.org/licenses/>.
*/
/*****************************************************************
 *  INSTITUT FUER ENERGIE- UND KLIMAFORSCHUNG                    *
 +  IEK-5 PHOTOVOLTAIK                                           *
 *                                                               *
 *        ########                _   _                          *
 *     ##########                |_| |_|                         *
 *    ##########     ##         _ _   _ _     ___ ____ _   _     *
 *   ##########     ####       | | | | | |   |_ _/ ___| | | |    *
 *   #########     #####    _  | | | | | |    | | |   | |_| |    *
 *   #    ###     ######   | |_| | |_| | |___ | | |___|  _  |    *
 *    #          ######     \___/ \___/|_____|___\____|_| |_|    *
 *     ##      #######      F o r s c h u n g s z e n t r u m    *
 *       ##########                                              *
 *                                                               *
 *   http://www.fz-juelich.de/iek/iek-5/DE/Home/home_node.html   *
 *****************************************************************
 *                                                               *
 *    Dr. Bart E. Pieters 2022                                   *
 *                                                               *
 *****************************************************************/

	
/* see the DOC.md file for documentation */

/* freespa provides an implementation of the SPA (solar position 
 * algorithm). A document decribing the algorithm can be found on 
 * NREL's website:
 * https://midcdmz.nrel.gov/spa/
 * 
 * The purpose of this implementation is to overcome licensing issues 
 * with NREL's spa. NREL's spa cannot be redistributed (sort of "free 
 * as in beer you may not share with your friends"). This 
 * implementation you can share under the GPLv3. 
 * 
 * NREL's spa is built around the algorithms found in:
 * Meeus, J., 1998. Astronomical Algorithms, second ed. Willmann-Bell, 
 * Inc., Richmond, Virginia, USA.
 */

#ifndef	_TIME_H_
#include <time.h>
#endif
#ifndef _FREESPA_H_
#define _FREESPA_H_

#ifndef FS_TIME_T
/* per default FS_TIME_T is the time_t type
 * This is most convenient but requires a 64bit signed integer time_t 
 * type
 */
#define FS_TIME_T time_t
#else
/* Not using the time_t type. The simplest solution is an int64_t
 * for which we need stdint.h
 */
#include <stdint.h>
#define FS_CUSTOM_TIME_T
#endif
/* We require a 64bit signed integer time_t type
 * These compile time asserts check for integer, 64bit and signedness
 */
_Static_assert ((FS_TIME_T) 1.5 == 1  , "error: FS_TIME_T type is not integer");
_Static_assert (sizeof(FS_TIME_T) == 8, "error: FS_TIME_T type is not 64 bit");
_Static_assert ((FS_TIME_T) -1 < 0    , "error: FS_TIME_T type is not signed");

// some error codes for the error flag in sol_pos
// errors are be combined with a binary OR
#define _FREESPA_DEU_OOR		0X01	// ΔUT1 out of range
#define _FREESPA_LON_OOR		0X02	// longitude out of range
#define _FREESPA_LAT_OOR		0X04	// latitude out of range
#define _FREESPA_ELE_OOR		0X08	// elevation out of range
#define _FREESPA_PRE_OOR		0X10	// pressure out of range
#define _FREESPA_TEM_OOR		0X20	// temperature out of range
#define _FREESPA_DIP_OOR		0X40	// geometric dip out of range
#define _FREESPA_GMTIMEF		0X80	// time conversion error 

// container struct for solar position data
typedef struct sol_pos {
	double z, a; // zenith, azimuth
	int E; // error flag
} sol_pos;

typedef struct solar_day {
	struct tm ev[11];
	FS_TIME_T t[11];
	double E[11];
	int status[11];
} solar_day;
			
// binary masks to enable/disable computing specific solar day events
#define _FREESPA_SUNRISE 0X01
#define _FREESPA_SUNSET  0X02
#define _FREESPA_CVDAWN  0X04
#define _FREESPA_CVDUSK  0X08
#define _FREESPA_NADAWN  0X10
#define _FREESPA_NADUSK  0X20
#define _FREESPA_ASDAWN  0X40
#define _FREESPA_ASDUSK  0X80

// binary mask variable to configure what solar events SolarDay computes
// default is all (0XFF)
extern int SDMASK;
// status flags
#define _FREESPA_EV_ERR       20
#define _FREESPA_EV_NA        10
#define _FREESPA_EV_OK         0
#define _FREESPA_EV_SUNABOVE   1
#define _FREESPA_EV_SUNBELOW  -1

// defines for the spring/autumn equinox and summer/winter solstice
#define _FREESPA_SPRINGEQ 0  
#define _FREESPA_SUMMERSO 1
#define _FREESPA_AUTUMNEQ 2  
#define _FREESPA_WINTERSO 3

// compute the real solar position
sol_pos SPA(struct tm *ut, double *delta_t, double delta_ut1, double lon, 
            double lat, double e);   
            
// correct for atmospheric refraction 
sol_pos ApSolposBennet(sol_pos P, double *gdip, double e, double p, double T);
sol_pos ApSolposBennetNA(sol_pos P, double *gdip, double e, double p, double T);

// compute true solar time
struct tm TrueSolarTime(struct tm *ut, double *delta_t, double delta_ut1, 
						double lon, double lat);

// compute the solar events
extern int SDMASK;
solar_day SolarDay(struct tm *ut, double *delta_t, double delta_ut1, 
                   double lon, double lat, double e, double *gdip, 
                   double p, double T, 
                   sol_pos (*refract)(sol_pos,double*,double,double,double));

// julian unix time routines
// get delta_t value from internal tables
double get_delta_t(struct tm *ut);

// populate a time struct with UTC from unix time
struct tm *gmjtime_r(FS_TIME_T *t, struct tm *ut);
struct tm *gmjtime(FS_TIME_T *t);
// create unix time from time struct
FS_TIME_T mkgmjtime(struct tm *ut);

// compute solstice/equinox times
struct tm *mkgmEQSOtime(struct tm *ut, int E, double *delta_t);
FS_TIME_T mkgmEQSOjtime(struct tm *ut, int E, double *delta_t);

// some internal test routines
int testjulian();
int testheliocentricpos();
#endif /* #ifndef _FREESPA_H_ */
