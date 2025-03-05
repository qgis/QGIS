/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_dial.h"
#include "qwt_dial_needle.h"
#include "qwt_math.h"
#include "qwt_scale_map.h"
#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt.h"

#include <qpainter.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qstyle.h>
#include <qstyleoption.h>

static inline double qwtAngleDist( double a1, double a2 )
{
    double dist = qAbs( a2 - a1 );
    if ( dist > 360.0 )
        dist -= 360.0;

    return dist;
}

static inline bool qwtIsOnArc( double angle, double min, double max )
{
    if ( min < max )
    {
        return ( angle >= min ) && ( angle <= max );
    }
    else
    {
        return ( angle >= min ) || ( angle <= max );
    }
}

static inline double qwtBoundedAngle( double min, double angle, double max )
{
    double from = qwtNormalizeDegrees( min );
    double to = qwtNormalizeDegrees( max );

    double a;

    if ( qwtIsOnArc( angle, from, to ) )
    {
        a = angle;
        if ( a < min )
            a += 360.0;
    }
    else
    {
        if ( qwtAngleDist( angle, from ) <
            qwtAngleDist( angle, to ) )
        {
            a = min;
        }
        else
        {
            a = max;
        }
    }

    return a;
}

class QwtDial::PrivateData
{
  public:
    PrivateData()
        : frameShadow( Sunken )
        , lineWidth( 0 )
        , mode( RotateNeedle )
        , origin( 90.0 )
        , minScaleArc( 0.0 )
        , maxScaleArc( 0.0 )
        , needle( NULL )
        , arcOffset( 0.0 )
        , mouseOffset( 0.0 )
    {
    }

    ~PrivateData()
    {
        delete needle;
    }
    Shadow frameShadow;
    int lineWidth;

    QwtDial::Mode mode;

    double origin;
    double minScaleArc;
    double maxScaleArc;

    QwtDialNeedle* needle;

    double arcOffset;
    double mouseOffset;

    QPixmap pixmapCache;
};

/*!
   \brief Constructor
   \param parent Parent widget

   Create a dial widget with no needle. The scale is initialized
   to [ 0.0, 360.0 ] and 360 steps ( QwtAbstractSlider::setTotalSteps() ).
   The origin of the scale is at 90°,

   The value is set to 0.0.

   The default mode is QwtDial::RotateNeedle.
 */
QwtDial::QwtDial( QWidget* parent )
    : QwtAbstractSlider( parent )
{
    m_data = new PrivateData;

    setFocusPolicy( Qt::TabFocus );

    QPalette p = palette();
    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        const QPalette::ColorGroup colorGroup =
            static_cast< QPalette::ColorGroup >( i );

        // Base: background color of the circle inside the frame.
        // WindowText: background color of the circle inside the scale

        p.setColor( colorGroup, QPalette::WindowText,
            p.color( colorGroup, QPalette::Base ) );
    }
    setPalette( p );

    QwtRoundScaleDraw* scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->setRadius( 0 );

    setScaleDraw( scaleDraw );

    setScaleArc( 0.0, 360.0 ); // scale as a full circle

    setScaleMaxMajor( 10 );
    setScaleMaxMinor( 5 );

    setValue( 0.0 );
}

//!  Destructor
QwtDial::~QwtDial()
{
    delete m_data;
}

/*!
   Sets the frame shadow value from the frame style.

   \param shadow Frame shadow
   \sa setLineWidth(), QFrame::setFrameShadow()
 */
void QwtDial::setFrameShadow( Shadow shadow )
{
    if ( shadow != m_data->frameShadow )
    {
        invalidateCache();

        m_data->frameShadow = shadow;
        if ( lineWidth() > 0 )
            update();
    }
}

/*!
   \return Frame shadow
   /sa setFrameShadow(), lineWidth(), QFrame::frameShadow()
 */
QwtDial::Shadow QwtDial::frameShadow() const
{
    return m_data->frameShadow;
}

/*!
   Sets the line width of the frame

   \param lineWidth Line width
   \sa setFrameShadow()
 */
void QwtDial::setLineWidth( int lineWidth )
{
    if ( lineWidth < 0 )
        lineWidth = 0;

    if ( m_data->lineWidth != lineWidth )
    {
        invalidateCache();

        m_data->lineWidth = lineWidth;
        update();
    }
}

