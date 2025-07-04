/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_CANVAS_H
#define QWT_POLAR_CANVAS_H

#include "qwt_global.h"
#include "qwt_point_polar.h"
#include <qframe.h>

class QPainter;
class QwtPolarPlot;

/*!
   \brief Canvas of a QwtPolarPlot.

   The canvas is the widget, where all polar items are painted to.

   \note In opposite to QwtPlot all axes are painted on the canvas.
   \sa QwtPolarPlot
 */
class QWT_EXPORT QwtPolarCanvas : public QFrame
{
    Q_OBJECT

  public:
    /*!
       \brief Paint attributes

       The default setting enables BackingStore

       \sa setPaintAttribute(), testPaintAttribute(), backingStore()
     */

    enum PaintAttribute
    {
        /*!
           Paint double buffered and reuse the content of the pixmap buffer
           for some spontaneous repaints that happen when a plot gets unhidden,
           deiconified or changes the focus.
         */
        BackingStore = 0x01
    };

    Q_DECLARE_FLAGS( PaintAttributes, PaintAttribute )

    explicit QwtPolarCanvas( QwtPolarPlot* );
    virtual ~QwtPolarCanvas();

    QwtPolarPlot* plot();
    const QwtPolarPlot* plot() const;

    void setPaintAttribute( PaintAttribute, bool on = true );
    bool testPaintAttribute( PaintAttribute ) const;

    const QPixmap* backingStore() const;
    void invalidateBackingStore();

    QwtPointPolar invTransform( const QPoint& ) const;
    QPoint transform( const QwtPointPolar& ) const;

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;

  private:
    class PrivateData;
    PrivateData* m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QwtPolarCanvas::PaintAttributes )

#endif
