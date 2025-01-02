
/***************************************************************************
  qgslabelingengine.cpp
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingengine.h"
#include "moc_qgslabelingengine.cpp"

#include "qgslogger.h"

#include "feature.h"
#include "labelposition.h"
#include "layer.h"
#include "pal.h"
#include "problem.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"
#include "qgssymbol.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayerlabelprovider.h"
#include "qgslabelingresults.h"
#include "qgsfillsymbol.h"
#include "qgsruntimeprofiler.h"
#include "qgslabelingenginerule.h"
#include "qgstextlabelfeature.h"

#include <QUuid>

// helper function for checking for job cancellation within PAL
static bool _palIsCanceled( void *ctx )
{
  return ( reinterpret_cast< QgsRenderContext * >( ctx ) )->renderingStopped();
}

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsLabelSorter
 * \brief Helper class for sorting labels into correct draw order
 */
class QgsLabelSorter
{
  public:

    explicit QgsLabelSorter( const QStringList &layerRenderingOrderIds )
      : mLayerRenderingOrderIds( layerRenderingOrderIds )
    {}

    bool operator()( pal::LabelPosition *lp1, pal::LabelPosition *lp2 ) const
    {
      QgsLabelFeature *lf1 = lp1->getFeaturePart()->feature();
      QgsLabelFeature *lf2 = lp2->getFeaturePart()->feature();

      if ( !qgsDoubleNear( lf1->zIndex(), lf2->zIndex() ) )
        return lf1->zIndex() < lf2->zIndex();

      //equal z-index, so fallback to respecting layer render order
      int layer1Pos = mLayerRenderingOrderIds.indexOf( lf1->provider()->layerId() );
      int layer2Pos = mLayerRenderingOrderIds.indexOf( lf2->provider()->layerId() );
      if ( layer1Pos != layer2Pos && layer1Pos >= 0 && layer2Pos >= 0 )
        return layer1Pos > layer2Pos; //higher positions are rendered first

      //same layer, so render larger labels first
      return lf1->size().width() * lf1->size().height() > lf2->size().width() * lf2->size().height();
    }

  private:

    const QStringList mLayerRenderingOrderIds;
};

///@endcond

//
// QgsLabelingEngine
//

QgsLabelingEngine::QgsLabelingEngine()
  : mResults( new QgsLabelingResults )
{}

QgsLabelingEngine::~QgsLabelingEngine()
{
  qDeleteAll( mProviders );
  qDeleteAll( mSubProviders );
}

void QgsLabelingEngine::setMapSettings( const QgsMapSettings &mapSettings )
{
  mMapSettings = mapSettings;
  mLayerRenderingOrderIds = mMapSettings.layerIds();
  if ( mResults )
    mResults->setMapSettings( mapSettings );
}

bool QgsLabelingEngine::prepare( QgsRenderContext &context )
{
  const QList<const QgsAbstractLabelingEngineRule *> rules = mMapSettings.labelingEngineSettings().rules();
  bool res = true;
  for ( const QgsAbstractLabelingEngineRule *rule : rules )
  {
    if ( !rule->active() || !rule->isAvailable() )
      continue;

    std::unique_ptr< QgsAbstractLabelingEngineRule > ruleClone( rule->clone() );
    res = ruleClone->prepare( context ) && res;
    mEngineRules.emplace_back( std::move( ruleClone ) );
  }
  return res;
}

QList< QgsMapLayer * > QgsLabelingEngine::participatingLayers() const
{
  QList< QgsMapLayer * > layers;

  // try to return layers sorted in the desired z order for rendering
  QList< QgsAbstractLabelProvider * > providersByZ = mProviders;
  std::sort( providersByZ.begin(), providersByZ.end(),
             []( const QgsAbstractLabelProvider * a, const QgsAbstractLabelProvider * b ) -> bool
  {
    const QgsVectorLayerLabelProvider *providerA = dynamic_cast<const QgsVectorLayerLabelProvider *>( a );
    const QgsVectorLayerLabelProvider *providerB = dynamic_cast<const QgsVectorLayerLabelProvider *>( b );

    if ( providerA && providerB )
    {
      return providerA->settings().zIndex < providerB->settings().zIndex ;
    }
    return false;
  } );

  QList< QgsAbstractLabelProvider * > subProvidersByZ = mSubProviders;
  std::sort( subProvidersByZ.begin(), subProvidersByZ.end(),
             []( const QgsAbstractLabelProvider * a, const QgsAbstractLabelProvider * b ) -> bool
  {
    const QgsVectorLayerLabelProvider *providerA = dynamic_cast<const QgsVectorLayerLabelProvider *>( a );
    const QgsVectorLayerLabelProvider *providerB = dynamic_cast<const QgsVectorLayerLabelProvider *>( b );

    if ( providerA && providerB )
    {
      return providerA->settings().zIndex < providerB->settings().zIndex ;
    }
    return false;
  } );

  for ( QgsAbstractLabelProvider *provider : std::as_const( providersByZ ) )
  {
    if ( provider->layer() && !layers.contains( provider->layer() ) )
      layers << provider->layer();
  }
  for ( QgsAbstractLabelProvider *provider : std::as_const( subProvidersByZ ) )
  {
    if ( provider->layer() && !layers.contains( provider->layer() ) )
      layers << provider->layer();
  }
  return layers;
}

