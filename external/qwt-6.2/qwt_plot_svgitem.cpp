/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_svgitem.h"
#include "qwt_text.h"
#include "qwt_graphic.h"

#include <qsvgrenderer.h>

/*!
   \brief Constructor
   \param title Title
 */
QwtPlotSvgItem::QwtPlotSvgItem( const QString& title )
    : QwtPlotGraphicItem( QwtText( title ) )
{
}

/*!
   \brief Constructor
   \param title Title
 */
QwtPlotSvgItem::QwtPlotSvgItem( const QwtText& title )
    : QwtPlotGraphicItem( title )
{
}

//! Destructor
QwtPlotSvgItem::~QwtPlotSvgItem()
{
}

/*!
   Load a SVG file

   \param rect Bounding rectangle
   \param fileName SVG file name

   \return true, if the SVG file could be loaded
 */
bool QwtPlotSvgItem::loadFile( const QRectF& rect,
    const QString& fileName )
{
    QwtGraphic graphic;

    QSvgRenderer renderer;

    const bool ok = renderer.load( fileName );
    if ( ok )
    {
        QPainter p( &graphic );
        renderer.render( &p );
    }

    setGraphic( rect, graphic );

    return ok;
}

/*!
   Load SVG data

   \param rect Bounding rectangle
   \param data in SVG format

   \return true, if the SVG data could be loaded
 */
bool QwtPlotSvgItem::loadData( const QRectF& rect,
    const QByteArray& data )
{
    QwtGraphic graphic;

    QSvgRenderer renderer;

    const bool ok = renderer.load( data );
    if ( ok )
    {
        QPainter p( &graphic );
        renderer.render( &p );
    }

    setGraphic( rect, graphic );

    return ok;
}
