/***************************************************************************
  qgsinterval.h
  -------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSINTERVAL_H
#define QGSINTERVAL_H

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsinterval.py.
 * See details in QEP #17
 ****************************************************************************/

#include <QVariant>
class QString;

/** \ingroup core
 * \class QgsInterval
 * \brief A representation of the interval between two datetime values.
 * \note Added in version 2.16
 */

class CORE_EXPORT QgsInterval
{
  public:

    // YEAR const value taken from postgres query
    // SELECT EXTRACT(EPOCH FROM interval '1 year')
    //! Seconds per year (average)
    static const int YEARS = 31557600;
    //! Seconds per month, based on 30 day month
    static const int MONTHS = 60 * 60 * 24 * 30;
    //! Seconds per week
    static const int WEEKS = 60 * 60 * 24 * 7;
    //! Seconds per day
    static const int DAY = 60 * 60 * 24;
    //! Seconds per hour
    static const int HOUR = 60 * 60;
    //! Seconds per minute
    static const int MINUTE = 60;

    /** Default constructor for QgsInterval. Creates an invalid interval.
     */
    QgsInterval();

    /** Constructor for QgsInterval.
     * @param seconds duration of interval in seconds
     */
    QgsInterval( double seconds );

    /** Returns the interval duration in years (based on an average year length)
     * @see setYears()
     */
    double years() const { return mSeconds / YEARS; }

    /** Sets the interval duration in years.
     * @param years duration in years (based on average year length)
     * @see years()
     */
    void setYears( double years ) { mSeconds = years * YEARS; mValid = true; }

    /** Returns the interval duration in months (based on a 30 day month).
     * @see setMonths()
     */
    double months() const { return mSeconds / MONTHS; }

    /** Sets the interval duration in months.
     * @param months duration in months (based on a 30 day month)
     * @see months()
     */
    void setMonths( double months ) { mSeconds = months * MONTHS; mValid = true; }

    /** Returns the interval duration in weeks.
     * @see setWeeks()
     */
    double weeks() const { return mSeconds / WEEKS; }

    /** Sets the interval duration in weeks.
     * @param weeks duration in weeks
     * @see weeks()
     */
    void setWeeks( double weeks ) { mSeconds = weeks * WEEKS; mValid = true; }

    /** Returns the interval duration in days.
     * @see setDays()
     */
    double days() const { return mSeconds / DAY; }

    /** Sets the interval duration in days.
     * @param days duration in days
     * @see days()
     */
    void setDays( double days ) { mSeconds = days * DAY; mValid = true; }

    /** Returns the interval duration in hours.
     * @see setHours()
     */
    double hours() const { return mSeconds / HOUR; }

    /** Sets the interval duration in hours.
     * @param hours duration in hours
     * @see hours()
     */
    void setHours( double hours ) { mSeconds = hours * HOUR; mValid = true; }

    /** Returns the interval duration in minutes.
     * @see setMinutes()
     */
    double minutes() const { return mSeconds / MINUTE; }

    /** Sets the interval duration in minutes.
     * @param minutes duration in minutes
     * @see minutes()
     */
    void setMinutes( double minutes ) { mSeconds = minutes * MINUTE; mValid = true; }

    /** Returns the interval duration in seconds.
     * @see setSeconds()
     */
    double seconds() const { return mSeconds; }

    /** Sets the interval duration in seconds.
     * @param seconds duration in seconds
     * @see seconds()
     */
    void setSeconds( double seconds ) { mSeconds = seconds; mValid = true; }

    /** Returns true if the interval is valid.
     * @see setValid()
     */
    bool isValid() const { return mValid; }

    /** Sets whether the interval is valid.
     * @param valid set to true to set the interval as valid.
     * @see isValid()
     */
    void setValid( bool valid ) { mValid = valid; }

    bool operator==( const QgsInterval& other ) const;

    /** Converts a string to an interval
     * @param string string to parse
     * @returns interval, or invalid interval if string could not be parsed
     */
    static QgsInterval fromString( const QString& string );

    //! Allows direct construction of QVariants from intervals.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    //! Duration of interval in seconds
    double mSeconds;

    //! True if interval is valid
    bool mValid;
};

Q_DECLARE_METATYPE( QgsInterval )

/** Returns the interval between two datetimes.
 * @param datetime1 start datetime
 * @param datetime2 datetime to subtract, ie subtract datetime2 from datetime1
 * @note added in QGIS 2.16
 * @note not available in Python bindings
 */
QgsInterval CORE_EXPORT operator-( const QDateTime& datetime1, const QDateTime& datetime2 );

/** Returns the interval between two dates.
 * @param date1 start date
 * @param date2 date to subtract, ie subtract date2 from date1
 * @note added in QGIS 2.16
 * @note not available in Python bindings
 */
QgsInterval CORE_EXPORT operator-( const QDate& date1, const QDate& date2 );

/** Returns the interval between two times.
 * @param time1 start time
 * @param time2 time to subtract, ie subtract time2 from time1
 * @note added in QGIS 2.16
 * @note not available in Python bindings
 */
QgsInterval CORE_EXPORT operator-( const QTime& time1, const QTime& time2 );

/** Adds an interval to a datetime
 * @param start initial datetime
 * @param interval interval to add
 * @note added in QGIS 2.16
 * @note not available in Python bindings
 */
QDateTime CORE_EXPORT operator+( const QDateTime& start, const QgsInterval& interval );

//! Debug string representation of interval
QDebug operator<<( QDebug dbg, const QgsInterval& interval );
\
#endif // QGSINTERVAL_H
