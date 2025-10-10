/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_analog_clock.h"
#include "qwt_dial_needle.h"
#include "qwt_round_scale_draw.h"
#include "qwt_text.h"
#include "qwt_math.h"

#include <qlocale.h>
#include <qdatetime.h>

namespace
{
    class QwtAnalogClockScaleDraw QWT_FINAL : public QwtRoundScaleDraw
    {
      public:
        QwtAnalogClockScaleDraw()
        {
            setSpacing( 8 );

            enableComponent( QwtAbstractScaleDraw::Backbone, false );

            setTickLength( QwtScaleDiv::MinorTick, 2 );
            setTickLength( QwtScaleDiv::MediumTick, 4 );
            setTickLength( QwtScaleDiv::MajorTick, 8 );

            setPenWidthF( 1.0 );
        }

        virtual QwtText label( double value ) const QWT_OVERRIDE
        {
            if ( qFuzzyCompare( value + 1.0, 1.0 ) )
                value = 60.0 * 60.0 * 12.0;

            return QLocale().toString( qRound( value / ( 60.0 * 60.0 ) ) );
        }
    };
}

/*!
   Constructor
   \param parent Parent widget
 */
QwtAnalogClock::QwtAnalogClock( QWidget* parent )
    : QwtDial( parent )
{
    setWrapping( true );
    setReadOnly( true );

    setOrigin( 270.0 );
    setScaleDraw( new QwtAnalogClockScaleDraw() );

    setTotalSteps( 60 );

    const int secondsPerHour = 60.0 * 60.0;

    QList< double > majorTicks;
    QList< double > minorTicks;

    for ( int i = 0; i < 12; i++ )
    {
        majorTicks += i * secondsPerHour;

        for ( int j = 1; j < 5; j++ )
            minorTicks += i * secondsPerHour + j * secondsPerHour / 5.0;
    }

    QwtScaleDiv scaleDiv;
    scaleDiv.setInterval( 0.0, 12.0 * secondsPerHour );
    scaleDiv.setTicks( QwtScaleDiv::MajorTick, majorTicks );
    scaleDiv.setTicks( QwtScaleDiv::MinorTick, minorTicks );
    setScale( scaleDiv );

    QColor knobColor = palette().color( QPalette::Active, QPalette::Text );
    knobColor = knobColor.darker( 120 );

    QColor handColor;
    int width;

    for ( int i = 0; i < NHands; i++ )
    {
        if ( i == SecondHand )
        {
            width = 2;
            handColor = knobColor.darker( 120 );
        }
        else
        {
            width = 8;
            handColor = knobColor;
        }

        QwtDialSimpleNeedle* hand = new QwtDialSimpleNeedle(
            QwtDialSimpleNeedle::Arrow, true, handColor, knobColor );
        hand->setWidth( width );

        m_hand[i] = NULL;
        setHand( static_cast< Hand >( i ), hand );
    }
}

//! Destructor
QwtAnalogClock::~QwtAnalogClock()
{
    for ( int i = 0; i < NHands; i++ )
        delete m_hand[i];
}

/*!
   Nop method, use setHand() instead
   \sa setHand()
 */
void QwtAnalogClock::setNeedle( QwtDialNeedle* )
{
    // no op
    return;
}

/*!
   Set a clock hand
   \param hand Specifies the type of hand
   \param needle Hand
   \sa hand()
 */
void QwtAnalogClock::setHand( Hand hand, QwtDialNeedle* needle )
{
    if ( hand >= 0 && hand < NHands )
    {
        delete m_hand[hand];
        m_hand[hand] = needle;
    }
}

/*!
   \return Clock hand
   \param hd Specifies the type of hand
   \sa setHand()
 */
QwtDialNeedle* QwtAnalogClock::hand( Hand hd )
{
    if ( hd < 0 || hd >= NHands )
        return NULL;

    return m_hand[hd];
}

/*!
   \return Clock hand
   \param hd Specifies the type of hand
   \sa setHand()
 */
const QwtDialNeedle* QwtAnalogClock::hand( Hand hd ) const
{
    return const_cast< QwtAnalogClock* >( this )->hand( hd );
}

/*!
   \brief Set the current time
 */
void QwtAnalogClock::setCurrentTime()
{
    setTime( QTime::currentTime() );
}

/*!
   Set a time
   \param time Time to display
 */
void QwtAnalogClock::setTime( const QTime& time )
{
    if ( time.isValid() )
    {
        setValue( ( time.hour() % 12 ) * 60.0 * 60.0
            + time.minute() * 60.0 + time.second() );
    }
    else
        setValid( false );
}

/*!
   \brief Draw the needle

   A clock has no single needle but three hands instead. drawNeedle()
   translates value() into directions for the hands and calls
   drawHand().

   \param painter Painter
   \param center Center of the clock
   \param radius Maximum length for the hands
   \param direction Dummy, not used.
   \param colorGroup ColorGroup

   \sa drawHand()
 */
void QwtAnalogClock::drawNeedle( QPainter* painter, const QPointF& center,
    double radius, double direction, QPalette::ColorGroup colorGroup ) const
{
    Q_UNUSED( direction );

    if ( isValid() )
    {
        const double hours = value() / ( 60.0 * 60.0 );
        const double minutes =
            ( value() - std::floor(hours) * 60.0 * 60.0 ) / 60.0;
        const double seconds = value() - std::floor(hours) * 60.0 * 60.0
            - std::floor(minutes) * 60.0;

        double angle[NHands];
        angle[HourHand] = 360.0 * hours / 12.0;
        angle[MinuteHand] = 360.0 * minutes / 60.0;
        angle[SecondHand] = 360.0 * seconds / 60.0;

        for ( int hand = 0; hand < NHands; hand++ )
        {
            const double d = 360.0 - angle[hand] - origin();
            drawHand( painter, static_cast< Hand >( hand ),
                center, radius, d, colorGroup );
        }
    }
}

/*!
   Draw a clock hand

   \param painter Painter
   \param hd Specify the type of hand
   \param center Center of the clock
   \param radius Maximum length for the hands
   \param direction Direction of the hand in degrees, counter clockwise
   \param cg ColorGroup
 */
void QwtAnalogClock::drawHand( QPainter* painter, Hand hd,
    const QPointF& center, double radius, double direction,
    QPalette::ColorGroup cg ) const
{
    const QwtDialNeedle* needle = hand( hd );
    if ( needle )
    {
        if ( hd == HourHand )
            radius = qRound( 0.8 * radius );

        needle->draw( painter, center, radius, direction, cg );
    }
}

#include "moc_qwt_analog_clock.cpp"
