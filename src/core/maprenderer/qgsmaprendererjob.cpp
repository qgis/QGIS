/***************************************************************************
  qgsmaprendererjob.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprendererjob.h"

#include <memory>

#include "qgselevationmap.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgeometrypaintdevice.h"
#include "qgsgeos.h"
#include "qgslabelingengine.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerstyle.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmaprenderercache.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerlabeling.h"
#include "qgsmessagelog.h"
#include "qgspainting.h"
#include "qgspallabeling.h"
#include "qgsrasterlabeling.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsrendercontext.h"
#include "qgsrendereditemresults.h"
#include "qgsruntimeprofiler.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerrenderer.h"
#include "qgsvectorlayerutils.h"

#include <QElapsedTimer>
#include <QPainter>
#include <QPicture>
#include <QString>
#include <QTimer>

#include "moc_qgsmaprendererjob.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryBool *QgsMapRendererJob::settingsLogCanvasRefreshEvent = new QgsSettingsEntryBool( u"logCanvasRefreshEvent"_s, QgsSettingsTree::sTreeMap, false );
const QgsSettingsEntryString *QgsMapRendererJob::settingsMaskBackend = new QgsSettingsEntryString( u"mask-backend"_s, QgsSettingsTree::sTreeMap, QString(), u"Backend engine to use for selective masking"_s );

///@cond PRIVATE

const QString QgsMapRendererJob::LABEL_CACHE_ID = u"_labels_"_s;
const QString QgsMapRendererJob::ELEVATION_MAP_CACHE_PREFIX = u"_elevation_map_"_s;
const QString QgsMapRendererJob::LABEL_PREVIEW_CACHE_ID = u"_preview_labels_"_s;

LayerRenderJob &LayerRenderJob::operator=( LayerRenderJob &&other )
{
  if ( &other == this )
    return *this;

  mContext = std::move( other.mContext );
  destinationImage = std::move( other.destinationImage );
  destinationPainter = std::move( other.destinationPainter );
  renderer = std::move( other.renderer );
  previewRenderImage = std::move( other.previewRenderImage );
  previewRenderPainter = std::move( other.previewRenderPainter );

  imageInitialized = other.imageInitialized;
  previewRenderImageInitialized = other.previewRenderImageInitialized;

  blendMode = other.blendMode;
  opacity = other.opacity;
  cached = other.cached;
  layer = other.layer;
  renderAboveLabels = other.renderAboveLabels;
  completed = other.completed;
  renderingTime = other.renderingTime;
  estimatedRenderingTime = other.estimatedRenderingTime;
  errors = other.errors;
  layerId = other.layerId;

  maskPaintDevice = std::move( other.maskPaintDevice );

  firstPassJob = other.firstPassJob;
  other.firstPassJob = nullptr;

  picture = std::move( other.picture );

  maskJobs = other.maskJobs;

  maskRequiresLayerRasterization = other.maskRequiresLayerRasterization;

  elevationMap = std::move( other.elevationMap );
  maskPainter = std::move( other.maskPainter );

  return *this;
}

LayerRenderJob::LayerRenderJob( LayerRenderJob &&other )
  : imageInitialized( other.imageInitialized )
  , previewRenderImageInitialized( other.previewRenderImageInitialized )
  , blendMode( other.blendMode )
  , opacity( other.opacity )
  , cached( other.cached )
  , renderAboveLabels( other.renderAboveLabels )
  , layer( other.layer )
  , completed( other.completed )
  , renderingTime( other.renderingTime )
  , estimatedRenderingTime( other.estimatedRenderingTime )
  , errors( other.errors )
  , layerId( other.layerId )
  , maskPainter( std::move( other.maskPainter ) )
  , maskRequiresLayerRasterization( other.maskRequiresLayerRasterization )
  , maskJobs( other.maskJobs )
{
  mContext = std::move( other.mContext );
  destinationImage = std::move( other.destinationImage );
  destinationPainter = std::move( other.destinationPainter );
  previewRenderImage = std::move( other.previewRenderImage );
  previewRenderPainter = std::move( other.previewRenderPainter );
  renderer = std::move( other.renderer );
  elevationMap = std::move( other.elevationMap );
  maskPaintDevice = std::move( other.maskPaintDevice );

  firstPassJob = other.firstPassJob;
  other.firstPassJob = nullptr;

  picture = std::move( other.picture );
}

bool LayerRenderJob::imageCanBeComposed() const
{
  if ( imageInitialized )
  {
    if ( renderer )
    {
      return renderer->isReadyToCompose();
    }
    else
    {
      return true;
    }
  }
  else
  {
    return false;
  }
}

QgsMapRendererJob::QgsMapRendererJob( const QgsMapSettings &settings )
  : mSettings( settings )
  , mRenderedItemResults( std::make_unique< QgsRenderedItemResults >( settings.extent() ) )
  , mLabelingEngineFeedback( new QgsLabelingEngineFeedback( this ) )
{}

QgsMapRendererJob::~QgsMapRendererJob() = default;

void QgsMapRendererJob::start()
{
  if ( mSettings.hasValidSettings() )
    startPrivate();
  else
  {
    mErrors.append( QgsMapRendererJob::Error( QString(), tr( "Invalid map settings" ) ) );
    emit finished();
  }
}

QStringList QgsMapRendererJob::layersRedrawnFromCache() const
{
  return mLayersRedrawnFromCache;
}

QgsRenderedItemResults *QgsMapRendererJob::takeRenderedItemResults()
{
  return mRenderedItemResults.release();
}

QgsMapRendererQImageJob::QgsMapRendererQImageJob( const QgsMapSettings &settings )
  : QgsMapRendererJob( settings )
{}


QgsMapRendererJob::Errors QgsMapRendererJob::errors() const
{
  return mErrors;
}

void QgsMapRendererJob::setCache( QgsMapRendererCache *cache )
{
  mCache = cache;
}

QgsLabelingEngineFeedback *QgsMapRendererJob::labelingEngineFeedback()
{
  return mLabelingEngineFeedback;
}

QHash<QgsMapLayer *, int> QgsMapRendererJob::perLayerRenderingTime() const
{
  QHash<QgsMapLayer *, int> result;
  for ( auto it = mPerLayerRenderingTime.constBegin(); it != mPerLayerRenderingTime.constEnd(); ++it )
  {
    if ( auto &&lKey = it.key() )
      result.insert( lKey, it.value() );
  }
  return result;
}

void QgsMapRendererJob::setLayerRenderingTimeHints( const QHash<QString, int> &hints )
{
  mLayerRenderingTimeHints = hints;
}

const QgsMapSettings &QgsMapRendererJob::mapSettings() const
{
  return mSettings;
}

bool QgsMapRendererJob::prepareLabelCache() const
{
  bool canCache = mCache;

  // calculate which layers will be labeled
  QSet< QgsMapLayer * > labeledLayers;
  const QList<QgsMapLayer *> layers = mSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    if ( QgsPalLabeling::staticWillUseLayer( ml ) )
      labeledLayers << ml;

    switch ( ml->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( ml );
        if ( vl->labelsEnabled() && vl->labeling()->requiresAdvancedEffects() )
        {
          canCache = false;
        }
        break;
      }

      case Qgis::LayerType::Mesh:
      {
        QgsMeshLayer *l = qobject_cast< QgsMeshLayer *>( ml );
        if ( l->labelsEnabled() && l->labeling()->requiresAdvancedEffects() )
        {
          canCache = false;
        }
        break;
      }

      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *l = qobject_cast< QgsRasterLayer *>( ml );
        if ( l->labelsEnabled() && l->labeling()->requiresAdvancedEffects() )
        {
          canCache = false;
        }
        break;
      }

      case Qgis::LayerType::VectorTile:
      {
        // TODO -- add detection of advanced labeling effects for vector tile layers
        break;
      }

      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        break;
    }

    if ( !canCache )
      break;
  }

  if ( mCache && mCache->hasCacheImage( LABEL_CACHE_ID ) )
  {
    // we may need to clear label cache and re-register labeled features - check for that here

    // can we reuse the cached label solution?
    const QList< QgsMapLayer * > labelDependentLayers = mCache->dependentLayers( LABEL_CACHE_ID );
    bool canUseCache = canCache && QSet< QgsMapLayer * >( labelDependentLayers.begin(), labelDependentLayers.end() ) == labeledLayers;
    if ( !canUseCache )
    {
      // no - participating layers have changed
      mCache->clearCacheImage( LABEL_CACHE_ID );
    }
  }
  return canCache;
}

bool QgsMapRendererJob::labelingHasNonDefaultCompositionModes() const
{
  const QList<QgsMapLayer *> layers = mSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    if ( !QgsPalLabeling::staticWillUseLayer( ml ) )
      continue;

    switch ( ml->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( ml );
        if ( vl->labelsEnabled() && vl->labeling()->hasNonDefaultCompositionMode() )
        {
          return true;
        }
        break;
      }

      case Qgis::LayerType::Mesh:
      {
        QgsMeshLayer *l = qobject_cast< QgsMeshLayer *>( ml );
        if ( l->labelsEnabled() && l->labeling()->hasNonDefaultCompositionMode() )
        {
          return true;
        }
        break;
      }

      case Qgis::LayerType::Raster:
      {
        QgsRasterLayer *l = qobject_cast< QgsRasterLayer *>( ml );
        if ( l->labelsEnabled() && l->labeling()->hasNonDefaultCompositionMode() )
        {
          return true;
        }
        break;
      }

      case Qgis::LayerType::VectorTile:
      {
        // TODO -- add detection of advanced labeling effects for vector tile layers
        break;
      }

      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        break;
    }
  }

  return false;
}

bool QgsMapRendererJob::reprojectToLayerExtent( const QgsMapLayer *ml, const QgsCoordinateTransform &ct, QgsRectangle &extent, QgsRectangle &r2 )
{
  bool res = true;
  // we can safely use ballpark transforms without bothering the user here -- at the likely scale of layer extents there
  // won't be an appreciable difference, and we aren't actually transforming any rendered points here anyway (just the layer extent)
  QgsCoordinateTransform approxTransform = ct;
  approxTransform.setBallparkTransformsAreAppropriate( true );

  try
  {
#ifdef QGISDEBUG
    // QgsLogger::debug<QgsRectangle>("Getting extent of canvas in layers CS. Canvas is ", extent, __FILE__, __FUNCTION__, __LINE__);
#endif
    // Split the extent into two if the source CRS is
    // geographic and the extent crosses the split in
    // geographic coordinates (usually +/- 180 degrees,
    // and is assumed to be so here), and draw each
    // extent separately.
    static const double SPLIT_COORD = 180.0;

    if ( ml->crs().isGeographic() )
    {
      if ( ml->type() == Qgis::LayerType::Vector && !approxTransform.destinationCrs().isGeographic() )
      {
        // if we transform from a projected coordinate system check
        // check if transforming back roughly returns the input
        // extend - otherwise render the world.
        QgsRectangle extent1 = approxTransform.transformBoundingBox( extent, Qgis::TransformDirection::Reverse );
        QgsRectangle extent2 = approxTransform.transformBoundingBox( extent1, Qgis::TransformDirection::Forward );

        QgsDebugMsgLevel(
          u"\n0:%1 %2x%3\n1:%4\n2:%5 %6x%7 (w:%8 h:%9)"_s.arg( extent.toString() )
            .arg( extent.width() )
            .arg( extent.height() )
            .arg( extent1.toString(), extent2.toString() )
            .arg( extent2.width() )
            .arg( extent2.height() )
            .arg( std::fabs( 1.0 - extent2.width() / extent.width() ) )
            .arg( std::fabs( 1.0 - extent2.height() / extent.height() ) ),
          3
        );

        // can differ by a maximum of up to 20% of height/width
        if ( qgsDoubleNear( extent2.xMinimum(), extent.xMinimum(), extent.width() * 0.2 )
             && qgsDoubleNear( extent2.xMaximum(), extent.xMaximum(), extent.width() * 0.2 )
             && qgsDoubleNear( extent2.yMinimum(), extent.yMinimum(), extent.height() * 0.2 )
             && qgsDoubleNear( extent2.yMaximum(), extent.yMaximum(), extent.height() * 0.2 ) )
        {
          extent = extent1;
        }
        else
        {
          extent = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
          res = false;
        }
      }
      else
      {
        // Note: ll = lower left point
        QgsPointXY ll = approxTransform.transform( extent.xMinimum(), extent.yMinimum(), Qgis::TransformDirection::Reverse );

        //   and ur = upper right point
        QgsPointXY ur = approxTransform.transform( extent.xMaximum(), extent.yMaximum(), Qgis::TransformDirection::Reverse );

        QgsDebugMsgLevel( u"in:%1 (ll:%2 ur:%3)"_s.arg( extent.toString(), ll.toString(), ur.toString() ), 4 );

        extent = approxTransform.transformBoundingBox( extent, Qgis::TransformDirection::Reverse );

        QgsDebugMsgLevel( u"out:%1 (w:%2 h:%3)"_s.arg( extent.toString() ).arg( extent.width() ).arg( extent.height() ), 4 );

        if ( ll.x() > ur.x() )
        {
          // the coordinates projected in reverse order than what one would expect.
          // we are probably looking at an area that includes longitude of 180 degrees.
          // we need to take into account coordinates from two intervals: (-180,x1) and (x2,180)
          // so let's use (-180,180). This hopefully does not add too much overhead. It is
          // more straightforward than rendering with two separate extents and more consistent
          // for rendering, labeling and caching as everything is rendered just in one go
          extent.setXMinimum( -SPLIT_COORD );
          extent.setXMaximum( SPLIT_COORD );
          res = false;
        }
      }

      // TODO: the above rule still does not help if using a projection that covers the whole
      // world. E.g. with EPSG:3857 the longitude spectrum -180 to +180 is mapped to approx.
      // -2e7 to +2e7. Converting extent from -5e7 to +5e7 is transformed as -90 to +90,
      // but in fact the extent should cover the whole world.
    }
    else // can't cross 180
    {
      if ( approxTransform.destinationCrs().isGeographic() && ( extent.xMinimum() <= -180 || extent.xMaximum() >= 180 || extent.yMinimum() <= -90 || extent.yMaximum() >= 90 ) )
      // Use unlimited rectangle because otherwise we may end up transforming wrong coordinates.
      // E.g. longitude -200 to +160 would be understood as +40 to +160 due to periodicity.
      // We could try to clamp coords to (-180,180) for lon resp. (-90,90) for lat,
      // but this seems like a safer choice.
      {
        extent = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
        res = false;
      }
      else
        extent = approxTransform.transformBoundingBox( extent, Qgis::TransformDirection::Reverse );
    }
  }
  catch ( QgsCsException &e )
  {
    QgsDebugError( u"Transform error caught: %1"_s.arg( e.what() ) );
    extent = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
    r2 = QgsRectangle( std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() );
    res = false;
  }

  return res;
}

std::unique_ptr<QImage> QgsMapRendererJob::allocateImage( const QString &layerId )
{
  auto image = std::make_unique< QImage >( mSettings.deviceOutputSize(), mSettings.outputImageFormat() );
  image->setDevicePixelRatio( static_cast<qreal>( mSettings.devicePixelRatio() ) );
  image->setDotsPerMeterX( 1000 * mSettings.outputDpi() / 25.4 );
  image->setDotsPerMeterY( 1000 * mSettings.outputDpi() / 25.4 );
  if ( image->isNull() )
  {
    QgsDebugError( u"Insufficient memory for image %1x%2"_s.arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) );
    mErrors.append( Error( layerId, tr( "Insufficient memory for image %1x%2" ).arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) ) );
    return {};
  }
  return image;
}

std::unique_ptr<QgsElevationMap> QgsMapRendererJob::allocateElevationMap( const QString &layerId )
{
  auto elevationMap = std::make_unique<QgsElevationMap>( mSettings.deviceOutputSize(), mSettings.devicePixelRatio() );
  if ( !elevationMap->isValid() )
  {
    QgsDebugError( u"Insufficient memory for elevation map %1x%2"_s.arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) );
    mErrors.append( Error( layerId, tr( "Insufficient memory for elevation map %1x%2" ).arg( mSettings.outputSize().width() ).arg( mSettings.outputSize().height() ) ) );
    return nullptr;
  }
  return elevationMap;
}

std::tuple< std::unique_ptr< QImage >, std::unique_ptr< QPainter > > QgsMapRendererJob::allocateImageAndPainter( const QString &layerId, const QgsRenderContext &context )
{
  std::unique_ptr< QPainter > painter;
  std::unique_ptr< QImage > image = allocateImage( layerId );
  if ( image )
  {
    painter = std::make_unique< QPainter >( image.get() );
    context.setPainterFlagsUsingContext( painter.get() );
  }
  return { std::move( image ), std::move( painter ) };
}

QgsMapRendererJob::PictureAndPainter QgsMapRendererJob::allocatePictureAndPainter( const QgsRenderContext *context )
{
  auto picture = std::make_unique<QPicture>();
  QPainter *painter = new QPainter( picture.get() );
  context->setPainterFlagsUsingContext( painter );
  return { std::move( picture ), painter };
}

std::vector<LayerRenderJob> QgsMapRendererJob::prepareJobs( QPainter *painter, QgsLabelingEngine *labelingEngine2, bool deferredPainterSet )
{
  std::vector< LayerRenderJob > layerJobs;

  // render all layers in the stack, starting at the base
  QListIterator<QgsMapLayer *> li( mSettings.layers() );
  li.toBack();

  if ( mCache )
  {
    bool cacheValid = mCache->updateParameters( mSettings.visibleExtent(), mSettings.mapToPixel() );
    Q_UNUSED( cacheValid )
    QgsDebugMsgLevel( u"CACHE VALID: %1"_s.arg( cacheValid ), 4 );
  }

  bool requiresLabelRedraw = !( mCache && mCache->hasCacheImage( LABEL_CACHE_ID ) );

  while ( li.hasPrevious() )
  {
    QgsMapLayer *ml = li.previous();

    QgsDebugMsgLevel(
      u"layer %1:  minscale:%2  maxscale:%3  scaledepvis:%4  blendmode:%5 isValid:%6"_s.arg( ml->name() )
        .arg( ml->minimumScale() )
        .arg( ml->maximumScale() )
        .arg( ml->hasScaleBasedVisibility() )
        .arg( ml->blendMode() )
        .arg( ml->isValid() ),
      3
    );

    if ( !ml->isValid() )
    {
      QgsDebugMsgLevel( u"Invalid Layer skipped"_s, 3 );
      mErrors.append( Error( ml->id(), QString( "Layer %1 is invalid, skipped" ).arg( ml->name() ) ) );
      continue;
    }

    if ( !ml->isInScaleRange( mSettings.scale() ) ) //|| mOverview )
    {
      QgsDebugMsgLevel( u"Layer not rendered because it is not within the defined visibility scale range"_s, 3 );
      continue;
    }

    if ( mSettings.isTemporal() && ml->temporalProperties() && !ml->temporalProperties()->isVisibleInTemporalRange( mSettings.temporalRange() ) )
    {
      QgsDebugMsgLevel( u"Layer not rendered because it is not visible within the map's time range"_s, 3 );
      continue;
    }

    if ( mPerLayerTemporalRange.contains( ml ) && ml->temporalProperties() && !ml->temporalProperties()->isVisibleInTemporalRange( mPerLayerTemporalRange[ml] ) )
    {
      QgsDebugMsgLevel( u"Layer not rendered because it is not visible within the layer's time range"_s, 3 );
      continue;
    }

    if ( !mSettings.zRange().isInfinite() && ml->elevationProperties() && !ml->elevationProperties()->isVisibleInZRange( mSettings.zRange(), ml ) )
    {
      QgsDebugMsgLevel( u"Layer not rendered because it is not visible within the map's z range"_s, 3 );
      continue;
    }

    QgsRectangle r1 = mSettings.visibleExtent(), r2;
    r1.grow( mSettings.extentBuffer() );
    QgsCoordinateTransform ct;

    ct = mSettings.layerTransform( ml );
    bool haveExtentInLayerCrs = true;
    if ( ct.isValid() )
    {
      haveExtentInLayerCrs = reprojectToLayerExtent( ml, ct, r1, r2 );
    }
    QgsDebugMsgLevel( "extent: " + r1.toString(), 3 );
    if ( !r1.isFinite() || !r2.isFinite() )
    {
      mErrors.append( Error( ml->id(), tr( "There was a problem transforming the layer's extent. Layer skipped." ) ) );
      continue;
    }

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );

    // Force render of layers that are being edited
    // or if there's a labeling engine that needs the layer to register features
    if ( mCache )
    {
      const bool requiresLabeling = ( labelingEngine2 && QgsPalLabeling::staticWillUseLayer( ml ) ) && requiresLabelRedraw;
      if ( ( vl && vl->isEditable() ) || requiresLabeling )
      {
        mCache->clearCacheImage( ml->id() );
      }
    }

    layerJobs.emplace_back( LayerRenderJob() );
    LayerRenderJob &job = layerJobs.back();
    job.layer = ml;
    job.layerId = ml->id();
    job.renderAboveLabels = ml->customProperty( u"rendering/renderAboveLabels"_s ).toBool();
    job.estimatedRenderingTime = mLayerRenderingTimeHints.value( ml->id(), 0 );

    job.setContext( std::make_unique< QgsRenderContext >( QgsRenderContext::fromMapSettings( mSettings ) ) );
    if ( !ml->customProperty( u"_noset_layer_expression_context"_s ).toBool() )
      job.context()->expressionContext().appendScope( QgsExpressionContextUtils::layerScope( ml ) );
    job.context()->setPainter( painter );
    job.context()->setLabelingEngine( labelingEngine2 );
    job.context()->setLabelSink( labelSink() );
    job.context()->setCoordinateTransform( ct );
    job.context()->setExtent( r1 );

    // Also check geographic, see: https://github.com/qgis/QGIS/issues/45200
    if ( !haveExtentInLayerCrs || ( ct.isValid() && ( ct.sourceCrs().isGeographic() != ct.destinationCrs().isGeographic() ) ) )
      job.context()->setFlag( Qgis::RenderContextFlag::ApplyClipAfterReprojection, true );

    if ( mFeatureFilterProvider )
    {
      job.context()->setFeatureFilterProvider( mFeatureFilterProvider );
    }

    if ( mPerLayerTemporalRange.contains( ml ) )
    {
      job.context()->setTemporalRange( mPerLayerTemporalRange.value( ml ) );
    }

    QgsMapLayerStyleOverride styleOverride( ml );
    if ( mSettings.layerStyleOverrides().contains( ml->id() ) )
      styleOverride.setOverrideStyle( mSettings.layerStyleOverrides().value( ml->id() ) );

    job.blendMode = ml->blendMode();

    if ( ml->type() == Qgis::LayerType::Raster )
    {
      // raster layers are abnormal wrt opacity handling -- opacity is sometimes handled directly within the raster layer renderer
      QgsRasterLayer *rl = qobject_cast< QgsRasterLayer * >( ml );
      if ( rl->renderer()->flags() & Qgis::RasterRendererFlag::InternalLayerOpacityHandling )
      {
        job.opacity = 1.0;
      }
      else
      {
        job.opacity = ml->opacity();
      }
    }
    else
    {
      job.opacity = ml->opacity();
    }

    const QgsElevationShadingRenderer shadingRenderer = mSettings.elevationShadingRenderer();

    // if we can use the cache, let's do it and avoid rendering!
    const bool canUseCache = mSettings.rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::Default && mCache;
    if ( canUseCache && mCache->hasCacheImage( ml->id() ) )
    {
      job.cached = true;
      job.imageInitialized = true;
      job.destinationImage = std::make_unique< QImage >( mCache->cacheImage( ml->id() ) );
      if ( shadingRenderer.isActive() && ml->elevationProperties() && ml->elevationProperties()->hasElevation() && mCache->hasCacheImage( ELEVATION_MAP_CACHE_PREFIX + ml->id() ) )
        job.elevationMap = std::make_unique< QgsElevationMap >( mCache->cacheImage( ELEVATION_MAP_CACHE_PREFIX + ml->id() ) );
      job.destinationImage->setDevicePixelRatio( static_cast<qreal>( mSettings.devicePixelRatio() ) );
      job.renderer.reset();
      job.context()->setPainter( nullptr );
      mLayersRedrawnFromCache.append( ml->id() );
      continue;
    }

    QElapsedTimer layerTime;
    layerTime.start();
    job.renderer.reset( ml->createMapRenderer( *( job.context() ) ) );
    if ( job.renderer )
    {
      job.renderer->setLayerRenderingTimeHint( job.estimatedRenderingTime );
      job.context()->setFeedback( job.renderer->feedback() );

      if ( job.renderer->flags().testFlag( Qgis::MapLayerRendererFlag::AffectsLabeling ) )
      {
        mAdditionalLabelLayers.append( ml );
      }
    }

    // If we are drawing with an alternative blending mode then we need to render to a separate image
    // before compositing this on the map. This effectively flattens the layer and prevents
    // blending occurring between objects on the layer
    if ( canUseCache || ( !painter && !deferredPainterSet ) || ( job.renderer && job.renderer->forceRasterRender() ) )
    {
      // Flattened image for drawing when a blending mode is set
      auto [image, painter] = allocateImageAndPainter( ml->id(), *job.context() );
      job.destinationImage = std::move( image );
      job.destinationPainter = std::move( painter );
      job.context()->setPainter( job.destinationPainter.get() );
      if ( !job.destinationImage )
      {
        job.renderer.reset();
        layerJobs.pop_back();
        continue;
      }
    }

    if ( shadingRenderer.isActive() && ml->elevationProperties() && ml->elevationProperties()->hasElevation() )
    {
      job.elevationMap = allocateElevationMap( ml->id() );
      job.context()->setElevationMap( job.elevationMap.get() );
    }

    if ( job.renderer && ( job.renderer->flags() & Qgis::MapLayerRendererFlag::RenderPartialOutputs ) && ( mSettings.flags() & Qgis::MapSettingsFlag::RenderPartialOutput ) )
    {
      if ( canUseCache && ( job.renderer->flags() & Qgis::MapLayerRendererFlag::RenderPartialOutputOverPreviousCachedImage ) && mCache->hasAnyCacheImage( job.layerId + u"_preview"_s ) )
      {
        const QImage cachedImage = mCache->transformedCacheImage( job.layerId + u"_preview"_s, mSettings.mapToPixel() );
        if ( !cachedImage.isNull() )
        {
          job.previewRenderImage = std::make_unique< QImage >( cachedImage );
          job.previewRenderImageInitialized = true;
          job.context()->setPreviewRenderPainter( new QPainter( job.previewRenderImage.get() ) );
          job.context()->setPainterFlagsUsingContext( painter );
        }
      }
      if ( !job.previewRenderImage )
      {
        auto [image, painter] = allocateImageAndPainter( ml->id(), *job.context() );
        job.previewRenderImage = std::move( image );
        job.previewRenderPainter = std::move( painter );
        job.previewRenderImageInitialized = false;
        job.context()->setPreviewRenderPainter( job.previewRenderPainter.get() );
      }

      if ( !job.previewRenderImage )
      {
        job.context()->setPreviewRenderPainter( nullptr );
        job.previewRenderPainter.reset();
      }
    }

    job.renderingTime = layerTime.elapsed(); // include job preparation time in layer rendering time
  }

  return layerJobs;
}

std::vector< LayerRenderJob > QgsMapRendererJob::prepareSecondPassJobs( std::vector< LayerRenderJob > &firstPassJobs, LabelRenderJob &labelJob )
{
  std::vector< LayerRenderJob > secondPassJobs;

  // We will need to quickly access the associated rendering job of a layer
  QHash<QString, LayerRenderJob *> layerJobMapping;

  // ... and layer that contains a mask (and whether there is effects implied or not)
  QMap<QString, bool> maskLayerHasEffects;
  QMap<int, bool> labelHasEffects;

  struct MaskSource
  {
      QString layerId;
      QString labelRuleId;
      int labelMaskId;
      bool hasEffects;
      MaskSource( const QString &layerId_, const QString &labelRuleId_, int labelMaskId_, bool hasEffects_ )
        : layerId( layerId_ )
        , labelRuleId( labelRuleId_ )
        , labelMaskId( labelMaskId_ )
        , hasEffects( hasEffects_ )
      {}
  };

  struct MaskedSymbolLayers
  {
      QSet<QString> maskedSymbolLayerIds;
      QList<MaskSource> maskSourceList;
  };

  // We collect for each layer, the set of symbol layers that will be "masked"
  // and the list of source layers that have a mask
  // Hash keys are the layer ID of the layer that WILL be masked
  QHash<QString, MaskedSymbolLayers> maskedSymbolLayers;

  const bool forceVector = mapSettings().testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) && !mapSettings().testFlag( Qgis::MapSettingsFlag::ForceRasterMasks );

  // First up, create a mapping of layer id to jobs. We need this to filter out any masking
  // which refers to layers which we aren't rendering as part of this map render
  for ( LayerRenderJob &job : firstPassJobs )
  {
    layerJobMapping[job.layerId] = &job;
  }

  const QHash<QString, QgsSelectiveMaskingSourceSet> selectiveMaskingSourceSets = mapSettings().selectiveMaskingSourceSets();

  QVector< QgsVectorLayer * > allRenderedVectorLayers;
  for ( LayerRenderJob &job : firstPassJobs )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( job.layer );
    if ( !vl )
      continue;

    allRenderedVectorLayers << vl;
  }

  // next, collate a master list of masked layers, skipping over any which refer to layers
  // which don't have a corresponding render job
  for ( LayerRenderJob &job : firstPassJobs )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( job.layer );
    if ( !vl )
      continue;

    // lambda function to factor code for both labels and symbol layers
    auto collectMasks = [&maskedSymbolLayers,
                         &maskLayerHasEffects]( const QgsMaskedLayers &objectsToBeMaskedByLayer, const QString &idOfLayerCreatingMask, const QString &ruleId = QString(), int labelMaskId = -1 ) {
      bool hasEffects = false;
      for ( auto it = objectsToBeMaskedByLayer.begin(); it != objectsToBeMaskedByLayer.end(); ++it )
      {
        const QString maskedLayerId = it.key();
        auto lit = maskedSymbolLayers.find( maskedLayerId );
        if ( lit == maskedSymbolLayers.end() )
        {
          MaskedSymbolLayers maskedObjects;
          maskedObjects.maskedSymbolLayerIds = it.value().symbolLayerIdsToMask;
          maskedObjects.maskSourceList = QList<MaskSource>() << MaskSource( idOfLayerCreatingMask, ruleId, labelMaskId, it.value().hasEffects );
          maskedSymbolLayers[maskedLayerId] = maskedObjects;
        }
        else
        {
          if ( lit->maskedSymbolLayerIds != it.value().symbolLayerIdsToMask )
          {
            QgsLogger::warning( u"Layer %1 : Different sets of symbol layers are masked by different sources ! Only one (arbitrary) set will be retained !"_s.arg( it.key() ) );
            continue;
          }
          lit->maskSourceList.push_back( MaskSource( idOfLayerCreatingMask, ruleId, labelMaskId, hasEffects ) );
        }
        hasEffects |= it.value().hasEffects;
      }
      if ( !objectsToBeMaskedByLayer.isEmpty() && labelMaskId == -1 )
        maskLayerHasEffects[idOfLayerCreatingMask] = hasEffects;
    };

    // collect label masks
    QHash<QString, QgsMaskedLayers> labelMasks = QgsVectorLayerUtils::collectObjectsMaskedByLabelsFromLayer( vl, selectiveMaskingSourceSets, allRenderedVectorLayers );
    for ( auto it = labelMasks.begin(); it != labelMasks.end(); it++ )
    {
      QString labelRule = it.key();
      // this is a hash of layer id to masks
      QgsMaskedLayers masks = it.value();

      // filter out masks to those which we are actually rendering
      QgsMaskedLayers usableMasks;
      for ( auto mit = masks.begin(); mit != masks.end(); mit++ )
      {
        const QString sourceLayerId = mit.key();
        // if we aren't rendering the source layer as part of this render, we can't process this mask
        if ( !layerJobMapping.contains( sourceLayerId ) )
          continue;
        else
          usableMasks.insert( sourceLayerId, mit.value() );
      }

      if ( usableMasks.empty() )
        continue;

      // group layers by QSet<QgsSymbolLayerReference>
      QSet<QgsSymbolLayerReference> slRefs;
      bool hasEffects = false;
      for ( auto mit = usableMasks.begin(); mit != usableMasks.end(); mit++ )
      {
        const QString sourceLayerId = mit.key();
        // if we aren't rendering the source layer as part of this render, we can't process this mask
        if ( !layerJobMapping.contains( sourceLayerId ) )
          continue;

        for ( const QString &symbolLayerId : std::as_const( mit.value().symbolLayerIdsToMask ) )
          slRefs.insert( QgsSymbolLayerReference( sourceLayerId, symbolLayerId ) );

        hasEffects |= mit.value().hasEffects;
      }
      // generate a new mask id for this set
      int labelMaskId = labelJob.maskIdProvider.insertLabelLayer( vl->id(), it.key(), slRefs );
      labelHasEffects[labelMaskId] = hasEffects;

      // now collect masks
      collectMasks( usableMasks, vl->id(), labelRule, labelMaskId );
    }

    // collect symbol layer masks
    const QgsMaskedLayers objectsToBeMaskedByLayer = QgsVectorLayerUtils::collectObjectsMaskedBySymbolLayersFromLayer( vl, selectiveMaskingSourceSets, allRenderedVectorLayers );
    collectMasks( objectsToBeMaskedByLayer, vl->id() );
  }

  if ( maskedSymbolLayers.isEmpty() )
    return secondPassJobs;

  // Prepare label mask images
  for ( int maskId = 0; maskId < labelJob.maskIdProvider.size(); maskId++ )
  {
    std::unique_ptr< QPaintDevice > maskPaintDevice;
    std::unique_ptr< QPainter > maskPainter;
    if ( forceVector && !labelHasEffects[maskId] )
    {
      // set a painter to get all masking instruction in order to later clip masked symbol layer
      auto geomPaintDevice = std::make_unique< QgsGeometryPaintDevice >( true );
      geomPaintDevice->setStrokedPathSegments( 4 );
      geomPaintDevice->setSimplificationTolerance( labelJob.context.maskSettings().simplifyTolerance() );
      maskPaintDevice = std::move( geomPaintDevice );
      maskPainter = std::make_unique< QPainter >( maskPaintDevice.get() );
    }
    else
    {
      // Note: we only need an alpha channel here, rather than a full RGBA image
      auto [maskImage, painter] = allocateImageAndPainter( u"label mask"_s, labelJob.context );

      // FIXME ? when maskImage is let to nullptr in case of out-of-memory
      // cppcheck-suppress nullPointer
      maskImage->fill( 0 );
      maskPaintDevice = std::move( maskImage );
      maskPainter = std::move( painter );
    }

    labelJob.context.setMaskPainter( maskPainter.get(), maskId );
    labelJob.maskPainters.emplace_back( std::move( maskPainter ) );
    labelJob.maskPaintDevices.emplace_back( std::move( maskPaintDevice ) );
  }
  labelJob.context.setMaskIdProvider( &labelJob.maskIdProvider );

  const bool hasNonDefaultComposition = labelingHasNonDefaultCompositionModes();

  // Prepare second pass jobs
  // - For raster rendering or vector rendering if effects are involved
  // 1st pass, 2nd pass and mask are rendered in QImage and composed in composeSecondPass
  // - For vector rendering if no effects are involved
  // 1st pass is rendered in QImage, clip paths are generated according to mask and used during
  // masked symbol layer rendering during second pass, which is rendered in QPicture, second
  // pass job picture

  // Allocate an image or picture for labels, as suitable.
  // If we have some non-default label composition modes, we CAN'T render to an image as that
  // "flattens" composition modes and prevents them interacting with underlying layers.
  const bool canUseImage = !forceVector && !hasNonDefaultComposition;
  if ( !labelJob.img && canUseImage )
  {
    labelJob.img = allocateImage( u"labels"_s );
  }
  else if ( !labelJob.picture && !canUseImage )
  {
    labelJob.picture = std::make_unique<QPicture>();
  }

  // first we initialize painter and mask painter for all jobs
  for ( LayerRenderJob &job : firstPassJobs )
  {
    job.maskRequiresLayerRasterization = false;

    auto it = maskedSymbolLayers.find( job.layerId );
    if ( it != maskedSymbolLayers.end() )
    {
      const QList<MaskSource> &sourceList = it->maskSourceList;
      for ( const MaskSource &source : sourceList )
      {
        job.maskRequiresLayerRasterization |= source.hasEffects;
      }
    }

    // update first pass job painter and device if needed
    const bool isRasterRendering = !forceVector || job.maskRequiresLayerRasterization || ( job.renderer && job.renderer->forceRasterRender() );
    if ( isRasterRendering && !job.destinationImage )
    {
      auto [img, painter] = allocateImageAndPainter( job.layerId, *job.context() );
      Q_ASSERT( !job.destinationPainter );
      job.destinationPainter = std::move( painter );
      job.context()->setPainter( job.destinationPainter.get() );
      job.destinationImage = std::move( img );
    }
    else if ( !isRasterRendering && !job.picture )
    {
      PictureAndPainter pictureAndPainter = allocatePictureAndPainter( job.context() );
      job.picture = std::move( pictureAndPainter.first );
      if ( job.context()->painter()->hasClipping() )
      {
        // need to copy clipping paths from original painter, so that e.g. the layout map bounds clipping path is respected
        pictureAndPainter.second->setClipping( true );
        pictureAndPainter.second->setClipPath( job.context()->painter()->clipPath() );
      }
      job.context()->setPainter( pictureAndPainter.second );
      // force recreation of layer renderer so it initialize correctly the renderer
      // especially the RasterLayerRender that need logicalDpiX from painting device
      job.context()->setFeedback( nullptr );
      job.renderer.reset( job.layer->createMapRenderer( *( job.context() ) ) );
      if ( job.renderer )
      {
        job.context()->setFeedback( job.renderer->feedback() );
      }
    }

    // for layer that mask, generate mask in first pass job
    if ( maskLayerHasEffects.contains( job.layerId ) )
    {
      std::unique_ptr< QPaintDevice > maskPaintDevice;
      std::unique_ptr< QPainter > maskPainter;
      if ( forceVector && !maskLayerHasEffects[job.layerId] )
      {
        // set a painter to get all masking instruction in order to later clip masked symbol layer
        auto geomPaintDevice = std::make_unique< QgsGeometryPaintDevice >();
        geomPaintDevice->setStrokedPathSegments( 4 );
        geomPaintDevice->setSimplificationTolerance( job.context()->maskSettings().simplifyTolerance() );
        maskPaintDevice = std::move( geomPaintDevice );
        maskPainter = std::make_unique< QPainter >( maskPaintDevice.get() );
      }
      else
      {
        // Note: we only need an alpha channel here, rather than a full RGBA image
        auto [maskImage, painter] = allocateImageAndPainter( job.layerId, *job.context() );
        maskImage->fill( 0 );
        maskPaintDevice = std::move( maskImage );
        maskPainter = std::move( painter );
      }

      job.context()->setMaskPainter( maskPainter.get() );
      job.maskPainter = std::move( maskPainter );
      job.maskPaintDevice = std::move( maskPaintDevice );
    }
  }

  for ( LayerRenderJob &job : firstPassJobs )
  {
    QgsMapLayer *ml = job.layer;

    auto it = maskedSymbolLayers.find( job.layerId );
    if ( it == maskedSymbolLayers.end() )
      continue;

    const QList<MaskSource> maskedSourceList = it->maskSourceList;
    const QSet<QString> maskedSymbolLayerIds = it->maskedSymbolLayerIds;

    secondPassJobs.emplace_back( LayerRenderJob() );
    LayerRenderJob &job2 = secondPassJobs.back();

    job2.maskRequiresLayerRasterization = job.maskRequiresLayerRasterization;

    // Points to the masking jobs. This will be needed during the second pass composition.
    for ( const MaskSource &source : maskedSourceList )
    {
      LayerRenderJob::MaskJob maskJob;

      if ( source.labelMaskId != -1 )
      {
        maskJob.maskPaintDeviceId = source.labelMaskId;
      }
      else
      {
        maskJob.layerRenderJob = layerJobMapping[source.layerId];
      }
      job2.maskJobs.emplace_back( maskJob );
    }

    // copy the context from the initial job
    job2.setContext( std::make_unique< QgsRenderContext >( *job.context() ) );
    // also assign layer to match initial job
    job2.layer = job.layer;
    job2.renderAboveLabels = job.renderAboveLabels;
    job2.layerId = job.layerId;

    // associate first pass job with second pass job
    job2.firstPassJob = &job;

    if ( !forceVector || job2.maskRequiresLayerRasterization )
    {
      Q_ASSERT( !job2.destinationImage );
      Q_ASSERT( !job2.destinationPainter );
      auto [img, painter] = allocateImageAndPainter( job.layerId, *job2.context() );
      job2.context()->setPainter( painter.get() );
      job2.destinationImage = std::move( img );
      job2.destinationPainter = std::move( painter );
    }
    else
    {
      PictureAndPainter pictureAndPainter = allocatePictureAndPainter( job2.context() );
      if ( job.context()->painter()->hasClipping() )
      {
        // need to copy clipping paths from original painter, so that e.g. the layout map bounds clipping path is respected
        pictureAndPainter.second->setClipping( true );
        pictureAndPainter.second->setClipPath( job.context()->painter()->clipPath() );
      }
      job2.picture = std::move( pictureAndPainter.first );
      job2.context()->setPainter( pictureAndPainter.second );
    }

    if ( !job2.destinationImage && !job2.picture )
    {
      secondPassJobs.pop_back();
      continue;
    }

    // FIXME: another possibility here, to avoid allocating a new map renderer and reuse the one from
    // the first pass job, would be to be able to call QgsMapLayerRenderer::render() with a QgsRenderContext.
    job2.context()->setFeedback( nullptr );
    job2.renderer.reset( ml->createMapRenderer( *job2.context() ) );
    if ( job2.renderer )
    {
      job2.context()->setFeedback( job2.renderer->feedback() );
    }

    // Render only the non masked symbol layer and we will compose 2nd pass with mask and first pass rendering in composeSecondPass
    // If vector output is enabled, disabled symbol layers would be actually rendered and masked with clipping path set in QgsMapRendererJob::initSecondPassJobs
    job2.context()->setDisabledSymbolLayersV2( maskedSymbolLayerIds );
  }

  return secondPassJobs;
}

QList<QPointer<QgsMapLayer> > QgsMapRendererJob::participatingLabelLayers( QgsLabelingEngine *engine )
{
  QList<QPointer<QgsMapLayer> > res = _qgis_listRawToQPointer( engine->participatingLayers() );

  for ( const QPointer<QgsMapLayer> &it : std::as_const( mAdditionalLabelLayers ) )
  {
    if ( !res.contains( it ) )
      res.append( it );
  }

  return res;
}

void QgsMapRendererJob::initSecondPassJobs( std::vector< LayerRenderJob > &secondPassJobs, LabelRenderJob &labelJob ) const
{
  if ( !mapSettings().testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) || mapSettings().testFlag( Qgis::MapSettingsFlag::ForceRasterMasks ) )
    return;

  for ( LayerRenderJob &job : secondPassJobs )
  {
    if ( job.maskRequiresLayerRasterization )
      continue;

    // we draw disabled symbol layer but me mask them with clipping path produced during first pass job
    // Resulting 2nd pass job picture will be the final rendering

    for ( const LayerRenderJob::MaskJob &p : std::as_const( job.maskJobs ) )
    {
      QPainter *maskPainter = p.layerRenderJob ? p.layerRenderJob->maskPainter.get() : labelJob.maskPainters[p.maskPaintDeviceId].get();

      const QSet<QString> layers = job.context()->disabledSymbolLayersV2();
      if ( QgsGeometryPaintDevice *geometryDevice = dynamic_cast<QgsGeometryPaintDevice *>( maskPainter->device() ) )
      {
        QgsGeometry geometry( geometryDevice->geometry().clone() );

#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 10
        // structure would be better, but too old GEOS
        geometry = geometry.makeValid( Qgis::MakeValidMethod::Linework );
#else
        geometry = geometry.makeValid( Qgis::MakeValidMethod::Structure );
#endif

        for ( const QString &symbolLayerId : layers )
        {
          job.context()->addSymbolLayerClipGeometry( symbolLayerId, geometry );
        }
      }
    }

    job.context()->setDisabledSymbolLayersV2( QSet<QString>() );
  }
}

LabelRenderJob QgsMapRendererJob::prepareLabelingJob( QPainter *painter, QgsLabelingEngine *labelingEngine2, bool canUseLabelCache )
{
  LabelRenderJob job;
  job.context = QgsRenderContext::fromMapSettings( mSettings );
  job.context.setPainter( painter );
  job.context.setLabelingEngine( labelingEngine2 );
  job.context.setFeedback( mLabelingEngineFeedback );
  if ( labelingEngine2 )
    job.context.labelingEngine()->prepare( job.context );

  QgsRectangle r1 = mSettings.visibleExtent();
  r1.grow( mSettings.extentBuffer() );
  job.context.setExtent( r1 );

  job.context.setFeatureFilterProvider( mFeatureFilterProvider );
  QgsCoordinateTransform ct;
  ct.setDestinationCrs( mSettings.destinationCrs() );
  job.context.setCoordinateTransform( ct );

  // no cache, no image allocation
  if ( mSettings.rasterizedRenderingPolicy() != Qgis::RasterizedRenderingPolicy::Default )
    return job;

  // if we can use the cache, let's do it and avoid rendering!
  bool hasCache = canUseLabelCache && mCache && mCache->hasCacheImage( LABEL_CACHE_ID ) && mCache->cacheImage( LABEL_CACHE_ID ).devicePixelRatio() == mSettings.devicePixelRatio();
  if ( hasCache )
  {
    job.cached = true;
    job.complete = true;
    job.img = std::make_unique< QImage >( mCache->cacheImage( LABEL_CACHE_ID ) );
    job.context.setPainter( nullptr );
  }
  else
  {
    if ( canUseLabelCache && ( mCache || !painter ) )
    {
      job.img = allocateImage( u"labels"_s );
    }
  }

  return job;
}


void QgsMapRendererJob::cleanupJobs( std::vector<LayerRenderJob> &jobs )
{
  for ( LayerRenderJob &job : jobs )
  {
    if ( job.destinationImage )
    {
      job.context()->setPainter( nullptr );
      job.destinationPainter.reset();

      if ( mCache && !job.cached && job.completed && job.layer )
      {
        QgsDebugMsgLevel( u"caching image for %1"_s.arg( job.layerId ), 2 );
        mCache->setCacheImageWithParameters( job.layerId, *job.destinationImage, mSettings.visibleExtent(), mSettings.mapToPixel(), QList< QgsMapLayer * >() << job.layer );
        mCache->setCacheImageWithParameters( job.layerId + u"_preview"_s, *job.destinationImage, mSettings.visibleExtent(), mSettings.mapToPixel(), QList< QgsMapLayer * >() << job.layer );
      }

      job.destinationImage.reset();
    }

    if ( job.previewRenderImage )
    {
      job.context()->setPreviewRenderPainter( nullptr );
      job.previewRenderPainter.reset();
      job.previewRenderImage.reset();
    }

    if ( job.elevationMap )
    {
      job.context()->setElevationMap( nullptr );
      if ( mCache && !job.cached && job.completed && job.layer )
      {
        QgsDebugMsgLevel( u"caching elevation map for %1"_s.arg( job.layerId ), 2 );
        mCache->setCacheImageWithParameters(
          ELEVATION_MAP_CACHE_PREFIX + job.layerId, job.elevationMap->rawElevationImage(), mSettings.visibleExtent(), mSettings.mapToPixel(), QList< QgsMapLayer * >() << job.layer
        );
        mCache->setCacheImageWithParameters(
          ELEVATION_MAP_CACHE_PREFIX + job.layerId + u"_preview"_s, job.elevationMap->rawElevationImage(), mSettings.visibleExtent(), mSettings.mapToPixel(), QList< QgsMapLayer * >() << job.layer
        );
      }

      job.elevationMap.reset();
    }

    if ( job.picture )
    {
      job.context()->setPainter( nullptr );
      job.destinationPainter.reset();
      job.picture.reset( nullptr );
    }

    if ( job.renderer )
    {
      const QStringList errors = job.renderer->errors();
      for ( const QString &message : errors )
        mErrors.append( Error( job.renderer->layerId(), message ) );

      mRenderedItemResults->appendResults( job.renderer->takeRenderedItemDetails(), *job.context() );

      job.context()->setFeedback( nullptr );
      job.renderer.reset();
    }

    if ( job.layer )
      mPerLayerRenderingTime.insert( job.layer, job.renderingTime );

    job.maskPainter.reset( nullptr );
    job.maskPaintDevice.reset( nullptr );
  }

  jobs.clear();
}

void QgsMapRendererJob::cleanupSecondPassJobs( std::vector< LayerRenderJob > &jobs )
{
  for ( LayerRenderJob &job : jobs )
  {
    if ( job.destinationImage )
    {
      job.context()->setPainter( nullptr );
      job.destinationPainter.reset();
      job.destinationImage.reset();
    }

    if ( job.previewRenderImage )
    {
      job.context()->setPreviewRenderPainter( nullptr );
      job.previewRenderPainter.reset();
      job.previewRenderImage.reset();
    }

    if ( job.picture )
    {
      job.context()->setPainter( nullptr );
      job.destinationPainter.reset();
    }

    if ( job.renderer )
    {
      job.context()->setFeedback( nullptr );
      job.renderer.reset();
    }

    if ( job.layer )
      mPerLayerRenderingTime.insert( job.layer, job.renderingTime );
  }

  jobs.clear();
}

void QgsMapRendererJob::cleanupLabelJob( LabelRenderJob &job )
{
  if ( job.img )
  {
    if ( mCache && !job.cached && !job.context.renderingStopped() )
    {
      QgsDebugMsgLevel( u"caching label result image"_s, 2 );
      mCache->setCacheImageWithParameters( LABEL_CACHE_ID, *job.img, mSettings.visibleExtent(), mSettings.mapToPixel(), _qgis_listQPointerToRaw( job.participatingLayers ) );
      mCache->setCacheImageWithParameters( LABEL_PREVIEW_CACHE_ID, *job.img, mSettings.visibleExtent(), mSettings.mapToPixel(), _qgis_listQPointerToRaw( job.participatingLayers ) );
    }

    job.img.reset();
  }

  job.picture.reset( nullptr );
  job.maskPainters.clear();
  job.maskPaintDevices.clear();
}


#define DEBUG_RENDERING 0

QImage QgsMapRendererJob::composeImage( const QgsMapSettings &settings, const std::vector<LayerRenderJob> &jobs, const LabelRenderJob &labelJob, const QgsMapRendererCache *cache )
{
  QImage image( settings.deviceOutputSize(), settings.outputImageFormat() );
  image.setDevicePixelRatio( settings.devicePixelRatio() );
  image.setDotsPerMeterX( static_cast<int>( settings.outputDpi() * 39.37 ) );
  image.setDotsPerMeterY( static_cast<int>( settings.outputDpi() * 39.37 ) );
  image.fill( settings.backgroundColor() );

  const QgsElevationShadingRenderer mapShadingRenderer = settings.elevationShadingRenderer();
  std::unique_ptr<QgsElevationMap> mainElevationMap;
  if ( mapShadingRenderer.isActive() )
    mainElevationMap = std::make_unique<QgsElevationMap>( settings.deviceOutputSize(), settings.devicePixelRatio() );

  QPainter painter( &image );

#if DEBUG_RENDERING
  int i = 0;
#endif
  for ( const LayerRenderJob &job : jobs )
  {
    if ( job.renderAboveLabels )
      continue; // skip layer for now, it will be rendered after labels

    QImage img = layerImageToBeComposed( settings, job, cache );
    if ( img.isNull() )
      continue; // image is not prepared and not even in cache

    painter.setCompositionMode( job.blendMode );
    painter.setOpacity( job.opacity );

    if ( mainElevationMap )
    {
      QgsElevationMap layerElevationMap = layerElevationToBeComposed( settings, job, cache );
      if ( layerElevationMap.isValid() )
        mainElevationMap->combine( layerElevationMap, mapShadingRenderer.combinedElevationMethod() );
    }


#if DEBUG_RENDERING
    img.save( QString( "/tmp/final_%1.png" ).arg( i ) );
    i++;
#endif

    painter.drawImage( 0, 0, img );
  }

  if ( mapShadingRenderer.isActive() && mainElevationMap )
  {
    mapShadingRenderer.renderShading( *mainElevationMap.get(), image, QgsRenderContext::fromMapSettings( settings ) );
  }

  // IMPORTANT - don't draw labelJob img before the label job is complete,
  // as the image is uninitialized and full of garbage before the label job
  // commences
  if ( labelJob.img && labelJob.complete )
  {
    painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
    painter.setOpacity( 1.0 );
    painter.drawImage( 0, 0, *labelJob.img );
  }
  else if ( labelJob.picture && labelJob.complete )
  {
    painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
    painter.setOpacity( 1.0 );
    QgsPainting::drawPicture( &painter, QPointF( 0, 0 ), *labelJob.picture );
  }
  // when checking for a label cache image, we only look for those which would be drawn between 30% and 300% of the
  // original size. We don't want to draw massive pixelated labels on top of everything else, and we also don't need
  // to draw tiny unreadable labels... better to draw nothing in this case and wait till the updated label results are ready!
  else if ( cache && cache->hasAnyCacheImage( LABEL_PREVIEW_CACHE_ID, 0.3, 3 ) )
  {
    const QImage labelCacheImage = cache->transformedCacheImage( LABEL_PREVIEW_CACHE_ID, settings.mapToPixel() );
    painter.setCompositionMode( QPainter::CompositionMode_SourceOver );
    painter.setOpacity( 1.0 );
    painter.drawImage( 0, 0, labelCacheImage );
  }

  // render any layers with the renderAboveLabels flag now
  for ( const LayerRenderJob &job : jobs )
  {
    if ( !job.renderAboveLabels )
      continue;

    QImage img = layerImageToBeComposed( settings, job, cache );
    if ( img.isNull() )
      continue; // image is not prepared and not even in cache

    painter.setCompositionMode( job.blendMode );
    painter.setOpacity( job.opacity );

    painter.drawImage( 0, 0, img );
  }

  painter.end();
#if DEBUG_RENDERING
  image.save( "/tmp/final.png" );
#endif
  return image;
}

QImage QgsMapRendererJob::layerImageToBeComposed( const QgsMapSettings &settings, const LayerRenderJob &job, const QgsMapRendererCache *cache )
{
  if ( job.imageCanBeComposed() )
  {
    if ( job.previewRenderImage && !job.completed )
      return *job.previewRenderImage;

    Q_ASSERT( job.destinationImage );
    return *job.destinationImage;
  }
  else
  {
    if ( cache && cache->hasAnyCacheImage( job.layerId + u"_preview"_s ) )
    {
      return cache->transformedCacheImage( job.layerId + u"_preview"_s, settings.mapToPixel() );
    }
    else
      return QImage();
  }
}

QgsElevationMap QgsMapRendererJob::layerElevationToBeComposed( const QgsMapSettings &settings, const LayerRenderJob &job, const QgsMapRendererCache *cache )
{
  if ( job.imageCanBeComposed() && job.elevationMap )
  {
    return *job.elevationMap;
  }
  else
  {
    if ( cache && cache->hasAnyCacheImage( ELEVATION_MAP_CACHE_PREFIX + job.layerId + u"_preview"_s ) )
      return QgsElevationMap( cache->transformedCacheImage( ELEVATION_MAP_CACHE_PREFIX + job.layerId + u"_preview"_s, settings.mapToPixel() ) );
    else
      return QgsElevationMap();
  }
}

void QgsMapRendererJob::composeSecondPass( std::vector<LayerRenderJob> &secondPassJobs, LabelRenderJob &labelJob, bool forceVector )
{
  // compose the second pass with the mask
  for ( LayerRenderJob &job : secondPassJobs )
  {
    const bool isRasterRendering = !forceVector || job.maskRequiresLayerRasterization;

    // Merge all mask images into the first one if we have more than one mask image
    if ( isRasterRendering && job.maskJobs.size() > 1 )
    {
      QPainter *maskPainter = nullptr;
      for ( const LayerRenderJob::MaskJob &p : job.maskJobs )
      {
        QImage *maskImage = static_cast<QImage *>( p.layerRenderJob ? p.layerRenderJob->maskPaintDevice.get() : labelJob.maskPaintDevices[p.maskPaintDeviceId].get() );
        if ( !maskPainter )
        {
          maskPainter = p.layerRenderJob ? p.layerRenderJob->maskPainter.get() : labelJob.maskPainters[p.maskPaintDeviceId].get();
        }
        else
        {
          maskPainter->drawImage( 0, 0, *maskImage );
        }
      }
    }

    if ( !job.maskJobs.empty() )
    {
      // All have been merged into the first
      LayerRenderJob::MaskJob p = *job.maskJobs.begin();
      if ( isRasterRendering )
      {
        QImage *maskImage = static_cast<QImage *>( p.layerRenderJob ? p.layerRenderJob->maskPaintDevice.get() : labelJob.maskPaintDevices[p.maskPaintDeviceId].get() );

        // Only retain parts of the second rendering that are "inside" the mask image
        QPainter *painter = job.context()->painter();

        painter->setCompositionMode( QPainter::CompositionMode_DestinationIn );

        //Create an "alpha binarized" image of the maskImage to :
        //* Eliminate antialiasing artifact
        //* Avoid applying mask opacity to elements under the mask but not masked
        QImage maskBinAlpha = maskImage->createMaskFromColor( 0 );
        QVector<QRgb> mswTable;
        mswTable.push_back( qRgba( 0, 0, 0, 255 ) );
        mswTable.push_back( qRgba( 0, 0, 0, 0 ) );
        maskBinAlpha.setColorTable( mswTable );
        painter->drawImage( 0, 0, maskBinAlpha );

        // Modify the first pass' image ...
        {
          QPainter tempPainter;

          // reuse the first pass painter, if available
          QPainter *painter1 = job.firstPassJob->context()->painter();
          if ( !painter1 )
          {
            tempPainter.begin( job.firstPassJob->destinationImage.get() );
            painter1 = &tempPainter;
          }

          // ... first retain parts that are "outside" the mask image
          painter1->setCompositionMode( QPainter::CompositionMode_DestinationOut );
          painter1->drawImage( 0, 0, *maskImage );

          // ... and overpaint the second pass' image on it
          painter1->setCompositionMode( QPainter::CompositionMode_DestinationOver );
          painter1->drawImage( 0, 0, *job.destinationImage );
        }
      }
      else
      {
        job.firstPassJob->picture = std::move( job.picture );
        job.picture = nullptr;
      }
    }
  }
}

void QgsMapRendererJob::logRenderingTime( const std::vector< LayerRenderJob > &jobs, const std::vector< LayerRenderJob > &secondPassJobs, const LabelRenderJob &labelJob )
{
  if ( !settingsLogCanvasRefreshEvent->value() )
    return;

  QMultiMap<int, QString> elapsed;
  for ( const LayerRenderJob &job : jobs )
    elapsed.insert( job.renderingTime, job.layerId );
  for ( const LayerRenderJob &job : secondPassJobs )
    elapsed.insert( job.renderingTime, job.layerId + QString( " (second pass)" ) );

  elapsed.insert( labelJob.renderingTime, tr( "Labeling" ) );

  QList<int> tt( elapsed.uniqueKeys() );
  std::sort( tt.begin(), tt.end(), std::greater<int>() );
  for ( int t : std::as_const( tt ) )
  {
    QgsMessageLog::logMessage( tr( "%1 ms: %2" ).arg( t ).arg( QStringList( elapsed.values( t ) ).join( ", "_L1 ) ), tr( "Rendering" ) );
  }
  QgsMessageLog::logMessage( u"---"_s, tr( "Rendering" ) );
}

void QgsMapRendererJob::drawLabeling( QgsRenderContext &renderContext, QgsLabelingEngine *labelingEngine2, QPainter *painter )
{
  QgsDebugMsgLevel( u"Draw labeling start"_s, 5 );

  std::unique_ptr< QgsScopedRuntimeProfile > labelingProfile;
  if ( renderContext.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    labelingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "(labeling)" ), u"rendering"_s );
  }

  QElapsedTimer t;
  t.start();

  // Reset the composition mode before rendering the labels
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  renderContext.setPainter( painter );

  if ( labelingEngine2 )
  {
    labelingEngine2->run( renderContext );
  }

  QgsDebugMsgLevel( u"Draw labeling took (seconds): %1"_s.arg( t.elapsed() / 1000. ), 2 );
}

void QgsMapRendererJob::drawLabeling( const QgsMapSettings &settings, QgsRenderContext &renderContext, QgsLabelingEngine *labelingEngine2, QPainter *painter )
{
  Q_UNUSED( settings )

  drawLabeling( renderContext, labelingEngine2, painter );
}

///@endcond PRIVATE
