/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_curve.h"
#include <qwt_painter.h>
#include <qwt_polar.h>
#include <qwt_scale_map.h>
#include <qwt_math.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_curve_fitter.h>
#include <qwt_clipper.h>
#include <qpainter.h>

static inline bool qwtInsidePole( const QwtScaleMap &map, double radius )
{
    return map.isInverting() ? ( radius > map.s1() ) : ( radius < map.s1() );
}

static int qwtVerifyRange( int size, int &i1, int &i2 )
{
    if ( size < 1 )
        return 0;

    i1 = qBound( 0, i1, size - 1 );
    i2 = qBound( 0, i2, size - 1 );

    if ( i1 > i2 )
        qSwap( i1, i2 );

    return ( i2 - i1 + 1 );
}

class QwtPolarCurve::PrivateData
{
public:
    PrivateData():
        style( QwtPolarCurve::Lines ),
        curveFitter( NULL ),
        legendAttributes( 0 )
    {
        symbol = new QwtSymbol();
        pen = QPen( Qt::black );
    }

    ~PrivateData()
    {
        delete symbol;
        delete curveFitter;
    }

    QwtPolarCurve::CurveStyle style;
    const QwtSymbol *symbol;
    QPen pen;
    QwtCurveFitter *curveFitter;

    QwtPolarCurve::LegendAttributes legendAttributes;
};

//! Constructor
QwtPolarCurve::QwtPolarCurve():
    QwtPolarItem( QwtText() )
{
    init();
}

/*!
  Constructor
  \param title title of the curve
*/
QwtPolarCurve::QwtPolarCurve( const QwtText &title ):
    QwtPolarItem( title )
{
    init();
}

/*!
  Constructor
  \param title title of the curve
*/
QwtPolarCurve::QwtPolarCurve( const QString &title ):
    QwtPolarItem( QwtText( title ) )
{
    init();
}

//! Destructor
QwtPolarCurve::~QwtPolarCurve()
{
    delete d_series;
    delete d_data;
}

//! Initialize data members
void QwtPolarCurve::init()
{
    d_data = new PrivateData;
    d_series = NULL;

    setItemAttribute( QwtPolarItem::AutoScale );
    setItemAttribute( QwtPolarItem::Legend );
    setZ( 20.0 );

    setRenderHint( RenderAntialiased, true );
}

//! \return QwtPolarCurve::Rtti_PolarCurve
int QwtPolarCurve::rtti() const
{
    return QwtPolarItem::Rtti_PolarCurve;
}

/*!
  Specify an attribute how to draw the legend identifier

  \param attribute Attribute
  \param on On/Off
  /sa LegendAttribute, testLegendAttribute()
*/
void QwtPolarCurve::setLegendAttribute( LegendAttribute attribute, bool on )
{
    if ( on )
        d_data->legendAttributes |= attribute;
    else
        d_data->legendAttributes &= ~attribute;
}

/*!
    \brief Test if a lefend attribute is enables

    \param attribute Legend attribute

    \return True if attribute is enabled
    \sa LegendAttribute, setLegendAttribute()
*/
bool QwtPolarCurve::testLegendAttribute( LegendAttribute attribute ) const
{
    return ( d_data->legendAttributes & attribute );
}

/*!
  Set the curve's drawing style

  \param style Curve style
  \sa CurveStyle, style()
*/
void QwtPolarCurve::setStyle( CurveStyle style )
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

/*!
    \return Current style
    \sa CurveStyle, setStyle()
*/
QwtPolarCurve::CurveStyle QwtPolarCurve::style() const
{
    return d_data->style;
}

/*!
  \brief Assign a symbol
  \param symbol Symbol
  \sa symbol()
*/
void QwtPolarCurve::setSymbol( const QwtSymbol *symbol )
{
    if ( symbol != d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol;
        itemChanged();
    }
}

/*!
    \return The current symbol
    \sa setSymbol()
*/
const QwtSymbol *QwtPolarCurve::symbol() const
{
    return d_data->symbol;
}

/*!
  \brief Assign a pen
  \param pen New pen
  \sa pen()
*/
void QwtPolarCurve::setPen( const QPen &pen )
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

/*!
    \return Pen used to draw the lines
    \sa setPen()
*/
const QPen& QwtPolarCurve::pen() const
{
    return d_data->pen;
}

/*!
  Initialize data with a pointer to QwtSeriesData<QwtPointPolar>.

  The x-values of the data object represent the azimuth,
  the y-value respresent the radius.

  \param data Data
*/
void QwtPolarCurve::setData( QwtSeriesData<QwtPointPolar> *data )
{
    if ( d_series != data )
    {
        delete d_series;
        d_series = data;
        itemChanged();
    }
}

/*!
  \brief Insert a curve fitter

  \param curveFitter Curve fitter

  A curve fitter interpolates the curve points. F.e QwtPolarFitter
  adds equidistant points so that the connection gets rounded instead
  of having straight lines. If curveFitter is NULL fitting is disabled.

  \sa curveFitter()
*/
void QwtPolarCurve::setCurveFitter( QwtCurveFitter *curveFitter )
{
    if ( curveFitter != d_data->curveFitter )
    {
        delete d_data->curveFitter;
        d_data->curveFitter = curveFitter;

        itemChanged();
    }
}

