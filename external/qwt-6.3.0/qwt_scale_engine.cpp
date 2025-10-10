/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_scale_engine.h"
#include "qwt_math.h"
#include "qwt_interval.h"
#include "qwt_transform.h"

#include <qdebug.h>

#include <limits>

static inline double qwtLog( double base, double value )
{
    return std::log( value ) / std::log( base );
}

static inline QwtInterval qwtLogInterval( double base, const QwtInterval& interval )
{
    return QwtInterval( qwtLog( base, interval.minValue() ),
        qwtLog( base, interval.maxValue() ) );
}

static inline QwtInterval qwtPowInterval( double base, const QwtInterval& interval )
{
    return QwtInterval( std::pow( base, interval.minValue() ),
        std::pow( base, interval.maxValue() ) );
}

#if 1

// this version often doesn't find the best ticks: f.e for 15: 5, 10
static double qwtStepSize( double intervalSize, int maxSteps, uint base )
{
    const double minStep =
        QwtScaleArithmetic::divideInterval( intervalSize, maxSteps, base );

    if ( minStep != 0.0 )
    {
        // # ticks per interval
        const int numTicks = qwtCeil( qAbs( intervalSize / minStep ) ) - 1;

        // Do the minor steps fit into the interval?
        if ( qwtFuzzyCompare( ( numTicks + 1 ) * qAbs( minStep ),
            qAbs( intervalSize ), intervalSize ) > 0 )
        {
            // The minor steps doesn't fit into the interval
            return 0.5 * intervalSize;
        }
    }

    return minStep;
}

#else

static double qwtStepSize( double intervalSize, int maxSteps, uint base )
{
    if ( maxSteps <= 0 )
        return 0.0;

    if ( maxSteps > 2 )
    {
        for ( int numSteps = maxSteps; numSteps > 1; numSteps-- )
        {
            const double stepSize = intervalSize / numSteps;

            const double p = std::floor( std::log( stepSize ) / std::log( base ) );
            const double fraction = std::pow( base, p );

            for ( uint n = base; n > 1; n /= 2 )
            {
                if ( qFuzzyCompare( stepSize, n * fraction ) )
                    return stepSize;

                if ( n == 3 && ( base % 2 ) == 0 )
                {
                    if ( qFuzzyCompare( stepSize, 2 * fraction ) )
                        return stepSize;
                }
            }
        }
    }

    return intervalSize * 0.5;
}

#endif

static const double _eps = 1.0e-6;

/*!
   Ceil a value, relative to an interval

   \param value Value to be ceiled
   \param intervalSize Interval size

   \return Rounded value

   \sa floorEps()
 */
double QwtScaleArithmetic::ceilEps( double value,
    double intervalSize )
{
    const double eps = _eps * intervalSize;

    value = ( value - eps ) / intervalSize;
    return std::ceil( value ) * intervalSize;
}

/*!
   Floor a value, relative to an interval

   \param value Value to be floored
   \param intervalSize Interval size

   \return Rounded value
   \sa floorEps()
 */
double QwtScaleArithmetic::floorEps( double value, double intervalSize )
{
    const double eps = _eps * intervalSize;

    value = ( value + eps ) / intervalSize;
    return std::floor( value ) * intervalSize;
}

/*!
   \brief Divide an interval into steps

   \f$stepSize = (intervalSize - intervalSize * 10e^{-6}) / numSteps\f$

   \param intervalSize Interval size
   \param numSteps Number of steps
   \return Step size
 */
double QwtScaleArithmetic::divideEps( double intervalSize, double numSteps )
{
    if ( numSteps == 0.0 || intervalSize == 0.0 )
        return 0.0;

    return ( intervalSize - ( _eps * intervalSize ) ) / numSteps;
}

/*!
   Calculate a step size for a given interval

   \param intervalSize Interval size
   \param numSteps Number of steps
   \param base Base for the division ( usually 10 )

   \return Calculated step size
 */
