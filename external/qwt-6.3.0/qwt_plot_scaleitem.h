/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SCALE_ITEM_H
#define QWT_PLOT_SCALE_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"
#include "qwt_scale_draw.h"

class QPalette;

/*!
   \brief A class which draws a scale inside the plot canvas

   QwtPlotScaleItem can be used to draw an axis inside the plot canvas.
   It might by synchronized to one of the axis of the plot, but can
   also display its own ticks and labels.

   It is allowed to synchronize the scale item with a disabled axis.
   In plots with vertical and horizontal scale items, it might be
   necessary to remove ticks at the intersections, by overloading
   updateScaleDiv().

   The scale might be at a specific position (f.e 0.0) or it might be
   aligned to a canvas border.

   \par Example
    The following example shows how to replace the left axis, by a scale item
    at the x position 0.0.
    \code
      QwtPlotScaleItem *scaleItem = new QwtPlotScaleItem( QwtScaleDraw::RightScale, 0.0 );
      scaleItem->setFont( plot->axisWidget( QwtAxis::YLeft )->font() );
      scaleItem->attach(plot);

      plot->setAxisVisible( QwtAxis::YLeft, false );
    \endcode
 */

class QWT_EXPORT QwtPlotScaleItem : public QwtPlotItem
{
  public:
    explicit QwtPlotScaleItem(
        QwtScaleDraw::Alignment = QwtScaleDraw::BottomScale,
        const double pos = 0.0 );

    virtual ~QwtPlotScaleItem();

    virtual int rtti() const QWT_OVERRIDE;

    void setScaleDiv( const QwtScaleDiv& );
    const QwtScaleDiv& scaleDiv() const;

    void setScaleDivFromAxis( bool on );
    bool isScaleDivFromAxis() const;

    void setPalette( const QPalette& );
    QPalette palette() const;

    void setFont( const QFont& );
    QFont font() const;

    void setScaleDraw( QwtScaleDraw* );

    const QwtScaleDraw* scaleDraw() const;
    QwtScaleDraw* scaleDraw();

    void setPosition( double pos );
    double position() const;

    void setBorderDistance( int );
    int borderDistance() const;

    void setAlignment( QwtScaleDraw::Alignment );

    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const QWT_OVERRIDE;

    virtual void updateScaleDiv(
        const QwtScaleDiv&, const QwtScaleDiv& ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
