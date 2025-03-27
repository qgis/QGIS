/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_grid.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_scale_map.h"
#include "qwt_scale_div.h"

#include <qpainter.h>
#include <qpen.h>

static inline bool qwtFuzzyGreaterOrEqual( double d1, double d2 )
{
    return ( d1 >= d2 ) || qFuzzyCompare( d1, d2 );
}

class QwtPlotGrid::PrivateData
{
  public:
    PrivateData()
        : xEnabled( true )
        , yEnabled( true )
        , xMinEnabled( false )
        , yMinEnabled( false )
    {
    }

    bool xEnabled;
    bool yEnabled;
    bool xMinEnabled;
    bool yMinEnabled;

    QwtScaleDiv xScaleDiv;
    QwtScaleDiv yScaleDiv;

    QPen majorPen;
    QPen minorPen;
};

//! Enables major grid, disables minor grid
QwtPlotGrid::QwtPlotGrid()
    : QwtPlotItem( QwtText( "Grid" ) )
{
    m_data = new PrivateData;

    setItemInterest( QwtPlotItem::ScaleInterest, true );
    setZ( 10.0 );
}

//! Destructor
QwtPlotGrid::~QwtPlotGrid()
{
    delete m_data;
}

//! \return QwtPlotItem::Rtti_PlotGrid
int QwtPlotGrid::rtti() const
{
    return QwtPlotItem::Rtti_PlotGrid;
}

/*!
   \brief Enable or disable vertical grid lines
   \param on Enable (true) or disable

   \sa Minor grid lines can be enabled or disabled with
      enableXMin()
 */
