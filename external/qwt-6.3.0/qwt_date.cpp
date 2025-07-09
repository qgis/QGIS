/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_date.h"
#include "qwt_math.h"

#include <qdebug.h>
#include <qlocale.h>

#include <limits>

#if QT_VERSION >= 0x050000

typedef qint64 QwtJulianDay;
static const QwtJulianDay minJulianDayD = Q_INT64_C( -784350574879 );
static const QwtJulianDay maxJulianDayD = Q_INT64_C( 784354017364 );

#else

// QDate stores the Julian day as unsigned int, but
// there is QDate::fromJulianDay( int ). That's why
// we have the range [ 1, INT_MAX ]

typedef int QwtJulianDay;
static const QwtJulianDay minJulianDayD = 1;
static const QwtJulianDay maxJulianDayD = std::numeric_limits< int >::max();

#endif

static QString qwtExpandedFormat( const QString& format,
    const QDateTime& dateTime, QwtDate::Week0Type week0Type )
{
    const int week = QwtDate::weekNumber( dateTime.date(), week0Type );

    QString weekNo;
    weekNo.setNum( week );

    QString weekNoWW;
    if ( weekNo.length() == 1 )
        weekNoWW += QLatin1Char( '0' );

    weekNoWW += weekNo;

    QString fmt = format;
    fmt.replace( QLatin1String( "ww" ), weekNoWW );
    fmt.replace( QLatin1Char( 'w' ), weekNo );

    if ( week == 1 && dateTime.date().month() != 1 )
    {
        // in case of week 1, we might need to increment the year

        QLatin1String s_yyyy( "yyyy" );
        QLatin1String s_yy( "yy" );

        // week 1 might start in the previous year

        bool doReplaceYear = fmt.contains( s_yy );

        if ( doReplaceYear )
        {
            if ( fmt.contains( 'M' ) )
            {
                // in case of also having 'M' we have a conflict about
                // which year to show

                doReplaceYear = false;
            }
            else
            {
                // in case of also having 'd' or 'dd' we have a conflict about
                // which year to show

                int numD = 0;

                for ( int i = 0; i < fmt.size(); i++ )
                {
                    if ( fmt[i] == 'd' )
                    {
                        numD++;
                    }
                    else
                    {
                        if ( numD > 0 && numD <= 2 )
                            break;

                        numD = 0;
                    }
                }

                if ( numD > 0 && numD <= 2 )
                    doReplaceYear = false;
            }
        }

        if ( doReplaceYear )
        {
            const QDate dt( dateTime.date().year() + 1, 1, 1 );
            const QString dtString = QLocale().toString( dt, s_yyyy );

            if ( fmt.contains( s_yyyy ) )
            {
                fmt.replace( s_yyyy, dtString );
            }
            else
            {
                fmt.replace( s_yy, dtString );
            }
        }
    }

    return fmt;
}

static inline Qt::DayOfWeek qwtFirstDayOfWeek()
{
    return QLocale().firstDayOfWeek();
}

static inline void qwtFloorTime(
    QwtDate::IntervalType intervalType, QDateTime& dt )
{
    // when dt is inside the special hour where DST is ending
    // an hour is no unique. Therefore we have to
    // use UTC time.

    const Qt::TimeSpec timeSpec = dt.timeSpec();

    if ( timeSpec == Qt::LocalTime )
        dt = dt.toTimeSpec( Qt::UTC );

    const QTime t = dt.time();
    switch( intervalType )
    {
        case QwtDate::Second:
        {
            dt.setTime( QTime( t.hour(), t.minute(), t.second() ) );
            break;
        }
        case QwtDate::Minute:
        {
            dt.setTime( QTime( t.hour(), t.minute(), 0 ) );
            break;
        }
        case QwtDate::Hour:
        {
            dt.setTime( QTime( t.hour(), 0, 0 ) );
            break;
        }
        default:
            break;
    }

    if ( timeSpec == Qt::LocalTime )
        dt = dt.toTimeSpec( Qt::LocalTime );
}

static inline QDateTime qwtToTimeSpec(
    const QDateTime& dt, Qt::TimeSpec spec )
{
    if ( dt.timeSpec() == spec )
        return dt;

    const qint64 jd = dt.date().toJulianDay();
    if ( jd < 0 || jd >= std::numeric_limits< int >::max() )
    {
        // the conversion between local time and UTC
        // is internally limited. To avoid
        // overflows we simply ignore the difference
        // for those dates

        QDateTime dt2 = dt;
        dt2.setTimeSpec( spec );
        return dt2;
    }

    return dt.toTimeSpec( spec );
}

