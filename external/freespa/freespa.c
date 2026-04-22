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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "freespa.h"
#include "freespa_tables.h"
#include "freespa_dt_table.h"

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

/* defines */
#define JD0 2451545.0 // Julian day noon 1 January 2000 Universal Time
#define ETJD0 946728000 // unix time for julian day JD0
#define SUN_RADIUS 4.6542695162932789e-03 // in radians

#define EARTH_R 6378136.6 //m at the equator [IERS Numerical Standards (IAG 1999)] 
#define ABSOLUTEZERO -273.15 //convert C to K
#define AP0 1010.0 // standard sea level air pressure
#define AT0 10.0 // standard sea level air temperature


#define deg2rad(a) (M_PI*(a)/180.0)
#define rad2deg(a) (180.0*(a)/M_PI)

/* structures for internal use */
typedef struct JulianDate {
	double JD, JDE, JC, JCE, JME;
	int E; // signal error state
} JulianDay;
typedef struct GeoCentricSolPos {
	double lat, lon, rad;
} GeoCentricSolPos;


/* Julian Day 
 * 
 * see
 * Meeus, J., 1998. Astronomical Algorithms, second ed. Willmann-Bell, 
 * Inc., Richmond, Virginia, USA.
 * Pages 59-66
 */
 
 
/* extrapolate Δt
 * Morrison, & Stephenson provide the following equation for Δt
 * 
 *                    2
 *            ⎡y-1820⎤
 * Δt(y) = 32 ⎢──────⎥  - 20
 *            ⎣ 100  ⎦
 * 
 * where y is the year, and Δt is in seconds.
 * 
 * Morrison, L. V., & Stephenson, F. R. (2004). Historical Values of 
 * the Earth’s Clock Error ΔT and the Calculation of Eclipses. 
 * Journal for the History of Astronomy, 35(3), 327–336. 
 * https://doi.org/10.1177/002182860403500305
 */
#define DELATTEXTRAP(y) (32.0*(((y)-1820.0)/100)*(((y)-1820.0)/100)-20)

/* interpolate delta t */
double get_delta_t(struct tm *ut)
{
	double dyear;
	int imin=0, imax=NDT-1, i;
	
	dyear=(double)ut->tm_year+1900.0+((double)ut->tm_mon+1.0)/12+(double)(ut->tm_mday-1.0)/365.0;
	if (freespa_delta_t_table[0]>dyear)
		return DELATTEXTRAP(dyear);
	if (freespa_delta_t_table[2*imax]<dyear)
		return DELATTEXTRAP(dyear);
	
	while (imax-imin>1)
	{
		i=(imin+imax)/2;
		if (freespa_delta_t_table[2*i]>dyear)
			imax=i;
		else if (freespa_delta_t_table[2*i]<dyear)
			imin=i;
		else
			return freespa_delta_t_table[2*i+1];
	}
	return freespa_delta_t_table[2*imin+1]+(dyear-freespa_delta_t_table[2*imin])*(freespa_delta_t_table[2*imax+1]-freespa_delta_t_table[2*imin+1])/(freespa_delta_t_table[2*imax]-freespa_delta_t_table[2*imin]);
}


JulianDay MakeJulianDay(struct tm *ut, double *delta_t, double delta_ut1)
{
	int month, year;
	double day, a, dt;
	JulianDay JD={0};
	JD.E=0;
	day = (double)ut->tm_mday + ((double)ut->tm_hour+((double)ut->tm_min+((double)ut->tm_sec+delta_ut1)/60.0)/60.0)/24;
	month=ut->tm_mon+1;
	year=ut->tm_year+1900;
    if (month < 3)
    {
        month += 12;
        year--;
    }
	JD.JD=trunc(365.25*((double)year+4716.0))+trunc(30.6001*((double)month+1.0))+ day - 1524.5;
    if (JD.JD > 2299160.0)
    {
        a = trunc((double)year/100.0);
        JD.JD += (2 - a + trunc(a/4));
    }
	if (delta_t)
		dt=*delta_t;
	else
		dt=get_delta_t(ut);
	// Julian Ephemeris Day
	JD.JDE=JD.JD+dt/86400.0;
	JD.JC=(JD.JD-JD0)/36525.0;
	JD.JCE=(JD.JDE-JD0)/36525.0;
	JD.JME=JD.JCE/10.0;
	return JD;
}


int SetIntLimits(double v, int *t)
{
	if ((v>INT_MIN)&&(v<INT_MAX))
	{
		(*t)=(int)v;
		return 0;
	}
	(*t)=0;
	return 1;
}

// Julian Day to tm struct 
struct tm *JDgmtime(JulianDay JD, struct tm *ut)
{
	double A,B,C,D,F,G,I,Z;
	double d;
	Z=trunc(JD.JD+0.5);
	F=JD.JD-Z;
	if (Z<2299161)
		A=Z;
	else
	{
		B=trunc((Z-1867216.25)/36524.25);
		A=Z+1+B-trunc(B/4.0);
	}
	C=A+1524;
	D=trunc((C-122.1)/365.25);
	G=trunc(365.25*D);
	I=trunc((C-G)/30.6001);
	d=C-G-trunc(30.6001*I)+F-0.5; // day starts at 00:00 not at 12:00
	
	// day 
	ut->tm_mday=(int)trunc(d)+1;
	// month since jan (0..11)
	if (I<14)
		ut->tm_mon=(int)(I-2);
	else
		ut->tm_mon=(int)(I-14);
	// year since 1900	
	// year may overflow for 64bit FS_TIME_T, if it does return NULL
	if (ut->tm_mon>1)
	{
		if (SetIntLimits(D-4716-1900,&(ut->tm_year)))
			return NULL;
	}
	else
	{
		if (SetIntLimits(D-4715-1900,&(ut->tm_year)))
			return NULL;
	}
	d-=trunc(d);
	// d in days
	d*=86400;
	d=round(d);
	ut->tm_sec=((int)d)%60;
	d-=(double)ut->tm_sec;
	d/=60;
	ut->tm_min=((int)d)%60;
	d-=(double)ut->tm_min;
	d/=60;
	ut->tm_hour=((int)d)%60;
	d-=(double)ut->tm_hour;
	return ut;	
}

