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
#include "qgsrendercontext.h"

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
    QgsPointLocator *vlpl = new QgsPointLocator( vl, destinationCrs(), mMapSettings.transformContext(), nullptr );
    connect( vlpl, &QgsPointLocator::initFinished, this, &QgsSnappingUtils::onInitFinished );
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
  if ( vl->geometryType() == QgsWkbTypes::NullGeometry || mStrategy == IndexNeverFull )
    return nullptr;

  QgsRectangle aoi( pointMap.x() - tolerance, pointMap.y() - tolerance,
                    pointMap.x() + tolerance, pointMap.y() + tolerance );

  QgsPointLocator *loc = locatorForLayer( vl );

  if ( loc->isIndexing() || isIndexPrepared( loc, aoi ) )
    return loc;
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
  connect( vlpl, &QgsPointLocator::initFinished, this, &QgsSnappingUtils::onInitFinished );

  mTemporaryLocators.insert( vl, vlpl );
  return mTemporaryLocators.value( vl );
}

bool QgsSnappingUtils::isIndexPrepared( QgsPointLocator *loc, const QgsRectangle &areaOfInterest )
{
  if ( mStrategy == IndexAlwaysFull && loc->hasIndex() )
    return true;

  if ( mStrategy == IndexExtent && loc->hasIndex() && ( !loc->extent() || loc->extent()->intersects( areaOfInterest ) ) )
    return true;

  QgsRectangle aoi( areaOfInterest );
  aoi.scale( 0.999 );
  return mStrategy == IndexHybrid && loc->hasIndex() && ( !loc->extent() || loc->extent()->contains( aoi ) ); // the index - even if it exists - is not suitable
}

