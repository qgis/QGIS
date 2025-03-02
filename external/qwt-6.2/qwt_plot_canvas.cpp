/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_canvas.h"
#include "qwt_painter.h"
#include "qwt_plot.h"

#include <qpainter.h>
#include <qpainterpath.h>
#include <qevent.h>

class QwtPlotCanvas::PrivateData
{
  public:
    PrivateData()
        : backingStore( NULL )
    {
    }

    ~PrivateData()
    {
        delete backingStore;
    }

    QwtPlotCanvas::PaintAttributes paintAttributes;
    QPixmap* backingStore;
};

/*!
   \brief Constructor

   \param plot Parent plot widget
   \sa QwtPlot::setCanvas()
 */
QwtPlotCanvas::QwtPlotCanvas( QwtPlot* plot )
    : QFrame( plot )
    , QwtPlotAbstractCanvas( this )
{
    m_data = new PrivateData;

    setPaintAttribute( QwtPlotCanvas::BackingStore, true );
    setPaintAttribute( QwtPlotCanvas::Opaque, true );
    setPaintAttribute( QwtPlotCanvas::HackStyledBackground, true );

    setLineWidth( 2 );
    setFrameShadow( QFrame::Sunken );
    setFrameShape( QFrame::Panel );
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
    delete m_data;
}

/*!
   \brief Changing the paint attributes

   \param attribute Paint attribute
   \param on On/Off

   \sa testPaintAttribute(), backingStore()
 */
void QwtPlotCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( m_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
        m_data->paintAttributes |= attribute;
    else
        m_data->paintAttributes &= ~attribute;

    switch ( attribute )
    {
        case BackingStore:
        {
            if ( on )
            {
                if ( m_data->backingStore == NULL )
                    m_data->backingStore = new QPixmap();

                if ( isVisible() )
                {
#if QT_VERSION >= 0x050000
                    *m_data->backingStore = grab( rect() );
#else
                    *m_data->backingStore =
                        QPixmap::grabWidget( this, rect() );
#endif
                }
            }
            else
            {
                delete m_data->backingStore;
                m_data->backingStore = NULL;
            }
            break;
        }
        case Opaque:
        {
            if ( on )
                setAttribute( Qt::WA_OpaquePaintEvent, true );

            break;
        }
        default:
        {
            break;
        }
    }
}

/*!
   Test whether a paint attribute is enabled

   \param attribute Paint attribute
   \return true, when attribute is enabled
   \sa setPaintAttribute()
 */
bool QwtPlotCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return m_data->paintAttributes & attribute;
}

//! \return Backing store, might be null
const QPixmap* QwtPlotCanvas::backingStore() const
{
    return m_data->backingStore;
}

//! Invalidate the internal backing store
void QwtPlotCanvas::invalidateBackingStore()
{
    if ( m_data->backingStore )
        *m_data->backingStore = QPixmap();
}

/*!
   Qt event handler for QEvent::PolishRequest and QEvent::StyleChange

   \param event Qt Event
   \return See QFrame::event()
 */
bool QwtPlotCanvas::event( QEvent* event )
{
    if ( event->type() == QEvent::PolishRequest )
    {
        if ( testPaintAttribute( QwtPlotCanvas::Opaque ) )
        {
            // Setting a style sheet changes the
            // Qt::WA_OpaquePaintEvent attribute, but we insist
            // on painting the background.

            setAttribute( Qt::WA_OpaquePaintEvent, true );
        }
    }

    if ( event->type() == QEvent::PolishRequest ||
        event->type() == QEvent::StyleChange )
    {
        updateStyleSheetInfo();
    }

    return QFrame::event( event );
}

/*!
   Paint event
   \param event Paint event
 */
void QwtPlotCanvas::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) &&
        m_data->backingStore != NULL )
    {
        QPixmap& bs = *m_data->backingStore;
        if ( bs.size() != size() * QwtPainter::devicePixelRatio( &bs ) )
        {
            bs = QwtPainter::backingStore( this, size() );

            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QPainter p( &bs );
                drawStyled( &p, testPaintAttribute( HackStyledBackground ) );
            }
            else
            {
                QPainter p;
                if ( borderRadius() <= 0.0 )
                {
                    QwtPainter::fillPixmap( this, bs );
                    p.begin( &bs );
                    drawCanvas( &p );
                }
                else
                {
                    p.begin( &bs );
                    drawUnstyled( &p );
                }

                if ( frameWidth() > 0 )
                    drawBorder( &p );
            }
        }

        painter.drawPixmap( 0, 0, *m_data->backingStore );
    }
    else
    {
        if ( testAttribute(Qt::WA_StyledBackground ) )
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                drawStyled( &painter, testPaintAttribute( HackStyledBackground ) );
            }
            else
            {
                drawCanvas( &painter );
            }
        }
        else
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                if ( autoFillBackground() )
                {
                    fillBackground( &painter );
                    drawBackground( &painter );
                }
            }
            else
            {
                if ( borderRadius() > 0.0 )
                {
                    QPainterPath clipPath;
                    clipPath.addRect( rect() );
                    clipPath = clipPath.subtracted( borderPath( rect() ) );

                    painter.save();

                    painter.setClipPath( clipPath, Qt::IntersectClip );
                    fillBackground( &painter );
                    drawBackground( &painter );

                    painter.restore();
                }
            }

            drawCanvas( &painter );

            if ( frameWidth() > 0 )
                drawBorder( &painter );
        }
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( &painter );
}

/*!
   Draw the border of the plot canvas

   \param painter Painter
   \sa setBorderRadius()
 */
void QwtPlotCanvas::drawBorder( QPainter* painter )
{
    if ( borderRadius() <= 0 )
    {
        drawFrame( painter );
        return;
    }

    QwtPlotAbstractCanvas::drawBorder( painter );
}

/*!
   Resize event
   \param event Resize event
 */
void QwtPlotCanvas::resizeEvent( QResizeEvent* event )
{
    QFrame::resizeEvent( event );
    updateStyleSheetInfo();
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
 */
void QwtPlotCanvas::replot()
{
    invalidateBackingStore();

    if ( testPaintAttribute( QwtPlotCanvas::ImmediatePaint ) )
        repaint( contentsRect() );
    else
        update( contentsRect() );
}

/*!
   Calculate the painter path for a styled or rounded border

   When the canvas has no styled background or rounded borders
   the painter path is empty.

   \param rect Bounding rectangle of the canvas
   \return Painter path, that can be used for clipping
 */
QPainterPath QwtPlotCanvas::borderPath( const QRect& rect ) const
{
    return canvasBorderPath( rect );
}

#if QWT_MOC_INCLUDE
#include "moc_qwt_plot_canvas.cpp"
#endif
