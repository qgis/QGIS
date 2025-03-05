/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_arrow_button.h"
#include "qwt_counter.h"
#include "qwt_painter.h"
#include "qwt_math.h"

#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qevent.h>
#include <qstyle.h>

class QwtCounter::PrivateData
{
  public:
    PrivateData()
        : minimum( 0.0 )
        , maximum( 0.0 )
        , singleStep( 1.0 )
        , isValid( false )
        , value( 0.0 )
        , wrapping( false )
    {
        increment[Button1] = 1;
        increment[Button2] = 10;
        increment[Button3] = 100;
    }

    QwtArrowButton* buttonDown[ButtonCnt];
    QwtArrowButton* buttonUp[ButtonCnt];
    QLineEdit* valueEdit;

    int increment[ButtonCnt];
    int numButtons;

    double minimum;
    double maximum;
    double singleStep;

    bool isValid;
    double value;

    bool wrapping;
};

/*!
   The counter is initialized with a range is set to [0.0, 1.0] with
   0.01 as single step size. The value is invalid.

   The default number of buttons is set to 2. The default increments are:
   \li Button 1: 1 step
   \li Button 2: 10 steps
   \li Button 3: 100 steps

   \param parent
 */
QwtCounter::QwtCounter( QWidget* parent )
    : QWidget( parent )
{
    initCounter();
}

void QwtCounter::initCounter()
{
    m_data = new PrivateData;

    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setContentsMargins( QMargins() );

    for ( int i = ButtonCnt - 1; i >= 0; i-- )
    {
        QwtArrowButton* btn =
            new QwtArrowButton( i + 1, Qt::DownArrow, this );
        btn->setFocusPolicy( Qt::NoFocus );
        layout->addWidget( btn );

        connect( btn, SIGNAL(released()), SLOT(btnReleased()) );
        connect( btn, SIGNAL(clicked()), SLOT(btnClicked()) );

        m_data->buttonDown[i] = btn;
    }

    m_data->valueEdit = new QLineEdit( this );
    m_data->valueEdit->setReadOnly( false );
    m_data->valueEdit->setValidator( new QDoubleValidator( m_data->valueEdit ) );
    layout->addWidget( m_data->valueEdit );

    connect( m_data->valueEdit, SIGNAL(editingFinished()), SLOT(textChanged()) );

    layout->setStretchFactor( m_data->valueEdit, 10 );

    for ( int i = 0; i < ButtonCnt; i++ )
    {
        QwtArrowButton* btn =
            new QwtArrowButton( i + 1, Qt::UpArrow, this );
        btn->setFocusPolicy( Qt::NoFocus );
        layout->addWidget( btn );

        connect( btn, SIGNAL(released()), SLOT(btnReleased()) );
        connect( btn, SIGNAL(clicked()), SLOT(btnClicked()) );

        m_data->buttonUp[i] = btn;
    }

    setNumButtons( 2 );
    setRange( 0.0, 1.0 );
    setSingleStep( 0.001 );
    setValue( 0.0 );

    setSizePolicy(
        QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );

    setFocusProxy( m_data->valueEdit );
    setFocusPolicy( Qt::StrongFocus );
}

//! Destructor
QwtCounter::~QwtCounter()
{
    delete m_data;
}

/*!
   Set the counter to be in valid/invalid state

   When the counter is set to invalid, no numbers are displayed and
   the buttons are disabled.

   \param on If true the counter will be set as valid

   \sa setValue(), isValid()
 */
void QwtCounter::setValid( bool on )
{
    if ( on != m_data->isValid )
    {
        m_data->isValid = on;

        updateButtons();

        if ( m_data->isValid )
        {
            showNumber( value() );
            Q_EMIT valueChanged( value() );
        }
        else
        {
            m_data->valueEdit->setText( QString() );
        }
    }
}

/*!
   \return True, if the value is valid
   \sa setValid(), setValue()
 */
bool QwtCounter::isValid() const
{
    return m_data->isValid;
}

/*!
   \brief Allow/disallow the user to manually edit the value

   \param on True disable editing
   \sa isReadOnly()
 */
void QwtCounter::setReadOnly( bool on )
{
    m_data->valueEdit->setReadOnly( on );
}

/*!
   \return True, when the line line edit is read only. (default is no)
   \sa setReadOnly()
 */
bool QwtCounter::isReadOnly() const
{
    return m_data->valueEdit->isReadOnly();
}

/*!
   \brief Set a new value without adjusting to the step raster

   The state of the counter is set to be valid.

   \param value New value

   \sa isValid(), value(), valueChanged()
   \warning The value is clipped when it lies outside the range.
 */

