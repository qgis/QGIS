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
    , mStrategy( IndexHybrid )
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
  Q_FOREACH ( QgsPointLocator* vlpl, mLocators )
    delete vlpl;
  mLocators.clear();

  Q_FOREACH ( QgsPointLocator* vlpl, mTemporaryLocators )
    delete vlpl;
  mTemporaryLocators.clear();
}


QgsPointLocator* QgsSnappingUtils::locatorForLayerUsingStrategy( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance )
{
  if ( willUseIndex( vl ) )
    return locatorForLayer( vl );
  else
    return temporaryLocatorForLayer( vl, pointMap, tolerance );
}

QgsPointLocator* QgsSnappingUtils::temporaryLocatorForLayer( QgsVectorLayer* vl, const QgsPoint& pointMap, double tolerance )
{
  if ( mTemporaryLocators.contains( vl ) )
    delete mTemporaryLocators.take( vl );

  QgsRectangle rect( pointMap.x() - tolerance, pointMap.y() - tolerance,
                     pointMap.x() + tolerance, pointMap.y() + tolerance );
  QgsPointLocator* vlpl = new QgsPointLocator( vl, destCRS(), &rect );
  mTemporaryLocators.insert( vl, vlpl );
  return mTemporaryLocators.value( vl );
}

bool QgsSnappingUtils::willUseIndex( QgsVectorLayer* vl ) const
{
  if ( mStrategy == IndexAlwaysFull )
    return true;
  else if ( mStrategy == IndexNeverFull )
    return false;
  else
  {
    if ( mHybridNonindexableLayers.contains( vl->id() ) )
      return false;

    // if the layer is too big, the locator will later stop indexing it after reaching a threshold
    return true;
  }
}


static QgsPointLocator::Match _findClosestSegmentIntersection( const QgsPoint& pt, const QgsPointLocator::MatchList& segments )
{
  if ( segments.isEmpty() )
    return QgsPointLocator::Match();

  QSet<QgsPoint> endpoints;

  // make a geometry
  QList<QgsGeometry*> geoms;
  Q_FOREACH ( const QgsPointLocator::Match& m, segments )
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
    Q_FOREACH ( const QgsPoint& p, g->asPolyline() )
    {
      if ( !endpoints.contains( p ) )
        newPoints << p;
    }
  }
  if ( g->wkbType() == QGis::WKBMultiLineString )
  {
    Q_FOREACH ( const QgsPolyline& pl, g->asMultiPolyline() )
    {
      Q_FOREACH ( const QgsPoint& p, pl )
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
  Q_FOREACH ( const QgsPoint& p, newPoints )
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


static void _replaceIfBetter( QgsPointLocator::Match& mBest, const QgsPointLocator::Match& mNew, double maxDistance )
{
  // is other match relevant?
  if ( !mNew.isValid() || mNew.distance() > maxDistance )
    return;

  // is other match actually better?
  if ( mBest.isValid() && mBest.type() == mNew.type() && mBest.distance() - 10e-6 < mNew.distance() )
    return;

  // prefer vertex matches to edge matches (even if they are closer)
  if ( mBest.type() == QgsPointLocator::Vertex && mNew.type() == QgsPointLocator::Edge )
    return;

  mBest = mNew; // the other match is better!
}


static void _updateBestMatch( QgsPointLocator::Match& bestMatch, const QgsPoint& pointMap, QgsPointLocator* loc, int type, double tolerance, QgsPointLocator::MatchFilter* filter )
{
  if ( type & QgsPointLocator::Vertex )
  {
    _replaceIfBetter( bestMatch, loc->nearestVertex( pointMap, tolerance, filter ), tolerance );
  }
  if ( bestMatch.type() != QgsPointLocator::Vertex && ( type & QgsPointLocator::Edge ) )
  {
    _replaceIfBetter( bestMatch, loc->nearestEdge( pointMap, tolerance, filter ), tolerance );
  }
}


QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QPoint& point, QgsPointLocator::MatchFilter* filter )
{
  return snapToMap( mMapSettings.mapToPixel().toMapCoordinates( point ), filter );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QgsPoint& pointMap, QgsPointLocator::MatchFilter* filter )
{
  if ( !mMapSettings.hasValidSettings() )
    return QgsPointLocator::Match();

  if ( mSnapToMapMode == SnapCurrentLayer )
  {
    if ( !mCurrentLayer )
      return QgsPointLocator::Match();

    prepareIndex( QList<QgsVectorLayer*>() << mCurrentLayer );

    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mDefaultTolerance, mCurrentLayer, mMapSettings, mDefaultUnit );
    int type = mDefaultType;

    // use ad-hoc locator
    QgsPointLocator* loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
    if ( !loc )
      return QgsPointLocator::Match();

    QgsPointLocator::Match bestMatch;
    _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );

    if ( mSnapOnIntersection )
    {
      QgsPointLocator* locEdges = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
      QgsPointLocator::MatchList edges = locEdges->edgesInRect( pointMap, tolerance );
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );
    }

    return bestMatch;
  }
  else if ( mSnapToMapMode == SnapAdvanced )
  {
    QList<QgsVectorLayer*> layers;
    Q_FOREACH ( const LayerConfig& layerConfig, mLayers )
      layers << layerConfig.layer;
    prepareIndex( layers );

    QgsPointLocator::Match bestMatch;
    QgsPointLocator::MatchList edges; // for snap on intersection
    double maxSnapIntTolerance = 0;

    Q_FOREACH ( const LayerConfig& layerConfig, mLayers )
    {
      double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
      if ( QgsPointLocator* loc = locatorForLayerUsingStrategy( layerConfig.layer, pointMap, tolerance ) )
      {
        _updateBestMatch( bestMatch, pointMap, loc, layerConfig.type, tolerance, filter );

        if ( mSnapOnIntersection )
        {
          edges << loc->edgesInRect( pointMap, tolerance );
          maxSnapIntTolerance = qMax( maxSnapIntTolerance, tolerance );
        }
      }
    }

    if ( mSnapOnIntersection )
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), maxSnapIntTolerance );

    return bestMatch;
  }
  else if ( mSnapToMapMode == SnapAllLayers )
  {
    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mDefaultTolerance, 0, mMapSettings, mDefaultUnit );
    int type = mDefaultType;

    QList<QgsVectorLayer*> layers;
    Q_FOREACH ( const QString& layerID, mMapSettings.layers() )
      if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) ) )
        layers << vl;
    prepareIndex( layers );

    QgsPointLocator::MatchList edges; // for snap on intersection
    QgsPointLocator::Match bestMatch;

    Q_FOREACH ( QgsVectorLayer* vl, layers )
    {
      if ( QgsPointLocator* loc = locatorForLayerUsingStrategy( vl, pointMap, tolerance ) )
      {
        _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );

        if ( mSnapOnIntersection )
          edges << loc->edgesInRect( pointMap, tolerance );
      }
    }

    if ( mSnapOnIntersection )
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );

    return bestMatch;
  }

  return QgsPointLocator::Match();
}