QStringList QgsLabelingEngine::participatingLayerIds() const
{
  QStringList layers;

  // try to return layers sorted in the desired z order for rendering
  QList< QgsAbstractLabelProvider * > providersByZ = mProviders;
  std::sort( providersByZ.begin(), providersByZ.end(),
             []( const QgsAbstractLabelProvider * a, const QgsAbstractLabelProvider * b ) -> bool
  {
    const QgsVectorLayerLabelProvider *providerA = dynamic_cast<const QgsVectorLayerLabelProvider *>( a );
    const QgsVectorLayerLabelProvider *providerB = dynamic_cast<const QgsVectorLayerLabelProvider *>( b );

    if ( providerA && providerB )
    {
      return providerA->settings().zIndex < providerB->settings().zIndex ;
    }
    return false;
  } );

  QList< QgsAbstractLabelProvider * > subProvidersByZ = mSubProviders;
  std::sort( subProvidersByZ.begin(), subProvidersByZ.end(),
             []( const QgsAbstractLabelProvider * a, const QgsAbstractLabelProvider * b ) -> bool
  {
    const QgsVectorLayerLabelProvider *providerA = dynamic_cast<const QgsVectorLayerLabelProvider *>( a );
    const QgsVectorLayerLabelProvider *providerB = dynamic_cast<const QgsVectorLayerLabelProvider *>( b );

    if ( providerA && providerB )
    {
      return providerA->settings().zIndex < providerB->settings().zIndex ;
    }
    return false;
  } );

  for ( QgsAbstractLabelProvider *provider : std::as_const( providersByZ ) )
  {
    if ( !layers.contains( provider->layerId() ) )
      layers << provider->layerId();
  }
  for ( QgsAbstractLabelProvider *provider : std::as_const( subProvidersByZ ) )
  {
    if ( !layers.contains( provider->layerId() ) )
      layers << provider->layerId();
  }
  return layers;
}

QString QgsLabelingEngine::addProvider( QgsAbstractLabelProvider *provider )
{
  provider->setEngine( this );
  mProviders << provider;
  const QString id = QUuid::createUuid().toString( QUuid::WithoutBraces );
  mProvidersById.insert( id, provider );
  return id;
}

QgsAbstractLabelProvider *QgsLabelingEngine::providerById( const QString &id )
{
  return mProvidersById.value( id );
}

void QgsLabelingEngine::removeProvider( QgsAbstractLabelProvider *provider )
{
  int idx = mProviders.indexOf( provider );
  if ( idx >= 0 )
  {
    mProvidersById.remove( mProvidersById.key( provider ) );
    delete mProviders.takeAt( idx );
  }
}

void QgsLabelingEngine::processProvider( QgsAbstractLabelProvider *provider, QgsRenderContext &context, pal::Pal &p )
{
  QgsAbstractLabelProvider::Flags flags = provider->flags();

  // create the pal layer
  pal::Layer *l = p.addLayer( provider,
                              provider->name(),
                              provider->placement(),
                              provider->priority(),
                              true,
                              flags.testFlag( QgsAbstractLabelProvider::DrawLabels ) );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( flags.testFlag( QgsAbstractLabelProvider::MergeConnectedLines ) );

  // set obstacle type
  l->setObstacleType( provider->obstacleType() );

  // set whether location of centroid must be inside of polygons
  l->setCentroidInside( flags.testFlag( QgsAbstractLabelProvider::CentroidMustBeInside ) );

  // set how to show upside-down labels
  l->setUpsidedownLabels( provider->upsidedownLabels() );

  const QList<QgsLabelFeature *> features = provider->labelFeatures( context );

  for ( QgsLabelFeature *feature : features )
  {
    try
    {
      l->registerFeature( feature );
    }
    catch ( std::exception &e )
    {
      Q_UNUSED( e )
      QgsDebugMsgLevel( QStringLiteral( "Ignoring feature %1 due PAL exception:" ).arg( feature->id() ) + QString::fromLatin1( e.what() ), 4 );
      continue;
    }
  }

  // any sub-providers?
  const auto subproviders = provider->subProviders();
  for ( QgsAbstractLabelProvider *subProvider : subproviders )
  {
    mSubProviders << subProvider;
    processProvider( subProvider, context, p );
  }
}

