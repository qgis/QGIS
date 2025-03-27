/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_abstract_slider.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"
#include "qwt_math.h"

#include <qevent.h>

static double qwtAlignToScaleDiv(
    const QwtAbstractSlider* slider, double value )
{
    const QwtScaleDiv& sd = slider->scaleDiv();

    const int tValue = slider->transform( value );

    if ( tValue == slider->transform( sd.lowerBound() ) )
        return sd.lowerBound();

    if ( tValue == slider->transform( sd.upperBound() ) )
        return sd.upperBound();

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        const QList< double > ticks = sd.ticks( i );
        for ( int j = 0; j < ticks.size(); j++ )
        {
            if ( slider->transform( ticks[ j ] ) == tValue )
                return ticks[ j ];
        }
    }

    return value;
}

class QwtAbstractSlider::PrivateData
{
  public:
    PrivateData()
        : isScrolling( false )
        , isTracking( true )
        , pendingValueChanged( false )
        , readOnly( false )
        , totalSteps( 100 )
        , singleSteps( 1 )
        , pageSteps( 10 )
        , stepAlignment( true )
        , isValid( false )
        , value( 0.0 )
        , wrapping( false )
        , invertedControls( false )
    {
    }

    bool isScrolling;
    bool isTracking;
    bool pendingValueChanged;

    bool readOnly;

    uint totalSteps;
    uint singleSteps;
    uint pageSteps;
    bool stepAlignment;

    bool isValid;
    double value;

    bool wrapping;
    bool invertedControls;
};

/*!
   \brief Constructor

   The scale is initialized to [0.0, 100.0], the
   number of steps is set to 100 with 1 and 10 and single
   an page step sizes. Step alignment is enabled.

   The initial value is invalid.

   \param parent Parent widget
 */
QwtAbstractSlider::QwtAbstractSlider( QWidget* parent )
    : QwtAbstractScale( parent )
{
    m_data = new QwtAbstractSlider::PrivateData;

    setScale( 0.0, 100.0 );
    setFocusPolicy( Qt::StrongFocus );
}

//! Destructor
QwtAbstractSlider::~QwtAbstractSlider()
{
    delete m_data;
}

/*!
   Set the value to be valid/invalid

   \param on When true, the value is invalidated

   \sa setValue()
 */
void QwtAbstractSlider::setValid( bool on )
{
    if ( on != m_data->isValid )
    {
        m_data->isValid = on;
        sliderChange();

        Q_EMIT valueChanged( m_data->value );
    }
}

//! \return True, when the value is invalid
bool QwtAbstractSlider::isValid() const
{
    return m_data->isValid;
}

/*!
   En/Disable read only mode

   In read only mode the slider can't be controlled by mouse
   or keyboard.

   \param on Enables in case of true
   \sa isReadOnly()

   \warning The focus policy is set to Qt::StrongFocus or Qt::NoFocus
 */
void QwtAbstractSlider::setReadOnly( bool on )
{
    if ( m_data->readOnly != on )
    {
        m_data->readOnly = on;
        setFocusPolicy( on ? Qt::StrongFocus : Qt::NoFocus );

        update();
    }
}

/*!
   In read only mode the slider can't be controlled by mouse
   or keyboard.

   \return true if read only
   \sa setReadOnly()
 */
bool QwtAbstractSlider::isReadOnly() const
{
    return m_data->readOnly;
}

/*!
   \brief Enables or disables tracking.

   If tracking is enabled, the slider emits the valueChanged()
   signal while the movable part of the slider is being dragged.
   If tracking is disabled, the slider emits the valueChanged() signal
   only when the user releases the slider.

   Tracking is enabled by default.
   \param on \c true (enable) or \c false (disable) tracking.

   \sa isTracking(), sliderMoved()
 */
void QwtAbstractSlider::setTracking( bool on )
{
    m_data->isTracking = on;
}

/*!
   \return True, when tracking has been enabled
   \sa setTracking()
 */
bool QwtAbstractSlider::isTracking() const
{
    return m_data->isTracking;
}

/*!
   Mouse press event handler
   \param event Mouse event
 */
void QwtAbstractSlider::mousePressEvent( QMouseEvent* event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !m_data->isValid || lowerBound() == upperBound() )
        return;

    m_data->isScrolling = isScrollPosition( event->pos() );

    if ( m_data->isScrolling )
    {
        m_data->pendingValueChanged = false;

        Q_EMIT sliderPressed();
    }
}