void QgsSnappingUtils::prepareIndex( const QList<QgsVectorLayer*>& layers )
{
  // check if we need to build any index
  QList<QgsVectorLayer*> layersToIndex;
  Q_FOREACH ( QgsVectorLayer* vl, layers )
  {
    if ( willUseIndex( vl ) && !locatorForLayer( vl )->hasIndex() )
      layersToIndex << vl;
  }
  if ( layersToIndex.isEmpty() )
    return;

  // build indexes
  QTime t; t.start();
  int i = 0;
  prepareIndexStarting( layersToIndex.count() );
  Q_FOREACH ( QgsVectorLayer* vl, layersToIndex )
  {
    QTime tt; tt.start();
    if ( !locatorForLayer( vl )->init( mStrategy == IndexHybrid ? 1000000 : -1 ) )
      mHybridNonindexableLayers.insert( vl->id() );
    QgsDebugMsg( QString( "Index init: %1 ms (%2)" ).arg( tt.elapsed() ).arg( vl->id() ) );
    prepareIndexProgress( ++i );
  }
  QgsDebugMsg( QString( "Prepare index total: %1 ms" ).arg( t.elapsed() ) );
}


QgsPointLocator::Match QgsSnappingUtils::snapToCurrentLayer( const QPoint& point, int type, QgsPointLocator::MatchFilter* filter )
{
  if ( !mCurrentLayer )
    return QgsPointLocator::Match();

  QgsPoint pointMap = mMapSettings.mapToPixel().toMapCoordinates( point );
  double tolerance = QgsTolerance::vertexSearchRadius( mMapSettings );

  QgsPointLocator* loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
  if ( !loc )
    return QgsPointLocator::Match();

  QgsPointLocator::Match bestMatch;
  _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );
  return bestMatch;
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
  // force map units - can't use layer units for just any layer
  if ( unit == QgsTolerance::LayerUnits )
    unit = QgsTolerance::ProjectUnits;

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
  QgsTolerance::UnitType unit = ( QgsTolerance::UnitType ) QgsProject::instance()->readNumEntry( "Digitizing", "/DefaultSnapToleranceUnit", QgsTolerance::ProjectUnits );
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
  else if ( snapMode == "all_layers" )
    mSnapToMapMode = SnapAllLayers;
  else   // either "advanced" or empty (for background compatibility)
    mSnapToMapMode = SnapAdvanced;



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
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    for ( LocatorsMap::iterator it = mLocators.begin(); it != mLocators.end(); )
    {
      if ( it.key()->id() == layerId )
      {
        delete it.value();
        it = mLocators.erase( it );
      }
      else
      {
        ++it;
      }
    }

    for ( LocatorsMap::iterator it = mTemporaryLocators.begin(); it != mTemporaryLocators.end(); )
    {
      if ( it.key()->id() == layerId )
      {
        delete it.value();
        it = mTemporaryLocators.erase( it );
      }
      else
      {
        ++it;
      }
    }
  }
}