/* julian unix time routines
 * For modern day it should be equivalent to the standard routines
 * in time.h (apart from the fact that mkgmtime is absent on many 
 * platforms). However these routines have a 10-day gap between the 
 * Julian and Gregorian calendar where the Julian calendar ends on 
 * October 4, 1582 (JD = 2299160), and the next day the Gregorian 
 * calendar starts on October 15, 1582.
 * 
 * This definition of unix time makes it compatible with the julian day 
 * as it is computed from a date in freespa, i.e. the julian days all 
 * have 86400 seconds. 
 */
struct tm *gmjtime_r(FS_TIME_T *t, struct tm *ut)
{
	JulianDay J;
	J.JD=((double)((*t)-ETJD0)/86400.0)+JD0;
	return JDgmtime(J, ut);
}

struct tm *gmjtime(FS_TIME_T *t)
{
	static struct tm _tmbuf;
	return gmjtime_r(t, &_tmbuf);
}
// inverse of above
FS_TIME_T mkgmjtime(struct tm *ut)
{
	JulianDay J;
	J=MakeJulianDay(ut, NULL, 0);
	return (FS_TIME_T)round((J.JD-JD0)*86400)+ETJD0;
}

FS_TIME_T JDmkgmjtime(JulianDay J)
{
	return (FS_TIME_T)round((J.JD-JD0)*86400)+ETJD0;
}

JulianDay MakeJulianDayEpoch(FS_TIME_T t, double *delta_t, double delta_ut1)
{
	struct tm ut;
	struct tm *p;
	JulianDay JD;
	
	p=gmjtime_r(&t, &ut);
	
	if (!p)
	{
		JD.JD=0.0;
		JD.JDE=0.0;
		JD.JC=0.0;
		JD.JCE=0.0;
		JD.JME=0.0;
		JD.E=_FREESPA_GMTIMEF;
	}
	else
		JD=MakeJulianDay(&ut, delta_t, delta_ut1);
	return JD;
}

/* Heliocentric Earth Coordinate ***************************/
// corresponding tables with periodic terms are in freespa_tables.h
double SummPTerms(const p_term p[], int N, JulianDay JD)
{
    int i=0;
    double s=0;
    for (i=0;i<N;i++)
		s+=p[i].A*cos(p[i].P+p[i].W*JD.JME);
    return s;
}

double Heliocentric_lon(JulianDay JD)
{
	double lon, pp;
	lon=SummPTerms(EarthLon0, N_LON0, JD);
	pp=JD.JME;
	lon+=SummPTerms(EarthLon1, N_LON1, JD)*pp;
	pp*=JD.JME;
	lon+=SummPTerms(EarthLon2, N_LON2, JD)*pp;
	pp*=JD.JME;
	lon+=SummPTerms(EarthLon3, N_LON3, JD)*pp;
	pp*=JD.JME;
	lon+=SummPTerms(EarthLon4, N_LON4, JD)*pp;
	pp*=JD.JME;
	lon+=SummPTerms(EarthLon5, N_LON5, JD)*pp;
	lon/=1.0e8;
	return lon;
}

double Heliocentric_lat(JulianDay JD)
{
	double lat;
	lat=SummPTerms(EarthLat0, N_LAT0, JD);
	lat+=SummPTerms(EarthLat1, N_LAT1, JD)*JD.JME;
	lat/=1.0e8;
	return lat;
}

double Heliocentric_rad(JulianDay JD)
{
	double rad, pp;
	rad=SummPTerms(EarthRad0, N_RAD0, JD);
	pp=JD.JME;
	rad+=SummPTerms(EarthRad1, N_RAD1, JD)*pp;
	pp*=JD.JME;
	rad+=SummPTerms(EarthRad2, N_RAD2, JD)*pp;
	pp*=JD.JME;
	rad+=SummPTerms(EarthRad3, N_RAD3, JD)*pp;
	pp*=JD.JME;
	rad+=SummPTerms(EarthRad4, N_RAD4, JD)*pp;
	rad/=1.0e8;
	return rad;
}

/* Geocentric Sun Coordinate ***************************/
GeoCentricSolPos Geocentric_pos(JulianDay JD)
{
	GeoCentricSolPos P;
	P.lat=fmod(-Heliocentric_lat(JD), 2*M_PI);
	P.lon=fmod(Heliocentric_lon(JD)+M_PI, 2*M_PI);
	if (P.lon<0)
		P.lon+=2*M_PI;
	P.rad=Heliocentric_rad(JD);
	return P;
}


/* nutation and the obliquity of the ecliptic 
 * 
 * see 
 * Meeus, J., 1998. Astronomical Algorithms, second ed. Willmann-Bell, 
 * Inc., Richmond, Virginia, USA.
 * Pages 143-148
 */

/* utillity function to evaluate polynomials of arbitrary order
 * computes: y=a[0]*x^(N-1)+a[1]*x^(N-2)+...+a[N-1]
 * where a is an array of coefficients, N the length of the array and x
 * the value the polynomial is evaluated for.
 */
static inline double poly(const double a[], int N, double x)
{
	int i;
	double r;
	r=a[0];
	for (i=1;i<N;i++)
		r=a[i]+x*r;
	return r;
}	
/* here we define several 3rd order polynomials 
 * to compute nutation and the obliquity of the ecliptic 
 */
const double MEAN_ELONGATION_MOON_SUN[] = 	{deg2rad(1.0/189474.0) , deg2rad(-1.9142e-03), deg2rad(445267.11148) , deg2rad(297.85036)};
const double MEAN_ANOMALY_SUN[] 		= 	{deg2rad(-1.0/300000.0), deg2rad(-0.0001603) , deg2rad(35999.05034)  , deg2rad(357.52772)};
const double MEAN_ANOMALY_MOON[]		= 	{deg2rad(1.0/56250.0)  , deg2rad(0.0086972)  , deg2rad(477198.867398), deg2rad(134.96298)};
const double ARG_LAT_MOON[] 			=	{deg2rad(1.0/327270.0) , deg2rad(-0.0036825) , deg2rad(483202.017538), deg2rad(93.27191)};
const double ASC_LON_MOON[] 			=	{deg2rad(1.0/450000.0) , deg2rad(0.0020708)  , deg2rad(-1934.136261) , deg2rad(125.04452)};

