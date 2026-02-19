/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_knob.h"
#include "qwt_round_scale_draw.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_math.h"
#include "qwt.h"

#include <qpainter.h>
#include <qpalette.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qmargins.h>
#include <qmath.h>

static QSize qwtKnobSizeHint( const QwtKnob* knob, int min )
{
    int knobWidth = knob->knobWidth();
    if ( knobWidth <= 0 )
        knobWidth = qMax( 3 * knob->markerSize(), min );

    // Add the scale radial thickness to the knobWidth
    const int extent = qwtCeil( knob->scaleDraw()->extent( knob->font() ) );
    const int d = 2 * ( extent + 4 ) + knobWidth;

    const QMargins m = knob->contentsMargins();
    return QSize( d + m.left() + m.right(), d + m.top() + m.bottom() );
}

static inline double qwtToScaleAngle( double angle )
{
    // the map is counter clockwise with the origin
    // at 90° using angles from -180° -> 180°

    double a = 90.0 - angle;
    if ( a <= -180.0 )
        a += 360.0;
    else if ( a >= 180.0 )
        a -= 360.0;

    return a;
}

static double qwtToDegrees( double value )
{
    return qwtNormalizeDegrees( 90.0 - value );
}

class QwtKnob::PrivateData
{
  public:
    PrivateData()
        : knobStyle( QwtKnob::Raised )
        , markerStyle( QwtKnob::Notch )
        , borderWidth( 2 )
        , borderDist( 4 )
        , scaleDist( 4 )
        , maxScaleTicks( 11 )
        , knobWidth( 0 )
        , alignment( Qt::AlignCenter )
        , markerSize( 8 )
        , totalAngle( 270.0 )
        , mouseOffset( 0.0 )
    {
    }

    QwtKnob::KnobStyle knobStyle;
    QwtKnob::MarkerStyle markerStyle;

    int borderWidth;
    int borderDist;
    int scaleDist;
    int maxScaleTicks;
    int knobWidth;
    Qt::Alignment alignment;
    int markerSize;

    double totalAngle;

    double mouseOffset;
};

/*!
   \brief Constructor

   Construct a knob with an angle of 270°. The style is
   QwtKnob::Raised and the marker style is QwtKnob::Notch.
   The width of the knob is set to 50 pixels.

   \param parent Parent widget

   \sa setTotalAngle()
 */
QwtKnob::QwtKnob( QWidget* parent )
    : QwtAbstractSlider( parent )
{
    m_data = new PrivateData;

    setScaleDraw( new QwtRoundScaleDraw() );

    setTotalAngle( 270.0 );

    setScale( 0.0, 10.0 );
    setValue( 0.0 );

    setSizePolicy( QSizePolicy::MinimumExpanding,
        QSizePolicy::MinimumExpanding );
}

//! Destructor
QwtKnob::~QwtKnob()
{
    delete m_data;
}

/*!
   \brief Set the knob type

   \param knobStyle Knob type
   \sa knobStyle(), setBorderWidth()
 */
void QwtKnob::setKnobStyle( KnobStyle knobStyle )
{
    if ( m_data->knobStyle != knobStyle )
    {
        m_data->knobStyle = knobStyle;
        update();
    }
}

/*!
    \return Marker type of the knob
    \sa setKnobStyle(), setBorderWidth()
 */
QwtKnob::KnobStyle QwtKnob::knobStyle() const
{
    return m_data->knobStyle;
}

/*!
   \brief Set the marker type of the knob

   \param markerStyle Marker type
   \sa markerStyle(), setMarkerSize()
 */
void QwtKnob::setMarkerStyle( MarkerStyle markerStyle )
{
    if ( m_data->markerStyle != markerStyle )
    {
        m_data->markerStyle = markerStyle;
        update();
    }
}

/*!
   \return Marker type of the knob
   \sa setMarkerStyle(), setMarkerSize()
 */
QwtKnob::MarkerStyle QwtKnob::markerStyle() const
{
    return m_data->markerStyle;
}

/*!
   \brief Set the total angle by which the knob can be turned
   \param angle Angle in degrees.

   The angle has to be between [10, 360] degrees. Angles above
   360 ( so that the knob can be turned several times around its axis )
   have to be set using setNumTurns().

   The default angle is 270 degrees.

   \sa totalAngle(), setNumTurns()
 */