void QgsLabelingEngine::registerLabels( QgsRenderContext &context )
{
  std::unique_ptr< QgsScopedRuntimeProfile > registeringProfile;
  if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    registeringProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Registering labels" ), QStringLiteral( "rendering" ) );
  }

  QgsLabelingEngineFeedback *feedback = qobject_cast< QgsLabelingEngineFeedback * >( context.feedback() );

  if ( feedback )
    feedback->emit labelRegistrationAboutToBegin();

  const QgsLabelingEngineSettings &settings = mMapSettings.labelingEngineSettings();

  mPal = std::make_unique< pal::Pal >();

  mPal->setMaximumLineCandidatesPerMapUnit( settings.maximumLineCandidatesPerCm() / context.convertToMapUnits( 10, Qgis::RenderUnit::Millimeters ) );
  mPal->setMaximumPolygonCandidatesPerMapUnitSquared( settings.maximumPolygonCandidatesPerCmSquared() / std::pow( context.convertToMapUnits( 10, Qgis::RenderUnit::Millimeters ), 2 ) );

  mPal->setShowPartialLabels( settings.testFlag( Qgis::LabelingFlag::UsePartialCandidates ) );
  mPal->setPlacementVersion( settings.placementVersion() );

  QList< QgsAbstractLabelingEngineRule * > rules;
  rules.reserve( static_cast< int >( mEngineRules.size() ) );
  for ( auto &it : mEngineRules )
  {
    rules.append( it.get() );
  }
  mPal->setRules( rules );

  // for each provider: get labels and register them in PAL
  const double step = !mProviders.empty() ? 100.0 / mProviders.size() : 1;
  int index = 0;
  for ( QgsAbstractLabelProvider *provider : std::as_const( mProviders ) )
  {
    if ( feedback )
    {
      feedback->emit providerRegistrationAboutToBegin( provider );
      feedback->setProgress( index * step );
    }
    index++;
    std::unique_ptr< QgsExpressionContextScopePopper > layerScopePopper;
    if ( provider->layerExpressionContextScope() )
    {
      layerScopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.expressionContext(), new QgsExpressionContextScope( *provider->layerExpressionContextScope() ) );
    }
    processProvider( provider, context, *mPal );
    if ( feedback )
      feedback->emit providerRegistrationFinished( provider );
  }
  if ( feedback )
    feedback->emit labelRegistrationFinished();
}

