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

#include "diagram/qgsdiagram.h"

#include "qgsdiagramrenderer.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsrenderer.h"
#include "qgsrendercontext.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
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
#include "qgsvectorlayertemporalproperties.h"
#include "qgsmapclippingutils.h"
#include "qgsfeaturerenderergenerator.h"

#include <QPicture>
#include <QTimer>

QgsVectorLayerRenderer::QgsVectorLayerRenderer( QgsVectorLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mLayer( layer )
  , mFields( layer->fields() )
  , mSource( std::make_unique< QgsVectorLayerFeatureSource >( layer ) )
{
  std::unique_ptr< QgsFeatureRenderer > mainRenderer( layer->renderer() ? layer->renderer()->clone() : nullptr );

  if ( !mainRenderer )
    return;

  QList< const QgsFeatureRendererGenerator * > generators = layer->featureRendererGenerators();
  std::sort( generators.begin(), generators.end(), []( const QgsFeatureRendererGenerator * g1, const QgsFeatureRendererGenerator * g2 )
  {
    return g1->level() < g2->level();
  } );

  bool insertedMainRenderer = false;
  double prevLevel = std::numeric_limits< double >::lowest();
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
  if ( renderContext()->vectorSimplifyMethod().simplifyHints() != QgsVectorSimplifyMethod::NoSimplification )
  {
    mSimplifyMethod = renderContext()->vectorSimplifyMethod();
    mSimplifyGeometry = renderContext()->vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::GeometrySimplification ||
                        renderContext()->vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::FullSimplification;
  }
  else
  {
    mSimplifyMethod = layer->simplifyMethod();
    mSimplifyGeometry = layer->simplifyDrawingCanbeApplied( *renderContext(), QgsVectorSimplifyMethod::GeometrySimplification );
  }

  mVertexMarkerOnlyForSelection = QgsSettingsRegistryCore::settingsDigitizingMarkerOnlyForSelected.value();

  QString markerTypeString = QgsSettingsRegistryCore::settingsDigitizingMarkerStyle.value();
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

  mVertexMarkerSize = QgsSettingsRegistryCore::settingsDigitizingMarkerSizeMm.value();

  QgsDebugMsgLevel( "rendering v2:\n  " + mRenderer->dump(), 2 );

  if ( mDrawVertexMarkers )
  {
    // set editing vertex markers style (main renderer only)
    mRenderer->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
  }
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
  if ( mGeometryType == QgsWkbTypes::NullGeometry || mGeometryType == QgsWkbTypes::UnknownGeometry )
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

  // if the previous layer render was relatively quick (e.g. less than 3 seconds), the we show any previously
  // cached version of the layer during rendering instead of the usual progressive updates
  if ( mRenderTimeHint > 0 && mRenderTimeHint <= MAX_TIME_TO_USE_CACHED_PREVIEW_IMAGE )
  {
    mBlockRenderUpdates = true;
    mElapsedTimer.start();
  }

  bool res = true;
  for ( const std::unique_ptr< QgsFeatureRenderer > &renderer : mRenderers )
  {
    res = renderInternal( renderer.get() ) && res;
  }

  mReadyToCompose = true;
  return res && !renderContext()->renderingStopped();
}

bool QgsVectorLayerRenderer::renderInternal( QgsFeatureRenderer *renderer )
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

  QString rendererFilter = renderer->filter( mFields );

  QgsRectangle requestExtent = context.extent();
  if ( !mClippingRegions.empty() )
  {
    mClipFilterGeom = QgsMapClippingUtils::calculateFeatureRequestGeometry( mClippingRegions, context, mApplyClipFilter );
    requestExtent = requestExtent.intersect( mClipFilterGeom.boundingBox() );

    mClipFeatureGeom = QgsMapClippingUtils::calculateFeatureIntersectionGeometry( mClippingRegions, context, mApplyClipGeometries );

    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, context, QgsMapLayerType::VectorLayer, needsPainterClipPath );
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
    featureRequest.setFlags( featureRequest.flags() | QgsFeatureRequest::EmbeddedSymbols );
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
        double rectSize = toleranceTransform.sourceCrs().mapUnits() == QgsUnitTypes::DistanceDegrees ? 0.0008983 /* ~100/(40075014/360=111319.4833) */ : 100;

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
      vectorMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
      context.setVectorSimplifyMethod( vectorMethod );
    }
  }
  else
  {
    QgsVectorSimplifyMethod vectorMethod;
    vectorMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
    context.setVectorSimplifyMethod( vectorMethod );
  }

  featureRequest.setFeedback( mFeedback.get() );
  // also set the interruption checker for the expression context, in case the renderer uses some complex expression
  // which could benefit from early exit paths...
  context.expressionContext().setFeedback( mFeedback.get() );

  QgsFeatureIterator fit = mSource->getFeatures( featureRequest );
  // Attach an interruption checker so that iterators that have potentially
  // slow fetchFeature() implementations, such as in the WFS provider, can
  // check it, instead of relying on just the mContext.renderingStopped() check
  // in drawRenderer()
  fit.setInterruptionChecker( mFeedback.get() );

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

      context.expressionContext().setFeature( fet );

      bool sel = isMainRenderer && context.showSelection() && mSelectedFeatureIds.contains( fet.id() );
      bool drawMarker = isMainRenderer && ( mDrawVertexMarkers && context.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

      // render feature
      bool rendered = false;
      if ( !context.testFlag( Qgis::RenderContextFlag::SkipSymbolRendering ) )
      {
        rendered = renderer->renderFeature( fet, context, -1, sel, drawMarker );
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
          QgsGeometry obstacleGeometry;
          QgsSymbolList symbols = renderer->originalSymbolsForFeature( fet, context );
          QgsSymbol *symbol = nullptr;
          if ( !symbols.isEmpty() && fet.geometry().type() == QgsWkbTypes::PointGeometry )
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
        }
      }
    }
    catch ( const QgsCsException &cse )
    {
      Q_UNUSED( cse )
      QgsDebugMsg( QStringLiteral( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                   .arg( fet.id() ).arg( cse.what() ) );
    }
  }

  delete context.expressionContext().popScope();

  stopRenderer( renderer, nullptr );
}

