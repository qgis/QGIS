/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_ZOOMER_H
#define QWT_PLOT_ZOOMER_H

#include "qwt_global.h"
#include "qwt_plot_picker.h"

class QSizeF;
template< typename T > class QStack;

/*!
   \brief QwtPlotZoomer provides stacked zooming for a plot widget

   QwtPlotZoomer selects rectangles from user inputs ( mouse or keyboard )
   translates them into plot coordinates and adjusts the axes to them.
   The selection is supported by a rubber band and optionally by displaying
   the coordinates of the current mouse position.

   Zooming can be repeated as often as possible, limited only by
   maxStackDepth() or minZoomSize().  Each rectangle is pushed on a stack.

   The default setting how to select rectangles is
   a QwtPickerDragRectMachine with the following bindings:

   - QwtEventPattern::MouseSelect1\n
    The first point of the zoom rectangle is selected by a mouse press,
    the second point from the position, where the mouse is released.

   - QwtEventPattern::KeySelect1\n
    The first key press selects the first, the second key press
    selects the second point.

   - QwtEventPattern::KeyAbort\n
    Discard the selection in the state, where the first point
    is selected.

   To traverse the zoom stack the following bindings are used:

   - QwtEventPattern::MouseSelect3, QwtEventPattern::KeyUndo\n
    Zoom out one position on the zoom stack

   - QwtEventPattern::MouseSelect6, QwtEventPattern::KeyRedo\n
    Zoom in one position on the zoom stack

   - QwtEventPattern::MouseSelect2, QwtEventPattern::KeyHome\n
    Zoom to the zoom base

   The setKeyPattern() and setMousePattern() functions can be used
   to configure the zoomer actions. The following example
   shows, how to configure the 'I' and 'O' keys for zooming in and out
   one position on the zoom stack. The "Home" key is used to
   "unzoom" the plot.

   \code
   zoomer = new QwtPlotZoomer( plot );
   zoomer->setKeyPattern( QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier );
   zoomer->setKeyPattern( QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier );
   zoomer->setKeyPattern( QwtEventPattern::KeyHome, Qt::Key_Home );
   \endcode

   QwtPlotZoomer is tailored for plots with one x and y axis, but it is
   allowed to attach a second QwtPlotZoomer ( without rubber band and tracker )
   for the other axes.

   \note The realtime example includes an derived zoomer class that adds
        scrollbars to the plot canvas.

   \sa QwtPlotPanner, QwtPlotMagnifier
 */

class QWT_EXPORT QwtPlotZoomer : public QwtPlotPicker
{
    Q_OBJECT
  public:
    explicit QwtPlotZoomer( QWidget*, bool doReplot = true );
    explicit QwtPlotZoomer( QwtAxisId xAxis, QwtAxisId yAxis,
        QWidget*, bool doReplot = true );

    virtual ~QwtPlotZoomer();

    virtual void setZoomBase( bool doReplot = true );
    virtual void setZoomBase( const QRectF& );

    QRectF zoomBase() const;
    QRectF zoomRect() const;

    virtual void setAxes( QwtAxisId xAxis, QwtAxisId yAxis ) QWT_OVERRIDE;

    void setMaxStackDepth( int );
    int maxStackDepth() const;

    const QStack< QRectF >& zoomStack() const;
    void setZoomStack( const QStack< QRectF >&,
        int zoomRectIndex = -1 );

    uint zoomRectIndex() const;

  public Q_SLOTS:
    void moveBy( double dx, double dy );
    virtual void moveTo( const QPointF& );

    virtual void zoom( const QRectF& );
    virtual void zoom( int offset );

  Q_SIGNALS:
    /*!
       A signal emitting the zoomRect(), when the plot has been
       zoomed in or out.

       \param rect Current zoom rectangle.
     */

    void zoomed( const QRectF& rect );

  protected:
    virtual void rescale();

    virtual QSizeF minZoomSize() const;

    virtual void widgetMouseReleaseEvent( QMouseEvent* ) QWT_OVERRIDE;
    virtual void widgetKeyPressEvent( QKeyEvent* ) QWT_OVERRIDE;

    virtual void begin() QWT_OVERRIDE;
    virtual bool end( bool ok = true ) QWT_OVERRIDE;
    virtual bool accept( QPolygon& ) const QWT_OVERRIDE;

  private:
    void init( bool doReplot );

    class PrivateData;
    PrivateData* m_data;
};

#endif