double QwtScaleArithmetic::divideInterval(
    double intervalSize, int numSteps, uint base )
{
    if ( numSteps <= 0 )
        return 0.0;

    const double v = QwtScaleArithmetic::divideEps( intervalSize, numSteps );
    if ( v == 0.0 )
        return 0.0;

    const double lx = qwtLog( base, std::fabs( v ) );
    const double p = std::floor( lx );

    const double fraction = std::pow( base, lx - p );

    uint n = base;
    while ( ( n > 1 ) && ( fraction <= n / 2 ) )
        n /= 2;

    double stepSize = n * std::pow( base, p );
    if ( v < 0 )
        stepSize = -stepSize;

    return stepSize;
}

class QwtScaleEngine::PrivateData
{
  public:
    PrivateData():
        attributes( QwtScaleEngine::NoAttribute ),
        lowerMargin( 0.0 ),
        upperMargin( 0.0 ),
        referenceValue( 0.0 ),
        base( 10 ),
        transform( NULL )
    {
    }

    ~PrivateData()
    {
        delete transform;
    }

    QwtScaleEngine::Attributes attributes;

    double lowerMargin;
    double upperMargin;

    double referenceValue;

    uint base;

    QwtTransform* transform;
};

/*!
   Constructor

   \param base Base of the scale engine
   \sa setBase()
 */
QwtScaleEngine::QwtScaleEngine( uint base )
{
    m_data = new PrivateData;
    setBase( base );
}


//! Destructor
QwtScaleEngine::~QwtScaleEngine ()
{
    delete m_data;
}

/*!
   Assign a transformation

   \param transform Transformation

   The transformation object is used as factory for clones
   that are returned by transformation()

   The scale engine takes ownership of the transformation.

   \sa QwtTransform::copy(), transformation()

 */
void QwtScaleEngine::setTransformation( QwtTransform* transform )
{
    if ( transform != m_data->transform )
    {
        delete m_data->transform;
        m_data->transform = transform;
    }
}

/*!
   Create and return a clone of the transformation
   of the engine. When the engine has no special transformation
   NULL is returned, indicating no transformation.

   \return A clone of the transformation
   \sa setTransformation()
 */
QwtTransform* QwtScaleEngine::transformation() const
{
    QwtTransform* transform = NULL;
    if ( m_data->transform )
        transform = m_data->transform->copy();

    return transform;
}

/*!
    \return the margin at the lower end of the scale
    The default margin is 0.

    \sa setMargins()
 */
double QwtScaleEngine::lowerMargin() const
{
    return m_data->lowerMargin;
}

/*!
    \return the margin at the upper end of the scale
    The default margin is 0.

    \sa setMargins()
 */
double QwtScaleEngine::upperMargin() const
{
    return m_data->upperMargin;
}

/*!
   \brief Specify margins at the scale's endpoints
   \param lower minimum distance between the scale's lower boundary and the
             smallest enclosed value
   \param upper minimum distance between the scale's upper boundary and the
             greatest enclosed value

   Margins can be used to leave a minimum amount of space between
   the enclosed intervals and the boundaries of the scale.

   \warning
   \li QwtLogScaleEngine measures the margins in decades.

   \sa upperMargin(), lowerMargin()
 */

void QwtScaleEngine::setMargins( double lower, double upper )
{
    m_data->lowerMargin = qwtMaxF( lower, 0.0 );
    m_data->upperMargin = qwtMaxF( upper, 0.0 );
}

/*!
   Calculate a step size for an interval size

   \param intervalSize Interval size
   \param numSteps Number of steps

   \return Step size
 */
double QwtScaleEngine::divideInterval(
    double intervalSize, int numSteps ) const
{
    return QwtScaleArithmetic::divideInterval(
        intervalSize, numSteps, m_data->base );
}

/*!
   Check if an interval "contains" a value

   \param interval Interval
   \param value Value

   \return True, when the value is inside the interval
 */