void QgsLabelingEngine::solve( QgsRenderContext &context )
{
  Q_ASSERT( mPal.get() );

  // NOW DO THE LAYOUT (from QgsPalLabeling::drawLabeling)
  const QgsLabelingEngineSettings &settings = mMapSettings.labelingEngineSettings();

  QPainter *painter = context.painter();

  QgsRectangle r1 = mMapSettings.visibleExtent();
  r1.grow( mMapSettings.extentBuffer() );
  QgsGeometry extentGeom = QgsGeometry::fromRect( r1 );

  QPolygonF visiblePoly = mMapSettings.visiblePolygonWithBuffer();
  visiblePoly.append( visiblePoly.at( 0 ) ); //close polygon

  // get map label boundary geometry - if one hasn't been explicitly set, we use the whole of the map's visible polygon
  QgsGeometry mapBoundaryGeom = !mMapSettings.labelBoundaryGeometry().isNull() ? mMapSettings.labelBoundaryGeometry() : QgsGeometry::fromQPolygonF( visiblePoly );

  // label blocking regions work by "chopping away" those regions from the permissible labeling area
  const QList< QgsLabelBlockingRegion > blockingRegions = mMapSettings.labelBlockingRegions();
  for ( const QgsLabelBlockingRegion &region : blockingRegions )
  {
    mapBoundaryGeom = mapBoundaryGeom.difference( region.geometry );
  }

  if ( settings.flags() & Qgis::LabelingFlag::DrawCandidates )
  {
    // draw map boundary
    QgsFeature f;
    f.setGeometry( mapBoundaryGeom );
    QVariantMap properties;
    properties.insert( QStringLiteral( "style" ), QStringLiteral( "no" ) );
    properties.insert( QStringLiteral( "style_border" ), QStringLiteral( "solid" ) );
    properties.insert( QStringLiteral( "color_border" ), QStringLiteral( "#0000ff" ) );
    properties.insert( QStringLiteral( "width_border" ), QStringLiteral( "0.3" ) );
    properties.insert( QStringLiteral( "joinstyle" ), QStringLiteral( "miter" ) );
    std::unique_ptr< QgsFillSymbol > boundarySymbol( QgsFillSymbol::createSimple( properties ) );
    boundarySymbol->startRender( context );
    boundarySymbol->renderFeature( f, context );
    boundarySymbol->stopRender( context );
  }

  if ( !qgsDoubleNear( mMapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
    // yes - this is rotated in the opposite direction... phew, this is confusing!
    mapBoundaryGeom.rotate( mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
  }

  QgsRectangle extent = extentGeom.boundingBox();

  mPal->registerCancellationCallback( &_palIsCanceled, reinterpret_cast< void * >( &context ) );

  QElapsedTimer t;
  t.start();

  // do the labeling itself
  try
  {
    mProblem = mPal->extractProblem( extent, mapBoundaryGeom, context );
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e )
    QgsDebugMsgLevel( "PAL EXCEPTION :-( " + QString::fromLatin1( e.what() ), 4 );
    return;
  }

  if ( context.renderingStopped() )
  {
    return; // it has been canceled
  }

#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // https://github.com/qgis/QGIS/issues/20071
  QgsMapToPixel xform = mMapSettings.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel &xform = mMapSettings->mapToPixel();
#endif

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  // TODO mCandidates.clear();
  if ( settings.testFlag( Qgis::LabelingFlag::DrawCandidates ) && mProblem )
  {
    painter->setBrush( Qt::NoBrush );
    for ( int i = 0; i < static_cast< int >( mProblem->featureCount() ); i++ )
    {
      for ( int j = 0; j < mProblem->featureCandidateCount( i ); j++ )
      {
        pal::LabelPosition *lp = mProblem->featureCandidate( i, j );

        drawLabelCandidateRect( lp, context, &xform );
      }
    }
  }

  // find the solution
  mLabels = mPal->solveProblem( mProblem.get(), context,
                                settings.testFlag( Qgis::LabelingFlag::UseAllLabels ),
                                settings.testFlag( Qgis::LabelingFlag::DrawUnplacedLabels )
                                || settings.testFlag( Qgis::LabelingFlag::CollectUnplacedLabels ) ? &mUnlabeled : nullptr );

  // sort labels
  std::sort( mLabels.begin(), mLabels.end(), QgsLabelSorter( mLayerRenderingOrderIds ) );

  QgsDebugMsgLevel( QStringLiteral( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( mLabels.size() ), 4 );
}

void QgsLabelingEngine::drawLabels( QgsRenderContext &context, const QString &layerId )
{
  QElapsedTimer t;
  t.start();

  std::unique_ptr< QgsScopedRuntimeProfile > drawingProfile;
  if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    drawingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering labels" ), QStringLiteral( "rendering" ) );
  }

  const QgsLabelingEngineSettings &settings = mMapSettings.labelingEngineSettings();

  context.setPainterFlagsUsingContext();
  QPainter *painter = context.painter();

  // prepare for rendering
  for ( QgsAbstractLabelProvider *provider : std::as_const( mProviders ) )
  {
    if ( !layerId.isEmpty() && provider->layerId() != layerId )
      continue;

    // provider will require the correct layer scope for expression preparation - at this stage, the existing expression context
    // only contains generic scopes
    QgsExpressionContextScopePopper popper( context.expressionContext(), provider->layerExpressionContextScope() ? new QgsExpressionContextScope( *provider->layerExpressionContextScope() ) : new QgsExpressionContextScope() );

    QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, provider->layerReferenceScale() );
    provider->startRender( context );
  }

  QgsExpressionContextScope *symbolScope = new QgsExpressionContextScope();
  std::unique_ptr< QgsExpressionContextScopePopper > symbolScopePopper = std::make_unique< QgsExpressionContextScopePopper >( context.expressionContext(), symbolScope );

  // draw label backgrounds
  for ( pal::LabelPosition *label : std::as_const( mLabels ) )
  {
    if ( context.renderingStopped() )
      break;

    QgsLabelFeature *lf = label->getFeaturePart()->feature();
    if ( !lf )
    {
      continue;
    }

    if ( !layerId.isEmpty() && lf->provider()->layerId() != layerId )
      continue;

    context.expressionContext().setFeature( lf->feature() );
    context.expressionContext().setFields( lf->feature().fields() );

    QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, lf->provider()->layerReferenceScale() );

    if ( lf->symbol() )
    {
      symbolScope = QgsExpressionContextUtils::updateSymbolScope( lf->symbol(), symbolScope );
    }
    lf->provider()->drawLabelBackground( context, label );
  }

  if ( engineSettings().testFlag( Qgis::LabelingFlag::DrawLabelRectOnly ) )
  {
    // features are pre-rotated but not scaled/translated,
    // so we only disable rotation here. Ideally, they'd be
    // also pre-scaled/translated, as suggested here:
    // https://github.com/qgis/QGIS/issues/20071
    QgsMapToPixel xform = context.mapToPixel();
    xform.setMapRotation( 0, 0, 0 );

    std::function<void( pal::LabelPosition * )> drawLabelRect;
    drawLabelRect = [&xform, painter, &drawLabelRect]( pal::LabelPosition * label )
    {
      QPointF outPt = xform.transform( label->getX(), label->getY() ).toQPointF();

      QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
      QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
      painter->save();
      painter->setRenderHint( QPainter::Antialiasing, false );
      painter->translate( QPointF( outPt.x(), outPt.y() ) );
      painter->rotate( -label->getAlpha() * 180 / M_PI );

      if ( label->conflictsWithObstacle() )
      {
        painter->setBrush( QColor( 255, 0, 0, 100 ) );
        painter->setPen( QColor( 255, 0, 0, 150 ) );
      }
      else
      {
        painter->setBrush( QColor( 0, 255, 0, 100 ) );
        painter->setPen( QColor( 0, 255, 0, 150 ) );
      }

      painter->drawRect( rect );
      painter->restore();

      if ( pal::LabelPosition *nextPart = label->nextPart() )
        drawLabelRect( nextPart );
    };

    for ( pal::LabelPosition *label : std::as_const( mLabels ) )
    {
      drawLabelRect( label );
    }

    if ( settings.testFlag( Qgis::LabelingFlag::DrawUnplacedLabels ) )
    {
      for ( pal::LabelPosition *label : std::as_const( mUnlabeled ) )
      {
        drawLabelRect( label );
      }
    }
  }
  else
  {
    if ( engineSettings().testFlag( Qgis::LabelingFlag::DrawLabelMetrics ) )
    {
      // features are pre-rotated but not scaled/translated,
      // so we only disable rotation here. Ideally, they'd be
      // also pre-scaled/translated, as suggested here:
      // https://github.com/qgis/QGIS/issues/20071
      QgsMapToPixel xform = context.mapToPixel();
      xform.setMapRotation( 0, 0, 0 );

      std::function<void( pal::LabelPosition * )> drawLabelMetricsRecursive;
      drawLabelMetricsRecursive = [&xform, &context, &drawLabelMetricsRecursive]( pal::LabelPosition * label )
      {
        QPointF outPt = xform.transform( label->getX(), label->getY() ).toQPointF();
        QgsLabelingEngine::drawLabelMetrics( label, xform, context, outPt );
        if ( pal::LabelPosition *nextPart = label->nextPart() )
          drawLabelMetricsRecursive( nextPart );
      };

      for ( pal::LabelPosition *label : std::as_const( mLabels ) )
      {
        drawLabelMetricsRecursive( label );
      }
    }

    // draw the labels
    for ( pal::LabelPosition *label : std::as_const( mLabels ) )
    {
      if ( context.renderingStopped() )
        break;

      QgsLabelFeature *lf = label->getFeaturePart()->feature();
      if ( !lf )
      {
        continue;
      }

      if ( !layerId.isEmpty() && lf->provider()->layerId() != layerId )
        continue;

      context.expressionContext().setFeature( lf->feature() );
      context.expressionContext().setFields( lf->feature().fields() );

      QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, lf->provider()->layerReferenceScale() );
      if ( lf->symbol() )
      {
        symbolScope = QgsExpressionContextUtils::updateSymbolScope( lf->symbol(), symbolScope );
      }
      lf->provider()->drawLabel( context, label );
      // finished with symbol -- we can't keep it around after this, it may be deleted
      lf->setSymbol( nullptr );
    }

    // draw unplaced labels. These are always rendered on top
    if ( settings.testFlag( Qgis::LabelingFlag::DrawUnplacedLabels ) || settings.testFlag( Qgis::LabelingFlag::CollectUnplacedLabels ) )
    {
      for ( pal::LabelPosition *label : std::as_const( mUnlabeled ) )
      {
        if ( context.renderingStopped() )
          break;
        QgsLabelFeature *lf = label->getFeaturePart()->feature();
        if ( !lf )
        {
          continue;
        }

        if ( !layerId.isEmpty() && lf->provider()->layerId() != layerId )
          continue;

        context.expressionContext().setFeature( lf->feature() );
        context.expressionContext().setFields( lf->feature().fields() );

        QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, lf->provider()->layerReferenceScale() );
        if ( lf->symbol() )
        {
          symbolScope = QgsExpressionContextUtils::updateSymbolScope( lf->symbol(), symbolScope );
        }
        lf->provider()->drawUnplacedLabel( context, label );
        // finished with symbol -- we can't keep it around after this, it may be deleted
        lf->setSymbol( nullptr );
      }
    }
  }

  symbolScopePopper.reset();

  // cleanup
  for ( QgsAbstractLabelProvider *provider : std::as_const( mProviders ) )
  {
    if ( !layerId.isEmpty() && provider->layerId() != layerId )
      continue;

    provider->stopRender( context );
  }

  // Reset composition mode for further drawing operations
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  QgsDebugMsgLevel( QStringLiteral( "LABELING draw:  %1 ms" ).arg( t.elapsed() ), 4 );
}