/*!
   \return Line width of the frame
   \sa setLineWidth(), frameShadow(), lineWidth()
 */
int QwtDial::lineWidth() const
{
    return m_data->lineWidth;
}

/*!
   \return bounding rectangle of the circle inside the frame
   \sa setLineWidth(), scaleInnerRect(), boundingRect()
 */
QRect QwtDial::innerRect() const
{
    const int lw = lineWidth();
    return boundingRect().adjusted( lw, lw, -lw, -lw );
}

/*!
   \return bounding rectangle of the dial including the frame
   \sa setLineWidth(), scaleInnerRect(), innerRect()
 */
QRect QwtDial::boundingRect() const
{
    const QRect cr = contentsRect();

    const int dim = qMin( cr.width(), cr.height() );

    QRect inner( 0, 0, dim, dim );
    inner.moveCenter( cr.center() );

    return inner;
}

/*!
   \return rectangle inside the scale
   \sa setLineWidth(), boundingRect(), innerRect()
 */
QRect QwtDial::scaleInnerRect() const
{
    QRect rect = innerRect();

    const QwtAbstractScaleDraw* sd = scaleDraw();
    if ( sd )
    {
        int scaleDist = qwtCeil( sd->extent( font() ) );
        scaleDist++; // margin

        rect.adjust( scaleDist, scaleDist, -scaleDist, -scaleDist );
    }

    return rect;
}

/*!
   \brief Change the mode of the dial.
   \param mode New mode

   In case of QwtDial::RotateNeedle the needle is rotating, in case of
   QwtDial::RotateScale, the needle points to origin()
   and the scale is rotating.

   The default mode is QwtDial::RotateNeedle.

   \sa mode(), setValue(), setOrigin()
 */
void QwtDial::setMode( Mode mode )
{
    if ( mode != m_data->mode )
    {
        invalidateCache();

        m_data->mode = mode;
        sliderChange();
    }
}

/*!
   \return Mode of the dial.
   \sa setMode(), origin(), setScaleArc(), value()
 */
QwtDial::Mode QwtDial::mode() const
{
    return m_data->mode;
}

/*!
   Invalidate the internal caches used to speed up repainting
 */
void QwtDial::invalidateCache()
{
    m_data->pixmapCache = QPixmap();
}

/*!
   Paint the dial
   \param event Paint event
 */
void QwtDial::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if ( m_data->mode == QwtDial::RotateScale )
    {
        painter.save();
        painter.setRenderHint( QPainter::Antialiasing, true );

        drawContents( &painter );

        painter.restore();
    }

    const QRect r = contentsRect();
    if ( r.size() != m_data->pixmapCache.size() )
    {
        m_data->pixmapCache = QwtPainter::backingStore( this, r.size() );
        m_data->pixmapCache.fill( Qt::transparent );

        QPainter p( &m_data->pixmapCache );
        p.setRenderHint( QPainter::Antialiasing, true );
        p.translate( -r.topLeft() );

        if ( m_data->mode != QwtDial::RotateScale )
            drawContents( &p );

        if ( lineWidth() > 0 )
            drawFrame( &p );

        if ( m_data->mode != QwtDial::RotateNeedle )
            drawNeedle( &p );
    }

    painter.drawPixmap( r.topLeft(), m_data->pixmapCache );

    if ( m_data->mode == QwtDial::RotateNeedle )
        drawNeedle( &painter );

    if ( hasFocus() )
        drawFocusIndicator( &painter );
}

/*!
   Draw the focus indicator
   \param painter Painter
 */
void QwtDial::drawFocusIndicator( QPainter* painter ) const
{
    QwtPainter::drawFocusRect( painter, this, boundingRect() );
}

/*!
   Draw the frame around the dial

   \param painter Painter
   \sa lineWidth(), frameShadow()
 */
void QwtDial::drawFrame( QPainter* painter )
{
    QwtPainter::drawRoundFrame( painter, boundingRect(),
        palette(), lineWidth(), m_data->frameShadow );
}

