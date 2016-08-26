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
#include "qgslogger.h"

QgsSnappingUtils::QgsSnappingUtils( QObject* parent )
    : QObject( parent )
    , mCurrentLayer( nullptr )
    , mStrategy( IndexHybrid )
    , mHybridPerLayerFeatureLimit( 50000 )
    , mIsIndexing( false )
{
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
    QgsPointLocator* vlpl = new QgsPointLocator( vl, destinationCrs() );
    mLocators.insert( vl, vlpl );
  }
  return mLocators.value( vl );
}

void QgsSnappingUtils::clearAllLocators()
{
  qDeleteAll( mLocators );
  mLocators.clear();

  qDeleteAll( mTemporaryLocators );
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
  QgsPointLocator* vlpl = new QgsPointLocator( vl, destinationCrs(), &rect );
  mTemporaryLocators.insert( vl, vlpl );
  return mTemporaryLocators.value( vl );
}

bool QgsSnappingUtils::isIndexPrepared( QgsVectorLayer* vl, const QgsRectangle& areaOfInterest )
{
  if ( vl->geometryType() == QgsWkbTypes::NullGeometry || mStrategy == IndexNeverFull )
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
  QList<QgsGeometry> geoms;
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

  QgsGeometry g = QgsGeometry::unaryUnion( geoms );

  // get intersection points
  QList<QgsPoint> newPoints;
  if ( g.wkbType() == QgsWkbTypes::LineString )
  {
    Q_FOREACH ( const QgsPoint& p, g.asPolyline() )
    {
      if ( !endpoints.contains( p ) )
        newPoints << p;
    }
  }
  if ( g.wkbType() == QgsWkbTypes::MultiLineString )
  {
    Q_FOREACH ( const QgsPolyline& pl, g.asMultiPolyline() )
    {
      Q_FOREACH ( const QgsPoint& p, pl )
      {
        if ( !endpoints.contains( p ) )
          newPoints << p;
      }
    }
  }

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
  if ( !mMapSettings.hasValidSettings() || !mSnappingConfig.enabled() )
    return QgsPointLocator::Match();

  if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer )
  {
    if ( !mCurrentLayer || mSnappingConfig.type() == 0 )
      return QgsPointLocator::Match();

    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mSnappingConfig.tolerance(), mCurrentLayer, mMapSettings, mSnappingConfig.units() );
    int type = mSnappingConfig.type();

    prepareIndex( QList<LayerAndAreaOfInterest>() << qMakePair( mCurrentLayer, _areaOfInterest( pointMap, tolerance ) ) );

    // use ad-hoc locator
    QgsPointLocator* loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
    if ( !loc )
      return QgsPointLocator::Match();

    QgsPointLocator::Match bestMatch;
    _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );

    if ( mSnappingConfig.intersectionSnapping() )
    {
      QgsPointLocator* locEdges = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
      QgsPointLocator::MatchList edges = locEdges->edgesInRect( pointMap, tolerance );
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );
    }

    return bestMatch;
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AdvancedConfiguration )
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

        if ( mSnappingConfig.intersectionSnapping() )
        {
          edges << loc->edgesInRect( pointMap, tolerance );
          maxSnapIntTolerance = qMax( maxSnapIntTolerance, tolerance );
        }
      }
    }

    if ( mSnappingConfig.intersectionSnapping() )
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), maxSnapIntTolerance );

    return bestMatch;
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AllLayers )
  {
    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mSnappingConfig.tolerance(), nullptr, mMapSettings, mSnappingConfig.units() );
    int type = mSnappingConfig.type();
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

        if ( mSnappingConfig.intersectionSnapping() )
          edges << loc->edgesInRect( pointMap, tolerance );
      }
    }

    if ( mSnappingConfig.intersectionSnapping() )
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
    if ( vl->geometryType() == QgsWkbTypes::NullGeometry || mStrategy == IndexNeverFull )
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

QgsSnappingConfig QgsSnappingUtils::config() const
{
  return mSnappingConfig;
}

void QgsSnappingUtils::setConfig( const QgsSnappingConfig& config )
{
  if ( mSnappingConfig == config )
    return;

  if ( mSnappingConfig.individualLayerSettings() != config.individualLayerSettings() )
    onIndividualLayerSettingsChanged( config.individualLayerSettings() );

  mSnappingConfig = config;
  emit configChanged();
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

QString QgsSnappingUtils::dump()
{
  QString msg = "--- SNAPPING UTILS DUMP ---\n";

  if ( !mMapSettings.hasValidSettings() )
  {
    msg += "invalid map settings!";
    return msg;
  }

  QList<LayerConfig> layers;

  if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer )
  {
    if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer && !mCurrentLayer )
    {
      msg += "no current layer!";
      return msg;
    }

    layers << LayerConfig( mCurrentLayer, QgsPointLocator::Types( mSnappingConfig.type() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AllLayers )
  {
    Q_FOREACH ( const QString& layerID, mMapSettings.layers() )
    {
      if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) ) )
        layers << LayerConfig( vl, QgsPointLocator::Types( mSnappingConfig.type() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
    }
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AdvancedConfiguration )
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

QgsCoordinateReferenceSystem QgsSnappingUtils::destinationCrs() const
{
  return mMapSettings.hasCrsTransformEnabled() ? mMapSettings.destinationCrs() : QgsCoordinateReferenceSystem();
}

void QgsSnappingUtils::onIndividualLayerSettingsChanged( const QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings> layerSettings )
{
  mLayers.clear();

  QHash<QgsVectorLayer*, QgsSnappingConfig::IndividualLayerSettings>::const_iterator i;

  for ( i = layerSettings.constBegin(); i != layerSettings.constEnd(); ++i )
  {
    if ( i->enabled() )
    {
      QgsPointLocator::Types t( i->type() == QgsSnappingConfig::Vertex ? QgsPointLocator::Vertex :
                                ( i->type() == QgsSnappingConfig::Segment ? QgsPointLocator::Edge :
                                  QgsPointLocator::Vertex | QgsPointLocator::Edge
                                )
                              );

      mLayers.append( LayerConfig( i.key(), t, i->tolerance(), i->units() ) );
    }
  }
}
