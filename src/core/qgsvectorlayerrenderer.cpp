
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
#include "qgsvectorlayerfeatureiterator.h"

#include <QSettings>

// TODO:
// - passing of cache to QgsVectorLayer


QgsVectorLayerRenderer::QgsVectorLayerRenderer( QgsVectorLayer* layer, QgsRenderContext& context )
    : QgsMapLayerRenderer( layer->id() )
    , mContext( context )
    , mFields( layer->pendingFields() )
    , mRendererV2( 0 )
    , mCache( 0 )
    , mLabeling( false )
    , mDiagrams( false )
    , mLayerTransparency( 0 )
{
  mSource = new QgsVectorLayerFeatureSource( layer );

  mRendererV2 = layer->rendererV2() ? layer->rendererV2()->clone() : 0;
  mSelectedFeatureIds = layer->selectedFeaturesIds();

  mDrawVertexMarkers = ( layer->editBuffer() != 0 );

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

  QgsDebugMsg( "rendering v2:\n" + mRendererV2->dump() );

  if ( mDrawVertexMarkers )
  {
    // set editing vertex markers style
    mRendererV2->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
  }

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
    mErrors.append( "No renderer for drawing." );
    return false;
  }

  // Per feature blending mode
  if ( mContext.useAdvancedEffects() && mFeatureBlendMode != QPainter::CompositionMode_SourceOver )
  {
    // set the painter to the feature blend mode, so that features drawn
    // on this layer will interact and blend with each other
    mContext.painter()->setCompositionMode( mFeatureBlendMode );
  }

  mRendererV2->startRender( mContext, mFields );

  QgsFeatureRequest featureRequest = QgsFeatureRequest()
                                     .setFilterRect( mContext.extent() )
                                     .setSubsetOfAttributes( mAttrNames, mFields );

  // enable the simplification of the geometries (Using the current map2pixel context) before send it to renderer engine.
  if ( mSimplifyGeometry )
  {
    QPainter* p = mContext.painter();
    double dpi = ( p->device()->logicalDpiX() + p->device()->logicalDpiY() ) / 2;
    double map2pixelTol = mSimplifyMethod.threshold() * 96.0f / dpi;

    const QgsMapToPixel& mtp = mContext.mapToPixel();
    map2pixelTol *= mtp.mapUnitsPerPixel();
    const QgsCoordinateTransform* ct = mContext.coordinateTransform();

    // resize the tolerance using the change of size of an 1-BBOX from the source CoordinateSystem to the target CoordinateSystem
    if ( ct && !(( QgsCoordinateTransform* )ct )->isShortCircuited() )
    {
      try
      {
        QgsPoint center = mContext.extent().center();
        double rectSize = ct->sourceCrs().geographicFlag() ?  0.0008983 /* ~100/(40075014/360=111319.4833) */ : 100;

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

          if ( targetHypothenuse != 0 )
            map2pixelTol *= ( sourceHypothenuse / targetHypothenuse );
        }
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( QObject::tr( "Simplify transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
      }
    }

    QgsSimplifyMethod simplifyMethod;
    simplifyMethod.setMethodType( QgsSimplifyMethod::OptimizeForRendering );
    simplifyMethod.setTolerance( map2pixelTol );
    simplifyMethod.setForceLocalOptimization( mSimplifyMethod.forceLocalOptimization() );

    featureRequest.setSimplifyMethod( simplifyMethod );
  }

  QgsFeatureIterator fit = mSource->getFeatures( featureRequest );

  if (( mRendererV2->capabilities() & QgsFeatureRendererV2::SymbolLevels ) && mRendererV2->usingSymbolLevels() ) {
    // find out the order
    PassSymbolMap levels;
    QgsSymbolV2List symbols = mRendererV2->symbols();
    for ( int i = 0; i < symbols.count(); i++ )
    {
      QgsSymbolV2* sym = symbols[i];
      for ( int j = 0; j < sym->symbolLayerCount(); j++ )
      {
        int level = sym->symbolLayer( j )->renderingPass();
        if ( level < 0 || level >= 1000 ) // ignore invalid levels
          continue;

        if ( ! levels.contains( level ) ) {
          levels[level] = PassSymbols( /* renderAsLayer*/ true );
        }
        levels[level].levels.append( QgsSymbolV2LevelItem(sym,j) );
      }
    }

    drawRendererV2Levels( fit, levels );
  }
  else {
    // no symbol levels defined
    // do we have a layer that forces it ?
    bool forceAsLayer = false;
    QgsSymbolV2List symbols = mRendererV2->symbols();
    PassSymbolMap levels;
    for ( int i = 0; i < symbols.count(); i++ )
    {
      QgsSymbolV2* sym = symbols[i];
      for ( int j = 0; j < sym->symbolLayerCount(); j++ )
      {
        if ( ! levels.contains( j ) ) {
          levels[j] = PassSymbols();
        }
        if ( sym->symbolLayer( j )->forceRenderAsLayer() ) {
          // mimick symbol levels
          levels[j].renderAsLayer = true;
          forceAsLayer = true;
        }
        levels[j].levels.append( QgsSymbolV2LevelItem(sym,j) );
      }
    }

    if ( forceAsLayer ) {
      drawRendererV2Levels( fit, levels );
    }
    else {
      // no symbol levels (user-defined or forced)
      drawRendererV2( fit );
    }
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

/**
 * Render given symbol layers of a feature
 * If layers is empty, -1 is passed to the renderer (all layers will be rendered)
 */
void QgsVectorLayerRenderer::renderFeature( const QgsFeature& feature, const QList<int>& layers )
{
  bool sel = mSelectedFeatureIds.contains( feature.id() );
  bool drawMarker = ( mDrawVertexMarkers && mContext.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

  // render feature
  QgsFeature& fet = const_cast<QgsFeature&>(feature);
  bool rendered;
  try
  {
    if ( ! layers.empty() ) {
      for ( QList<int>::const_iterator lit = layers.begin(); lit != layers.end(); ++lit )
      {
        rendered = mRendererV2->renderFeature( fet, mContext, *lit, sel, drawMarker );
      }
    }
    else {
      rendered = mRendererV2->renderFeature( fet, mContext, -1, sel, drawMarker );
    }
  }
  catch ( const QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( QString( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                 .arg( fet.id() ).arg( cse.what() ) );
  }

  if ( mCache )
  {
    // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
    mCache->cacheGeometry( fet.id(), *fet.geometry() );
  }

  // labeling - register feature
  if ( rendered && mContext.labelingEngine() )
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
}

void QgsVectorLayerRenderer::drawRendererV2( QgsFeatureIterator& fit )
{
  QgsFeature fet;
  QList<int> layers;
  while ( fit.nextFeature( fet ) )
  {
    if ( !fet.geometry() )
      continue; // skip features without geometry

    if ( mContext.renderingStopped() )
    {
      qDebug( "breaking!" );
      break;
    }

    renderFeature( fet, layers );
  }

  stopRendererV2( NULL );
}

void QgsVectorLayerRenderer::drawRendererV2Levels( QgsFeatureIterator& fit, const PassSymbolMap& passes )
{
  // all features of the layer, accessed by their id
  QHash< QgsFeatureId, QgsFeature > allFeatures;
  // for each symbol layer, a hash associating a symbol with a list of features (id)
  typedef QList<QgsFeatureId> FeatureIdList;
  typedef QHash< QgsSymbolV2*, FeatureIdList > SymbolFeatures;
  SymbolFeatures features;

  // start the selection renderer
  QgsSingleSymbolRendererV2* selRenderer = NULL;
  if ( !mSelectedFeatureIds.isEmpty() )
  {
    selRenderer = new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( mGeometryType ) );
    selRenderer->symbol()->setColor( mContext.selectionColor() );
    selRenderer->setVertexMarkerAppearance( mVertexMarkerStyle, mVertexMarkerSize );
    selRenderer->startRender( mContext, mFields );
  }

  // 1. fetch features
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    if ( !fet.geometry() )
      continue; // skip features without geometry

    if ( mContext.renderingStopped() )
    {
      qDebug( "rendering stop!" );
      stopRendererV2( selRenderer );
      return;
    }

    QgsSymbolV2* sym = mRendererV2->symbolForFeature( fet );
    if ( !sym )
    {
      continue;
    }

    allFeatures[ fet.id() ] = fet;
    if ( !features.contains( sym ) )
    {
      features.insert( sym, FeatureIdList() );
    }
    features[sym].append( fet.id() );

    if ( mCache )
    {
      // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
      mCache->cacheGeometry( fet.id(), *fet.geometry() );
    }

    if ( sym && mContext.labelingEngine() )
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
  }

  //
  // 2. draw features in correct order
  //
  // Two types of renderings can be mixed :
  // - the "regular" rendering where each feature is iterated over for each of its symbol layers, called "feature-wise" rendering
  // - the "layer" rendering where, for one given symbol layer, features are iterated over,
  //   this is needed for symbol layers that have pre- and a post-render phases (exterior fills for instance)

  PassSymbolMap::const_iterator pit = passes.begin();
  while ( pit != passes.end() )
  {
    if ( ! pit.value().renderAsLayer )
    {
      // regroup layers and render them feature wise
      QList<int> levels;
      while ( pit != passes.end() && (! pit.value().renderAsLayer) ) {
        levels.append( pit.key() );
        pit++;
      }
      // render feature-wise
      for ( QHash<QgsFeatureId, QgsFeature>::const_iterator fit = allFeatures.begin(); fit != allFeatures.end(); ++fit )
      {
        renderFeature( fit.value(), levels );
      }
    }
    else
    {
      // render as a layer
      QList<int> layers;
      layers.append(0);

      const QgsSymbolV2Level& level = pit.value().levels;
      for ( int i = 0; i < level.count(); i++ )
      {
        QgsSymbolV2LevelItem& item = const_cast<QgsSymbolV2LevelItem&>(level[i]);
        int layer = item.layer();
        QgsSymbolV2* symbol = item.symbol();
        if ( !features.contains( item.symbol() ) )
        {
          symbol->postRender( mContext, layer );
          QgsDebugMsg( "level item's symbol not found!" );
          continue;
        }
        symbol->preRender( mContext, layer );
        FeatureIdList& lst = features[item.symbol()];
        FeatureIdList::iterator fit;
        for ( fit = lst.begin(); fit != lst.end(); ++fit )
        {
          if ( mContext.renderingStopped() )
          {
            stopRendererV2( selRenderer );
            return;
          }

          const QgsFeature& fet = allFeatures[*fit];
          layers[0] = layer;
          renderFeature( fet, layers );
        }
        symbol->postRender( mContext, layer );
      }

      // next pass
      pit++;
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
    return;

  if ( mContext.labelingEngine()->prepareLayer( layer, attributeNames, mContext ) )
  {
    mLabeling = true;

    QgsPalLayerSettings& palyr = mContext.labelingEngine()->layer( mLayerID );
    Q_UNUSED( palyr );

#if 0 // TODO: limit of labels, font not found
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
    return;

  if ( !layer->diagramRenderer() || !layer->diagramLayerSettings() )
    return;

  mDiagrams = true;

  const QgsDiagramRendererV2* diagRenderer = layer->diagramRenderer();
  const QgsDiagramLayerSettings* diagSettings = layer->diagramLayerSettings();

  mContext.labelingEngine()->addDiagramLayer( layer, diagSettings ); // will make internal copy of diagSettings + initialize it

  //add attributes needed by the diagram renderer
  QList<QString> att = diagRenderer->diagramAttributes();
  QList<QString>::const_iterator attIt = att.constBegin();
  for ( ; attIt != att.constEnd(); ++attIt )
  {
    QgsExpression* expression = diagRenderer->diagram()->getExpression( *attIt, &mFields );
    QStringList columns = expression->referencedColumns();
    QStringList::const_iterator columnsIterator = columns.constBegin();
    for ( ; columnsIterator != columns.constEnd(); ++columnsIterator )
    {
      if ( !attributeNames.contains( *columnsIterator ) )
        attributeNames << *columnsIterator;
    }
  }

  const QgsLinearlyInterpolatedDiagramRenderer* linearlyInterpolatedDiagramRenderer = dynamic_cast<const QgsLinearlyInterpolatedDiagramRenderer*>( layer->diagramRenderer() );
  if ( linearlyInterpolatedDiagramRenderer != NULL )
  {
    if ( linearlyInterpolatedDiagramRenderer->classificationAttributeIsExpression() )
    {
      QgsExpression* expression = diagRenderer->diagram()->getExpression( linearlyInterpolatedDiagramRenderer->classificationAttributeExpression(), &mFields );
      QStringList columns = expression->referencedColumns();
      QStringList::const_iterator columnsIterator = columns.constBegin();
      for ( ; columnsIterator != columns.constEnd(); ++columnsIterator )
      {
        if ( !attributeNames.contains( *columnsIterator ) )
          attributeNames << *columnsIterator;
      }
    }
    else
    {
      QString name = mFields.at( linearlyInterpolatedDiagramRenderer->classificationAttribute() ).name();
      if ( !attributeNames.contains( name ) )
        attributeNames << name;
    }
  }

  //and the ones needed for data defined diagram positions
  if ( diagSettings->xPosColumn != -1 )
    attributeNames << mFields.at( diagSettings->xPosColumn ).name();
  if ( diagSettings->yPosColumn != -1 )
    attributeNames << mFields.at( diagSettings->yPosColumn ).name();
}