#if 0

static inline double qwtToJulianDay( int year, int month, int day )
{
    // code from QDate but using doubles to avoid overflows
    // for large values

    const int m1 = ( month - 14 ) / 12;
    const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
    const double y1 = std::floor( ( 4900.0 + year + m1 ) / 100 );

    return std::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
           - std::floor( ( 3 * y1 ) / 4 ) + day - 32075;
}

static inline qint64 qwtFloorDiv64( qint64 a, int b )
{
    if ( a < 0 )
        a -= b - 1;

    return a / b;
}

static inline qint64 qwtFloorDiv( int a, int b )
{
    if ( a < 0 )
        a -= b - 1;

    return a / b;
}

#endif

static inline QDate qwtToDate( int year, int month = 1, int day = 1 )
{
#if QT_VERSION >= 0x050000
    return QDate( year, month, day );
#else
    if ( year > 100000 )
    {
        // code from QDate but using doubles to avoid overflows
        // for large values

        const int m1 = ( month - 14 ) / 12;
        const int m2 = ( 367 * ( month - 2 - 12 * m1 ) ) / 12;
        const double y1 = std::floor( ( 4900.0 + year + m1 ) / 100 );

        const double jd = std::floor( ( 1461.0 * ( year + 4800 + m1 ) ) / 4 ) + m2
            - std::floor( ( 3 * y1 ) / 4 ) + day - 32075;

        if ( jd > maxJulianDayD )
        {
            qWarning() << "qwtToDate: overflow";
            return QDate();
        }

        return QDate::fromJulianDay( static_cast< QwtJulianDay >( jd ) );
    }
    else
    {
        return QDate( year, month, day );
    }
#endif
}

/*!
   Translate from double to QDateTime

   \param value Number of milliseconds since the epoch,
               1970-01-01T00:00:00 UTC
   \param timeSpec Time specification
   \return Datetime value

   \sa toDouble(), QDateTime::setMSecsSinceEpoch()
   \note The return datetime for Qt::OffsetFromUTC will be Qt::UTC
 */
QDateTime QwtDate::toDateTime( double value, Qt::TimeSpec timeSpec )
{
    const int msecsPerDay = 86400000;

    const double days = static_cast< qint64 >( std::floor( value / msecsPerDay ) );

    const double jd = QwtDate::JulianDayForEpoch + days;
    if ( ( jd > maxJulianDayD ) || ( jd < minJulianDayD ) )
    {
        qWarning() << "QwtDate::toDateTime: overflow";
        return QDateTime();
    }

    const QDate d = QDate::fromJulianDay( static_cast< QwtJulianDay >( jd ) );

    const int msecs = static_cast< int >( value - days * msecsPerDay );

    static const QTime timeNull( 0, 0, 0, 0 );

    QDateTime dt( d, timeNull.addMSecs( msecs ), Qt::UTC );

    if ( timeSpec == Qt::LocalTime )
        dt = qwtToTimeSpec( dt, timeSpec );

    return dt;
}

/*!
   Translate from QDateTime to double

   \param dateTime Datetime value
   \return Number of milliseconds since 1970-01-01T00:00:00 UTC has passed.

   \sa toDateTime(), QDateTime::toMSecsSinceEpoch()
   \warning For values very far below or above 1970-01-01 UTC rounding errors
           will happen due to the limited significance of a double.
 */
double QwtDate::toDouble( const QDateTime& dateTime )
{
    const int msecsPerDay = 86400000;

    const QDateTime dt = qwtToTimeSpec( dateTime, Qt::UTC );

    const double days = dt.date().toJulianDay() - QwtDate::JulianDayForEpoch;

    const QTime time = dt.time();
    const double secs = 3600.0 * time.hour() +
        60.0 * time.minute() + time.second();

    return days * msecsPerDay + time.msec() + 1000.0 * secs;
}

/*!
   Ceil a datetime according the interval type

   \param dateTime Datetime value
   \param intervalType Interval type, how to ceil.
                      F.e. when intervalType = QwtDate::Months, the result
                      will be ceiled to the next beginning of a month
   \return Ceiled datetime
   \sa floor()
 */
