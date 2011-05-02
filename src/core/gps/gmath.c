/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: gmath.c 17 2008-03-11 11:56:11Z xtimor $
 *
 */

/*! \file gmath.h */

#include "gmath.h"

#include <math.h>
#include <float.h>

/**
 * \fn nmea_degree2radian
 * \brief Convert degree to radian
 */
double nmea_degree2radian( double val )
{ return ( val * NMEA_PI180 ); }

/**
 * \fn nmea_radian2degree
 * \brief Convert radian to degree
 */
double nmea_radian2degree( double val )
{ return ( val / NMEA_PI180 ); }

/**
 * \brief Convert NDEG (NMEA degree) to fractional degree
 */
double nmea_ndeg2degree( double val )
{
  double deg = (( int )( val / 100 ) );
  val = deg + ( val - deg * 100 ) / 60;
  return val;
}

/**
 * \brief Convert fractional degree to NDEG (NMEA degree)
 */
double nmea_degree2ndeg( double val )
{
  double int_part;
  double fra_part;
  fra_part = modf( val, &int_part );
  val = int_part * 100 + fra_part * 60;
  return val;
}

/**
 * \fn nmea_ndeg2radian
 * \brief Convert NDEG (NMEA degree) to radian
 */
double nmea_ndeg2radian( double val )
{ return nmea_degree2radian( nmea_ndeg2degree( val ) ); }

/**
 * \fn nmea_radian2ndeg
 * \brief Convert radian to NDEG (NMEA degree)
 */
double nmea_radian2ndeg( double val )
{ return nmea_degree2ndeg( nmea_radian2degree( val ) ); }

/**
 * \brief Calculate PDOP (Position Dilution Of Precision) factor
 */
double nmea_calc_pdop( double hdop, double vdop )
{
  return sqrt( pow( hdop, 2 ) + pow( vdop, 2 ) );
}

double nmea_dop2meters( double dop )
{ return ( dop * NMEA_DOP_FACTOR ); }

double nmea_meters2dop( double meters )
{ return ( meters / NMEA_DOP_FACTOR ); }

/**
 * \brief Calculate distance between two points
 * \return Distance in meters
 */
double nmea_distance(
  const nmeaPOS *from_pos,    /**< From position in radians */
  const nmeaPOS *to_pos       /**< To position in radians */
)
{
  double dist = (( double )NMEA_EARTHRADIUS_M ) * acos(
                  sin( to_pos->lat ) * sin( from_pos->lat ) +
                  cos( to_pos->lat ) * cos( from_pos->lat ) * cos( to_pos->lon - from_pos->lon )
                );
  return dist;
}

/**
 * \brief Calculate distance between two points
 * This function uses an algorithm for an oblate spheroid earth model.
 * The algorithm is described here:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 * \return Distance in meters
 */
