/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_abstract_scale.h"
#include "qwt_scale_engine.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_div.h"
#include "qwt_scale_map.h"
#include "qwt_interval.h"

#include <qcoreevent.h>

class QwtAbstractScale::PrivateData
{
  public:
    PrivateData():
        maxMajor( 5 ),
        maxMinor( 3 ),
        stepSize( 0.0 )
    {
        scaleEngine = new QwtLinearScaleEngine();
        scaleDraw = new QwtScaleDraw();
    }

    ~PrivateData()
    {
        delete scaleEngine;
        delete scaleDraw;
    }

    QwtScaleEngine* scaleEngine;
    QwtAbstractScaleDraw* scaleDraw;

    int maxMajor;
    int maxMinor;
    double stepSize;
};

/*!
   Constructor

   \param parent Parent widget

   Creates a default QwtScaleDraw and a QwtLinearScaleEngine.
   The initial scale boundaries are set to [ 0.0, 100.0 ]

   The scaleStepSize() is initialized to 0.0, scaleMaxMajor() to 5
   and scaleMaxMajor to 3.
 */

QwtAbstractScale::QwtAbstractScale( QWidget* parent )
    : QWidget( parent )
{
    m_data = new PrivateData;
    rescale( 0.0, 100.0, m_data->stepSize );
}

//! Destructor
QwtAbstractScale::~QwtAbstractScale()
{
    delete m_data;
}

/*!
   Set the lower bound of the scale

   \param value Lower bound

   \sa lowerBound(), setScale(), setUpperBound()
   \note For inverted scales the lower bound
        is greater than the upper bound
 */
void QwtAbstractScale::setLowerBound( double value )
{
    setScale( value, upperBound() );
}

/*!
   \return Lower bound of the scale
   \sa setLowerBound(), setScale(), upperBound()
 */
double QwtAbstractScale::lowerBound() const
{
    return m_data->scaleDraw->scaleDiv().lowerBound();
}

/*!
   Set the upper bound of the scale

   \param value Upper bound

   \sa upperBound(), setScale(), setLowerBound()
   \note For inverted scales the lower bound
        is greater than the upper bound
 */
void QwtAbstractScale::setUpperBound( double value )
{
    setScale( lowerBound(), value );
}

/*!
   \return Upper bound of the scale
   \sa setUpperBound(), setScale(), lowerBound()
 */
double QwtAbstractScale::upperBound() const
{
    return m_data->scaleDraw->scaleDiv().upperBound();
}

/*!
   \brief Specify a scale.

   Define a scale by an interval

   The ticks are calculated using scaleMaxMinor(),
   scaleMaxMajor() and scaleStepSize().

   \param lowerBound lower limit of the scale interval
   \param upperBound upper limit of the scale interval

   \note For inverted scales the lower bound
        is greater than the upper bound
 */
void QwtAbstractScale::setScale( double lowerBound, double upperBound )
{
    rescale( lowerBound, upperBound, m_data->stepSize );
}

/*!
   \brief Specify a scale.

   Define a scale by an interval

   The ticks are calculated using scaleMaxMinor(),
   scaleMaxMajor() and scaleStepSize().

   \param interval Interval
 */
void QwtAbstractScale::setScale( const QwtInterval& interval )
{
    setScale( interval.minValue(), interval.maxValue() );
}

/*!
   \brief Specify a scale.

   scaleMaxMinor(), scaleMaxMajor() and scaleStepSize() and have no effect.

   \param scaleDiv Scale division
   \sa setAutoScale()
 */
void QwtAbstractScale::setScale( const QwtScaleDiv& scaleDiv )
{
    if ( scaleDiv != m_data->scaleDraw->scaleDiv() )
    {
#if 1
        if ( m_data->scaleEngine )
        {
            m_data->scaleDraw->setTransformation(
                m_data->scaleEngine->transformation() );
        }
#endif

        m_data->scaleDraw->setScaleDiv( scaleDiv );

        scaleChange();
    }
}