void QgsLabelingEngine::cleanup()
{
  mUnlabeled.clear();
  mLabels.clear();
  mProblem.reset();
  mPal.reset();
}

QgsLabelingResults *QgsLabelingEngine::takeResults()
{
  return mResults.release();
}

void QgsLabelingEngine::drawLabelCandidateRect( pal::LabelPosition *lp, QgsRenderContext &context, const QgsMapToPixel *xform, QList<QgsLabelCandidate> *candidates )
{
  QPainter *painter = context.painter();
  if ( !painter )
    return;

  QgsPointXY outPt = xform->transform( lp->getX(), lp->getY() );

  painter->save();

  QgsPointXY outPt2 = xform->transform( lp->getX() + lp->getWidth(), lp->getY() + lp->getHeight() );
  QRectF rect( 0, 0, outPt2.x() - outPt.x(), outPt2.y() - outPt.y() );
  painter->translate( QPointF( outPt.x(), outPt.y() ) );
  painter->rotate( -lp->getAlpha() * 180 / M_PI );

  if ( lp->conflictsWithObstacle() )
  {
    painter->setPen( QColor( 255, 0, 0, 64 ) );
  }
  else
  {
    painter->setPen( QColor( 0, 0, 0, 64 ) );
  }
  painter->drawRect( rect );
  painter->restore();

  // save the rect
  rect.moveTo( outPt.x(), outPt.y() );
  if ( candidates )
    candidates->append( QgsLabelCandidate( rect, lp->cost() * 1000 ) );

  // show all parts of the multipart label
  if ( lp->nextPart() )
    drawLabelCandidateRect( lp->nextPart(), context, xform, candidates );
}

