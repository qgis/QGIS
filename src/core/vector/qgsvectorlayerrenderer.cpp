/***************************************************************************
  qgsvectorlayerrenderer.cpp
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

#include "qgsvectorlayerrenderer.h"

#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsrenderer.h"
#include "qgsrendercontext.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbollayer.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerdiagramprovider.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgspainteffect.h"
#include "qgsfeaturefilterprovider.h"
#include "qgsexception.h"
#include "qgslabelsink.h"
#include "qgslogger.h"
#include "qgssettingsregistrycore.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsvectorlayerselectionproperties.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsmapclippingutils.h"
#include "qgsfeaturerenderergenerator.h"
#include "qgssettingsentryimpl.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"

#include <QPicture>
#include <QTimer>
#include <QThread>

QgsVectorLayerRenderer::QgsVectorLayerRenderer( QgsVectorLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mLayer( layer )
  , mLayerName( layer->name() )
  , mFields( layer->fields() )
  , mSource( std::make_unique< QgsVectorLayerFeatureSource >( layer ) )
  , mNoSetLayerExpressionContext( layer->customProperty( QStringLiteral( "_noset_layer_expression_context" ) ).toBool() )
  , mEnableProfile( context.flags() & Qgis::RenderContextFlag::RecordProfile )
{
  QElapsedTimer timer;
  timer.start();
  std::unique_ptr< QgsFeatureRenderer > mainRenderer( layer->renderer() ? layer->renderer()->clone() : nullptr );

  QgsVectorLayerSelectionProperties *selectionProperties = qobject_cast< QgsVectorLayerSelectionProperties * >( layer->selectionProperties() );
  switch ( selectionProperties->selectionRenderingMode() )
  {
    case Qgis::SelectionRenderingMode::Default:
      break;

    case Qgis::SelectionRenderingMode::CustomColor:
    {
      // overwrite default selection color if layer has a specific selection color set
      const QColor layerSelectionColor = selectionProperties->selectionColor();
      if ( layerSelectionColor.isValid() )
        context.setSelectionColor( layerSelectionColor );
      break;
    }

    case Qgis::SelectionRenderingMode::CustomSymbol:
    {
      if ( QgsSymbol *selectionSymbol =  qobject_cast< QgsVectorLayerSelectionProperties * >( layer->selectionProperties() )->selectionSymbol() )
        mSelectionSymbol.reset( selectionSymbol->clone() );
      break;
    }
  }

  if ( !mainRenderer )
    return;

  QList< const QgsFeatureRendererGenerator * > generators = layer->featureRendererGenerators();
  std::sort( generators.begin(), generators.end(), []( const QgsFeatureRendererGenerator * g1, const QgsFeatureRendererGenerator * g2 )
  {
    return g1->level() < g2->level();
  } );

  bool insertedMainRenderer = false;
  double prevLevel = std::numeric_limits< double >::lowest();
  // cppcheck-suppress danglingLifetime
  mRenderer = mainRenderer.get();
  for ( const QgsFeatureRendererGenerator *generator : std::as_const( generators ) )
  {
    if ( generator->level() >= 0 && prevLevel < 0 && !insertedMainRenderer )
    {
      // insert main renderer when level changes from <0 to >0
      mRenderers.emplace_back( std::move( mainRenderer ) );
      insertedMainRenderer = true;
    }
    mRenderers.emplace_back( generator->createRenderer() );
    prevLevel = generator->level();
  }
  // cppcheck-suppress accessMoved
  if ( mainRenderer )
  {
    // cppcheck-suppress accessMoved
    mRenderers.emplace_back( std::move( mainRenderer ) );
  }

  mSelectedFeatureIds = layer->selectedFeatureIds();

  mDrawVertexMarkers = nullptr != layer->editBuffer();

  mGeometryType = layer->geometryType();

  mFeatureBlendMode = layer->featureBlendMode();

  if ( context.isTemporal() )
  {
    QgsVectorLayerTemporalContext temporalContext;
    temporalContext.setLayer( layer );
    mTemporalFilter = qobject_cast< const QgsVectorLayerTemporalProperties * >( layer->temporalProperties() )->createFilterString( temporalContext, context.temporalRange() );
    QgsDebugMsgLevel( "Rendering with Temporal Filter: " + mTemporalFilter, 2 );
  }

  // if there's already a simplification method specified via the context, we respect that. Otherwise, we fall back
  // to the layer's individual setting
  if ( renderContext()->vectorSimplifyMethod().simplifyHints() != Qgis::VectorRenderingSimplificationFlags( Qgis::VectorRenderingSimplificationFlag::NoSimplification ) )
  {
    mSimplifyMethod = renderContext()->vectorSimplifyMethod();
    mSimplifyGeometry = renderContext()->vectorSimplifyMethod().simplifyHints() & Qgis::VectorRenderingSimplificationFlag::GeometrySimplification ||
                        renderContext()->vectorSimplifyMethod().simplifyHints() & Qgis::VectorRenderingSimplificationFlag::FullSimplification;
  }
  else
  {
    mSimplifyMethod = layer->simplifyMethod();
    mSimplifyGeometry = layer->simplifyDrawingCanbeApplied( *renderContext(), Qgis::VectorRenderingSimplificationFlag::GeometrySimplification );
  }

  mVertexMarkerOnlyForSelection = QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected->value();

  QString markerTypeString = QgsSettingsRegistryCore::settingsDigitizingMarkerStyle->value();
  if ( markerTypeString == QLatin1String( "Cross" ) )
  {
    mVertexMarkerStyle = Qgis::VertexMarkerType::Cross;
  }
  else if ( markerTypeString == QLatin1String( "SemiTransparentCircle" ) )
  {
    mVertexMarkerStyle = Qgis::VertexMarkerType::SemiTransparentCircle;
  }
  else
  {
    mVertexMarkerStyle = Qgis::VertexMarkerType::NoMarker;
  }

  mVertexMarkerSize = QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm->value();

  // cppcheck-suppress danglingLifetime
  QgsDebugMsgLevel( "rendering v2:\n  " + mRenderer->dump(), 2 );

  if ( mDrawVertexMarkers )
  {
    // set editing vertex markers style (main renderer only)
    mRenderer->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
  }
  if ( !mNoSetLayerExpressionContext )
    renderContext()->expressionContext() << QgsExpressionContextUtils::layerScope( layer );

  for ( const std::unique_ptr< QgsFeatureRenderer > &renderer : mRenderers )
  {
    mAttrNames.unite( renderer->usedAttributes( context ) );
  }
  if ( context.hasRenderedFeatureHandlers() )
  {
    const QList< QgsRenderedFeatureHandlerInterface * > handlers = context.renderedFeatureHandlers();
    for ( QgsRenderedFeatureHandlerInterface *handler : handlers )
      mAttrNames.unite( handler->usedAttributes( layer, context ) );
  }

  //register label and diagram layer to the labeling engine
  prepareLabeling( layer, mAttrNames );
  prepareDiagrams( layer, mAttrNames );

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( context, layer );

  if ( std::any_of( mRenderers.begin(), mRenderers.end(), []( const auto & renderer ) { return renderer->forceRasterRender(); } ) )
  {
    //raster rendering is forced for this layer
    mForceRasterRender = true;
  }

  if ( context.testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) &&
       ( ( layer->blendMode() != QPainter::CompositionMode_SourceOver )
         || ( layer->featureBlendMode() != QPainter::CompositionMode_SourceOver )
         || ( !qgsDoubleNear( layer->opacity(), 1.0 ) ) ) )
  {
    //layer properties require rasterization
    mForceRasterRender = true;
  }

  mReadyToCompose = false;
  mPreparationTime = timer.elapsed();
}

QgsVectorLayerRenderer::~QgsVectorLayerRenderer() = default;

void QgsVectorLayerRenderer::setLayerRenderingTimeHint( int time )
{
  mRenderTimeHint = time;
}

QgsFeedback *QgsVectorLayerRenderer::feedback() const
{
  return mFeedback.get();
}

bool QgsVectorLayerRenderer::forceRasterRender() const
{
  return mForceRasterRender;
}

bool QgsVectorLayerRenderer::render()
{
  if ( mGeometryType == Qgis::GeometryType::Null || mGeometryType == Qgis::GeometryType::Unknown )
  {
    mReadyToCompose = true;
    return true;
  }

  if ( mRenderers.empty() )
  {
    mReadyToCompose = true;
    mErrors.append( QObject::tr( "No renderer for drawing." ) );
    return false;
  }

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
  }

  // if the previous layer render was relatively quick (e.g. less than 3 seconds), the we show any previously
  // cached version of the layer during rendering instead of the usual progressive updates
  if ( mRenderTimeHint > 0 && mRenderTimeHint <= MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mBlockRenderUpdates = true;
    mElapsedTimer.start();
  }

  bool res = true;
  int rendererIndex = 0;
  for ( const std::unique_ptr< QgsFeatureRenderer > &renderer : mRenderers )
  {
    if ( mFeedback->isCanceled() || !res )
    {
      break;
    }
    res = renderInternal( renderer.get(), rendererIndex++ ) && res;
  }

  mReadyToCompose = true;
  return res && !renderContext()->renderingStopped();
}

bool QgsVectorLayerRenderer::renderInternal( QgsFeatureRenderer *renderer, int rendererIndex )
{
  const bool isMainRenderer = renderer == mRenderer;

  QgsRenderContext &context = *renderContext();
  context.setSymbologyReferenceScale( renderer->referenceScale() );

  if ( renderer->type() == QLatin1String( "nullSymbol" ) )
  {
    // a little shortcut for the null symbol renderer - most of the time it is not going to render anything
    // so we can even skip the whole loop to fetch features
    if ( !isMainRenderer ||
         ( !mDrawVertexMarkers && !mLabelProvider && !mDiagramProvider && mSelectedFeatureIds.isEmpty() ) )
      return true;
  }

  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    QString title;
    if ( mRenderers.size() > 1 )
      title = QObject::tr( "Preparing render %1" ).arg( rendererIndex + 1 );
    else
      title = QObject::tr( "Preparing render" );
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( title, QStringLiteral( "rendering" ) );
  }

  QgsScopedQPainterState painterState( context.painter() );

  bool usingEffect = false;
  if ( renderer->paintEffect() && renderer->paintEffect()->enabled() )
  {
    usingEffect = true;
    renderer->paintEffect()->begin( context );
  }

  // Per feature blending mode
  if ( context.useAdvancedEffects() && mFeatureBlendMode != QPainter::CompositionMode_SourceOver )
  {
    // set the painter to the feature blend mode, so that features drawn
    // on this layer will interact and blend with each other
    context.painter()->setCompositionMode( mFeatureBlendMode );
  }

  renderer->startRender( context, mFields );

  if ( renderer->canSkipRender() )
  {
    // nothing to draw for now...
    renderer->stopRender( context );
    return true;
  }

  QString rendererFilter = renderer->filter( mFields );

  QgsRectangle requestExtent = context.extent();
  if ( !mClippingRegions.empty() )
  {
    mClipFilterGeom = QgsMapClippingUtils::calculateFeatureRequestGeometry( mClippingRegions, context, mApplyClipFilter );
    requestExtent = requestExtent.intersect( mClipFilterGeom.boundingBox() );

    mClipFeatureGeom = QgsMapClippingUtils::calculateFeatureIntersectionGeometry( mClippingRegions, context, mApplyClipGeometries );

    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, context, Qgis::LayerType::Vector, needsPainterClipPath );
    if ( needsPainterClipPath )
      context.painter()->setClipPath( path, Qt::IntersectClip );

    mLabelClipFeatureGeom = QgsMapClippingUtils::calculateLabelIntersectionGeometry( mClippingRegions, context, mApplyLabelClipGeometries );

    if ( mDiagramProvider )
      mDiagramProvider->setClipFeatureGeometry( mLabelClipFeatureGeom );
  }
  renderer->modifyRequestExtent( requestExtent, context );

  QgsFeatureRequest featureRequest = QgsFeatureRequest()
                                     .setFilterRect( requestExtent )
                                     .setSubsetOfAttributes( mAttrNames, mFields )
                                     .setExpressionContext( context.expressionContext() );
  if ( renderer->orderByEnabled() )
  {
    featureRequest.setOrderBy( renderer->orderBy() );
  }

  const QgsFeatureFilterProvider *featureFilterProvider = context.featureFilterProvider();
  if ( featureFilterProvider )
  {
    featureFilterProvider->filterFeatures( mLayer, featureRequest );
  }
  if ( !rendererFilter.isEmpty() && rendererFilter != QLatin1String( "TRUE" ) )
  {
    featureRequest.combineFilterExpression( rendererFilter );
  }
  if ( !mTemporalFilter.isEmpty() )
  {
    featureRequest.combineFilterExpression( mTemporalFilter );
  }

  if ( renderer->usesEmbeddedSymbols() )
  {
    featureRequest.setFlags( featureRequest.flags() | Qgis::FeatureRequestFlag::EmbeddedSymbols );
  }

  // enable the simplification of the geometries (Using the current map2pixel context) before send it to renderer engine.
  if ( mSimplifyGeometry )
  {
    double map2pixelTol = mSimplifyMethod.threshold();
    bool validTransform = true;

    const QgsMapToPixel &mtp = context.mapToPixel();
    map2pixelTol *= mtp.mapUnitsPerPixel();
    const QgsCoordinateTransform ct = context.coordinateTransform();

    // resize the tolerance using the change of size of an 1-BBOX from the source CoordinateSystem to the target CoordinateSystem
    if ( ct.isValid() && !ct.isShortCircuited() )
    {
      try
      {
        QgsCoordinateTransform toleranceTransform = ct;
        QgsPointXY center = context.extent().center();
        double rectSize = toleranceTransform.sourceCrs().mapUnits() == Qgis::DistanceUnit::Degrees ? 0.0008983 /* ~100/(40075014/360=111319.4833) */ : 100;

        QgsRectangle sourceRect = QgsRectangle( center.x(), center.y(), center.x() + rectSize, center.y() + rectSize );
        toleranceTransform.setBallparkTransformsAreAppropriate( true );
        QgsRectangle targetRect = toleranceTransform.transform( sourceRect );

        QgsDebugMsgLevel( QStringLiteral( "Simplify - SourceTransformRect=%1" ).arg( sourceRect.toString( 16 ) ), 4 );
        QgsDebugMsgLevel( QStringLiteral( "Simplify - TargetTransformRect=%1" ).arg( targetRect.toString( 16 ) ), 4 );

        if ( !sourceRect.isEmpty() && sourceRect.isFinite() && !targetRect.isEmpty() && targetRect.isFinite() )
        {
          QgsPointXY minimumSrcPoint( sourceRect.xMinimum(), sourceRect.yMinimum() );
          QgsPointXY maximumSrcPoint( sourceRect.xMaximum(), sourceRect.yMaximum() );
          QgsPointXY minimumDstPoint( targetRect.xMinimum(), targetRect.yMinimum() );
          QgsPointXY maximumDstPoint( targetRect.xMaximum(), targetRect.yMaximum() );

          double sourceHypothenuse = std::sqrt( minimumSrcPoint.sqrDist( maximumSrcPoint ) );
          double targetHypothenuse = std::sqrt( minimumDstPoint.sqrDist( maximumDstPoint ) );

          QgsDebugMsgLevel( QStringLiteral( "Simplify - SourceHypothenuse=%1" ).arg( sourceHypothenuse ), 4 );
          QgsDebugMsgLevel( QStringLiteral( "Simplify - TargetHypothenuse=%1" ).arg( targetHypothenuse ), 4 );

          if ( !qgsDoubleNear( targetHypothenuse, 0.0 ) )
            map2pixelTol *= ( sourceHypothenuse / targetHypothenuse );
        }
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( QObject::tr( "Simplify transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
        validTransform = false;
      }
    }

    if ( validTransform )
    {
      QgsSimplifyMethod simplifyMethod;
      simplifyMethod.setMethodType( QgsSimplifyMethod::OptimizeForRendering );
      simplifyMethod.setTolerance( map2pixelTol );
      simplifyMethod.setThreshold( mSimplifyMethod.threshold() );
      simplifyMethod.setForceLocalOptimization( mSimplifyMethod.forceLocalOptimization() );
      featureRequest.setSimplifyMethod( simplifyMethod );

      QgsVectorSimplifyMethod vectorMethod = mSimplifyMethod;
      vectorMethod.setTolerance( map2pixelTol );
      context.setVectorSimplifyMethod( vectorMethod );
    }
    else
    {
      QgsVectorSimplifyMethod vectorMethod;
      vectorMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::NoSimplification );
      context.setVectorSimplifyMethod( vectorMethod );
    }
  }
  else
  {
    QgsVectorSimplifyMethod vectorMethod;
    vectorMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::NoSimplification );
    context.setVectorSimplifyMethod( vectorMethod );
  }

  featureRequest.setFeedback( mFeedback.get() );
  // also set the interruption checker for the expression context, in case the renderer uses some complex expression
  // which could benefit from early exit paths...
  context.expressionContext().setFeedback( mFeedback.get() );

  std::unique_ptr< QgsScopedRuntimeProfile > preparingFeatureItProfile;
  if ( mEnableProfile )
  {
    preparingFeatureItProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Prepare feature iteration" ), QStringLiteral( "rendering" ) );
  }

  QgsFeatureIterator fit = mSource->getFeatures( featureRequest );
  // Attach an interruption checker so that iterators that have potentially
  // slow fetchFeature() implementations, such as in the WFS provider, can
  // check it, instead of relying on just the mContext.renderingStopped() check
  // in drawRenderer()

  fit.setInterruptionChecker( mFeedback.get() );

  preparingFeatureItProfile.reset();
  preparingProfile.reset();

  std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
  if ( mEnableProfile )
  {
    renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering" ), QStringLiteral( "rendering" ) );
  }

  if ( ( renderer->capabilities() & QgsFeatureRenderer::SymbolLevels ) && renderer->usingSymbolLevels() )
    drawRendererLevels( renderer, fit );
  else
    drawRenderer( renderer, fit );

  if ( !fit.isValid() )
  {
    mErrors.append( QStringLiteral( "Data source invalid" ) );
  }

  if ( usingEffect )
  {
    renderer->paintEffect()->end( context );
  }

  context.expressionContext().setFeedback( nullptr );
  return true;
}


