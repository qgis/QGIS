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
#include <limits>

QgsMapToolDeleteRing::QgsMapToolDeleteRing( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , vlayer( NULL )
    , mRubberBand( 0 )
    , mPressedFid( 0 )
    , mPressedPartNum( 0 )
    , mPressedRingNum( 0 )
{
  mToolName = tr( "Delete ring" );
}

QgsMapToolDeleteRing::~QgsMapToolDeleteRing()
{
  delete mRubberBand;
}

void QgsMapToolDeleteRing::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeleteRing::canvasPressEvent( QMouseEvent *e )
{
  delete mRubberBand;
  mRubberBand = 0;
  mPressedFid = -1;
  mPressedPartNum = -1;
  mPressedRingNum = -1;

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

  QgsPoint p = toLayerCoordinates( vlayer, e->pos() );

  QgsGeometry* ringGeom = ringUnderPoint( p, mPressedFid, mPressedPartNum, mPressedRingNum );

  if ( mPressedFid != -1 )
  {
    QgsFeature f;
    vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );
    mRubberBand = createRubberBand( vlayer->geometryType() );

    mRubberBand->setToGeometry( ringGeom, vlayer );
    mRubberBand->show();
  }
}

void QgsMapToolDeleteRing::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );

  delete mRubberBand;
  mRubberBand = 0;

  if ( mPressedFid == -1 )
    return;

  QgsFeature f;
  QgsGeometry* g;

  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );

  g = f.geometry();
  if ( g->deleteRing( mPressedRingNum, mPressedPartNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( mPressedFid, g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }
}

QgsGeometry* QgsMapToolDeleteRing::ringUnderPoint( QgsPoint p, QgsFeatureId& fid, int& partNum, int& ringNum )
{
  //There is no clean way to find if we are inside the ring of a feature,
  //so we iterate over all the features visible in the canvas
  //If several rings are found at this position, the smallest one is chosen,
  //in order to be able to delete a ring inside another ring
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( toLayerCoordinates( vlayer, mCanvas->extent() ) ) );
  QgsFeature f;
  QgsGeometry* g;
  QgsGeometry* ringGeom = 0;
  QgsMultiPolygon pol;
  QgsPolygon tempPol;
  QgsGeometry* tempGeom;
  double area = std::numeric_limits<double>::max();
  while ( fit.nextFeature( f ) )
  {
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

    for ( int i = 0; i < pol.size() ; ++i )
    {//for each part
      if ( pol[i].size() > 1 )
      {
        for ( int j = 1; j < pol[i].size();++j )
        {
          tempPol = QgsPolygon() << pol[i][j];
          tempGeom = QgsGeometry::fromPolygon( tempPol );
          if ( tempGeom->area() < area && tempGeom->contains( &p ) )
          {
            fid = f.id();
            partNum = i;
            ringNum = j;
            ringGeom = tempGeom;
            area = tempGeom->area();
          }
        }
      }
    }
  }
  return ringGeom;
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
