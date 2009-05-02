/***************************************************************************
    qgsmaptooldeletehole.h  - delete a hole from polygon
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

#include "qgsmaptooldeletehole.h"

#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolDeleteHole::QgsMapToolDeleteHole( QgsMapCanvas* canvas )
  : QgsMapToolVertexEdit( canvas ), mCross( 0 )
{
}

QgsMapToolDeleteHole::~QgsMapToolDeleteHole()
{
  delete mCross;
}

void QgsMapToolDeleteHole::canvasMoveEvent( QMouseEvent * e )
{
  //nothing to do
}

void QgsMapToolDeleteHole::canvasPressEvent( QMouseEvent * e )
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

void QgsMapToolDeleteHole::canvasReleaseEvent( QMouseEvent * e )
{
  delete mCross;
  mCross = 0;

  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  if ( !vlayer )
    return;


  if ( mRecentSnappingResults.size() > 0 )
  {
    QList<QgsSnappingResult>::iterator sr_it = mRecentSnappingResults.begin();
    for ( ; sr_it != mRecentSnappingResults.end(); ++sr_it )
    {
      deleteHole( sr_it->snappedAtGeometry, sr_it->snappedVertexNr, vlayer);
    }
  }
}


void QgsMapToolDeleteHole::deleteHole( int fId, int beforeVertexNr, QgsVectorLayer* vlayer)
{
  QgsFeature f;
  vlayer->featureAtId( fId, f );

  QgsGeometry* g = f.geometry();
  QGis::WkbType wkbtype = g->wkbType();
  int ringNum, partNum = 0;

  if (wkbtype == QGis::WKBPolygon || wkbtype == QGis::WKBPolygon25D)
  {
    ringNum = ringNumInPolygon( g, beforeVertexNr );
  }
  else if (wkbtype == QGis::WKBMultiPolygon || wkbtype == QGis::WKBMultiPolygon25D)
  {
    ringNum = ringNumInMultiPolygon( g, beforeVertexNr, partNum );
  }
  else
    return;

  if (g->deleteHole( ringNum, partNum ))
  {
    vlayer->deleteFeature( fId );
    vlayer->addFeature(f);
    mCanvas->refresh();
  }
  
}

int QgsMapToolDeleteHole::ringNumInPolygon( QgsGeometry* g, int vertexNr )
{
  QgsPolygon polygon = g->asPolygon();
  for (int ring = 0; ring < polygon.count(); ring++)
  {
    if (vertexNr < polygon[ring].count())
      return ring;

    vertexNr -= polygon[ring].count();
  }
  return -1;
}

int QgsMapToolDeleteHole::ringNumInMultiPolygon( QgsGeometry* g, int vertexNr, int& partNum )
{
  QgsMultiPolygon mpolygon = g->asMultiPolygon();
  for (int part = 0; part < mpolygon.count(); part++)
  {
    const QgsPolygon& polygon = mpolygon[part];
    for (int ring = 0; ring < polygon.count(); ring++)
    {
      if (vertexNr < polygon[ring].count())
      {
        partNum = part;
        return ring;
      }

      vertexNr -= polygon[ring].count();
    }
  }
  return -1;
}


void QgsMapToolDeleteHole::deactivate()
{
  delete mCross;
  mCross = 0;

  QgsMapTool::deactivate();
}