static QgsPointLocator::Match _findClosestSegmentIntersection( const QgsPointXY &pt, const QgsPointLocator::MatchList &segments )
{
  if ( segments.isEmpty() )
    return QgsPointLocator::Match();

  QSet<QgsPointXY> endpoints;

  // make a geometry
  QVector<QgsGeometry> geoms;
  const auto constSegments = segments;
  for ( const QgsPointLocator::Match &m : constSegments )
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
    const auto constAsPolyline = g.asPolyline();
    for ( const QgsPointXY &p : constAsPolyline )
    {
      if ( !endpoints.contains( p ) )
        newPoints << p;
    }
  }
  if ( g.wkbType() == QgsWkbTypes::MultiLineString )
  {
    const auto constAsMultiPolyline = g.asMultiPolyline();
    for ( const QgsPolylineXY &pl : constAsMultiPolyline )
    {
      const auto constPl = pl;
      for ( const QgsPointXY &p : constPl )
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
  const auto constNewPoints = newPoints;
  for ( const QgsPointXY &p : constNewPoints )
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

  // ORDER
  // LineEndpoint
  // Vertex, Intersection
  // Middle
  // Centroid
  // Edge
  // Area

  // first line endpoint -- these are like vertex matches, but even more strict
  if ( ( bestMatch.type() & QgsPointLocator::LineEndpoint ) && !( candidateMatch.type() & QgsPointLocator::LineEndpoint ) )
    return;
  if ( candidateMatch.type() & QgsPointLocator::LineEndpoint )
  {
    bestMatch = candidateMatch;
    return;
  }

  // Second Vertex, or intersection
  if ( ( bestMatch.type() & QgsPointLocator::Vertex ) && !( candidateMatch.type() & QgsPointLocator::Vertex ) )
    return;
  if ( candidateMatch.type() & QgsPointLocator::Vertex )
  {
    bestMatch = candidateMatch;
    return;
  }

  // prefer vertex, centroid, middle matches over edge matches (even if they are closer)
  if ( ( bestMatch.type() & QgsPointLocator::Centroid || bestMatch.type() & QgsPointLocator::MiddleOfSegment ) && ( candidateMatch.type() & QgsPointLocator::Edge  || candidateMatch.type() & QgsPointLocator::Area ) )
    return;

  // prefer middle matches over centroid matches (even if they are closer)
  if ( ( bestMatch.type() & QgsPointLocator::MiddleOfSegment ) && ( candidateMatch.type() & QgsPointLocator::Centroid ) )
    return;

  bestMatch = candidateMatch; // the other match is better!
}

static void _updateBestMatch( QgsPointLocator::Match &bestMatch, const QgsPointXY &pointMap, QgsPointLocator *loc, QgsPointLocator::Types type, double tolerance, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  if ( type & QgsPointLocator::Vertex )
  {
    _replaceIfBetter( bestMatch, loc->nearestVertex( pointMap, tolerance, filter, relaxed ), tolerance );
  }
  if ( bestMatch.type() != QgsPointLocator::Vertex && ( type & QgsPointLocator::Edge ) )
  {
    _replaceIfBetter( bestMatch, loc->nearestEdge( pointMap, tolerance, filter, relaxed ), tolerance );
  }
  if ( bestMatch.type() != QgsPointLocator::Vertex && bestMatch.type() != QgsPointLocator::Edge && ( type & QgsPointLocator::Area ) )
  {
    // if edges were detected, set tolerance to 0 to only do pointInPolygon (and avoid redo nearestEdge)
    if ( type & QgsPointLocator::Edge )
      tolerance = 0;
    _replaceIfBetter( bestMatch, loc->nearestArea( pointMap, tolerance, filter, relaxed ), tolerance );
  }
  if ( type & QgsPointLocator::Centroid )
  {
    _replaceIfBetter( bestMatch, loc->nearestCentroid( pointMap, tolerance, filter ), tolerance );
  }
  if ( type & QgsPointLocator::MiddleOfSegment )
  {
    _replaceIfBetter( bestMatch, loc->nearestMiddleOfSegment( pointMap, tolerance, filter ), tolerance );
  }
  if ( type & QgsPointLocator::LineEndpoint )
  {
    _replaceIfBetter( bestMatch, loc->nearestLineEndpoints( pointMap, tolerance, filter ), tolerance );
  }
}


static QgsPointLocator::Types _snappingTypeToPointLocatorType( Qgis::SnappingTypes type )
{
  return QgsPointLocator::Types( static_cast<int>( type ) );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( QPoint point, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  return snapToMap( mMapSettings.mapToPixel().toMapCoordinates( point ), filter, relaxed );
}

inline QgsRectangle _areaOfInterest( const QgsPointXY &point, double tolerance )
{
  return QgsRectangle( point.x() - tolerance, point.y() - tolerance,
                       point.x() + tolerance, point.y() + tolerance );
}

QgsPointLocator::Match QgsSnappingUtils::snapToMap( const QgsPointXY &pointMap, QgsPointLocator::MatchFilter *filter, bool relaxed )
{
  if ( !mMapSettings.hasValidSettings() || !mSnappingConfig.enabled() )
  {
    return QgsPointLocator::Match();
  }

  if ( mSnappingConfig.mode() == Qgis::SnappingMode::ActiveLayer )
  {
    if ( !mCurrentLayer || mSnappingConfig.typeFlag().testFlag( Qgis::SnappingType::NoSnap ) )
      return QgsPointLocator::Match();

    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mSnappingConfig.tolerance(), mCurrentLayer, mMapSettings, mSnappingConfig.units() );
    QgsPointLocator::Types type = _snappingTypeToPointLocatorType( mSnappingConfig.typeFlag() );

    prepareIndex( QList<LayerAndAreaOfInterest>() << qMakePair( mCurrentLayer, _areaOfInterest( pointMap, tolerance ) ), relaxed );

    // use ad-hoc locator
    QgsPointLocator *loc = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
    if ( !loc )
      return QgsPointLocator::Match();

    QgsPointLocator::Match bestMatch;
    QgsPointLocator::MatchList edges; // for snap on intersection
    _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter, relaxed );

    if ( mSnappingConfig.intersectionSnapping() )
    {
      QgsPointLocator *locEdges = locatorForLayerUsingStrategy( mCurrentLayer, pointMap, tolerance );
      if ( !locEdges )
        return QgsPointLocator::Match();
      edges = locEdges->edgesInRect( pointMap, tolerance );
    }

    for ( QgsVectorLayer *vl : mExtraSnapLayers )
    {
      QgsPointLocator *loc = locatorForLayerUsingStrategy( vl, pointMap, tolerance );
      _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter, false );
      if ( mSnappingConfig.intersectionSnapping() )
        edges << loc->edgesInRect( pointMap, tolerance );
    }

    if ( mSnappingConfig.intersectionSnapping() )
    {
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );
    }

    return bestMatch;
  }
  else if ( mSnappingConfig.mode() == Qgis::SnappingMode::AdvancedConfiguration )
  {
    QList<LayerAndAreaOfInterest> layers;
    QList<LayerConfig> filteredConfigs;

    //maximum scale is the one with smallest denominator
    //minimum scale is the one with highest denominator
    //So : maxscale < range on which snapping is enabled < minscale
    bool inRangeGlobal = ( mSnappingConfig.minimumScale() <= 0.0 || mMapSettings.scale() <= mSnappingConfig.minimumScale() )
                         && ( mSnappingConfig.maximumScale() <= 0.0 || mMapSettings.scale() >= mSnappingConfig.maximumScale() );

    for ( const LayerConfig &layerConfig : std::as_const( mLayers ) )
    {
      QgsSnappingConfig::IndividualLayerSettings layerSettings = mSnappingConfig.individualLayerSettings( layerConfig.layer );

      bool inRangeLayer = ( layerSettings.minimumScale() <= 0.0 || mMapSettings.scale() <= layerSettings.minimumScale() )
                          && ( layerSettings.maximumScale() <= 0.0 || mMapSettings.scale() >= layerSettings.maximumScale() );

      //If limit to scale is disabled, snapping activated on all layer
      //If no per layer config is set use the global one, otherwise use the layer config
      if ( mSnappingConfig.scaleDependencyMode() == QgsSnappingConfig::Disabled
           || ( mSnappingConfig.scaleDependencyMode() == QgsSnappingConfig::Global && inRangeGlobal )
           || ( mSnappingConfig.scaleDependencyMode() == QgsSnappingConfig::PerLayer  && inRangeLayer ) )
      {
        double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
        layers << qMakePair( layerConfig.layer, _areaOfInterest( pointMap, tolerance ) );
        filteredConfigs << layerConfig;
      }
    }
    prepareIndex( layers, relaxed );

    QgsPointLocator::Match bestMatch;
    QgsPointLocator::MatchList edges; // for snap on intersection
    double maxTolerance = 0;
    QgsPointLocator::Type maxTypes = QgsPointLocator::Invalid;

    for ( const LayerConfig &layerConfig : std::as_const( filteredConfigs ) )
    {
      double tolerance = QgsTolerance::toleranceInProjectUnits( layerConfig.tolerance, layerConfig.layer, mMapSettings, layerConfig.unit );
      if ( QgsPointLocator *loc = locatorForLayerUsingStrategy( layerConfig.layer, pointMap, tolerance ) )
      {
        _updateBestMatch( bestMatch, pointMap, loc, layerConfig.type, tolerance, filter, relaxed );

        if ( mSnappingConfig.intersectionSnapping() )
        {
          edges << loc->edgesInRect( pointMap, tolerance );
        }
        // We keep the maximum tolerance for intersection snapping and extra snapping
        maxTolerance = std::max( maxTolerance, tolerance );
        // To avoid yet an additional setting, on extra snappings, we use the combination of all enabled snap types
        maxTypes = static_cast<QgsPointLocator::Type>( maxTypes | layerConfig.type );
      }
    }

    for ( QgsVectorLayer *vl : mExtraSnapLayers )
    {
      QgsPointLocator *loc = locatorForLayerUsingStrategy( vl, pointMap, maxTolerance );
      _updateBestMatch( bestMatch, pointMap, loc, maxTypes, maxTolerance, filter, false );
      if ( mSnappingConfig.intersectionSnapping() )
        edges << loc->edgesInRect( pointMap, maxTolerance );
    }

    if ( mSnappingConfig.intersectionSnapping() )
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), maxTolerance );

    return bestMatch;
  }
  else if ( mSnappingConfig.mode() == Qgis::SnappingMode::AllLayers )
  {
    // data from project
    double tolerance = QgsTolerance::toleranceInProjectUnits( mSnappingConfig.tolerance(), nullptr, mMapSettings, mSnappingConfig.units() );
    QgsPointLocator::Types type = _snappingTypeToPointLocatorType( mSnappingConfig.typeFlag() );
    QgsRectangle aoi = _areaOfInterest( pointMap, tolerance );

    QList<LayerAndAreaOfInterest> layers;
    const auto constLayers = mMapSettings.layers( true );
    for ( QgsMapLayer *layer : constLayers )
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
        layers << qMakePair( vl, aoi );
    prepareIndex( layers, relaxed );

    QgsPointLocator::MatchList edges; // for snap on intersection
    QgsPointLocator::Match bestMatch;

    for ( const LayerAndAreaOfInterest &entry : std::as_const( layers ) )
    {
      QgsVectorLayer *vl = entry.first;
      if ( QgsPointLocator *loc = locatorForLayerUsingStrategy( vl, pointMap, tolerance ) )
      {
        _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter, relaxed );

        if ( mSnappingConfig.intersectionSnapping() )
          edges << loc->edgesInRect( pointMap, tolerance );
      }
    }

    for ( QgsVectorLayer *vl : mExtraSnapLayers )
    {
      QgsPointLocator *loc = locatorForLayerUsingStrategy( vl, pointMap, tolerance );
      _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter, false );
      if ( mSnappingConfig.intersectionSnapping() )
        edges << loc->edgesInRect( pointMap, tolerance );
    }

    if ( mSnappingConfig.intersectionSnapping() )
      _replaceIfBetter( bestMatch, _findClosestSegmentIntersection( pointMap, edges ), tolerance );

    return bestMatch;
  }

  return QgsPointLocator::Match();
}

