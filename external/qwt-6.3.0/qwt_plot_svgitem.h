/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SVG_ITEM_H
#define QWT_PLOT_SVG_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_graphicitem.h"

class QByteArray;

/*!
   \brief A plot item, which displays
         data in Scalable Vector Graphics (SVG) format.

   SVG images are often used to display maps

   QwtPlotSvgItem is only a small convenience wrapper class for
   QwtPlotGraphicItem, that creates a QwtGraphic from SVG data.
 */

class QWT_EXPORT QwtPlotSvgItem : public QwtPlotGraphicItem
{
  public:
    explicit QwtPlotSvgItem( const QString& title = QString() );
    explicit QwtPlotSvgItem( const QwtText& title );
    virtual ~QwtPlotSvgItem();

    bool loadFile( const QRectF&, const QString& fileName );
    bool loadData( const QRectF&, const QByteArray& );
};

#endif