void QwtCounter::setValue( double value )
{
    const double vmin = qwtMinF( m_data->minimum, m_data->maximum );
    const double vmax = qwtMaxF( m_data->minimum, m_data->maximum );

    value = qBound( vmin, value, vmax );

    if ( !m_data->isValid || value != m_data->value )
    {
        m_data->isValid = true;
        m_data->value = value;

        showNumber( value );
        updateButtons();

        Q_EMIT valueChanged( value );
    }
}

/*!
   \return Current value of the counter
   \sa setValue(), valueChanged()
 */
double QwtCounter::value() const
{
    return m_data->value;
}

/*!
   \brief Set the minimum and maximum values

   The maximum is adjusted if necessary to ensure that the range remains valid.
   The value might be modified to be inside of the range.

   \param min Minimum value
   \param max Maximum value

   \sa minimum(), maximum()
 */
void QwtCounter::setRange( double min, double max )
{
    max = qwtMaxF( min, max );

    if ( m_data->maximum == max && m_data->minimum == min )
        return;

    m_data->minimum = min;
    m_data->maximum = max;

    setSingleStep( singleStep() );

    const double value = qBound( min, m_data->value, max );

    if ( value != m_data->value )
    {
        m_data->value = value;

        if ( m_data->isValid )
        {
            showNumber( value );
            Q_EMIT valueChanged( value );
        }
    }

    updateButtons();
}

/*!
   Set the minimum value of the range

   \param value Minimum value
   \sa setRange(), setMaximum(), minimum()

   \note The maximum is adjusted if necessary to ensure that the range remains valid.
 */
void QwtCounter::setMinimum( double value )
{
    setRange( value, maximum() );
}

/*!
   \return The minimum of the range
   \sa setRange(), setMinimum(), maximum()
 */
double QwtCounter::minimum() const
{
    return m_data->minimum;
}

/*!
   Set the maximum value of the range

   \param value Maximum value
   \sa setRange(), setMinimum(), maximum()
 */
void QwtCounter::setMaximum( double value )
{
    setRange( minimum(), value );
}

/*!
   \return The maximum of the range
   \sa setRange(), setMaximum(), minimum()
 */
double QwtCounter::maximum() const
{
    return m_data->maximum;
}

/*!
   \brief Set the step size of the counter

   A value <= 0.0 disables stepping

   \param stepSize Single step size
   \sa singleStep()
 */
void QwtCounter::setSingleStep( double stepSize )
{
    m_data->singleStep = qwtMaxF( stepSize, 0.0 );
}

/*!
   \return Single step size
   \sa setSingleStep()
 */
double QwtCounter::singleStep() const
{
    return m_data->singleStep;
}

/*!
   \brief En/Disable wrapping

   If wrapping is true stepping up from maximum() value will take
   you to the minimum() value and vice versa.

   \param on En/Disable wrapping
   \sa wrapping()
 */
void QwtCounter::setWrapping( bool on )
{
    m_data->wrapping = on;
}

/*!
   \return True, when wrapping is set
   \sa setWrapping()
 */
bool QwtCounter::wrapping() const
{
    return m_data->wrapping;
}

/*!
   Specify the number of buttons on each side of the label

   \param numButtons Number of buttons
   \sa numButtons()
 */
void QwtCounter::setNumButtons( int numButtons )
{
    if ( numButtons < 0 || numButtons > QwtCounter::ButtonCnt )
        return;

    for ( int i = 0; i < QwtCounter::ButtonCnt; i++ )
    {
        if ( i < numButtons )
        {
            m_data->buttonDown[i]->show();
            m_data->buttonUp[i]->show();
        }
        else
        {
            m_data->buttonDown[i]->hide();
            m_data->buttonUp[i]->hide();
        }
    }

    m_data->numButtons = numButtons;
}

/*!
   \return The number of buttons on each side of the widget.
   \sa setNumButtons()
 */
int QwtCounter::numButtons() const
{
    return m_data->numButtons;
}

/*!
   Specify the number of steps by which the value
   is incremented or decremented when a specified button
   is pushed.

   \param button Button index
   \param numSteps Number of steps

   \sa incSteps()
 */
void QwtCounter::setIncSteps( QwtCounter::Button button, int numSteps )
{
    if ( button >= 0 && button < QwtCounter::ButtonCnt )
        m_data->increment[ button ] = numSteps;
}

/*!
   \return The number of steps by which a specified button increments the value
          or 0 if the button is invalid.
   \param button Button index

   \sa setIncSteps()
 */
int QwtCounter::incSteps( QwtCounter::Button button ) const
{
    if ( button >= 0 && button < QwtCounter::ButtonCnt )
        return m_data->increment[ button ];

    return 0;
}