double nmea_distance_ellipsoid(
  const nmeaPOS *from_pos,    /**< From position in radians */
  const nmeaPOS *to_pos,      /**< To position in radians */
  double *from_azimuth,       /**< (O) azimuth at "from" position in radians */
  double *to_azimuth          /**< (O) azimuth at "to" position in radians */
)
{
  /* All variables */
  double f, a, b, sqr_a, sqr_b;
  double L, phi1, phi2, U1, U2, sin_U1, sin_U2, cos_U1, cos_U2;
  double sigma, sin_sigma, cos_sigma, cos_2_sigmam, sqr_cos_2_sigmam, sqr_cos_alpha, lambda, sin_lambda, cos_lambda, delta_lambda;
  int remaining_steps;
  double sqr_u, A, B, delta_sigma;

  /* Check input */
  NMEA_ASSERT( from_pos != 0 );
  NMEA_ASSERT( to_pos != 0 );

  if (( from_pos->lat == to_pos->lat ) && ( from_pos->lon == to_pos->lon ) )
  { /* Identical points */
    if ( from_azimuth != 0 )
      *from_azimuth = 0;
    if ( to_azimuth != 0 )
      *to_azimuth = 0;
    return 0;
  } /* Identical points */

  /* Earth geometry */
  f = NMEA_EARTH_FLATTENING;
  a = NMEA_EARTH_SEMIMAJORAXIS_M;
  b = ( 1 - f ) * a;
  sqr_a = a * a;
  sqr_b = b * b;

  /* Calculation */
  L = to_pos->lon - from_pos->lon;
  phi1 = from_pos->lat;
  phi2 = to_pos->lat;
  U1 = atan(( 1 - f ) * tan( phi1 ) );
  U2 = atan(( 1 - f ) * tan( phi2 ) );
  sin_U1 = sin( U1 );
  sin_U2 = sin( U2 );
  cos_U1 = cos( U1 );
  cos_U2 = cos( U2 );

  /* Initialize iteration */
  sigma = 0;
  sin_sigma = sin( sigma );
  cos_sigma = cos( sigma );
  cos_2_sigmam = 0;
  sqr_cos_2_sigmam = cos_2_sigmam * cos_2_sigmam;
  sqr_cos_alpha = 0;
  lambda = L;
  sin_lambda = sin( lambda );
  cos_lambda = cos( lambda );
  delta_lambda = lambda;
  remaining_steps = 20;

  while (( delta_lambda > 1e-12 ) && ( remaining_steps > 0 ) )
  { /* Iterate */
    /* Variables */
    double tmp1, tmp2, sin_alpha, cos_alpha, C, lambda_prev;

    /* Calculation */
    tmp1 = cos_U2 * sin_lambda;
    tmp2 = cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda;
    sin_sigma = sqrt( tmp1 * tmp1 + tmp2 * tmp2 );
    cos_sigma = sin_U1 * sin_U2 + cos_U1 * cos_U2 * cos_lambda;
    sin_alpha = cos_U1 * cos_U2 * sin_lambda / sin_sigma;
    cos_alpha = cos( asin( sin_alpha ) );
    sqr_cos_alpha = cos_alpha * cos_alpha;
    cos_2_sigmam = cos_sigma - 2 * sin_U1 * sin_U2 / sqr_cos_alpha;
    sqr_cos_2_sigmam = cos_2_sigmam * cos_2_sigmam;
    C = f / 16 * sqr_cos_alpha * ( 4 + f * ( 4 - 3 * sqr_cos_alpha ) );
    lambda_prev = lambda;
    sigma = asin( sin_sigma );
    lambda = L +
             ( 1 - C ) * f * sin_alpha
             * ( sigma + C * sin_sigma * ( cos_2_sigmam + C * cos_sigma * ( -1 + 2 * sqr_cos_2_sigmam ) ) );
    delta_lambda = lambda_prev - lambda;
    if ( delta_lambda < 0 ) delta_lambda = -delta_lambda;
    sin_lambda = sin( lambda );
    cos_lambda = cos( lambda );
    remaining_steps--;
  }  /* Iterate */

  /* More calculation  */
  sqr_u = sqr_cos_alpha * ( sqr_a - sqr_b ) / sqr_b;
  A = 1 + sqr_u / 16384 * ( 4096 + sqr_u * ( -768 + sqr_u * ( 320 - 175 * sqr_u ) ) );
  B = sqr_u / 1024 * ( 256 + sqr_u * ( -128 + sqr_u * ( 74 - 47 * sqr_u ) ) );
  delta_sigma = B * sin_sigma * (
                  cos_2_sigmam + B / 4 * (
                    cos_sigma * ( -1 + 2 * sqr_cos_2_sigmam ) -
                    B / 6 * cos_2_sigmam * ( -3 + 4 * sin_sigma * sin_sigma ) * ( -3 + 4 * sqr_cos_2_sigmam )
                  ) );

  /* Calculate result */
  if ( from_azimuth != 0 )
  {
    double tan_alpha_1 = cos_U2 * sin_lambda / ( cos_U1 * sin_U2 - sin_U1 * cos_U2 * cos_lambda );
    *from_azimuth = atan( tan_alpha_1 );
  }
  if ( to_azimuth != 0 )
  {
    double tan_alpha_2 = cos_U1 * sin_lambda / ( -sin_U1 * cos_U2 + cos_U1 * sin_U2 * cos_lambda );
    *to_azimuth = atan( tan_alpha_2 );
  }

  return b * A *( sigma - delta_sigma );
}

