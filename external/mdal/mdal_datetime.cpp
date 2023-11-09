/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "mdal_datetime.hpp"
#include "mdal_utils.hpp"


constexpr double MILLISECONDS_IN_SECOND = 1000;
constexpr double MILLISECONDS_IN_MINUTE = 1000 * 60;
constexpr double MILLISECONDS_IN_HOUR = 1000 * 60 * 60;
constexpr double MILLISECONDS_IN_DAY = 1000 * 60 * 60 * 24;
constexpr double MILLISECONDS_IN_WEEK = 1000 * 60 * 60 * 24 * 7;

//https://www.unidata.ucar.edu/software/netcdf-java/current/CDM/CalendarDateTime.html
constexpr double MILLISECONDS_IN_EXACT_YEAR = 3.15569259747e10; //CF Compliant
constexpr double MILLISECONDS_IN_MONTH_CF = MILLISECONDS_IN_EXACT_YEAR / 12.0; //CF Compliant

MDAL::DateTime::DateTime() = default;

MDAL::DateTime::DateTime( int year, int month, int day, int hours, int minutes, double seconds, MDAL::DateTime::Calendar calendar )
{
  DateTimeValues value{year, month, day, hours, minutes, seconds};

  switch ( calendar )
  {
    case MDAL::DateTime::Gregorian:
      setWithGregorianJulianCalendarDate( value );
      break;
    case MDAL::DateTime::ProlepticGregorian:
      setWithGregorianCalendarDate( value );
      break;
    case MDAL::DateTime::Julian:
      setWithJulianCalendarDate( value );
      break;
  }
}

MDAL::DateTime::DateTime( double value, Epoch epoch ):  mValid( true )
{
  switch ( epoch )
  {
    case MDAL::DateTime::Unix:
      mJulianTime = ( DateTime( 1970, 01, 01, 0, 0, 0, Gregorian ) + RelativeTimestamp( value, RelativeTimestamp::seconds ) ).mJulianTime;
      break;
    case MDAL::DateTime::JulianDay:
      mJulianTime = int64_t( value * MILLISECONDS_IN_DAY + 0.5 );
      break;
  }
}

MDAL::DateTime::DateTime( const std::string &fromISO8601 )
{
  std::vector<std::string> splitedDateTime = split( fromISO8601, 'T' );

  if ( splitedDateTime.size() != 2 )
    return;
  //parse date
  std::vector<std::string> splitedDate = split( splitedDateTime.at( 0 ), '-' );
  if ( splitedDate.size() != 3 )
    return;

  //parse time
  splitedDateTime[1] = replace( splitedDateTime.at( 1 ), "Z", "", ContainsBehaviour::CaseInsensitive );
  std::vector<std::string> splitedTime = split( splitedDateTime.at( 1 ), ':' );
  if ( splitedTime.size() < 2 || splitedTime.size() > 3 )
    return;

  DateTimeValues dateTimeValues;
  dateTimeValues.year = toInt( splitedDate[0] );
  dateTimeValues.month = toInt( splitedDate[1] );
  dateTimeValues.day = toInt( splitedDate[2] );
  dateTimeValues.hours = toInt( splitedTime[0] );
  dateTimeValues.minutes = toInt( splitedTime[1] );
  if ( splitedTime.size() == 3 )
    dateTimeValues.seconds = toDouble( splitedTime[2] );
  else
    dateTimeValues.seconds = 0.0;

  setWithGregorianCalendarDate( dateTimeValues );
}

std::string MDAL::DateTime::toStandardCalendarISO8601() const
{
  if ( mValid )
  {
    DateTimeValues value = dateTimeGregorianProleptic();
    if ( value.year > 0 )
      return toString( value );
  }

  return "";
}

double MDAL::DateTime::toJulianDay() const
{
  return mJulianTime / MILLISECONDS_IN_DAY;
}

std::string MDAL::DateTime::toJulianDayString() const
{
  return std::to_string( toJulianDay() );
}

std::vector<int> MDAL::DateTime::expandToCalendarArray() const
{
  std::vector<int> dateTimeArray( 6, 0 );
  if ( mValid )
  {
    DateTimeValues value = dateTimeGregorianProleptic();
    dateTimeArray[0] = value.year;
    dateTimeArray[1] = value.month;
    dateTimeArray[2] = value.day;
    dateTimeArray[3] = value.hours;
    dateTimeArray[4] = value.minutes;
    dateTimeArray[5] = int( value.seconds + 0.5 );
  }

  return dateTimeArray;
}


MDAL::DateTime MDAL::DateTime::operator+( const MDAL::RelativeTimestamp &duration ) const
{
  if ( !mValid )
    return DateTime();
  return DateTime( mJulianTime + duration.mDuration );
}


MDAL::DateTime MDAL::DateTime::operator-( const MDAL::RelativeTimestamp &duration ) const
{
  if ( !mValid )
    return DateTime();
  return DateTime( mJulianTime - duration.mDuration );
}