void QwtPlotGrid::enableX( bool on )
{
    if ( m_data->xEnabled != on )
    {
        m_data->xEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/*!
   \brief Enable or disable horizontal grid lines
   \param on Enable (true) or disable
   \sa Minor grid lines can be enabled or disabled with enableYMin()
 */
void QwtPlotGrid::enableY( bool on )
{
    if ( m_data->yEnabled != on )
    {
        m_data->yEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/*!
   \brief Enable or disable  minor vertical grid lines.
   \param on Enable (true) or disable
   \sa enableX()
 */
void QwtPlotGrid::enableXMin( bool on )
{
    if ( m_data->xMinEnabled != on )
    {
        m_data->xMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/*!
   \brief Enable or disable minor horizontal grid lines
   \param on Enable (true) or disable
   \sa enableY()
 */
void QwtPlotGrid::enableYMin( bool on )
{
    if ( m_data->yMinEnabled != on )
    {
        m_data->yMinEnabled = on;

        legendChanged();
        itemChanged();
    }
}

/*!
   Assign an x axis scale division

   \param scaleDiv Scale division
 */
void QwtPlotGrid::setXDiv( const QwtScaleDiv& scaleDiv )
{
    if ( m_data->xScaleDiv != scaleDiv )
    {
        m_data->xScaleDiv = scaleDiv;
        itemChanged();
    }
}

/*!
   Assign a y axis division

   \param scaleDiv Scale division
 */
void QwtPlotGrid::setYDiv( const QwtScaleDiv& scaleDiv )
{
    if ( m_data->yScaleDiv != scaleDiv )
    {
        m_data->yScaleDiv = scaleDiv;
        itemChanged();
    }
}

/*!
   Build and assign a pen for both major and minor grid lines

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotGrid::setPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setPen( QPen( color, width, style ) );
}

/*!
   Assign a pen for both major and minor grid lines

   \param pen Pen
   \sa setMajorPen(), setMinorPen()
 */
void QwtPlotGrid::setPen( const QPen& pen )
{
    if ( m_data->majorPen != pen || m_data->minorPen != pen )
    {
        m_data->majorPen = pen;
        m_data->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   Build and assign a pen for both major grid lines

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotGrid::setMajorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMajorPen( QPen( color, width, style ) );
}

/*!
   Assign a pen for the major grid lines

   \param pen Pen
   \sa majorPen(), setMinorPen(), setPen()
 */
void QwtPlotGrid::setMajorPen( const QPen& pen )
{
    if ( m_data->majorPen != pen )
    {
        m_data->majorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   Build and assign a pen for the minor grid lines

   In Qt5 the default pen width is 1.0 ( 0.0 in Qt4 ) what makes it
   non cosmetic ( see QPen::isCosmetic() ). This method has been introduced
   to hide this incompatibility.

   \param color Pen color
   \param width Pen width
   \param style Pen style

   \sa pen(), brush()
 */
void QwtPlotGrid::setMinorPen( const QColor& color, qreal width, Qt::PenStyle style )
{
    setMinorPen( QPen( color, width, style ) );
}

/*!
   Assign a pen for the minor grid lines

   \param pen Pen
   \sa minorPen(), setMajorPen(), setPen()
 */
void QwtPlotGrid::setMinorPen( const QPen& pen )
{
    if ( m_data->minorPen != pen )
    {
        m_data->minorPen = pen;

        legendChanged();
        itemChanged();
    }
}

/*!
   \brief Draw the grid

   The grid is drawn into the bounding rectangle such that
   grid lines begin and end at the rectangle's borders. The X and Y
   maps are used to map the scale divisions into the drawing region
   screen.

   \param painter  Painter
   \param xMap X axis map
   \param yMap Y axis
   \param canvasRect Contents rectangle of the plot canvas
 */
void QwtPlotGrid::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    //  draw minor grid lines
    QPen minorPen = m_data->minorPen;
    minorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( minorPen );

    if ( m_data->xEnabled && m_data->xMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    if ( m_data->yEnabled && m_data->yMinEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MinorTick ) );
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MediumTick ) );
    }

    //  draw major grid lines
    QPen majorPen = m_data->majorPen;
    majorPen.setCapStyle( Qt::FlatCap );

    painter->setPen( majorPen );

    if ( m_data->xEnabled )
    {
        drawLines( painter, canvasRect, Qt::Vertical, xMap,
            m_data->xScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }

    if ( m_data->yEnabled )
    {
        drawLines( painter, canvasRect, Qt::Horizontal, yMap,
            m_data->yScaleDiv.ticks( QwtScaleDiv::MajorTick ) );
    }
}

void QwtPlotGrid::drawLines( QPainter* painter, const QRectF& canvasRect,
    Qt::Orientation orientation, const QwtScaleMap& scaleMap,
    const QList< double >& values ) const
{
    const double x1 = canvasRect.left();
    const double x2 = canvasRect.right() - 1.0;
    const double y1 = canvasRect.top();
    const double y2 = canvasRect.bottom() - 1.0;

    const bool doAlign = QwtPainter::roundingAlignment( painter );

    for ( int i = 0; i < values.count(); i++ )
    {
        double value = scaleMap.transform( values[i] );
        if ( doAlign )
            value = qRound( value );

        if ( orientation == Qt::Horizontal )
        {
            if ( qwtFuzzyGreaterOrEqual( value, y1 ) &&
                qwtFuzzyGreaterOrEqual( y2, value ) )
            {
                QwtPainter::drawLine( painter, x1, value, x2, value );
            }
        }
        else
        {
            if ( qwtFuzzyGreaterOrEqual( value, x1 ) &&
                qwtFuzzyGreaterOrEqual( x2, value ) )
            {
                QwtPainter::drawLine( painter, value, y1, value, y2 );
            }
        }
    }
}

/*!
   \return the pen for the major grid lines
   \sa setMajorPen(), setMinorPen(), setPen()
 */
const QPen& QwtPlotGrid::majorPen() const
{
    return m_data->majorPen;
}

/*!
   \return the pen for the minor grid lines
   \sa setMinorPen(), setMajorPen(), setPen()
 */
const QPen& QwtPlotGrid::minorPen() const
{
    return m_data->minorPen;
}

/*!
   \return true if vertical grid lines are enabled
   \sa enableX()
 */
bool QwtPlotGrid::xEnabled() const
{
    return m_data->xEnabled;
}

/*!
   \return true if minor vertical grid lines are enabled
   \sa enableXMin()
 */
bool QwtPlotGrid::xMinEnabled() const
{
    return m_data->xMinEnabled;
}

/*!
   \return true if horizontal grid lines are enabled
   \sa enableY()
 */
bool QwtPlotGrid::yEnabled() const
{
    return m_data->yEnabled;
}

/*!
   \return true if minor horizontal grid lines are enabled
   \sa enableYMin()
 */
bool QwtPlotGrid::yMinEnabled() const
{
    return m_data->yMinEnabled;
}


/*! \return the scale division of the x axis */
const QwtScaleDiv& QwtPlotGrid::xScaleDiv() const
{
    return m_data->xScaleDiv;
}

/*! \return the scale division of the y axis */
const QwtScaleDiv& QwtPlotGrid::yScaleDiv() const
{
    return m_data->yScaleDiv;
}

/*!
   Update the grid to changes of the axes scale division

   \param xScaleDiv Scale division of the x-axis
   \param yScaleDiv Scale division of the y-axis

   \sa QwtPlot::updateAxes()
 */
void QwtPlotGrid::updateScaleDiv( const QwtScaleDiv& xScaleDiv,
    const QwtScaleDiv& yScaleDiv )
{
    setXDiv( xScaleDiv );
    setYDiv( yScaleDiv );
}
