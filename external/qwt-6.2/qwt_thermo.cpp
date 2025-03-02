/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_thermo.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_math.h"

#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qmargins.h>

#include <algorithm>
#include <functional>

static inline void qwtDrawLine( QPainter* painter, int pos,
    const QColor& color, const QRect& pipeRect, const QRect& liquidRect,
    Qt::Orientation orientation )
{
    painter->setPen( color );
    if ( orientation == Qt::Horizontal )
    {
        if ( pos >= liquidRect.left() && pos < liquidRect.right() )
            painter->drawLine( pos, pipeRect.top(), pos, pipeRect.bottom() );
    }
    else
    {
        if ( pos >= liquidRect.top() && pos < liquidRect.bottom() )
            painter->drawLine( pipeRect.left(), pos, pipeRect.right(), pos );
    }
}

static QVector< double > qwtTickList( const QwtScaleDiv& scaleDiv )
{
    QVector< double > values;

    double lowerLimit = scaleDiv.interval().minValue();
    double upperLimit = scaleDiv.interval().maxValue();

    if ( upperLimit < lowerLimit )
        qSwap( lowerLimit, upperLimit );

    values += lowerLimit;

    for ( int tickType = QwtScaleDiv::MinorTick;
        tickType < QwtScaleDiv::NTickTypes; tickType++ )
    {
        const QList< double > ticks = scaleDiv.ticks( tickType );

        for ( int i = 0; i < ticks.count(); i++ )
        {
            const double v = ticks[i];
            if ( v > lowerLimit && v < upperLimit )
                values += v;
        }
    }

    values += upperLimit;

    return values;
}

class QwtThermo::PrivateData
{
  public:
    PrivateData()
        : orientation( Qt::Vertical )
        , scalePosition( QwtThermo::TrailingScale )
        , spacing( 3 )
        , borderWidth( 2 )
        , pipeWidth( 10 )
        , alarmLevel( 0.0 )
        , alarmEnabled( false )
        , autoFillPipe( true )
        , originMode( QwtThermo::OriginMinimum )
        , origin( 0.0 )
        , colorMap( NULL )
        , value( 0.0 )
    {
        rangeFlags = QwtInterval::IncludeBorders;
    }

    ~PrivateData()
    {
        delete colorMap;
    }

    Qt::Orientation orientation;
    QwtThermo::ScalePosition scalePosition;

    int spacing;
    int borderWidth;
    int pipeWidth;

    QwtInterval::BorderFlags rangeFlags;
    double alarmLevel;
    bool alarmEnabled;
    bool autoFillPipe;
    QwtThermo::OriginMode originMode;
    double origin;

    QwtColorMap* colorMap;

    double value;
};

/*!
   Constructor
   \param parent Parent widget
 */
QwtThermo::QwtThermo( QWidget* parent )
    : QwtAbstractScale( parent )
{
    m_data = new PrivateData;

    QSizePolicy policy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
    if ( m_data->orientation == Qt::Vertical )
        policy.transpose();

    setSizePolicy( policy );

    setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    layoutThermo( true );
}

//! Destructor
QwtThermo::~QwtThermo()
{
    delete m_data;
}

/*!
   \brief Exclude/Include min/max values

   According to the flags minValue() and maxValue()
   are included/excluded from the pipe. In case of an
   excluded value the corresponding tick is painted
   1 pixel off of the pipeRect().

   F.e. when a minimum
   of 0.0 has to be displayed as an empty pipe the minValue()
   needs to be excluded.

   \param flags Range flags
   \sa rangeFlags()
 */
void QwtThermo::setRangeFlags( QwtInterval::BorderFlags flags )
{
    if ( m_data->rangeFlags != flags )
    {
        m_data->rangeFlags = flags;
        update();
    }
}

/*!
   \return Range flags
   \sa setRangeFlags()
 */
QwtInterval::BorderFlags QwtThermo::rangeFlags() const
{
    return m_data->rangeFlags;
}

/*!
   Set the current value.

   \param value New Value
   \sa value()
 */
void QwtThermo::setValue( double value )
{
    if ( m_data->value != value )
    {
        m_data->value = value;
        update();
    }
}

//! Return the value.
double QwtThermo::value() const
{
    return m_data->value;
}

/*!
   \brief Set a scale draw

   For changing the labels of the scales, it
   is necessary to derive from QwtScaleDraw and
   overload QwtScaleDraw::label().

   \param scaleDraw ScaleDraw object, that has to be created with
                   new and will be deleted in ~QwtThermo() or the next
                   call of setScaleDraw().
 */
