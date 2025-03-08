/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_widget_overlay.h"
#include "qwt_painter.h"

#include <qpainter.h>
#include <qpaintengine.h>
#include <qpainterpath.h>
#include <qimage.h>
#include <qevent.h>

#include <cstdlib>

static QImage::Format qwtMaskImageFormat()
{
    if ( QwtPainter::isX11GraphicsSystem() )
        return QImage::Format_ARGB32;

    return QImage::Format_ARGB32_Premultiplied;
}

static QRegion qwtAlphaMask( const QImage& image, const QRegion& region )
{
    const int w = image.width();
    const int h = image.height();

    QRegion mask;
    QRect rect;

#if QT_VERSION >= 0x050800
    for ( QRegion::const_iterator it = region.cbegin();
        it != region.cend(); ++it )
    {
        const QRect& r = *it;
#else
    const QVector< QRect > rects = region.rects();
    for ( int i = 0; i < rects.size(); i++ )
    {
        const QRect& r = rects[i];
#endif
        int x1, x2, y1, y2;
        r.getCoords( &x1, &y1, &x2, &y2 );

        x1 = qMax( x1, 0 );
        x2 = qMin( x2, w - 1 );
        y1 = qMax( y1, 0 );
        y2 = qMin( y2, h - 1 );

        for ( int y = y1; y <= y2; ++y )
        {
            bool inRect = false;
            int rx0 = -1;

            const uint* line =
                reinterpret_cast< const uint* > ( image.scanLine( y ) ) + x1;
            for ( int x = x1; x <= x2; x++ )
            {
                const bool on = ( ( *line++ >> 24 ) != 0 );
                if ( on != inRect )
                {
                    if ( inRect  )
                    {
                        rect.setCoords( rx0, y, x - 1, y );
                        mask += rect;
                    }
                    else
                    {
                        rx0 = x;
                    }

                    inRect = on;
                }
            }

            if ( inRect )
            {
                rect.setCoords( rx0, y, x2, y );
                mask = mask.united( rect );
            }
        }
    }

    return mask;
}

class QwtWidgetOverlay::PrivateData
{
  public:
    PrivateData()
        : maskMode( QwtWidgetOverlay::MaskHint )
        , renderMode( QwtWidgetOverlay::AutoRenderMode )
        , rgbaBuffer( NULL )
    {
    }

    ~PrivateData()
    {
        resetRgbaBuffer();
    }

    void resetRgbaBuffer()
    {
        if ( rgbaBuffer )
        {
            std::free( rgbaBuffer );
            rgbaBuffer = NULL;
        }
    }

    MaskMode maskMode;
    RenderMode renderMode;
    uchar* rgbaBuffer;
};

/*!
   \brief Constructor
   \param widget Parent widget, where the overlay is aligned to
 */
QwtWidgetOverlay::QwtWidgetOverlay( QWidget* widget )
    : QWidget( widget )
{
    m_data = new PrivateData;

    setAttribute( Qt::WA_TransparentForMouseEvents );
    setAttribute( Qt::WA_NoSystemBackground );
    setFocusPolicy( Qt::NoFocus );

    if ( widget )
    {
        resize( widget->size() );
        widget->installEventFilter( this );
    }
}

//! Destructor
QwtWidgetOverlay::~QwtWidgetOverlay()
{
    delete m_data;
}

/*!
   \brief Specify how to find the mask for the overlay

   \param mode New mode
   \sa maskMode()
 */
void QwtWidgetOverlay::setMaskMode( MaskMode mode )
{
    if ( mode != m_data->maskMode )
    {
        m_data->maskMode = mode;
        m_data->resetRgbaBuffer();
    }
}

/*!
   \return Mode how to find the mask for the overlay
   \sa setMaskMode()
 */
QwtWidgetOverlay::MaskMode QwtWidgetOverlay::maskMode() const
{
    return m_data->maskMode;
}

/*!
   Set the render mode
   \param mode Render mode

   \sa RenderMode, renderMode()
 */
void QwtWidgetOverlay::setRenderMode( RenderMode mode )
{
    m_data->renderMode = mode;
}

/*!
   \return Render mode
   \sa RenderMode, setRenderMode()
 */
QwtWidgetOverlay::RenderMode QwtWidgetOverlay::renderMode() const
{
    return m_data->renderMode;
}

/*!
   Recalculate the mask and repaint the overlay
 */
void QwtWidgetOverlay::updateOverlay()
{
    updateMask();
    update();
}

void QwtWidgetOverlay::updateMask()
{
    m_data->resetRgbaBuffer();

    QRegion mask;

    if ( m_data->maskMode == QwtWidgetOverlay::MaskHint )
    {
        mask = maskHint();
    }
    else if ( m_data->maskMode == QwtWidgetOverlay::AlphaMask )
    {
        // TODO: the image doesn't need to be larger than
        //       the bounding rectangle of the hint !!

        QRegion hint = maskHint();
        if ( hint.isEmpty() )
            hint += QRect( 0, 0, width(), height() );

        // A fresh buffer from calloc() is usually faster
        // than reinitializing an existing one with
        // QImage::fill( 0 ) or memset()

        m_data->rgbaBuffer = ( uchar* )::calloc( width() * height(), 4 );

        QImage image( m_data->rgbaBuffer,
            width(), height(), qwtMaskImageFormat() );

        QPainter painter( &image );
        draw( &painter );
        painter.end();

        mask = qwtAlphaMask( image, hint );

        if ( m_data->renderMode == QwtWidgetOverlay::DrawOverlay )
        {
            // we don't need the buffer later
            m_data->resetRgbaBuffer();
        }
    }

    // A bug in Qt initiates a full repaint of the widget
    // when we change the mask, while we are visible !

    setVisible( false );

    if ( mask.isEmpty() )
        clearMask();
    else
        setMask( mask );

    setVisible( true );
}

/*!
   Paint event
   \param event Paint event

   \sa drawOverlay()
 */
void QwtWidgetOverlay::paintEvent( QPaintEvent* event )
{
    const QRegion& clipRegion = event->region();

    QPainter painter( this );

    bool useRgbaBuffer = false;
    if ( m_data->renderMode == QwtWidgetOverlay::CopyAlphaMask )
    {
        useRgbaBuffer = true;
    }
    else if ( m_data->renderMode == QwtWidgetOverlay::AutoRenderMode )
    {
        if ( painter.paintEngine()->type() == QPaintEngine::Raster )
            useRgbaBuffer = true;
    }

    if ( m_data->rgbaBuffer && useRgbaBuffer )
    {
        const QImage image( m_data->rgbaBuffer,
            width(), height(), qwtMaskImageFormat() );

        const int rectCount = clipRegion.rectCount();

        if ( rectCount > 2000 )
        {
            // the region is to complex
            painter.setClipRegion( clipRegion );

            const QRect r = clipRegion.boundingRect();
            painter.drawImage( r.topLeft(), image, r );
        }
        else
        {
#if QT_VERSION >= 0x050800
            for ( QRegion::const_iterator it = clipRegion.cbegin();
                it != clipRegion.cend(); ++it )
            {
                const QRect& r = *it;
                painter.drawImage( r.topLeft(), image, r );
            }
#else
            const QVector< QRect > rects = clipRegion.rects();
            for ( int i = 0; i < rects.size(); i++ )
            {
                const QRect& r = rects[i];
                painter.drawImage( r.topLeft(), image, r );
            }
#endif
        }
    }
    else
    {
        painter.setClipRegion( clipRegion );
        draw( &painter );
    }
}

/*!
   Resize event
   \param event Resize event
 */
void QwtWidgetOverlay::resizeEvent( QResizeEvent* event )
{
    Q_UNUSED( event );

    m_data->resetRgbaBuffer();
}

void QwtWidgetOverlay::draw( QPainter* painter ) const
{
    if ( QWidget* widget = parentWidget() )
    {
        painter->setClipRect( widget->contentsRect() );

        // something special for the plot canvas

        const int idx = widget->metaObject()->indexOfMethod( "borderPath(QRect)" );
        if ( idx >= 0 )
        {
            QPainterPath clipPath;

            ( void )QMetaObject::invokeMethod(
                widget, "borderPath", Qt::DirectConnection,
                Q_RETURN_ARG( QPainterPath, clipPath ), Q_ARG( QRect, rect() ) );

            if (!clipPath.isEmpty() )
                painter->setClipPath( clipPath, Qt::IntersectClip );
        }
    }

    drawOverlay( painter );
}

/*!
   \brief Calculate an approximation for the mask

   - MaskHint
     The hint is used as mask.

   - AlphaMask
     The hint is used to speed up the algorithm
     for calculating a mask from non transparent pixels

   - NoMask
     The hint is unused.

   The default implementation returns an invalid region
   indicating no hint.

   \return Hint for the mask
 */
QRegion QwtWidgetOverlay::maskHint() const
{
    return QRegion();
}

/*!
   \brief Event filter

   Resize the overlay according to the size of the parent widget.

   \param object Object to be filtered
   \param event Event

   \return See QObject::eventFilter()
 */

bool QwtWidgetOverlay::eventFilter( QObject* object, QEvent* event )
{
    if ( object == parent() && event->type() == QEvent::Resize )
    {
        QResizeEvent* resizeEvent = static_cast< QResizeEvent* >( event );
        resize( resizeEvent->size() );
    }

    return QObject::eventFilter( object, event );
}
