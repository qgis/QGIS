/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_scale_div.h"
#include "qwt_polar_plot.h"
#include "qwt_polar_canvas.h"
#include "qwt_polar_panner.h"

//! Create a plot panner for a polar plot canvas
QwtPolarPanner::QwtPolarPanner( QwtPolarCanvas *canvas ):
    QwtPanner( canvas )
{
  connect( this, SIGNAL( panned( int, int ) ),
           SLOT( movePlot( int, int ) ) );
}

//! Destructor
QwtPolarPanner::~QwtPolarPanner()
{
}

//! Return observed plot canvas
QwtPolarCanvas *QwtPolarPanner::canvas()
{
  QWidget *w = parentWidget();
  if ( w && w->inherits( "QwtPolarCanvas" ) )
    return ( QwtPolarCanvas * )w;

  return NULL;
}

//! Return observed plot canvas
const QwtPolarCanvas *QwtPolarPanner::canvas() const
{
  return (( QwtPolarPanner * )this )->canvas();
}

//! Return observed plot
QwtPolarPlot *QwtPolarPanner::plot()
{
  QwtPolarCanvas *c = canvas();
  if ( c )
    return c->plot();

  return NULL;
}

//! Return observed plot
const QwtPolarPlot *QwtPolarPanner::plot() const
{
  return (( QwtPolarPanner * )this )->plot();
}

/*!
   Adjust the zoomed area according to dx/dy

   \param dx Pixel offset in x direction
   \param dy Pixel offset in y direction

   \sa QwtPanner::panned(), QwtPolarPlot::zoom()
*/
void QwtPolarPanner::movePlot( int dx, int dy )
{
  QwtPolarPlot *plot = QwtPolarPanner::plot();
  if ( plot == NULL || ( dx == 0 && dy == 0 ) )
    return;

  const QwtScaleMap map = plot->scaleMap( QwtPolar::Radius );

  QwtPolarPoint pos = plot->zoomPos();
  if ( map.s1() <= map.s2() )
  {
    pos.setRadius(
      map.xTransform( map.s1() + pos.radius() ) - map.p1() );
    pos.setPoint( pos.toPoint() - QwtDoublePoint( dx, -dy ) );
    pos.setRadius(
      map.invTransform( map.p1() + pos.radius() ) - map.s1() );
  }
  else
  {
    pos.setRadius(
      map.xTransform( map.s1() - pos.radius() ) - map.p1() );
    pos.setPoint( pos.toPoint() - QwtDoublePoint( dx, -dy ) );
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

  \param me Mouse event
*/
void QwtPolarPanner::widgetMousePressEvent( QMouseEvent *me )
{
  const QwtPolarPlot *plot = QwtPolarPanner::plot();
  if ( plot )
  {
    if ( plot->zoomFactor() < 1.0 )
      QwtPanner::widgetMousePressEvent( me );
  }
}


