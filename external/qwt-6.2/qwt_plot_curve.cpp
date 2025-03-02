/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_curve.h"
#include "qwt_point_data.h"
#include "qwt_math.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_spline_curve_fitter.h"
#include "qwt_symbol.h"
#include "qwt_point_mapper.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

#include <qpainter.h>
#include <qpainterpath.h>

static inline QRectF qwtIntersectedClipRect( const QRectF& rect, QPainter* painter )
{
    QRectF clipRect = rect;
    if ( painter->hasClipping() )
        clipRect &= painter->clipBoundingRect();

    return clipRect;
}

static void qwtUpdateLegendIconSize( QwtPlotCurve* curve )
{
    if ( curve->symbol() &&
        curve->testLegendAttribute( QwtPlotCurve::LegendShowSymbol ) )
    {
        QSize sz = curve->symbol()->boundingRect().size();
        sz += QSize( 2, 2 ); // margin

        if ( curve->testLegendAttribute( QwtPlotCurve::LegendShowLine ) )
        {
            // Avoid, that the line is completely covered by the symbol

            int w = qwtCeil( 1.5 * sz.width() );
            if ( w % 2 )
                w++;

            sz.setWidth( qMax( 8, w ) );
        }

        curve->setLegendIconSize( sz );
    }
}

static int qwtVerifyRange( int size, int& i1, int& i2 )
{
    if ( size < 1 )
        return 0;

    i1 = qBound( 0, i1, size - 1 );
    i2 = qBound( 0, i2, size - 1 );

    if ( i1 > i2 )
        qSwap( i1, i2 );

    return ( i2 - i1 + 1 );
}

class QwtPlotCurve::PrivateData
{
  public:
    PrivateData()
        : style( QwtPlotCurve::Lines )
        , baseline( 0.0 )
        , symbol( NULL )
        , pen( Qt::black )
        , paintAttributes( QwtPlotCurve::ClipPolygons | QwtPlotCurve::FilterPoints )
    {
        curveFitter = new QwtSplineCurveFitter;
    }

    ~PrivateData()
    {
        delete symbol;
        delete curveFitter;
    }

    QwtPlotCurve::CurveStyle style;
    double baseline;

    const QwtSymbol* symbol;
    QwtCurveFitter* curveFitter;

    QPen pen;
    QBrush brush;

    QwtPlotCurve::CurveAttributes attributes;
    QwtPlotCurve::PaintAttributes paintAttributes;

    QwtPlotCurve::LegendAttributes legendAttributes;
};

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotCurve::QwtPlotCurve( const QwtText& title )
    : QwtPlotSeriesItem( title )
{
    init();
}

/*!
   Constructor
   \param title Title of the curve
 */
QwtPlotCurve::QwtPlotCurve( const QString& title )
    : QwtPlotSeriesItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPlotCurve::~QwtPlotCurve()
{
    delete m_data;
}

//! Initialize internal members
void QwtPlotCurve::init()
{
    setItemAttribute( QwtPlotItem::Legend );
    setItemAttribute( QwtPlotItem::AutoScale );

    m_data = new PrivateData;
    setData( new QwtPointSeriesData() );

    setZ( 20.0 );
}

//! \return QwtPlotItem::Rtti_PlotCurve
int QwtPlotCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotCurve;
}

/*!
   Specify an attribute how to draw the curve

   \param attribute Paint attribute
   \param on On/Off
   \sa testPaintAttribute()
 */
void QwtPlotCurve::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;
}

/*!
    \return True, when attribute is enabled
    \sa setPaintAttribute()
 */
bool QwtPlotCurve::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( m_data->paintAttributes & attribute );
}

/*!
   Specify an attribute how to draw the legend icon

   \param attribute Attribute
   \param on On/Off
   /sa testLegendAttribute(). legendIcon()
 */
void QwtPlotCurve::setLegendAttribute( LegendAttribute attribute, bool on )
{
    if ( on != testLegendAttribute( attribute ) )
    {
        if ( on )
            m_data->legendAttributes |= attribute;
        else
            m_data->legendAttributes &= ~attribute;

        qwtUpdateLegendIconSize( this );
        legendChanged();
    }
}

/*!
   \return True, when attribute is enabled
   \sa setLegendAttribute()
 */
