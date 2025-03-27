/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_zoneitem.h"
#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_text.h"
#include "qwt_interval.h"

#include <qpainter.h>

class QwtPlotZoneItem::PrivateData
{
  public:
    PrivateData()
        : orientation( Qt::Vertical )
        , pen( Qt::NoPen )
    {
        QColor c( Qt::darkGray );
        c.setAlpha( 100 );
        brush = QBrush( c );
    }

    Qt::Orientation orientation;
    QPen pen;
    QBrush brush;
    QwtInterval interval;
};

/*!
   \brief Constructor

   Initializes the zone with no pen and a semi transparent gray brush

   Sets the following item attributes:

   - QwtPlotItem::AutoScale: false
   - QwtPlotItem::Legend:    false

   The z value is initialized by 5

   \sa QwtPlotItem::setItemAttribute(), QwtPlotItem::setZ()
 */
QwtPlotZoneItem::QwtPlotZoneItem()
    : QwtPlotItem( QwtText( "Zone" ) )
{
    m_data = new PrivateData;

    setItemAttribute( QwtPlotItem::AutoScale, false );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 5 );
}

//! Destructor
QwtPlotZoneItem::~QwtPlotZoneItem()
{
    delete m_data;
}

//! \return QwtPlotItem::Rtti_PlotZone
int QwtPlotZoneItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotZone;
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
void QwtPlotZoneItem::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   \brief Assign a pen

   The pen is used to draw the border lines of the zone

   \param pen Pen
   \sa pen(), setBrush()
 */
void QwtPlotZoneItem::setPen( const QPen& pen )
{
    if ( m_data->pen != pen )
    {
        m_data->pen = pen;
        itemChanged();
    }
}

/*!
   \return Pen used to draw the border lines
   \sa setPen(), brush()
 */
const QPen& QwtPlotZoneItem::pen() const
{
    return m_data->pen;
}

/*!
   \brief Assign a brush

   The brush is used to fill the zone

   \param brush Brush
   \sa pen(), setBrush()
 */
void QwtPlotZoneItem::setBrush( const QBrush& brush )
{
    if ( m_data->brush != brush )
    {
        m_data->brush = brush;
        itemChanged();
    }
}

/*!
   \return Brush used to fill the zone
   \sa setPen(), brush()
 */
const QBrush& QwtPlotZoneItem::brush() const
{
    return m_data->brush;
}

/*!
   \brief Set the orientation of the zone

   A horizontal zone highlights an interval of the y axis,
   a vertical zone of the x axis. It is unbounded in the
   opposite direction.

   \sa orientation(), QwtPlotItem::setAxes()
 */
void QwtPlotZoneItem::setOrientation( Qt::Orientation orientation )
{
    if ( m_data->orientation != orientation )
    {
        m_data->orientation = orientation;
        itemChanged();
    }
}

/*!
   \return Orientation of the zone
   \sa setOrientation()
 */
Qt::Orientation QwtPlotZoneItem::orientation() const
{
    return m_data->orientation;
}

/*!
   Set the interval of the zone

   For a horizontal zone the interval is related to the y axis,
   for a vertical zone it is related to the x axis.

   \param min Minimum of the interval
   \param max Maximum of the interval

   \sa interval(), setOrientation()
 */
void QwtPlotZoneItem::setInterval( double min, double max )
{
    setInterval( QwtInterval( min, max ) );
}

/*!
   Set the interval of the zone

   For a horizontal zone the interval is related to the y axis,
   for a vertical zone it is related to the x axis.

   \param interval Zone interval

   \sa interval(), setOrientation()
 */
void QwtPlotZoneItem::setInterval( const QwtInterval& interval )
{
    if ( m_data->interval != interval )
    {
        m_data->interval = interval;
        itemChanged();
    }
}

/*!
   \return Zone interval
   \sa setInterval(), orientation()
 */
QwtInterval QwtPlotZoneItem::interval() const
{
    return m_data->interval;
}

/*!
   Draw the zone

   \param painter Painter
   \param xMap x Scale Map
   \param yMap y Scale Map
   \param canvasRect Contents rectangle of the canvas in painter coordinates
 */

void QwtPlotZoneItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    if ( !m_data->interval.isValid() )
        return;

    QPen pen = m_data->pen;
    pen.setCapStyle( Qt::FlatCap );

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    if ( m_data->orientation == Qt::Horizontal )
    {
        double y1 = yMap.transform( m_data->interval.minValue() );
        double y2 = yMap.transform( m_data->interval.maxValue() );

        if ( doAlign )
        {
            y1 = qRound( y1 );
            y2 = qRound( y2 );
        }

        QRectF r( canvasRect.left(), y1, canvasRect.width(), y2 - y1 );
        r = r.normalized();

        if ( ( m_data->brush.style() != Qt::NoBrush ) && ( y1 != y2 ) )
        {
            QwtPainter::fillRect( painter, r, m_data->brush );
        }

        if ( m_data->pen.style() != Qt::NoPen )
        {
            painter->setPen( m_data->pen );

            QwtPainter::drawLine( painter, r.left(), r.top(), r.right(), r.top() );
            QwtPainter::drawLine( painter, r.left(), r.bottom(), r.right(), r.bottom() );
        }
    }
    else
    {
        double x1 = xMap.transform( m_data->interval.minValue() );
        double x2 = xMap.transform( m_data->interval.maxValue() );

        if ( doAlign )
        {
            x1 = qRound( x1 );
            x2 = qRound( x2 );
        }

        QRectF r( x1, canvasRect.top(), x2 - x1, canvasRect.height() );
        r = r.normalized();

        if ( ( m_data->brush.style() != Qt::NoBrush ) && ( x1 != x2 ) )
        {
            QwtPainter::fillRect( painter, r, m_data->brush );
        }

        if ( m_data->pen.style() != Qt::NoPen )
        {
            painter->setPen( m_data->pen );

            QwtPainter::drawLine( painter, r.left(), r.top(), r.left(), r.bottom() );
            QwtPainter::drawLine( painter, r.right(), r.top(), r.right(), r.bottom() );
        }
    }
}

/*!
   The bounding rectangle is build from the interval in one direction
   and something invalid for the opposite direction.

   \return An invalid rectangle with valid boundaries in one direction
 */
QRectF QwtPlotZoneItem::boundingRect() const
{
    QRectF br = QwtPlotItem::boundingRect();

    const QwtInterval& intv = m_data->interval;

    if ( intv.isValid() )
    {
        if ( m_data->orientation == Qt::Horizontal )
        {
            br.setTop( intv.minValue() );
            br.setBottom( intv.maxValue() );
        }
        else
        {
            br.setLeft( intv.minValue() );
            br.setRight( intv.maxValue() );
        }
    }

    return br;
}