void QwtThermo::setScaleDraw( QwtScaleDraw* scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
    layoutThermo( true );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
 */
const QwtScaleDraw* QwtThermo::scaleDraw() const
{
    return static_cast< const QwtScaleDraw* >( abstractScaleDraw() );
}

/*!
   \return the scale draw of the thermo
   \sa setScaleDraw()
 */
QwtScaleDraw* QwtThermo::scaleDraw()
{
    return static_cast< QwtScaleDraw* >( abstractScaleDraw() );
}

/*!
   Paint event handler
   \param event Paint event
 */
void QwtThermo::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    const QRect tRect = pipeRect();

    if ( !tRect.contains( event->rect() ) )
    {
        if ( m_data->scalePosition != QwtThermo::NoScale )
            scaleDraw()->draw( &painter, palette() );
    }

    const int bw = m_data->borderWidth;

    const QBrush brush = palette().brush( QPalette::Base );
    qDrawShadePanel( &painter,
        tRect.adjusted( -bw, -bw, bw, bw ),
        palette(), true, bw,
        m_data->autoFillPipe ? &brush : NULL );

    drawLiquid( &painter, tRect );
}

/*!
   Resize event handler
   \param event Resize event
 */
void QwtThermo::resizeEvent( QResizeEvent* event )
{
    Q_UNUSED( event );
    layoutThermo( false );
}

/*!
   Qt change event handler
   \param event Event
 */
void QwtThermo::changeEvent( QEvent* event )
{
    switch( event->type() )
    {
        case QEvent::StyleChange:
        case QEvent::FontChange:
        {
            layoutThermo( true );
            break;
        }
        default:
            break;
    }
}

/*!
   Recalculate the QwtThermo geometry and layout based on
   pipeRect() and the fonts.

   \param update_geometry notify the layout system and call update
         to redraw the scale
 */
void QwtThermo::layoutThermo( bool update_geometry )
{
    const QRect tRect = pipeRect();
    const int bw = m_data->borderWidth + m_data->spacing;
    const bool inverted = ( upperBound() < lowerBound() );

    int from, to;

    if ( m_data->orientation == Qt::Horizontal )
    {
        from = tRect.left();
        to = tRect.right();

        if ( m_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }
        if ( m_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }

        if ( m_data->scalePosition == QwtThermo::TrailingScale )
        {
            scaleDraw()->setAlignment( QwtScaleDraw::TopScale );
            scaleDraw()->move( from, tRect.top() - bw );
        }
        else
        {
            scaleDraw()->setAlignment( QwtScaleDraw::BottomScale );
            scaleDraw()->move( from, tRect.bottom() + bw );
        }

        scaleDraw()->setLength( qMax( to - from, 0 ) );
    }
    else // Qt::Vertical
    {
        from = tRect.top();
        to = tRect.bottom();

        if ( m_data->rangeFlags & QwtInterval::ExcludeMinimum )
        {
            if ( inverted )
                from--;
            else
                to++;
        }
        if ( m_data->rangeFlags & QwtInterval::ExcludeMaximum )
        {
            if ( inverted )
                to++;
            else
                from--;
        }

        if ( m_data->scalePosition == QwtThermo::LeadingScale )
        {
            scaleDraw()->setAlignment( QwtScaleDraw::RightScale );
            scaleDraw()->move( tRect.right() + bw, from );
        }
        else
        {
            scaleDraw()->setAlignment( QwtScaleDraw::LeftScale );
            scaleDraw()->move( tRect.left() - bw, from );
        }

        scaleDraw()->setLength( qMax( to - from, 0 ) );
    }

    if ( update_geometry )
    {
        updateGeometry();
        update();
    }
}

/*!
   \return Bounding rectangle of the pipe ( without borders )
          in widget coordinates
 */
QRect QwtThermo::pipeRect() const
{
    int mbd = 0;
    if ( m_data->scalePosition != QwtThermo::NoScale )
    {
        int d1, d2;
        scaleDraw()->getBorderDistHint( font(), d1, d2 );
        mbd = qMax( d1, d2 );
    }
    const int bw = m_data->borderWidth;
    const int scaleOff = bw + mbd;

    const QRect cr = contentsRect();

    QRect pipeRect = cr;
    if ( m_data->orientation == Qt::Horizontal )
    {
        pipeRect.adjust( scaleOff, 0, -scaleOff, 0 );

        if ( m_data->scalePosition == QwtThermo::TrailingScale )
            pipeRect.setTop( cr.top() + cr.height() - bw - m_data->pipeWidth );
        else
            pipeRect.setTop( bw );

        pipeRect.setHeight( m_data->pipeWidth );
    }
    else // Qt::Vertical
    {
        pipeRect.adjust( 0, scaleOff, 0, -scaleOff );

        if ( m_data->scalePosition == QwtThermo::LeadingScale )
            pipeRect.setLeft( bw );
        else
            pipeRect.setLeft( cr.left() + cr.width() - bw - m_data->pipeWidth );

        pipeRect.setWidth( m_data->pipeWidth );
    }

    return pipeRect;
}