/*!
   \brief Set the maximum number of major tick intervals.

   The scale's major ticks are calculated automatically such that
   the number of major intervals does not exceed ticks.

   The default value is 5.

   \param ticks Maximal number of major ticks.

   \sa scaleMaxMajor(), setScaleMaxMinor(),
      setScaleStepSize(), QwtScaleEngine::divideInterval()
 */
void QwtAbstractScale::setScaleMaxMajor( int ticks )
{
    if ( ticks != m_data->maxMajor )
    {
        m_data->maxMajor = ticks;
        updateScaleDraw();
    }
}

/*!
   \return Maximal number of major tick intervals
   \sa setScaleMaxMajor(), scaleMaxMinor()
 */
int QwtAbstractScale::scaleMaxMajor() const
{
    return m_data->maxMajor;
}

/*!
   \brief Set the maximum number of minor tick intervals

   The scale's minor ticks are calculated automatically such that
   the number of minor intervals does not exceed ticks.
   The default value is 3.

   \param ticks Maximal number of minor ticks.

   \sa scaleMaxMajor(), setScaleMaxMinor(),
      setScaleStepSize(), QwtScaleEngine::divideInterval()
 */
void QwtAbstractScale::setScaleMaxMinor( int ticks )
{
    if ( ticks != m_data->maxMinor )
    {
        m_data->maxMinor = ticks;
        updateScaleDraw();
    }
}

/*!
   \return Maximal number of minor tick intervals
   \sa setScaleMaxMinor(), scaleMaxMajor()
 */
int QwtAbstractScale::scaleMaxMinor() const
{
    return m_data->maxMinor;
}

/*!
   \brief Set the step size used for calculating a scale division

   The step size is hint for calculating the intervals for
   the major ticks of the scale. A value of 0.0 is interpreted
   as no hint.

   \param stepSize Hint for the step size of the scale

   \sa scaleStepSize(), QwtScaleEngine::divideScale()

   \note Position and distance between the major ticks also
         depends on scaleMaxMajor().
 */
void QwtAbstractScale::setScaleStepSize( double stepSize )
{
    if ( stepSize != m_data->stepSize )
    {
        m_data->stepSize = stepSize;
        updateScaleDraw();
    }
}

/*!
   \return Hint for the step size of the scale
   \sa setScaleStepSize(), QwtScaleEngine::divideScale()
 */
double QwtAbstractScale::scaleStepSize() const
{
    return m_data->stepSize;
}

/*!
   \brief Set a scale draw

   scaleDraw has to be created with new and will be deleted in
   the destructor or the next call of setAbstractScaleDraw().

   \sa abstractScaleDraw()
 */
void QwtAbstractScale::setAbstractScaleDraw( QwtAbstractScaleDraw* scaleDraw )
{
    if ( scaleDraw == NULL || scaleDraw == m_data->scaleDraw )
        return;

    if ( m_data->scaleDraw != NULL )
        scaleDraw->setScaleDiv( m_data->scaleDraw->scaleDiv() );

    delete m_data->scaleDraw;
    m_data->scaleDraw = scaleDraw;
}

/*!
    \return Scale draw
    \sa setAbstractScaleDraw()
 */
QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw()
{
    return m_data->scaleDraw;
}

/*!
    \return Scale draw
    \sa setAbstractScaleDraw()
 */
const QwtAbstractScaleDraw* QwtAbstractScale::abstractScaleDraw() const
{
    return m_data->scaleDraw;
}

/*!
   \brief Set a scale engine

   The scale engine is responsible for calculating the scale division
   and provides a transformation between scale and widget coordinates.

   scaleEngine has to be created with new and will be deleted in
   the destructor or the next call of setScaleEngine.
 */
void QwtAbstractScale::setScaleEngine( QwtScaleEngine* scaleEngine )
{
    if ( scaleEngine != NULL && scaleEngine != m_data->scaleEngine )
    {
        delete m_data->scaleEngine;
        m_data->scaleEngine = scaleEngine;
    }
}