void QgsVectorLayerRenderer::drawRenderer( QgsFeatureRenderer *renderer, QgsFeatureIterator &fit )
{
  QElapsedTimer timer;
  timer.start();
  quint64 totalLabelTime = 0;

  const bool isMainRenderer = renderer == mRenderer;

  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  QgsRenderContext &context = *renderContext();
  context.expressionContext().appendScope( symbolScope );

  std::unique_ptr< QgsGeometryEngine > clipEngine;
  if ( mApplyClipFilter )
  {
    clipEngine.reset( QgsGeometry::createGeometryEngine( mClipFilterGeom.constGet() ) );
    clipEngine->prepareGeometry();
  }

  if ( mSelectionSymbol && isMainRenderer )
    mSelectionSymbol->startRender( context, mFields );

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    try
    {
      if ( context.renderingStopped() )
      {
        QgsDebugMsgLevel( QStringLiteral( "Drawing of vector layer %1 canceled." ).arg( layerId() ), 2 );
        break;
      }

      if ( !fet.hasGeometry() || fet.geometry().isEmpty() )
        continue; // skip features without geometry

      if ( clipEngine && !clipEngine->intersects( fet.geometry().constGet() ) )
        continue; // skip features outside of clipping region

      if ( mApplyClipGeometries )
        context.setFeatureClipGeometry( mClipFeatureGeom );

      if ( ! mNoSetLayerExpressionContext )
        context.expressionContext().setFeature( fet );

      const bool featureIsSelected = isMainRenderer && context.showSelection() && mSelectedFeatureIds.contains( fet.id() );
      bool drawMarker = isMainRenderer && ( mDrawVertexMarkers && context.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || featureIsSelected ) );

      // render feature
      bool rendered = false;
      if ( !context.testFlag( Qgis::RenderContextFlag::SkipSymbolRendering ) )
      {
        if ( featureIsSelected && mSelectionSymbol )
        {
          // note: here we pass "false" for the selected argument, as we don't want to change
          // the user's defined selection symbol colors or settings in any way
          mSelectionSymbol->renderFeature( fet, context, -1, false, drawMarker );
          rendered = renderer->willRenderFeature( fet, context );
        }
        else
        {
          rendered = renderer->renderFeature( fet, context, -1, featureIsSelected, drawMarker );
        }
      }
      else
      {
        rendered = renderer->willRenderFeature( fet, context );
      }

      // labeling - register feature
      if ( rendered )
      {
        // as soon as first feature is rendered, we can start showing layer updates.
        // but if we are blocking render updates (so that a previously cached image is being shown), we wait
        // at most e.g. 3 seconds before we start forcing progressive updates.
        if ( !mBlockRenderUpdates || mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
        {
          mReadyToCompose = true;
        }

        // new labeling engine
        if ( isMainRenderer && context.labelingEngine() && ( mLabelProvider || mDiagramProvider ) )
        {
          const quint64 startLabelTime = timer.elapsed();
          QgsGeometry obstacleGeometry;
          QgsSymbolList symbols = renderer->originalSymbolsForFeature( fet, context );
          QgsSymbol *symbol = nullptr;
          if ( !symbols.isEmpty() && fet.geometry().type() == Qgis::GeometryType::Point )
          {
            obstacleGeometry = QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, context, symbols );
          }

          if ( !symbols.isEmpty() )
          {
            symbol = symbols.at( 0 );
            QgsExpressionContextUtils::updateSymbolScope( symbol, symbolScope );
          }

          if ( mApplyLabelClipGeometries )
            context.setFeatureClipGeometry( mLabelClipFeatureGeom );

          if ( mLabelProvider )
          {
            mLabelProvider->registerFeature( fet, context, obstacleGeometry, symbol );
          }
          if ( mDiagramProvider )
          {
            mDiagramProvider->registerFeature( fet, context, obstacleGeometry );
          }

          if ( mApplyLabelClipGeometries )
            context.setFeatureClipGeometry( QgsGeometry() );

          totalLabelTime += ( timer.elapsed() - startLabelTime );
        }
      }
    }
    catch ( const QgsCsException &cse )
    {
      Q_UNUSED( cse )
      QgsDebugError( QStringLiteral( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                     .arg( fet.id() ).arg( cse.what() ) );
    }
  }

  delete context.expressionContext().popScope();

  std::unique_ptr< QgsScopedRuntimeProfile > cleanupProfile;
  if ( mEnableProfile )
  {
    QgsApplication::profiler()->record( QObject::tr( "Rendering features" ), ( timer.elapsed() - totalLabelTime ) / 1000.0, QStringLiteral( "rendering" ) );
    if ( totalLabelTime > 0 )
    {
      QgsApplication::profiler()->record( QObject::tr( "Registering labels" ), totalLabelTime / 1000.0, QStringLiteral( "rendering" ) );
    }
    cleanupProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Finalizing" ), QStringLiteral( "rendering" ) );
  }

  if ( mSelectionSymbol && isMainRenderer )
    mSelectionSymbol->stopRender( context );

  stopRenderer( renderer, nullptr );
}