void QwtKnob::setTotalAngle ( double angle )
{
    angle = qBound( 10.0, angle, 360.0 );

    if ( angle != m_data->totalAngle )
    {
        m_data->totalAngle = angle;

        scaleDraw()->setAngleRange( -0.5 * m_data->totalAngle,
            0.5 * m_data->totalAngle );

        updateGeometry();
        update();
    }
}

/*!
   \return the total angle
   \sa setTotalAngle(), setNumTurns(), numTurns()
 */
double QwtKnob::totalAngle() const
{
    return m_data->totalAngle;
}

/*!
   \brief Set the number of turns

   When numTurns > 1 the knob can be turned several times around its axis
   - otherwise the total angle is floored to 360°.

   \sa numTurns(), totalAngle(), setTotalAngle()
 */

void QwtKnob::setNumTurns( int numTurns )
{
    numTurns = qMax( numTurns, 1 );

    if ( numTurns == 1 && m_data->totalAngle <= 360.0 )
        return;

    const double angle = numTurns * 360.0;
    if ( angle != m_data->totalAngle )
    {
        m_data->totalAngle = angle;

        scaleDraw()->setAngleRange( -0.5 * m_data->totalAngle,
            0.5 * m_data->totalAngle );

        updateGeometry();
        update();
    }
}

/*!
   \return Number of turns.

   When the total angle is below 360° numTurns() is ceiled to 1.
   \sa setNumTurns(), setTotalAngle(), totalAngle()
 */
int QwtKnob::numTurns() const
{
    return qwtCeil( m_data->totalAngle / 360.0 );
}

/*!
   Change the scale draw of the knob

   For changing the labels of the scales, it
   is necessary to derive from QwtRoundScaleDraw and
   overload QwtRoundScaleDraw::label().

   \sa scaleDraw()
 */
void QwtKnob::setScaleDraw( QwtRoundScaleDraw* scaleDraw )
{
    setAbstractScaleDraw( scaleDraw );
    setTotalAngle( m_data->totalAngle );

    updateGeometry();
    update();
}

/*!
   \return the scale draw of the knob
   \sa setScaleDraw()
 */
const QwtRoundScaleDraw* QwtKnob::scaleDraw() const
{
    return static_cast< const QwtRoundScaleDraw* >( abstractScaleDraw() );
}

/*!
   \return the scale draw of the knob
   \sa setScaleDraw()
 */
QwtRoundScaleDraw* QwtKnob::scaleDraw()
{
    return static_cast< QwtRoundScaleDraw* >( abstractScaleDraw() );
}

/*!
   Calculate the bounding rectangle of the knob without the scale

   \return Bounding rectangle of the knob
   \sa knobWidth(), alignment(), QWidget::contentsRect()
 */
QRect QwtKnob::knobRect() const
{
    const QRect cr = contentsRect();

    const int extent = qwtCeil( scaleDraw()->extent( font() ) );
    const int d = extent + m_data->scaleDist;

    int w = m_data->knobWidth;
    if ( w <= 0 )
    {
        const int dim = qMin( cr.width(), cr.height() );

        w = dim - 2 * ( d );
        w = qMax( 0, w );
    }

    QRect r( 0, 0, w, w );

    if ( m_data->alignment & Qt::AlignLeft )
    {
        r.moveLeft( cr.left() + d );
    }
    else if ( m_data->alignment & Qt::AlignRight )
    {
        r.moveRight( cr.right() - d );
    }
    else
    {
        r.moveCenter( QPoint( cr.center().x(), r.center().y() ) );
    }

    if ( m_data->alignment & Qt::AlignTop )
    {
        r.moveTop( cr.top() + d );
    }
    else if ( m_data->alignment & Qt::AlignBottom )
    {
        r.moveBottom( cr.bottom() - d );
    }
    else
    {
        r.moveCenter( QPoint( r.center().x(), cr.center().y() ) );
    }

    return r;
}

/*!
   \brief Determine what to do when the user presses a mouse button.

   \param pos Mouse position

   \retval True, when pos is inside the circle of the knob.
   \sa scrolledTo()
 */
bool QwtKnob::isScrollPosition( const QPoint& pos ) const
{
    const QRect kr = knobRect();

    const QRegion region( kr, QRegion::Ellipse );
    if ( region.contains( pos ) && ( pos != kr.center() ) )
    {
        const double angle = QLineF( kr.center(), pos ).angle();
        const double valueAngle = qwtToDegrees( scaleMap().transform( value() ) );

        m_data->mouseOffset = qwtNormalizeDegrees( angle - valueAngle );

        return true;
    }

    return false;
}