/*!
   \return Scale engine
   \sa setScaleEngine()
 */
const QwtScaleEngine* QwtAbstractScale::scaleEngine() const
{
    return m_data->scaleEngine;
}

/*!
   \return Scale engine
   \sa setScaleEngine()
 */
QwtScaleEngine* QwtAbstractScale::scaleEngine()
{
    return m_data->scaleEngine;
}

/*!
   \return Scale boundaries and positions of the ticks

   The scale division might have been assigned explicitly
   or calculated implicitly by rescale().
 */
const QwtScaleDiv& QwtAbstractScale::scaleDiv() const
{
    return m_data->scaleDraw->scaleDiv();
}

/*!
   \return Map to translate between scale and widget coordinates
 */
const QwtScaleMap& QwtAbstractScale::scaleMap() const
{
    return m_data->scaleDraw->scaleMap();
}

/*!
   Translate a scale value into a widget coordinate

   \param value Scale value
   \return Corresponding widget coordinate for value
   \sa scaleMap(), invTransform()
 */
int QwtAbstractScale::transform( double value ) const
{
    return qRound( m_data->scaleDraw->scaleMap().transform( value ) );
}

/*!
   Translate a widget coordinate into a scale value

   \param value Widget coordinate
   \return Corresponding scale coordinate for value
   \sa scaleMap(), transform()
 */
double QwtAbstractScale::invTransform( int value ) const
{
    return m_data->scaleDraw->scaleMap().invTransform( value );
}

/*!
   \return True, when the scale is increasing in opposite direction
          to the widget coordinates
 */
bool QwtAbstractScale::isInverted() const
{
    return m_data->scaleDraw->scaleMap().isInverting();
}

/*!
   \return The boundary with the smaller value
   \sa maximum(), lowerBound(), upperBound()
 */
double QwtAbstractScale::minimum() const
{
    return qMin( m_data->scaleDraw->scaleDiv().lowerBound(),
        m_data->scaleDraw->scaleDiv().upperBound() );
}

/*!
   \return The boundary with the larger value
   \sa minimum(), lowerBound(), upperBound()
 */
double QwtAbstractScale::maximum() const
{
    return qMax( m_data->scaleDraw->scaleDiv().lowerBound(),
        m_data->scaleDraw->scaleDiv().upperBound() );
}

//! Notify changed scale
void QwtAbstractScale::scaleChange()
{
}

/*!
   Recalculate the scale division and update the scale.

   \param lowerBound Lower limit of the scale interval
   \param upperBound Upper limit of the scale interval
   \param stepSize Major step size

   \sa scaleChange()
 */
void QwtAbstractScale::rescale(
    double lowerBound, double upperBound, double stepSize )
{
    const QwtScaleDiv scaleDiv = m_data->scaleEngine->divideScale(
        lowerBound, upperBound, m_data->maxMajor, m_data->maxMinor, stepSize );

    if ( scaleDiv != m_data->scaleDraw->scaleDiv() )
    {
#if 1
        m_data->scaleDraw->setTransformation(
            m_data->scaleEngine->transformation() );
#endif

        m_data->scaleDraw->setScaleDiv( scaleDiv );
        scaleChange();
    }
}

/*!
   Change Event handler
   \param event Change event

   Invalidates internal caches if necessary
 */
void QwtAbstractScale::changeEvent( QEvent* event )
{
    if ( event->type() == QEvent::LocaleChange )
    {
        m_data->scaleDraw->invalidateCache();
    }

    QWidget::changeEvent( event );
}

/*!
   Recalculate ticks and scale boundaries.
 */
void QwtAbstractScale::updateScaleDraw()
{
    rescale( m_data->scaleDraw->scaleDiv().lowerBound(),
        m_data->scaleDraw->scaleDiv().upperBound(), m_data->stepSize );
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_abstract_scale.cpp"
#endif