/*!
  \return The curve fitter
  \sa setCurveFitter()
*/
QwtCurveFitter *QwtPolarCurve::curveFitter() const
{
    return d_data->curveFitter;
}

/*!
  Draw the curve

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param radius Radius of the complete plot area in painter coordinates
  \param canvasRect Contents rect of the canvas in painter coordinates
*/
void QwtPolarCurve::draw( QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, double radius,
    const QRectF &canvasRect ) const
{
    Q_UNUSED( radius );
    Q_UNUSED( canvasRect );

    draw( painter, azimuthMap, radialMap, pole, 0, -1 );
}

/*!
  \brief Draw an interval of the curve
  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted. If to < 0 the
         curve will be painted to its last point.

  \sa drawCurve(), drawSymbols(),
*/
void QwtPolarCurve::draw( QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to ) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if ( to < 0 )
        to = dataSize() - 1;

    if ( qwtVerifyRange( dataSize(), from, to ) > 0 )
    {
        painter->save();
        painter->setPen( d_data->pen );

        drawCurve( painter, d_data->style,
            azimuthMap, radialMap, pole, from, to );

        painter->restore();

        if ( d_data->symbol->style() != QwtSymbol::NoSymbol )
        {
            painter->save();
            drawSymbols( painter, *d_data->symbol,
                azimuthMap, radialMap, pole, from, to );
            painter->restore();
        }
    }
}

/*!
  Draw the line part (without symbols) of a curve interval.

  \param painter Painter
  \param style Curve style, see QwtPolarCurve::CurveStyle
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted.
  \sa draw(), drawLines()
*/
void QwtPolarCurve::drawCurve( QPainter *painter, int style,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to ) const
{
    switch ( style )
    {
        case Lines:
            drawLines( painter, azimuthMap, radialMap, pole, from, to );
            break;
        case NoCurve:
        default:
            break;
    }
}

/*!
  Draw lines

  \param painter Painter
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted.
  \sa draw(), drawLines(), setCurveFitter()
*/
void QwtPolarCurve::drawLines( QPainter *painter,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to ) const
{
    int size = to - from + 1;
    if ( size <= 0 )
        return;

    QPolygonF polyline;
    if ( d_data->curveFitter )
    {
        QPolygonF points( size );
        for ( int j = from; j <= to; j++ )
        {
            const QwtPointPolar point = sample( j );
            points[j - from] = QPointF( point.azimuth(), point.radius() );
        }

        points = d_data->curveFitter->fitCurve( points );

        polyline.resize( points.size() );

        QPointF *polylineData = polyline.data();
        QPointF *pointsData = polyline.data();

        for ( int i = 0; i < points.size(); i++ )
        {
            const QwtPointPolar point( pointsData[i].x(), pointsData[i].y() );

            double r = radialMap.transform( point.radius() );
            const double a = azimuthMap.transform( point.azimuth() );

            polylineData[i] = qwtPolar2Pos( pole, r, a );
        }
    }
    else
    {
        polyline.resize( size );
        QPointF *polylineData = polyline.data();

        for ( int i = from; i <= to; i++ )
        {
            QwtPointPolar point = sample( i );
            if ( !qwtInsidePole( radialMap, point.radius() ) )
            {
                double r = radialMap.transform( point.radius() );
                const double a = azimuthMap.transform( point.azimuth() );
                polylineData[i - from] = qwtPolar2Pos( pole, r, a );
            }
            else
            {
                polylineData[i - from] = pole;
            }
        }
    }

    QRectF clipRect;
    if ( painter->hasClipping() )
        clipRect = painter->clipRegion().boundingRect();
    else
    {
        clipRect = painter->window();
        if ( !clipRect.isEmpty() )
            clipRect = painter->transform().inverted().mapRect( clipRect );
    }

    if ( !clipRect.isEmpty() )
    {
        double off = qCeil( qMax((qreal)1.0,painter->pen().widthF() ) );
        clipRect = clipRect.toRect().adjusted( -off, -off, off, off );
        polyline = QwtClipper::clipPolygonF( clipRect, polyline );
    }

    QwtPainter::drawPolyline( painter, polyline );
    painter->drawPolyline( polyline );
}