void Nutation_lon_obliquity(JulianDay JD, double *del_psi, double *del_eps)
{
    int i, j;
    double sum, sum_psi=0, sum_eps=0;
    double x[5];
    x[0]=poly(MEAN_ELONGATION_MOON_SUN, 4, JD.JCE);
	x[1]=poly(MEAN_ANOMALY_SUN, 4,JD.JCE);
	x[2]=poly(MEAN_ANOMALY_MOON, 4,JD.JCE);
	x[3]=poly(ARG_LAT_MOON, 4,JD.JCE);
	x[4]=poly(ASC_LON_MOON, 4,JD.JCE);
    for (i = 0; i < NY; i++)
    {
        sum=0;
		for (j = 0; j < 5; j++)
			sum += x[j]*Y_Terms[i][j];
			
        sum_psi += (PE_Terms[i][0] + JD.JCE*PE_Terms[i][1])*sin(sum);
        sum_eps += (PE_Terms[i][2] + JD.JCE*PE_Terms[i][3])*cos(sum);
    }

    *del_psi = sum_psi * deg2rad(1/36000000.0);
    *del_eps = sum_eps * deg2rad(1/36000000.0);
    // actual obliquity computed in the main solpos routine
}

/* solpos - internal routine to compute the solar position
 * input: 
 *  - longitude (lon)
 *  - latitude (lat)
 *  - elevation (e)
 *  - pressure (p)
 *  - temperature (T)
 *  - Julian Date (JD)
 *  - Geocentric Solar Position GP
 * 
 * output:
 *  - sol_pos struct with the real solar position
 */
 
// some more polynomials
const double ECLIPTIC_MEAN_OBLIQUITY[] = {2.45,5.79,27.87,7.12,-39.05,-249.67,-51.38,1999.25,-1.55,-4680.93,84381.448};
const double GSTA[] = {deg2rad(-1/38710000.0),deg2rad(0.000387933),0.0,deg2rad(280.46061837)};
sol_pos solpos(double lon, double lat, double e, JulianDay JD, GeoCentricSolPos GP)
{
	double dtau, v, H, u, x, y;
	double lambda, alpha, delta, xi;
	double delta_prime, H_prime;
	//double aplpha_prime;
	double dpsi, deps, eps, dalpha;
	double h;
	sol_pos P;
	
	// aberation correction
	dtau=deg2rad(-20.4898/3600.0)/GP.rad;
	
	// nutation and the obliquity of the ecliptic
	Nutation_lon_obliquity(JD, &dpsi, &deps);
	eps=deps+poly(ECLIPTIC_MEAN_OBLIQUITY,11,JD.JME/10)*deg2rad(1/3600.0);
	
	// aparent sun longitude
	lambda=GP.lon+dpsi+dtau;
	
	// sidereal time at Greenwich 
	v=poly(GSTA,4,JD.JC)+deg2rad(360.98564736629)*(JD.JD - JD0);
	v+=dpsi*cos(eps);
	
	// sun right ascension
	alpha=atan2(sin(lambda)*cos(eps)-tan(GP.lat)*sin(eps),cos(lambda));
	if (alpha<0)
		alpha+=2*M_PI;
		
	// sun declination
	delta=asin(sin(GP.lat)*cos(eps)+cos(GP.lat)*sin(eps)*sin(lambda));
	
	// hour angle
	H=v+lon-alpha; 
	
	// equatorial horizontal parallax of the sun
	xi=deg2rad(8.794/3600.0)/GP.rad;
	// term u
	u=atan(0.99664719*tan(lat));
	// x & y
	
	x=cos(u)+e*cos(lat)/EARTH_R;
	y=0.99664719*sin(u)+e*sin(lat)/EARTH_R;
	
	// parallax in the sun right ascension
	dalpha=atan2(-x*sin(xi)*sin(H),cos(delta)-x*sin(xi)*cos(H));
	
	// topocentric sun right ascension
	//alpha_prime=alpha+dalpha;
	
	// topocentric sun declination
	delta_prime=atan2((sin(delta)-y*sin(xi))*cos(dalpha),cos(delta)-x*sin(xi)*cos(H));
	
	// topocentric local hour angle
	H_prime=H-dalpha;
	
	// topocentric elevation angle without atmospheric refraction correction
	h=asin(sin(lat)*sin(delta_prime)+cos(lat)*cos(delta_prime)*cos(H_prime));
	
	// compute sun zenith
	P.z=M_PI/2-h;
	
	// compute sun azimuth
	P.a=fmod(M_PI+atan2(sin(H_prime),cos(H_prime)*sin(lat)-tan(delta_prime)*cos(lat)),2*M_PI);
	
	// limit angular range
	P.z=fmod(P.z,2*M_PI);
	if (P.z<0)
	{
		P.z=-P.z;
		P.a=fmod(P.a+M_PI,2*M_PI);
	}
	if (P.z>M_PI)
	{
		P.z=2*M_PI-P.z;
		P.a=fmod(P.a+2*M_PI,2*M_PI);
	}
	return P;
}
// Equation of Time
// sun mean longitude polynomial
const double SMLON[] = {deg2rad(-1/2000000.0),deg2rad(-1/15300.0),deg2rad(1/49931.0),deg2rad(0.03032028),deg2rad(360007.6982779),deg2rad(280.4664567)};
double EoT(double lat, JulianDay JD, GeoCentricSolPos GP)
{
	double M, E;
	double dtau;
	double lambda, alpha, eps, deps, dpsi;
	
	
	// aberation correction
	dtau=deg2rad(-20.4898/3600.0)/GP.rad;
	
	// nutation and the obliquity of the ecliptic
	Nutation_lon_obliquity(JD, &dpsi, &deps);
	eps=deps+poly(ECLIPTIC_MEAN_OBLIQUITY,11,JD.JME/10)*deg2rad(1/3600.0);
	
	// aparent sun longitude
	lambda=GP.lon+dpsi+dtau;
	// sun right ascension
	alpha=atan2(sin(lambda)*cos(eps)-tan(GP.lat)*sin(eps),cos(lambda));
	M=poly(SMLON,6,JD.JME);
	E=fmod(M-deg2rad(0.0057183)-alpha+dpsi*cos(eps),2*M_PI);
	if (E>deg2rad(5))
		E-=2*M_PI;
	if (E<-deg2rad(5))
		E+=2*M_PI;
	return E;
}

/* check input values */
int InputCheck(double delta_ut1, double lon, 
            double lat, double e, double p, double T)
{
	int E=0;
	
	if ((delta_ut1<-1)||(delta_ut1>1))
		E|=_FREESPA_DEU_OOR;
		
	if ((lon<-M_PI)||(lon>M_PI))
		E|=_FREESPA_LON_OOR;
		
	if ((lat<-M_PI/2)||(lat>M_PI/2))
		E|=_FREESPA_LAT_OOR;
	
	if (e<-EARTH_R)
		E|=_FREESPA_ELE_OOR;
	
	if ((p<0)||(p>5000))
		E|=_FREESPA_PRE_OOR;
		
	if (T<ABSOLUTEZERO)
		E|=_FREESPA_TEM_OOR;
	return E;
}

