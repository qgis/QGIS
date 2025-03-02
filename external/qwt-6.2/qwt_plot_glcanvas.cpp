/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_glcanvas.h"
#include "qwt_plot.h"
#include "qwt_painter.h"

#include <qcoreevent.h>
#include <qpainter.h>
#include <qpainterpath.h>
#include <qglframebufferobject.h>

namespace
{
    class QwtPlotGLCanvasFormat : public QGLFormat
    {
      public:
        QwtPlotGLCanvasFormat()
            : QGLFormat( QGLFormat::defaultFormat() )
        {
            setSampleBuffers( true );
        }
    };
}

class QwtPlotGLCanvas::PrivateData
{
  public:
    PrivateData()
        : fboDirty( true )
        , fbo( NULL )
    {
    }

    ~PrivateData()
    {
        delete fbo;
    }

    bool fboDirty;
    QGLFramebufferObject* fbo;
};

/*!
   \brief Constructor

   \param plot Parent plot widget
   \sa QwtPlot::setCanvas()
 */
QwtPlotGLCanvas::QwtPlotGLCanvas( QwtPlot* plot )
    : QGLWidget( QwtPlotGLCanvasFormat(), plot )
    , QwtPlotAbstractGLCanvas( this )
{
    init();
}
/*!
   \brief Constructor

   \param format OpenGL rendering options
   \param plot Parent plot widget
   \sa QwtPlot::setCanvas()
 */
QwtPlotGLCanvas::QwtPlotGLCanvas( const QGLFormat& format, QwtPlot* plot )
    : QGLWidget( format, plot )
    , QwtPlotAbstractGLCanvas( this )
{
    init();
}

//! Destructor
QwtPlotGLCanvas::~QwtPlotGLCanvas()
{
    delete m_data;
}

void QwtPlotGLCanvas::init()
{
    m_data = new PrivateData;

#if 1
    setAttribute( Qt::WA_OpaquePaintEvent, true );
#endif
    setLineWidth( 2 );
    setFrameShadow( QFrame::Sunken );
    setFrameShape( QFrame::Panel );
}

/*!
   Paint event

   \param event Paint event
   \sa QwtPlot::drawCanvas()
 */
void QwtPlotGLCanvas::paintEvent( QPaintEvent* event )
{
    QGLWidget::paintEvent( event );
}

/*!
   Qt event handler for QEvent::PolishRequest and QEvent::StyleChange
   \param event Qt Event
   \return See QGLWidget::event()
 */
bool QwtPlotGLCanvas::event( QEvent* event )
{
    const bool ok = QGLWidget::event( event );

    if ( event->type() == QEvent::PolishRequest ||
        event->type() == QEvent::StyleChange )
    {
        // assuming, that we always have a styled background
        // when we have a style sheet

        setAttribute( Qt::WA_StyledBackground,
            testAttribute( Qt::WA_StyleSheet ) );
    }

    return ok;
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
 */
void QwtPlotGLCanvas::replot()
{
    QwtPlotAbstractGLCanvas::replot();
}

//! Invalidate the internal backing store
void QwtPlotGLCanvas::invalidateBackingStore()
{
    m_data->fboDirty = true;
}

void QwtPlotGLCanvas::clearBackingStore()
{
    delete m_data->fbo;
    m_data->fbo = NULL;
}

/*!
   Calculate the painter path for a styled or rounded border

   When the canvas has no styled background or rounded borders
   the painter path is empty.

   \param rect Bounding rectangle of the canvas
   \return Painter path, that can be used for clipping
 */
QPainterPath QwtPlotGLCanvas::borderPath( const QRect& rect ) const
{
    return canvasBorderPath( rect );
}

//! No operation - reserved for some potential use in the future
void QwtPlotGLCanvas::initializeGL()
{
}

//! Paint the plot
void QwtPlotGLCanvas::paintGL()
{
    const bool hasFocusIndicator =
        hasFocus() && focusIndicator() == CanvasFocusIndicator;

    QPainter painter;

    if ( testPaintAttribute( QwtPlotGLCanvas::BackingStore ) )
    {
        const qreal pixelRatio = QwtPainter::devicePixelRatio( NULL );
        const QRect rect( 0, 0, width() * pixelRatio, height() * pixelRatio );

        if ( hasFocusIndicator )
            painter.begin( this );

        if ( m_data->fbo )
        {
            if ( m_data->fbo->size() != rect.size() )
            {
                delete m_data->fbo;
                m_data->fbo = NULL;
            }
        }

        if ( m_data->fbo == NULL )
        {
            QGLFramebufferObjectFormat format;
            format.setSamples( 4 );
            format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

            m_data->fbo = new QGLFramebufferObject( rect.size(), format );
            m_data->fboDirty = true;
        }

        if ( m_data->fboDirty )
        {
            QPainter fboPainter( m_data->fbo );
            fboPainter.scale( pixelRatio, pixelRatio );
            draw( &fboPainter );
            fboPainter.end();

            m_data->fboDirty = false;
        }

        /*
            Why do we have this strange translation - but, anyway
            QwtPlotGLCanvas in combination with scaling factor
            is not very likely to happen as using QwtPlotOpenGLCanvas
            usually makes more sense then.
         */

        QGLFramebufferObject::blitFramebuffer( NULL,
            rect.translated( 0, height() - rect.height() ), m_data->fbo, rect );
    }
    else
    {
        painter.begin( this );
        draw( &painter );
    }

    if ( hasFocusIndicator )
        drawFocusIndicator( &painter );
}

//! No operation - reserved for some potential use in the future
void QwtPlotGLCanvas::resizeGL( int, int )
{
    // nothing to do
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_plot_glcanvas.cpp"
#endif