/*!
   \brief Draw the contents inside the frame

   QPalette::Window is the background color outside of the frame.
   QPalette::Base is the background color inside the frame.
   QPalette::WindowText is the background color inside the scale.

   \param painter Painter

   \sa boundingRect(), innerRect(),
    scaleInnerRect(), QWidget::setPalette()
 */
void QwtDial::drawContents( QPainter* painter ) const
{
    if ( testAttribute( Qt::WA_NoSystemBackground ) ||
        palette().brush( QPalette::Base ) !=
        palette().brush( QPalette::Window ) )
    {
        const QRectF br = boundingRect();

        painter->save();
        painter->setPen( Qt::NoPen );
        painter->setBrush( palette().brush( QPalette::Base ) );
        painter->drawEllipse( br );
        painter->restore();
    }

    const QRectF insideScaleRect = scaleInnerRect();
    if ( palette().brush( QPalette::WindowText ) !=
        palette().brush( QPalette::Base ) )
    {
        painter->save();
        painter->setPen( Qt::NoPen );
        painter->setBrush( palette().brush( QPalette::WindowText ) );
        painter->drawEllipse( insideScaleRect );
        painter->restore();
    }

    const QPointF center = insideScaleRect.center();
    const double radius = 0.5 * insideScaleRect.width();

    painter->save();
    drawScale( painter, center, radius );
    painter->restore();

    painter->save();
    drawScaleContents( painter, center, radius );
    painter->restore();
}

/*!
   Draw the needle

   \param painter Painter
   \param center Center of the dial
   \param radius Length for the needle
   \param direction Direction of the needle in degrees, counter clockwise
   \param colorGroup ColorGroup
 */
void QwtDial::drawNeedle( QPainter* painter, const QPointF& center,
    double radius, double direction, QPalette::ColorGroup colorGroup ) const
{
    if ( m_data->needle )
    {
        direction = 360.0 - direction; // counter clockwise
        m_data->needle->draw( painter, center, radius, direction, colorGroup );
    }
}

void QwtDial::drawNeedle( QPainter* painter ) const
{
    if ( !isValid() )
        return;

    QPalette::ColorGroup colorGroup;
    if ( isEnabled() )
        colorGroup = hasFocus() ? QPalette::Active : QPalette::Inactive;
    else
        colorGroup = QPalette::Disabled;

    const QRectF sr = scaleInnerRect();

    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, true );
    drawNeedle( painter, sr.center(), 0.5 * sr.width(),
        scaleMap().transform( value() ) + 270.0, colorGroup );
    painter->restore();
}

/*!
   Draw the scale

   \param painter Painter
   \param center Center of the dial
   \param radius Radius of the scale
 */
void QwtDial::drawScale( QPainter* painter,
    const QPointF& center, double radius ) const
{
    QwtRoundScaleDraw* sd = const_cast< QwtRoundScaleDraw* >( scaleDraw() );
    if ( sd == NULL )
        return;

    sd->setRadius( radius );
    sd->moveCenter( center );

    QPalette pal = palette();

    const QColor textColor = pal.color( QPalette::Text );
    pal.setColor( QPalette::WindowText, textColor ); // ticks, backbone

    painter->setFont( font() );
    painter->setPen( QPen( textColor, sd->penWidthF() ) );

    painter->setBrush( Qt::red );
    sd->draw( painter, pal );
}

/*!
   Draw the contents inside the scale

   Paints nothing.

   \param painter Painter
   \param center Center of the contents circle
   \param radius Radius of the contents circle
 */
void QwtDial::drawScaleContents( QPainter* painter,
    const QPointF& center, double radius ) const
{
    Q_UNUSED(painter);
    Q_UNUSED(center);
    Q_UNUSED(radius);
}

/*!
   Set a needle for the dial

   \param needle Needle

   \warning The needle will be deleted, when a different needle is
           set or in ~QwtDial()
 */
void QwtDial::setNeedle( QwtDialNeedle* needle )
{
    if ( needle != m_data->needle )
    {
        if ( m_data->needle )
            delete m_data->needle;

        m_data->needle = needle;
        update();
    }
}

/*!
   \return needle
   \sa setNeedle()
 */
const QwtDialNeedle* QwtDial::needle() const
{
    return m_data->needle;
}