/* SPA - exported routine to compute the solar position
 * input: 
 *  - ut			time struct with UTC
 *  - delta_t		pointer to Δt value, the difference between the 
 *                  Earth rotation time and the Terrestrial Time. If 
 * 					the pointer is NULL, use internal tables to find Δt.
 *  - delta_ut1 	is a fraction of a second that is added to the UTC 
 * 					to adjust for the irregular Earth rotation rate.
 *  - lon			observer longitude (radians)
 *  - lat 			observer latitude (radians)
 *  - e				observer elevation (m)
 *  - p				atmospheric pressure (mb)
 *  - T 			Temperature (C)
 * 
 * output:
 *  - sol_pos struct with the real solar position
 */
sol_pos SPA(struct tm *ut, double *delta_t, double delta_ut1, double lon, 
            double lat, double e)
{
	JulianDay D;
	GeoCentricSolPos G;
	sol_pos P;
	P.E=InputCheck(delta_ut1, lon, lat, e, 1010, 10);
	if (!P.E)
	{
		D=MakeJulianDay(ut, delta_t, delta_ut1);
		
		if (D.E)
		{
			P.E|=D.E;
			return P;
		}
		G=Geocentric_pos(D);
		P=solpos(lon,lat,e,D,G);
	}
	return P;
}


/* Atmospheric Refraction
 * 
 * see 
 * Meeus, J., 1998. Astronomical Algorithms, second ed. Willmann-Bell, 
 * Inc., Richmond, Virginia, USA.
 * Pages 105-108
 */

/* base form of Bennet's formula *and* that of its approximate reverse
 * i.e. we use the same functional form to compute from the true to 
 * aparent solar position, as we use to compute back.
 */
static inline double Refr(const double coeff[], double p, double T, double h)
{
	//converts true and aparent solar elevation
	return (p/AP0)*((AT0-ABSOLUTEZERO)/(T-ABSOLUTEZERO))*coeff[0]/tan(h+coeff[1]/(h+coeff[2]));
}

/* solar refraction according to Bennet
 * Used to convert the aparent solar position into the true solar 
 * position
 */
static inline double Bennet(double p, double T, double h)
{
	const double BENNET[] = {2.9088820866572158e-04,2.2267533386408395e-03,7.6794487087750510e-02};
	return Refr(BENNET, p, T, h);
}
/* solar refraction according to Saemundsson
 * Used to convert the true solar position into the aparent solar 
 * position, i.e. the approximate inverse of Bennet.
 * 
 * Note I did not get a hold of the original publication, only the 
 * reproduced equation in 
 * 
 * Meeus, J., 1998. Astronomical Algorithms, second ed. Willmann-Bell, 
 * Inc., Richmond, Virginia, USA.
 * 
 * Here the original equation is attributed to:
 * 
 * Saemundsson, Thorsteinn, "Atmospheric Refraction", Sky and 
 * Telescope, Vol 72, No. 1, page 70
 */
static inline double iBennet(double p, double T, double h)
{
	const double IBENNET[] = {2.9670597283903603e-04,3.1375594238030984e-03,8.9186324776910242e-02};
	/* B. Pieters 04.04.2023
	 * 
	 * following coefficients were determined on some arbitrary Tuesday 
	 * morning and appeared to provide a more accurate inverse Bennet 
	 * function. You can use it if you want but I suppose we better 
	 * stick to the published one above, or the BennetNA functions
	 * const double IBENNET[] = {2.5506322825243782e-04,2.7177487894134013e-03,8.9352063972327561e-02};
	 */
	return Refr(IBENNET, p, T, h);
}

/* Modified Bennet accoding to Hohenkerk
 * 
 * Equation published in:
 * Wilson, Teresa, "Evaluating the Effectiveness of Current Atmospheric 
 * Refraction Models in Predicting Sunrise and Sunset Times", Open 
 * Access Dissertation, Michigan Technological University, 2018.
 * https://doi.org/10.37099/mtu.dc.etdr/697
 * 
 * on page 26, Eq. 2.8. 
 * 
 * Wilson in turn attributes this to Hohenkerk:
 * Hohenkerk, C. Refraction: Bennetts Refraction Formulae, updated for 
 * use in The Nautical Almanac 2005 Onwards Internal memo, unpublished, 
 * HMNAO, Chilton, UK: Rutherford Appleton Laboratory, 2003.
 * 
 */
static inline double BennetNA(double p, double T, double h)
{
	const double BENNET[] = {2.9088820866572158e-04,2.2297995128387070e-03,7.5398223686155036e-02};
	return Refr(BENNET, p, T, h);
}
/* Approximate Inverse of BennetNA (own fit)
 */
static inline double iBennetNA(double p, double T, double h)
{
	const double IBENNET[] = {2.5561812083991283e-04,2.8037159466528061e-03,8.9542023921733521e-02};
	return Refr(IBENNET, p, T, h);
}

/* ApSolpos - correct for atmospheric refraction
 * input: 
 *  - refr. model	function pointer taking pressure, time, and aparent 
 * 					solar altitude. 
 *  - irefr.model	(approximate) inverse of above model
 *  - P 			Solar position struct with the real solar position 
 *  - gdip			pointer to a geometric dip value. If NULL the 
 * 					geometric dip is computed from the elevation 
 * 					assuming the horizon is at sea level (only if the 
 * 					elevation > 0)
 *  - e				observer elevation (m)
 *  - p				atmospheric pressure (mb)
 *  - T 			Temperature (C)
 * 
 * output:
 *  - sol_pos struct with the aparent solar position
 */
