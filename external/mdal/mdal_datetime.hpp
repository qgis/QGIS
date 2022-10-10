/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_DATE_TIME_HPP
#define MDAL_DATE_TIME_HPP

#include <string>
#include <vector>

#include "mdal.h"

namespace MDAL
{

  class RelativeTimestamp
  {
    public:
      enum Unit
      {
        milliseconds = 0,
        seconds,
        minutes,
        hours,
        days,
        weeks,
        months_CF,
        exact_years
      };

      RelativeTimestamp();
      RelativeTimestamp( double duration, Unit unit );

      double value( Unit unit ) const;

      bool operator==( const RelativeTimestamp &other ) const;
      bool operator<( const RelativeTimestamp &other ) const;

    private:
      RelativeTimestamp( int64_t ms );
      int64_t mDuration = 0; //in ms

      friend class DateTime;
  };

  class DateTime
  {
    public:

      enum Calendar
      {
        Gregorian = 0,
        ProlepticGregorian,
        Julian,
      };

      enum Epoch
      {
        Unix = 0,
        JulianDay
      };

      DateTime();
      //! Constructor with date/time values and calendar type
      DateTime( int year, int month, int day, int hours = 0, int minutes = 0, double seconds = 0, Calendar calendar = Gregorian );
      //! Constructor with Julian day or Unix Epoch
      DateTime( double value, Epoch epoch );

      //! Constructor with ISO 8601 string
      DateTime( const std::string &fromISO8601 );

      //! Returns a string with the date/time expressed in Greogrian proleptic calendar with ISO8601 format (local time zone)
      //! Do not support negative year
      std::string toStandardCalendarISO8601() const;

      //! Returns the Julian day value
      double toJulianDay() const;

      //! Returns the Julain day value expressed with a string
      std::string toJulianDayString() const;

      //! Returns a array of int with {year,month,day,hours,minutes,seconds} with standard calendar format
      std::vector<int> expandToCalendarArray() const;

      //! operators
      RelativeTimestamp operator-( const DateTime &other ) const;
      DateTime operator+( const RelativeTimestamp &duration ) const;
      DateTime operator-( const RelativeTimestamp &duration ) const;
      bool operator==( const DateTime &other ) const;
      bool operator!=( const DateTime &other ) const;
      bool operator<( const DateTime &other ) const;

      bool isValid() const;

    private:

      struct DateTimeValues
      {
        int year;
        int month;
        int day;
        int hours;
        int minutes;
        double seconds;
      };

      DateTime( int64_t julianTime );

      DateTimeValues dateTimeGregorianProleptic() const;

      void setWithGregorianCalendarDate( DateTimeValues values );
      void setWithJulianCalendarDate( DateTimeValues values );
      void setWithGregorianJulianCalendarDate( DateTimeValues values );//Uses the adapted formula depending of the date (< or > 1582-10-15)

      std::string toString( DateTimeValues values ) const;

      int64_t mJulianTime = 0; //Julian day in ms

      bool mValid = false;
  };
}

#endif // MDAL_DATE_TIME_HPP