/*!
   \brief Determine the value for a new position of the mouse

   \param pos Mouse position

   \return Value for the mouse position
   \sa isScrollPosition()
 */
double QwtKnob::scrolledTo( const QPoint& pos ) const
{
    double angle = QLineF( rect().center(), pos ).angle();
    angle = qwtNormalizeDegrees( angle - m_data->mouseOffset );

    if ( scaleMap().pDist() > 360.0 )
    {
        angle = qwtToDegrees( angle );

        const double v = scaleMap().transform( value() );

        int numTurns = qwtFloor( ( v - scaleMap().p1() ) / 360.0 );

        double valueAngle = qwtNormalizeDegrees( v );
        if ( qAbs( valueAngle - angle ) > 180.0 )
        {
            numTurns += ( angle > valueAngle ) ? -1 : 1;
        }

        angle += scaleMap().p1() + numTurns * 360.0;

        if ( !wrapping() )
        {
            const double boundedAngle =
                qBound( scaleMap().p1(), angle, scaleMap().p2() );

            m_data->mouseOffset += ( boundedAngle - angle );
            angle = boundedAngle;
        }
    }
    else
    {
        angle = qwtToScaleAngle( angle );

        double boundedAngle = qBound( scaleMap().p1(), angle, scaleMap().p2() );

        if ( !wrapping() )
        {
            const double currentAngle = scaleMap().transform( value() );

            if ( ( currentAngle > 90.0 ) && ( boundedAngle < -90.0 ) )
                boundedAngle = scaleMap().p2();
            else if ( ( currentAngle < -90.0 ) && ( boundedAngle > 90.0 ) )
                boundedAngle = scaleMap().p1();

            m_data->mouseOffset += ( boundedAngle - angle );
        }

        angle = boundedAngle;
    }

    return scaleMap().invTransform( angle );
}

/*!
   Handle QEvent::StyleChange and QEvent::FontChange;
   \param event Change event
 */
void QwtKnob::changeEvent( QEvent* event )
{
    switch( event->type() )
    {
        case QEvent::StyleChange:
        case QEvent::FontChange:
        {
            updateGeometry();
            update();
            break;
        }
        default:
            break;
    }
}

/*!
   Repaint the knob
   \param event Paint event
 */
void QwtKnob::paintEvent( QPaintEvent* event )
{
    const QRectF knobRect = this->knobRect();

    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    painter.setRenderHint( QPainter::Antialiasing, true );

    if ( !knobRect.contains( event->region().boundingRect() ) )
    {
        scaleDraw()->setRadius( 0.5 * knobRect.width() + m_data->scaleDist );
        scaleDraw()->moveCenter( knobRect.center() );

        scaleDraw()->draw( &painter, palette() );
    }

    drawKnob( &painter, knobRect );

    drawMarker( &painter, knobRect,
        qwtNormalizeDegrees( scaleMap().transform( value() ) ) );

    painter.setRenderHint( QPainter::Antialiasing, false );

    if ( hasFocus() )
        drawFocusIndicator( &painter );
}

/*!
   \brief Draw the knob

   \param painter painter
   \param knobRect Bounding rectangle of the knob (without scale)
 */