/*!
  Draw symbols

  \param painter Painter
  \param symbol Curve symbol
  \param azimuthMap Maps azimuth values to values related to 0.0, M_2PI
  \param radialMap Maps radius values into painter coordinates.
  \param pole Position of the pole in painter coordinates
  \param from index of the first point to be painted
  \param to index of the last point to be painted.

  \sa setSymbol(), draw(), drawCurve()
*/
void QwtPolarCurve::drawSymbols( QPainter *painter, const QwtSymbol &symbol,
    const QwtScaleMap &azimuthMap, const QwtScaleMap &radialMap,
    const QPointF &pole, int from, int to ) const
{
    painter->setBrush( symbol.brush() );
    painter->setPen( symbol.pen() );

    const int chunkSize = 500;

    for ( int i = from; i <= to; i += chunkSize )
    {
        const int n = qMin( chunkSize, to - i + 1 );

        QPolygonF points;
        for ( int j = 0; j < n; j++ )
        {
            const QwtPointPolar point = sample( i + j );

            if ( !qwtInsidePole( radialMap, point.radius() ) )
            {
                const double r = radialMap.transform( point.radius() );
                const double a = azimuthMap.transform( point.azimuth() );

                points += qwtPolar2Pos( pole, r, a );
            }
            else
            {
                points += pole;
            }
        }

        if ( points.size() > 0 )
            symbol.drawSymbols( painter, points );
    }
}

/*!
  \return Number of points
  \sa setData()
*/
size_t QwtPolarCurve::dataSize() const
{
    return d_series->size();
}

//!  Update the widget that represents the curve on the legend
void QwtPolarCurve::updateLegend( QwtLegend *legend ) const
{
    if ( legend && testItemAttribute( QwtPolarCurve::Legend )
            && ( d_data->legendAttributes & QwtPolarCurve::LegendShowSymbol )
            && d_data->symbol
            && d_data->symbol->style() != QwtSymbol::NoSymbol )
    {
        QWidget *lgdItem = legend->find( this );
        if ( lgdItem == NULL )
        {
            lgdItem = legendItem();
            if ( lgdItem )
                legend->insert( this, lgdItem );
        }

        QwtLegendItem *l = qobject_cast<QwtLegendItem *>( lgdItem );
        if ( l )
        {
            QSize sz = d_data->symbol->boundingSize();
            sz += QSize( 2, 2 ); // margin

            if ( d_data->legendAttributes & QwtPolarCurve::LegendShowLine )
            {
                // Avoid, that the line is completely covered by the symbol

                int w = qCeil( 1.5 * sz.width() );
                if ( w % 2 )
                    w++;

                sz.setWidth( qMax( 8, w ) );
            }

            l->setIdentifierSize( sz );
        }
    }

    QwtPolarItem::updateLegend( legend );
}

/*!
  \brief Draw the identifier representing the curve on the legend

  \param painter Qt Painter
  \param rect Bounding rectangle for the identifier

  \sa setLegendAttribute
*/
void QwtPolarCurve::drawLegendIdentifier(
    QPainter *painter, const QRectF &rect ) const
{
    if ( rect.isEmpty() )
        return;

    const double dim = qMin( rect.width(), rect.height() );

    QSizeF size( dim, dim );

    QRectF r( 0, 0, size.width(), size.height() );
    r.moveCenter( rect.center() );

    if ( d_data->legendAttributes == 0 )
    {
        QBrush brush;
        if ( style() != QwtPolarCurve::NoCurve )
            brush = QBrush( pen().color() );
        else if ( d_data->symbol &&
            ( d_data->symbol->style() != QwtSymbol::NoSymbol ) )
        {
            brush = QBrush( d_data->symbol->pen().color() );
        }
        if ( brush.style() != Qt::NoBrush )
            painter->fillRect( r, brush );
    }
    if ( d_data->legendAttributes & QwtPolarCurve::LegendShowLine )
    {
        if ( pen() != Qt::NoPen )
        {
            painter->setPen( pen() );
            QwtPainter::drawLine( painter, rect.left(), rect.center().y(),
                rect.right() - 1.0, rect.center().y() );
        }
    }
    if ( d_data->legendAttributes & QwtPolarCurve::LegendShowSymbol )
    {
        if ( d_data->symbol &&
            ( d_data->symbol->style() != QwtSymbol::NoSymbol ) )
        {
            QSize symbolSize = d_data->symbol->boundingSize();
            symbolSize -= QSize( 2, 2 );

            // scale the symbol size down if it doesn't fit into rect.

            double xRatio = 1.0;
            if ( rect.width() < symbolSize.width() )
                xRatio = rect.width() / symbolSize.width();
            double yRatio = 1.0;
            if ( rect.height() < symbolSize.height() )
                yRatio = rect.height() / symbolSize.height();

            const double ratio = qMin( xRatio, yRatio );

            painter->save();
            painter->scale( ratio, ratio );

            d_data->symbol->drawSymbol( painter, rect.center() / ratio );

            painter->restore();
        }
    }
}

/*!
   Interval, that is necessary to display the item
   This interval can be useful for operations like clipping or autoscaling

   \param scaleId Scale index
   \return bounding interval

   \sa QwtData::boundingRect()
*/
QwtInterval QwtPolarCurve::boundingInterval( int scaleId ) const
{
    const QRectF boundingRect = d_series->boundingRect();

    if ( scaleId == QwtPolar::ScaleAzimuth )
        return QwtInterval( boundingRect.left(), boundingRect.right() );

    if ( scaleId == QwtPolar::ScaleRadius )
        return QwtInterval( boundingRect.top(), boundingRect.bottom() );

    return QwtInterval();
}