/*!
   Mouse Move Event handler
   \param event Mouse event
 */
void QwtAbstractSlider::mouseMoveEvent( QMouseEvent* event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( m_data->isValid && m_data->isScrolling )
    {
        double value = scrolledTo( event->pos() );
        if ( value != m_data->value )
        {
            value = boundedValue( value );

            if ( m_data->stepAlignment )
            {
                value = alignedValue( value );
            }
            else
            {
                value = qwtAlignToScaleDiv( this, value );
            }

            if ( value != m_data->value )
            {
                m_data->value = value;

                sliderChange();

                Q_EMIT sliderMoved( m_data->value );

                if ( m_data->isTracking )
                    Q_EMIT valueChanged( m_data->value );
                else
                    m_data->pendingValueChanged = true;
            }
        }
    }
}

/*!
   Mouse Release Event handler
   \param event Mouse event
 */
void QwtAbstractSlider::mouseReleaseEvent( QMouseEvent* event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( m_data->isScrolling && m_data->isValid )
    {
        m_data->isScrolling = false;

        if ( m_data->pendingValueChanged )
            Q_EMIT valueChanged( m_data->value );

        Q_EMIT sliderReleased();
    }
}

/*!
   Wheel Event handler

   In/decreases the value by s number of steps. The direction
   depends on the invertedControls() property.

   When the control or shift modifier is pressed the wheel delta
   ( divided by 120 ) is mapped to an increment according to
   pageSteps(). Otherwise it is mapped to singleSteps().

   \param event Wheel event
 */
void QwtAbstractSlider::wheelEvent( QWheelEvent* event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !m_data->isValid || m_data->isScrolling )
        return;

#if QT_VERSION < 0x050000
    const int wheelDelta = event->delta();
#else
    const QPoint delta = event->angleDelta();
    const int wheelDelta = ( qAbs( delta.x() ) > qAbs( delta.y() ) )
        ? delta.x() : delta.y();
#endif

    int numSteps = 0;

    if ( ( event->modifiers() & Qt::ControlModifier ) ||
        ( event->modifiers() & Qt::ShiftModifier ) )
    {
        // one page regardless of delta
        numSteps = m_data->pageSteps;
        if ( wheelDelta < 0 )
            numSteps = -numSteps;
    }
    else
    {
        const int numTurns = ( wheelDelta / 120 );
        numSteps = numTurns * m_data->singleSteps;
    }

    if ( m_data->invertedControls )
        numSteps = -numSteps;

    const double value = incrementedValue( m_data->value, numSteps );
    if ( value != m_data->value )
    {
        m_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved( m_data->value );
        Q_EMIT valueChanged( m_data->value );
    }
}

/*!
   Handles key events

   QwtAbstractSlider handles the following keys:

   - Qt::Key_Left\n
    Add/Subtract singleSteps() in direction to lowerBound();
   - Qt::Key_Right\n
    Add/Subtract singleSteps() in direction to upperBound();
   - Qt::Key_Down\n
    Subtract singleSteps(), when invertedControls() is false
   - Qt::Key_Up\n
    Add singleSteps(), when invertedControls() is false
   - Qt::Key_PageDown\n
    Subtract pageSteps(), when invertedControls() is false
   - Qt::Key_PageUp\n
    Add pageSteps(), when invertedControls() is false
   - Qt::Key_Home\n
    Set the value to the minimum()
   - Qt::Key_End\n
    Set the value to the maximum()

   \param event Key event
   \sa isReadOnly()
 */
