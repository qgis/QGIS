/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/
#include <qwt_global.h>

#if defined(QWT_VERSION) && QWT_VERSION<0x060000

#ifndef HISTOGRAM_ITEM_H
#define HISTOGRAM_ITEM_H

#include <qglobal.h>
#include <qcolor.h>

#include "qwt_plot_item.h"

class QwtIntervalData;
class QString;

class HistogramItem: public QwtPlotItem
{
  public:
    explicit HistogramItem( const QString &title = QString::null );
    explicit HistogramItem( const QwtText &title );
    virtual ~HistogramItem();

    void setData( const QwtIntervalData &data );
    const QwtIntervalData &data() const;

    void setColor( const QColor & );
    QColor color() const;

    void setFlat( bool flat );
    bool flat() const;

    void setSpacing( int spacing );
    int spacing() const;

    void setPen( const QPen& pen );
    QPen pen() const;

    virtual QwtDoubleRect boundingRect() const override;

    virtual int rtti() const override;

    virtual void draw( QPainter *, const QwtScaleMap &xMap,
                       const QwtScaleMap &yMap, const QRect & ) const override;

    virtual void updateLegend( QwtLegend * ) const override;

    void setBaseline( double reference );
    double baseline() const;

    enum HistogramAttribute
    {
      Auto = 0,
      Xfy = 1
    };

    void setHistogramAttribute( HistogramAttribute, bool on = true );
    bool testHistogramAttribute( HistogramAttribute ) const;

  protected:
    virtual void drawBar( QPainter *,
                          Qt::Orientation o, const QRect & ) const;

  private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#include <qstring.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_interval_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_legend_item.h>

class HistogramItem::PrivateData
{
  public:
    int attributes;
    QwtIntervalData data;
    QColor color;
    QPen pen;
    double reference;
    bool flat;
    int spacing;
};

#endif
#endif
