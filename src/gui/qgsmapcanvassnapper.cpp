/***************************************************************************
                              qgsmapcanvassnapper.cpp
                              -----------------------
  begin                : June 21, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvassnapper.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include <QSettings>
#include "qgslogger.h"
#include "qgsgeometry.h"

QgsMapCanvasSnapper::QgsMapCanvasSnapper( QgsMapCanvas* canvas )
    : mMapCanvas( canvas )
    , mSnapper( 0 )
{
  if ( !canvas )
    return;

  mSnapper = new QgsSnapper( canvas->mapSettings() );
}

QgsMapCanvasSnapper::QgsMapCanvasSnapper(): mMapCanvas( 0 ), mSnapper( 0 )
{
}

QgsMapCanvasSnapper::~QgsMapCanvasSnapper()
{
  delete mSnapper;
}

void QgsMapCanvasSnapper::setMapCanvas( QgsMapCanvas* canvas )
{
  mMapCanvas = canvas;
  delete mSnapper;
  if ( mMapCanvas )
  {
    mSnapper = new QgsSnapper( canvas->mapSettings() );
  }
  else
  {
    mSnapper = 0;
  }
}

int QgsMapCanvasSnapper::snapToCurrentLayer( const QPoint& p, QList<QgsSnappingResult>& results,
    QgsSnapper::SnappingType snap_to,
    double snappingTol,
    const QList<QgsPoint>& excludePoints )
{
  results.clear();

  if ( !mSnapper || !mMapCanvas )
    return 1;

  //topological editing on?
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  if ( topologicalEditing == 0 )
  {
    mSnapper->setSnapMode( QgsSnapper::SnapWithOneResult );
  }
  else
  {
    mSnapper->setSnapMode( QgsSnapper::SnapWithResultsForSamePosition );
  }

  //current vector layer
  QgsMapLayer* currentLayer = mMapCanvas->currentLayer();
  if ( !currentLayer )
    return 2;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
    return 3;

  QgsSnapper::SnapLayer snapLayer;
  snapLayer.mLayer = vlayer;
  snapLayer.mSnapTo = snap_to;
  snapLayer.mUnitType = QgsTolerance::MapUnits;

  if ( snappingTol < 0 )
  {
    //use search tolerance for vertex editing
    snapLayer.mTolerance = QgsTolerance::vertexSearchRadius( vlayer, mMapCanvas->mapSettings() );
  }
  else
  {
    snapLayer.mTolerance = snappingTol;
  }

  QList<QgsSnapper::SnapLayer> snapLayers;
  snapLayers.append( snapLayer );
  mSnapper->setSnapLayers( snapLayers );

  if ( mSnapper->snapPoint( p, results, excludePoints ) != 0 )
    return 4;

  return 0;
}

int QgsMapCanvasSnapper::snapToBackgroundLayers( const QPoint& p, QList<QgsSnappingResult>& results, const QList<QgsPoint>& excludePoints )
{
  const QgsPoint mapCoordPoint = mMapCanvas->mapSettings().mapToPixel().toMapCoordinates( p.x(), p.y() );
  return snapToBackgroundLayers( mapCoordPoint, results, excludePoints );
}

int QgsMapCanvasSnapper::snapToBackgroundLayers( const QgsPoint& point, QList<QgsSnappingResult>& results, const QList<QgsPoint>& excludePoints )
{
  results.clear();

  if ( !mSnapper )
    return 5;

  //topological editing on?
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );

  //snapping on intersection on?
  int intersectionSnapping = QgsProject::instance()->readNumEntry( "Digitizing", "/IntersectionSnapping", 0 );

  if ( topologicalEditing == 0 )
  {
    if ( intersectionSnapping == 0 )
      mSnapper->setSnapMode( QgsSnapper::SnapWithOneResult );
    else
      mSnapper->setSnapMode( QgsSnapper::SnapWithResultsWithinTolerances );
  }
  else if ( intersectionSnapping == 0 )
  {
    mSnapper->setSnapMode( QgsSnapper::SnapWithResultsForSamePosition );
  }
  else
  {
    mSnapper->setSnapMode( QgsSnapper::SnapWithResultsWithinTolerances );
  }

  //read snapping settings from project
  bool snappingDefinedInProject, ok;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", QStringList(), &snappingDefinedInProject );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", QStringList(), &ok );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", QStringList(), &ok );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", QStringList(), &ok );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", QStringList(), &ok );

  if ( !( layerIdList.size() == enabledList.size() &&
          layerIdList.size() == toleranceList.size() &&
          layerIdList.size() == toleranceUnitList.size() &&
          layerIdList.size() == snapToList.size() ) )
  {
    // lists must have the same size, otherwise something is wrong
    return 1;
  }

  QList<QgsSnapper::SnapLayer> snapLayers;
  QgsSnapper::SnapLayer snapLayer;

  // Use snapping information from the project
  if ( snappingDefinedInProject )
  {
    // set layers, tolerances, snap to segment/vertex to QgsSnapper
    QStringList::const_iterator layerIt( layerIdList.constBegin() );
    QStringList::const_iterator tolIt( toleranceList.constBegin() );
    QStringList::const_iterator tolUnitIt( toleranceUnitList.constBegin() );
    QStringList::const_iterator snapIt( snapToList.constBegin() );
    QStringList::const_iterator enabledIt( enabledList.constBegin() );
    for ( ; layerIt != layerIdList.constEnd(); ++layerIt, ++tolIt, ++tolUnitIt, ++snapIt, ++enabledIt )
    {
      if ( *enabledIt != "enabled" )
      {
        // skip layer if snapping is not enabled
        continue;
      }

      //layer
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( *layerIt ) );
      if ( !vlayer || !vlayer->hasGeometryType() )
        continue;

      snapLayer.mLayer = vlayer;

      //tolerance
      snapLayer.mTolerance = tolIt->toDouble();
      snapLayer.mUnitType = ( QgsTolerance::UnitType ) tolUnitIt->toInt();

      // segment or vertex
      if ( *snapIt == "to_vertex" )
      {
        snapLayer.mSnapTo = QgsSnapper::SnapToVertex;
      }
      else if ( *snapIt == "to_segment" )
      {
        snapLayer.mSnapTo = QgsSnapper::SnapToSegment;
      }
      else
      {
        // to vertex and segment
        snapLayer.mSnapTo = QgsSnapper::SnapToVertexAndSegment;
      }

      snapLayers.append( snapLayer );
    }
  }
  else
  {
    // nothing in project. Use default snapping tolerance to vertex of current layer
    QgsMapLayer* currentLayer = mMapCanvas->currentLayer();
    if ( !currentLayer )
      return 2;

    QgsVectorLayer* currentVectorLayer = qobject_cast<QgsVectorLayer *>( currentLayer );
    if ( !currentVectorLayer )
      return 3;

    snapLayer.mLayer = currentVectorLayer;

    //default snap mode
    QSettings settings;
    QString defaultSnapString = settings.value( "/qgis/digitizing/default_snap_mode", "off" ).toString();
    if ( defaultSnapString == "to segment" )
    {
      snapLayer.mSnapTo = QgsSnapper::SnapToSegment;
    }
    else if ( defaultSnapString == "to vertex and segment" )
    {
      snapLayer.mSnapTo = QgsSnapper::SnapToVertexAndSegment;
    }
    else if ( defaultSnapString == "to vertex" )
    {
      snapLayer.mSnapTo = QgsSnapper::SnapToVertex;
    }
    else
    {
      return 0;
    }

    //default snapping tolerance (returned in map units)
    snapLayer.mTolerance = QgsTolerance::defaultTolerance( currentVectorLayer, mMapCanvas->mapSettings() );
    snapLayer.mUnitType = QgsTolerance::MapUnits;

    snapLayers.append( snapLayer );
  }

  mSnapper->setSnapLayers( snapLayers );

  if ( mSnapper->snapPoint( point, results, excludePoints ) != 0 )
    return 4;

  if ( intersectionSnapping != 1 )
    return 0;

  QList<QgsSnappingResult> segments;
  QList<QgsSnappingResult> points;
  for ( QList<QgsSnappingResult>::const_iterator it = results.constBegin();
        it != results.constEnd();
        ++it )
  {
    if ( it->snappedVertexNr == -1 )
    {
      QgsDebugMsg( "segment" );
      segments.push_back( *it );
    }
    else
    {
      QgsDebugMsg( "no segment" );
      points.push_back( *it );
    }
  }

  if ( segments.length() < 2 )
    return 0;

  QList<QgsSnappingResult> myResults;

  for ( QList<QgsSnappingResult>::const_iterator oSegIt = segments.constBegin();
        oSegIt != segments.constEnd();
        ++oSegIt )
  {
    QgsDebugMsg( QString::number( oSegIt->beforeVertexNr ) );

    QVector<QgsPoint> vertexPoints;
    vertexPoints.append( oSegIt->beforeVertex );
    vertexPoints.append( oSegIt->afterVertex );

    QgsGeometry* lineA = QgsGeometry::fromPolyline( vertexPoints );

    for ( QList<QgsSnappingResult>::iterator iSegIt = segments.begin();
          iSegIt != segments.end();
          ++iSegIt )
    {
      QVector<QgsPoint> vertexPoints;
      vertexPoints.append( iSegIt->beforeVertex );
      vertexPoints.append( iSegIt->afterVertex );
      QgsGeometry* lineB = QgsGeometry::fromPolyline( vertexPoints );

      QgsGeometry* intersectionPoint = lineA->intersection( lineB );
      if ( intersectionPoint->type()  == QGis::Point )
      {
        //We have to check the intersection point is inside the tolerance distance for both layers
        double toleranceA = 0;
        double toleranceB = 0;
        for ( int i = 0 ;i < snapLayers.size();++i )
        {
          if ( snapLayers[i].mLayer == oSegIt->layer )
          {
            toleranceA = QgsTolerance::toleranceInMapUnits( snapLayers[i].mTolerance, snapLayers[i].mLayer, mMapCanvas->mapSettings(), snapLayers[i].mUnitType );
          }
          if ( snapLayers[i].mLayer == iSegIt->layer )
          {
            toleranceB = QgsTolerance::toleranceInMapUnits( snapLayers[i].mTolerance, snapLayers[i].mLayer, mMapCanvas->mapSettings(), snapLayers[i].mUnitType );
          }
        }
        QgsGeometry* cursorPoint = QgsGeometry::fromPoint( point );
        double distance = intersectionPoint->distance( *cursorPoint );
        if ( distance < toleranceA && distance < toleranceB )
        {
          iSegIt->snappedVertex = intersectionPoint->asPoint();
          myResults.append( *iSegIt );
        }
      }
    }
  }

  if ( myResults.length() > 0 )
  {
    results.clear();
    results = myResults;
  }

  return 0;
}
