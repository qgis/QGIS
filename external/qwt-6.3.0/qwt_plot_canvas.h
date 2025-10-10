/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_CANVAS_H
#define QWT_PLOT_CANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"

#include <qframe.h>

class QwtPlot;
class QPixmap;
class QPainterPath;

/*!
   \brief Canvas of a QwtPlot.

   Canvas is the widget where all plot items are displayed

   \sa QwtPlot::setCanvas(), QwtPlotGLCanvas, QwtPlotOpenGLCanvas
 */
class QWT_EXPORT QwtPlotCanvas : public QFrame, public QwtPlotAbstractCanvas
{
    Q_OBJECT

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )

  public:

    /*!
       \brief Paint attributes

       The default setting enables BackingStore and Opaque.

       \sa setPaintAttribute(), testPaintAttribute()
     */
    enum PaintAttribute
    {
        /*!
           \brief Paint double buffered reusing the content
                 of the pixmap buffer when possible.

           Using a backing store might improve the performance
           significantly, when working with widget overlays ( like rubber bands ).
           Disabling the cache might improve the performance for
           incremental paints (using QwtPlotDirectPainter ).

           \sa backingStore(), invalidateBackingStore()
         */
        BackingStore = 1,

        /*!
           \brief Try to fill the complete contents rectangle
                 of the plot canvas

           When using styled backgrounds Qt assumes, that the
           canvas doesn't fill its area completely
           ( f.e because of rounded borders ) and fills the area
           below the canvas. When this is done with gradients it might
           result in a serious performance bottleneck - depending on the size.

           When the Opaque attribute is enabled the canvas tries to
           identify the gaps with some heuristics and to fill those only.

           \warning Will not work for semitransparent backgrounds
         */
        Opaque       = 2,

        /*!
           \brief Try to improve painting of styled backgrounds

           QwtPlotCanvas supports the box model attributes for
           customizing the layout with style sheets. Unfortunately
           the design of Qt style sheets has no concept how to
           handle backgrounds with rounded corners - beside of padding.

           When HackStyledBackground is enabled the plot canvas tries
           to separate the background from the background border
           by reverse engineering to paint the background before and
           the border after the plot items. In this order the border
           gets perfectly antialiased and you can avoid some pixel
           artifacts in the corners.
         */
        HackStyledBackground = 4,

        /*!
           When ImmediatePaint is set replot() calls repaint()
           instead of update().

           \sa replot(), QWidget::repaint(), QWidget::update()
         */
        ImmediatePaint = 8
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    explicit QwtPlotCanvas( QwtPlot* = NULL );
    virtual ~QwtPlotCanvas();

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    const QPixmap* backingStore() const;
    Q_INVOKABLE void invalidateBackingStore();

    virtual bool event( QEvent* ) QWT_OVERRIDE;

    Q_INVOKABLE QPainterPath borderPath( const QRect& ) const;

  public Q_SLOTS:
    void replot();

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;

    virtual void drawBorder( QPainter* ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotCanvas::PaintAttributes )

#endif