/*!
   \brief Set the orientation.
   \param orientation Allowed values are Qt::Horizontal and Qt::Vertical.

   \sa orientation(), scalePosition()
 */
void QwtThermo::setOrientation( Qt::Orientation orientation )
{
    if ( orientation == m_data->orientation )
        return;

    m_data->orientation = orientation;

    if ( !testAttribute( Qt::WA_WState_OwnSizePolicy ) )
    {
        QSizePolicy sp = sizePolicy();
        sp.transpose();
        setSizePolicy( sp );

        setAttribute( Qt::WA_WState_OwnSizePolicy, false );
    }

    layoutThermo( true );
}

/*!
   \return Orientation
   \sa setOrientation()
 */
Qt::Orientation QwtThermo::orientation() const
{
    return m_data->orientation;
}

/*!
   \brief Change how the origin is determined.
   \sa originMode(), serOrigin(), origin()
 */
void QwtThermo::setOriginMode( OriginMode m )
{
    if ( m == m_data->originMode )
        return;

    m_data->originMode = m;
    update();
}

/*!
   \return Mode, how the origin is determined.
   \sa setOriginMode(), serOrigin(), origin()
 */
QwtThermo::OriginMode QwtThermo::originMode() const
{
    return m_data->originMode;
}

/*!
   \brief Specifies the custom origin.

   If originMode is set to OriginCustom this property controls where the
   liquid starts.

   \param origin New origin level
   \sa setOriginMode(), originMode(), origin()
 */
void QwtThermo::setOrigin( double origin )
{
    if ( origin == m_data->origin )
        return;

    m_data->origin = origin;
    update();
}

/*!
   \return Origin of the thermo, when OriginCustom is enabled
   \sa setOrigin(), setOriginMode(), originMode()
 */
double QwtThermo::origin() const
{
    return m_data->origin;
}

/*!
   \brief Change the position of the scale
   \param scalePosition Position of the scale.

   \sa ScalePosition, scalePosition()
 */
void QwtThermo::setScalePosition( ScalePosition scalePosition )
{
    if ( m_data->scalePosition == scalePosition )
        return;

    m_data->scalePosition = scalePosition;

    if ( testAttribute( Qt::WA_WState_Polished ) )
        layoutThermo( true );
}

/*!
   \return Scale position.
   \sa setScalePosition()
 */
QwtThermo::ScalePosition QwtThermo::scalePosition() const
{
    return m_data->scalePosition;
}

//! Notify a scale change.
void QwtThermo::scaleChange()
{
    layoutThermo( true );
}

/*!
   Redraw the liquid in thermometer pipe.
   \param painter Painter
   \param pipeRect Bounding rectangle of the pipe without borders
 */
void QwtThermo::drawLiquid(
    QPainter* painter, const QRect& pipeRect ) const
{
    painter->save();
    painter->setClipRect( pipeRect, Qt::IntersectClip );
    painter->setPen( Qt::NoPen );

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    QRect liquidRect = fillRect( pipeRect );

    if ( m_data->colorMap != NULL )
    {
        const QwtInterval interval = scaleDiv().interval().normalized();

        // Because the positions of the ticks are rounded
        // we calculate the colors for the rounded tick values

        QVector< double > values = qwtTickList( scaleDraw()->scaleDiv() );

        if ( scaleMap.isInverting() )
            std::sort( values.begin(), values.end(), std::greater< double >() );
        else
            std::sort( values.begin(), values.end(), std::less< double >() );

        int from;
        if ( !values.isEmpty() )
        {
            from = qRound( scaleMap.transform( values[0] ) );
            qwtDrawLine( painter, from,
                m_data->colorMap->color( interval, values[0] ),
                pipeRect, liquidRect, m_data->orientation );
        }

        for ( int i = 1; i < values.size(); i++ )
        {
            const int to = qRound( scaleMap.transform( values[i] ) );

            for ( int pos = from + 1; pos < to; pos++ )
            {
                const double v = scaleMap.invTransform( pos );

                qwtDrawLine( painter, pos,
                    m_data->colorMap->color( interval, v ),
                    pipeRect, liquidRect, m_data->orientation );
            }

            qwtDrawLine( painter, to,
                m_data->colorMap->color( interval, values[i] ),
                pipeRect, liquidRect, m_data->orientation );

            from = to;
        }
    }
    else
    {
        if ( !liquidRect.isEmpty() && m_data->alarmEnabled )
        {
            const QRect r = alarmRect( liquidRect );
            if ( !r.isEmpty() )
            {
                painter->fillRect( r, palette().brush( QPalette::Highlight ) );
                liquidRect = QRegion( liquidRect ).subtracted( r ).boundingRect();
            }
        }

        painter->fillRect( liquidRect, palette().brush( QPalette::ButtonText ) );
    }

    painter->restore();
}