QDateTime QwtDate::ceil( const QDateTime& dateTime, IntervalType intervalType )
{
    if ( dateTime.date() >= QwtDate::maxDate() )
        return dateTime;

    QDateTime dt = dateTime;

    switch ( intervalType )
    {
        case QwtDate::Millisecond:
        {
            break;
        }
        case QwtDate::Second:
        {
            qwtFloorTime( QwtDate::Second, dt );
            if ( dt < dateTime )
                dt = dt.addSecs( 1 );

            break;
        }
        case QwtDate::Minute:
        {
            qwtFloorTime( QwtDate::Minute, dt );
            if ( dt < dateTime )
                dt = dt.addSecs( 60 );

            break;
        }
        case QwtDate::Hour:
        {
            qwtFloorTime( QwtDate::Hour, dt );
            if ( dt < dateTime )
                dt = dt.addSecs( 3600 );

            break;
        }
        case QwtDate::Day:
        {
            dt.setTime( QTime( 0, 0 ) );
            if ( dt < dateTime )
                dt = dt.addDays( 1 );

            break;
        }
        case QwtDate::Week:
        {
            dt.setTime( QTime( 0, 0 ) );
            if ( dt < dateTime )
                dt = dt.addDays( 1 );

            int days = qwtFirstDayOfWeek() - dt.date().dayOfWeek();
            if ( days < 0 )
                days += 7;

            dt = dt.addDays( days );

            break;
        }
        case QwtDate::Month:
        {
            dt.setTime( QTime( 0, 0 ) );
            dt.setDate( qwtToDate( dateTime.date().year(),
                dateTime.date().month() ) );

            if ( dt < dateTime )
                dt = dt.addMonths( 1 );

            break;
        }
        case QwtDate::Year:
        {
            dt.setTime( QTime( 0, 0 ) );

            const QDate d = dateTime.date();

            int year = d.year();
            if ( d.month() > 1 || d.day() > 1 || !dateTime.time().isNull() )
                year++;

            if ( year == 0 )
                year++; // there is no year 0

            dt.setDate( qwtToDate( year ) );
            break;
        }
    }

    return dt;
}

/*!
   Floor a datetime according the interval type

   \param dateTime Datetime value
   \param intervalType Interval type, how to ceil.
                      F.e. when intervalType = QwtDate::Months,
                      the result will be ceiled to the next
                      beginning of a month
   \return Floored datetime
   \sa floor()
 */
QDateTime QwtDate::floor( const QDateTime& dateTime,
    IntervalType intervalType )
{
    if ( dateTime.date() <= QwtDate::minDate() )
        return dateTime;

    QDateTime dt = dateTime;

    switch ( intervalType )
    {
        case QwtDate::Millisecond:
        {
            break;
        }
        case QwtDate::Second:
        case QwtDate::Minute:
        case QwtDate::Hour:
        {
            qwtFloorTime( intervalType, dt );
            break;
        }
        case QwtDate::Day:
        {
            dt.setTime( QTime( 0, 0 ) );
            break;
        }
        case QwtDate::Week:
        {
            dt.setTime( QTime( 0, 0 ) );

            int days = dt.date().dayOfWeek() - qwtFirstDayOfWeek();
            if ( days < 0 )
                days += 7;

            dt = dt.addDays( -days );

            break;
        }
        case QwtDate::Month:
        {
            dt.setTime( QTime( 0, 0 ) );

            const QDate date = qwtToDate( dt.date().year(),
                dt.date().month() );
            dt.setDate( date );

            break;
        }
        case QwtDate::Year:
        {
            dt.setTime( QTime( 0, 0 ) );

            const QDate date = qwtToDate( dt.date().year() );
            dt.setDate( date );

            break;
        }
    }

    return dt;
}

/*!
   Minimum for the supported date range

   The range of valid dates depends on how QDate stores the
   Julian day internally.

   - For Qt4 it is "Tue Jan 2 -4713"
   - For Qt5 it is "Thu Jan 1 -2147483648"

   \return minimum of the date range
   \sa maxDate()
 */
QDate QwtDate::minDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( minJulianDayD );

    return date;
}

/*!
   Maximum for the supported date range

   The range of valid dates depends on how QDate stores the
   Julian day internally.

   - For Qt4 it is "Tue Jun 3 5874898"
   - For Qt5 it is "Tue Dec 31 2147483647"

   \return maximum of the date range
   \sa minDate()
   \note The maximum differs between Qt4 and Qt5
 */
