/***************************************************************************
  qgssnappingutils.cpp
  --------------------------------------
  Date                 : November 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnappingutils.h"

#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QSettings>


QgsSnappingUtils::QgsSnappingUtils()
    : mCurrentLayer( 0 )
    , mSnapToMapMode( SnapCurrentLayer )
    , mSnapOnIntersection( false )
{

}

QgsSnappingUtils::~QgsSnappingUtils()
{
  foreach ( QgsPointLocator* vlpl, mLocators )
    delete vlpl;
  mLocators.clear();
}


QgsPointLocator* QgsSnappingUtils::locatorForLayer( QgsVectorLayer* vl )
{
  if ( !vl )
    return 0;

  if ( !mLocators.contains( vl ) )
  {
    QgsPointLocator* vlpl = new QgsPointLocator( vl, destCRS() );
    mLocators.insert( vl, vlpl );
  }
  return mLocators.value( vl );
}


static int _defaultSnapMode()
{
  QSettings settings;
  QString defaultSnapString = settings.value( "/qgis/digitizing/default_snap_mode", "off" ).toString();
  if ( defaultSnapString == "to segment" )
    return QgsPointLocator::Edge;
  else if ( defaultSnapString == "to vertex and segment" )
    return QgsPointLocator::Edge | QgsPointLocator::Vertex;
  else if ( defaultSnapString == "to vertex" )
    return QgsPointLocator::Vertex;
  else
    return 0;
}

// return snap tolerance in map units (not in layer units as from QgsTolerance)
static double _snapTolerance( double tolerance, QgsTolerance::UnitType units, const QgsMapSettings& mapSettings )
{
  if ( units == QgsTolerance::MapUnits )
    return tolerance;
  else // pixels
    return tolerance * mapSettings.mapUnitsPerPixel();
}

// return default snap tolerance in map units (not in layer units as from QgsTolerance)
static double _defaultSnapTolerance( const QgsMapSettings& mapSettings )
{
  QSettings settings;
  return _snapTolerance( settings.value( "/qgis/digitizing/default_snapping_tolerance", 0 ).toDouble(),
                         ( QgsTolerance::UnitType ) settings.value( "/qgis/digitizing/default_snapping_tolerance_unit", 0 ).toInt(),
                         mapSettings );
}


QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QPoint& point )
{
  Q_ASSERT( mMapSettings.hasValidSettings() );

  // TODO: snapping on intersection

  QgsPoint pointMap = mMapSettings.mapToPixel().toMapCoordinates( point );

  if ( mSnapToMapMode == SnapCurrentLayer )
  {
    if ( !mCurrentLayer )
      return QgsPointLocator::Match();

    // data from QSettings
    double tolerance = _defaultSnapTolerance( mMapSettings );
    int type = _defaultSnapMode();

    // use ad-hoc locator
    QgsPointLocator* loc = locatorForLayer( mCurrentLayer );
    loc->init( QgsPointLocator::Vertex | QgsPointLocator::Edge );
    if ( !loc )
      return QgsPointLocator::Match();

    QgsPointLocator::Match bestMatch;
    if ( type & QgsPointLocator::Vertex )
      bestMatch.replaceIfBetter( loc->nearestVertex( pointMap ), tolerance );
    if ( type & QgsPointLocator::Edge )
      bestMatch.replaceIfBetter( loc->nearestEdge( pointMap ), tolerance );
    return bestMatch;
  }
  else if ( mSnapToMapMode == SnapPerLayerConfig )
  {
    QgsPointLocator::Match bestMatch;

    foreach ( const LayerConfig& layerConfig, mLayers )
    {
      double tolerance = _snapTolerance( layerConfig.tolerance, layerConfig.unit, mMapSettings );
      if ( QgsPointLocator* loc = locatorForLayer( layerConfig.layer ) )
      {
        loc->init( layerConfig.type );
        if ( layerConfig.type & QgsPointLocator::Vertex )
          bestMatch.replaceIfBetter( loc->nearestVertex( pointMap ), tolerance );
        if ( layerConfig.type & QgsPointLocator::Edge )
          bestMatch.replaceIfBetter( loc->nearestEdge( pointMap ), tolerance );
      }
    }
    return bestMatch;
  }

  return QgsPointLocator::Match();
}

const QgsCoordinateReferenceSystem* QgsSnappingUtils::destCRS()
{
  return mMapSettings.hasCrsTransformEnabled() ? &mMapSettings.destinationCrs() : 0;
}


void QgsSnappingUtils::readFromProject()
{
  mSnapToMapMode = SnapCurrentLayer;
  mLayers.clear();

  //snapping on intersection on?
  setSnapOnIntersections( QgsProject::instance()->readNumEntry( "Digitizing", "/IntersectionSnapping", 0 ) );

  //read snapping settings from project
  bool snappingDefinedInProject, ok;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", QStringList(), &snappingDefinedInProject );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", QStringList(), &ok );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", QStringList(), &ok );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", QStringList(), &ok );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", QStringList(), &ok );

  // lists must have the same size, otherwise something is wrong
  if ( layerIdList.size() != enabledList.size() ||
       layerIdList.size() != toleranceList.size() ||
       layerIdList.size() != toleranceUnitList.size() ||
       layerIdList.size() != snapToList.size() )
    return;

  if ( !snappingDefinedInProject )
    return; // nothing defined in project - use current layer

  // Use snapping information from the project
  mSnapToMapMode = SnapPerLayerConfig;

  // load layers, tolerances, snap type
  QStringList::const_iterator layerIt( layerIdList.constBegin() );
  QStringList::const_iterator tolIt( toleranceList.constBegin() );
  QStringList::const_iterator tolUnitIt( toleranceUnitList.constBegin() );
  QStringList::const_iterator snapIt( snapToList.constBegin() );
  QStringList::const_iterator enabledIt( enabledList.constBegin() );
  for ( ; layerIt != layerIdList.constEnd(); ++layerIt, ++tolIt, ++tolUnitIt, ++snapIt, ++enabledIt )
  {
    // skip layer if snapping is not enabled
    if ( *enabledIt != "enabled" )
      continue;

    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( QgsMapLayerRegistry::instance()->mapLayer( *layerIt ) );
    if ( !vlayer || !vlayer->hasGeometryType() )
      continue;

    int t = ( *snapIt == "to_vertex" ? QgsPointLocator::Vertex :
              ( *snapIt == "to_segment" ? QgsPointLocator::Edge :
                QgsPointLocator::Vertex | QgsPointLocator::Edge ) );
    mLayers.append( LayerConfig( vlayer, t, tolIt->toDouble(), ( QgsTolerance::UnitType ) tolUnitIt->toInt() ) );
  }

}

