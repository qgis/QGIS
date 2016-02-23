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

//#include "qgsfeatureiterator.h"
#include "diagram/qgsdiagram.h"
#include "qgsdiagramrendererv2.h"
#include "qgsgeometrycache.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsrendererv2.h"
#include "qgsrendercontext.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgssymbollayerv2.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerdiagramprovider.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgspainteffect.h"
#include "qgsfeaturefilterprovider.h"

#include <QSettings>
#include <QPicture>

// TODO:
// - passing of cache to QgsVectorLayer


QgsVectorLayerRenderer::QgsVectorLayerRenderer( QgsVectorLayer* layer, QgsRenderContext& context )
    : QgsMapLayerRenderer( layer->id() )
    , mContext( context )
    , mInterruptionChecker( context )
    , mLayer( layer )
    , mFields( layer->fields() )
    , mRendererV2( nullptr )
    , mCache( nullptr )
    , mLabeling( false )
    , mDiagrams( false )
    , mLabelProvider( nullptr )
    , mDiagramProvider( nullptr )
    , mLayerTransparency( 0 )
{
  mSource = new QgsVectorLayerFeatureSource( layer );

  mRendererV2 = layer->rendererV2() ? layer->rendererV2()->clone() : nullptr;
  mSelectedFeatureIds = layer->selectedFeaturesIds();

  mDrawVertexMarkers = nullptr != layer->editBuffer();

  mGeometryType = layer->geometryType();

  mLayerTransparency = layer->layerTransparency();
  mFeatureBlendMode = layer->featureBlendMode();

  mSimplifyMethod = layer->simplifyMethod();
  mSimplifyGeometry = layer->simplifyDrawingCanbeApplied( mContext, QgsVectorSimplifyMethod::GeometrySimplification );

  QSettings settings;
  mVertexMarkerOnlyForSelection = settings.value( "/qgis/digitizing/marker_only_for_selected", false ).toBool();

  QString markerTypeString = settings.value( "/qgis/digitizing/marker_style", "Cross" ).toString();
  if ( markerTypeString == "Cross" )
  {
    mVertexMarkerStyle = QgsVectorLayer::Cross;
  }
  else if ( markerTypeString == "SemiTransparentCircle" )
  {
    mVertexMarkerStyle = QgsVectorLayer::SemiTransparentCircle;
  }
  else
  {
    mVertexMarkerStyle = QgsVectorLayer::NoMarker;
  }

  mVertexMarkerSize = settings.value( "/qgis/digitizing/marker_size", 3 ).toInt();

  if ( !mRendererV2 )
    return;

  QgsDebugMsg( "rendering v2:\n  " + mRendererV2->dump() );

  if ( mDrawVertexMarkers )
  {
    // set editing vertex markers style
    mRendererV2->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
  }

  mContext.expressionContext() << QgsExpressionContextUtils::layerScope( layer );

  mAttrNames = mRendererV2->usedAttributes();

  //register label and diagram layer to the labeling engine
  prepareLabeling( layer, mAttrNames );
  prepareDiagrams( layer, mAttrNames );

}


QgsVectorLayerRenderer::~QgsVectorLayerRenderer()
{
  delete mRendererV2;
  delete mSource;
}


