/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_DIRECT_PAINTER_H
#define QWT_PLOT_DIRECT_PAINTER_H

#include "qwt_global.h"
#include <qobject.h>

class QRegion;
class QwtPlotSeriesItem;

/*!
    \brief Painter object trying to paint incrementally

    Often applications want to display samples while they are
    collected. When there are too many samples complete replots
    will be expensive to be processed in a collection cycle.

    QwtPlotDirectPainter offers an API to paint
    subsets ( f.e all additions points ) without erasing/repainting
    the plot canvas.

    On certain environments it might be important to calculate a proper
    clip region before painting. F.e. for Qt Embedded only the clipped part
    of the backing store will be copied to a ( maybe unaccelerated )
    frame buffer.

    \warning Incremental painting will only help when no replot is triggered
             by another operation ( like changing scales ) and nothing needs
             to be erased.
 */
class QWT_EXPORT QwtPlotDirectPainter : public QObject
{
  public:
    /*!
       \brief Paint attributes
       \sa setAttribute(), testAttribute(), drawSeries()
     */
    enum Attribute
    {
        /*!
           Initializing a QPainter is an expensive operation.
           When AtomicPainter is set each call of drawSeries() opens/closes
           a temporary QPainter. Otherwise QwtPlotDirectPainter tries to
           use the same QPainter as long as possible.
         */
        AtomicPainter = 0x01,

        /*!
           When FullRepaint is set the plot canvas is explicitly repainted
           after the samples have been rendered.
         */
        FullRepaint = 0x02,

        /*!
           When QwtPlotCanvas::BackingStore is enabled the painter
           has to paint to the backing store and the widget. In certain
           situations/environments it might be faster to paint to
           the backing store only and then copy the backing store to the canvas.
           This flag can also be useful for settings, where Qt fills the
           the clip region with the widget background.
         */
        CopyBackingStore = 0x04
    };

    Q_DECLARE_FLAGS( Attributes, Attribute )

    explicit QwtPlotDirectPainter( QObject* parent = NULL );
    virtual ~QwtPlotDirectPainter();

    void setAttribute( Attribute, bool on );
    bool testAttribute( Attribute ) const;

    void setClipping( bool );
    bool hasClipping() const;

    void setClipRegion( const QRegion& );
    QRegion clipRegion() const;

    void drawSeries( QwtPlotSeriesItem*, int from, int to );
    void reset();

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotDirectPainter::Attributes )

#endif
