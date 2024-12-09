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
#include "moc_qgsmaptooldeletering.cpp"

#include "qgsmapcanvas.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsrubberband.h"
#include "qgsmapmouseevent.h"

#include <limits>


QgsMapToolDeleteRing::QgsMapToolDeleteRing( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
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

void QgsMapToolDeleteRing::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  //nothing to do
}

void QgsMapToolDeleteRing::canvasPressEvent( QgsMapMouseEvent *e )
{
  delete mRubberBand;
  mRubberBand = nullptr;
  mPressedFid = -1;
  mPressedPartNum = -1;
  mPressedRingNum = -1;

  QgsMapLayer *currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( vlayer->geometryType() != Qgis::GeometryType::Polygon )
  {
    emit messageEmitted( tr( "Delete ring can only be used in a polygon layer." ) );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  const QgsPointXY p = toLayerCoordinates( vlayer, e->pos() );

  const QgsGeometry ringGeom = ringUnderPoint( p, mPressedFid, mPressedPartNum, mPressedRingNum );

  if ( mPressedRingNum != -1 )
  {
    QgsFeature f;
    vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );
    mRubberBand = createRubberBand( vlayer->geometryType() );

    mRubberBand->setToGeometry( ringGeom, vlayer );
    mRubberBand->show();
  }
  else if ( vlayer->selectedFeatureCount() > 0 )
  {
    emit messageEmitted(
      tr( "If there are selected features, the delete ring tool only applies to those. Clear the selection and try again." ),
      Qgis::MessageLevel::Warning
    );
  }
}

void QgsMapToolDeleteRing::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )

  delete mRubberBand;
  mRubberBand = nullptr;

  if ( mPressedFid == -1 )
    return;

  QgsFeature f;

  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );

  QgsGeometry g = f.geometry();
  if ( g.deleteRing( mPressedRingNum, mPressedPartNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( mPressedFid, g );
    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

QgsGeometry QgsMapToolDeleteRing::ringUnderPoint( const QgsPointXY &p, QgsFeatureId &fid, int &partNum, int &ringNum )
{
  //There is no clean way to find if we are inside the ring of a feature,
  //so we iterate over all the features visible in the canvas
  //If several rings are found at this position, the smallest one is chosen,
  //in order to be able to delete a ring inside another ring
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( toLayerCoordinates( vlayer, mCanvas->extent() ) ) );
  QgsFeature f;
  QgsGeometry g;
  QgsGeometry ringGeom;
  QgsMultiPolygonXY pol;
  QgsPolygonXY tempPol;
  QgsGeometry tempGeom;
  double area = std::numeric_limits<double>::max();
  const QgsFeatureIds selectedFeatureIds = vlayer->selectedFeatureIds();
  while ( fit.nextFeature( f ) )
  {
    if ( !selectedFeatureIds.isEmpty() && !selectedFeatureIds.contains( f.id() ) )
      continue;

    g = f.geometry();
    if ( g.isNull() || QgsWkbTypes::geometryType( g.wkbType() ) != Qgis::GeometryType::Polygon )
      continue;

    if ( !QgsWkbTypes::isMultiType( g.wkbType() ) )
    {
      pol = QgsMultiPolygonXY() << g.asPolygon();
    }
    else
    {
      pol = g.asMultiPolygon();
    }

    for ( int i = 0; i < pol.size(); ++i )
    {
      //for each part
      if ( pol[i].size() > 1 )
      {
        for ( int j = 1; j < pol[i].size(); ++j )
        {
          tempPol = QgsPolygonXY() << pol[i][j];
          tempGeom = QgsGeometry::fromPolygonXY( tempPol );
          if ( tempGeom.area() < area && tempGeom.contains( &p ) )
          {
            fid = f.id();
            partNum = i;
            ringNum = j;
            area = tempGeom.area();
            ringGeom = tempGeom;
          }
        }
      }
    }
  }
  return ringGeom;
}


void QgsMapToolDeleteRing::deleteRing( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer *vlayer )
{
  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fId ) ).nextFeature( f );

  const QgsGeometry g = f.geometry();
  const Qgis::WkbType wkbtype = g.wkbType();
  int ringNum, partNum = 0;

  if ( QgsWkbTypes::geometryType( wkbtype ) != Qgis::GeometryType::Polygon )
    return;

  if ( !QgsWkbTypes::isMultiType( wkbtype ) )
  {
    ringNum = ringNumInPolygon( g, beforeVertexNr );
  }
  else
  {
    ringNum = ringNumInMultiPolygon( g, beforeVertexNr, partNum );
  }

  QgsGeometry editableGeom = f.geometry();
  if ( editableGeom.deleteRing( ringNum, partNum ) )
  {
    vlayer->beginEditCommand( tr( "Ring deleted" ) );
    vlayer->changeGeometry( fId, editableGeom );
    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

int QgsMapToolDeleteRing::ringNumInPolygon( const QgsGeometry &g, int vertexNr )
{
  QgsPolygonXY polygon = g.asPolygon();
  for ( int ring = 0; ring < polygon.count(); ring++ )
  {
    if ( vertexNr < polygon[ring].count() )
      return ring;

    vertexNr -= polygon[ring].count();
  }
  return -1;
}

int QgsMapToolDeleteRing::ringNumInMultiPolygon( const QgsGeometry &g, int vertexNr, int &partNum )
{
  QgsMultiPolygonXY mpolygon = g.asMultiPolygon();
  for ( int part = 0; part < mpolygon.count(); part++ )
  {
    const QgsPolygonXY &polygon = mpolygon[part];
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
