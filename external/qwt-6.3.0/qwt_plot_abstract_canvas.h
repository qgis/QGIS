/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_ABSTRACT_CANVAS_H
#define QWT_PLOT_ABSTRACT_CANVAS_H

#include "qwt_global.h"
#include <qframe.h>

class QwtPlot;

/*!
   \brief Base class for all type of plot canvases
 */
class QWT_EXPORT QwtPlotAbstractCanvas
{
  public:
    /*!
       \brief Focus indicator
       The default setting is NoFocusIndicator
       \sa setFocusIndicator(), focusIndicator(), drawFocusIndicator()
     */

    enum FocusIndicator
    {
        //! Don't paint a focus indicator
        NoFocusIndicator,

        /*!
           The focus is related to the complete canvas.
           Paint the focus indicator using drawFocusIndicator()
         */
        CanvasFocusIndicator,

        /*!
           The focus is related to an item (curve, point, ...) on
           the canvas. It is up to the application to display a
           focus indication using f.e. highlighting.
         */
        ItemFocusIndicator
    };

    explicit QwtPlotAbstractCanvas( QWidget* canvasWidget );
    virtual ~QwtPlotAbstractCanvas();

    QwtPlot* plot();
    const QwtPlot* plot() const;

    void setFocusIndicator( FocusIndicator );
    FocusIndicator focusIndicator() const;

    void setBorderRadius( double );
    double borderRadius() const;

  protected:
    QWidget* canvasWidget();
    const QWidget* canvasWidget() const;

    virtual void drawFocusIndicator( QPainter* );
    virtual void drawBorder( QPainter* );
    virtual void drawBackground( QPainter* );

    void fillBackground( QPainter* );
    void drawCanvas( QPainter* );
    void drawStyled( QPainter*, bool );
    void drawUnstyled( QPainter* );

    QPainterPath canvasBorderPath( const QRect& rect ) const;
    void updateStyleSheetInfo();

  private:
    Q_DISABLE_COPY(QwtPlotAbstractCanvas)

    class PrivateData;
    PrivateData* m_data;
};

/*!
   \brief Base class of QwtPlotOpenGLCanvas and QwtPlotGLCanvas
 */
class QWT_EXPORT QwtPlotAbstractGLCanvas : public QwtPlotAbstractCanvas
{
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
           When ImmediatePaint is set replot() calls repaint()
           instead of update().

           \sa replot(), QWidget::repaint(), QWidget::update()
         */
        ImmediatePaint = 8,
    };

    //! Paint attributes
    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    explicit QwtPlotAbstractGLCanvas( QWidget* canvasWidget );
    virtual ~QwtPlotAbstractGLCanvas();

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    void setFrameStyle( int style );
    int frameStyle() const;

    void setFrameShadow( QFrame::Shadow );
    QFrame::Shadow frameShadow() const;

    void setFrameShape( QFrame::Shape );
    QFrame::Shape frameShape() const;

    void setLineWidth( int );
    int lineWidth() const;

    void setMidLineWidth( int );
    int midLineWidth() const;

    int frameWidth() const;
    QRect frameRect() const;

    //! Invalidate the internal backing store
    virtual void invalidateBackingStore() = 0;

  protected:
    void replot();
    void draw( QPainter* );

  private:
    virtual void clearBackingStore() = 0;

    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPlotAbstractGLCanvas::PaintAttributes )

#endif
