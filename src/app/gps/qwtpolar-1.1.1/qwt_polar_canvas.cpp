/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_canvas.h"
#include "qwt_polar_plot.h"
#include <qwt_painter.h>
#include <qpainter.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qstyleoption.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

static inline void qwtDrawStyledBackground(
    QWidget *widget, QPainter *painter )
{
    QStyleOption opt;
    opt.initFrom( widget );
    widget->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, widget );
}

static QWidget *qwtBackgroundWidget( QWidget *w )
{
    if ( w->parentWidget() == NULL )
        return w;

    if ( w->autoFillBackground() )
    {
        const QBrush brush = w->palette().brush( w->backgroundRole() );
        if ( brush.color().alpha() > 0 )
            return w;
    }

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        QImage image( 1, 1, QImage::Format_ARGB32 );
        image.fill( Qt::transparent );

        QPainter painter( &image );
        painter.translate( -w->rect().center() );
        qwtDrawStyledBackground( w, &painter );
        painter.end();

        if ( qAlpha( image.pixel( 0, 0 ) ) != 0 )
            return w;
    }

    return qwtBackgroundWidget( w->parentWidget() );
}

class QwtPolarCanvas::PrivateData
{
public:
    PrivateData():
        paintAttributes( 0 ),
        backingStore( NULL )
    {
    }

    ~PrivateData()
    {
        delete backingStore;
    }

    QwtPolarCanvas::PaintAttributes paintAttributes;
    QPixmap *backingStore;
};

//! Constructor
QwtPolarCanvas::QwtPolarCanvas( QwtPolarPlot *plot ):
    QFrame( plot )
{
    d_data = new PrivateData;

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif
    setFocusPolicy( Qt::WheelFocus );

    setPaintAttribute( BackingStore, true );
}

//! Destructor
QwtPolarCanvas::~QwtPolarCanvas()
{
    delete d_data;
}

//! \return Parent plot widget
QwtPolarPlot *QwtPolarCanvas::plot()
{
    return qobject_cast<QwtPolarPlot *>( parent() );
}

//! \return Parent plot widget
const QwtPolarPlot *QwtPolarCanvas::plot() const
{
    return qobject_cast<QwtPolarPlot *>( parent() );
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  The default setting enables BackingStore

  \sa testPaintAttribute(), paintCache()
*/
void QwtPolarCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch( attribute )
    {
        case BackingStore:
        {
            if ( on )
            {
                if ( d_data->backingStore == NULL )
                    d_data->backingStore = new QPixmap();

                if ( isVisible() )
                {
                    const QRect cr = contentsRect();
                    *d_data->backingStore = QPixmap::grabWidget( this, cr );
                }
            }
            else
            {
                delete d_data->backingStore;
                d_data->backingStore = NULL;
            }
            break;
        }
    }
}

/*!
  Test wether a paint attribute is enabled

  \param attribute Paint attribute
  \return true if the attribute is enabled
  \sa setPaintAttribute()
*/
bool QwtPolarCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute ) != 0;
}

//! \return Backing store, might be null
const QPixmap *QwtPolarCanvas::backingStore() const
{
    return d_data->backingStore;
}

//! Invalidate the internal backing store
void QwtPolarCanvas::invalidateBackingStore()
{
    if ( d_data->backingStore )
        *d_data->backingStore = QPixmap();
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPolarCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( ( d_data->paintAttributes & BackingStore )
        && d_data->backingStore != NULL )
    {
        QPixmap &bs = *d_data->backingStore;
        if ( bs.size() != size() )
        {
            bs = QPixmap( size() );
#ifdef Q_WS_X11
            if ( bs.x11Info().screen() != x11Info().screen() )
                bs.x11SetScreen( x11Info().screen() );
#endif

            QPainter p;

            if ( testAttribute( Qt::WA_StyledBackground ) )
            {
                p.begin( &bs );
                qwtDrawStyledBackground( this, &p );
            }
            else
            {
                if ( autoFillBackground() )
                {
                    p.begin( &bs );
                    p.fillRect( rect(), palette().brush( backgroundRole() ) );
                }
                else
                {
                    QWidget *bgWidget = qwtBackgroundWidget( plot() );

                    QwtPainter::fillPixmap( bgWidget, bs,
                        mapTo( bgWidget, rect().topLeft() ) );

                    p.begin( &bs );
                }
            }

            plot()->drawCanvas( &p, contentsRect() );

            if ( frameWidth() > 0 )
                drawFrame( &p );
        }

        painter.drawPixmap( 0, 0, *d_data->backingStore );
    }
    else
    {
        qwtDrawStyledBackground( this, &painter );

        plot()->drawCanvas( &painter, contentsRect() );

        if ( frameWidth() > 0 )
            drawFrame( &painter );
    }
}

/*!
  Resize event
  \param event Resize event
*/
void QwtPolarCanvas::resizeEvent( QResizeEvent *event )
{
    QFrame::resizeEvent( event );

    for ( int scaleId = 0; scaleId < QwtPolar::ScaleCount; scaleId++ )
        plot()->updateScale( scaleId );
}

/*!
    Translate a point from widget into plot coordinates

    \param pos Point in widget coordinates of the plot canvas
    \return Point in plot coordinates

    \sa transform()
*/
QwtPointPolar QwtPolarCanvas::invTransform( const QPoint &pos ) const
{
    const QwtPolarPlot *pl = plot();

    const QwtScaleMap azimuthMap = pl->scaleMap( QwtPolar::Azimuth );
    const QwtScaleMap radialMap = pl->scaleMap( QwtPolar::Radius );

    const QPointF center = pl->plotRect().center();

    double dx = pos.x() - center.x();
    double dy = -( pos.y() - center.y() );

    const QwtPointPolar polarPos = QwtPointPolar( QPoint( dx, dy ) ).normalized();

    double azimuth = azimuthMap.invTransform( polarPos.azimuth() );

    // normalize the azimuth
    double min = azimuthMap.s1();
    double max = azimuthMap.s2();
    if ( max < min )
        qSwap( min, max );

    if ( azimuth < min )
    {
        azimuth += max - min;
    }
    else if ( azimuth > max )
    {
        azimuth -= max - min;
    }

    const double radius = radialMap.invTransform( polarPos.radius() );

    return QwtPointPolar( azimuth, radius );
}

/*!
    Translate a point from plot into widget coordinates

    \param polarPos Point in plot coordinates
    \return Point in widget coordinates
    \sa transform()
*/
QPoint QwtPolarCanvas::transform( const QwtPointPolar &polarPos ) const
{
    const QwtPolarPlot *pl = plot();

    const QwtScaleMap azimuthMap = pl->scaleMap( QwtPolar::Azimuth );
    const QwtScaleMap radialMap = pl->scaleMap( QwtPolar::Radius );

    const double radius = radialMap.transform( polarPos.radius() );
    const double azimuth = azimuthMap.transform( polarPos.azimuth() );

    const QPointF pos = qwtPolar2Pos(
        pl->plotRect().center(), radius, azimuth );

    return pos.toPoint();
}