sol_pos ApSolpos(double (*refr)(double, double, double), double (*irefr)(double, double, double), sol_pos P, double *gdip, double e, double p, double T)
{
	double dip, h, dh=0, a_refr;
	P.E=InputCheck(0, 0, 0, e, p, T);
	if (gdip)
	{
		dip=*gdip;
		if (fabs(dip)>M_PI/2)
			P.E|=_FREESPA_DIP_OOR;
	}
	else
	{
		/* this Eq. for dip only holds for positive elevations!
		 * if you need a negative dip (you are in a pit) you need to
		 * provide your own dip value as I do not know how far the
		 * rim of the pit is...
		 * You can use the gdip pointer for that.
		 */
		dip=0;
		if (e>0)
			dip=acos(EARTH_R/(EARTH_R+e));
	}
	if (P.E)
	{
		P.z=0;
		P.a=0;
		return P;
	}	
	// given the aparent solar height at the horizon, Bennet gives the 
	// refraction angle
	a_refr=refr(p,T,-dip);
	h=M_PI/2-P.z;
	
	// compute if the sun is visible
	if (h>=-a_refr-SUN_RADIUS-dip)
		dh=irefr(p,T,h);
		
	// compute aparent sun zenith
	P.z=P.z-dh;
	P.z=fmod(P.z,2*M_PI);
	if (P.z<0)
	{
		P.z=-P.z;
		P.a=fmod(P.a+M_PI,2*M_PI);
	}
	if (P.z>M_PI)
	{
		P.z=2*M_PI-P.z;
		P.a=fmod(P.a+M_PI,2*M_PI);
	}
	return P;
}
/* Exported functions for atmospheric refraction. */
// original Bennet
sol_pos ApSolposBennet(sol_pos P, double *gdip, double e, double p, double T)
{
	return ApSolpos(&Bennet, &iBennet, P, gdip, e, p, T);
}

// modified Bennet
sol_pos ApSolposBennetNA(sol_pos P, double *gdip, double e, double p, double T)
{
	return ApSolpos(&BennetNA, &iBennetNA, P, gdip, e, p, T);
}

/* TrueSolarTime - exported routine to convert UTC to the local solar time
 * input: 
 *  - ut			time struct with UTC
 *  - delta_t		pointer to Δt value, the difference between the 
 *                  Earth rotation time and the Terrestrial Time. If 
 * 					the pointer is NULL, use internal tables to find Δt.
 *  - delta_ut1 	is a fraction of a second that is added to the UTC 
 * 					to adjust for the irregular Earth rotation rate.
 *  - lon			observer longitude (radians)
 *  - lat 			observer latitude (radians)
 * 
 * output:
 *  - tm struct with local solar time
 */
struct tm TrueSolarTime(struct tm *ut, double *delta_t, double delta_ut1, 
                        double lon, double lat)
{
	double E;
	struct tm nt={0};
	JulianDay D;
	GeoCentricSolPos G;
	if (InputCheck(delta_ut1, lon, lat, 0, 1, 10))
		return nt;
	D=MakeJulianDay(ut, delta_t, delta_ut1);
	G=Geocentric_pos(D);
	E=EoT(lat,D,G);	
	D.JD+=(lon+E)/M_PI/2; 
	JDgmtime(D, &nt);
	return nt;
}

/* Computing the Solar Day Events
 * I got tired of all those complicated algorithms to compute sunrise 
 * and sunset and other related events. Instead of complicated I opt 
 * for simple, modular, and iterative. The iterative makes it easy to
 * be accurate whilst keeping the code clean and simple, it just takes 
 * a bit more time to compute. This code is much easier to maintain and 
 * test.
 *  
 * I first compute a local transit time and the previous and next solar 
 * midnights. Using SPA we then find the maximal zenith at the previous
 * midnight, the minimum zenith angle at tranit and the maximum zenith 
 * for the next midnight.
 * From here on we can use fairly simple interative solvers for the 
 * various rise and set events (sunrise/sunset/civil dawn/civil dusk/
 * nautical dawn/nautical dusk/astronomical dawn/astronomical dusk)  
 */

/* FindSolTime:
 * To find the transit and mid night events we use the EoT routine with 
 * fixed point iterations.
 * FindSolTime: 
 * input:
 *  - t:			input unix time
 * 	- hour			target solar time
 * 	- min			target solar time
 * 	- sec			target solar time
 *  - delta_t		pointer to Δt value, the difference between the 
 *                  Earth rotation time and the Terrestrial Time. If 
 * 					the pointer is NULL, use internal tables to find Δt.
 *  - delta_ut1 	is a fraction of a second that is added to the UTC 
 * 					to adjust for the irregular Earth rotation rate.
 *  - lon			observer longitude (radians)
 *  - lat 			observer latitude (radians)
 * output:
 * 	- closest unix time for which the local true solar time was as 
 *    specified by the target solar time
 * 		
 */
// fraction of a day represented by one second.
#define FRACDAYSEC 1.1574074074074073e-05
#define MAX_FPITER 20
FS_TIME_T FindSolTime(FS_TIME_T t, int hour, int min, int sec, double *delta_t, 
                   double delta_ut1, double lon, double lat)
{
	double E;
	double dt=1;
	int iter=0;
	struct tm nt={0};
	JulianDay D, Dn;
	GeoCentricSolPos G;
	
	D=MakeJulianDayEpoch(t, delta_t, delta_ut1);
	// first estimate, ignore EoT
	gmjtime_r(&t, &nt);
	// compute fraction of a day to shift time to target
	dt=(double)(hour-nt.tm_hour)/24.4;
	dt+=(double)(min-nt.tm_min)/1440.0;
	dt+=(double)(sec-nt.tm_sec)/86400.0;
	// we limit dt to less than 0.5 days
	// this should keep the time from slipping into other days
	if (dt>0.5)
		dt-=1.0;
	if (dt<-0.5)
		dt+=1.0;
	D.JD+=dt; 
	
	dt=1;
	while ((fabs(dt)>FRACDAYSEC)&&(iter<MAX_FPITER))
	{
		// adapt julian day D
		Dn=D;
		G=Geocentric_pos(D);
		E=EoT(lat,D,G);	
		Dn.JD+=(lon+E)/M_PI/2; 
		JDgmtime(Dn, &nt);
		// compute fraction of a day to shift the time D
		dt=(double)(hour-nt.tm_hour)/24.4;
		dt+=(double)(min-nt.tm_min)/1440.0;
		dt+=(double)(sec-nt.tm_sec)/86400.0;
		if (dt>0.5)
			dt-=1.0;
		if (dt<-0.5)
			dt+=1.0;
		D.JD+=dt; 
		iter++;
	}
	return JDmkgmjtime(D);
}