void QwtKnob::drawKnob( QPainter* painter, const QRectF& knobRect ) const
{
    double dim = qMin( knobRect.width(), knobRect.height() );
    dim -= m_data->borderWidth * 0.5;

    QRectF aRect( 0, 0, dim, dim );
    aRect.moveCenter( knobRect.center() );

    QPen pen( Qt::NoPen );
    if ( m_data->borderWidth > 0 )
    {
        QColor c1 = palette().color( QPalette::Light );
        QColor c2 = palette().color( QPalette::Dark );

        QLinearGradient gradient( aRect.topLeft(), aRect.bottomRight() );
        gradient.setColorAt( 0.0, c1 );
        gradient.setColorAt( 0.3, c1 );
        gradient.setColorAt( 0.7, c2 );
        gradient.setColorAt( 1.0, c2 );

        pen = QPen( gradient, m_data->borderWidth );
    }

    QBrush brush;
    switch( m_data->knobStyle )
    {
        case QwtKnob::Raised:
        {
            double off = 0.3 * knobRect.width();
            QRadialGradient gradient( knobRect.center(),
                knobRect.width(), knobRect.topLeft() + QPointF( off, off ) );

            gradient.setColorAt( 0.0, palette().color( QPalette::Midlight ) );
            gradient.setColorAt( 1.0, palette().color( QPalette::Button ) );

            brush = QBrush( gradient );

            break;
        }
        case QwtKnob::Styled:
        {
            QRadialGradient gradient(knobRect.center().x() - knobRect.width() / 3,
                knobRect.center().y() - knobRect.height() / 2,
                knobRect.width() * 1.3,
                knobRect.center().x(),
                knobRect.center().y() - knobRect.height() / 2);

            const QColor c = palette().color( QPalette::Button );
            gradient.setColorAt(0, c.lighter(110) );
            gradient.setColorAt( 0.5, c);
            gradient.setColorAt( 0.501, c.darker(102) );
            gradient.setColorAt(1, c.darker(115) );

            brush = QBrush( gradient );

            break;
        }
        case QwtKnob::Sunken:
        {
            QLinearGradient gradient(
                knobRect.topLeft(), knobRect.bottomRight() );
            gradient.setColorAt( 0.0, palette().color( QPalette::Mid ) );
            gradient.setColorAt( 0.5, palette().color( QPalette::Button ) );
            gradient.setColorAt( 1.0, palette().color( QPalette::Midlight ) );
            brush = QBrush( gradient );

            break;
        }
        case QwtKnob::Flat:
        default:
            brush = palette().brush( QPalette::Button );
    }

    painter->setPen( pen );
    painter->setBrush( brush );
    painter->drawEllipse( aRect );
}


/*!
   \brief Draw the marker at the knob's front

   \param painter Painter
   \param rect Bounding rectangle of the knob without scale
   \param angle Angle of the marker in degrees
               ( clockwise, 0 at the 12 o'clock position )
 */
void QwtKnob::drawMarker( QPainter* painter,
    const QRectF& rect, double angle ) const
{
    if ( m_data->markerStyle == NoMarker || !isValid() )
        return;

    const double radians = qwtRadians( angle );
    const double sinA = -qFastSin( radians );
    const double cosA = qFastCos( radians );

    const double xm = rect.center().x();
    const double ym = rect.center().y();
    const double margin = 4.0;

    double radius = 0.5 * ( rect.width() - m_data->borderWidth ) - margin;
    if ( radius < 1.0 )
        radius = 1.0;

    double markerSize = m_data->markerSize;
    if ( markerSize <= 0 )
        markerSize = qRound( 0.4 * radius );

    switch ( m_data->markerStyle )
    {
        case Notch:
        case Nub:
        {
            const double dotWidth = qwtMinF( markerSize, radius );

            const double dotCenterDist = radius - 0.5 * dotWidth;
            if ( dotCenterDist > 0.0 )
            {
                const QPointF center( xm - sinA * dotCenterDist,
                    ym - cosA * dotCenterDist );

                QRectF ellipse( 0.0, 0.0, dotWidth, dotWidth );
                ellipse.moveCenter( center );

                QColor c1 = palette().color( QPalette::Light );
                QColor c2 = palette().color( QPalette::Mid );

                if ( m_data->markerStyle == Notch )
                    qSwap( c1, c2 );

                QLinearGradient gradient(
                    ellipse.topLeft(), ellipse.bottomRight() );
                gradient.setColorAt( 0.0, c1 );
                gradient.setColorAt( 1.0, c2 );

                painter->setPen( Qt::NoPen );
                painter->setBrush( gradient );

                painter->drawEllipse( ellipse );
            }
            break;
        }
        case Dot:
        {
            const double dotWidth = qwtMinF( markerSize, radius);

            const double dotCenterDist = radius - 0.5 * dotWidth;
            if ( dotCenterDist > 0.0 )
            {
                const QPointF center( xm - sinA * dotCenterDist,
                    ym - cosA * dotCenterDist );

                QRectF ellipse( 0.0, 0.0, dotWidth, dotWidth );
                ellipse.moveCenter( center );

                painter->setPen( Qt::NoPen );
                painter->setBrush( palette().color( QPalette::ButtonText ) );
                painter->drawEllipse( ellipse );
            }

            break;
        }
        case Tick:
        {
            const double rb = qwtMaxF( radius - markerSize, 1.0 );
            const double re = radius;

            const QLineF line( xm - sinA * rb, ym - cosA * rb,
                xm - sinA * re, ym - cosA * re );

            QPen pen( palette().color( QPalette::ButtonText ), 0 );
            pen.setCapStyle( Qt::FlatCap );
            painter->setPen( pen );
            painter->drawLine ( line );

            break;
        }
        case Triangle:
        {
            const double rb = qwtMaxF( radius - markerSize, 1.0 );
            const double re = radius;

            painter->translate( rect.center() );
            painter->rotate( angle - 90.0 );

            QPolygonF polygon;
            polygon += QPointF( re, 0.0 );
            polygon += QPointF( rb, 0.5 * ( re - rb ) );
            polygon += QPointF( rb, -0.5 * ( re - rb ) );

            painter->setPen( Qt::NoPen );
            painter->setBrush( palette().color( QPalette::ButtonText ) );
            painter->drawPolygon( polygon );

            painter->resetTransform();

            break;
        }
        default:
            break;
    }
}

