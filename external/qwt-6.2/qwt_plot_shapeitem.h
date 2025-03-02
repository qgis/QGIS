/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SHAPE_ITEM_H
#define QWT_PLOT_SHAPE_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"

#include <qstring.h>

class QPainterPath;
class QPolygonF;

/*!
   \brief A plot item, which displays any graphical shape,
         that can be defined by a QPainterPath

   A QPainterPath is a shape composed from intersecting and uniting
   regions, rectangles, ellipses or irregular areas defined by lines, and curves.
   QwtPlotShapeItem displays a shape with a pen and brush.

   QwtPlotShapeItem offers a couple of optimizations like clipping or weeding.
   These algorithms need to convert the painter path into polygons that might be
   less performant for paths built from curves and ellipses.

   More complex shapes, that can't be expressed by a QPainterPath can be displayed
   using QwtPlotGraphicItem.

   \sa QwtPlotZone, QwtPlotGraphicItem
 */
class QWT_EXPORT QwtPlotShapeItem : public QwtPlotItem
{
  public:
    /*!
        Attributes to modify the drawing algorithm.
        The default disables all attributes

        \sa setPaintAttribute(), testPaintAttribute()
     */
    enum PaintAttribute
    {
        /*!
           Clip polygons before painting them. In situations, where points
           are far outside the visible area (f.e when zooming deep) this
           might be a substantial improvement for the painting performance

           But polygon clipping will convert the painter path into
           polygons what might introduce a negative impact on the
           performance of paths composed from curves or ellipses.
         */
        ClipPolygons = 0x01,
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    //! Mode how to display the item on the legend
    enum LegendMode
    {
        //! Display a scaled down version of the shape
        LegendShape,

        //! Display a filled rectangle
        LegendColor
    };

    explicit QwtPlotShapeItem( const QString& title = QString() );
    explicit QwtPlotShapeItem( const QwtText& title );

    virtual ~QwtPlotShapeItem();

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setLegendMode( LegendMode );
    LegendMode legendMode() const;

    void setRect( const QRectF& );
    void setPolygon( const QPolygonF& );

    void setShape( const QPainterPath& );
    QPainterPath shape() const;

    void setPen( const QColor&, qreal width = 0.0, Qt::PenStyle = Qt::SolidLine );
    void setPen( const QPen& );
    QPen pen() const;

    void setBrush( const QBrush& );
    QBrush brush() const;

    void setRenderTolerance( double );
    double renderTolerance() const;

    virtual QRectF boundingRect() const QWT_OVERRIDE;

    virtual void draw( QPainter*,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect ) const QWT_OVERRIDE;

    virtual QwtGraphic legendIcon(
        int index, const QSizeF& ) const QWT_OVERRIDE;

    virtual int rtti() const QWT_OVERRIDE;

  private:
    void init();

    class PrivateData;
    PrivateData* m_data;
};

#endif