/*!
   \return needle
   \sa setNeedle()
 */
QwtDialNeedle* QwtDial::needle()
{
    return m_data->needle;
}

//! \return the scale draw
QwtRoundScaleDraw* QwtDial::scaleDraw()
{
    return static_cast< QwtRoundScaleDraw* >( abstractScaleDraw() );
}

//! \return the scale draw
const QwtRoundScaleDraw* QwtDial::scaleDraw() const
{
    return static_cast< const QwtRoundScaleDraw* >( abstractScaleDraw() );
}

/*!
   Set an individual scale draw

   The motivation for setting a scale draw is often
   to overload QwtRoundScaleDraw::label() to return
   individual tick labels.

   \param scaleDraw Scale draw
   \warning The previous scale draw is deleted
 */
void QwtDial::setScaleDraw( QwtRoundScaleDraw* scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
    sliderChange();
}

/*!
   Change the arc of the scale

   \param minArc Lower limit
   \param maxArc Upper limit

   \sa minScaleArc(), maxScaleArc()
 */
void QwtDial::setScaleArc( double minArc, double maxArc )
{
    if ( minArc != 360.0 && minArc != -360.0 )
        minArc = std::fmod( minArc, 360.0 );
    if ( maxArc != 360.0 && maxArc != -360.0 )
        maxArc = std::fmod( maxArc, 360.0 );

    double minScaleArc = qwtMinF( minArc, maxArc );
    double maxScaleArc = qwtMaxF( minArc, maxArc );

    if ( maxScaleArc - minScaleArc > 360.0 )
        maxScaleArc = minScaleArc + 360.0;

    if ( ( minScaleArc != m_data->minScaleArc ) ||
        ( maxScaleArc != m_data->maxScaleArc ) )
    {
        m_data->minScaleArc = minScaleArc;
        m_data->maxScaleArc = maxScaleArc;

        invalidateCache();
        sliderChange();
    }
}

/*!
   Set the lower limit for the scale arc

   \param min Lower limit of the scale arc
   \sa setScaleArc(), setMaxScaleArc()
 */
void QwtDial::setMinScaleArc( double min )
{
    setScaleArc( min, m_data->maxScaleArc );
}

/*!
   \return Lower limit of the scale arc
   \sa setScaleArc()
 */
double QwtDial::minScaleArc() const
{
    return m_data->minScaleArc;
}

/*!
   Set the upper limit for the scale arc

   \param max Upper limit of the scale arc
   \sa setScaleArc(), setMinScaleArc()
 */
void QwtDial::setMaxScaleArc( double max )
{
    setScaleArc( m_data->minScaleArc, max );
}

/*!
   \return Upper limit of the scale arc
   \sa setScaleArc()
 */
double QwtDial::maxScaleArc() const
{
    return m_data->maxScaleArc;
}

/*!
   \brief Change the origin

   The origin is the angle where scale and needle is relative to.

   \param origin New origin
   \sa origin()
 */
void QwtDial::setOrigin( double origin )
{
    invalidateCache();

    m_data->origin = origin;
    sliderChange();
}

/*!
   The origin is the angle where scale and needle is relative to.

   \return Origin of the dial
   \sa setOrigin()
 */
double QwtDial::origin() const
{
    return m_data->origin;
}

/*!
   \return Size hint
   \sa minimumSizeHint()
 */
QSize QwtDial::sizeHint() const
{
    int sh = 0;
    if ( scaleDraw() )
        sh = qwtCeil( scaleDraw()->extent( font() ) );

    const int d = 6 * sh + 2 * lineWidth();

    QSize hint( d, d );
    if ( !isReadOnly() )
        hint = qwtExpandedToGlobalStrut( hint );

    return hint;
}

/*!
   \return Minimum size hint
   \sa sizeHint()
 */
QSize QwtDial::minimumSizeHint() const
{
    int sh = 0;
    if ( scaleDraw() )
        sh = qwtCeil( scaleDraw()->extent( font() ) );

    const int d = 3 * sh + 2 * lineWidth();

    return QSize( d, d );
}

/*!
   \brief Determine what to do when the user presses a mouse button.

   \param pos Mouse position

   \retval True, when the inner circle contains pos
   \sa scrolledTo()
 */