/*!
   Draw the focus indicator
   \param painter Painter
 */
void QwtKnob::drawFocusIndicator( QPainter* painter ) const
{
    const QRect cr = contentsRect();

    int w = m_data->knobWidth;
    if ( w <= 0 )
    {
        w = qMin( cr.width(), cr.height() );
    }
    else
    {
        const int extent = qCeil( scaleDraw()->extent( font() ) );
        w += 2 * ( extent + m_data->scaleDist );
    }

    QRect focusRect( 0, 0, w, w );
    focusRect.moveCenter( cr.center() );

    QwtPainter::drawFocusRect( painter, this, focusRect );
}

/*!
   \brief Set the alignment of the knob

   Similar to a QLabel::alignment() the flags decide how
   to align the knob inside of contentsRect().

   The default setting is Qt::AlignCenter

   \param alignment Or'd alignment flags

   \sa alignment(), setKnobWidth(), knobRect()
 */
void QwtKnob::setAlignment( Qt::Alignment alignment )
{
    if ( m_data->alignment != alignment )
    {
        m_data->alignment = alignment;
        update();
    }
}

/*!
   \return Alignment of the knob inside of contentsRect()
   \sa setAlignment(), knobWidth(), knobRect()
 */
Qt::Alignment QwtKnob::alignment() const
{
    return m_data->alignment;
}

/*!
   \brief Change the knob's width.

   Setting a fixed value for the diameter of the knob
   is helpful for aligning several knobs in a row.

   \param width New width

   \sa knobWidth(), setAlignment()
   \note Modifies the sizePolicy()
 */
void QwtKnob::setKnobWidth( int width )
{
    width = qMax( width, 0 );

    if ( width != m_data->knobWidth )
    {
        QSizePolicy::Policy policy;
        if ( width > 0 )
            policy = QSizePolicy::Minimum;
        else
            policy = QSizePolicy::MinimumExpanding;

        setSizePolicy( policy, policy );

        m_data->knobWidth = width;

        updateGeometry();
        update();
    }
}

//! Return the width of the knob
int QwtKnob::knobWidth() const
{
    return m_data->knobWidth;
}

/*!
   \brief Set the knob's border width
   \param borderWidth new border width
 */
void QwtKnob::setBorderWidth( int borderWidth )
{
    m_data->borderWidth = qMax( borderWidth, 0 );

    updateGeometry();
    update();
}

//! Return the border width
int QwtKnob::borderWidth() const
{
    return m_data->borderWidth;
}

/*!
   \brief Set the size of the marker

   When setting a size <= 0 the marker will
   automatically scaled to 40% of the radius of the knob.

   \sa markerSize(), markerStyle()
 */
void QwtKnob::setMarkerSize( int size )
{
    if ( m_data->markerSize != size )
    {
        m_data->markerSize = size;
        update();
    }
}

/*!
   \return Marker size
   \sa setMarkerSize()
 */
int QwtKnob::markerSize() const
{
    return m_data->markerSize;
}

/*!
   \return sizeHint()
 */
QSize QwtKnob::sizeHint() const
{
    const QSize hint = qwtKnobSizeHint( this, 50 );
    return qwtExpandedToGlobalStrut( hint );
}

/*!
   \return Minimum size hint
   \sa sizeHint()
 */
QSize QwtKnob::minimumSizeHint() const
{
    return qwtKnobSizeHint( this, 20 );
}

#include "moc_qwt_knob.cpp"