void QgsLabelingEngine::drawLabelMetrics( pal::LabelPosition *label, const QgsMapToPixel &xform, QgsRenderContext &context, const QPointF &renderPoint )
{
  QPainter *painter = context.painter();
  if ( !painter )
    return;

  QgsPointXY outPt2 = xform.transform( label->getX() + label->getWidth(), label->getY() + label->getHeight() );
  QRectF rect( 0, 0, outPt2.x() - renderPoint.x(), outPt2.y() - renderPoint.y() );
  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->translate( QPointF( renderPoint.x(), renderPoint.y() ) );
  painter->rotate( -label->getAlpha() * 180 / M_PI );

  painter->setBrush( Qt::NoBrush );
  painter->setPen( QColor( 255, 0, 0, 220 ) );

  painter->drawRect( rect );

  painter->setPen( QColor( 0, 0, 0, 60 ) );
  const QgsMargins &margins = label->getFeaturePart()->feature()->visualMargin();
  if ( margins.top() > 0 )
  {
    const double topMargin = margins.top() / context.mapToPixel().mapUnitsPerPixel();
    painter->drawLine( QPointF( rect.left(), rect.top() - topMargin ), QPointF( rect.right(), rect.top() - topMargin ) );
  }
  if ( margins.bottom() > 0 )
  {
    const double bottomMargin = margins.top() / context.mapToPixel().mapUnitsPerPixel();
    painter->drawLine( QPointF( rect.left(), rect.bottom() + bottomMargin ), QPointF( rect.right(), rect.bottom() + bottomMargin ) );
  }

  const QRectF outerBounds = label->getFeaturePart()->feature()->outerBounds();
  if ( !outerBounds.isNull() )
  {
    const QRectF mapOuterBounds = QRectF( label->getX() + outerBounds.left(),
                                          label->getY() + outerBounds.top(),
                                          outerBounds.width(), outerBounds.height() );

    QgsPointXY outerBoundsPt1 = xform.transform( mapOuterBounds.left(), mapOuterBounds.top() );
    QgsPointXY outerBoundsPt2 = xform.transform( mapOuterBounds.right(), mapOuterBounds.bottom() );

    const QRectF outerBoundsPixel( outerBoundsPt1.x() - renderPoint.x(),
                                   outerBoundsPt1.y() - renderPoint.y(),
                                   outerBoundsPt2.x() - outerBoundsPt1.x(),
                                   outerBoundsPt2.y() - outerBoundsPt1.y() );

    QPen pen( QColor( 255, 0, 255, 140 ) );
    pen.setCosmetic( true );
    pen.setWidth( 1 );
    painter->setPen( pen );
    painter->drawRect( outerBoundsPixel );
  }

  if ( QgsTextLabelFeature *textFeature = dynamic_cast< QgsTextLabelFeature * >( label->getFeaturePart()->feature() ) )
  {
    const QgsTextDocumentMetrics &metrics = textFeature->documentMetrics();
    const QgsTextDocument &document = textFeature->document();
    const int blockCount = document.size();

    double prevBlockBaseline = rect.bottom() - rect.top();
    const double verticalAlignOffset = -metrics.blockVerticalMargin( document.size() - 1 );

    // draw block baselines
    for ( int blockIndex = 0; blockIndex < blockCount; ++blockIndex )
    {
      const double blockBaseLine = metrics.baselineOffset( blockIndex, Qgis::TextLayoutMode::Labeling );

      const QgsTextBlock &block = document.at( blockIndex );
      const int fragmentCount = block.size();
      double left = metrics.blockLeftMargin( blockIndex );
      for ( int fragmentIndex = 0; fragmentIndex < fragmentCount; ++fragmentIndex )
      {
        const double fragmentVerticalOffset = metrics.fragmentVerticalOffset( blockIndex, fragmentIndex, Qgis::TextLayoutMode::Labeling );
        const double right = left + metrics.fragmentHorizontalAdvance( blockIndex, fragmentIndex, Qgis::TextLayoutMode::Labeling );

        if ( fragmentIndex > 0 )
        {
          QPen pen( QColor( 0, 0, 255, 220 ) );
          pen.setStyle( Qt::PenStyle::DashLine );

          painter->setPen( pen );

          painter->drawLine( QPointF( rect.left() + left, rect.top() + blockBaseLine + fragmentVerticalOffset + verticalAlignOffset ),
                             QPointF( rect.left() + left, rect.top() + prevBlockBaseline + verticalAlignOffset ) );

        }

        painter->setPen( QColor( 0, 0, 255, 220 ) );
        painter->drawLine( QPointF( rect.left() + left, rect.top()  + blockBaseLine + fragmentVerticalOffset + verticalAlignOffset ),
                           QPointF( rect.left() + right, rect.top() + blockBaseLine + fragmentVerticalOffset + verticalAlignOffset ) );
        left = right;
      }
      prevBlockBaseline = blockBaseLine;
    }
  }

  painter->restore();
}