bool QwtScaleEngine::contains(
    const QwtInterval& interval, double value ) const
{
    if ( !interval.isValid() )
        return false;

    if ( qwtFuzzyCompare( value, interval.minValue(), interval.width() ) < 0 )
        return false;

    if ( qwtFuzzyCompare( value, interval.maxValue(), interval.width() ) > 0 )
        return false;

    return true;
}

/*!
   Remove ticks from a list, that are not inside an interval

   \param ticks Tick list
   \param interval Interval

   \return Stripped tick list
 */
QList< double > QwtScaleEngine::strip( const QList< double >& ticks,
    const QwtInterval& interval ) const
{
    if ( !interval.isValid() || ticks.count() == 0 )
        return QList< double >();

    if ( contains( interval, ticks.first() )
        && contains( interval, ticks.last() ) )
    {
        return ticks;
    }

    QList< double > strippedTicks;
    for ( int i = 0; i < ticks.count(); i++ )
    {
        if ( contains( interval, ticks[i] ) )
            strippedTicks += ticks[i];
    }
    return strippedTicks;
}

/*!
   \brief Build an interval around a value

   In case of v == 0.0 the interval is [-0.5, 0.5],
   otherwise it is [0.5 * v, 1.5 * v]

   \param value Initial value
   \return Calculated interval
 */

QwtInterval QwtScaleEngine::buildInterval( double value ) const
{
    const double delta = ( value == 0.0 ) ? 0.5 : qAbs( 0.5 * value );
    const double max = std::numeric_limits< double >::max();

    if ( max - delta < value )
        return QwtInterval( max - delta, max );

    if ( -max + delta > value )
        return QwtInterval( -max, -max + delta );

    return QwtInterval( value - delta, value + delta );
}

/*!
   Change a scale attribute

   \param attribute Attribute to change
   \param on On/Off

   \sa Attribute, testAttribute()
 */
void QwtScaleEngine::setAttribute( Attribute attribute, bool on )
{
    if ( on )
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;
}

/*!
   \return True, if attribute is enabled.

   \param attribute Attribute to be tested
   \sa Attribute, setAttribute()
 */
bool QwtScaleEngine::testAttribute( Attribute attribute ) const
{
    return ( m_data->attributes & attribute );
}

/*!
   Change the scale attribute

   \param attributes Set scale attributes
   \sa Attribute, attributes()
 */
void QwtScaleEngine::setAttributes( Attributes attributes )
{
    m_data->attributes = attributes;
}

/*!
   \return Scale attributes
   \sa Attribute, setAttributes(), testAttribute()
 */
QwtScaleEngine::Attributes QwtScaleEngine::attributes() const
{
    return m_data->attributes;
}

/*!
   \brief Specify a reference point
   \param reference New reference value

   The reference point is needed if options IncludeReference or
   Symmetric are active. Its default value is 0.0.

   \sa Attribute
 */
void QwtScaleEngine::setReference( double reference )
{
    m_data->referenceValue = reference;
}

/*!
   \return the reference value
   \sa setReference(), setAttribute()
 */
double QwtScaleEngine::reference() const
{
    return m_data->referenceValue;
}

/*!
   Set the base of the scale engine

   While a base of 10 is what 99.9% of all applications need
   certain scales might need a different base: f.e 2

   The default setting is 10

   \param base Base of the engine

   \sa base()
 */
void QwtScaleEngine::setBase( uint base )
{
    m_data->base = qMax( base, 2U );
}

/*!
   \return base Base of the scale engine
   \sa setBase()
 */
uint QwtScaleEngine::base() const
{
    return m_data->base;
}

/*!
   Constructor

   \param base Base of the scale engine
   \sa setBase()
 */
QwtLinearScaleEngine::QwtLinearScaleEngine( uint base ):
    QwtScaleEngine( base )
{
}

