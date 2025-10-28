/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_panner.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_scale_div.h"
#include "qwt_point_polar.h"

//! Create a plot panner for a polar plot canvas
QwtPolarPanner::QwtPolarPanner( QwtPolarCanvas* canvas )
    : QwtPanner( canvas )
{
    connect( this, SIGNAL(panned(int,int)),
        SLOT(movePlot(int,int)) );
}

//! Destructor
QwtPolarPanner::~QwtPolarPanner()
{
}

//! \return observed plot canvas
QwtPolarCanvas* QwtPolarPanner::canvas()
{
    return qobject_cast< QwtPolarCanvas* >( parent() );
}

//! \return observed plot canvas
const QwtPolarCanvas* QwtPolarPanner::canvas() const
{
    return qobject_cast< const QwtPolarCanvas* >( parent() );
}

//! \return observed plot
QwtPolarPlot* QwtPolarPanner::plot()
{
    QwtPolarCanvas* c = canvas();
    if ( c )
        return c->plot();

    return NULL;
}

//! \return observed plot
const QwtPolarPlot* QwtPolarPanner::plot() const
{
    const QwtPolarCanvas* c = canvas();
    if ( c )
        return c->plot();

    return NULL;
}

/*!
   Adjust the zoomed area according to dx/dy

   \param dx Pixel offset in x direction
   \param dy Pixel offset in y direction

   \sa QwtPanner::panned(), QwtPolarPlot::zoom()
 */
void QwtPolarPanner::movePlot( int dx, int dy )
{
    QwtPolarPlot* plot = QwtPolarPanner::plot();
    if ( plot == NULL || ( dx == 0 && dy == 0 ) )
        return;

    const QwtScaleMap map = plot->scaleMap( QwtPolar::Radius );

    QwtPointPolar pos = plot->zoomPos();
    if ( map.s1() <= map.s2() )
    {
        pos.setRadius(
            map.transform( map.s1() + pos.radius() ) - map.p1() );
        pos.setPoint( pos.toPoint() - QPointF( dx, -dy ) );
        pos.setRadius(
            map.invTransform( map.p1() + pos.radius() ) - map.s1() );
    }
    else
    {
        pos.setRadius(
            map.transform( map.s1() - pos.radius() ) - map.p1() );
        pos.setPoint( pos.toPoint() - QPointF( dx, -dy ) );
        pos.setRadius(
            map.s1() - map.invTransform( map.p1() + pos.radius() ) );
    }

    const bool doAutoReplot = plot->autoReplot();
    plot->setAutoReplot( false );

    plot->zoom( pos, plot->zoomFactor() );

    plot->setAutoReplot( doAutoReplot );
    plot->replot();
}

/*!
   Block panning when the plot zoom factor is >= 1.0.

   \param event Mouse event
 */
void QwtPolarPanner::widgetMousePressEvent( QMouseEvent* event )
{
    const QwtPolarPlot* plot = QwtPolarPanner::plot();
    if ( plot )
    {
        if ( plot->zoomFactor() < 1.0 )
            QwtPanner::widgetMousePressEvent( event );
    }
}

#include "moc_qwt_polar_panner.cpp"