bool QgsVectorLayerRenderer::render()
{
  if ( mGeometryType == QGis::NoGeometry || mGeometryType == QGis::UnknownGeometry )
    return true;

  if ( !mRendererV2 )
  {
    mErrors.append( QObject::tr( "No renderer for drawing." ) );
    return false;
  }

  bool usingEffect = false;
  if ( mRendererV2->paintEffect() && mRendererV2->paintEffect()->enabled() )
  {
    usingEffect = true;
    mRendererV2->paintEffect()->begin( mContext );
  }

  // Per feature blending mode
  if ( mContext.useAdvancedEffects() && mFeatureBlendMode != QPainter::CompositionMode_SourceOver )
  {
    // set the painter to the feature blend mode, so that features drawn
    // on this layer will interact and blend with each other
    mContext.painter()->setCompositionMode( mFeatureBlendMode );
  }

  mRendererV2->startRender( mContext, mFields );

  QString rendererFilter = mRendererV2->filter( mFields );

  QgsRectangle requestExtent = mContext.extent();
  mRendererV2->modifyRequestExtent( requestExtent, mContext );

  QgsFeatureRequest featureRequest = QgsFeatureRequest()
                                     .setFilterRect( requestExtent )
                                     .setSubsetOfAttributes( mAttrNames, mFields )
                                     .setExpressionContext( mContext.expressionContext() );
  if ( mRendererV2->orderByEnabled() )
  {
    featureRequest.setOrderBy( mRendererV2->orderBy() );
  }

  const QgsFeatureFilterProvider* featureFilterProvider = mContext.featureFilterProvider();
  if ( featureFilterProvider )
  {
    featureFilterProvider->filterFeatures( mLayer, featureRequest );
  }
  if ( !rendererFilter.isEmpty() && rendererFilter != "TRUE" )
  {
    featureRequest.combineFilterExpression( rendererFilter );
  }

  // enable the simplification of the geometries (Using the current map2pixel context) before send it to renderer engine.
  if ( mSimplifyGeometry )
  {
    double map2pixelTol = mSimplifyMethod.threshold();
    bool validTransform = true;

    const QgsMapToPixel& mtp = mContext.mapToPixel();
    map2pixelTol *= mtp.mapUnitsPerPixel();
    const QgsCoordinateTransform* ct = mContext.coordinateTransform();

    // resize the tolerance using the change of size of an 1-BBOX from the source CoordinateSystem to the target CoordinateSystem
    if ( ct && !( ct->isShortCircuited() ) )
    {
      try
      {
        QgsPoint center = mContext.extent().center();
        double rectSize = ct->sourceCrs().geographicFlag() ? 0.0008983 /* ~100/(40075014/360=111319.4833) */ : 100;

        QgsRectangle sourceRect = QgsRectangle( center.x(), center.y(), center.x() + rectSize, center.y() + rectSize );
        QgsRectangle targetRect = ct->transform( sourceRect );

        QgsDebugMsg( QString( "Simplify - SourceTransformRect=%1" ).arg( sourceRect.toString( 16 ) ) );
        QgsDebugMsg( QString( "Simplify - TargetTransformRect=%1" ).arg( targetRect.toString( 16 ) ) );

        if ( !sourceRect.isEmpty() && sourceRect.isFinite() && !targetRect.isEmpty() && targetRect.isFinite() )
        {
          QgsPoint minimumSrcPoint( sourceRect.xMinimum(), sourceRect.yMinimum() );
          QgsPoint maximumSrcPoint( sourceRect.xMaximum(), sourceRect.yMaximum() );
          QgsPoint minimumDstPoint( targetRect.xMinimum(), targetRect.yMinimum() );
          QgsPoint maximumDstPoint( targetRect.xMaximum(), targetRect.yMaximum() );

          double sourceHypothenuse = sqrt( minimumSrcPoint.sqrDist( maximumSrcPoint ) );
          double targetHypothenuse = sqrt( minimumDstPoint.sqrDist( maximumDstPoint ) );

          QgsDebugMsg( QString( "Simplify - SourceHypothenuse=%1" ).arg( sourceHypothenuse ) );
          QgsDebugMsg( QString( "Simplify - TargetHypothenuse=%1" ).arg( targetHypothenuse ) );

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
      mContext.setVectorSimplifyMethod( vectorMethod );
    }
    else
    {
      QgsVectorSimplifyMethod vectorMethod;
      vectorMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
      mContext.setVectorSimplifyMethod( vectorMethod );
    }
  }
  else
  {
    QgsVectorSimplifyMethod vectorMethod;
    vectorMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
    mContext.setVectorSimplifyMethod( vectorMethod );
  }

  QgsFeatureIterator fit = mSource->getFeatures( featureRequest );
  // Attach an interruption checker so that iterators that have potentially
  // slow fetchFeature() implementations, such as in the WFS provider, can
  // check it, instead of relying on just the mContext.renderingStopped() check
  // in drawRendererV2()
  fit.setInterruptionChecker( &mInterruptionChecker );

  if (( mRendererV2->capabilities() & QgsFeatureRendererV2::SymbolLevels ) && mRendererV2->usingSymbolLevels() )
    drawRendererV2Levels( fit );
  else
    drawRendererV2( fit );

  if ( usingEffect )
  {
    mRendererV2->paintEffect()->end( mContext );
  }

  //apply layer transparency for vector layers
  if ( mContext.useAdvancedEffects() && mLayerTransparency != 0 )
  {
    // a layer transparency has been set, so update the alpha for the flattened layer
    // by combining it with the layer transparency
    QColor transparentFillColor = QColor( 0, 0, 0, 255 - ( 255 * mLayerTransparency / 100 ) );
    // use destination in composition mode to merge source's alpha with destination
    mContext.painter()->setCompositionMode( QPainter::CompositionMode_DestinationIn );
    mContext.painter()->fillRect( 0, 0, mContext.painter()->device()->width(),
                                  mContext.painter()->device()->height(), transparentFillColor );
  }

  return true;
}

void QgsVectorLayerRenderer::setGeometryCachePointer( QgsGeometryCache* cache )
{
  mCache = cache;

  if ( mCache )
  {
    // Destroy all cached geometries and clear the references to them
    mCache->setCachedGeometriesRect( mContext.extent() );
  }
}



void QgsVectorLayerRenderer::drawRendererV2( QgsFeatureIterator& fit )
{
  QgsExpressionContextScope* symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  mContext.expressionContext().appendScope( symbolScope );

  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    try
    {
      if ( mContext.renderingStopped() )
      {
        QgsDebugMsg( QString( "Drawing of vector layer %1 cancelled." ).arg( layerID() ) );
        break;
      }

      if ( !fet.constGeometry() )
        continue; // skip features without geometry

      mContext.expressionContext().setFeature( fet );

      bool sel = mContext.showSelection() && mSelectedFeatureIds.contains( fet.id() );
      bool drawMarker = ( mDrawVertexMarkers && mContext.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

      if ( mCache )
      {
        // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
        mCache->cacheGeometry( fet.id(), *fet.constGeometry() );
      }

      // render feature
      bool rendered = mRendererV2->renderFeature( fet, mContext, -1, sel, drawMarker );

      // labeling - register feature
      if ( rendered )
      {
        if ( mContext.labelingEngine() )
        {
          if ( mLabeling )
          {
            mContext.labelingEngine()->registerFeature( mLayerID, fet, mContext );
          }
          if ( mDiagrams )
          {
            mContext.labelingEngine()->registerDiagramFeature( mLayerID, fet, mContext );
          }
        }
        // new labeling engine
        if ( mContext.labelingEngineV2() )
        {
          QScopedPointer<QgsGeometry> obstacleGeometry;
          QgsSymbolV2List symbols = mRendererV2->originalSymbolsForFeature( fet, mContext );

          if ( !symbols.isEmpty() && fet.constGeometry()->type() == QGis::Point )
          {
            obstacleGeometry.reset( QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, mContext, symbols ) );
          }

          if ( !symbols.isEmpty() )
          {
            QgsExpressionContextUtils::updateSymbolScope( symbols.at( 0 ), symbolScope );
          }

          if ( mLabelProvider )
          {
            mLabelProvider->registerFeature( fet, mContext, obstacleGeometry.data() );
          }
          if ( mDiagramProvider )
          {
            mDiagramProvider->registerFeature( fet, mContext, obstacleGeometry.data() );
          }
        }
      }
    }
    catch ( const QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsg( QString( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                   .arg( fet.id() ).arg( cse.what() ) );
    }
  }

  delete mContext.expressionContext().popScope();

  stopRendererV2( nullptr );
}

void QgsVectorLayerRenderer::drawRendererV2Levels( QgsFeatureIterator& fit )
{
  QHash< QgsSymbolV2*, QList<QgsFeature> > features; // key = symbol, value = array of features

  QgsSingleSymbolRendererV2* selRenderer = nullptr;
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    selRenderer = new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( mGeometryType ) );
    selRenderer->symbol()->setColor( mContext.selectionColor() );
    selRenderer->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
    selRenderer->startRender( mContext, mFields );
  }

  QgsExpressionContextScope* symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  mContext.expressionContext().appendScope( symbolScope );

  // 1. fetch features
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    if ( mContext.renderingStopped() )
    {
      qDebug( "rendering stop!" );
      stopRendererV2( selRenderer );
      delete mContext.expressionContext().popScope();
      return;
    }

    if ( !fet.constGeometry() )
      continue; // skip features without geometry

    mContext.expressionContext().setFeature( fet );
    QgsSymbolV2* sym = mRendererV2->symbolForFeature( fet, mContext );
    if ( !sym )
    {
      continue;
    }

    if ( !features.contains( sym ) )
    {
      features.insert( sym, QList<QgsFeature>() );
    }
    features[sym].append( fet );

    if ( mCache )
    {
      // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
      mCache->cacheGeometry( fet.id(), *fet.constGeometry() );
    }

    if ( mContext.labelingEngine() )
    {
      mContext.expressionContext().setFeature( fet );
      if ( mLabeling )
      {
        mContext.labelingEngine()->registerFeature( mLayerID, fet, mContext );
      }
      if ( mDiagrams )
      {
        mContext.labelingEngine()->registerDiagramFeature( mLayerID, fet, mContext );
      }
    }
    // new labeling engine
    if ( mContext.labelingEngineV2() )
    {
      QScopedPointer<QgsGeometry> obstacleGeometry;
      QgsSymbolV2List symbols = mRendererV2->originalSymbolsForFeature( fet, mContext );

      if ( !symbols.isEmpty() && fet.constGeometry()->type() == QGis::Point )
      {
        obstacleGeometry.reset( QgsVectorLayerLabelProvider::getPointObstacleGeometry( fet, mContext, symbols ) );
      }

      if ( !symbols.isEmpty() )
      {
        QgsExpressionContextUtils::updateSymbolScope( symbols.at( 0 ), symbolScope );
      }

      if ( mLabelProvider )
      {
        mLabelProvider->registerFeature( fet, mContext, obstacleGeometry.data() );
      }
      if ( mDiagramProvider )
      {
        mDiagramProvider->registerFeature( fet, mContext, obstacleGeometry.data() );
      }
    }
  }

  delete mContext.expressionContext().popScope();

  // find out the order
  QgsSymbolV2LevelOrder levels;
  QgsSymbolV2List symbols = mRendererV2->symbols( mContext );
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolV2LevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolV2Level() );
      levels[level].append( item );
    }
  }

  // 2. draw features in correct order
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolV2Level& level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = level[i];
      if ( !features.contains( item.symbol() ) )
      {
        QgsDebugMsg( "level item's symbol not found!" );
        continue;
      }
      int layer = item.layer();
      QList<QgsFeature>& lst = features[item.symbol()];
      QList<QgsFeature>::iterator fit;
      for ( fit = lst.begin(); fit != lst.end(); ++fit )
      {
        if ( mContext.renderingStopped() )
        {
          stopRendererV2( selRenderer );
          return;
        }

        bool sel = mSelectedFeatureIds.contains( fit->id() );
        // maybe vertex markers should be drawn only during the last pass...
        bool drawMarker = ( mDrawVertexMarkers && mContext.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

        mContext.expressionContext().setFeature( *fit );

        try
        {
          mRendererV2->renderFeature( *fit, mContext, layer, sel, drawMarker );
        }
        catch ( const QgsCsException &cse )
        {
          Q_UNUSED( cse );
          QgsDebugMsg( QString( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                       .arg( fet.id() ).arg( cse.what() ) );
        }
      }
    }
  }

  stopRendererV2( selRenderer );
}