void QgsVectorLayerRenderer::drawRendererLevels( QgsFeatureRenderer *renderer, QgsFeatureIterator &fit )
{
  const bool isMainRenderer = renderer == mRenderer;

  QHash< QgsSymbol *, QList<QgsFeature> > features; // key = symbol, value = array of features
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

  // 1. fetch features
  QgsFeature fet;
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

    context.expressionContext().setFeature( fet );
    QgsSymbol *sym = renderer->symbolForFeature( fet, context );
    if ( !sym )
    {
      continue;
    }

    if ( !context.testFlag( Qgis::RenderContextFlag::SkipSymbolRendering ) )
    {
      if ( !features.contains( sym ) )
      {
        features.insert( sym, QList<QgsFeature>() );
      }
      features[sym].append( fet );
    }

    // new labeling engine
    if ( isMainRenderer && context.labelingEngine() && ( mLabelProvider || mDiagramProvider ) )
    {
      QgsGeometry obstacleGeometry;
      QgsSymbolList symbols = renderer->originalSymbolsForFeature( fet, context );
      QgsSymbol *symbol = nullptr;
      if ( !symbols.isEmpty() && fet.geometry().type() == QgsWkbTypes::PointGeometry )
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
    }
  }

  if ( mApplyLabelClipGeometries )
    context.setFeatureClipGeometry( QgsGeometry() );

  scopePopper.reset();

  if ( features.empty() )
  {
    // nothing to draw
    stopRenderer( renderer, selRenderer );
    return;
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

  if ( mApplyClipGeometries )
    context.setFeatureClipGeometry( mClipFeatureGeom );

  // 2. draw features in correct order
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolLevel &level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolLevelItem &item = level[i];
      if ( !features.contains( item.symbol() ) )
      {
        QgsDebugMsg( QStringLiteral( "level item's symbol not found!" ) );
        continue;
      }
      int layer = item.layer();
      QList<QgsFeature> &lst = features[item.symbol()];
      QList<QgsFeature>::iterator fit;
      for ( fit = lst.begin(); fit != lst.end(); ++fit )
      {
        if ( context.renderingStopped() )
        {
          stopRenderer( renderer, selRenderer );
          return;
        }

        bool sel = isMainRenderer && context.showSelection() && mSelectedFeatureIds.contains( fit->id() );
        // maybe vertex markers should be drawn only during the last pass...
        bool drawMarker = isMainRenderer && ( mDrawVertexMarkers && context.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

        context.expressionContext().setFeature( *fit );

        try
        {
          renderer->renderFeature( *fit, context, layer, sel, drawMarker );

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
          QgsDebugMsg( QStringLiteral( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                       .arg( fet.id() ).arg( cse.what() ) );
        }
      }
    }
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