/*!
   Set the number of increment steps for button 1
   \param nSteps Number of steps
 */
void QwtCounter::setStepButton1( int nSteps )
{
    setIncSteps( QwtCounter::Button1, nSteps );
}

//! returns the number of increment steps for button 1
int QwtCounter::stepButton1() const
{
    return incSteps( QwtCounter::Button1 );
}

/*!
   Set the number of increment steps for button 2
   \param nSteps Number of steps
 */
void QwtCounter::setStepButton2( int nSteps )
{
    setIncSteps( QwtCounter::Button2, nSteps );
}

//! returns the number of increment steps for button 2
int QwtCounter::stepButton2() const
{
    return incSteps( QwtCounter::Button2 );
}

/*!
   Set the number of increment steps for button 3
   \param nSteps Number of steps
 */
void QwtCounter::setStepButton3( int nSteps )
{
    setIncSteps( QwtCounter::Button3, nSteps );
}

//! returns the number of increment steps for button 3
int QwtCounter::stepButton3() const
{
    return incSteps( QwtCounter::Button3 );
}

//! Set from lineedit
void QwtCounter::textChanged()
{
    bool converted = false;

    const double value = m_data->valueEdit->text().toDouble( &converted );
    if ( converted )
        setValue( value );
}

/*!
   Handle QEvent::PolishRequest events
   \param event Event
   \return see QWidget::event()
 */
bool QwtCounter::event( QEvent* event )
{
    if ( event->type() == QEvent::PolishRequest )
    {
        const QFontMetrics fm = m_data->valueEdit->fontMetrics();

        const int w = QwtPainter::horizontalAdvance( fm, "W" ) + 8;
        for ( int i = 0; i < ButtonCnt; i++ )
        {
            m_data->buttonDown[i]->setMinimumWidth( w );
            m_data->buttonUp[i]->setMinimumWidth( w );
        }
    }

    return QWidget::event( event );
}

/*!
   Handle key events

   - Ctrl + Qt::Key_Home\n
    Step to minimum()
   - Ctrl + Qt::Key_End\n
    Step to maximum()
   - Qt::Key_Up\n
    Increment by incSteps(QwtCounter::Button1)
   - Qt::Key_Down\n
    Decrement by incSteps(QwtCounter::Button1)
   - Qt::Key_PageUp\n
    Increment by incSteps(QwtCounter::Button2)
   - Qt::Key_PageDown\n
    Decrement by incSteps(QwtCounter::Button2)
   - Shift + Qt::Key_PageUp\n
    Increment by incSteps(QwtCounter::Button3)
   - Shift + Qt::Key_PageDown\n
    Decrement by incSteps(QwtCounter::Button3)

   \param event Key event
 */
void QwtCounter::keyPressEvent ( QKeyEvent* event )
{
    bool accepted = true;

    switch ( event->key() )
    {
        case Qt::Key_Home:
        {
            if ( event->modifiers() & Qt::ControlModifier )
                setValue( minimum() );
            else
                accepted = false;
            break;
        }
        case Qt::Key_End:
        {
            if ( event->modifiers() & Qt::ControlModifier )
                setValue( maximum() );
            else
                accepted = false;
            break;
        }
        case Qt::Key_Up:
        {
            incrementValue( m_data->increment[0] );
            break;
        }
        case Qt::Key_Down:
        {
            incrementValue( -m_data->increment[0] );
            break;
        }
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        {
            int increment = m_data->increment[0];
            if ( m_data->numButtons >= 2 )
                increment = m_data->increment[1];
            if ( m_data->numButtons >= 3 )
            {
                if ( event->modifiers() & Qt::ShiftModifier )
                    increment = m_data->increment[2];
            }
            if ( event->key() == Qt::Key_PageDown )
                increment = -increment;
            incrementValue( increment );
            break;
        }
        default:
        {
            accepted = false;
        }
    }

    if ( accepted )
    {
        event->accept();
        return;
    }

    QWidget::keyPressEvent ( event );
}

/*!
   Handle wheel events
   \param event Wheel event
 */
void QwtCounter::wheelEvent( QWheelEvent* event )
{
    event->accept();

    if ( m_data->numButtons <= 0 )
        return;

    int increment = m_data->increment[0];
    if ( m_data->numButtons >= 2 )
    {
        if ( event->modifiers() & Qt::ControlModifier )
            increment = m_data->increment[1];
    }
    if ( m_data->numButtons >= 3 )
    {
        if ( event->modifiers() & Qt::ShiftModifier )
            increment = m_data->increment[2];
    }

#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
    const int wheelDelta = event->delta();
#else
    const QPoint wheelPos = event->position().toPoint();

    const QPoint delta = event->angleDelta();
    const int wheelDelta = ( qAbs( delta.x() ) > qAbs( delta.y() ) )
        ? delta.x() : delta.y();
#endif

    for ( int i = 0; i < m_data->numButtons; i++ )
    {
        if ( m_data->buttonDown[i]->geometry().contains( wheelPos ) ||
            m_data->buttonUp[i]->geometry().contains( wheelPos ) )
        {
            increment = m_data->increment[i];
        }
    }

    incrementValue( wheelDelta / 120 * increment );
}

