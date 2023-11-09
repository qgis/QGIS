/***************************************************************************
                          qgselevationprofiletoolidentify.cpp
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

#include "qgselevationprofiletoolidentify.h"
#include "qgsplotcanvas.h"
#include "qgsplotmouseevent.h"
#include "qgsapplication.h"
#include "qgsplotrubberband.h"
#include "qgselevationprofilecanvas.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgisapp.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgspointcloudlayer.h"

QgsElevationProfileToolIdentify::QgsElevationProfileToolIdentify( QgsElevationProfileCanvas *canvas )
  : QgsPlotTool( canvas, tr( "Identify" ) )
{
  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::Identify ) );

  mRubberBand.reset( new QgsPlotRectangularRubberBand( canvas ) );
  mRubberBand->setBrush( QBrush( QColor( 254, 178, 76, 63 ) ) );
  mRubberBand->setPen( QPen( QBrush( QColor( 254, 58, 29, 100 ) ), 0 ) );
}

QgsElevationProfileToolIdentify::~QgsElevationProfileToolIdentify() = default;

Qgis::PlotToolFlags QgsElevationProfileToolIdentify::flags() const
{
  return Qgis::PlotToolFlag::ShowContextMenu;
}

void QgsElevationProfileToolIdentify::plotPressEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  // don't allow the band to be dragged outside of the plot area
  const QRectF plotArea = qgis::down_cast< QgsElevationProfileCanvas * >( mCanvas )->plotArea();
  if ( !plotArea.contains( event->pos() ) )
  {
    mMousePressStartPos = constrainPointToRect( event->pos(), plotArea );
    mSnappedMousePressStartPos = mMousePressStartPos;
  }
  else
  {
    mMousePressStartPos = event->pos();
    mSnappedMousePressStartPos = event->snappedPoint();
  }

  mMarquee = true;

  mRubberBand->start( mMousePressStartPos, Qt::KeyboardModifiers() );
}

void QgsElevationProfileToolIdentify::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }


  QgsMapToolIdentifyAction *identifyTool2D = QgisApp::instance()->identifyMapTool();
  identifyTool2D->clearResults();

  const bool clickOnly = !isClickAndDrag( mMousePressStartPos.toPoint(), event->pos() );
  mRubberBand->finish( event->pos() );
  mMarquee = false;
  QVector<QgsProfileIdentifyResults> results;
  if ( !clickOnly )
  {
    // don't allow the band to be dragged outside of the plot area
    const QRectF plotArea = qgis::down_cast< QgsElevationProfileCanvas * >( mCanvas )->plotArea();
    QPointF end;
    if ( !plotArea.contains( event->pos() ) )
    {
      end = constrainPointToRect( event->pos(), plotArea );
    }
    else
    {
      end = event->snappedPoint().toQPointF();
    }

    results = qgis::down_cast< QgsElevationProfileCanvas * >( mCanvas )->identify( QRectF( mSnappedMousePressStartPos.toQPointF(), end ) );
  }
  else
  {
    results = qgis::down_cast< QgsElevationProfileCanvas * >( mCanvas )->identify( mSnappedMousePressStartPos.toQPointF() );
  }

  if ( results.empty() )
    return;

  QList<QgsMapToolIdentify::IdentifyResult> identifyResults;
  for ( const QgsProfileIdentifyResults &result : std::as_const( results ) )
  {
    if ( QgsMapLayer *layer = result.layer() )
    {
      identifyTool2D->fromElevationProfileLayerIdentificationToIdentifyResults( layer, result.results(), identifyResults );
    }
  }

  identifyTool2D->showIdentifyResults( identifyResults );
}

void QgsElevationProfileToolIdentify::plotMoveEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
  if ( !mMarquee )
  {
    return;
  }

  // don't allow the band to be dragged outside of the plot area
  const QRectF plotArea = qgis::down_cast< QgsElevationProfileCanvas * >( mCanvas )->plotArea();
  QPointF movePoint;
  if ( !plotArea.contains( event->pos() ) )
  {
    movePoint = constrainPointToRect( event->pos(), plotArea );
  }
  else
  {
    movePoint = event->snappedPoint().toQPointF();
  }

  mRubberBand->update( movePoint, Qt::KeyboardModifiers() );
}