/*!
   \brief Change the spacing between pipe and scale

   A spacing of 0 means, that the backbone of the scale is below
   the pipe.

   The default setting is 3 pixels.

   \param spacing Number of pixels
   \sa spacing();
 */
void QwtThermo::setSpacing( int spacing )
{
    if ( spacing <= 0 )
        spacing = 0;

    if ( spacing != m_data->spacing  )
    {
        m_data->spacing = spacing;
        layoutThermo( true );
    }
}

/*!
   \return Number of pixels between pipe and scale
   \sa setSpacing()
 */
int QwtThermo::spacing() const
{
    return m_data->spacing;
}

/*!
   Set the border width of the pipe.
   \param width Border width
   \sa borderWidth()
 */
void QwtThermo::setBorderWidth( int width )
{
    if ( width <= 0 )
        width = 0;

    if ( width != m_data->borderWidth  )
    {
        m_data->borderWidth = width;
        layoutThermo( true );
    }
}

/*!
   \return Border width of the thermometer pipe.
   \sa setBorderWidth()
 */
int QwtThermo::borderWidth() const
{
    return m_data->borderWidth;
}

/*!
   \brief Assign a color map for the fill color

   \param colorMap Color map
   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
void QwtThermo::setColorMap( QwtColorMap* colorMap )
{
    if ( colorMap != m_data->colorMap )
    {
        delete m_data->colorMap;
        m_data->colorMap = colorMap;
    }
}

/*!
   \return Color map for the fill color
   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
QwtColorMap* QwtThermo::colorMap()
{
    return m_data->colorMap;
}

/*!
   \return Color map for the fill color
   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
const QwtColorMap* QwtThermo::colorMap() const
{
    return m_data->colorMap;
}

/*!
   \brief Change the brush of the liquid.

   Changes the QPalette::ButtonText brush of the palette.

   \param brush New brush.
   \sa fillBrush(), QWidget::setPalette()
 */
void QwtThermo::setFillBrush( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::ButtonText, brush );
    setPalette( pal );
}

/*!
   \return Liquid ( QPalette::ButtonText ) brush.
   \sa setFillBrush(), QWidget::palette()
 */
QBrush QwtThermo::fillBrush() const
{
    return palette().brush( QPalette::ButtonText );
}

/*!
   \brief Specify the liquid brush above the alarm threshold

   Changes the QPalette::Highlight brush of the palette.

   \param brush New brush.
   \sa alarmBrush(), QWidget::setPalette()

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
void QwtThermo::setAlarmBrush( const QBrush& brush )
{
    QPalette pal = palette();
    pal.setBrush( QPalette::Highlight, brush );
    setPalette( pal );
}

/*!
   \return Liquid brush ( QPalette::Highlight ) above the alarm threshold.
   \sa setAlarmBrush(), QWidget::palette()

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
QBrush QwtThermo::alarmBrush() const
{
    return palette().brush( QPalette::Highlight );
}

/*!
   Specify the alarm threshold.

   \param level Alarm threshold
   \sa alarmLevel()

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
void QwtThermo::setAlarmLevel( double level )
{
    m_data->alarmLevel = level;
    m_data->alarmEnabled = 1;
    update();
}

/*!
   \return Alarm threshold.
   \sa setAlarmLevel()

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
double QwtThermo::alarmLevel() const
{
    return m_data->alarmLevel;
}

/*!
   Change the width of the pipe.

   \param width Width of the pipe
   \sa pipeWidth()
 */
void QwtThermo::setPipeWidth( int width )
{
    if ( width > 0 )
    {
        m_data->pipeWidth = width;
        layoutThermo( true );
    }
}

/*!
   \return Width of the pipe.
   \sa setPipeWidth()
 */
int QwtThermo::pipeWidth() const
{
    return m_data->pipeWidth;
}

/*!
   \brief Enable or disable the alarm threshold
   \param on true (disabled) or false (enabled)

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
void QwtThermo::setAlarmEnabled( bool on )
{
    m_data->alarmEnabled = on;
    update();
}

/*!
   \return True, when the alarm threshold is enabled.

   \warning The alarm threshold has no effect, when
           a color map has been assigned
 */