/**
 * \brief Horizontal move of point position
 */
int nmea_move_horz(
  const nmeaPOS *start_pos,   /**< Start position in radians */
  nmeaPOS *end_pos,           /**< Result position in radians */
  double azimuth,             /**< Azimuth (degree) [0, 359] */
  double distance             /**< Distance (km) */
)
{
  nmeaPOS p1 = *start_pos;
  int RetVal = 1;

  distance /= NMEA_EARTHRADIUS_KM; /* Angular distance covered on earth's surface */
  azimuth = nmea_degree2radian( azimuth );

  end_pos->lat = asin(
                   sin( p1.lat ) * cos( distance ) + cos( p1.lat ) * sin( distance ) * cos( azimuth ) );
  end_pos->lon = p1.lon + atan2(
                   sin( azimuth ) * sin( distance ) * cos( p1.lat ), cos( distance ) - sin( p1.lat ) * sin( end_pos->lat ) );

  if ( NMEA_POSIX( isnan )( end_pos->lat ) || NMEA_POSIX( isnan )( end_pos->lon ) )
  {
    end_pos->lat = 0; end_pos->lon = 0;
    RetVal = 0;
  }

  return RetVal;
}

/**
 * \brief Horizontal move of point position
 * This function uses an algorithm for an oblate spheroid earth model.
 * The algorithm is described here:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 */
