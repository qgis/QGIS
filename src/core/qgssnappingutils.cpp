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

#include "qgsgeometry.h"
#include "qgsmaplayerregistry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"


QgsSnappingUtils::QgsSnappingUtils( QObject* parent )
    : QObject( parent )
    , mCurrentLayer( 0 )
    , mSnapToMapMode( SnapCurrentLayer )
    , mDefaultType( QgsPointLocator::Vertex )
    , mDefaultTolerance( 10 )
    , mDefaultUnit( QgsTolerance::Pixels )
    , mSnapOnIntersection( false )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( onLayersWillBeRemoved( QStringList ) ) );
}

QgsSnappingUtils::~QgsSnappingUtils()
{
  clearAllLocators();
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

void QgsSnappingUtils::clearAllLocators()
{
  foreach ( QgsPointLocator* vlpl, mLocators )
    delete vlpl;
  mLocators.clear();
}


// return snap tolerance in map units (not in layer units as from QgsTolerance)
static double _snapTolerance( double tolerance, QgsTolerance::UnitType units, const QgsMapSettings& mapSettings )
{
  if ( units == QgsTolerance::MapUnits )
    return tolerance;
  else // pixels
    return tolerance * mapSettings.mapUnitsPerPixel();
}


static QgsPointLocator::Match _findClosestSegmentIntersection( const QgsPoint& pt, const QgsPointLocator::MatchList& segments )
{
  QSet<QgsPoint> endpoints;

  // make a geometry
  QList<QgsGeometry*> geoms;
  foreach ( const QgsPointLocator::Match& m, segments )
  {
    if ( m.hasEdge() )
    {
      QgsPolyline pl( 2 );
      m.edgePoints( pl[0], pl[1] );
      geoms << QgsGeometry::fromPolyline( pl );
      endpoints << pl[0] << pl[1];
    }
  }

  QgsGeometry* g = QgsGeometry::unaryUnion( geoms );
  qDeleteAll( geoms );

  // get intersection points
  QList<QgsPoint> newPoints;
  if ( g->wkbType() == QGis::WKBLineString )
  {
    foreach ( const QgsPoint& p, g->asPolyline() )
    {
      if ( !endpoints.contains( p ) )
        newPoints << p;
    }
  }
  if ( g->wkbType() == QGis::WKBMultiLineString )
  {
    foreach ( const QgsPolyline& pl, g->asMultiPolyline() )
    {
      foreach ( const QgsPoint& p, pl )
      {
        if ( !endpoints.contains( p ) )
          newPoints << p;
      }
    }
  }
  delete g;

  if ( newPoints.isEmpty() )
    return QgsPointLocator::Match();

  // find the closest points
  QgsPoint minP;
  double minSqrDist = 1e20;  // "infinity"
  foreach ( const QgsPoint& p, newPoints )
  {
    double sqrDist = pt.sqrDist( p.x(), p.y() );
    if ( sqrDist < minSqrDist )
    {
      minSqrDist = sqrDist;
      minP = p;
    }
  }

  return QgsPointLocator::Match( QgsPointLocator::Vertex, 0, 0, sqrt( minSqrDist ), minP );
}


QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QPoint& point )
{
  return snapToMap( mMapSettings.mapToPixel().toMapCoordinates( point ) );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QgsPoint& pointMap )
{
  Q_ASSERT( mMapSettings.hasValidSettings() );

  if ( mSnapToMapMode == SnapCurrentLayer )
  {
    if ( !mCurrentLayer )
      return QgsPointLocator::Match();

    // data from project
    double tolerance = _snapTolerance( mDefaultTolerance, mDefaultUnit, mMapSettings );
    int type = mDefaultType;

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

    if ( mSnapOnIntersection )
    {
      QgsPointLocator::MatchList edges = locatorForLayer( mCurrentLayer )->edgesInTolerance( pointMap, tolerance );
      bestMatch.replaceIfBetter( _findClosestSegmentIntersection( pointMap, edges ), tolerance );
    }

    return bestMatch;
  }
  else if ( mSnapToMapMode == SnapPerLayerConfig )
  {
    QgsPointLocator::Match bestMatch;
    QgsPointLocator::MatchList edges; // for snap on intersection
    double maxSnapIntTolerance = 0;

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

        if ( mSnapOnIntersection )
        {
          edges << loc->edgesInTolerance( pointMap, tolerance );
          maxSnapIntTolerance = qMax( maxSnapIntTolerance, tolerance );
        }
      }
    }

    if ( mSnapOnIntersection )
      bestMatch.replaceIfBetter( _findClosestSegmentIntersection( pointMap, edges ), maxSnapIntTolerance );

    return bestMatch;
  }

  return QgsPointLocator::Match();
}

void QgsSnappingUtils::setMapSettings( const QgsMapSettings& settings )
{
  QString oldDestCRS = mMapSettings.hasCrsTransformEnabled() ? mMapSettings.destinationCrs().authid() : QString();
  QString newDestCRS = settings.hasCrsTransformEnabled() ? settings.destinationCrs().authid() : QString();
  mMapSettings = settings;

  if ( newDestCRS != oldDestCRS )
    clearAllLocators();
}

void QgsSnappingUtils::setDefaultSettings( int type, double tolerance, QgsTolerance::UnitType unit )
{
  mDefaultType = type;
  mDefaultTolerance = tolerance;
  mDefaultUnit = unit;
}

void QgsSnappingUtils::defaultSettings( int& type, double& tolerance, QgsTolerance::UnitType& unit )
{
  type = mDefaultType;
  tolerance = mDefaultTolerance;
  unit = mDefaultUnit;
}

const QgsCoordinateReferenceSystem* QgsSnappingUtils::destCRS()
{
  return mMapSettings.hasCrsTransformEnabled() ? &mMapSettings.destinationCrs() : 0;
}


void QgsSnappingUtils::readConfigFromProject()
{
  mSnapToMapMode = SnapCurrentLayer;
  mLayers.clear();

  QString snapMode = QgsProject::instance()->readEntry( "Digitizing", "/SnappingMode" );

  int type = 0;
  QString snapType = QgsProject::instance()->readEntry( "Digitizing", "/DefaultSnapType", QString( "off" ) );
  if ( snapType == "to segment" )
    type = QgsPointLocator::Edge;
  else if ( snapType == "to vertex and segment" )
    type = QgsPointLocator::Vertex | QgsPointLocator::Edge;
  else if ( snapType == "to vertex" )
    type = QgsPointLocator::Vertex;
  double tolerance = QgsProject::instance()->readDoubleEntry( "Digitizing", "/DefaultSnapTolerance", 0 );
  QgsTolerance::UnitType unit = ( QgsTolerance::UnitType ) QgsProject::instance()->readNumEntry( "Digitizing", "/DefaultSnapToleranceUnit", 0 );
  setDefaultSettings( type, tolerance, unit );

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
  if ( snapMode == "current_layer" )
    mSnapToMapMode = SnapCurrentLayer;
  else   // either "advanced" or empty (for background compatibility)
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

void QgsSnappingUtils::onLayersWillBeRemoved( QStringList layerIds )
{
  // remove locators for layers that are going to be deleted
  foreach ( QString layerId, layerIds )
  {
    for ( LocatorsMap::const_iterator it = mLocators.constBegin(); it != mLocators.constEnd(); ++it )
    {
      if ( it.key()->id() == layerId )
      {
        delete mLocators.take( it.key() );
        continue;
      }
    }
  }
}