/* FindSolZenith:
 * Find a time for which the sun is at a certain zenith within a solar 
 * day
 * input:
 *  - t1			unix time for a minimum or maximum zenith
 * 	- t2			unix time for the *subsequent* maximum or minimum
 * 					note that t2 should be about 0.5 days after t1
 * 	- z1			solar zenith angle for t1
 * 	- z2			solar zenith angle for t2
 *  - delta_t		pointer to Δt value, the difference between the 
 *                  Earth rotation time and the Terrestrial Time. If 
 * 					the pointer is NULL, use internal tables to find Δt.
 *  - delta_ut1 	is a fraction of a second that is added to the UTC 
 * 					to adjust for the irregular Earth rotation rate.
 *  - lon			observer longitude (radians)
 *  - lat 			observer latitude (radians)
 *  - e				observer elevation (m)
 *  - gdip			geometric dip, i.e. how far the horizon is below the 
 *                  observer (in rad). If this pointer is NULL the 
 *                  geometric dip is computed from the observer elevation
 *                  (assuming the horizon is at sea level)
 *  - p				atmospheric pressure (mb)
 *  - T 			Temperature (C)
 *  - z 			Target Zenith
 * 
 * output:
 * 	- tz			unix time at which the zenith was as specified
 * 
 * return value:
 * 0				All OK
 * 1				NA, solar zenith always lower than target
 * -1				NA, solar zenith always higher than target		
 */
// accuracy of solar rise/set events
#define Z_EPS deg2rad(0.05)
#define MAXRAT 2
#define Z_MAXITER 100
int FindSolZenith(FS_TIME_T t1, FS_TIME_T t2, double z1, double z2, double *delta_t, 
                  double delta_ut1, double lon, double lat, double e, double *gdip, 
                  double p, double T, 
                  sol_pos (*refract)(sol_pos,double*,double,double,double), 
                  double z, FS_TIME_T *tz, double *E)
{
	double a, b, w, R;
	sol_pos P;
	double zmin, zmax, eb;
	FS_TIME_T tt, tmin, tmax, tb;
	struct tm ut={0}, *put;
	int iter=0;
	
	(*tz)=0;
	(*E)=0;
	if ((z<z1)&&(z<z2))
		return -1; // sun always below z
	if ((z>z1)&&(z>z2))
		return 1; // sun always above z
		
	/* We make a first guess by assuming the following function for the
	 * solar zenith:
	 * z(t) = a + b cos(ω (t-t1))
	 * 
	 * as at t1 and t2 the zenith is in a maximum/minimum we set:
	 *       π
	 * ω = ─────
	 *     t2-t1
	 * 
	 * Further, we find:
	 * a = -(cos(t2*w)*z1-cos(t1*w)*z2)/(cos(t1*w)-cos(t2*w))
	 * b = (z1-z2)/(cos(t1*w)-cos(t2*w))
	 * 
	 * we can then estimate the moment for the desired zenith as:
	 * 
	 *          arccos(z/b-a/b)
	 * t = t1 + ───────────────
	 *                 ω
	 */
	w=M_PI/((double)(t2-t1));
	b=(cos((double)t1*w)-cos((double)t2*w));
	a=-(cos((double)t2*w)*z1-cos((double)t1*w)*z2)/b;
	b=(z1-z2)/b;
	
	R=(double)(2*((z2<z1))-1); // sun rising (R=1.0) or falling (R=-1.0)
	// first guess:
	tt=t1+(FS_TIME_T)round(acos(z/b-a/b)/w);
	if ((tt<t1)||(tt>t2))
		tt=(t1+t2)/2;
	put=gmjtime_r(&tt, &ut);
	P=SPA(put, delta_t, delta_ut1, lon, lat, e);
	P=refract(P,gdip,e,p,T);
	tb=tt;
	eb=P.z-z;
	if (fabs(P.z-z)<Z_EPS)
	{
		(*E)=eb;
		(*tz)=tb;
		return 0;
	}
	
	if (R*(P.z-z)>0)
	{
		// tt too small
		tmin=tt;
		zmin=P.z;
		tmax=t2;
		zmax=z2;
	}
	else
	{
		tmax=tt;
		zmax=P.z;
		tmin=t1;
		zmin=z1;
	}
	/* use bisection */
	while ((tmax-tmin>1)&&(iter<Z_MAXITER))
	{
		/* bisection */
		tt=(FS_TIME_T)round(((z-zmin)*(double)tmax+(zmax-z)*(double)tmin)/((z-zmin)+(zmax-z)));
		if ((tt<t1)||(tt>t2))
			tt=(t1+t2)/2;
		// avoid all too asymmetrical brackets
		if (((tt-tmin)>MAXRAT*(tmax-tt))||(MAXRAT*(tt-tmin)<(tmax-tt)))
			tt=(tmin+tmax)/2;
		put=gmjtime_r(&tt, &ut);
		P=SPA(put, delta_t, delta_ut1, lon, lat, e);
		P=refract(P,gdip,e,p,T);
		if (fabs(P.z-z)<fabs(eb))
		{
			eb=P.z-z;
			tb=tt;
		}
		if (fabs(eb)<Z_EPS)
		{
			(*E)=eb;
			(*tz)=tb;
			return 0;
		}
		
		if (R*(P.z-z)>0)
		{
			// tt too small
			tmin=tt;
			zmin=P.z;
		}
		else
		{
			tmax=tt;
			zmax=P.z;
		}
		iter++;
	}
	(*E)=eb;
	(*tz)=tb;
	return 0;
}

/* solar_day strcut lists 11 solar events on a day
 * Index	Event
 * 0:		solar midnight before time t
 * 1:		solar transit closest to time t
 * 2:		solar midnight after time t
 * 3:		sunrise
 * 4:		sunset
 * 5:		civil dawn
 * 6:		civil dusk
 * 7:		nautical dawn
 * 8:		nautical dusk
 * 9:		astronomical dawn
 * 10:		astronomical dusk
 * The followin routine finds em.
 */
/* binary mask to enable/disable computing specific events
 * i.e. if you only care about sunrise and sunset you may set it to:
 * SDMASK=(_FREESPA_SUNRISE|_FREESPA_SUNSET);
 * and save some computational effort for the rest
 * Note the solar midnight before, solar transit, and solar midnight 
 *      after, are always computed.
 */