void QwtAbstractSlider::keyPressEvent( QKeyEvent* event )
{
    if ( isReadOnly() )
    {
        event->ignore();
        return;
    }

    if ( !m_data->isValid || m_data->isScrolling )
        return;

    int numSteps = 0;
    double value = m_data->value;

    switch ( event->key() )
    {
        case Qt::Key_Left:
        {
            numSteps = -static_cast< int >( m_data->singleSteps );
            if ( isInverted() )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_Right:
        {
            numSteps = m_data->singleSteps;
            if ( isInverted() )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_Down:
        {
            numSteps = -static_cast< int >( m_data->singleSteps );
            if ( m_data->invertedControls )
                numSteps = -numSteps;
            break;
        }
        case Qt::Key_Up:
        {
            numSteps = m_data->singleSteps;
            if ( m_data->invertedControls )
                numSteps = -numSteps;

            break;
        }
        case Qt::Key_PageUp:
        {
            numSteps = m_data->pageSteps;
            if ( m_data->invertedControls )
                numSteps = -numSteps;
            break;
        }
        case Qt::Key_PageDown:
        {
            numSteps = -static_cast< int >( m_data->pageSteps );
            if ( m_data->invertedControls )
                numSteps = -numSteps;
            break;
        }
        case Qt::Key_Home:
        {
            value = minimum();
            break;
        }
        case Qt::Key_End:
        {
            value = maximum();
            break;
        }
        default:
        {
            event->ignore();
        }
    }

    if ( numSteps != 0 )
    {
        value = incrementedValue( m_data->value, numSteps );
    }

    if ( value != m_data->value )
    {
        m_data->value = value;
        sliderChange();

        Q_EMIT sliderMoved( m_data->value );
        Q_EMIT valueChanged( m_data->value );
    }
}

/*!
   \brief Set the number of steps

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   The default setting is 100.

   \param stepCount Number of steps

   \sa totalSteps(), setSingleSteps(), setPageSteps()
 */
void QwtAbstractSlider::setTotalSteps( uint stepCount )
{
    m_data->totalSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setTotalSteps(), singleSteps(), pageSteps()
 */
uint QwtAbstractSlider::totalSteps() const
{
    return m_data->totalSteps;
}

/*!
   \brief Set the number of steps for a single increment

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   \param stepCount Number of steps

   \sa singleSteps(), setTotalSteps(), setPageSteps()
 */

void QwtAbstractSlider::setSingleSteps( uint stepCount )
{
    m_data->singleSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setSingleSteps(), totalSteps(), pageSteps()
 */
uint QwtAbstractSlider::singleSteps() const
{
    return m_data->singleSteps;
}

/*!
   \brief Set the number of steps for a page increment

   The range of the slider is divided into a number of steps from
   which the value increments according to user inputs depend.

   \param stepCount Number of steps

   \sa pageSteps(), setTotalSteps(), setSingleSteps()
 */

void QwtAbstractSlider::setPageSteps( uint stepCount )
{
    m_data->pageSteps = stepCount;
}

/*!
   \return Number of steps
   \sa setPageSteps(), totalSteps(), singleSteps()
 */
uint QwtAbstractSlider::pageSteps() const
{
    return m_data->pageSteps;
}

/*!
   \brief Enable step alignment

   When step alignment is enabled values resulting from slider
   movements are aligned to the step size.

   \param on Enable step alignment when true
   \sa stepAlignment()
 */
void QwtAbstractSlider::setStepAlignment( bool on )
{
    if ( on != m_data->stepAlignment )
    {
        m_data->stepAlignment = on;
    }
}

/*!
   \return True, when step alignment is enabled
   \sa setStepAlignment()
 */
bool QwtAbstractSlider::stepAlignment() const
{
    return m_data->stepAlignment;
}

/*!
   Set the slider to the specified value

   \param value New value
   \sa setValid(), sliderChange(), valueChanged()
 */
void QwtAbstractSlider::setValue( double value )
{
    value = qBound( minimum(), value, maximum() );

    const bool changed = ( m_data->value != value ) || !m_data->isValid;

    m_data->value = value;
    m_data->isValid = true;

    if ( changed )
    {
        sliderChange();
        Q_EMIT valueChanged( m_data->value );
    }
}

//! Returns the current value.
double QwtAbstractSlider::value() const
{
    return m_data->value;
}

/*!
   If wrapping is true stepping up from upperBound() value will
   take you to the minimum() value and vice versa.

   \param on En/Disable wrapping
   \sa wrapping()
 */
void QwtAbstractSlider::setWrapping( bool on )
{
    m_data->wrapping = on;
}

/*!
   \return True, when wrapping is set
   \sa setWrapping()
 */
bool QwtAbstractSlider::wrapping() const
{
    return m_data->wrapping;
}

/*!
   Invert wheel and key events

   Usually scrolling the mouse wheel "up" and using keys like page
   up will increase the slider's value towards its maximum.
   When invertedControls() is enabled the value is scrolled
   towards its minimum.

   Inverting the controls might be f.e. useful for a vertical slider
   with an inverted scale ( decreasing from top to bottom ).

   \param on Invert controls, when true

   \sa invertedControls(), keyEvent(), wheelEvent()
 */
void QwtAbstractSlider::setInvertedControls( bool on )
{
    m_data->invertedControls = on;
}

/*!
   \return True, when the controls are inverted
   \sa setInvertedControls()
 */
bool QwtAbstractSlider::invertedControls() const
{
    return m_data->invertedControls;
}

/*!
   Increment the slider

   The step size depends on the number of totalSteps()

   \param stepCount Number of steps
   \sa setTotalSteps(), incrementedValue()
 */
void QwtAbstractSlider::incrementValue( int stepCount )
{
    const double value = incrementedValue(
        m_data->value, stepCount );

    if ( value != m_data->value )
    {
        m_data->value = value;
        sliderChange();
    }
}

/*!
   Increment a value

   \param value Value
   \param stepCount Number of steps

   \return Incremented value
 */
double QwtAbstractSlider::incrementedValue(
    double value, int stepCount ) const
{
    if ( m_data->totalSteps == 0 )
        return value;

    const QwtTransform* transformation =
        scaleMap().transformation();

    if ( transformation == NULL )
    {
        const double range = maximum() - minimum();
        value += stepCount * range / m_data->totalSteps;
    }
    else
    {
        QwtScaleMap map = scaleMap();
        map.setPaintInterval( 0, m_data->totalSteps );

        // we need equidistant steps according to
        // paint device coordinates
        const double range = transformation->transform( maximum() )
            - transformation->transform( minimum() );

        const double stepSize = range / m_data->totalSteps;

        double v = transformation->transform( value );

        v = qRound( v / stepSize ) * stepSize;
        v += stepCount * range / m_data->totalSteps;

        value = transformation->invTransform( v );
    }

    value = boundedValue( value );

    if ( m_data->stepAlignment )
        value = alignedValue( value );

    return value;
}

double QwtAbstractSlider::boundedValue( double value ) const
{
    const double vmin = minimum();
    const double vmax = maximum();

    if ( m_data->wrapping && vmin != vmax )
    {
        if ( qFuzzyCompare( scaleMap().pDist(), 360.0 ) )
        {
            // full circle scales: min and max are the same

            if ( qFuzzyCompare( value, vmax ) )
            {
                value = vmin;
            }
            else
            {
                const double range = vmax - vmin;

                if ( value < vmin )
                {
                    value += std::ceil( ( vmin - value ) / range ) * range;
                }
                else if ( value > vmax )
                {
                    value -= std::ceil( ( value - vmax ) / range ) * range;
                }
            }
        }
        else
        {
            if ( value < vmin )
                value = vmax;
            else if ( value > vmax )
                value = vmin;
        }
    }
    else
    {
        value = qBound( vmin, value, vmax );
    }

    return value;
}

double QwtAbstractSlider::alignedValue( double value ) const
{
    if ( m_data->totalSteps == 0 )
        return value;

    double stepSize;

    if ( scaleMap().transformation() == NULL )
    {
        stepSize = ( maximum() - minimum() ) / m_data->totalSteps;
        if ( stepSize > 0.0 )
        {
            value = lowerBound() +
                qRound( ( value - lowerBound() ) / stepSize ) * stepSize;
        }
    }
    else
    {
        stepSize = ( scaleMap().p2() - scaleMap().p1() ) / m_data->totalSteps;

        if ( stepSize > 0.0 )
        {
            double v = scaleMap().transform( value );

            v = scaleMap().p1() +
                qRound( ( v - scaleMap().p1() ) / stepSize ) * stepSize;

            value = scaleMap().invTransform( v );
        }
    }

    if ( qAbs( stepSize ) > 1e-12 )
    {
        if ( qFuzzyCompare( value + 1.0, 1.0 ) )
        {
            // correct rounding error if value = 0
            value = 0.0;
        }
        else
        {
            // correct rounding error at the border
            if ( qFuzzyCompare( value, upperBound() ) )
                value = upperBound();
            else if ( qFuzzyCompare( value, lowerBound() ) )
                value = lowerBound();
        }
    }

    return value;
}

/*!
   Update the slider according to modifications of the scale
 */
void QwtAbstractSlider::scaleChange()
{
    const double value = qBound( minimum(), m_data->value, maximum() );

    const bool changed = ( value != m_data->value );
    if ( changed )
    {
        m_data->value = value;
    }

    if ( m_data->isValid || changed )
        Q_EMIT valueChanged( m_data->value );

    updateGeometry();
    update();
}

//! Calling update()
void QwtAbstractSlider::sliderChange()
{
    update();
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_abstract_slider.cpp"
#endif