void QgsSnappingUtils::onInitFinished( bool ok )
{
  QgsPointLocator *loc = static_cast<QgsPointLocator *>( sender() );

  // point locator init didn't work out - too many features!
  // let's make the allowed area smaller for the next time
  if ( !ok )
  {
    mHybridMaxAreaPerLayer[loc->layer()->id()] /= 4;
  }
}

void QgsSnappingUtils::prepareIndex( const QList<LayerAndAreaOfInterest> &layers, bool relaxed )
{
  // check if we need to build any index
  QList<LayerAndAreaOfInterest> layersToIndex;
  const auto constLayers = layers;
  for ( const LayerAndAreaOfInterest &entry : constLayers )
  {
    QgsVectorLayer *vl = entry.first;

    if ( vl->geometryType() == QgsWkbTypes::NullGeometry || mStrategy == IndexNeverFull )
      continue;

    QgsPointLocator *loc = locatorForLayer( vl );

    if ( !loc->isIndexing() && !isIndexPrepared( loc, entry.second ) )
      layersToIndex << entry;
  }
  if ( !layersToIndex.isEmpty() )
  {
    // build indexes
    QElapsedTimer t;
    int i = 0;

    if ( !relaxed )
    {
      t.start();
      prepareIndexStarting( layersToIndex.count() );
    }

    for ( const LayerAndAreaOfInterest &entry : layersToIndex )
    {
      QgsVectorLayer *vl = entry.first;
      QgsPointLocator *loc = locatorForLayer( vl );

      if ( loc->isIndexing() && !relaxed )
      {
        loc->waitForIndexingFinished();
      }


      if ( !mEnableSnappingForInvisibleFeature )
      {
        QgsRenderContext ctx = QgsRenderContext::fromMapSettings( mMapSettings );
        loc->setRenderContext( &ctx );
      }

      if ( mStrategy == IndexExtent )
      {
        QgsRectangle rect( mMapSettings.visibleExtent() );
        loc->setExtent( &rect );
        loc->init( -1, relaxed );
      }
      else if ( mStrategy == IndexHybrid )
      {
        // first time the layer is used? - let's set an initial guess about indexing
        if ( !mHybridMaxAreaPerLayer.contains( vl->id() ) )
        {
          long long totalFeatureCount = vl->featureCount();
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
          loc->init( -1, relaxed );
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
          loc->init( mHybridPerLayerFeatureLimit, relaxed );
        }

      }
      else  // full index strategy
        loc->init( relaxed );

      if ( !relaxed )
        prepareIndexProgress( ++i );
    }

    if ( !relaxed )
    {
      QgsDebugMsg( QStringLiteral( "Prepare index total: %1 ms" ).arg( t.elapsed() ) );
    }
  }
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
  _updateBestMatch( bestMatch, pointMap, loc, type, tolerance, filter, false );
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

  if ( mSnappingConfig.mode() == Qgis::SnappingMode::ActiveLayer )
  {
    if ( mSnappingConfig.mode() == Qgis::SnappingMode::ActiveLayer && !mCurrentLayer )
    {
      msg += QLatin1String( "no current layer!" );
      return msg;
    }

    layers << LayerConfig( mCurrentLayer, _snappingTypeToPointLocatorType( mSnappingConfig.typeFlag() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
  }
  else if ( mSnappingConfig.mode() == Qgis::SnappingMode::AllLayers )
  {
    const auto constLayers = mMapSettings.layers( true );
    for ( QgsMapLayer *layer : constLayers )
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
        layers << LayerConfig( vl, _snappingTypeToPointLocatorType( mSnappingConfig.typeFlag() ), mSnappingConfig.tolerance(), mSnappingConfig.units() );
    }
  }
  else if ( mSnappingConfig.mode() == Qgis::SnappingMode::AdvancedConfiguration )
  {
    layers = mLayers;
  }

  const auto constLayers = layers;
  for ( const LayerConfig &layer : constLayers )
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
        msg += QLatin1String( "index : ???\n" ); // should not happen
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
      mLayers.append( LayerConfig( i.key(), _snappingTypeToPointLocatorType( static_cast<Qgis::SnappingTypes>( i->typeFlag() ) ), i->tolerance(), i->units() ) );
    }
  }
}
