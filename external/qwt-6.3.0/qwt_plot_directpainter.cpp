/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_directpainter.h"
#include "qwt_scale_map.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_seriesitem.h"

#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>

static inline void qwtRenderItem(
    QPainter* painter, const QRect& canvasRect,
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    // A minor performance improvement is possible
    // with caching the maps. TODO ...

    QwtPlot* plot = seriesItem->plot();
    const QwtScaleMap xMap = plot->canvasMap( seriesItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( seriesItem->yAxis() );

    painter->setRenderHint( QPainter::Antialiasing,
        seriesItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    seriesItem->drawSeries( painter, xMap, yMap, canvasRect, from, to );
}

static inline bool qwtHasBackingStore( const QwtPlotCanvas* canvas )
{
    return canvas->testPaintAttribute( QwtPlotCanvas::BackingStore )
           && canvas->backingStore() && !canvas->backingStore()->isNull();
}

class QwtPlotDirectPainter::PrivateData
{
  public:
    PrivateData()
        : hasClipping( false )
        , seriesItem( NULL )
        , from( 0 )
        , to( 0 )
    {
    }

    QwtPlotDirectPainter::Attributes attributes;

    bool hasClipping;
    QRegion clipRegion;

    QPainter painter;

    QwtPlotSeriesItem* seriesItem;
    int from;
    int to;
};

//! Constructor
QwtPlotDirectPainter::QwtPlotDirectPainter( QObject* parent )
    : QObject( parent )
{
    m_data = new PrivateData;
}

//! Destructor
QwtPlotDirectPainter::~QwtPlotDirectPainter()
{
    delete m_data;
}

/*!
   Change an attribute

   \param attribute Attribute to change
   \param on On/Off

   \sa Attribute, testAttribute()
 */
void QwtPlotDirectPainter::setAttribute( Attribute attribute, bool on )
{
    if ( bool( m_data->attributes & attribute ) != on )
    {
        if ( on )
            m_data->attributes |= attribute;
        else
            m_data->attributes &= ~attribute;

        if ( ( attribute == AtomicPainter ) && on )
            reset();
    }
}

/*!
   \return True, when attribute is enabled
   \param attribute Attribute to be tested
   \sa Attribute, setAttribute()
 */
bool QwtPlotDirectPainter::testAttribute( Attribute attribute ) const
{
    return m_data->attributes & attribute;
}

/*!
   En/Disables clipping

   \param enable Enables clipping is true, disable it otherwise
   \sa hasClipping(), clipRegion(), setClipRegion()
 */
void QwtPlotDirectPainter::setClipping( bool enable )
{
    m_data->hasClipping = enable;
}

/*!
   \return true, when clipping is enabled
   \sa setClipping(), clipRegion(), setClipRegion()
 */
bool QwtPlotDirectPainter::hasClipping() const
{
    return m_data->hasClipping;
}

/*!
   \brief Assign a clip region and enable clipping

   Depending on the environment setting a proper clip region might improve
   the performance heavily. F.e. on Qt embedded only the clipped part of
   the backing store will be copied to a ( maybe unaccelerated ) frame buffer
   device.

   \param region Clip region
   \sa clipRegion(), hasClipping(), setClipping()
 */
void QwtPlotDirectPainter::setClipRegion( const QRegion& region )
{
    m_data->clipRegion = region;
    m_data->hasClipping = true;
}

/*!
   \return Currently set clip region.
   \sa setClipRegion(), setClipping(), hasClipping()
 */
QRegion QwtPlotDirectPainter::clipRegion() const
{
    return m_data->clipRegion;
}

/*!
   \brief Draw a set of points of a seriesItem.

   When observing an measurement while it is running, new points have to be
   added to an existing seriesItem. drawSeries() can be used to display them avoiding
   a complete redraw of the canvas.

   Setting plot()->canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
   will result in faster painting, if the paint engine of the canvas widget
   supports this feature.

   \param seriesItem Item to be painted
   \param from Index of the first point to be painted
   \param to Index of the last point to be painted. If to < 0 the
         series will be painted to its last point.
 */
void QwtPlotDirectPainter::drawSeries(
    QwtPlotSeriesItem* seriesItem, int from, int to )
{
    if ( seriesItem == NULL || seriesItem->plot() == NULL )
        return;

    QWidget* canvas = seriesItem->plot()->canvas();
    const QRect canvasRect = canvas->contentsRect();

    QwtPlotCanvas* plotCanvas = qobject_cast< QwtPlotCanvas* >( canvas );

    if ( plotCanvas && qwtHasBackingStore( plotCanvas ) )
    {
        QPainter painter( const_cast< QPixmap* >( plotCanvas->backingStore() ) );

        if ( m_data->hasClipping )
            painter.setClipRegion( m_data->clipRegion );

        qwtRenderItem( &painter, canvasRect, seriesItem, from, to );

        painter.end();

        if ( testAttribute( QwtPlotDirectPainter::FullRepaint ) )
        {
            plotCanvas->repaint();
            return;
        }
    }

    bool immediatePaint = true;
    if ( !canvas->testAttribute( Qt::WA_WState_InPaintEvent ) )
    {
#if QT_VERSION < 0x050000
        if ( !canvas->testAttribute( Qt::WA_PaintOutsidePaintEvent ) )
#endif
        immediatePaint = false;
    }

    if ( immediatePaint )
    {
        if ( !m_data->painter.isActive() )
        {
            reset();

            m_data->painter.begin( canvas );
            canvas->installEventFilter( this );
        }

        if ( m_data->hasClipping )
        {
            m_data->painter.setClipRegion(
                QRegion( canvasRect ) & m_data->clipRegion );
        }
        else
        {
            if ( !m_data->painter.hasClipping() )
                m_data->painter.setClipRect( canvasRect );
        }

        qwtRenderItem( &m_data->painter, canvasRect, seriesItem, from, to );

        if ( m_data->attributes & QwtPlotDirectPainter::AtomicPainter )
        {
            reset();
        }
        else
        {
            if ( m_data->hasClipping )
                m_data->painter.setClipping( false );
        }
    }
    else
    {
        reset();

        m_data->seriesItem = seriesItem;
        m_data->from = from;
        m_data->to = to;

        QRegion clipRegion = canvasRect;
        if ( m_data->hasClipping )
            clipRegion &= m_data->clipRegion;

        canvas->installEventFilter( this );
        canvas->repaint(clipRegion);
        canvas->removeEventFilter( this );

        m_data->seriesItem = NULL;
    }
}

//! Close the internal QPainter
void QwtPlotDirectPainter::reset()
{
    if ( m_data->painter.isActive() )
    {
        QWidget* w = static_cast< QWidget* >( m_data->painter.device() );
        if ( w )
            w->removeEventFilter( this );

        m_data->painter.end();
    }
}

//! Event filter
bool QwtPlotDirectPainter::eventFilter( QObject*, QEvent* event )
{
    if ( event->type() == QEvent::Paint )
    {
        reset();

        if ( m_data->seriesItem )
        {
            const QPaintEvent* pe = static_cast< QPaintEvent* >( event );

            QWidget* canvas = m_data->seriesItem->plot()->canvas();

            QPainter painter( canvas );
            painter.setClipRegion( pe->region() );

            bool doCopyCache = testAttribute( CopyBackingStore );

            if ( doCopyCache )
            {
                QwtPlotCanvas* plotCanvas =
                    qobject_cast< QwtPlotCanvas* >( canvas );
                if ( plotCanvas )
                {
                    doCopyCache = qwtHasBackingStore( plotCanvas );
                    if ( doCopyCache )
                    {
                        painter.drawPixmap( plotCanvas->rect().topLeft(),
                            *plotCanvas->backingStore() );
                    }
                }
            }

            if ( !doCopyCache )
            {
                qwtRenderItem( &painter, canvas->contentsRect(),
                    m_data->seriesItem, m_data->from, m_data->to );
            }

            return true; // don't call QwtPlotCanvas::paintEvent()
        }
    }

    return false;
}