bool QwtDial::isScrollPosition( const QPoint& pos ) const
{
    const QRegion region( innerRect(), QRegion::Ellipse );
    if ( region.contains( pos ) && ( pos != innerRect().center() ) )
    {
        double angle = QLineF( rect().center(), pos ).angle();
        if ( m_data->mode == QwtDial::RotateScale )
            angle = 360.0 - angle;

        double valueAngle =
            qwtNormalizeDegrees( 90.0 - scaleMap().transform( value() ) );

        m_data->mouseOffset = qwtNormalizeDegrees( angle - valueAngle );
        m_data->arcOffset = scaleMap().p1();

        return true;
    }

    return false;
}

/*!
   \brief Determine the value for a new position of the
         slider handle.

   \param pos Mouse position

   \return Value for the mouse position
   \sa isScrollPosition()
 */
double QwtDial::scrolledTo( const QPoint& pos ) const
{
    double angle = QLineF( rect().center(), pos ).angle();
    if ( m_data->mode == QwtDial::RotateScale )
    {
        angle += scaleMap().p1() - m_data->arcOffset;
        angle = 360.0 - angle;
    }

    angle = qwtNormalizeDegrees( angle - m_data->mouseOffset );
    angle = qwtNormalizeDegrees( 90.0 - angle );

    if ( scaleMap().pDist() >= 360.0 )
    {
        if ( angle < scaleMap().p1() )
            angle += 360.0;

        if ( !wrapping() )
        {
            double boundedAngle = angle;

            const double arc = angle - scaleMap().transform( value() );
            if ( qAbs( arc ) > 180.0 )
            {
                boundedAngle = ( arc > 0 )
                    ? scaleMap().p1() : scaleMap().p2();
            }

            m_data->mouseOffset += ( boundedAngle - angle );

            angle = boundedAngle;
        }
    }
    else
    {
        const double boundedAngle =
            qwtBoundedAngle( scaleMap().p1(), angle, scaleMap().p2() );

        if ( !wrapping() )
            m_data->mouseOffset += ( boundedAngle - angle );

        angle = boundedAngle;
    }

    return scaleMap().invTransform( angle );
}

/*!
   Change Event handler
   \param event Change event

   Invalidates internal paint caches if necessary
 */
void QwtDial::changeEvent( QEvent* event )
{
    switch( event->type() )
    {
        case QEvent::EnabledChange:
        case QEvent::FontChange:
        case QEvent::StyleChange:
        case QEvent::PaletteChange:
        case QEvent::LanguageChange:
        case QEvent::LocaleChange:
        {
            invalidateCache();
            break;
        }
        default:
            break;
    }

    QwtAbstractSlider::changeEvent( event );
}

/*!
   Wheel Event handler
   \param event Wheel event
 */
void QwtDial::wheelEvent( QWheelEvent* event )
{
#if QT_VERSION < 0x050e00
    const QPoint wheelPos = event->pos();
#else
    const QPoint wheelPos = event->position().toPoint();
#endif

    const QRegion region( innerRect(), QRegion::Ellipse );
    if ( region.contains( wheelPos ) )
        QwtAbstractSlider::wheelEvent( event );
}

void QwtDial::setAngleRange( double angle, double span )
{
    if ( QwtRoundScaleDraw* sd = scaleDraw() )
    {
        angle = qwtNormalizeDegrees( angle - 270.0 );
        sd->setAngleRange( angle, angle + span );
    }
}

/*!
   Invalidate the internal caches and call
   QwtAbstractSlider::scaleChange()
 */
void QwtDial::scaleChange()
{
    invalidateCache();
    QwtAbstractSlider::scaleChange();
}

void QwtDial::sliderChange()
{
    setAngleRange( m_data->origin + m_data->minScaleArc,
        m_data->maxScaleArc - m_data->minScaleArc );

    if ( mode() == RotateScale )
    {
        const double arc = scaleMap().transform( value() ) - scaleMap().p1();
        setAngleRange( m_data->origin - arc,
            m_data->maxScaleArc - m_data->minScaleArc );
    }

    QwtAbstractSlider::sliderChange();
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_dial.cpp"
#endif
