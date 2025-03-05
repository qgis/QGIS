/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_OPENGL_CANVAS_H
#define QWT_PLOT_OPENGL_CANVAS_H

#include "qwt_global.h"
#include "qwt_plot_abstract_canvas.h"

#include <qopenglwidget.h>

class QwtPlot;

/*!
   \brief An alternative canvas for a QwtPlot derived from QOpenGLWidget

   Even if QwtPlotOpenGLCanvas is not derived from QFrame it imitates
   its API. When using style sheets it supports the box model - beside
   backgrounds with rounded borders.

   \sa QwtPlot::setCanvas(), QwtPlotCanvas, QwtPlotCanvas::OpenGLBuffer

   \note Another way for getting hardware accelerated graphics is using
        an OpenGL offscreen buffer ( QwtPlotCanvas::OpenGLBuffer ) with QwtPlotCanvas.
        Performance is worse, than rendering straight to a QOpenGLWidget, but is usually
        better integrated into a desktop application.
 */
class QWT_EXPORT QwtPlotOpenGLCanvas : public QOpenGLWidget, public QwtPlotAbstractGLCanvas
{
    Q_OBJECT

    Q_PROPERTY( QFrame::Shadow frameShadow READ frameShadow WRITE setFrameShadow )
    Q_PROPERTY( QFrame::Shape frameShape READ frameShape WRITE setFrameShape )
    Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
    Q_PROPERTY( int midLineWidth READ midLineWidth WRITE setMidLineWidth )
    Q_PROPERTY( int frameWidth READ frameWidth )
    Q_PROPERTY( QRect frameRect READ frameRect DESIGNABLE false )

    Q_PROPERTY( double borderRadius READ borderRadius WRITE setBorderRadius )

  public:
    explicit QwtPlotOpenGLCanvas( QwtPlot* = NULL );
    explicit QwtPlotOpenGLCanvas( const QSurfaceFormat&, QwtPlot* = NULL);
    virtual ~QwtPlotOpenGLCanvas();

    Q_INVOKABLE virtual void invalidateBackingStore() QWT_OVERRIDE;
    Q_INVOKABLE QPainterPath borderPath( const QRect& ) const;

    virtual bool event( QEvent* ) QWT_OVERRIDE;

  public Q_SLOTS:
    void replot();

  protected:
    virtual void paintEvent( QPaintEvent* ) QWT_OVERRIDE;

    virtual void initializeGL() QWT_OVERRIDE;
    virtual void paintGL() QWT_OVERRIDE;
    virtual void resizeGL( int width, int height ) QWT_OVERRIDE;

  private:
    void init( const QSurfaceFormat& );
    virtual void clearBackingStore() QWT_OVERRIDE;

    class PrivateData;
    PrivateData* m_data;
};

#endif