void QgsVectorLayerRenderer::drawRendererLevels( QgsFeatureRenderer *renderer, QgsFeatureIterator &fit )
{
  const bool isMainRenderer = renderer == mRenderer;

  // We need to figure out in which order all the features should be rendered.
  // Ordering is based on (a) a "level" which is determined by the configured
  // feature rendering order" and (b) the symbol level. The "level" is
  // determined by the values of the attributes defined in the feature
  // rendering order settings. Each time the attribute(s) have a new distinct
  // value, a new empty QHash is added to the "features" list. This QHash is
  // then filled by mappings from the symbol to a list of all the features
  // that should be rendered by that symbol.
  //
  // If orderBy is not enabled, this list will only ever contain a single
  // element.
  QList<QHash< QgsSymbol *, QList<QgsFeature> >> features;

  // We have at least one "level" for the features.
  features.push_back( {} );

  QSet<int> orderByAttributeIdx;
  if ( renderer->orderByEnabled() )
  {
    orderByAttributeIdx = renderer->orderBy().usedAttributeIndices( mSource->fields() );
  }

  QgsRenderContext &context = *renderContext();

  QgsSingleSymbolRenderer *selRenderer = nullptr;
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    selRenderer = new QgsSingleSymbolRenderer( QgsSymbol::defaultSymbol( mGeometryType ) );
    selRenderer->symbol()->setColor( context.selectionColor() );
    selRenderer->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
    selRenderer->startRender( context, mFields );
  }

  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  std::unique_ptr< QgsExpressionContextScopePopper > scopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.expressionContext(), symbolScope );


  std::unique_ptr< QgsGeometryEngine > clipEngine;
  if ( mApplyClipFilter )
  {
    clipEngine.reset( QgsGeometry::createGeometryEngine( mClipFilterGeom.constGet() ) );
    clipEngine->prepareGeometry();
  }

  if ( mApplyLabelClipGeometries )
    context.setFeatureClipGeometry( mLabelClipFeatureGeom );

  std::unique_ptr< QgsScopedRuntimeProfile > fetchFeaturesProfile;
  if ( mEnableProfile )
  {
    fetchFeaturesProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Fetching features" ), QStringLiteral( "rendering" ) );
  }

  QElapsedTimer timer;
  timer.start();
  quint64 totalLabelTime = 0;

  // 1. fetch features
  QgsFeature fet;
  QVector<QVariant> prevValues; // previous values of ORDER BY attributes
  while ( fit.nextFeature( fet ) )
  {
    if ( context.renderingStopped() )
    {
      qDebug( "rendering stop!" );
      stopRenderer( renderer, selRenderer );
      return;
    }

    if ( !fet.hasGeometry() )
      continue; // skip features without geometry

    if ( clipEngine && !clipEngine->intersects( fet.geometry().constGet() ) )
      continue; // skip features outside of clipping region

    if ( ! mNoSetLayerExpressionContext )
      context.expressionContext().setFeature( fet );
    QgsSymbol *sym = renderer->symbolForFeature( fet, context );
    if ( !sym )
    {
      continue;
    }

    if ( renderer->orderByEnabled() )
    {
      QVector<QVariant> currentValues;
      for ( const int idx : std::as_const( orderByAttributeIdx ) )
      {
        currentValues.push_back( fet.attribute( idx ) );
      }
      if ( prevValues.empty() )
      {
        prevValues = std::move( currentValues );
      }
      else if ( currentValues != prevValues )
      {
        // Current values of ORDER BY attributes are different than previous
        // values of these attributes. Start a new level.
        prevValues = std::move( currentValues );
        features.push_back( {} );
      }
    }

    if ( !context.testFlag( Qgis::RenderContextFlag::SkipSymbolRendering ) )
    {
      QHash<QgsSymbol *, QList<QgsFeature> > &featuresBack = features.back();
      auto featuresBackIt = featuresBack.find( sym );
      if ( featuresBackIt == featuresBack.end() )
      {
        featuresBackIt = featuresBack.insert( sym, QList<QgsFeature>() );
      }
      featuresBackIt->append( fet );
    }

    // new labeling engine
    if ( isMainRenderer && context.labelingEngine() && ( mLabelProvider || mDiagramProvider ) )
    {
      const quint64 startLabelTime = timer.elapsed();

      QgsGeometry obstacleGeometry;
      QgsSymbolList symbols = renderer->originalSymbolsForFeature( fet, context );
      QgsSymbol *symbol = nullptr;
      if ( !symbols.isEmpty() && fet.geometry().type() == Qgis::GeometryType::Point )
      {
        obstacleGeometry = QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, context, symbols );
      }

      if ( !symbols.isEmpty() )
      {
        symbol = symbols.at( 0 );
        QgsExpressionContextUtils::updateSymbolScope( symbol, symbolScope );
      }

      if ( mLabelProvider )
      {
        mLabelProvider->registerFeature( fet, context, obstacleGeometry, symbol );
      }
      if ( mDiagramProvider )
      {
        mDiagramProvider->registerFeature( fet, context, obstacleGeometry );
      }

      totalLabelTime += ( timer.elapsed() - startLabelTime );
    }
  }

  fetchFeaturesProfile.reset();
  if ( mEnableProfile )
  {
    if ( totalLabelTime > 0 )
    {
      QgsApplication::profiler()->record( QObject::tr( "Registering labels" ), totalLabelTime / 1000.0, QStringLiteral( "rendering" ) );
    }
  }

  if ( mApplyLabelClipGeometries )
    context.setFeatureClipGeometry( QgsGeometry() );

  scopePopper.reset();

  if ( features.back().empty() )
  {
    // nothing to draw
    stopRenderer( renderer, selRenderer );
    return;
  }


  std::unique_ptr< QgsScopedRuntimeProfile > sortingProfile;
  if ( mEnableProfile )
  {
    sortingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Sorting features" ), QStringLiteral( "rendering" ) );
  }
  // find out the order
  QgsSymbolLevelOrder levels;
  QgsSymbolList symbols = renderer->symbols( context );
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbol *sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolLevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolLevel() );
      levels[level].append( item );
    }
  }
  sortingProfile.reset();

  if ( mApplyClipGeometries )
    context.setFeatureClipGeometry( mClipFeatureGeom );

  // 2. draw features in correct order
  for ( const QHash< QgsSymbol *, QList<QgsFeature> > &featureLists : features )
  {
    for ( int l = 0; l < levels.count(); l++ )
    {
      const QgsSymbolLevel &level = levels[l];
      std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
      if ( mEnableProfile )
      {
        renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering symbol level %1" ).arg( l + 1 ), QStringLiteral( "rendering" ) );
      }

      for ( int i = 0; i < level.count(); i++ )
      {
        const QgsSymbolLevelItem &item = level[i];
        if ( !featureLists.contains( item.symbol() ) )
        {
          QgsDebugError( QStringLiteral( "level item's symbol not found!" ) );
          continue;
        }
        const int layer = item.layer();
        const QList<QgsFeature> &lst = featureLists[item.symbol()];
        for ( const QgsFeature &feature : lst )
        {
          if ( context.renderingStopped() )
          {
            stopRenderer( renderer, selRenderer );
            return;
          }

          const bool featureIsSelected = isMainRenderer && context.showSelection() && mSelectedFeatureIds.contains( feature.id() );
          if ( featureIsSelected && mSelectionSymbol )
            continue; // defer rendering of selected symbols

          // maybe vertex markers should be drawn only during the last pass...
          const bool drawMarker = isMainRenderer && ( mDrawVertexMarkers && context.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || featureIsSelected ) );

          if ( ! mNoSetLayerExpressionContext )
            context.expressionContext().setFeature( feature );

          try
          {
            renderer->renderFeature( feature, context, layer, featureIsSelected, drawMarker );

            // as soon as first feature is rendered, we can start showing layer updates.
            // but if we are blocking render updates (so that a previously cached image is being shown), we wait
            // at most e.g. 3 seconds before we start forcing progressive updates.
            if ( !mBlockRenderUpdates || mElapsedTimer.elapsed() > MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
            {
              mReadyToCompose = true;
            }
          }
          catch ( const QgsCsException &cse )
          {
            Q_UNUSED( cse )
            QgsDebugError( QStringLiteral( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                           .arg( fet.id() ).arg( cse.what() ) );
          }
        }
      }
    }
  }

  if ( mSelectionSymbol && !mSelectedFeatureIds.empty() && isMainRenderer && context.showSelection() )
  {
    mSelectionSymbol->startRender( context, mFields );

    for ( const QHash< QgsSymbol *, QList<QgsFeature> > &featureLists : features )
    {
      for ( auto it = featureLists.constBegin(); it != featureLists.constEnd(); ++it )
      {
        const QList<QgsFeature> &lst = it.value();
        for ( const QgsFeature &feature : lst )
        {
          if ( context.renderingStopped() )
          {
            break;
          }

          const bool featureIsSelected = mSelectedFeatureIds.contains( feature.id() );
          if ( !featureIsSelected )
            continue;

          const bool drawMarker = mDrawVertexMarkers && context.drawEditingInformation();
          // note: here we pass "false" for the selected argument, as we don't want to change
          // the user's defined selection symbol colors or settings in any way
          mSelectionSymbol->renderFeature( feature, context, -1, false, drawMarker );
        }
      }
    }

    mSelectionSymbol->stopRender( context );
  }

  std::unique_ptr< QgsScopedRuntimeProfile > cleanupProfile;
  if ( mEnableProfile )
  {
    cleanupProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Finalizing" ), QStringLiteral( "rendering" ) );
  }

  stopRenderer( renderer, selRenderer );
}

