/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_date_scale_engine.h"
#include "qwt_math.h"
#include "qwt_interval.h"

#include <qdatetime.h>

#include <limits>

static inline double qwtMsecsForType( int type )
{
    static const double msecs[] =
    {
        1.0,
        1000.0,
        60.0 * 1000.0,
        3600.0 * 1000.0,
        24.0 * 3600.0 * 1000.0,
        7.0 * 24.0 * 3600.0 * 1000.0,
        30.0 * 24.0 * 3600.0 * 1000.0,
        365.0 * 24.0 * 3600.0 * 1000.0,
    };

    if ( type < 0 || type >= static_cast< int >( sizeof( msecs ) / sizeof( msecs[0] ) ) )
        return 1.0;

    return msecs[ type ];
}

static inline int qwtAlignValue(
    double value, double stepSize, bool up )
{
    double d = value / stepSize;
    d = up ? std::ceil( d ) : std::floor( d );

    return static_cast< int >( d * stepSize );
}

static double qwtIntervalWidth( const QDateTime& minDate,
    const QDateTime& maxDate, QwtDate::IntervalType intervalType )
{
    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            return minDate.msecsTo( maxDate );
        }
        case QwtDate::Second:
        {
            return minDate.secsTo( maxDate );
        }
        case QwtDate::Minute:
        {
            const double secsTo = minDate.secsTo( maxDate );
            return std::floor( secsTo / 60 );
        }
        case QwtDate::Hour:
        {
            const double secsTo = minDate.secsTo( maxDate );
            return std::floor( secsTo / 3600 );
        }
        case QwtDate::Day:
        {
            return minDate.daysTo( maxDate );
        }
        case QwtDate::Week:
        {
            return std::floor( minDate.daysTo( maxDate ) / 7.0 );
        }
        case QwtDate::Month:
        {
            const double years =
                double( maxDate.date().year() ) - minDate.date().year();

            int months = maxDate.date().month() - minDate.date().month();
            if ( maxDate.date().day() < minDate.date().day() )
                months--;

            return years * 12 + months;
        }
        case QwtDate::Year:
        {
            double years =
                double( maxDate.date().year() ) - minDate.date().year();

            if ( maxDate.date().month() < minDate.date().month() )
                years -= 1.0;

            return years;
        }
    }

    return 0.0;
}

static double qwtRoundedIntervalWidth(
    const QDateTime& minDate, const QDateTime& maxDate,
    QwtDate::IntervalType intervalType )
{
    const QDateTime minD = QwtDate::floor( minDate, intervalType );
    const QDateTime maxD = QwtDate::ceil( maxDate, intervalType );

    return qwtIntervalWidth( minD, maxD, intervalType );
}

static inline int qwtStepCount( int intervalSize, int maxSteps,
    const int limits[], size_t numLimits )
{
    for ( uint i = 0; i < numLimits; i++ )
    {
        const int numSteps = intervalSize / limits[ i ];

        if ( numSteps > 1 && numSteps <= maxSteps &&
            numSteps * limits[ i ] == intervalSize )
        {
            return numSteps;
        }
    }

    return 0;
}

static int qwtStepSize( int intervalSize, int maxSteps, uint base )
{
    if ( maxSteps <= 0 )
        return 0;

    if ( maxSteps > 2 )
    {
        for ( int numSteps = maxSteps; numSteps > 1; numSteps-- )
        {
            const double stepSize = double( intervalSize ) / numSteps;

            const double p = std::floor( std::log( stepSize ) / std::log( double( base ) ) );
            const double fraction = std::pow( base, p );

            for ( uint n = base; n >= 1; n /= 2 )
            {
                if ( qFuzzyCompare( stepSize, n * fraction ) )
                    return qRound( stepSize );

                if ( n == 3 && ( base % 2 ) == 0 )
                {
                    if ( qFuzzyCompare( stepSize, 2 * fraction ) )
                        return qRound( stepSize );
                }
            }
        }
    }

    return 0;
}