bool MDAL::DateTime::operator==( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return true;

  return ( mValid && other.mValid ) && ( mJulianTime == other.mJulianTime );
}

bool MDAL::DateTime::operator!=( const MDAL::DateTime &other ) const
{
  return ! operator==( other );
}

bool MDAL::DateTime::operator<( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return false;
  return ( mValid && other.mValid ) && ( mJulianTime < other.mJulianTime );
}

bool MDAL::DateTime::isValid() const { return mValid; }

MDAL::DateTime::DateTime( int64_t julianTime ): mJulianTime( julianTime ), mValid( true )
{}

/*
MDAL::DateTime::DateTimeValues MDAL::DateTime::dateTimeGregorianJulianCalendar() const
{
  // https://fr.wikipedia.org/wiki/Jour_julien
  DateTimeValues values;
  int Z = int( mJulianTime / MILLISECONDS_IN_DAY + 0.5 ); // integer part of julian days count
  double F = ( mJulianTime - MILLISECONDS_IN_DAY * ( Z - 0.5 ) ) / MILLISECONDS_IN_DAY; // fractional part of julian days count;
  int S;

  if ( Z < 2299161 )
    S = Z;
  else
  {
    int alpha = int( ( Z - 1867216.25 ) / 36524.25 );
    S = Z + 1 + alpha - int( alpha / 4 );
  }

  int B = S + 1524;
  int C = int( ( B - 122.1 ) / 365.25 );
  int D = int( 365.25 * C );
  int E = int( ( B - D ) / 30.6001 );

  values.day = B - D - int( 30.6001 * E );
  if ( E < 14 )
    values.month = E - 1;
  else
    values.month = E - 13;

  if ( values.month > 2 )
    values.year = C - 4716;
  else
    values.year = C - 4715;

  values.hours = int( F / MILLISECONDS_IN_HOUR );
  F = int( F - values.hours * MILLISECONDS_IN_HOUR );
  values.minutes = int( F / MILLISECONDS_IN_MINUTE );
  F = int( F  - values.minutes * MILLISECONDS_IN_MINUTE );
  values.seconds = int( F / MILLISECONDS_IN_SECOND );

  return values;
}
*/

MDAL::DateTime::DateTimeValues MDAL::DateTime::dateTimeGregorianProleptic() const
{
  // https://fr.wikipedia.org/wiki/Jour_julien
  DateTimeValues values;
  int Z = int( mJulianTime / MILLISECONDS_IN_DAY + 0.5 ); // integer part of julian days count
  int F = int( mJulianTime - MILLISECONDS_IN_DAY * ( Z - 0.5 ) ) ; // fractional part of julian days count in ms;

  int alpha = int( ( Z - 1867216.25 ) / 36524.25 );
  int S = Z + 1 + alpha - int( alpha / 4 );

  int B = S + 1524;
  int C = int( ( B - 122.1 ) / 365.25 );
  int D = int( 365.25 * C );
  int E = int( ( B - D ) / 30.6001 );

  values.day = B - D - int( 30.6001 * E );
  if ( E < 14 )
    values.month = E - 1;
  else
    values.month = E - 13;

  if ( values.month > 2 )
    values.year = C - 4716;
  else
    values.year = C - 4715;

  values.hours = int( F / MILLISECONDS_IN_HOUR );
  F = int( F - values.hours * MILLISECONDS_IN_HOUR );
  values.minutes = int( F / MILLISECONDS_IN_MINUTE );
  F = int( F  - values.minutes * MILLISECONDS_IN_MINUTE );
  values.seconds = int( F / MILLISECONDS_IN_SECOND );

  return values;
}


void MDAL::DateTime::setWithGregorianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  // https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  int A = values.year / 100;
  int B = A / 4;
  int C = 2 - A + B;
  int E = int( 365.25 * ( values.year + 4716 ) );
  int F = int( 30.6001 * ( values.month + 1 ) );
  double julianDay = C + values.day + E + F - 1524.5;

  mValid = true;
  mJulianTime = int64_t( julianDay * MILLISECONDS_IN_DAY +
                         ( values.hours ) * MILLISECONDS_IN_HOUR +
                         values.minutes * MILLISECONDS_IN_MINUTE +
                         values.seconds * MILLISECONDS_IN_SECOND );
}

void MDAL::DateTime::setWithJulianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  // https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  int E = int( 365.25 * ( values.year + 4716 ) );
  int F = int( 30.6001 * ( values.month + 1 ) );
  double julianDay = values.day + E + F - 1524.5;

  mValid = true;
  mJulianTime = int64_t( julianDay * MILLISECONDS_IN_DAY +
                         ( values.hours ) * MILLISECONDS_IN_HOUR +
                         values.minutes * MILLISECONDS_IN_MINUTE +
                         values.seconds * MILLISECONDS_IN_SECOND );
}