int SDMASK=(_FREESPA_SUNRISE|_FREESPA_SUNSET|_FREESPA_CVDAWN|_FREESPA_CVDUSK|_FREESPA_NADAWN|_FREESPA_NADUSK|_FREESPA_ASDAWN|_FREESPA_ASDUSK);
solar_day SolarDay(struct tm *ut, double *delta_t, double delta_ut1, 
                   double lon, double lat, double e, double *gdip, 
                   double p, double T, 
                   sol_pos (*refract)(sol_pos,double*,double,double,double))
{
	solar_day D={0};
	FS_TIME_T t, tp, tc, tn;
	sol_pos Pp, Pc, Pn;
	double dip;
	struct tm *put;
	int i;
	
	for (i=0;i<11;i++)
		D.status[i]=_FREESPA_EV_NA; 
	
	if (InputCheck(delta_ut1, lon, lat, e, p, T))
		return D;
	
	if (!refract) // no refraction function provided. For simplicity we fall back to bennet in vacuum.
	{
		refract=&ApSolposBennet;
		p=0;
	}
	
	t=mkgmjtime(ut);
	// find the closest transit
	tc=FindSolTime(t, 12, 0, 0, delta_t, delta_ut1, lon, lat);
	// previous low point (approx 0.5 day before transit)
	tp=FindSolTime(tc-43200, 0, 0, 0, delta_t, delta_ut1, lon, lat);
	// next low point (approx 0.5 day after transit)
	tn=FindSolTime(tc+43200, 0, 0, 0, delta_t, delta_ut1, lon, lat);
	
	// previous low
	put=gmjtime_r(&tp, D.ev);
	D.t[0]=tp;
	D.status[0]=_FREESPA_EV_OK;
	D.E[0]=NAN;
	Pp=SPA(put, delta_t, delta_ut1, lon, lat, e);
	Pp=refract(Pp,gdip,e,p,T);
	
	// current transit
	put=gmjtime_r(&tc, D.ev+1);
	D.t[1]=tc;
	D.status[1]=_FREESPA_EV_OK;
	D.E[1]=NAN;
	Pc=SPA(put, delta_t, delta_ut1, lon, lat, e);
	Pc=refract(Pc,gdip,e,p,T);
	
	// next low
	put=gmjtime_r(&tn, D.ev+2);
	D.t[2]=tn;
	D.status[2]=_FREESPA_EV_OK;
	D.E[2]=NAN;
	Pn=SPA(put, delta_t, delta_ut1, lon, lat, e);
	Pn=refract(Pn,gdip,e,p,T);
	
	// geometric dip
	if (gdip)
	{
		dip=*gdip;
		if (fabs(dip)>M_PI/2)
		{
			for (i=0;i<11;i++)
				D.status[i]=_FREESPA_EV_ERR; 
			return D;
		}
	}
	else
	{
		dip=0;
		if (e>0)
			dip=acos(EARTH_R/(EARTH_R+e));
	}
		
		
	// compute events
	i=3;	
	if (SDMASK&_FREESPA_SUNRISE)
	{
		D.status[i]=FindSolZenith(tp, tc, Pp.z, Pc.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2+SUN_RADIUS, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	if (SDMASK&_FREESPA_SUNSET)
	{
		D.status[i]=FindSolZenith(tc, tn, Pc.z, Pn.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2+SUN_RADIUS, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	
	dip+=deg2rad(6);
	if (SDMASK&_FREESPA_CVDAWN)
	{
		D.status[i]=FindSolZenith(tp, tc, Pp.z, Pc.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	if (SDMASK&_FREESPA_CVDUSK)
	{
		D.status[i]=FindSolZenith(tc, tn, Pc.z, Pn.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	
	dip+=SUN_RADIUS+deg2rad(6);
	if (SDMASK&_FREESPA_NADAWN)
	{
		D.status[i]=FindSolZenith(tp, tc, Pp.z, Pc.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	if (SDMASK&_FREESPA_NADUSK)
	{
		D.status[i]=FindSolZenith(tc, tn, Pc.z, Pn.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	
	dip+=deg2rad(6);
	if (SDMASK&_FREESPA_ASDAWN)
	{
		D.status[i]=FindSolZenith(tp, tc, Pp.z, Pc.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	if (SDMASK&_FREESPA_ASDUSK)
	{
		D.status[i]=FindSolZenith(tc, tn, Pc.z, Pn.z, delta_t, delta_ut1, lon, lat, e, gdip, p, T, refract, dip+M_PI/2, D.t+i, D.E+i);
		put=gmjtime_r(D.t+i, D.ev+i);
	}
	i++;
	return D;
	
}

/* code to compute equinox and solstice events */
/* Tables from Meeus, page 178 */
/* Table 27.A, for the years -1000 to 1000 */
const double JDME0_EQSO[4][5]={
	{1721139.29189, 365242.13740, 0.06134, 0.00111, 0.00071}, // SPRINGEQ
	{1721233.25401, 365241.72562, 0.05323, 0.00907, 0.00025}, // SUMMERSO
	{1721325.70455, 365242.49558, 0.11677, 0.00297, 0.00074}, // AUTUMNEQ
	{1721414.39987, 365242.88257, 0.00769, 0.00933, 0.00006}, // WINTERSO
};
/* Table 27.B, for the years 1000 to 3000 */
const double JDME2000_EQSO[4][5]={
	{2451623.80984, 365242.37404, 0.05169, 0.00411, 0.00057}, // SPRINGEQ
	{2451716.56767, 365241.62603, 0.00325, 0.00888, 0.00030}, // SUMMERSO
	{2451810.21715, 365242.01767, 0.11575, 0.00337, 0.00078}, // AUTUMNEQ
	{2451900.05952, 365242.74049, 0.06223, 0.00823, 0.00032}, // WINTERSO
};

double Mean_EQSO(struct tm *ut, int E) // polynomial approximation JDE
{
	double *table;
	double y, JDE;
	// select correct table
	if (ut->tm_year<-900)
	{
		y=((double)ut->tm_year+1900.0)/1000;
		table=(double *)JDME0_EQSO[E];
	}
	else
	{
		y=((double)ut->tm_year-100.0)/1000;
		table=(double *)JDME2000_EQSO[E];
	}
	return table[0]+y*(table[1]+y*(table[2]+y*(table[3]+y*table[4])));
}
double PertubationTerms_EQSO(double T)
{
	// pertubation terms in rad!
	const double PT[24][3] = {
		{485,5.6716219372807721e+00,3.3757041381353048e+01},
		{203,5.8857738365004781e+00,5.7533848531501758e+02},
		{199,5.9704223052222014e+00,3.5231216280757538e-01},
		{182,4.8607419668042079e-01,7.7713771552463541e+03},
		{156,1.2765338149086527e+00,7.8604194554533876e+02},
		{136,2.9935887330206743e+00,3.9302097277266938e+02},
		{77,3.8840557173881809e+00,1.1506769706300352e+03},
		{74,5.1787409565175748e+00,5.2969102188531025e+01},
		{70,4.2512729920077881e+00,1.5773435804179030e+02},
		{58,2.0910789768144062e+00,5.8849268282144840e+02},
		{52,5.1865949381515497e+00,2.6298272103200158e+00},
		{50,3.6686820876920800e-01,3.9814904682100170e+01},
		{45,4.3203880303867628e+00,5.2236940057977904e+02},
		{44,5.6749380628595620e+00,5.5075533081445974e+02},
		{29,1.0634291132401450e+00,7.7552256689088878e+01},
		{18,2.7073547356936039e+00,1.1790629008647159e+03},
		{17,5.0403363468344242e+00,7.9629809364200341e+01},
		{16,3.4564500506495701e+00,1.0977078858947966e+03},
		{14,3.4864697137838725e+00,5.4867777813934822e+02},
		{12,1.6648695734773908e+00,2.5443144545527034e+02},
		{12,5.0110148154009195e+00,5.5731427814345443e+02},
		{12,5.5991907733230084e+00,6.0697767436883066e+02},
		{9,3.9746383055666867e+00,2.1329913134717980e+01},
		{8,2.6965336943312390e-01,2.9424635013737048e+02},
	};
	int i;
	double s=0;
	for (i=0;i<24;i++)
		s+=PT[i][0]*cos(PT[i][1]+T*PT[i][2]);
	return s;
}

JulianDay EQSO_i(struct tm *ut, int E, double *delta_t)
{
	JulianDay JD={0};
	double T, W, L,S, JDE, dt;
	if ((E<0)||(E>3))
	{
		JD.E=_FREESPA_GMTIMEF; // is this OK?
		return JD;
	}
	JDE=Mean_EQSO(ut, E);
	
	T=(JDE-JD0)/36525;
	W=6.2830759e+02*T-4.311e-02;
	L=1+0.0334*cos(W)+0.0007*cos(2*W);
	S=PertubationTerms_EQSO(T);
	JDE=JDE+0.00001*S/L;
	
	// compute delta t
	dt=get_delta_t(ut);
	// populate struct
	JD.JD=JDE-dt/86400.0; // is this right, the JDE from this algorithm is julian day?
	JD.JDE=JDE; 
	JD.JC=(JD.JD-JD0)/36525.0;
	JD.JCE=(JD.JDE-JD0)/36525.0;
	JD.JME=JD.JCE/10.0;
	return JD;
}


struct tm *mkgmEQSOtime(struct tm *ut, int E, double *delta_t)
{
	return JDgmtime(EQSO_i(ut, E, delta_t), ut);
}
FS_TIME_T mkgmEQSOjtime(struct tm *ut, int E, double *delta_t)
{
	return JDmkgmjtime(EQSO_i(ut, E, delta_t));
}

/* some unit tests */
// unit test julian date computation with a table of dates
int testjulian()
{
	int ERR=0;
	int i, N=4;
	FS_TIME_T dates[] = {946728000, 915148800, 538704000, -11676096000};
	double JDs[] = {2451545.0, 2451179.5, 2446822.5, 2305447.5};
    char* timestr;
#define JDEPS (1e-3/(24*60*60)) // 1 ms accuracy 
	JulianDay D;
	struct tm ut={0};
	struct tm *p=NULL;
	timestr=malloc(80*sizeof(char));
	for (i=0;i<N;i++)
	{
		p=gmjtime_r(&dates[i], &ut);	
		if (strftime(timestr, 80, "%Y-%m-%d %H:%M:%S %Z",p)>0)
			printf("testing %s", timestr);
		else
			fprintf(stderr,"Error: timestring exceeds 80 bytes\n");
		D=MakeJulianDayEpoch(dates[i], 0,0);
		printf("--> JD:%.1f\n", D.JD);
		
		if (fabs(D.JD-JDs[i])>JDEPS)
		{
			fprintf(stderr, "Error: julian day computation is off by more than %e\n", JDEPS);
			fprintf(stderr, "Failed for EPOCH %ld\n", dates[i]);
			fprintf(stderr, "got %.16e\n", D.JD);
			fprintf(stderr, "should be %.16e\n", JDs[i]);
			fprintf(stderr, "diff %.16e\n", D.JD-JDs[i]);
			ERR=1;
		}
	}
	free(timestr);
	return ERR;
}

// unit test heliocentric coordinate
int testheliocentricpos()
{
	int ERR=0;
	FS_TIME_T date =1066419030;
	double JD = 2452930.312847222;
	double lat=-1.7649101008724539e-06, lon=4.1919774712578828e-01, rad=0.9965422974;// all angles in radians
	double delta_t=67.0;
	double delta_ut1=0;
	double Hlon,Hlat, HR;
	JulianDay D;
	D=MakeJulianDayEpoch(date, &delta_t, delta_ut1);

	if (fabs(D.JD-JD)>JDEPS)
	{
		fprintf(stderr, "Error: julian day computation is off by more than %e\n", JDEPS);
		fprintf(stderr, "Failed for EPOCH %ld\n", date);
		fprintf(stderr, "got %e\n", D.JD);
		fprintf(stderr, "should be %e\n", JD);
		ERR=1;
	}
	Hlon=fmod(Heliocentric_lon(D), M_PI);
	if (fabs(Hlon-lon)>1e-6)
	{
		fprintf(stderr, "Error: heliocentric longitude deviates by %e rad.\n", Hlon-lon);
		fprintf(stderr, "       got %e, should be %e\n", Hlon,lon);
		ERR=1;
	}
	Hlat=fmod(Heliocentric_lat(D), M_PI);
	if (fabs(Hlat-lat)>1e-6)
	{
		fprintf(stderr, "Error: heliocentric latitude deviates by %e rad.\n", Hlat-lat);
		fprintf(stderr, "       got %e, should be %e\n", Hlat,lat);
		ERR=1;
	}
	HR=Heliocentric_rad(D);
	if (fabs(HR-rad)>1e-6)
	{
		fprintf(stderr, "Error: heliocentric radius deviates by %e AU.\n", HR-rad);
		fprintf(stderr, "       got %e, should be %e\n", HR,rad);
		ERR=1;
	}
	
	return ERR;
}




