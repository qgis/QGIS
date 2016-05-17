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
    , mCurrentLayer( nullptr )
    , mSnapToMapMode( SnapCurrentLayer )
    , mStrategy( IndexHybrid )
    , mDefaultType( QgsPointLocator::Vertex )
    , mDefaultTolerance( 10 )
    , mDefaultUnit( QgsTolerance::Pixels )
    , mSnapOnIntersection( false )
    , mHybridPerLayerFeatureLimit( 50000 )
    , mIsIndexing( false )
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
    return nullptr;

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
  QgsRectangle aoi( pointMap.x() - tolerance, pointMap.y() - tolerance,
                    pointMap.x() + tolerance, pointMap.y() + tolerance );
  if ( isIndexPrepared( vl, aoi ) )
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

bool QgsSnappingUtils::isIndexPrepared( QgsVectorLayer* vl, const QgsRectangle& areaOfInterest )
{
  if ( vl->geometryType() == QGis::NoGeometry || mStrategy == IndexNeverFull )
    return false;

  QgsPointLocator* loc = locatorForLayer( vl );

  if ( mStrategy == IndexAlwaysFull && loc->hasIndex() )
    return true;

  if ( mStrategy == IndexHybrid && loc->hasIndex() && ( !loc->extent() || loc->extent()->contains( areaOfInterest ) ) )
    return true;

  return false; // the index - even if it exists - is not suitable
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

  return QgsPointLocator::Match( QgsPointLocator::Vertex, nullptr, 0, sqrt( minSqrDist ), minP );
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


QgsPointLocator::Match QgsSnappingUtils::snapToMap( QPoint point, QgsPointLocator::MatchFilter* filter )
{
  return snapToMap( mMapSettings.mapToPixel().toMapCoordinates( point ), filter );
}

inline QgsRectangle _areaOfInterest( const QgsPoint& point, double tolerance )
{
  return QgsRectangle( point.x() - tolerance, point.y() - tolerance,
                       point.x() + tolerance, point.y() + tolerance );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QgsPoint& pointMap, QgsPointLocator::MatchFilter* filter )
{
  if ( !mMapSettings.hasValidSettings() )
    return QgsPointLocator::Match();

  if ( mSnapToMapMode == SnapCurrentLayer )
  {
    if ( !mCurrentLayer || mDefaultType == 0 )
      return QgsPointLocator::Match();

    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mDefaultTolerance, mCurrentLayer, mMapSettings, mDefaultUnit );
    int type = mDefaultType;

    prepareIndex( QList<LayerAndAreaOfInterest>() << qMakePair( mCurrentLayer, _areaOfInterest( pointMap, tolerance ) ) );

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
    QList<LayerAndAreaOfInterest> layers;
    Q_FOREACH ( const LayerConfig& layerConfig, mLayers )
    {
      double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
      layers << qMakePair( layerConfig.layer, _areaOfInterest( pointMap, tolerance ) );
    }
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
    double tolerance = QgsTolerance::toleranceInProjectUnits( mDefaultTolerance, nullptr, mMapSettings, mDefaultUnit );
    int type = mDefaultType;
    QgsRectangle aoi = _areaOfInterest( pointMap, tolerance );

    QList<LayerAndAreaOfInterest> layers;
    Q_FOREACH ( const QString& layerID, mMapSettings.layers() )
      if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) ) )
        layers << qMakePair( vl, aoi );
    prepareIndex( layers );

    QgsPointLocator::MatchList edges; // for snap on intersection
    QgsPointLocator::Match bestMatch;

    Q_FOREACH ( const LayerAndAreaOfInterest& entry, layers )
    {
      QgsVectorLayer* vl = entry.first;
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


void QgsSnappingUtils::prepareIndex( const QList<LayerAndAreaOfInterest>& layers )
{
  if ( mIsIndexing )
    return;
  mIsIndexing = true;

  // check if we need to build any index
  QList<LayerAndAreaOfInterest> layersToIndex;
  Q_FOREACH ( const LayerAndAreaOfInterest& entry, layers )
  {
    QgsVectorLayer* vl = entry.first;
    if ( vl->geometryType() == QGis::NoGeometry || mStrategy == IndexNeverFull )
      continue;

    if ( !isIndexPrepared( vl, entry.second ) )
      layersToIndex << entry;
  }
  if ( !layersToIndex.isEmpty() )
  {
    // build indexes
    QTime t;
    t.start();
    int i = 0;
    prepareIndexStarting( layersToIndex.count() );
    Q_FOREACH ( const LayerAndAreaOfInterest& entry, layersToIndex )
    {
      QgsVectorLayer* vl = entry.first;
      QTime tt;
      tt.start();
      QgsPointLocator* loc = locatorForLayer( vl );
      if ( mStrategy == IndexHybrid )
      {
        // first time the layer is used? - let's set an initial guess about indexing
        if ( !mHybridMaxAreaPerLayer.contains( vl->id() ) )
        {
          int totalFeatureCount = vl->pendingFeatureCount();
          if ( totalFeatureCount < mHybridPerLayerFeatureLimit )
          {
            // index the whole layer
            mHybridMaxAreaPerLayer[vl->id()] = -1;
          }
          else
          {
            // estimate for how big area it probably makes sense to build partial index to not exceed the limit
            // (we may change the limit later)
            QgsRectangle layerExtent = mMapSettings.layerExtentToOutputExtent( vl, vl->extent() );
            double totalArea = layerExtent.width() * layerExtent.height();
            mHybridMaxAreaPerLayer[vl->id()] = totalArea * mHybridPerLayerFeatureLimit / totalFeatureCount / 4;
          }
        }

        double indexReasonableArea = mHybridMaxAreaPerLayer[vl->id()];
        if ( indexReasonableArea == -1 )
        {
          // we can safely index the whole layer
          loc->init();
        }
        else
        {
          // use area as big as we think may fit into our limit
          QgsPoint c = entry.second.center();
          double halfSide = sqrt( indexReasonableArea ) / 2;
          QgsRectangle rect( c.x() - halfSide, c.y() - halfSide,
                             c.x() + halfSide, c.y() + halfSide );
          loc->setExtent( &rect );

          // see if it's possible build index for this area
          if ( !loc->init( mHybridPerLayerFeatureLimit ) )
          {
            // hmm that didn't work out - too many features!
            // let's make the allowed area smaller for the next time
            mHybridMaxAreaPerLayer[vl->id()] /= 4;
          }
        }

      }
      else  // full index strategy
        loc->init();

      QgsDebugMsg( QString( "Index init: %1 ms (%2)" ).arg( tt.elapsed() ).arg( vl->id() ) );
      prepareIndexProgress( ++i );
    }
    QgsDebugMsg( QString( "Prepare index total: %1 ms" ).arg( t.elapsed() ) );
  }
  mIsIndexing = false;
}


QgsPointLocator::Match QgsSnappingUtils::snapToCurrentLayer( QPoint point, int type, QgsPointLocator::MatchFilter* filter )
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

void QgsSnappingUtils::setCurrentLayer( QgsVectorLayer* layer )
{
  mCurrentLayer = layer;
}

void QgsSnappingUtils::setSnapToMapMode( QgsSnappingUtils::SnapToMapMode mode )
{
  if ( mSnapToMapMode == mode )
    return;

  mSnapToMapMode = mode;
  emit configChanged();
}

void QgsSnappingUtils::setDefaultSettings( int type, double tolerance, QgsTolerance::UnitType unit )
{
  // force map units - can't use layer units for just any layer
  if ( unit == QgsTolerance::LayerUnits )
    unit = QgsTolerance::ProjectUnits;

  if ( mDefaultType == type && mDefaultTolerance == tolerance && mDefaultUnit == unit )
    return;

  mDefaultType = type;
  mDefaultTolerance = tolerance;
  mDefaultUnit = unit;

  if ( mSnapToMapMode != SnapAdvanced ) // does not affect advanced mode
    emit configChanged();
}

void QgsSnappingUtils::defaultSettings( int& type, double& tolerance, QgsTolerance::UnitType& unit )
{
  type = mDefaultType;
  tolerance = mDefaultTolerance;
  unit = mDefaultUnit;
}

void QgsSnappingUtils::setLayers( const QList<QgsSnappingUtils::LayerConfig>& layers )
{
  if ( mLayers == layers )
    return;

  mLayers = layers;
  if ( mSnapToMapMode == SnapAdvanced ) // only affects advanced mode
    emit configChanged();
}

void QgsSnappingUtils::setSnapOnIntersections( bool enabled )
{
  if ( mSnapOnIntersection == enabled )
    return;

  mSnapOnIntersection = enabled;
  emit configChanged();
}

QString QgsSnappingUtils::dump()
{
  QString msg = "--- SNAPPING UTILS DUMP ---\n";

  if ( !mMapSettings.hasValidSettings() )
  {
    msg += "invalid map settings!";
    return msg;
  }

  QList<LayerConfig> layers;

  if ( mSnapToMapMode == SnapCurrentLayer )
  {
    if ( mSnapToMapMode == SnapCurrentLayer && !mCurrentLayer )
    {
      msg += "no current layer!";
      return msg;
    }

    layers << LayerConfig( mCurrentLayer, QgsPointLocator::Types( mDefaultType ), mDefaultTolerance, mDefaultUnit );
  }
  else if ( mSnapToMapMode == SnapAllLayers )
  {
    Q_FOREACH ( const QString& layerID, mMapSettings.layers() )
    {
      if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) ) )
        layers << LayerConfig( vl, QgsPointLocator::Types( mDefaultType ), mDefaultTolerance, mDefaultUnit );
    }
  }
  else if ( mSnapToMapMode == SnapAdvanced )
  {
    layers = mLayers;
  }

  Q_FOREACH ( const LayerConfig& layer, layers )
  {
    msg += QString( "layer : %1\n"
                    "config: %2   tolerance %3 %4\n" )
           .arg( layer.layer->name() )
           .arg( layer.type ).arg( layer.tolerance ).arg( layer.unit );

    if ( mStrategy == IndexAlwaysFull || mStrategy == IndexHybrid )
    {
      if ( QgsPointLocator* loc = locatorForLayer( layer.layer ) )
      {
        QString extentStr, cachedGeoms, limit( "no max area" );
        if ( const QgsRectangle* r = loc->extent() )
        {
          extentStr = QString( " extent %1" ).arg( r->toString() );
        }
        else
          extentStr = "full extent";
        if ( loc->hasIndex() )
          cachedGeoms = QString( "%1 feats" ).arg( loc->cachedGeometryCount() );
        else
          cachedGeoms = "not initialized";
        if ( mStrategy == IndexHybrid )
        {
          if ( mHybridMaxAreaPerLayer.contains( layer.layer->id() ) )
          {
            double maxArea = mStrategy == IndexHybrid ? mHybridMaxAreaPerLayer[layer.layer->id()] : -1;
            if ( maxArea != -1 )
              limit = QString( "max area %1" ).arg( maxArea );
          }
          else
            limit = "not evaluated";
        }
        msg += QString( "index : YES | %1 | %2 | %3\n" ).arg( cachedGeoms, extentStr, limit );
      }
      else
        msg += QString( "index : ???\n" ); // should not happen
    }
    else
      msg += "index : NO\n";
    msg += "-\n";
  }

  return msg;
}

const QgsCoordinateReferenceSystem* QgsSnappingUtils::destCRS()
{
  return mMapSettings.hasCrsTransformEnabled() ? &mMapSettings.destinationCrs() : nullptr;
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
  QgsTolerance::UnitType unit = static_cast< QgsTolerance::UnitType >( QgsProject::instance()->readNumEntry( "Digitizing", "/DefaultSnapToleranceUnit", QgsTolerance::ProjectUnits ) );
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

    QgsPointLocator::Types t( *snapIt == "to_vertex" ? QgsPointLocator::Vertex :
                              ( *snapIt == "to_segment" ? QgsPointLocator::Edge :
                                QgsPointLocator::Vertex | QgsPointLocator::Edge
                              )
                            );
    mLayers.append( LayerConfig( vlayer, t, tolIt->toDouble(), static_cast< QgsTolerance::UnitType >( tolUnitIt->toInt() ) ) );
  }

  emit configChanged();
}

void QgsSnappingUtils::onLayersWillBeRemoved( const QStringList& layerIds )
{
  // remove locators for layers that are going to be deleted
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    if ( mHybridMaxAreaPerLayer.contains( layerId ) )
      mHybridMaxAreaPerLayer.remove( layerId );

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