QDate QwtDate::maxDate()
{
    static QDate date;
    if ( !date.isValid() )
        date = QDate::fromJulianDay( maxJulianDayD );

    return date;
}

/*!
   \brief Date of the first day of the first week for a year

   The first day of a week depends on the current locale
   ( QLocale::firstDayOfWeek() ).

   \param year Year
   \param type Option how to identify the first week
   \return First day of week 0

   \sa QLocale::firstDayOfWeek(), weekNumber()
 */
QDate QwtDate::dateOfWeek0( int year, Week0Type type )
{
    const Qt::DayOfWeek firstDayOfWeek = qwtFirstDayOfWeek();

    QDate dt0( year, 1, 1 );

    // floor to the first day of the week
    int days = dt0.dayOfWeek() - firstDayOfWeek;
    if ( days < 0 )
        days += 7;

    dt0 = dt0.addDays( -days );

    if ( type == QwtDate::FirstThursday )
    {
        // according to ISO 8601 the first week is defined
        // by the first Thursday.

        int d = Qt::Thursday - firstDayOfWeek;
        if ( d < 0 )
            d += 7;

        if ( dt0.addDays( d ).year() < year )
            dt0 = dt0.addDays( 7 );
    }

    return dt0;
}

/*!
   Find the week number of a date

   - QwtDate::FirstThursday\n
    Corresponding to ISO 8601 ( see QDate::weekNumber() ).

   - QwtDate::FirstDay\n
    Number of weeks that have begun since dateOfWeek0().

   \param date Date
   \param type Option how to identify the first week

   \return Week number, starting with 1
 */
int QwtDate::weekNumber( const QDate& date, Week0Type type )
{
    int weekNo;

    if ( type == QwtDate::FirstDay )
    {
        QDate day0;

        if ( date.month() == 12 && date.day() >= 24 )
        {
            // week 1 usually starts in the previous years.
            // and we have to check if we are already there

            day0 = dateOfWeek0( date.year() + 1, type );
            if ( day0.daysTo( date ) < 0 )
                day0 = dateOfWeek0( date.year(), type );
        }
        else
        {
            day0 = dateOfWeek0( date.year(), type );
        }

        weekNo = day0.daysTo( date ) / 7 + 1;
    }
    else
    {
        weekNo = date.weekNumber();
    }

    return weekNo;
}

/*!
   Offset in seconds from Coordinated Universal Time

   The offset depends on the time specification of dateTime:

   - Qt::UTC
     0, dateTime has no offset
   - Qt::OffsetFromUTC
     returns dateTime.offsetFromUtc()
   - Qt::LocalTime:
     number of seconds from the UTC

   For Qt::LocalTime the offset depends on the timezone and
   daylight savings.

   \param dateTime Datetime value
   \return Offset in seconds
 */
int QwtDate::utcOffset( const QDateTime& dateTime )
{
    int seconds = 0;

    switch( dateTime.timeSpec() )
    {
        case Qt::UTC:
        {
            break;
        }
        case Qt::OffsetFromUTC:
        {
#if QT_VERSION >= 0x050200
            seconds = dateTime.offsetFromUtc();
#else
            seconds = dateTime.utcOffset();
#endif
            break;
        }
        default:
        {
            const QDateTime dt1( dateTime.date(), dateTime.time(), Qt::UTC );
            seconds = dateTime.secsTo( dt1 );
        }
    }

    return seconds;
}

/*!
   Translate a datetime into a string

   Beside the format expressions documented in QDateTime::toString()
   the following expressions are supported:

   - w\n
    week number: ( 1 - 53 )
   - ww\n
    week number with a leading zero ( 01 - 53 )

   As week 1 usually starts in the previous year a special rule
   is applied for formats, where the year is expected to match the
   week number - even if the date belongs to the previous year.

   \param dateTime Datetime value
   \param format Format string
   \param week0Type Specification of week 0

   \return Datetime string
   \sa QDateTime::toString(), weekNumber(), QwtDateScaleDraw
 */
QString QwtDate::toString( const QDateTime& dateTime,
    const QString& format, Week0Type week0Type )
{
    QString fmt = format;
    if ( fmt.contains( 'w' ) )
    {
        fmt = qwtExpandedFormat( fmt, dateTime, week0Type );
    }

    return QLocale().toString( dateTime, fmt );
}
