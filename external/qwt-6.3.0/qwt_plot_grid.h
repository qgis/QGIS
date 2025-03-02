/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_GRID_H
#define QWT_PLOT_GRID_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

class QPainter;
class QPen;
class QwtScaleMap;
class QwtScaleDiv;

/*!
   \brief A class which draws a coordinate grid

   The QwtPlotGrid class can be used to draw a coordinate grid.
   A coordinate grid consists of major and minor vertical
   and horizontal grid lines. The locations of the grid lines
   are determined by the X and Y scale divisions which can
   be assigned with setXDiv() and setYDiv().
   The draw() member draws the grid within a bounding
   rectangle.
 */

class QWT_EXPORT QwtPlotGrid : public QwtPlotItem
{
  public:
    explicit QwtPlotGrid();
    virtual ~QwtPlotGrid();

    virtual int rtti() const QWT_OVERRIDE;

    void enableX( bool );
    bool xEnabled() const;

    void enableY( bool );
    bool yEnabled() const;

    void enableXMin( bool );
    bool xMinEnabled() const;

    void enableYMin( bool );
    bool yMinEnabled() const;

    void setXDiv( const QwtScaleDiv& );
    const QwtScaleDiv& xScaleDiv() const;

    void setYDiv( const QwtScaleDiv& );
    const QwtScaleDiv& yScaleDiv() const;

    void setPen( const QColor&,
        qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    void setPen( const QPen& );

    void setMajorPen( const QColor&,
        qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );

    void setMajorPen( const QPen& );
    const QPen& majorPen() const;

    void setMinorPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setMinorPen( const QPen& );
    const QPen& minorPen() const;

    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const QWT_OVERRIDE;

    virtual void updateScaleDiv(
        const QwtScaleDiv& xScaleDiv, const QwtScaleDiv& yScaleDiv ) QWT_OVERRIDE;

  private:
    void drawLines( QPainter*, const QRectF&,
        Qt::Orientation, const QwtScaleMap&,
        const QList< double >& ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