void MDAL::DateTime::setWithGregorianJulianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  // https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html

  mValid = true;

  if ( values.year > 1582 ||
       ( values.year == 1582 && ( values.month > 10 || ( values.month == 10 && values.day >= 15 ) ) ) ) // gregorian calendar
  {
    setWithGregorianCalendarDate( values );
  }
  else
    setWithJulianCalendarDate( values );
}

std::string MDAL::DateTime::toString( MDAL::DateTime::DateTimeValues values ) const
{
  int milliseconds = int( ( values.seconds - int( values.seconds ) ) * 1000 + 0.5 );
  std::string msStr;
  if ( milliseconds > 0 )
  {
    if ( milliseconds < 10 )
      msStr = prependZero( std::to_string( milliseconds ), 3 );
    else if ( milliseconds < 100 )
      msStr = prependZero( std::to_string( milliseconds ), 2 );
    else if ( milliseconds < 1000 )
      msStr = std::to_string( milliseconds );

    msStr = std::string( "," ).append( msStr );
  }

  std::string strDateTime = prependZero( std::to_string( values.year ), 4 ) + "-" +
                            prependZero( std::to_string( values.month ), 2 ) + "-" +
                            prependZero( std::to_string( values.day ), 2 ) + "T" +
                            prependZero( std::to_string( values.hours ), 2 ) + ":" +
                            prependZero( std::to_string( values.minutes ), 2 ) + ":" +
                            prependZero( std::to_string( int( values.seconds ) ), 2 ) +
                            msStr;

  return strDateTime;
}

MDAL::RelativeTimestamp MDAL::DateTime::operator-( const MDAL::DateTime &other ) const
{
  if ( !mValid || !other.mValid )
    return RelativeTimestamp();
  return RelativeTimestamp( mJulianTime - other.mJulianTime );
}


MDAL::RelativeTimestamp::RelativeTimestamp() = default;

MDAL::RelativeTimestamp::RelativeTimestamp( double duration, MDAL::RelativeTimestamp::Unit unit )
{
  switch ( unit )
  {
    case MDAL::RelativeTimestamp::milliseconds:
      mDuration = int64_t( duration );
      break;
    case MDAL::RelativeTimestamp::seconds:
      mDuration = int64_t( duration * MILLISECONDS_IN_SECOND + 0.5 );
      break;
    case MDAL::RelativeTimestamp::minutes:
      mDuration = int64_t( duration * MILLISECONDS_IN_MINUTE + 0.5 );
      break;
    case MDAL::RelativeTimestamp::hours:
      mDuration = int64_t( duration * MILLISECONDS_IN_HOUR + 0.5 );
      break;
    case MDAL::RelativeTimestamp::days:
      mDuration = int64_t( duration * MILLISECONDS_IN_DAY + 0.5 );
      break;
    case MDAL::RelativeTimestamp::weeks:
      mDuration = int64_t( duration * MILLISECONDS_IN_WEEK + 0.5 );
      break;
    case MDAL::RelativeTimestamp::months_CF:
      mDuration = int64_t( duration * MILLISECONDS_IN_MONTH_CF + 0.5 );
      break;
    case MDAL::RelativeTimestamp::exact_years:
      mDuration = int64_t( duration * MILLISECONDS_IN_EXACT_YEAR + 0.5 );
      break;
  }
}

double MDAL::RelativeTimestamp::value( MDAL::RelativeTimestamp::Unit unit ) const
{
  switch ( unit )
  {
    case MDAL::RelativeTimestamp::milliseconds:
      return double( mDuration );
    case MDAL::RelativeTimestamp::seconds:
      return mDuration / MILLISECONDS_IN_SECOND;
    case MDAL::RelativeTimestamp::minutes:
      return mDuration  / MILLISECONDS_IN_MINUTE;
    case MDAL::RelativeTimestamp::hours:
      return mDuration / MILLISECONDS_IN_HOUR;
    case MDAL::RelativeTimestamp::days:
      return double( mDuration ) / MILLISECONDS_IN_DAY;
    case MDAL::RelativeTimestamp::weeks:
      return double( mDuration )  / MILLISECONDS_IN_WEEK;
    case MDAL::RelativeTimestamp::months_CF:
      return double( mDuration ) / MILLISECONDS_IN_MONTH_CF;
    case MDAL::RelativeTimestamp::exact_years:
      return double( mDuration )  / MILLISECONDS_IN_EXACT_YEAR;
  }

  return 0;
}

bool MDAL::RelativeTimestamp::operator==( const MDAL::RelativeTimestamp &other ) const
{
  return mDuration == other.mDuration;
}

bool MDAL::RelativeTimestamp::operator<( const MDAL::RelativeTimestamp &other ) const
{
  return mDuration < other.mDuration;
}

MDAL::RelativeTimestamp::RelativeTimestamp( int64_t ms ): mDuration( ms )
{}