static int qwtDivideInterval( double intervalSize, int numSteps,
    const int limits[], size_t numLimits )
{
    const int v = qwtCeil( intervalSize / double( numSteps ) );

    for ( uint i = 0; i < numLimits - 1; i++ )
    {
        if ( v <= limits[i] )
            return limits[i];
    }

    return limits[ numLimits - 1 ];
}

static double qwtDivideScale( double intervalSize, int numSteps,
    QwtDate::IntervalType intervalType )
{
    if ( intervalType != QwtDate::Day )
    {
        if ( ( intervalSize > numSteps ) &&
            ( intervalSize <= 2 * numSteps ) )
        {
            return 2.0;
        }
    }

    double stepSize;

    switch( intervalType )
    {
        case QwtDate::Second:
        case QwtDate::Minute:
        {
            static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Hour:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12, 24 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Day:
        {
            const double v = intervalSize / double( numSteps );
            if ( v <= 5.0 )
                stepSize = std::ceil( v );
            else
                stepSize = std::ceil( v / 7 ) * 7;

            break;
        }
        case QwtDate::Week:
        {
            static int limits[] = { 1, 2, 4, 8, 12, 26, 52 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Month:
        {
            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            stepSize = qwtDivideInterval( intervalSize, numSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            break;
        }
        case QwtDate::Year:
        case QwtDate::Millisecond:
        default:
        {
            stepSize = QwtScaleArithmetic::divideInterval(
                intervalSize, numSteps, 10 );
        }
    }

    return stepSize;
}

static double qwtDivideMajorStep( double stepSize, int maxMinSteps,
    QwtDate::IntervalType intervalType )
{
    double minStepSize = 0.0;

    switch( intervalType )
    {
        case QwtDate::Second:
        {
            minStepSize = qwtStepSize( stepSize, maxMinSteps, 10 );
            if ( minStepSize == 0.0 )
                minStepSize = 0.5 * stepSize;

            break;
        }
        case QwtDate::Minute:
        {
            static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

            int numSteps;

            if ( stepSize > maxMinSteps )
            {
                numSteps = qwtStepCount( stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );

            }
            else
            {
                numSteps = qwtStepCount( stepSize * 60, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = stepSize / numSteps;

            break;
        }
        case QwtDate::Hour:
        {
            int numSteps = 0;

            if ( stepSize > maxMinSteps )
            {
                static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

                numSteps = qwtStepCount( stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }
            else
            {
                static int limits[] = { 1, 2, 5, 10, 15, 20, 30, 60 };

                numSteps = qwtStepCount( stepSize * 60, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = stepSize / numSteps;

            break;
        }
        case QwtDate::Day:
        {
            int numSteps = 0;

            if ( stepSize > maxMinSteps )
            {
                static int limits[] = { 1, 2, 3, 7, 14, 28 };

                numSteps = qwtStepCount( stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }
            else
            {
                static int limits[] = { 1, 2, 3, 4, 6, 12, 24, 48, 72 };

                numSteps = qwtStepCount( stepSize * 24, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );
            }

            if ( numSteps > 0 )
                minStepSize = stepSize / numSteps;

            break;
        }
        case QwtDate::Week:
        {
            const int daysInStep = stepSize * 7;

            if ( maxMinSteps >= daysInStep )
            {
                // we want to have one tick per day
                minStepSize = 1.0 / 7.0;
            }
            else
            {
                // when the stepSize is more than a week we want to
                // have a tick for each week

                const int stepSizeInWeeks = stepSize;

                if ( stepSizeInWeeks <= maxMinSteps )
                {
                    minStepSize = 1;
                }
                else
                {
                    minStepSize = QwtScaleArithmetic::divideInterval(
                        stepSizeInWeeks, maxMinSteps, 10 );
                }
            }
            break;
        }
        case QwtDate::Month:
        {
            // fractions of months doesn't make any sense

            if ( stepSize < maxMinSteps )
                maxMinSteps = static_cast< int >( stepSize );

            static int limits[] = { 1, 2, 3, 4, 6, 12 };

            int numSteps = qwtStepCount( stepSize, maxMinSteps,
                limits, sizeof( limits ) / sizeof( int ) );

            if ( numSteps > 0 )
                minStepSize = stepSize / numSteps;

            break;
        }
        case QwtDate::Year:
        {
            if ( stepSize >= maxMinSteps )
            {
                minStepSize = QwtScaleArithmetic::divideInterval(
                    stepSize, maxMinSteps, 10 );
            }
            else
            {
                // something in months

                static int limits[] = { 1, 2, 3, 4, 6, 12 };

                int numSteps = qwtStepCount( 12 * stepSize, maxMinSteps,
                    limits, sizeof( limits ) / sizeof( int ) );

                if ( numSteps > 0 )
                    minStepSize = stepSize / numSteps;
            }

            break;
        }
        default:
            break;
    }

    if ( intervalType != QwtDate::Month
        && minStepSize == 0.0 )
    {
        minStepSize = 0.5 * stepSize;
    }

    return minStepSize;
}

static QList< double > qwtDstTicks( const QDateTime& dateTime,
    int secondsMajor, int secondsMinor )
{
    if ( secondsMinor <= 0 )
        QList< double >();

    QDateTime minDate = dateTime.addSecs( -secondsMajor );
    minDate = QwtDate::floor( minDate, QwtDate::Hour );

    const double utcOffset = QwtDate::utcOffset( dateTime );

    // find the hours where daylight saving time happens

    double dstMin = QwtDate::toDouble( minDate );
    while ( minDate < dateTime &&
        QwtDate::utcOffset( minDate ) != utcOffset )
    {
        minDate = minDate.addSecs( 3600 );
        dstMin += 3600 * 1000.0;
    }

    QList< double > ticks;
    ticks.reserve( 3600 / secondsMinor);

    for ( int i = 0; i < 3600; i += secondsMinor )
        ticks += dstMin + i * 1000.0;

    return ticks;
}

static QwtScaleDiv qwtDivideToSeconds(
    const QDateTime& minDate, const QDateTime& maxDate,
    double stepSize, int maxMinSteps,
    QwtDate::IntervalType intervalType )
{
    // calculate the min step size
    double minStepSize = 0;

    if ( maxMinSteps > 1 )
    {
        minStepSize = qwtDivideMajorStep( stepSize,
            maxMinSteps, intervalType );
    }

    bool daylightSaving = false;
    if ( minDate.timeSpec() == Qt::LocalTime )
    {
        daylightSaving = intervalType > QwtDate::Hour;
        if ( intervalType == QwtDate::Hour )
        {
            daylightSaving = stepSize > 1;
        }
    }

    const double s = qwtMsecsForType( intervalType ) / 1000;
    const int secondsMajor = static_cast< int >( stepSize * s );
    const double secondsMinor = minStepSize * s;

    // UTC excludes daylight savings. So from the difference
    // of a date and its UTC counterpart we can find out
    // the daylight saving hours

    const double utcOffset = QwtDate::utcOffset( minDate );
    double dstOff = 0;

    QList< double > majorTicks;
    QList< double > mediumTicks;
    QList< double > minorTicks;

    for ( QDateTime dt = minDate; dt <= maxDate;
        dt = dt.addSecs( secondsMajor ) )
    {
        if ( !dt.isValid() )
            break;

        double majorValue = QwtDate::toDouble( dt );

        if ( daylightSaving )
        {
            const double offset = utcOffset - QwtDate::utcOffset( dt );
            majorValue += offset * 1000.0;

            if ( offset > dstOff )
            {
                // we add some minor ticks for the DST hour,
                // otherwise the ticks will be unaligned: 0, 2, 3, 5 ...
                minorTicks += qwtDstTicks(
                    dt, secondsMajor, qRound( secondsMinor ) );
            }

            dstOff = offset;
        }

        if ( majorTicks.isEmpty() || majorTicks.last() != majorValue )
            majorTicks += majorValue;

        if ( secondsMinor > 0.0 )
        {
            const int numMinorSteps = qwtFloor( secondsMajor / secondsMinor );

            for ( int i = 1; i < numMinorSteps; i++ )
            {
                const QDateTime mt = dt.addMSecs(
                    qRound64( i * secondsMinor * 1000 ) );

                double minorValue = QwtDate::toDouble( mt );
                if ( daylightSaving )
                {
                    const double offset = utcOffset - QwtDate::utcOffset( mt );
                    minorValue += offset * 1000.0;
                }

                if ( minorTicks.isEmpty() || minorTicks.last() != minorValue )
                {
                    const bool isMedium = ( numMinorSteps % 2 == 0 )
                        && ( i != 1 ) && ( i == numMinorSteps / 2 );

                    if ( isMedium )
                        mediumTicks += minorValue;
                    else
                        minorTicks += minorValue;
                }
            }
        }
    }

    QwtScaleDiv scaleDiv;

    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

static QwtScaleDiv qwtDivideToMonths(
    QDateTime& minDate, const QDateTime& maxDate,
    double stepSize, int maxMinSteps )
{
    // months are intervals with non
    // equidistant ( in ms ) steps: we have to build the
    // scale division manually

    int minStepDays = 0;
    int minStepSize = 0.0;

    if ( maxMinSteps > 1 )
    {
        if ( stepSize == 1 )
        {
            if ( maxMinSteps >= 30 )
                minStepDays = 1;
            else if ( maxMinSteps >= 6 )
                minStepDays = 5;
            else if ( maxMinSteps >= 3 )
                minStepDays = 10;
            else
                minStepDays = 15;
        }
        else
        {
            minStepSize = qwtDivideMajorStep(
                stepSize, maxMinSteps, QwtDate::Month );
        }
    }

    QList< double > majorTicks;
    QList< double > mediumTicks;
    QList< double > minorTicks;

    for ( QDateTime dt = minDate;
        dt <= maxDate; dt = dt.addMonths( stepSize ) )
    {
        if ( !dt.isValid() )
            break;

        majorTicks += QwtDate::toDouble( dt );

        if ( minStepDays > 0 )
        {
            for ( int days = minStepDays;
                days < 30; days += minStepDays )
            {
                const double tick = QwtDate::toDouble( dt.addDays( days ) );

                if ( days == 15 && minStepDays != 15 )
                    mediumTicks += tick;
                else
                    minorTicks += tick;
            }
        }
        else if ( minStepSize > 0.0 )
        {
            const int numMinorSteps = qRound( stepSize / (double) minStepSize );

            for ( int i = 1; i < numMinorSteps; i++ )
            {
                const double minorValue =
                    QwtDate::toDouble( dt.addMonths( i * minStepSize ) );

                if ( ( numMinorSteps % 2 == 0 ) && ( i == numMinorSteps / 2 ) )
                    mediumTicks += minorValue;
                else
                    minorTicks += minorValue;
            }
        }
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

static QwtScaleDiv qwtDivideToYears(
    const QDateTime& minDate, const QDateTime& maxDate,
    double stepSize, int maxMinSteps )
{
    QList< double > majorTicks;
    QList< double > mediumTicks;
    QList< double > minorTicks;

    double minStepSize = 0.0;

    if ( maxMinSteps > 1 )
    {
        minStepSize = qwtDivideMajorStep(
            stepSize, maxMinSteps, QwtDate::Year );
    }

    int numMinorSteps = 0;
    if ( minStepSize > 0.0 )
        numMinorSteps = qwtFloor( stepSize / minStepSize );

    bool dateBC = minDate.date().year() < -1;

    for ( QDateTime dt = minDate; dt <= maxDate;
        dt = dt.addYears( stepSize ) )
    {
        if ( dateBC && dt.date().year() > 1 )
        {
            // there is no year 0 in the Julian calendar
            dt = dt.addYears( -1 );
            dateBC = false;
        }

        if ( !dt.isValid() )
            break;

        majorTicks += QwtDate::toDouble( dt );

        for ( int i = 1; i < numMinorSteps; i++ )
        {
            QDateTime tickDate;

            const double years = qRound( i * minStepSize );
            if ( years >= std::numeric_limits< int >::max() / 12 )
            {
                tickDate = dt.addYears( years );
            }
            else
            {
                tickDate = dt.addMonths( qRound( years * 12 ) );
            }

            const bool isMedium = ( numMinorSteps > 2 ) &&
                ( numMinorSteps % 2 == 0 ) && ( i == numMinorSteps / 2 );

            const double minorValue = QwtDate::toDouble( tickDate );
            if ( isMedium )
                mediumTicks += minorValue;
            else
                minorTicks += minorValue;
        }

        if ( QwtDate::maxDate().addYears( -stepSize ) < dt.date() )
        {
            break;
        }
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( QwtDate::toDouble( minDate ),
        QwtDate::toDouble( maxDate ) );

    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MediumTick, mediumTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );

    return scaleDiv;
}

class QwtDateScaleEngine::PrivateData
{
  public:
    explicit PrivateData( Qt::TimeSpec spec )
        : timeSpec( spec )
        , utcOffset( 0 )
        , week0Type( QwtDate::FirstThursday )
        , maxWeeks( 4 )
    {
    }

    Qt::TimeSpec timeSpec;
    int utcOffset;
    QwtDate::Week0Type week0Type;
    int maxWeeks;
};


/*!
   \brief Constructor

   The engine is initialized to build scales for the
   given time specification. It classifies intervals > 4 weeks
   as >= Qt::Month. The first week of a year is defined like
   for QwtDate::FirstThursday.

   \param timeSpec Time specification

   \sa setTimeSpec(), setMaxWeeks(), setWeek0Type()
 */
QwtDateScaleEngine::QwtDateScaleEngine( Qt::TimeSpec timeSpec )
    : QwtLinearScaleEngine( 10 )
{
    m_data = new PrivateData( timeSpec );
}

//! Destructor
QwtDateScaleEngine::~QwtDateScaleEngine()
{
    delete m_data;
}

/*!
   Set the time specification used by the engine

   \param timeSpec Time specification
   \sa timeSpec(), setUtcOffset(), toDateTime()
 */
void QwtDateScaleEngine::setTimeSpec( Qt::TimeSpec timeSpec )
{
    m_data->timeSpec = timeSpec;
}

/*!
   \return Time specification used by the engine
   \sa setTimeSpec(), utcOffset(), toDateTime()
 */
Qt::TimeSpec QwtDateScaleEngine::timeSpec() const
{
    return m_data->timeSpec;
}

/*!
   Set the offset in seconds from Coordinated Universal Time

   \param seconds Offset in seconds

   \note The offset has no effect beside for the time specification
        Qt::OffsetFromUTC.

   \sa QDate::utcOffset(), setTimeSpec(), toDateTime()
 */
void QwtDateScaleEngine::setUtcOffset( int seconds )
{
    m_data->utcOffset = seconds;
}

/*!
   \return Offset in seconds from Coordinated Universal Time
   \note The offset has no effect beside for the time specification
        Qt::OffsetFromUTC.

   \sa QDate::setUtcOffset(), setTimeSpec(), toDateTime()
 */
int QwtDateScaleEngine::utcOffset() const
{
    return m_data->utcOffset;
}

/*!
   Sets how to identify the first week of a year.

   \param week0Type Mode how to identify the first week of a year

   \sa week0Type(), setMaxWeeks()
   \note week0Type has no effect beside for intervals classified as
        QwtDate::Week.
 */
void QwtDateScaleEngine::setWeek0Type( QwtDate::Week0Type week0Type )
{
    m_data->week0Type = week0Type;
}

/*!
   \return Setting how to identify the first week of a year.
   \sa setWeek0Type(), maxWeeks()
 */
QwtDate::Week0Type QwtDateScaleEngine::week0Type() const
{
    return m_data->week0Type;
}

/*!
   Set a upper limit for the number of weeks, when an interval
   can be classified as Qt::Week.

   The default setting is 4 weeks.

   \param weeks Upper limit for the number of weeks

   \note In business charts a year is often divided
        into weeks [1-52]
   \sa maxWeeks(), setWeek0Type()
 */
void QwtDateScaleEngine::setMaxWeeks( int weeks )
{
    m_data->maxWeeks = qMax( weeks, 0 );
}

/*!
   \return Upper limit for the number of weeks, when an interval
          can be classified as Qt::Week.
   \sa setMaxWeeks(), week0Type()
 */
int QwtDateScaleEngine::maxWeeks() const
{
    return m_data->maxWeeks;
}

/*!
   Classification of a date/time interval division

   \param minDate Minimum ( = earlier ) of the interval
   \param maxDate Maximum ( = later ) of the interval
   \param maxSteps Maximum for the number of steps

   \return Interval classification
 */
QwtDate::IntervalType QwtDateScaleEngine::intervalType(
    const QDateTime& minDate, const QDateTime& maxDate,
    int maxSteps ) const
{
    const double jdMin = minDate.date().toJulianDay();
    const double jdMax = maxDate.date().toJulianDay();

    if ( ( jdMax - jdMin ) / 365 > maxSteps )
        return QwtDate::Year;

    const int months = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Month );
    if ( months > maxSteps * 6 )
        return QwtDate::Year;

    const int days = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Day );
    const int weeks = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Week );

    if ( weeks > m_data->maxWeeks )
    {
        if ( days > 4 * maxSteps * 7 )
            return QwtDate::Month;
    }

    if ( days > maxSteps * 7 )
        return QwtDate::Week;

    const int hours = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Hour );
    if ( hours > maxSteps * 24 )
        return QwtDate::Day;

    const int seconds = qwtRoundedIntervalWidth( minDate, maxDate, QwtDate::Second );

    if ( seconds >= maxSteps * 3600 )
        return QwtDate::Hour;

    if ( seconds >= maxSteps * 60 )
        return QwtDate::Minute;

    if ( seconds >= maxSteps )
        return QwtDate::Second;

    return QwtDate::Millisecond;
}

/*!
   Align and divide an interval

   The algorithm aligns and divides the interval into steps.

   Datetime interval divisions are usually not equidistant and the
   calculated stepSize can only be used as an approximation
   for the steps calculated by divideScale().

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)

   \sa QwtScaleEngine::setAttribute()
 */
void QwtDateScaleEngine::autoScale( int maxNumSteps,
    double& x1, double& x2, double& stepSize ) const
{
    stepSize = 0.0;

    QwtInterval interval( x1, x2 );
    interval = interval.normalized();

    interval.setMinValue( interval.minValue() - lowerMargin() );
    interval.setMaxValue( interval.maxValue() + upperMargin() );

    if ( testAttribute( QwtScaleEngine::Symmetric ) )
        interval = interval.symmetrize( reference() );

    if ( testAttribute( QwtScaleEngine::IncludeReference ) )
        interval = interval.extend( reference() );

    if ( interval.width() == 0.0 )
        interval = buildInterval( interval.minValue() );

    const QDateTime from = toDateTime( interval.minValue() );
    const QDateTime to = toDateTime( interval.maxValue() );

    if ( from.isValid() && to.isValid() )
    {
        if ( maxNumSteps < 1 )
            maxNumSteps = 1;

        const QwtDate::IntervalType intvType =
            intervalType( from, to, maxNumSteps );

        const double width = qwtIntervalWidth( from, to, intvType );

        const double stepWidth = qwtDivideScale( width, maxNumSteps, intvType );
        if ( stepWidth != 0.0 && !testAttribute( QwtScaleEngine::Floating ) )
        {
            const QDateTime d1 = alignDate( from, stepWidth, intvType, false );
            const QDateTime d2 = alignDate( to, stepWidth, intvType, true );

            interval.setMinValue( QwtDate::toDouble( d1 ) );
            interval.setMaxValue( QwtDate::toDouble( d2 ) );
        }

        stepSize = stepWidth * qwtMsecsForType( intvType );
    }

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division for a date/time interval

   \param x1 First interval limit
   \param x2 Second interval limit
   \param maxMajorSteps Maximum for the number of major steps
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the scaleEngine
                   calculates one.
   \return Calculated scale division
 */
QwtScaleDiv QwtDateScaleEngine::divideScale( double x1, double x2,
    int maxMajorSteps, int maxMinorSteps, double stepSize ) const
{
    if ( maxMajorSteps < 1 )
        maxMajorSteps = 1;

    const double min = qwtMinF( x1, x2 );
    const double max = qwtMaxF( x1, x2 );

    const QDateTime from = toDateTime( min );
    const QDateTime to = toDateTime( max );

    if ( from == to )
        return QwtScaleDiv();

    stepSize = qAbs( stepSize );
    if ( stepSize > 0.0 )
    {
        // as interval types above hours are not equidistant
        // ( even days might have 23/25 hours because of daylight saving )
        // the stepSize is used as a hint only

        maxMajorSteps = qwtCeil( ( max - min ) / stepSize );
    }

    const QwtDate::IntervalType intvType =
        intervalType( from, to, maxMajorSteps );

    QwtScaleDiv scaleDiv;

    if ( intvType == QwtDate::Millisecond )
    {
        // for milliseconds and below we can use the decimal system
        scaleDiv = QwtLinearScaleEngine::divideScale( min, max,
            maxMajorSteps, maxMinorSteps, stepSize );
    }
    else
    {
        const QDateTime minDate = QwtDate::floor( from, intvType );
        const QDateTime maxDate = QwtDate::ceil( to, intvType );

        scaleDiv = buildScaleDiv( minDate, maxDate,
            maxMajorSteps, maxMinorSteps, intvType );

        // scaleDiv has been calculated from an extended interval
        // adjusted to the step size. We have to shrink it again.

        scaleDiv = scaleDiv.bounded( min, max );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

QwtScaleDiv QwtDateScaleEngine::buildScaleDiv(
    const QDateTime& minDate, const QDateTime& maxDate,
    int maxMajorSteps, int maxMinorSteps,
    QwtDate::IntervalType intervalType ) const
{
    // calculate the step size
    const double stepSize = qwtDivideScale(
        qwtIntervalWidth( minDate, maxDate, intervalType ),
        maxMajorSteps, intervalType );

    // align minDate to the step size
    QDateTime dt0 = alignDate( minDate, stepSize, intervalType, false );
    if ( !dt0.isValid() )
    {
        // the floored date is out of the range of a
        // QDateTime - we ceil instead.
        dt0 = alignDate( minDate, stepSize, intervalType, true );
    }

    QwtScaleDiv scaleDiv;

    if ( intervalType <= QwtDate::Week )
    {
        scaleDiv = qwtDivideToSeconds( dt0, maxDate,
            stepSize, maxMinorSteps, intervalType );
    }
    else
    {
        if( intervalType == QwtDate::Month )
        {
            scaleDiv = qwtDivideToMonths( dt0, maxDate,
                stepSize, maxMinorSteps );
        }
        else if ( intervalType == QwtDate::Year )
        {
            scaleDiv = qwtDivideToYears( dt0, maxDate,
                stepSize, maxMinorSteps );
        }
    }


    return scaleDiv;
}

/*!
   Align a date/time value for a step size

   For Qt::Day alignments there is no "natural day 0" -
   instead the first day of the year is used to avoid jumping
   major ticks positions when panning a scale. For other alignments
   ( f.e according to the first day of the month ) alignDate()
   has to be overloaded.

   \param dateTime Date/time value
   \param stepSize Step size
   \param intervalType Interval type
   \param up When true dateTime is ceiled - otherwise it is floored

   \return Aligned date/time value
 */
QDateTime QwtDateScaleEngine::alignDate(
    const QDateTime& dateTime, double stepSize,
    QwtDate::IntervalType intervalType, bool up ) const
{
    // what about: (year == 1582 && month == 10 && day > 4 && day < 15) ??

    QDateTime dt = dateTime;

    if ( dateTime.timeSpec() == Qt::OffsetFromUTC )
    {
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc( 0 );
#else
        dt.setUtcOffset( 0 );
#endif
    }

    switch( intervalType )
    {
        case QwtDate::Millisecond:
        {
            const int ms = qwtAlignValue(
                dt.time().msec(), stepSize, up );

            dt = QwtDate::floor( dateTime, QwtDate::Second );
            dt = dt.addMSecs( ms );

            break;
        }
        case QwtDate::Second:
        {
            int second = dt.time().second();
            if ( up )
            {
                if ( dt.time().msec() > 0 )
                    second++;
            }

            const int s = qwtAlignValue( second, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Minute );
            dt = dt.addSecs( s );

            break;
        }
        case QwtDate::Minute:
        {
            int minute = dt.time().minute();
            if ( up )
            {
                if ( dt.time().msec() > 0 || dt.time().second() > 0 )
                    minute++;
            }

            const int m = qwtAlignValue( minute, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Hour );
            dt = dt.addSecs( m * 60 );

            break;
        }
        case QwtDate::Hour:
        {
            int hour = dt.time().hour();
            if ( up )
            {
                if ( dt.time().msec() > 0 || dt.time().second() > 0
                    || dt.time().minute() > 0 )
                {
                    hour++;
                }
            }
            const int h = qwtAlignValue( hour, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Day );
            dt = dt.addSecs( h * 3600 );

            break;
        }
        case QwtDate::Day:
        {
            // What date do we expect f.e. from an alignment of 5 days ??
            // Aligning them to the beginning of the year avoids at least
            // jumping major ticks when panning

            int day = dt.date().dayOfYear();
            if ( up )
            {
                if ( dt.time() > QTime( 0, 0 ) )
                    day++;
            }

            const int d = qwtAlignValue( day, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Year );
            dt = dt.addDays( d - 1 );

            break;
        }
        case QwtDate::Week:
        {
            const QDate date = QwtDate::dateOfWeek0(
                dt.date().year(), m_data->week0Type );

            int numWeeks = date.daysTo( dt.date() ) / 7;
            if ( up )
            {
                if ( dt.time() > QTime( 0, 0 ) ||
                    date.daysTo( dt.date() ) % 7 )
                {
                    numWeeks++;
                }
            }

            const int d = qwtAlignValue( numWeeks, stepSize, up ) * 7;

            dt = QwtDate::floor( dt, QwtDate::Day );
            dt.setDate( date );
            dt = dt.addDays( d );

            break;
        }
        case QwtDate::Month:
        {
            int month = dt.date().month();
            if ( up )
            {
                if ( dt.date().day() > 1 ||
                    dt.time() > QTime( 0, 0 ) )
                {
                    month++;
                }
            }

            const int m = qwtAlignValue( month - 1, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Year );
            dt = dt.addMonths( m );

            break;
        }
        case QwtDate::Year:
        {
            int year = dateTime.date().year();
            if ( up )
            {
                if ( dateTime.date().dayOfYear() > 1 ||
                    dt.time() > QTime( 0, 0 ) )
                {
                    year++;
                }
            }

            const int y = qwtAlignValue( year, stepSize, up );

            dt = QwtDate::floor( dt, QwtDate::Day );
            if ( y == 0 )
            {
                // there is no year 0 in the Julian calendar
                dt.setDate( QDate( stepSize, 1, 1 ).addYears( -stepSize ) );
            }
            else
            {
                dt.setDate( QDate( y, 1, 1 ) );
            }

            break;
        }
    }

    if ( dateTime.timeSpec() == Qt::OffsetFromUTC )
    {
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc( dateTime.offsetFromUtc() );
#else
        dt.setUtcOffset( dateTime.utcOffset() );
#endif
    }

    return dt;
}

/*!
   Translate a double value into a QDateTime object.

   For QDateTime result is bounded by QwtDate::minDate() and QwtDate::maxDate()

   \return QDateTime object initialized with timeSpec() and utcOffset().
   \sa timeSpec(), utcOffset(), QwtDate::toDateTime()
 */
QDateTime QwtDateScaleEngine::toDateTime( double value ) const
{
    QDateTime dt = QwtDate::toDateTime( value, m_data->timeSpec );
    if ( !dt.isValid() )
    {
        const QDate date = ( value <= 0.0 )
            ? QwtDate::minDate() : QwtDate::maxDate();

        dt = QDateTime( date, QTime( 0, 0 ), m_data->timeSpec );
    }

    if ( m_data->timeSpec == Qt::OffsetFromUTC )
    {
        dt = dt.addSecs( m_data->utcOffset );
#if QT_VERSION >= 0x050200
        dt.setOffsetFromUtc( m_data->utcOffset );
#else
        dt.setUtcOffset( m_data->utcOffset );
#endif
    }

    return dt;
}