bool QwtThermo::alarmEnabled() const
{
    return m_data->alarmEnabled;
}

/*!
   \return the minimum size hint
   \sa minimumSizeHint()
 */
QSize QwtThermo::sizeHint() const
{
    return minimumSizeHint();
}

/*!
   \return Minimum size hint
   \warning The return value depends on the font and the scale.
   \sa sizeHint()
 */
QSize QwtThermo::minimumSizeHint() const
{
    int w = 0, h = 0;

    if ( m_data->scalePosition != NoScale )
    {
        const int sdExtent = qwtCeil( scaleDraw()->extent( font() ) );
        const int sdLength = scaleDraw()->minLength( font() );

        w = sdLength;
        h = m_data->pipeWidth + sdExtent + m_data->spacing;

    }
    else // no scale
    {
        w = 200;
        h = m_data->pipeWidth;
    }

    if ( m_data->orientation == Qt::Vertical )
        qSwap( w, h );

    w += 2 * m_data->borderWidth;
    h += 2 * m_data->borderWidth;

    // finally add the margins
    const QMargins m = contentsMargins();
    w += m.left() + m.right();
    h += m.top() + m.bottom();

    return QSize( w, h );
}

/*!
   \brief Calculate the filled rectangle of the pipe

   \param pipeRect Rectangle of the pipe
   \return Rectangle to be filled ( fill and alarm brush )

   \sa pipeRect(), alarmRect()
 */
QRect QwtThermo::fillRect( const QRect& pipeRect ) const
{
    double origin;
    if ( m_data->originMode == OriginMinimum )
    {
        origin = qMin( lowerBound(), upperBound() );
    }
    else if ( m_data->originMode == OriginMaximum )
    {
        origin = qMax( lowerBound(), upperBound() );
    }
    else // OriginCustom
    {
        origin = m_data->origin;
    }

    const QwtScaleMap scaleMap = scaleDraw()->scaleMap();

    int from = qRound( scaleMap.transform( m_data->value ) );
    int to = qRound( scaleMap.transform( origin ) );

    if ( to < from )
        qSwap( from, to );

    QRect fillRect = pipeRect;
    if ( m_data->orientation == Qt::Horizontal )
    {
        fillRect.setLeft( from );
        fillRect.setRight( to );
    }
    else // Qt::Vertical
    {
        fillRect.setTop( from );
        fillRect.setBottom( to );
    }

    return fillRect.normalized();
}

/*!
   \brief Calculate the alarm rectangle of the pipe

   \param fillRect Filled rectangle in the pipe
   \return Rectangle to be filled with the alarm brush

   \sa pipeRect(), fillRect(), alarmLevel(), alarmBrush()
 */
QRect QwtThermo::alarmRect( const QRect& fillRect ) const
{
    QRect alarmRect( 0, 0, -1, -1); // something invalid

    if ( !m_data->alarmEnabled )
        return alarmRect;

    const bool inverted = ( upperBound() < lowerBound() );

    bool increasing;
    if ( m_data->originMode == OriginCustom )
    {
        increasing = m_data->value > m_data->origin;
    }
    else
    {
        increasing = m_data->originMode == OriginMinimum;
    }

    const QwtScaleMap map = scaleDraw()->scaleMap();
    const int alarmPos = qRound( map.transform( m_data->alarmLevel ) );
    const int valuePos = qRound( map.transform( m_data->value ) );

    if ( m_data->orientation == Qt::Horizontal )
    {
        int v1, v2;
        if ( inverted )
        {
            v1 = fillRect.left();

            v2 = alarmPos - 1;
            v2 = qMin( v2, increasing ? fillRect.right() : valuePos );
        }
        else
        {
            v1 = alarmPos + 1;
            v1 = qMax( v1, increasing ? fillRect.left() : valuePos );

            v2 = fillRect.right();

        }
        alarmRect.setRect( v1, fillRect.top(), v2 - v1 + 1, fillRect.height() );
    }
    else
    {
        int v1, v2;
        if ( inverted )
        {
            v1 = alarmPos + 1;
            v1 = qMax( v1, increasing ? fillRect.top() : valuePos );

            v2 = fillRect.bottom();
        }
        else
        {
            v1 = fillRect.top();

            v2 = alarmPos - 1;
            v2 = qMin( v2, increasing ? fillRect.bottom() : valuePos );
        }
        alarmRect.setRect( fillRect.left(), v1, fillRect.width(), v2 - v1 + 1 );
    }

    return alarmRect;
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_thermo.cpp"
#endif