void QgsVectorLayerRenderer::stopRenderer( QgsFeatureRenderer *renderer, QgsSingleSymbolRenderer *selRenderer )
{
  QgsRenderContext &context = *renderContext();
  renderer->stopRender( context );
  if ( selRenderer )
  {
    selRenderer->stopRender( context );
    delete selRenderer;
  }
}

void QgsVectorLayerRenderer::prepareLabeling( QgsVectorLayer *layer, QSet<QString> &attributeNames )
{
  QgsRenderContext &context = *renderContext();
  // TODO: add attributes for geometry generator
  if ( QgsLabelingEngine *engine2 = context.labelingEngine() )
  {
    if ( layer->labelsEnabled() )
    {
      if ( context.labelSink() )
      {
        if ( const QgsRuleBasedLabeling *rbl = dynamic_cast<const QgsRuleBasedLabeling *>( layer->labeling() ) )
        {
          mLabelProvider = new QgsRuleBasedLabelSinkProvider( *rbl, layer, context.labelSink() );
        }
        else
        {
          QgsPalLayerSettings settings = layer->labeling()->settings();
          mLabelProvider = new QgsLabelSinkProvider( layer, QString(), context.labelSink(), &settings );
        }
      }
      else
      {
        mLabelProvider = layer->labeling()->provider( layer );
      }
      if ( mLabelProvider )
      {
        engine2->addProvider( mLabelProvider );
        if ( !mLabelProvider->prepare( context, attributeNames ) )
        {
          engine2->removeProvider( mLabelProvider );
          mLabelProvider = nullptr; // deleted by engine
        }
      }
    }
  }

#if 0 // TODO: limit of labels, font not found
  QgsPalLayerSettings &palyr = mContext.labelingEngine()->layer( mLayerID );

  // see if feature count limit is set for labeling
  if ( palyr.limitNumLabels && palyr.maxNumLabels > 0 )
  {
    QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                          .setFilterRect( mContext.extent() )
                                          .setNoAttributes() );

    // total number of features that may be labeled
    QgsFeature f;
    int nFeatsToLabel = 0;
    while ( fit.nextFeature( f ) )
    {
      nFeatsToLabel++;
    }
    palyr.mFeaturesToLabel = nFeatsToLabel;
  }

  // notify user about any font substitution
  if ( !palyr.mTextFontFound && !mLabelFontNotFoundNotified )
  {
    emit labelingFontNotFound( this, palyr.mTextFontFamily );
    mLabelFontNotFoundNotified = true;
  }
#endif
}

void QgsVectorLayerRenderer::prepareDiagrams( QgsVectorLayer *layer, QSet<QString> &attributeNames )
{
  QgsRenderContext &context = *renderContext();
  if ( QgsLabelingEngine *engine2 = context.labelingEngine() )
  {
    if ( layer->diagramsEnabled() )
    {
      mDiagramProvider = new QgsVectorLayerDiagramProvider( layer );
      // need to be added before calling prepare() - uses map settings from engine
      engine2->addProvider( mDiagramProvider );
      if ( !mDiagramProvider->prepare( context, attributeNames ) )
      {
        engine2->removeProvider( mDiagramProvider );
        mDiagramProvider = nullptr;  // deleted by engine
      }
    }
  }
}