void QwtCounter::incrementValue( int numSteps )
{
    const double min = m_data->minimum;
    const double max = m_data->maximum;
    double stepSize = m_data->singleStep;

    if ( !m_data->isValid || min >= max || stepSize <= 0.0 )
        return;


#if 1
    stepSize = qwtMaxF( stepSize, 1.0e-10 * ( max - min ) );
#endif

    double value = m_data->value + numSteps * stepSize;

    if ( m_data->wrapping )
    {
        const double range = max - min;

        if ( value < min )
        {
            value += std::ceil( ( min - value ) / range ) * range;
        }
        else if ( value > max )
        {
            value -= std::ceil( ( value - max ) / range ) * range;
        }
    }
    else
    {
        value = qBound( min, value, max );
    }

    value = min + qRound( ( value - min ) / stepSize ) * stepSize;

    if ( stepSize > 1e-12 )
    {
        if ( qFuzzyCompare( value + 1.0, 1.0 ) )
        {
            // correct rounding error if value = 0
            value = 0.0;
        }
        else if ( qFuzzyCompare( value, max ) )
        {
            // correct rounding error at the border
            value = max;
        }
    }

    if ( value != m_data->value )
    {
        m_data->value = value;
        showNumber( m_data->value );
        updateButtons();

        Q_EMIT valueChanged( m_data->value );
    }
}


/*!
   \brief Update buttons according to the current value

   When the QwtCounter under- or over-flows, the focus is set to the smallest
   up- or down-button and counting is disabled.

   Counting is re-enabled on a button release event (mouse or space bar).
 */
void QwtCounter::updateButtons()
{
    if ( m_data->isValid )
    {
        // 1. save enabled state of the smallest down- and up-button
        // 2. change enabled state on under- or over-flow

        for ( int i = 0; i < QwtCounter::ButtonCnt; i++ )
        {
            m_data->buttonDown[i]->setEnabled( value() > minimum() );
            m_data->buttonUp[i]->setEnabled( value() < maximum() );
        }
    }
    else
    {
        for ( int i = 0; i < QwtCounter::ButtonCnt; i++ )
        {
            m_data->buttonDown[i]->setEnabled( false );
            m_data->buttonUp[i]->setEnabled( false );
        }
    }
}
/*!
   Display number string

   \param number Number
 */
void QwtCounter::showNumber( double number )
{
    QString text;
    text.setNum( number );

    const int cursorPos = m_data->valueEdit->cursorPosition();
    m_data->valueEdit->setText( text );
    m_data->valueEdit->setCursorPosition( cursorPos );
}

//!  Button clicked
void QwtCounter::btnClicked()
{
    for ( int i = 0; i < ButtonCnt; i++ )
    {
        if ( m_data->buttonUp[i] == sender() )
            incrementValue( m_data->increment[i] );

        if ( m_data->buttonDown[i] == sender() )
            incrementValue( -m_data->increment[i] );
    }
}

//!  Button released
void QwtCounter::btnReleased()
{
    Q_EMIT buttonReleased( value() );
}

//! A size hint
QSize QwtCounter::sizeHint() const
{
    QString tmp;

    int w = tmp.setNum( minimum() ).length();
    int w1 = tmp.setNum( maximum() ).length();
    if ( w1 > w )
        w = w1;
    w1 = tmp.setNum( minimum() + singleStep() ).length();
    if ( w1 > w )
        w = w1;
    w1 = tmp.setNum( maximum() - singleStep() ).length();
    if ( w1 > w )
        w = w1;

    tmp.fill( '9', w );

    w = QwtPainter::horizontalAdvance( m_data->valueEdit->fontMetrics(), tmp ) + 2;

    if ( m_data->valueEdit->hasFrame() )
        w += 2 * style()->pixelMetric( QStyle::PM_DefaultFrameWidth );

    // Now we replace default sizeHint contribution of m_data->valueEdit by
    // what we really need.

    w += QWidget::sizeHint().width() - m_data->valueEdit->sizeHint().width();

    const int h = qMin( QWidget::sizeHint().height(),
        m_data->valueEdit->minimumSizeHint().height() );

    return QSize( w, h );
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_counter.cpp"
#endif
