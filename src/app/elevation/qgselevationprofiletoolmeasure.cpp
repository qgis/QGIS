/***************************************************************************
                          qgselevationprofiletoolmeasure.cpp
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationprofiletoolmeasure.h"
#include "qgselevationprofilecanvas.h"
#include "qgsapplication.h"
#include "qgsplotmouseevent.h"
#include "qgsguiutils.h"
#include <QGraphicsLineItem>

QgsElevationProfileToolMeasure::QgsElevationProfileToolMeasure( QgsElevationProfileCanvas *canvas )
  : QgsPlotTool( canvas, QObject::tr( "Measure Tool" ) )
  , mElevationCanvas( canvas )
{
  setCursor( Qt::CrossCursor );

  mRubberBand = new QGraphicsLineItem();
  mRubberBand->setZValue( 1000 );

  QPen pen;
  pen.setWidthF( QgsGuiUtils::scaleIconSize( 2 ) );
  pen.setCosmetic( true );
  QgsSettings settings;
  const int red = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  const int green = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  const int blue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  pen.setColor( QColor( red, green, blue, 100 ) );
  mRubberBand->setPen( pen );

  mRubberBand->hide();
  mElevationCanvas->scene()->addItem( mRubberBand );
}

void QgsElevationProfileToolMeasure::deactivate()
{
  mRubberBand->hide();
  emit cleared();
  QgsPlotTool::deactivate();
}

void QgsElevationProfileToolMeasure::plotMoveEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
  if ( !mMeasureInProgress )
    return;

  const QRectF plotArea = mElevationCanvas->plotArea();
  const QPointF snappedPoint = event->snappedPoint().toQPointF();
  const QgsProfilePoint endPoint = mElevationCanvas->canvasPointToPlotPoint( constrainPointToRect( snappedPoint, plotArea ) );

  mRubberBand->setLine( QLineF( mRubberBand->line().p1(), mElevationCanvas->mapToScene( snappedPoint.toPoint() ) ) );

  const double distance = std::sqrt( std::pow( mStartPoint.distance() - endPoint.distance(), 2 ) + std::pow( mStartPoint.elevation() - endPoint.elevation(), 2 ) );
  emit distanceChanged( distance );
}

void QgsElevationProfileToolMeasure::plotPressEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  if ( mMeasureInProgress )
  {
    mMeasureInProgress = false;
  }
  else
  {
    const QRectF plotArea = mElevationCanvas->plotArea();
    if ( !plotArea.contains( event->pos() ) )
    {
      event->ignore();
      return;
    }

    const QPointF snappedPoint = event->snappedPoint().toQPointF();

    mRubberBand->setLine( QLineF( mElevationCanvas->mapToScene( snappedPoint.toPoint() ), mElevationCanvas->mapToScene( snappedPoint.toPoint() ) ) );
    mRubberBand->show();

    mStartPoint = mElevationCanvas->canvasPointToPlotPoint( snappedPoint );
    mMeasureInProgress = true;
  }
}

void QgsElevationProfileToolMeasure::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }
}


QgsElevationProfileToolMeasure::~QgsElevationProfileToolMeasure() = default;
