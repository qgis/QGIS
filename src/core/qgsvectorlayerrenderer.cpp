
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

  // find out the order
  bool useLevels = false;
  PassSymbolMap levels;
  QgsSymbolV2List symbols = mRendererV2->symbols();
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = j;
      if ( mRendererV2->usingSymbolLevels() ) {
        useLevels = true;
        level = sym->symbolLayer( j )->renderingPass();
        if ( level < 0 || level >= 1000 ) // ignore invalid levels
          continue;
      }
      else if ( sym->symbolLayer( j )->forceRenderWithLevels() ) {
        useLevels = true;
      }

      if ( ! levels.contains( level ) ) {
        levels[level] = QgsSymbolV2Level();
      }
      levels[level].append( QgsSymbolV2LevelItem(sym,j) );
    }
  }

  if ( useLevels ) {
    drawRendererV2Levels( fit, levels );
  }
  else {
    // no symbol levels defined, call the fast drawRendererV2
    drawRendererV2( fit );
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
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    try
    {
      if ( !fet.geometry() )
        continue; // skip features without geometry

      if ( mContext.renderingStopped() )
      {
        qDebug( "breaking!" );
        break;
      }

      bool sel = mSelectedFeatureIds.contains( fet.id() );
      bool drawMarker = ( mDrawVertexMarkers && mContext.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || sel ) );

      // render feature
      bool rendered = mRendererV2->renderFeature( fet, mContext, -1, sel, drawMarker );

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
    catch ( const QgsCsException &cse )
    {
      Q_UNUSED( cse );
      QgsDebugMsg( QString( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                   .arg( fet.id() ).arg( cse.what() ) );
    }
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

    // collect symbols for this feature
    QgsSymbolV2List syms;
    if ( mRendererV2->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature )
    {
      syms = mRendererV2->symbolsForFeature( fet );
    }
    else
    {
      QgsSymbolV2* sym = mRendererV2->symbolForFeature( fet );
      if ( sym ) {
        syms.append( sym );
      }
    }
    if ( syms.empty() )
    {
      continue;
    }

    allFeatures[ fet.id() ] = fet;
    foreach( QgsSymbolV2* sym, syms )
    {
      if ( !features.contains( sym ) )
      {
        features.insert( sym, FeatureIdList() );
      }
      features[sym].append( fet.id() );
    }

    if ( mCache )
    {
      // Cache this for the use of (e.g.) modifying the feature's uncommitted geometry.
      mCache->cacheGeometry( fet.id(), *fet.geometry() );
    }

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
  }

  //
  // 2. draw features in correct order
  //

  // for each distinct rendering pass
  for ( PassSymbolMap::const_iterator pit = passes.begin(); pit != passes.end(); ++pit )
  {
    const QgsSymbolV2Level& level = pit.value();
    // call preRender for each symbol layer of the pass
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = const_cast<QgsSymbolV2LevelItem&>(level[i]);
      item.symbol()->preRender( mContext, item.layer() );
    }

    for ( QHash<QgsFeatureId, QgsFeature>::const_iterator fit = allFeatures.begin(); fit != allFeatures.end(); ++fit )
    {
      const QgsFeature& fet = fit.value();

      // for each symbol layer of the same rendering pass
      for ( int i = 0; i < level.count(); i++ )
      {
        QgsSymbolV2LevelItem& item = const_cast<QgsSymbolV2LevelItem&>(level[i]);
        int layer = item.layer();
        QgsSymbolV2* symbol = item.symbol();

        // only call render on features defined for this symbol
        SymbolFeatures::const_iterator existingSymbol = features.find( symbol );
        if ( existingSymbol == features.end() )
        {
          QgsDebugMsg( "level item's symbol not found!" );
          continue;
        }
        if ( ! existingSymbol.value().contains(fet.id()) )
        {
          QgsDebugMsg("no feature for this symbol");
          continue;
        }

        // render the proper symbol layer
        bool selected = mSelectedFeatureIds.contains( fet.id() );
        bool drawMarker = ( mDrawVertexMarkers && mContext.drawEditingInformation() && ( !mVertexMarkerOnlyForSelection || selected ) );
        try
        {
          mRendererV2->renderFeatureWithSymbol( const_cast<QgsFeature&>(fet), symbol, mContext, layer, selected, drawMarker );
        }
        catch ( const QgsCsException &cse )
        {
          Q_UNUSED( cse );
          QgsDebugMsg( QString( "Failed to transform a point while drawing a feature with ID '%1'. Ignoring this feature. %2" )
                       .arg( fet.id() ).arg( cse.what() ) );
        }
      }
    }
    // call postRender for each symbol layer
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = const_cast<QgsSymbolV2LevelItem&>(level[i]);
      item.symbol()->postRender( mContext, item.layer() );
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
