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
#include <chrono>

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsunittypes.h"
#include "qgis.h"

class QString;

/**
 * \ingroup core
 * \class QgsInterval
 * \brief A representation of the interval between two datetime values.
 * \since QGIS 2.16
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

    /**
     * Default constructor for QgsInterval. Creates an invalid interval.
     */
    QgsInterval() = default;

    /**
     * Constructor for QgsInterval.
     * \param seconds duration of interval in seconds
     */
    QgsInterval( double seconds );

    /**
     * Constructor for QgsInterval.
     * \param milliseconds duration of interval in milliseconds
     */
    QgsInterval( std::chrono::milliseconds milliseconds ) SIP_SKIP;

    /**
     * Constructor for QgsInterval, using the specified \a duration and \a units.
     */
    QgsInterval( double duration, QgsUnitTypes::TemporalUnit unit );

    /**
     * Constructor for QgsInterval, using the specified \a years, \a months,
     * \a weeks, \a days, \a hours, \a minutes and \a seconds.
     *
     * \note Month units assumes a 30 day month length.
     * \note Year units assumes a 365.25 day year length.
     *
     * \since QGIS 3.14
     */
    QgsInterval( double years, double months, double weeks, double days, double hours, double minutes, double seconds );

    /**
     * Returns the interval duration in years (based on an average year length)
     *
     * If the originalUnit() is QgsUnitTypes::TemporalYears then this value
     * will match the exact number of months as returned by originalDuration(),
     * otherwise it will be calculated using the average year length (31557600 seconds).
     *
     * \see setYears()
     */
    double years() const;

    /**
     * Sets the interval duration in years.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalYears
     *
     * \param years duration in years (based on average year length)
     * \see years()
     */
    void setYears( double years );

    /**
     * Returns the interval duration in months (based on a 30 day month).
     *
     * If the originalUnit() is QgsUnitTypes::TemporalMonths then this value
     * will match the exact number of months as returned by originalDuration(),
     * otherwise it will be calculated using the assumption that
     * a month consists of exactly 30 days.
     *
     * \see setMonths()
     */
    double months() const;

    /**
     * Sets the interval duration in months.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalMonths
     *
     * \param months duration in months (based on a 30 day month)
     * \see months()
     */
    void setMonths( double months );

    /**
     * Returns the interval duration in weeks.
     *
     * If the originalUnit() is QgsUnitTypes::TemporalWeeks then this value
     * will match the exact number of weeks as returned by originalDuration(),
     * otherwise it will be calculated using the QgsInterval::WEEKS constant.
     *
     * \see setWeeks()
     */
    double weeks() const;

    /**
     * Sets the interval duration in weeks.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalWeeks
     *
     * \param weeks duration in weeks
     * \see weeks()
     */
    void setWeeks( double weeks );

    /**
     * Returns the interval duration in days.
     *
     * If the originalUnit() is QgsUnitTypes::TemporalDays then this value
     * will match the exact number of days as returned by originalDuration(),
     * otherwise it will be calculated using the QgsInterval::DAY constant.
     *
     * \see setDays()
     */
    double days() const;

    /**
     * Sets the interval duration in days.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalDays
     *
     * \param days duration in days
     * \see days()
     */
    void setDays( double days );

    /**
     * Returns the interval duration in hours.
     *
     * If the originalUnit() is QgsUnitTypes::TemporalHours then this value
     * will match the exact number of hours as returned by originalDuration(),
     * otherwise it will be calculated using the QgsInterval::HOUR constant.
     *
     * \see setHours()
     */
    double hours() const;

    /**
     * Sets the interval duration in hours.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * The original unit to QgsUnitTypes::TemporalHours
     *
     * \param hours duration in hours
     * \see hours()
     */
    void setHours( double hours );

    /**
     * Returns the interval duration in minutes.
     *
     * If the originalUnit() is QgsUnitTypes::TemporalMinutes then this value
     * will match the exact number of minutes as returned by originalDuration(),
     * otherwise it will be calculated using the QgsInterval::MINUTE constant.
     *
     * \see setMinutes()
     */
    double minutes() const;

    /**
     * Sets the interval duration in minutes.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalMinutes
     *
     * \param minutes duration in minutes
     * \see minutes()
     */
    void setMinutes( double minutes );

    /**
     * Returns the interval duration in seconds.
     * \see setSeconds()
     */
    double seconds() const { return mSeconds; }

    /**
     * Sets the interval duration in seconds.
     *
     * Replaces the interval size and changes the original interval unit
     * and duration, \see originalDuration() and \see originalUnit().
     *
     * Changes the original unit to QgsUnitTypes::TemporalSeconds
     *
     * \param seconds duration in seconds
     * \see seconds()
     */
    void setSeconds( double seconds );

    /**
     * Returns TRUE if the interval is valid.
     * \see setValid()
     */
    bool isValid() const { return mValid; }

    /**
     * Sets whether the interval is valid.
     * \param valid set to TRUE to set the interval as valid.
     * \see isValid()
     */
    void setValid( bool valid ) { mValid = valid; }

    /**
     * Returns the original interval duration.
     *
     * This original interval duration can be updated through calling
     * QgsInterval setter methods.
     *
     * \see originalUnit() for the corresponding unit.
     *
     * If the original interval duration is not available or
     * interval was set with a mix of units, calling originalUnit()
     * will return QgsUnitTypes::TemporalUnknownUnit
     *
     * Returns 0.0 if the original duration was not set.
     *
     * \since 3.18
     */
    double originalDuration() const { return mOriginalDuration; }

    /**
     * Returns the original interval temporal unit.
     *
     * The interval temporal unit can be set through the
     * QgsInterval constructors or through the available setter methods.
     *
     * Returns QgsUnitTypes::TemporalUnknownUnit if unit was not set when creating the
     * QgsInterval instance or interval was set with a mix of units.
     *
     * \see originalDuration()
     *
     * \since 3.18
     */
    QgsUnitTypes::TemporalUnit originalUnit() const { return mOriginalUnit; }

    bool operator==( QgsInterval other ) const
    {
      if ( !mValid && !other.mValid )
        return true;
      else if ( mValid && other.mValid && ( mOriginalUnit != QgsUnitTypes::TemporalUnknownUnit || other.mOriginalUnit != QgsUnitTypes::TemporalUnknownUnit ) )
        return mOriginalUnit == other.mOriginalUnit && mOriginalDuration == other.mOriginalDuration;
      else if ( mValid && other.mValid )
        return qgsDoubleNear( mSeconds, other.mSeconds );
      else
        return false;
    }

    bool operator!=( QgsInterval other ) const
    {
      return !( *this == other );
    }

    /**
     * Converts a string to an interval
     * \param string string to parse
     * \returns interval, or invalid interval if string could not be parsed
     */
    static QgsInterval fromString( const QString &string );

    //! Allows direct construction of QVariants from intervals.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    //! Duration of interval in seconds
    double mSeconds = 0.0;

    //! True if interval is valid
    bool mValid = false;

    //! Interval duration
    double mOriginalDuration = 0.0;

    //! Interval unit
    QgsUnitTypes::TemporalUnit mOriginalUnit = QgsUnitTypes::TemporalUnknownUnit;
};

Q_DECLARE_METATYPE( QgsInterval )

#ifndef SIP_RUN

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)

/**
 * Returns the interval between two datetimes.
 * \param datetime1 start datetime
 * \param datetime2 datetime to subtract, ie subtract datetime2 from datetime1
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
QgsInterval CORE_EXPORT operator-( const QDateTime &datetime1, const QDateTime &datetime2 );

#endif

/**
 * Returns the interval between two dates.
 * \param date1 start date
 * \param date2 date to subtract, ie subtract date2 from date1
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
QgsInterval CORE_EXPORT operator-( QDate date1, QDate date2 );

/**
 * Returns the interval between two times.
 * \param time1 start time
 * \param time2 time to subtract, ie subtract time2 from time1
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
QgsInterval CORE_EXPORT operator-( QTime time1, QTime time2 );

/**
 * Adds an interval to a datetime
 * \param start initial datetime
 * \param interval interval to add
 * \note not available in Python bindings
 * \since QGIS 2.16
 */
QDateTime CORE_EXPORT operator+( const QDateTime &start, const QgsInterval &interval );

//! Debug string representation of interval
QDebug CORE_EXPORT operator<<( QDebug dbg, const QgsInterval &interval );

#endif

#endif // QGSINTERVAL_H
