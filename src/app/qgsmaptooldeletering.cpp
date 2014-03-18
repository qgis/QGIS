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
#include "qgsmessagebar.h"
#include "qgisapp.h"

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolDeleteRing::QgsMapToolDeleteRing( QgsMapCanvas* canvas )
    : QgsMapToolVertexEdit( canvas )
{
  mToolName = tr( "Delete ring" );
}

QgsMapToolDeleteRing::~QgsMapToolDeleteRing()
{
}

void QgsMapToolDeleteRing::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeleteRing::canvasPressEvent( QMouseEvent *e )
{
  //do snap -> new recent snapping results
  if ( mSnapper.snapToCurrentLayer( e->pos(), mRecentSnappingResults, QgsSnapper::SnapToVertexAndSegment ) != 0 )
  {
    //error
  }
}

void QgsMapToolDeleteRing::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );

  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( vlayer->geometryType() != QGis::Polygon )
  {
    emit messageEmitted( tr( "Delete ring can only be used in a polygon layer." ) );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  QgsPoint p = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(),e->y());
  p = toLayerCoordinates(vlayer, p);

  //No easy way to find if we are in the ring of a feature,
  //so we iterate over all the features visible in the canvas
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( mCanvas->extent() ) );
  QgsFeature f;
  QgsGeometry* g;
  QgsMultiPolygon pol;
  QgsPolygon tempPol;
  QgsGeometry* tempGeom;
  int fid, partNum, ringNum;
  double area = std::numeric_limits<double>::max();
  while ( fit.nextFeature( f ) )
  {
    QgsDebugMsg(QString("Feature %1").arg(f.id()));
    g = f.geometry();
    if ( !g )
      continue;
    if ( g->wkbType() == QGis::WKBPolygon ||  g->wkbType()  == QGis::WKBPolygon25D )
    {
      pol = QgsMultiPolygon() << g->asPolygon();
    }
    else
    {
      pol = g->asMultiPolygon();
    }

    for (int i = 0; i < pol.size() ; ++i)
    {//for each part
      QgsDebugMsg(QString("Feature %1 part %2").arg(f.id()).arg(i));
      if ( pol[i].size() > 1 )
      {
        QgsDebugMsg(QString("%1 has a ring in part %2").arg(f.id()).arg(i));
        for ( int j = 1; j<pol[i].size();++j)
        {
          tempPol = QgsPolygon()<<pol[i][j];
          tempGeom = QgsGeometry::fromPolygon( tempPol );
          if (tempGeom->area() < area && tempGeom->contains(&p) )
          {
            QgsDebugMsg(QString("%1, part %2, ring %3 contains the cursor").arg(f.id()).arg(i).arg(j));
            fid=f.id();
            partNum=i;
            ringNum=j;
            area=tempGeom->area();
          }
        }
      }
    }
  }

  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fid ) ).nextFeature( f );

  g = f.geometry();
  if ( g->deleteRing( ringNum, partNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( fid, g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }
/*
  if ( g->deleteRing( ringNum, partNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( f.id(), g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }
*/
}

void QgsMapToolDeleteRing::deleteRing( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer* vlayer )
{
  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fId ) ).nextFeature( f );

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

int QgsMapToolDeleteRing::partNumberOfPoint(QgsGeometry* g, QgsPoint p)
{
  return 0;
}

int QgsMapToolDeleteRing::ringNumberOfPoint(QgsGeometry* g, QgsPoint p, int partNum)
{
  return 0;
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

  QgsMapTool::deactivate();
}
