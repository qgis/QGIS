/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_graphicitem.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

class QwtPlotGraphicItem::PrivateData
{
  public:
    QRectF boundingRect;
    QwtGraphic graphic;
};

/*!
   \brief Constructor

   Sets the following item attributes:
   - QwtPlotItem::AutoScale: true
   - QwtPlotItem::Legend:    false

   \param title Title
 */
QwtPlotGraphicItem::QwtPlotGraphicItem( const QString& title )
    : QwtPlotItem( QwtText( title ) )
{
    init();
}

/*!
   \brief Constructor

   Sets the following item attributes:
   - QwtPlotItem::AutoScale: true
   - QwtPlotItem::Legend:    false

   \param title Title
 */
QwtPlotGraphicItem::QwtPlotGraphicItem( const QwtText& title )
    : QwtPlotItem( title )
{
    init();
}

//! Destructor
QwtPlotGraphicItem::~QwtPlotGraphicItem()
{
    delete m_data;
}

void QwtPlotGraphicItem::init()
{
    m_data = new PrivateData();
    m_data->boundingRect = QwtPlotItem::boundingRect();

    setItemAttribute( QwtPlotItem::AutoScale, true );
    setItemAttribute( QwtPlotItem::Legend, false );

    setZ( 8.0 );
}

//! \return QwtPlotItem::Rtti_PlotGraphic
int QwtPlotGraphicItem::rtti() const
{
    return QwtPlotItem::Rtti_PlotGraphic;
}

/*!
   Set the graphic to be displayed

   \param rect Rectangle in plot coordinates
   \param graphic Recorded sequence of painter commands
 */
void QwtPlotGraphicItem::setGraphic(
    const QRectF& rect, const QwtGraphic& graphic )
{
    m_data->boundingRect = rect;
    m_data->graphic = graphic;

    legendChanged();
    itemChanged();
}

/*!
   \return Recorded sequence of painter commands
   \sa setGraphic()
 */
QwtGraphic QwtPlotGraphicItem::graphic() const
{
    return m_data->graphic;
}

//! Bounding rectangle of the item
QRectF QwtPlotGraphicItem::boundingRect() const
{
    return m_data->boundingRect;
}

/*!
   Draw the item

   \param painter Painter
   \param xMap X-Scale Map
   \param yMap Y-Scale Map
   \param canvasRect Contents rect of the plot canvas
 */
void QwtPlotGraphicItem::draw( QPainter* painter,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect ) const
{
    if ( m_data->graphic.isEmpty() )
        return;

    QRectF r = QwtScaleMap::transform( xMap, yMap, boundingRect() );

    if ( !r.intersects( canvasRect ) )
        return;

    if ( QwtPainter::roundingAlignment( painter ) )
    {
        r.setLeft ( qRound( r.left() ) );
        r.setRight ( qRound( r.right() ) );
        r.setTop ( qRound( r.top() ) );
        r.setBottom ( qRound( r.bottom() ) );
    }

    m_data->graphic.render( painter, r );
}