int nmea_move_horz_ellipsoid(
  const nmeaPOS *start_pos,   /**< Start position in radians */
  nmeaPOS *end_pos,           /**< (O) Result position in radians */
  double azimuth,             /**< Azimuth in radians */
  double distance,            /**< Distance (km) */
  double *end_azimuth         /**< (O) Azimuth at end position in radians */
)
{
  /* Variables */
  double f, a, b, sqr_a, sqr_b;
  double phi1, tan_U1, sin_U1, cos_U1, s, alpha1, sin_alpha1, cos_alpha1;
  double sigma1, sin_alpha, sqr_cos_alpha, sqr_u, A, B;
  double sigma_initial, sigma, sigma_prev, sin_sigma, cos_sigma, cos_2_sigmam, sqr_cos_2_sigmam, delta_sigma;
  int remaining_steps;
  double tmp1, phi2, lambda, C, L;

  /* Check input */
  NMEA_ASSERT( start_pos != 0 );
  NMEA_ASSERT( end_pos != 0 );

  if ( fabs( distance ) < 1e-12 )
  { /* No move */
    *end_pos = *start_pos;
    if ( end_azimuth != 0 ) *end_azimuth = azimuth;
    return !( NMEA_POSIX( isnan )( end_pos->lat ) || NMEA_POSIX( isnan )( end_pos->lon ) );
  } /* No move */

  /* Earth geometry */
  f = NMEA_EARTH_FLATTENING;
  a = NMEA_EARTH_SEMIMAJORAXIS_M;
  b = ( 1 - f ) * a;
  sqr_a = a * a;
  sqr_b = b * b;

  /* Calculation */
  phi1 = start_pos->lat;
  tan_U1 = ( 1 - f ) * tan( phi1 );
  cos_U1 = 1 / sqrt( 1 + tan_U1 * tan_U1 );
  sin_U1 = tan_U1 * cos_U1;
  s = distance;
  alpha1 = azimuth;
  sin_alpha1 = sin( alpha1 );
  cos_alpha1 = cos( alpha1 );
  sigma1 = atan2( tan_U1, cos_alpha1 );
  sin_alpha = cos_U1 * sin_alpha1;
  sqr_cos_alpha = 1 - sin_alpha * sin_alpha;
  sqr_u = sqr_cos_alpha * ( sqr_a - sqr_b ) / sqr_b;
  A = 1 + sqr_u / 16384 * ( 4096 + sqr_u * ( -768 + sqr_u * ( 320 - 175 * sqr_u ) ) );
  B = sqr_u / 1024 * ( 256 + sqr_u * ( -128 + sqr_u * ( 74 - 47 * sqr_u ) ) );

  /* Initialize iteration */
  sigma_initial = s / ( b * A );
  sigma = sigma_initial;
  sin_sigma = sin( sigma );
  cos_sigma = cos( sigma );
  cos_2_sigmam = cos( 2 * sigma1 + sigma );
  sqr_cos_2_sigmam = cos_2_sigmam * cos_2_sigmam;
  delta_sigma = 0;
  sigma_prev = 2 * NMEA_PI;
  remaining_steps = 20;

  while (( fabs( sigma - sigma_prev ) > 1e-12 ) && ( remaining_steps > 0 ) )
  { /* Iterate */
    cos_2_sigmam = cos( 2 * sigma1 + sigma );
    sqr_cos_2_sigmam = cos_2_sigmam * cos_2_sigmam;
    sin_sigma = sin( sigma );
    cos_sigma = cos( sigma );
    delta_sigma = B * sin_sigma * (
                    cos_2_sigmam + B / 4 * (
                      cos_sigma * ( -1 + 2 * sqr_cos_2_sigmam ) -
                      B / 6 * cos_2_sigmam * ( -3 + 4 * sin_sigma * sin_sigma ) * ( -3 + 4 * sqr_cos_2_sigmam )
                    ) );
    sigma_prev = sigma;
    sigma = sigma_initial + delta_sigma;
    remaining_steps --;
  } /* Iterate */

  /* Calculate result */
  tmp1 = ( sin_U1 * sin_sigma - cos_U1 * cos_sigma * cos_alpha1 );
  phi2 = atan2(
           sin_U1 * cos_sigma + cos_U1 * sin_sigma * cos_alpha1,
           ( 1 - f ) * sqrt( sin_alpha * sin_alpha + tmp1 * tmp1 )
         );
  lambda = atan2(
             sin_sigma * sin_alpha1,
             cos_U1 * cos_sigma - sin_U1 * sin_sigma * cos_alpha1
           );
  C = f / 16 * sqr_cos_alpha * ( 4 + f * ( 4 - 3 * sqr_cos_alpha ) );
  L = lambda -
      ( 1 - C ) * f * sin_alpha * (
        sigma + C * sin_sigma *
        ( cos_2_sigmam + C * cos_sigma * ( -1 + 2 * sqr_cos_2_sigmam ) )
      );

  /* Result */
  end_pos->lon = start_pos->lon + L;
  end_pos->lat = phi2;
  if ( end_azimuth != 0 )
  {
    *end_azimuth = atan2(
                     sin_alpha, -sin_U1 * sin_sigma + cos_U1 * cos_sigma * cos_alpha1
                   );
  }
  return !( NMEA_POSIX( isnan )( end_pos->lat ) || NMEA_POSIX( isnan )( end_pos->lon ) );
}

/**
 * \brief Convert position from INFO to radians position
 */
void nmea_info2pos( const nmeaINFO *info, nmeaPOS *pos )
{
  pos->lat = nmea_ndeg2radian( info->lat );
  pos->lon = nmea_ndeg2radian( info->lon );
}

/**
 * \brief Convert radians position to INFOs position
 */
void nmea_pos2info( const nmeaPOS *pos, nmeaINFO *info )
{
  info->lat = nmea_radian2ndeg( pos->lat );
  info->lon = nmea_radian2ndeg( pos->lon );
}