//
//  QgsDefaultLabelingEngine
//

QgsDefaultLabelingEngine::QgsDefaultLabelingEngine()
  : QgsLabelingEngine()
{

}

void QgsDefaultLabelingEngine::run( QgsRenderContext &context )
{
  registerLabels( context );
  if ( context.renderingStopped() )
  {
    cleanup();
    return; // it has been canceled
  }

  solve( context );
  if ( context.renderingStopped() )
  {
    cleanup();
    return;
  }

  drawLabels( context );
  cleanup();
}


//
//  QgsStagedRenderLabelingEngine
//

QgsStagedRenderLabelingEngine::QgsStagedRenderLabelingEngine()
  : QgsLabelingEngine()
{

}

void QgsStagedRenderLabelingEngine::run( QgsRenderContext &context )
{
  registerLabels( context );
  if ( context.renderingStopped() )
  {
    cleanup();
    return; // it has been canceled
  }

  solve( context );
  if ( context.renderingStopped() )
  {
    cleanup();
    return;
  }
}


void QgsStagedRenderLabelingEngine::renderLabelsForLayer( QgsRenderContext &context, const QString &layerId )
{
  drawLabels( context, layerId );
}

void QgsStagedRenderLabelingEngine::finalize()
{
  cleanup();
}


////

QgsAbstractLabelProvider *QgsLabelFeature::provider() const
{
  return mLayer ? mLayer->provider() : nullptr;

}

QgsAbstractLabelProvider::QgsAbstractLabelProvider( QgsMapLayer *layer, const QString &providerId )
  : mLayerId( layer ? layer->id() : QString() )
  , mLayer( layer )
  , mProviderId( providerId )
{
  if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
  {
    mLayerExpressionContextScope.reset( vl->createExpressionContextScope() );
    if ( const QgsFeatureRenderer *renderer = vl->renderer() )
      mLayerReferenceScale = renderer->referenceScale();
  }
}

void QgsAbstractLabelProvider::drawUnplacedLabel( QgsRenderContext &, pal::LabelPosition * ) const
{

}

void QgsAbstractLabelProvider::drawLabelBackground( QgsRenderContext &, pal::LabelPosition * ) const
{

}

void QgsAbstractLabelProvider::startRender( QgsRenderContext &context )
{
  const auto subproviders = subProviders();
  for ( QgsAbstractLabelProvider *subProvider : subproviders )
  {
    subProvider->startRender( context );
  }
}

void QgsAbstractLabelProvider::stopRender( QgsRenderContext &context )
{
  const auto subproviders = subProviders();
  for ( QgsAbstractLabelProvider *subProvider : subproviders )
  {
    subProvider->stopRender( context );
  }
}

QgsExpressionContextScope *QgsAbstractLabelProvider::layerExpressionContextScope() const
{
  return mLayerExpressionContextScope.get();
}

//
// QgsLabelingUtils
//

QString QgsLabelingUtils::encodePredefinedPositionOrder( const QVector<Qgis::LabelPredefinedPointPosition> &positions )
{
  QStringList predefinedOrderString;
  const auto constPositions = positions;
  for ( Qgis::LabelPredefinedPointPosition position : constPositions )
  {
    switch ( position )
    {
      case Qgis::LabelPredefinedPointPosition::TopLeft:
        predefinedOrderString << QStringLiteral( "TL" );
        break;
      case Qgis::LabelPredefinedPointPosition::TopSlightlyLeft:
        predefinedOrderString << QStringLiteral( "TSL" );
        break;
      case Qgis::LabelPredefinedPointPosition::TopMiddle:
        predefinedOrderString << QStringLiteral( "T" );
        break;
      case Qgis::LabelPredefinedPointPosition::TopSlightlyRight:
        predefinedOrderString << QStringLiteral( "TSR" );
        break;
      case Qgis::LabelPredefinedPointPosition::TopRight:
        predefinedOrderString << QStringLiteral( "TR" );
        break;
      case Qgis::LabelPredefinedPointPosition::MiddleLeft:
        predefinedOrderString << QStringLiteral( "L" );
        break;
      case Qgis::LabelPredefinedPointPosition::MiddleRight:
        predefinedOrderString << QStringLiteral( "R" );
        break;
      case Qgis::LabelPredefinedPointPosition::BottomLeft:
        predefinedOrderString << QStringLiteral( "BL" );
        break;
      case Qgis::LabelPredefinedPointPosition::BottomSlightlyLeft:
        predefinedOrderString << QStringLiteral( "BSL" );
        break;
      case Qgis::LabelPredefinedPointPosition::BottomMiddle:
        predefinedOrderString << QStringLiteral( "B" );
        break;
      case Qgis::LabelPredefinedPointPosition::BottomSlightlyRight:
        predefinedOrderString << QStringLiteral( "BSR" );
        break;
      case Qgis::LabelPredefinedPointPosition::BottomRight:
        predefinedOrderString << QStringLiteral( "BR" );
        break;
      case Qgis::LabelPredefinedPointPosition::OverPoint:
        predefinedOrderString << QStringLiteral( "O" );
        break;
    }
  }
  return predefinedOrderString.join( ',' );
}

