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


QgsMapCanvasSnapper::QgsMapCanvasSnapper( QgsMapCanvas* canvas ): mMapCanvas( canvas ), mSnapper( 0 )
{
  if ( canvas )
  {
    QgsMapRenderer* canvasRender = canvas->mapRenderer();
    if ( canvasRender )
    {
      mSnapper = new QgsSnapper( canvasRender );
    }
  }
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
    mSnapper = new QgsSnapper( canvas->mapRenderer() );
  }
  else
  {
    mSnapper = 0;
  }
}

int QgsMapCanvasSnapper::snapToCurrentLayer( const QPoint& p, QList<QgsSnappingResult>& results, QgsSnapper::SnappingType snap_to, double snappingTol, const QList<QgsPoint>& excludePoints )
{
  results.clear();

  if ( mSnapper && mMapCanvas )
  {

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
    {
      return 2;
    }
    QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
    if ( !vlayer )
    {
      return 3;
    }

    QgsSnapper::SnapLayer snapLayer;
    snapLayer.mLayer = vlayer;
    snapLayer.mSnapTo = snap_to;
    snapLayer.mUnitType = QgsTolerance::MapUnits;

    QSettings settings;

    if ( snappingTol < 0 )
    {
      //use search tolerance for vertex editing
      snapLayer.mTolerance = QgsTolerance::vertexSearchRadius( vlayer, mMapCanvas->mapRenderer() );
    }
    else
    {
      snapLayer.mTolerance = snappingTol;
    }

    QList<QgsSnapper::SnapLayer> snapLayers;
    snapLayers.append( snapLayer );
    mSnapper->setSnapLayers( snapLayers );

    if ( mSnapper->snapPoint( p, results, excludePoints ) != 0 )
    {
      return 4;
    }

    return 0;
  }
  else
  {
    return 1;
  }
}

int QgsMapCanvasSnapper::snapToBackgroundLayers( const QPoint& p, QList<QgsSnappingResult>& results, const QList<QgsPoint>& excludePoints )
{
  results.clear();

  if ( mSnapper )
  {
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

    //read snapping settings from project
    bool ok; //todo: take the default snapping tolerance for all vector layers if snapping not defined in project
    bool snappingDefinedInProject = true;
    QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", &ok );
    if ( !ok )
    {
      snappingDefinedInProject = false;
    }
    QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", &ok );
    QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", &ok );
    QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", &ok );
    QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", &ok );

    if ( !( layerIdList.size() == enabledList.size() && layerIdList.size() == toleranceList.size() && layerIdList.size() == toleranceUnitList.size() && layerIdList.size() == snapToList.size() ) )
    {
      return 1; //lists must have the same size, otherwise something is wrong
    }

    QList<QgsSnapper::SnapLayer> snapLayers;
    QgsSnapper::SnapLayer snapLayer;

    //Use snapping information from the project
    if ( snappingDefinedInProject )
    {
      //set layers, tolerances, snap to segment/vertex to QgsSnapper
      QgsMapLayer* layer = 0;
      QgsVectorLayer* vlayer = 0;

      QStringList::const_iterator layerIt = layerIdList.constBegin();
      QStringList::const_iterator tolIt = toleranceList.constBegin();
      QStringList::const_iterator tolUnitIt = toleranceUnitList.constBegin();
      QStringList::const_iterator snapIt = snapToList.constBegin();
      QStringList::const_iterator enabledIt = enabledList.constBegin();

      for ( ; layerIt != layerIdList.constEnd(); ++layerIt, ++tolIt, ++tolUnitIt, ++snapIt, ++enabledIt )
      {
        if (( *enabledIt ) != "enabled" ) //skip layer if snapping is not enabled
        {
          continue;
        }

        //layer
        layer = QgsMapLayerRegistry::instance()->mapLayer( *layerIt );
        if ( layer )
        {
          vlayer = qobject_cast<QgsVectorLayer *>( layer );
          if ( vlayer )
          {
            snapLayer.mLayer = vlayer;
          }
        }

        //tolerance
        snapLayer.mTolerance = tolIt->toDouble();
        snapLayer.mUnitType = ( QgsTolerance::UnitType ) tolUnitIt->toInt();

        //segment or vertex
        if (( *snapIt ) == "to_vertex" )
        {
          snapLayer.mSnapTo = QgsSnapper::SnapToVertex;
        }
        else if (( *snapIt ) == "to_segment" )
        {
          snapLayer.mSnapTo = QgsSnapper::SnapToSegment;
        }
        else //to vertex and segment
        {
          snapLayer.mSnapTo = QgsSnapper::SnapToVertexAndSegment;
        }

        snapLayers.append( snapLayer );
      }
    }
    else //nothing in project. Use default snapping tolerance to vertex of current layer
    {
      QgsMapLayer* currentLayer = mMapCanvas->currentLayer();
      if ( !currentLayer )
      {
        return 2;
      }

      QgsVectorLayer* currentVectorLayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( !currentVectorLayer )
      {
        return 3;
      }

      snapLayer.mLayer = currentVectorLayer;
      QSettings settings;

      //default snap mode
      QString defaultSnapString = settings.value( "/qgis/digitizing/default_snap_mode", "to vertex" ).toString();
      if ( defaultSnapString == "to segment" )
      {
        snapLayer.mSnapTo = QgsSnapper::SnapToSegment;
      }
      else if ( defaultSnapString == "to vertex and segment" )
      {
        snapLayer.mSnapTo = QgsSnapper::SnapToVertexAndSegment;
      }
      else
      {
        snapLayer.mSnapTo = QgsSnapper::SnapToVertex;
      }

      //default snapping tolerance (returned in map units)
      snapLayer.mTolerance = QgsTolerance::defaultTolerance( currentVectorLayer, mMapCanvas->mapRenderer() );
      snapLayer.mUnitType = QgsTolerance::MapUnits;

      snapLayers.append( snapLayer );
    }

    mSnapper->setSnapLayers( snapLayers );

    if ( mSnapper->snapPoint( p, results, excludePoints ) != 0 )
    {
      return 4;
    }
    return 0;
  }
  else
  {
    return 5;
  }
}