void QgsVectorLayerRenderer::stopRendererV2( QgsSingleSymbolRendererV2* selRenderer )
{
  mRendererV2->stopRender( mContext );
  if ( selRenderer )
  {
    selRenderer->stopRender( mContext );
    delete selRenderer;
  }
}




void QgsVectorLayerRenderer::prepareLabeling( QgsVectorLayer* layer, QStringList& attributeNames )
{
  if ( !mContext.labelingEngine() )
  {
    if ( QgsLabelingEngineV2* engine2 = mContext.labelingEngineV2() )
    {
      if ( layer->labeling() )
      {
        mLabelProvider = layer->labeling()->provider( layer );
        if ( mLabelProvider )
        {
          engine2->addProvider( mLabelProvider );
          if ( !mLabelProvider->prepare( mContext, attributeNames ) )
          {
            engine2->removeProvider( mLabelProvider );
            mLabelProvider = nullptr; // deleted by engine
          }
        }
      }
    }
    return;
  }

  if ( mContext.labelingEngine()->prepareLayer( layer, attributeNames, mContext ) )
  {
    mLabeling = true;

#if 0 // TODO: limit of labels, font not found
    QgsPalLayerSettings& palyr = mContext.labelingEngine()->layer( mLayerID );

    // see if feature count limit is set for labeling
    if ( palyr.limitNumLabels && palyr.maxNumLabels > 0 )
    {
      QgsFeatureIterator fit = getFeatures( QgsFeatureRequest()
                                            .setFilterRect( mContext.extent() )
                                            .setSubsetOfAttributes( QgsAttributeList() ) );

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
}

void QgsVectorLayerRenderer::prepareDiagrams( QgsVectorLayer* layer, QStringList& attributeNames )
{
  if ( !mContext.labelingEngine() )
  {
    if ( QgsLabelingEngineV2* engine2 = mContext.labelingEngineV2() )
    {
      if ( layer->diagramsEnabled() )
      {
        mDiagramProvider = new QgsVectorLayerDiagramProvider( layer );
        // need to be added before calling prepare() - uses map settings from engine
        engine2->addProvider( mDiagramProvider );
        if ( !mDiagramProvider->prepare( mContext, attributeNames ) )
        {
          engine2->removeProvider( mDiagramProvider );
          mDiagramProvider = nullptr;  // deleted by engine
        }
      }
    }
    return;
  }

  if ( !layer->diagramsEnabled() )
    return;

  mDiagrams = true;

  mContext.labelingEngine()->prepareDiagramLayer( layer, attributeNames, mContext ); // will make internal copy of diagSettings + initialize it

}

/*  -----------------------------------------  */
/*  QgsVectorLayerRendererInterruptionChecker  */
/*  -----------------------------------------  */

QgsVectorLayerRendererInterruptionChecker::QgsVectorLayerRendererInterruptionChecker
( const QgsRenderContext& context )
    : mContext( context )
{
}

bool QgsVectorLayerRendererInterruptionChecker::mustStop() const
{
  return mContext.renderingStopped();
}