//! Destructor
QwtLinearScaleEngine::~QwtLinearScaleEngine()
{
}

/*!
   Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)

   \sa setAttribute()
 */
void QwtLinearScaleEngine::autoScale( int maxNumSteps,
    double& x1, double& x2, double& stepSize ) const
{
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

    stepSize = QwtScaleArithmetic::divideInterval(
        interval.width(), qMax( maxNumSteps, 1 ), base() );

    if ( !testAttribute( QwtScaleEngine::Floating ) )
        interval = align( interval, stepSize );

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division for an interval

   \param x1 First interval limit
   \param x2 Second interval limit
   \param maxMajorSteps Maximum for the number of major steps
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the engine
                   calculates one.

   \return Calculated scale division
 */
QwtScaleDiv QwtLinearScaleEngine::divideScale( double x1, double x2,
    int maxMajorSteps, int maxMinorSteps, double stepSize ) const
{
    QwtInterval interval = QwtInterval( x1, x2 ).normalized();

    if ( interval.widthL() > std::numeric_limits< double >::max() )
    {
        qWarning() << "QwtLinearScaleEngine::divideScale: overflow";
        return QwtScaleDiv();
    }

    if ( interval.width() <= 0 )
        return QwtScaleDiv();

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
        if ( maxMajorSteps < 1 )
            maxMajorSteps = 1;

        stepSize = QwtScaleArithmetic::divideInterval(
            interval.width(), maxMajorSteps, base() );
    }

    QwtScaleDiv scaleDiv;

    if ( stepSize != 0.0 )
    {
        QList< double > ticks[QwtScaleDiv::NTickTypes];
        buildTicks( interval, stepSize, maxMinorSteps, ticks );

        scaleDiv = QwtScaleDiv( interval, ticks );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

/*!
   \brief Calculate ticks for an interval

   \param interval Interval
   \param stepSize Step size
   \param maxMinorSteps Maximum number of minor steps
   \param ticks Arrays to be filled with the calculated ticks

   \sa buildMajorTicks(), buildMinorTicks
 */
void QwtLinearScaleEngine::buildTicks(
    const QwtInterval& interval, double stepSize, int maxMinorSteps,
    QList< double > ticks[QwtScaleDiv::NTickTypes] ) const
{
    const QwtInterval boundingInterval = align( interval, stepSize );

    ticks[QwtScaleDiv::MajorTick] =
        buildMajorTicks( boundingInterval, stepSize );

    if ( maxMinorSteps > 0 )
    {
        buildMinorTicks( ticks[QwtScaleDiv::MajorTick], maxMinorSteps, stepSize,
            ticks[QwtScaleDiv::MinorTick], ticks[QwtScaleDiv::MediumTick] );
    }

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        ticks[i] = strip( ticks[i], interval );

        // ticks very close to 0.0 are explicitly set to 0.0

        for ( int j = 0; j < ticks[i].count(); j++ )
        {
            if ( qwtFuzzyCompare( ticks[i][j], 0.0, stepSize ) == 0 )
                ticks[i][j] = 0.0;
        }
    }
}

/*!
   \brief Calculate major ticks for an interval

   \param interval Interval
   \param stepSize Step size

   \return Calculated ticks
 */
QList< double > QwtLinearScaleEngine::buildMajorTicks(
    const QwtInterval& interval, double stepSize ) const
{
    int numTicks = qRound( interval.width() / stepSize ) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    QList< double > ticks;
    ticks.reserve( numTicks );

    ticks += interval.minValue();
    for ( int i = 1; i < numTicks - 1; i++ )
        ticks += interval.minValue() + i * stepSize;
    ticks += interval.maxValue();

    return ticks;
}

/*!
   \brief Calculate minor/medium ticks for major ticks

   \param majorTicks Major ticks
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size
   \param minorTicks Array to be filled with the calculated minor ticks
   \param mediumTicks Array to be filled with the calculated medium ticks

 */
void QwtLinearScaleEngine::buildMinorTicks(
    const QList< double >& majorTicks,
    int maxMinorSteps, double stepSize,
    QList< double >& minorTicks,
    QList< double >& mediumTicks ) const
{
    double minStep = qwtStepSize( stepSize, maxMinorSteps, base() );
    if ( minStep == 0.0 )
        return;

    // # ticks per interval
    const int numTicks = qwtCeil( qAbs( stepSize / minStep ) ) - 1;

    int medIndex = -1;
    if ( numTicks % 2 )
        medIndex = numTicks / 2;

    // calculate minor ticks

    for ( int i = 0; i < majorTicks.count(); i++ )
    {
        double val = majorTicks[i];
        for ( int k = 0; k < numTicks; k++ )
        {
            val += minStep;

            double alignedValue = val;
            if ( qwtFuzzyCompare( val, 0.0, stepSize ) == 0 )
                alignedValue = 0.0;

            if ( k == medIndex )
                mediumTicks += alignedValue;
            else
                minorTicks += alignedValue;
        }
    }
}

/*!
   \brief Align an interval to a step size

   The limits of an interval are aligned that both are integer
   multiples of the step size.

   \param interval Interval
   \param stepSize Step size

   \return Aligned interval
 */
QwtInterval QwtLinearScaleEngine::align(
    const QwtInterval& interval, double stepSize ) const
{
    double x1 = interval.minValue();
    double x2 = interval.maxValue();

    // when there is no rounding beside some effect, when
    // calculating with doubles, we keep the original value

    const double eps = 0.000000000001; // since Qt 4.8: qFuzzyIsNull
    const double max = std::numeric_limits< double >::max();

    if ( -max + stepSize <= x1 )
    {
        const double x = QwtScaleArithmetic::floorEps( x1, stepSize );
        if ( qAbs(x) <= eps || !qFuzzyCompare( x1, x ) )
            x1 = x;
    }

    if ( max - stepSize >= x2 )
    {
        const double x = QwtScaleArithmetic::ceilEps( x2, stepSize );
        if ( qAbs(x) <= eps || !qFuzzyCompare( x2, x ) )
            x2 = x;
    }

    return QwtInterval( x1, x2 );
}

/*!
   Constructor

   \param base Base of the scale engine
   \sa setBase()
 */
QwtLogScaleEngine::QwtLogScaleEngine( uint base ):
    QwtScaleEngine( base )
{
    setTransformation( new QwtLogTransform() );
}

//! Destructor
QwtLogScaleEngine::~QwtLogScaleEngine()
{
}

/*!
    Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)

   \sa QwtScaleEngine::setAttribute()
 */
void QwtLogScaleEngine::autoScale( int maxNumSteps,
    double& x1, double& x2, double& stepSize ) const
{
    if ( x1 > x2 )
        qSwap( x1, x2 );

    const double logBase = base();

    QwtInterval interval( x1 / std::pow( logBase, lowerMargin() ),
        x2 * std::pow( logBase, upperMargin() ) );

    if ( interval.maxValue() / interval.minValue() < logBase )
    {
        // scale width is less than one step -> try to build a linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        linearScaler.autoScale( maxNumSteps, x1, x2, stepSize );

        QwtInterval linearInterval = QwtInterval( x1, x2 ).normalized();
        linearInterval = linearInterval.limited(
            QwtLogTransform::LogMin, QwtLogTransform::LogMax );

        if ( linearInterval.maxValue() / linearInterval.minValue() < logBase )
        {
            stepSize = 0.0;
            return;
        }
    }

    double logRef = 1.0;
    if ( reference() > QwtLogTransform::LogMin / 2 )
        logRef = qwtMinF( reference(), QwtLogTransform::LogMax / 2 );

    if ( testAttribute( QwtScaleEngine::Symmetric ) )
    {
        const double delta = qwtMaxF( interval.maxValue() / logRef,
            logRef / interval.minValue() );
        interval.setInterval( logRef / delta, logRef * delta );
    }

    if ( testAttribute( QwtScaleEngine::IncludeReference ) )
        interval = interval.extend( logRef );

    interval = interval.limited( QwtLogTransform::LogMin, QwtLogTransform::LogMax );

    if ( interval.width() == 0.0 )
        interval = buildInterval( interval.minValue() );

    stepSize = divideInterval( qwtLogInterval( logBase, interval ).width(),
        qMax( maxNumSteps, 1 ) );
    if ( stepSize < 1.0 )
        stepSize = 1.0;

    if ( !testAttribute( QwtScaleEngine::Floating ) )
        interval = align( interval, stepSize );

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division for an interval

   \param x1 First interval limit
   \param x2 Second interval limit
   \param maxMajorSteps Maximum for the number of major steps
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the engine
                   calculates one.

   \return Calculated scale division
 */
QwtScaleDiv QwtLogScaleEngine::divideScale( double x1, double x2,
    int maxMajorSteps, int maxMinorSteps, double stepSize ) const
{
    QwtInterval interval = QwtInterval( x1, x2 ).normalized();
    interval = interval.limited( QwtLogTransform::LogMin, QwtLogTransform::LogMax );

    if ( interval.width() <= 0 )
        return QwtScaleDiv();

    const double logBase = base();

    if ( interval.maxValue() / interval.minValue() < logBase )
    {
        // scale width is less than one decade -> build linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        return linearScaler.divideScale( x1, x2,
            maxMajorSteps, maxMinorSteps, 0.0 );
    }

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
        if ( maxMajorSteps < 1 )
            maxMajorSteps = 1;

        stepSize = divideInterval(
            qwtLogInterval( logBase, interval ).width(), maxMajorSteps );
        if ( stepSize < 1.0 )
            stepSize = 1.0; // major step must be >= 1 decade
    }

    QwtScaleDiv scaleDiv;
    if ( stepSize != 0.0 )
    {
        QList< double > ticks[QwtScaleDiv::NTickTypes];
        buildTicks( interval, stepSize, maxMinorSteps, ticks );

        scaleDiv = QwtScaleDiv( interval, ticks );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

/*!
   \brief Calculate ticks for an interval

   \param interval Interval
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size
   \param ticks Arrays to be filled with the calculated ticks

   \sa buildMajorTicks(), buildMinorTicks
 */
void QwtLogScaleEngine::buildTicks(
    const QwtInterval& interval, double stepSize, int maxMinorSteps,
    QList< double > ticks[QwtScaleDiv::NTickTypes] ) const
{
    const QwtInterval boundingInterval = align( interval, stepSize );

    ticks[QwtScaleDiv::MajorTick] =
        buildMajorTicks( boundingInterval, stepSize );

    if ( maxMinorSteps > 0 )
    {
        buildMinorTicks( ticks[QwtScaleDiv::MajorTick], maxMinorSteps, stepSize,
            ticks[QwtScaleDiv::MinorTick], ticks[QwtScaleDiv::MediumTick] );
    }

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
        ticks[i] = strip( ticks[i], interval );
}

/*!
   \brief Calculate major ticks for an interval

   \param interval Interval
   \param stepSize Step size

   \return Calculated ticks
 */
QList< double > QwtLogScaleEngine::buildMajorTicks(
    const QwtInterval& interval, double stepSize ) const
{
    double width = qwtLogInterval( base(), interval ).width();

    int numTicks = qRound( width / stepSize ) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    const double lxmin = std::log( interval.minValue() );
    const double lxmax = std::log( interval.maxValue() );
    const double lstep = ( lxmax - lxmin ) / double( numTicks - 1 );

    QList< double > ticks;
    ticks.reserve( numTicks );

    ticks += interval.minValue();

    for ( int i = 1; i < numTicks - 1; i++ )
        ticks += std::exp( lxmin + double( i ) * lstep );

    ticks += interval.maxValue();

    return ticks;
}

/*!
   \brief Calculate minor/medium ticks for major ticks

   \param majorTicks Major ticks
   \param maxMinorSteps Maximum number of minor steps
   \param stepSize Step size
   \param minorTicks Array to be filled with the calculated minor ticks
   \param mediumTicks Array to be filled with the calculated medium ticks
 */
void QwtLogScaleEngine::buildMinorTicks(
    const QList< double >& majorTicks,
    int maxMinorSteps, double stepSize,
    QList< double >& minorTicks,
    QList< double >& mediumTicks ) const
{
    const double logBase = base();

    if ( stepSize < 1.1 )          // major step width is one base
    {
        double minStep = divideInterval( stepSize, maxMinorSteps + 1 );
        if ( minStep == 0.0 )
            return;

        const int numSteps = qRound( stepSize / minStep );

        int mediumTickIndex = -1;
        if ( ( numSteps > 2 ) && ( numSteps % 2 == 0 ) )
            mediumTickIndex = numSteps / 2;

        for ( int i = 0; i < majorTicks.count() - 1; i++ )
        {
            const double v = majorTicks[i];
            const double s = logBase / numSteps;

            if ( s >= 1.0 )
            {
                if ( !qFuzzyCompare( s, 1.0 ) )
                    minorTicks += v * s;

                for ( int j = 2; j < numSteps; j++ )
                {
                    minorTicks += v * j * s;
                }
            }
            else
            {
                for ( int j = 1; j < numSteps; j++ )
                {
                    const double tick = v + j * v * ( logBase - 1 ) / numSteps;
                    if ( j == mediumTickIndex )
                        mediumTicks += tick;
                    else
                        minorTicks += tick;
                }
            }
        }
    }
    else
    {
        double minStep = divideInterval( stepSize, maxMinorSteps );
        if ( minStep == 0.0 )
            return;

        if ( minStep < 1.0 )
            minStep = 1.0;

        // # subticks per interval
        int numTicks = qRound( stepSize / minStep ) - 1;

        // Do the minor steps fit into the interval?
        if ( qwtFuzzyCompare( ( numTicks + 1 ) * minStep,
            stepSize, stepSize ) > 0 )
        {
            numTicks = 0;
        }

        if ( numTicks < 1 )
            return;

        int mediumTickIndex = -1;
        if ( ( numTicks > 2 ) && ( numTicks % 2 ) )
            mediumTickIndex = numTicks / 2;

        // substep factor = base^substeps
        const qreal minFactor = qwtMaxF( std::pow( logBase, minStep ), logBase );

        for ( int i = 0; i < majorTicks.count(); i++ )
        {
            double tick = majorTicks[i];
            for ( int j = 0; j < numTicks; j++ )
            {
                tick *= minFactor;

                if ( j == mediumTickIndex )
                    mediumTicks += tick;
                else
                    minorTicks += tick;
            }
        }
    }
}

/*!
   \brief Align an interval to a step size

   The limits of an interval are aligned that both are integer
   multiples of the step size.

   \param interval Interval
   \param stepSize Step size

   \return Aligned interval
 */
QwtInterval QwtLogScaleEngine::align(
    const QwtInterval& interval, double stepSize ) const
{
    const QwtInterval intv = qwtLogInterval( base(), interval );

    double x1 = QwtScaleArithmetic::floorEps( intv.minValue(), stepSize );
    if ( qwtFuzzyCompare( interval.minValue(), x1, stepSize ) == 0 )
        x1 = interval.minValue();

    double x2 = QwtScaleArithmetic::ceilEps( intv.maxValue(), stepSize );
    if ( qwtFuzzyCompare( interval.maxValue(), x2, stepSize ) == 0 )
        x2 = interval.maxValue();

    return qwtPowInterval( base(), QwtInterval( x1, x2 ) );
}
