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
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsrenderer.h"

QgsSnappingUtils::QgsSnappingUtils( QObject *parent, bool enableSnappingForInvisibleFeature )
  : QObject( parent )
  , mSnappingConfig( QgsProject::instance() )
  , mEnableSnappingForInvisibleFeature( enableSnappingForInvisibleFeature )
{
}

QgsSnappingUtils::~QgsSnappingUtils()
{
  clearAllLocators();
}


QgsPointLocator *QgsSnappingUtils::locatorForLayer( QgsVectorLayer *vl )
{
  if ( !vl )
    return nullptr;

  if ( !mLocators.contains( vl ) )
  {
    QgsPointLocator *vlpl = new QgsPointLocator( vl, destinationCrs(), mMapSettings.transformContext() );
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


QgsPointLocator *QgsSnappingUtils::locatorForLayerUsingStrategy( QgsVectorLayer *vl, const QgsPointXY &pointMap, double tolerance )
{
  QgsRectangle aoi( pointMap.x() - tolerance, pointMap.y() - tolerance,
                    pointMap.x() + tolerance, pointMap.y() + tolerance );
  if ( isIndexPrepared( vl, aoi ) )
    return locatorForLayer( vl );
  else
    return temporaryLocatorForLayer( vl, pointMap, tolerance );
}

QgsPointLocator *QgsSnappingUtils::temporaryLocatorForLayer( QgsVectorLayer *vl, const QgsPointXY &pointMap, double tolerance )
{
  if ( mTemporaryLocators.contains( vl ) )
    delete mTemporaryLocators.take( vl );

  QgsRectangle rect( pointMap.x() - tolerance, pointMap.y() - tolerance,
                     pointMap.x() + tolerance, pointMap.y() + tolerance );
  QgsPointLocator *vlpl = new QgsPointLocator( vl, destinationCrs(), mMapSettings.transformContext(), &rect );
  mTemporaryLocators.insert( vl, vlpl );
  return mTemporaryLocators.value( vl );
}

bool QgsSnappingUtils::isIndexPrepared( QgsVectorLayer *vl, const QgsRectangle &areaOfInterest )
{
  if ( vl->geometryType() == QgsWkbTypes::NullGeometry || mStrategy == IndexNeverFull )
    return false;

  QgsPointLocator *loc = locatorForLayer( vl );

  if ( mStrategy == IndexAlwaysFull && loc->hasIndex() )
    return true;

  QgsRectangle aoi( areaOfInterest );
  aoi.scale( 0.999 );
  return ( mStrategy == IndexHybrid || mStrategy == IndexExtent ) && loc->hasIndex() && ( !loc->extent() || loc->extent()->contains( aoi ) ); // the index - even if it exists - is not suitable
}

static QgsPointLocator::Match _findClosestSegmentIntersection( const QgsPointXY &pt, const QgsPointLocator::MatchList &segments )
{
  if ( segments.isEmpty() )
    return QgsPointLocator::Match();

  QSet<QgsPointXY> endpoints;

  // make a geometry
  QVector<QgsGeometry> geoms;
  Q_FOREACH ( const QgsPointLocator::Match &m, segments )
  {
    if ( m.hasEdge() )
    {
      QgsPolylineXY pl( 2 );
      m.edgePoints( pl[0], pl[1] );
      geoms << QgsGeometry::fromPolylineXY( pl );
      endpoints << pl[0] << pl[1];
    }
  }

  QgsGeometry g = QgsGeometry::unaryUnion( geoms );

  // get intersection points
  QList<QgsPointXY> newPoints;
  if ( g.wkbType() == QgsWkbTypes::LineString )
  {
    Q_FOREACH ( const QgsPointXY &p, g.asPolyline() )
    {
      if ( !endpoints.contains( p ) )
        newPoints << p;
    }
  }
  if ( g.wkbType() == QgsWkbTypes::MultiLineString )
  {
    Q_FOREACH ( const QgsPolylineXY &pl, g.asMultiPolyline() )
    {
      Q_FOREACH ( const QgsPointXY &p, pl )
      {
        if ( !endpoints.contains( p ) )
          newPoints << p;
      }
    }
  }

  if ( newPoints.isEmpty() )
    return QgsPointLocator::Match();

  // find the closest points
  QgsPointXY minP;
  double minSqrDist = 1e20;  // "infinity"
  Q_FOREACH ( const QgsPointXY &p, newPoints )
  {
    double sqrDist = pt.sqrDist( p.x(), p.y() );
    if ( sqrDist < minSqrDist )
    {
      minSqrDist = sqrDist;
      minP = p;
    }
  }

  return QgsPointLocator::Match( QgsPointLocator::Vertex, nullptr, 0, std::sqrt( minSqrDist ), minP );
}

static void _replaceIfBetter( QgsPointLocator::Match &bestMatch, const QgsPointLocator::Match &candidateMatch, double maxDistance )
{

  // is candidate match relevant?
  if ( !candidateMatch.isValid() || candidateMatch.distance() > maxDistance )
    return;

  // is candidate match actually better?
  if ( bestMatch.isValid() && bestMatch.type() == candidateMatch.type() && bestMatch.distance() - 10e-6 < candidateMatch.distance() )
    return;

  // prefer vertex matches over edge matches (even if they are closer)
  if ( bestMatch.type() == QgsPointLocator::Vertex && candidateMatch.type() == QgsPointLocator::Edge )
    return;

  bestMatch = candidateMatch; // the other match is better!
}

static void _updateBestMatch( QgsPointLocator::Match &bestMatch, const QgsPointXY &pointMap, QgsPointLocator *loc, QgsPointLocator::Types type, double tolerance, QgsPointLocator::MatchFilter *filter )
{
  if ( type & QgsPointLocator::Vertex )
  {
    _replaceIfBetter( bestMatch, loc->nearestVertex( pointMap, tolerance, filter ), tolerance );
  }
  if ( bestMatch.type() != QgsPointLocator::Vertex && ( type & QgsPointLocator::Edge ) )
  {
    _replaceIfBetter( bestMatch, loc->nearestEdge( pointMap, tolerance, filter ), tolerance );
  }
  if ( bestMatch.type() != QgsPointLocator::Vertex && bestMatch.type() != QgsPointLocator::Edge && ( type & QgsPointLocator::Area ) )
  {
    // if edges were detected, set tolerance to 0 to only do pointInPolygon (and avoid redo nearestEdge)
    if ( type & QgsPointLocator::Edge )
      tolerance = 0;
    _replaceIfBetter( bestMatch, loc->nearestArea( pointMap, tolerance, filter ), tolerance );
  }
}


static QgsPointLocator::Types _snappingTypeToPointLocatorType( QgsSnappingConfig::SnappingType type )
{
  // watch out: vertex+segment vs segment are in different order in the two enums
  switch ( type )
  {
    case QgsSnappingConfig::Vertex:
      return QgsPointLocator::Vertex;
    case QgsSnappingConfig::VertexAndSegment:
      return QgsPointLocator::Types( QgsPointLocator::Vertex | QgsPointLocator::Edge );
    case QgsSnappingConfig::Segment:
      return QgsPointLocator::Edge;
    default:
      return QgsPointLocator::Invalid;
  }
}


QgsPointLocator::Match QgsSnappingUtils::snapToMap( QPoint point, QgsPointLocator::MatchFilter *filter )
{
  return snapToMap( mMapSettings.mapToPixel().toMapCoordinates( point ), filter );
}

inline QgsRectangle _areaOfInterest( const QgsPointXY &point, double tolerance )
{
  return QgsRectangle( point.x() - tolerance, point.y() - tolerance,
                       point.x() + tolerance, point.y() + tolerance );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QgsPointXY &pointMap, QgsPointLocator::MatchFilter *filter )
{
  if ( !mMapSettings.hasValidSettings() || !mSnappingConfig.enabled() )
  {
    return QgsPointLocator::Match();
  }

  if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer )
  {
    if ( !mCurrentLayer || mSnappingConfig.type() == 0 )
      return QgsPointLocator::Match();

    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mSnappingConfig.tolerance(), mCurrentLayer, mMapSettings, mSnappingConfig.units() );
    QgsPointLocator::Types type = _snappingTypeToPointLocatorType( mSnappingConfig.type() );

    prepareIndex( QList<LayerAndAreaOfInterest>() << qMakePair( mCurrentLayer, _areaOfInterest( pointMap, tolerance ) ) );

    // use ad-hoc locator
    QgsPointLocator *loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
    if ( !loc )
      return QgsPointLocator::Match();

    QgsPointLocator::Match bestMatch;
    _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );

    if ( mSnappingConfig.intersectionSnapping() )
    {
      QgsPointLocator *locEdges = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
      QgsPointLocator::MatchList edges = locEdges->edgesInRect( pointMap, tolerance );
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );
    }

    return bestMatch;
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AdvancedConfiguration )
  {
    QList<LayerAndAreaOfInterest> layers;
    Q_FOREACH ( const LayerConfig &layerConfig, mLayers )
    {
      double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
      layers << qMakePair( layerConfig.layer, _areaOfInterest( pointMap, tolerance ) );
    }
    prepareIndex( layers );

    QgsPointLocator::Match bestMatch;
    QgsPointLocator::MatchList edges; // for snap on intersection
    double maxSnapIntTolerance = 0;

    Q_FOREACH ( const LayerConfig &layerConfig, mLayers )
    {
      double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
      if ( QgsPointLocator *loc = locatorForLayerUsingStrategy( layerConfig.layer, pointMap, tolerance ) )
      {
        _updateBestMatch( bestMatch, pointMap, loc, layerConfig.type, tolerance, filter );

        if ( mSnappingConfig.intersectionSnapping() )
        {
          edges << loc->edgesInRect( pointMap, tolerance );
          maxSnapIntTolerance = std::max( maxSnapIntTolerance, tolerance );
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
    QgsPointLocator::Types type = _snappingTypeToPointLocatorType( mSnappingConfig.type() );
    QgsRectangle aoi = _areaOfInterest( pointMap, tolerance );

    QList<LayerAndAreaOfInterest> layers;
    Q_FOREACH ( QgsMapLayer *layer, mMapSettings.layers() )
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
        layers << qMakePair( vl, aoi );
    prepareIndex( layers );

    QgsPointLocator::MatchList edges; // for snap on intersection
    QgsPointLocator::Match bestMatch;

    Q_FOREACH ( const LayerAndAreaOfInterest &entry, layers )
    {
      QgsVectorLayer *vl = entry.first;
      if ( QgsPointLocator *loc = locatorForLayerUsingStrategy( vl, pointMap, tolerance ) )
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

void QgsSnappingUtils::prepareIndex( const QList<LayerAndAreaOfInterest> &layers )
{
  if ( mIsIndexing )
    return;
  mIsIndexing = true;

  // check if we need to build any index
  QList<LayerAndAreaOfInterest> layersToIndex;
  Q_FOREACH ( const LayerAndAreaOfInterest &entry, layers )
  {
    QgsVectorLayer *vl = entry.first;

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
    Q_FOREACH ( const LayerAndAreaOfInterest &entry, layersToIndex )
    {
      QgsVectorLayer *vl = entry.first;
      QTime tt;
      tt.start();

      QgsPointLocator *loc = locatorForLayer( vl );

      if ( !mEnableSnappingForInvisibleFeature )
      {
        QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mMapSettings );
        loc->setRenderContext( &ctx );
      }

      if ( mStrategy == IndexExtent )
      {
        QgsRectangle rect( mMapSettings.extent() );
        loc->setExtent( &rect );
        loc->init();
      }
      else if ( mStrategy == IndexHybrid )
      {
        // first time the layer is used? - let's set an initial guess about indexing
        if ( !mHybridMaxAreaPerLayer.contains( vl->id() ) )
        {
          int totalFeatureCount = vl->featureCount();
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
          QgsPointXY c = entry.second.center();
          double halfSide = std::sqrt( indexReasonableArea ) / 2;
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

      QgsDebugMsg( QStringLiteral( "Index init: %1 ms (%2)" ).arg( tt.elapsed() ).arg( vl->id() ) );
      prepareIndexProgress( ++i );
    }
    QgsDebugMsg( QStringLiteral( "Prepare index total: %1 ms" ).arg( t.elapsed() ) );
  }
  mIsIndexing = false;
}

QgsSnappingConfig QgsSnappingUtils::config() const
{
  return mSnappingConfig;
}

void QgsSnappingUtils::setEnableSnappingForInvisibleFeature( bool enable )
{
  mEnableSnappingForInvisibleFeature = enable;
}

void QgsSnappingUtils::setConfig( const QgsSnappingConfig &config )
{
  if ( mSnappingConfig == config )
    return;

  if ( mSnappingConfig.individualLayerSettings() != config.individualLayerSettings() )
    onIndividualLayerSettingsChanged( config.individualLayerSettings() );

  mSnappingConfig = config;

  emit configChanged( mSnappingConfig );
}

void QgsSnappingUtils::toggleEnabled()
{
  mSnappingConfig.setEnabled( !mSnappingConfig.enabled() );
  emit configChanged( mSnappingConfig );
}

QgsPointLocator::Match QgsSnappingUtils::snapToCurrentLayer( QPoint point, QgsPointLocator::Types type, QgsPointLocator::MatchFilter *filter )
{
  if ( !mCurrentLayer )
    return QgsPointLocator::Match();

  QgsPointXY pointMap = mMapSettings.mapToPixel().toMapCoordinates( point );
  double tolerance = QgsTolerance::vertexSearchRadius( mMapSettings );

  QgsPointLocator *loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
  if ( !loc )
    return QgsPointLocator::Match();

  QgsPointLocator::Match bestMatch;
  _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter );
  return bestMatch;
}

void QgsSnappingUtils::setMapSettings( const QgsMapSettings &settings )
{
  QString oldDestCRS = mMapSettings.destinationCrs().authid();
  QString newDestCRS = settings.destinationCrs().authid();
  mMapSettings = settings;

  if ( newDestCRS != oldDestCRS )
    clearAllLocators();
}

void QgsSnappingUtils::setCurrentLayer( QgsVectorLayer *layer )
{
  mCurrentLayer = layer;
}

QString QgsSnappingUtils::dump()
{
  QString msg = QStringLiteral( "--- SNAPPING UTILS DUMP ---\n" );

  if ( !mMapSettings.hasValidSettings() )
  {
    msg += QLatin1String( "invalid map settings!" );
    return msg;
  }

  QList<LayerConfig> layers;

  if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer )
  {
    if ( mSnappingConfig.mode() == QgsSnappingConfig::ActiveLayer && !mCurrentLayer )
    {
      msg += QLatin1String( "no current layer!" );
      return msg;
    }

    layers << LayerConfig( mCurrentLayer, _snappingTypeToPointLocatorType( mSnappingConfig.type() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AllLayers )
  {
    Q_FOREACH ( QgsMapLayer *layer, mMapSettings.layers() )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
        layers << LayerConfig( vl, _snappingTypeToPointLocatorType( mSnappingConfig.type() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
    }
  }
  else if ( mSnappingConfig.mode() == QgsSnappingConfig::AdvancedConfiguration )
  {
    layers = mLayers;
  }

  Q_FOREACH ( const LayerConfig &layer, layers )
  {
    msg += QString( "layer : %1\n"
                    "config: %2   tolerance %3 %4\n" )
           .arg( layer.layer->name() )
           .arg( layer.type ).arg( layer.tolerance ).arg( layer.unit );

    if ( mStrategy == IndexAlwaysFull || mStrategy == IndexHybrid || mStrategy == IndexExtent )
    {
      if ( QgsPointLocator *loc = locatorForLayer( layer.layer ) )
      {
        QString extentStr, cachedGeoms, limit( QStringLiteral( "no max area" ) );
        if ( const QgsRectangle *r = loc->extent() )
        {
          extentStr = QStringLiteral( " extent %1" ).arg( r->toString() );
        }
        else
          extentStr = QStringLiteral( "full extent" );
        if ( loc->hasIndex() )
          cachedGeoms = QStringLiteral( "%1 feats" ).arg( loc->cachedGeometryCount() );
        else
          cachedGeoms = QStringLiteral( "not initialized" );
        if ( mStrategy == IndexHybrid )
        {
          if ( mHybridMaxAreaPerLayer.contains( layer.layer->id() ) )
          {
            double maxArea = mStrategy == IndexHybrid ? mHybridMaxAreaPerLayer[layer.layer->id()] : -1;
            if ( maxArea != -1 )
              limit = QStringLiteral( "max area %1" ).arg( maxArea );
          }
          else
            limit = QStringLiteral( "not evaluated" );
        }
        msg += QStringLiteral( "index : YES | %1 | %2 | %3\n" ).arg( cachedGeoms, extentStr, limit );
      }
      else
        msg += QStringLiteral( "index : ???\n" ); // should not happen
    }
    else
      msg += QLatin1String( "index : NO\n" );
    msg += QLatin1String( "-\n" );
  }

  return msg;
}

QgsCoordinateReferenceSystem QgsSnappingUtils::destinationCrs() const
{
  return mMapSettings.destinationCrs();
}

void QgsSnappingUtils::onIndividualLayerSettingsChanged( const QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> &layerSettings )
{
  mLayers.clear();

  QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings>::const_iterator i;

  for ( i = layerSettings.constBegin(); i != layerSettings.constEnd(); ++i )
  {
    if ( i->enabled() )
    {
      mLayers.append( LayerConfig( i.key(), _snappingTypeToPointLocatorType( i->type() ), i->tolerance(), i->units() ) );
    }
  }
}