QVector<Qgis::LabelPredefinedPointPosition> QgsLabelingUtils::decodePredefinedPositionOrder( const QString &positionString )
{
  QVector<Qgis::LabelPredefinedPointPosition> result;
  const QStringList predefinedOrderList = positionString.split( ',' );
  result.reserve( predefinedOrderList.size() );
  for ( const QString &position : predefinedOrderList )
  {
    QString cleaned = position.trimmed().toUpper();
    if ( cleaned == QLatin1String( "TL" ) )
      result << Qgis::LabelPredefinedPointPosition::TopLeft;
    else if ( cleaned == QLatin1String( "TSL" ) )
      result << Qgis::LabelPredefinedPointPosition::TopSlightlyLeft;
    else if ( cleaned == QLatin1String( "T" ) )
      result << Qgis::LabelPredefinedPointPosition::TopMiddle;
    else if ( cleaned == QLatin1String( "TSR" ) )
      result << Qgis::LabelPredefinedPointPosition::TopSlightlyRight;
    else if ( cleaned == QLatin1String( "TR" ) )
      result << Qgis::LabelPredefinedPointPosition::TopRight;
    else if ( cleaned == QLatin1String( "L" ) )
      result << Qgis::LabelPredefinedPointPosition::MiddleLeft;
    else if ( cleaned == QLatin1String( "R" ) )
      result << Qgis::LabelPredefinedPointPosition::MiddleRight;
    else if ( cleaned == QLatin1String( "BL" ) )
      result << Qgis::LabelPredefinedPointPosition::BottomLeft;
    else if ( cleaned == QLatin1String( "BSL" ) )
      result << Qgis::LabelPredefinedPointPosition::BottomSlightlyLeft;
    else if ( cleaned == QLatin1String( "B" ) )
      result << Qgis::LabelPredefinedPointPosition::BottomMiddle;
    else if ( cleaned == QLatin1String( "BSR" ) )
      result << Qgis::LabelPredefinedPointPosition::BottomSlightlyRight;
    else if ( cleaned == QLatin1String( "BR" ) )
      result << Qgis::LabelPredefinedPointPosition::BottomRight;
    else if ( cleaned == QLatin1String( "O" ) )
      result << Qgis::LabelPredefinedPointPosition::OverPoint;
  }
  return result;
}

QString QgsLabelingUtils::encodeLinePlacementFlags( Qgis::LabelLinePlacementFlags flags )
{
  QStringList parts;
  if ( flags & Qgis::LabelLinePlacementFlag::OnLine )
    parts << QStringLiteral( "OL" );
  if ( flags & Qgis::LabelLinePlacementFlag::AboveLine )
    parts << QStringLiteral( "AL" );
  if ( flags & Qgis::LabelLinePlacementFlag::BelowLine )
    parts << QStringLiteral( "BL" );
  if ( !( flags & Qgis::LabelLinePlacementFlag::MapOrientation ) )
    parts << QStringLiteral( "LO" );
  return parts.join( ',' );
}

Qgis::LabelLinePlacementFlags QgsLabelingUtils::decodeLinePlacementFlags( const QString &string )
{
  Qgis::LabelLinePlacementFlags flags = Qgis::LabelLinePlacementFlags();
  const QStringList flagList = string.split( ',' );
  bool foundLineOrientationFlag = false;
  for ( const QString &flag : flagList )
  {
    QString cleaned = flag.trimmed().toUpper();
    if ( cleaned == QLatin1String( "OL" ) )
      flags |= Qgis::LabelLinePlacementFlag::OnLine;
    else if ( cleaned == QLatin1String( "AL" ) )
      flags |= Qgis::LabelLinePlacementFlag::AboveLine;
    else if ( cleaned == QLatin1String( "BL" ) )
      flags |= Qgis::LabelLinePlacementFlag::BelowLine;
    else if ( cleaned == QLatin1String( "LO" ) )
      foundLineOrientationFlag = true;
  }
  if ( !foundLineOrientationFlag )
    flags |= Qgis::LabelLinePlacementFlag::MapOrientation;
  return flags;
}
