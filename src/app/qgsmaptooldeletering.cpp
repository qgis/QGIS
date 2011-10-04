/***************************************************************************
    qgsmaptooldeletering.cpp  - delete a ring from polygon
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldeletering.h"

#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolDeleteRing::QgsMapToolDeleteRing( QgsMapCanvas* canvas )
    : QgsMapToolVertexEdit( canvas ), mCross( 0 )
{
}

QgsMapToolDeleteRing::~QgsMapToolDeleteRing()
{
  delete mCross;
}

void QgsMapToolDeleteRing::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeleteRing::canvasPressEvent( QMouseEvent *e )
{
  delete mCross;
  mCross = 0;

  mRecentSnappingResults.clear();
  //do snap -> new recent snapping results
  if ( mSnapper.snapToCurrentLayer( e->pos(), mRecentSnappingResults, QgsSnapper::SnapToVertex ) != 0 )
  {
    //error
  }

  if ( mRecentSnappingResults.size() > 0 )
  {
    QgsPoint markerPoint = mRecentSnappingResults.begin()->snappedVertex;

    //show vertex marker
    mCross = new QgsVertexMarker( mCanvas );
    mCross->setIconType( QgsVertexMarker::ICON_X );
    mCross->setCenter( markerPoint );
  }
  else
  {
    displaySnapToleranceWarning();
  }
}

void QgsMapToolDeleteRing::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  delete mCross;
  mCross = 0;

  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
    return;


  if ( mRecentSnappingResults.size() > 0 )
  {
    QList<QgsSnappingResult>::iterator sr_it = mRecentSnappingResults.begin();
    for ( ; sr_it != mRecentSnappingResults.end(); ++sr_it )
    {
      deleteRing( sr_it->snappedAtGeometry, sr_it->snappedVertexNr, vlayer );
    }
  }
}


void QgsMapToolDeleteRing::deleteRing( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer* vlayer )
{
  QgsFeature f;
  vlayer->featureAtId( fId, f );

  QgsGeometry* g = f.geometry();
  QGis::WkbType wkbtype = g->wkbType();
  int ringNum, partNum = 0;

  if ( wkbtype == QGis::WKBPolygon || wkbtype == QGis::WKBPolygon25D )
  {
    ringNum = ringNumInPolygon( g, beforeVertexNr );
  }
  else if ( wkbtype == QGis::WKBMultiPolygon || wkbtype == QGis::WKBMultiPolygon25D )
  {
    ringNum = ringNumInMultiPolygon( g, beforeVertexNr, partNum );
  }
  else
    return;

  if ( g->deleteRing( ringNum, partNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( fId, g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }

}

int QgsMapToolDeleteRing::ringNumInPolygon( QgsGeometry* g, int vertexNr )
{
  QgsPolygon polygon = g->asPolygon();
  for ( int ring = 0; ring < polygon.count(); ring++ )
  {
    if ( vertexNr < polygon[ring].count() )
      return ring;

    vertexNr -= polygon[ring].count();
  }
  return -1;
}

int QgsMapToolDeleteRing::ringNumInMultiPolygon( QgsGeometry* g, int vertexNr, int& partNum )
{
  QgsMultiPolygon mpolygon = g->asMultiPolygon();
  for ( int part = 0; part < mpolygon.count(); part++ )
  {
    const QgsPolygon& polygon = mpolygon[part];
    for ( int ring = 0; ring < polygon.count(); ring++ )
    {
      if ( vertexNr < polygon[ring].count() )
      {
        partNum = part;
        return ring;
      }

      vertexNr -= polygon[ring].count();
    }
  }
  return -1;
}


void QgsMapToolDeleteRing::deactivate()
{
  delete mCross;
  mCross = 0;

  QgsMapTool::deactivate();
}