bool QwtPlotCurve::testLegendAttribute( LegendAttribute attribute ) const
{
    return ( m_data->legendAttributes & attribute );
}

/*!
   Specify the attributes how to draw the legend icon

   \param attributes Attributes
   /sa setLegendAttribute(). legendIcon()
 */
void QwtPlotCurve::setLegendAttributes( LegendAttributes attributes )
{
    if ( attributes != m_data->legendAttributes )
    {
        m_data->legendAttributes = attributes;

        qwtUpdateLegendIconSize( this );
        legendChanged();
    }
}

/*!
   \return Attributes how to draw the legend icon
   \sa setLegendAttributes(), testLegendAttribute()
 */
QwtPlotCurve::LegendAttributes QwtPlotCurve::legendAttributes() const
{
    return m_data->legendAttributes;
}

/*!
   Set the curve's drawing style

   \param style Curve style
   \sa style()
 */
void QwtPlotCurve::setStyle( CurveStyle style )
{
    if ( style != m_data->style )
    {
        m_data->style = style;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Style of the curve
   \sa setStyle()
 */
QwtPlotCurve::CurveStyle QwtPlotCurve::style() const
{
    return m_data->style;
}

/*!
   \brief Assign a symbol

   The curve will take the ownership of the symbol, hence the previously
   set symbol will be delete by setting a new one. If \p symbol is
   \c NULL no symbol will be drawn.

   \param symbol Symbol
   \sa symbol()
 */
void QwtPlotCurve::setSymbol( QwtSymbol* symbol )
{
    if ( symbol != m_data->symbol )
    {
        delete m_data->symbol;
        m_data->symbol = symbol;

        qwtUpdateLegendIconSize( this );

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Current symbol or NULL, when no symbol has been assigned
   \sa setSymbol()
 */
const QwtSymbol* QwtPlotCurve::symbol() const
{
    return m_data->symbol;
}

/*!
   Build and assign a pen

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotCurve::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   Assign a pen

   \param pen New pen
   \sa pen(), brush()
 */
void QwtPlotCurve::setPen( const QPen& pen )
{
    if ( pen != m_data->pen )
    {
        m_data->pen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Pen used to draw the lines
   \sa setPen(), brush()
 */
const QPen& QwtPlotCurve::pen() const
{
    return m_data->pen;
}

/*!
   \brief Assign a brush.

   In case of brush.style() != QBrush::NoBrush
   and style() != QwtPlotCurve::Sticks
   the area between the curve and the baseline will be filled.

   In case !brush.color().isValid() the area will be filled by
   pen.color(). The fill algorithm simply connects the first and the
   last curve point to the baseline. So the curve data has to be sorted
   (ascending or descending).

   \param brush New brush
   \sa brush(), setBaseline(), baseline()
 */
void QwtPlotCurve::setBrush( const QBrush& brush )
{
    if ( brush != m_data->brush )
    {
        m_data->brush = brush;

        legendChanged();
        itemChanged();
    }
}

/*!
   \return Brush used to fill the area between lines and the baseline
   \sa setBrush(), setBaseline(), baseline()
 */
const QBrush& QwtPlotCurve::brush() const
{
    return m_data->brush;
}

/*!
   Draw an interval of the curve

   \param painter Painter
   \param xMap Maps x-values into pixel coordinates.
   \param yMap Maps y-values into pixel coordinates.
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

   \sa drawCurve(), drawSymbols(),
 */
void QwtPlotCurve::drawSeries( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const size_t numSamples = dataSize();

    if ( !painter || numSamples <= 0 )
        return;

    if ( to < 0 )
        to = numSamples - 1;

    if ( qwtVerifyRange( numSamples, from, to ) > 0 )
    {
        painter->save();
        painter->setPen( m_data->pen );

        /*
           Qt 4.0.0 is slow when drawing lines, but it's even
           slower when the painter has a brush. So we don't
           set the brush before we really need it.
         */

        drawCurve( painter, m_data->style, xMap, yMap, canvasRect, from, to );
        painter->restore();

        if ( m_data->symbol &&
            ( m_data->symbol->style() != QwtSymbol::NoSymbol ) )
        {
            painter->save();
            drawSymbols( painter, *m_data->symbol,
                xMap, yMap, canvasRect, from, to );
            painter->restore();
        }
    }
}

/*!
   \brief Draw the line part (without symbols) of a curve interval.
   \param painter Painter
   \param style curve style, see QwtPlotCurve::CurveStyle
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from index of the first point to be painted
   \param to index of the last point to be painted
   \sa draw(), drawDots(), drawLines(), drawSteps(), drawSticks()
 */
void QwtPlotCurve::drawCurve( QPainter* painter, int style,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    switch ( style )
    {
        case Lines:
            if ( testCurveAttribute( Fitted ) )
            {
                // we always need the complete
                // curve for fitting
                from = 0;
                to = dataSize() - 1;
            }
            drawLines( painter, xMap, yMap, canvasRect, from, to );
            break;
        case Sticks:
            drawSticks( painter, xMap, yMap, canvasRect, from, to );
            break;
        case Steps:
            drawSteps( painter, xMap, yMap, canvasRect, from, to );
            break;
        case Dots:
            drawDots( painter, xMap, yMap, canvasRect, from, to );
            break;
        case NoCurve:
        default:
            break;
    }
}

/*!
   \brief Draw lines

   If the CurveAttribute Fitted is enabled a QwtCurveFitter tries
   to interpolate/smooth the curve, before it is painted.

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from index of the first point to be painted
   \param to index of the last point to be painted

   \sa setCurveAttribute(), setCurveFitter(), draw(),
      drawLines(), drawDots(), drawSteps(), drawSticks()
 */
void QwtPlotCurve::drawLines( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    if ( from > to )
        return;

    const bool doFit = ( m_data->attributes & Fitted ) && m_data->curveFitter;
    const bool doAlign = !doFit && QwtPainter::roundingAlignment( painter );
    const bool doFill = ( m_data->brush.style() != Qt::NoBrush )
        && ( m_data->brush.color().alpha() > 0 );

    QRectF clipRect;
    if ( m_data->paintAttributes & ClipPolygons )
    {
        clipRect = qwtIntersectedClipRect( canvasRect, painter );

        const qreal pw = QwtPainter::effectivePenWidth( painter->pen() );
        clipRect = clipRect.adjusted(-pw, -pw, pw, pw);
    }

    QwtPointMapper mapper;

    if ( doAlign )
    {
        mapper.setFlag( QwtPointMapper::RoundPoints, true );
        mapper.setFlag( QwtPointMapper::WeedOutIntermediatePoints,
            testPaintAttribute( FilterPointsAggressive ) );
    }

    mapper.setFlag( QwtPointMapper::WeedOutPoints,
        testPaintAttribute( FilterPoints ) ||
        testPaintAttribute( FilterPointsAggressive ) );

    mapper.setBoundingRect( canvasRect );

    QPolygonF polyline = mapper.toPolygonF( xMap, yMap, data(), from, to );

    if ( doFill )
    {
        if ( doFit )
        {
            // it might be better to extend and draw the curvePath, but for
            // the moment we keep an implementation, where we translate the
            // path back to a polyline.

            polyline = m_data->curveFitter->fitCurve( polyline );
        }

        if ( painter->pen().style() != Qt::NoPen )
        {
            // here we are wasting memory for the filled copy,
            // do polygon clipping twice etc .. TODO

            QPolygonF filled = polyline;
            fillCurve( painter, xMap, yMap, canvasRect, filled );
            filled.clear();

            if ( m_data->paintAttributes & ClipPolygons )
                QwtClipper::clipPolygonF( clipRect, polyline, false );

            QwtPainter::drawPolyline( painter, polyline );
        }
        else
        {
            fillCurve( painter, xMap, yMap, canvasRect, polyline );
        }
    }
    else
    {
        if ( testPaintAttribute( ClipPolygons ) )
        {
            QwtClipper::clipPolygonF( clipRect, polyline, false );
        }

        if ( doFit )
        {
            if ( m_data->curveFitter->mode() == QwtCurveFitter::Path )
            {
                const QPainterPath curvePath =
                    m_data->curveFitter->fitCurvePath( polyline );

                painter->drawPath( curvePath );
            }
            else
            {
                polyline = m_data->curveFitter->fitCurve( polyline );
                QwtPainter::drawPolyline( painter, polyline );
            }
        }
        else
        {
            QwtPainter::drawPolyline( painter, polyline );
        }
    }
}

/*!
   Draw sticks

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from index of the first point to be painted
   \param to index of the last point to be painted

   \sa draw(), drawCurve(), drawDots(), drawLines(), drawSteps()
 */
void QwtPlotCurve::drawSticks( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    Q_UNUSED( canvasRect )

    painter->save();
    painter->setRenderHint( QPainter::Antialiasing, false );

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double x0 = xMap.transform( m_data->baseline );
    double y0 = yMap.transform( m_data->baseline );
    if ( doAlign )
    {
        x0 = qRound( x0 );
        y0 = qRound( y0 );
    }

    const Qt::Orientation o = orientation();

    const QwtSeriesData< QPointF >* series = data();

    for ( int i = from; i <= to; i++ )
    {
        const QPointF sample = series->sample( i );
        double xi = xMap.transform( sample.x() );
        double yi = yMap.transform( sample.y() );
        if ( doAlign )
        {
            xi = qRound( xi );
            yi = qRound( yi );
        }

        if ( o == Qt::Horizontal )
            QwtPainter::drawLine( painter, x0, yi, xi, yi );
        else
            QwtPainter::drawLine( painter, xi, y0, xi, yi );
    }

    painter->restore();
}

/*!
   Draw dots

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from index of the first point to be painted
   \param to index of the last point to be painted

   \sa draw(), drawCurve(), drawSticks(), drawLines(), drawSteps()
 */
void QwtPlotCurve::drawDots( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const QColor color = painter->pen().color();

    if ( painter->pen().style() == Qt::NoPen || color.alpha() == 0 )
    {
        return;
    }

    const bool doFill = ( m_data->brush.style() != Qt::NoBrush )
        && ( m_data->brush.color().alpha() > 0 );
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    QwtPointMapper mapper;
    mapper.setBoundingRect( canvasRect );
    mapper.setFlag( QwtPointMapper::RoundPoints, doAlign );

    if ( m_data->paintAttributes & FilterPoints )
    {
        if ( ( color.alpha() == 255 )
            && !( painter->renderHints() & QPainter::Antialiasing ) )
        {
            mapper.setFlag( QwtPointMapper::WeedOutPoints, true );
        }
    }

    if ( doFill )
    {
        mapper.setFlag( QwtPointMapper::WeedOutPoints, false );

        QPolygonF points = mapper.toPolygonF(
            xMap, yMap, data(), from, to );

        QwtPainter::drawPoints( painter, points );
        fillCurve( painter, xMap, yMap, canvasRect, points );
    }
    else if ( m_data->paintAttributes & ImageBuffer )
    {
        const QImage image = mapper.toImage( xMap, yMap,
            data(), from, to, m_data->pen,
            painter->testRenderHint( QPainter::Antialiasing ),
            renderThreadCount() );

        painter->drawImage( canvasRect.toAlignedRect(), image );
    }
    else if ( m_data->paintAttributes & MinimizeMemory )
    {
        const QwtSeriesData< QPointF >* series = data();

        for ( int i = from; i <= to; i++ )
        {
            const QPointF sample = series->sample( i );

            double xi = xMap.transform( sample.x() );
            double yi = yMap.transform( sample.y() );

            if ( doAlign )
            {
                xi = qRound( xi );
                yi = qRound( yi );
            }

            QwtPainter::drawPoint( painter, QPointF( xi, yi ) );
        }
    }
    else
    {
        if ( doAlign )
        {
            const QPolygon points = mapper.toPoints(
                xMap, yMap, data(), from, to );

            QwtPainter::drawPoints( painter, points );
        }
        else
        {
            const QPolygonF points = mapper.toPointsF(
                xMap, yMap, data(), from, to );

            QwtPainter::drawPoints( painter, points );
        }
    }
}

/*!
   Draw step function

   The direction of the steps depends on Inverted attribute.

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from index of the first point to be painted
   \param to index of the last point to be painted

   \sa CurveAttribute, setCurveAttribute(),
      draw(), drawCurve(), drawDots(), drawLines(), drawSticks()
 */
void QwtPlotCurve::drawSteps( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    const bool doAlign = QwtPainter::roundingAlignment( painter );

    QPolygonF polygon( 2 * ( to - from ) + 1 );
    QPointF* points = polygon.data();

    bool inverted = orientation() == Qt::Vertical;
    if ( m_data->attributes & Inverted )
        inverted = !inverted;

    const QwtSeriesData< QPointF >* series = data();

    int i, ip;
    for ( i = from, ip = 0; i <= to; i++, ip += 2 )
    {
        const QPointF sample = series->sample( i );
        double xi = xMap.transform( sample.x() );
        double yi = yMap.transform( sample.y() );
        if ( doAlign )
        {
            xi = qRound( xi );
            yi = qRound( yi );
        }

        if ( ip > 0 )
        {
            const QPointF& p0 = points[ip - 2];
            QPointF& p = points[ip - 1];

            if ( inverted )
            {
                p.rx() = p0.x();
                p.ry() = yi;
            }
            else
            {
                p.rx() = xi;
                p.ry() = p0.y();
            }
        }

        points[ip].rx() = xi;
        points[ip].ry() = yi;
    }

    if ( m_data->paintAttributes & ClipPolygons )
    {
        QRectF clipRect = qwtIntersectedClipRect( canvasRect, painter );

        const qreal pw = QwtPainter::effectivePenWidth( painter->pen() );
        clipRect = clipRect.adjusted(-pw, -pw, pw, pw);

        const QPolygonF clipped = QwtClipper::clippedPolygonF(
            clipRect, polygon, false );

        QwtPainter::drawPolyline( painter, clipped );
    }
    else
    {
        QwtPainter::drawPolyline( painter, polygon );
    }

    if ( m_data->brush.style() != Qt::NoBrush )
        fillCurve( painter, xMap, yMap, canvasRect, polygon );
}


/*!
   Specify an attribute for drawing the curve

   \param attribute Curve attribute
   \param on On/Off

   /sa testCurveAttribute(), setCurveFitter()
 */
void QwtPlotCurve::setCurveAttribute( CurveAttribute attribute, bool on )
{
    if ( bool( m_data->attributes & attribute ) == on )
        return;

    if ( on )
        m_data->attributes |= attribute;
    else
        m_data->attributes &= ~attribute;

    itemChanged();
}

/*!
    \return true, if attribute is enabled
    \sa setCurveAttribute()
 */
bool QwtPlotCurve::testCurveAttribute( CurveAttribute attribute ) const
{
    return m_data->attributes & attribute;
}

/*!
   Assign a curve fitter

   The curve fitter "smooths" the curve points, when the Fitted
   CurveAttribute is set. setCurveFitter(NULL) also disables curve fitting.

   The curve fitter operates on the translated points ( = widget coordinates)
   to be functional for logarithmic scales. Obviously this is less performant
   for fitting algorithms, that reduce the number of points.

   For situations, where curve fitting is used to improve the performance
   of painting huge series of points it might be better to execute the fitter
   on the curve points once and to cache the result in the QwtSeriesData object.

   \param curveFitter() Curve fitter
   \sa Fitted
 */
void QwtPlotCurve::setCurveFitter( QwtCurveFitter* curveFitter )
{
    delete m_data->curveFitter;
    m_data->curveFitter = curveFitter;

    itemChanged();
}

/*!
   Get the curve fitter. If curve fitting is disabled NULL is returned.

   \return Curve fitter
   \sa setCurveFitter(), Fitted
 */
QwtCurveFitter* QwtPlotCurve::curveFitter() const
{
    return m_data->curveFitter;
}

/*!
   Fill the area between the curve and the baseline with
   the curve brush

   \param painter Painter
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param polygon Polygon - will be modified !

   \sa setBrush(), setBaseline(), setStyle()
 */
void QwtPlotCurve::fillCurve( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, QPolygonF& polygon ) const
{
    if ( m_data->brush.style() == Qt::NoBrush )
        return;

    closePolyline( painter, xMap, yMap, polygon );
    if ( polygon.count() <= 2 ) // a line can't be filled
        return;

    QBrush brush = m_data->brush;
    if ( !brush.color().isValid() )
        brush.setColor( m_data->pen.color() );

    if ( m_data->paintAttributes & ClipPolygons )
    {
        const QRectF clipRect = qwtIntersectedClipRect( canvasRect, painter );
        QwtClipper::clipPolygonF( clipRect, polygon, true );
    }

    painter->save();

    painter->setPen( Qt::NoPen );
    painter->setBrush( brush );

    QwtPainter::drawPolygon( painter, polygon );

    painter->restore();
}

/*!
   \brief Complete a polygon to be a closed polygon including the
         area between the original polygon and the baseline.

   \param painter Painter
   \param xMap X map
   \param yMap Y map
   \param polygon Polygon to be completed
 */
void QwtPlotCurve::closePolyline( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    QPolygonF& polygon ) const
{
    if ( polygon.size() < 2 )
        return;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    double baseline = m_data->baseline;

    if ( orientation() == Qt::Vertical )
    {
        if ( yMap.transformation() )
            baseline = yMap.transformation()->bounded( baseline );

        double refY = yMap.transform( baseline );
        if ( doAlign )
            refY = qRound( refY );

        polygon += QPointF( polygon.last().x(), refY );
        polygon += QPointF( polygon.first().x(), refY );
    }
    else
    {
        if ( xMap.transformation() )
            baseline = xMap.transformation()->bounded( baseline );

        double refX = xMap.transform( baseline );
        if ( doAlign )
            refX = qRound( refX );

        polygon += QPointF( refX, polygon.last().y() );
        polygon += QPointF( refX, polygon.first().y() );
    }
}

/*!
   Draw symbols

   \param painter Painter
   \param symbol Curve symbol
   \param xMap x map
   \param yMap y map
   \param canvasRect Contents rectangle of the canvas
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted

   \sa setSymbol(), drawSeries(), drawCurve()
 */
void QwtPlotCurve::drawSymbols( QPainter* painter, const QwtSymbol& symbol,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to ) const
{
    QwtPointMapper mapper;
    mapper.setFlag( QwtPointMapper::RoundPoints,
        QwtPainter::roundingAlignment( painter ) );
    mapper.setFlag( QwtPointMapper::WeedOutPoints,
        testPaintAttribute( QwtPlotCurve::FilterPoints ) );

    const QRectF clipRect = qwtIntersectedClipRect( canvasRect, painter );
    mapper.setBoundingRect( clipRect );

    const int chunkSize = 500;

    for ( int i = from; i <= to; i += chunkSize )
    {
        const int n = qMin( chunkSize, to - i + 1 );

        const QPolygonF points = mapper.toPointsF( xMap, yMap,
            data(), i, i + n - 1 );

        if ( points.size() > 0 )
            symbol.drawSymbols( painter, points );
    }
}

/*!
   \brief Set the value of the baseline

   The baseline is needed for filling the curve with a brush or
   the Sticks drawing style.

   The interpretation of the baseline depends on the orientation().
   With Qt::Vertical, the baseline is interpreted as a horizontal line
   at y = baseline(), with Qt::Horizontal, it is interpreted as a vertical
   line at x = baseline().

   The default value is 0.0.

   \param value Value of the baseline
   \sa baseline(), setBrush(), setStyle(), QwtPlotAbstractSeriesItem::orientation()
 */
void QwtPlotCurve::setBaseline( double value )
{
    if ( m_data->baseline != value )
    {
        m_data->baseline = value;
        itemChanged();
    }
}

/*!
   \return Value of the baseline
   \sa setBaseline()
 */
double QwtPlotCurve::baseline() const
{
    return m_data->baseline;
}

/*!
   Find the closest curve point for a specific position

   \param pos Position, where to look for the closest curve point
   \param dist If dist != NULL, closestPoint() returns the distance between
              the position and the closest curve point
   \return Index of the closest curve point, or -1 if none can be found
          ( f.e when the curve has no points )
   \note closestPoint() implements a dumb algorithm, that iterates
        over all points
 */
int QwtPlotCurve::closestPoint( const QPointF& pos, double* dist ) const
{
    const size_t numSamples = dataSize();

    if ( plot() == NULL || numSamples <= 0 )
        return -1;

    const QwtSeriesData< QPointF >* series = data();

    const QwtScaleMap xMap = plot()->canvasMap( xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( yAxis() );

    int index = -1;
    double dmin = 1.0e10;

    for ( uint i = 0; i < numSamples; i++ )
    {
        const QPointF sample = series->sample( i );

        const double cx = xMap.transform( sample.x() ) - pos.x();
        const double cy = yMap.transform( sample.y() ) - pos.y();

        const double f = qwtSqr( cx ) + qwtSqr( cy );
        if ( f < dmin )
        {
            index = i;
            dmin = f;
        }
    }
    if ( dist )
        *dist = std::sqrt( dmin );

    return index;
}

/*!
   \return Icon representing the curve on the legend

   \param index Index of the legend entry
                ( ignored as there is only one )
   \param size Icon size

   \sa QwtPlotItem::setLegendIconSize(), QwtPlotItem::legendData()
 */
QwtGraphic QwtPlotCurve::legendIcon( int index, const QSizeF& size ) const
{
    Q_UNUSED( index );

    if ( size.isEmpty() )
        return QwtGraphic();

    QwtGraphic graphic;
    graphic.setDefaultSize( size );
    graphic.setRenderHint( QwtGraphic::RenderPensUnscaled, true );

    QPainter painter( &graphic );
    painter.setRenderHint( QPainter::Antialiasing,
        testRenderHint( QwtPlotItem::RenderAntialiased ) );

    if ( m_data->legendAttributes == 0 ||
        m_data->legendAttributes & QwtPlotCurve::LegendShowBrush )
    {
        QBrush brush = m_data->brush;

        if ( brush.style() == Qt::NoBrush &&
            m_data->legendAttributes == 0 )
        {
            if ( style() != QwtPlotCurve::NoCurve )
            {
                brush = QBrush( pen().color() );
            }
            else if ( m_data->symbol &&
                ( m_data->symbol->style() != QwtSymbol::NoSymbol ) )
            {
                brush = QBrush( m_data->symbol->pen().color() );
            }
        }

        if ( brush.style() != Qt::NoBrush )
        {
            QRectF r( 0, 0, size.width(), size.height() );
            painter.fillRect( r, brush );
        }
    }

    if ( m_data->legendAttributes & QwtPlotCurve::LegendShowLine )
    {
        if ( pen() != Qt::NoPen )
        {
            QPen pn = pen();
            pn.setCapStyle( Qt::FlatCap );

            painter.setPen( pn );

            const double y = 0.5 * size.height();
            QwtPainter::drawLine( &painter, 0.0, y, size.width(), y );
        }
    }

    if ( m_data->legendAttributes & QwtPlotCurve::LegendShowSymbol )
    {
        if ( m_data->symbol )
        {
            QRectF r( 0, 0, size.width(), size.height() );
            m_data->symbol->drawSymbol( &painter, r );
        }
    }

    return graphic;
}

/*!
   Assign a series of points

   setSamples() is just a wrapper for setData() without any additional
   value - beside that it is easier to find for the developer.

   \param data Data
   \warning The item takes ownership of the data object, deleting
           it when its not used anymore.
 */
void QwtPlotCurve::setSamples( QwtSeriesData< QPointF >* data )
{
    setData( data );
}

/*!
   Initialize data with an array of points.

   \param samples Vector of points
   \note QVector is implicitly shared
   \note QPolygonF is derived from QVector<QPointF>
 */
void QwtPlotCurve::setSamples( const QVector< QPointF >& samples )
{
    setData( new QwtPointSeriesData( samples ) );
}

/*!
   \brief Initialize the data by pointing to memory blocks which
         are not managed by QwtPlotCurve.

   setRawSamples is provided for efficiency.
   It is important to keep the pointers
   during the lifetime of the underlying QwtCPointerData class.

   \param xData pointer to x data
   \param yData pointer to y data
   \param size size of x and y

   \sa QwtCPointerData
 */
void QwtPlotCurve::setRawSamples(
    const double* xData, const double* yData, int size )
{
    setData( new QwtCPointerData< double >( xData, yData, size ) );
}

/*!
   \brief Initialize the data by pointing to memory blocks which
         are not managed by QwtPlotCurve.

   setRawSamples is provided for efficiency.
   It is important to keep the pointers
   during the lifetime of the underlying QwtCPointerData class.

   \param xData pointer to x data
   \param yData pointer to y data
   \param size size of x and y

   \sa QwtCPointerData
 */
void QwtPlotCurve::setRawSamples(
    const float* xData, const float* yData, int size )
{
    setData( new QwtCPointerData< float >( xData, yData, size ) );
}

/*!
   \brief Initialize the data by pointing to a memory block which
         is not managed by QwtPlotCurve.

   The memory contains the y coordinates, while the index is
   interpreted as x coordinate.

   setRawSamples() is provided for efficiency. It is important to
   keep the pointers during the lifetime of the underlying
   QwtCPointerValueData class.

   \param yData pointer to y data
   \param size size of x and y

   \sa QwtCPointerData
 */
void QwtPlotCurve::setRawSamples( const double* yData, int size )
{
    setData( new QwtCPointerValueData< double >( yData, size ) );
}

/*!
   \brief Initialize the data by pointing to memory blocks which
         are not managed by QwtPlotCurve.

   The memory contains the y coordinates, while the index is
   interpreted as x coordinate.

   setRawSamples() is provided for efficiency. It is important to
   keep the pointers during the lifetime of the underlying
   QwtCPointerValueData class.

   \param yData pointer to y data
   \param size size of x and y

   \sa QwtCPointerData
 */
void QwtPlotCurve::setRawSamples( const float* yData, int size )
{
    setData( new QwtCPointerValueData< float >( yData, size ) );
}

/*!
   Set data by copying x- and y-values from specified memory blocks.
   Contrary to setRawSamples(), this function makes a 'deep copy' of
   the data.

   \param xData pointer to x values
   \param yData pointer to y values
   \param size size of xData and yData

   \sa QwtPointArrayData
 */
void QwtPlotCurve::setSamples(
    const double* xData, const double* yData, int size )
{
    setData( new QwtPointArrayData< double >( xData, yData, size ) );
}

/*!
   Set data by copying x- and y-values from specified memory blocks.
   Contrary to setRawSamples(), this function makes a 'deep copy' of
   the data.

   \param xData pointer to x values
   \param yData pointer to y values
   \param size size of xData and yData

   \sa QwtPointArrayData
 */
void QwtPlotCurve::setSamples(
    const float* xData, const float* yData, int size )
{
    setData( new QwtPointArrayData< float >( xData, yData, size ) );
}

/*!
   \brief Initialize data with x- and y-arrays (explicitly shared)

   \param xData x data
   \param yData y data

   \sa QwtPointArrayData
 */
void QwtPlotCurve::setSamples( const QVector< double >& xData,
    const QVector< double >& yData )
{
    setData( new QwtPointArrayData< double >( xData, yData ) );
}

/*!
   \brief Initialize data with x- and y-arrays (explicitly shared)

   \param xData x data
   \param yData y data

   \sa QwtPointArrayData
 */
void QwtPlotCurve::setSamples( const QVector< float >& xData,
    const QVector< float >& yData )
{
    setData( new QwtPointArrayData< float >( xData, yData ) );
}

/*!
   Set data by copying y-values from a specified memory block.

   The memory contains the y coordinates, while the index is
   interpreted as x coordinate.

   \param yData y data
   \param size size of yData

   \sa QwtValuePointData
 */
void QwtPlotCurve::setSamples( const double* yData, int size )
{
    setData( new QwtValuePointData< double >( yData, size ) );
}

/*!
   Set data by copying y-values from a specified memory block.

   The vector contains the y coordinates, while the index is
   interpreted as x coordinate.

   \param yData y data
   \param size size of yData

   \sa QwtValuePointData
 */
void QwtPlotCurve::setSamples( const float* yData, int size )
{
    setData( new QwtValuePointData< float >( yData, size ) );
}

/*!
   Initialize data with an array of y values (explicitly shared)

   The vector contains the y coordinates, while the index is
   the x coordinate.

   \param yData y data

   \sa QwtValuePointData
 */
void QwtPlotCurve::setSamples( const QVector< double >& yData )
{
    setData( new QwtValuePointData< double >( yData ) );
}

/*!
   Initialize data with an array of y values (explicitly shared)

   The vector contains the y coordinates, while the index is
   the x coordinate.

   \param yData y data

   \sa QwtValuePointData
 */
void QwtPlotCurve::setSamples( const QVector< float >& yData )
{
    setData( new QwtValuePointData< float >( yData ) );
}
