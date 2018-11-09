
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

#include "qgslogger.h"

#include "feature.h"
#include "labelposition.h"
#include "layer.h"
#include "pal.h"
#include "problem.h"
#include "qgsrendercontext.h"
#include "qgsmaplayer.h"


// helper function for checking for job cancelation within PAL
static bool _palIsCanceled( void *ctx )
{
  return ( reinterpret_cast< QgsRenderContext * >( ctx ) )->renderingStopped();
}

/**
 * \ingroup core
 * \class QgsLabelSorter
 * Helper class for sorting labels into correct draw order
 */
class QgsLabelSorter
{
  public:

    explicit QgsLabelSorter( const QgsMapSettings &mapSettings )
      : mMapSettings( mapSettings )
    {}

    bool operator()( pal::LabelPosition *lp1, pal::LabelPosition *lp2 ) const
    {
      QgsLabelFeature *lf1 = lp1->getFeaturePart()->feature();
      QgsLabelFeature *lf2 = lp2->getFeaturePart()->feature();

      if ( !qgsDoubleNear( lf1->zIndex(), lf2->zIndex() ) )
        return lf1->zIndex() < lf2->zIndex();

      //equal z-index, so fallback to respecting layer render order
      QStringList layerIds = mMapSettings.layerIds();
      int layer1Pos = layerIds.indexOf( lf1->provider()->layerId() );
      int layer2Pos = layerIds.indexOf( lf2->provider()->layerId() );
      if ( layer1Pos != layer2Pos && layer1Pos >= 0 && layer2Pos >= 0 )
        return layer1Pos > layer2Pos; //higher positions are rendered first

      //same layer, so render larger labels first
      return lf1->size().width() * lf1->size().height() > lf2->size().width() * lf2->size().height();
    }

  private:

    const QgsMapSettings &mMapSettings;
};


QgsLabelingEngine::QgsLabelingEngine()
  : mResults( new QgsLabelingResults )
{}

QgsLabelingEngine::~QgsLabelingEngine()
{
  qDeleteAll( mProviders );
  qDeleteAll( mSubProviders );
}

QList< QgsMapLayer * > QgsLabelingEngine::participatingLayers() const
{
  QSet< QgsMapLayer * > layers;
  Q_FOREACH ( QgsAbstractLabelProvider *provider, mProviders )
  {
    if ( provider->layer() )
      layers << provider->layer();
  }
  Q_FOREACH ( QgsAbstractLabelProvider *provider, mSubProviders )
  {
    if ( provider->layer() )
      layers << provider->layer();
  }
  return layers.toList();
}

void QgsLabelingEngine::addProvider( QgsAbstractLabelProvider *provider )
{
  provider->setEngine( this );
  mProviders << provider;
}

void QgsLabelingEngine::removeProvider( QgsAbstractLabelProvider *provider )
{
  int idx = mProviders.indexOf( provider );
  if ( idx >= 0 )
  {
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
                              flags.testFlag( QgsAbstractLabelProvider::DrawLabels ),
                              flags.testFlag( QgsAbstractLabelProvider::DrawAllLabels ) );

  // extra flags for placement of labels for linestrings
  l->setArrangementFlags( static_cast< pal::LineArrangementFlags >( provider->linePlacementFlags() ) );

  // set label mode (label per feature is the default)
  l->setLabelMode( flags.testFlag( QgsAbstractLabelProvider::LabelPerFeaturePart ) ? pal::Layer::LabelPerFeaturePart : pal::Layer::LabelPerFeature );

  // set whether adjacent lines should be merged
  l->setMergeConnectedLines( flags.testFlag( QgsAbstractLabelProvider::MergeConnectedLines ) );

  // set obstacle type
  l->setObstacleType( provider->obstacleType() );

  // set whether location of centroid must be inside of polygons
  l->setCentroidInside( flags.testFlag( QgsAbstractLabelProvider::CentroidMustBeInside ) );

  // set how to show upside-down labels
  pal::Layer::UpsideDownLabels upsdnlabels;
  switch ( provider->upsidedownLabels() )
  {
    case QgsPalLayerSettings::Upright:
      upsdnlabels = pal::Layer::Upright;
      break;
    case QgsPalLayerSettings::ShowDefined:
      upsdnlabels = pal::Layer::ShowDefined;
      break;
    case QgsPalLayerSettings::ShowAll:
      upsdnlabels = pal::Layer::ShowAll;
      break;
    default:
      Q_ASSERT( "unsupported upside-down label setting" && false );
      return;
  }
  l->setUpsidedownLabels( upsdnlabels );


  QList<QgsLabelFeature *> features = provider->labelFeatures( context );

  Q_FOREACH ( QgsLabelFeature *feature, features )
  {
    try
    {
      l->registerFeature( feature );
    }
    catch ( std::exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsgLevel( QStringLiteral( "Ignoring feature %1 due PAL exception:" ).arg( feature->id() ) + QString::fromLatin1( e.what() ), 4 );
      continue;
    }
  }

  // any sub-providers?
  Q_FOREACH ( QgsAbstractLabelProvider *subProvider, provider->subProviders() )
  {
    mSubProviders << subProvider;
    processProvider( subProvider, context, p );
  }
}


void QgsLabelingEngine::run( QgsRenderContext &context )
{
  const QgsLabelingEngineSettings &settings = mMapSettings.labelingEngineSettings();

  pal::Pal p;
  pal::SearchMethod s;
  switch ( settings.searchMethod() )
  {
    default:
    case QgsLabelingEngineSettings::Chain:
      s = pal::CHAIN;
      break;
    case QgsLabelingEngineSettings::Popmusic_Tabu:
      s = pal::POPMUSIC_TABU;
      break;
    case QgsLabelingEngineSettings::Popmusic_Chain:
      s = pal::POPMUSIC_CHAIN;
      break;
    case QgsLabelingEngineSettings::Popmusic_Tabu_Chain:
      s = pal::POPMUSIC_TABU_CHAIN;
      break;
    case QgsLabelingEngineSettings::Falp:
      s = pal::FALP;
      break;
  }
  p.setSearch( s );

  // set number of candidates generated per feature
  int candPoint, candLine, candPolygon;
  settings.numCandidatePositions( candPoint, candLine, candPolygon );
  p.setPointP( candPoint );
  p.setLineP( candLine );
  p.setPolyP( candPolygon );

  p.setShowPartial( settings.testFlag( QgsLabelingEngineSettings::UsePartialCandidates ) );


  // for each provider: get labels and register them in PAL
  Q_FOREACH ( QgsAbstractLabelProvider *provider, mProviders )
  {
    bool appendedLayerScope = false;
    if ( QgsMapLayer *ml = provider->layer() )
    {
      appendedLayerScope = true;
      context.expressionContext().appendScope( QgsExpressionContextUtils::layerScope( ml ) );
    }
    processProvider( provider, context, p );
    if ( appendedLayerScope )
      delete context.expressionContext().popScope();
  }


  // NOW DO THE LAYOUT (from QgsPalLabeling::drawLabeling)

  QPainter *painter = context.painter();

  QgsGeometry extentGeom = QgsGeometry::fromRect( mMapSettings.visibleExtent() );
  QPolygonF visiblePoly = mMapSettings.visiblePolygon();
  visiblePoly.append( visiblePoly.at( 0 ) ); //close polygon
  QgsGeometry mapBoundaryGeom = QgsGeometry::fromQPolygonF( visiblePoly );

  if ( !qgsDoubleNear( mMapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom.rotate( -mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
    // yes - this is rotated in the opposite direction... phew, this is confusing!
    mapBoundaryGeom.rotate( mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
  }

  QgsRectangle extent = extentGeom.boundingBox();


  p.registerCancelationCallback( &_palIsCanceled, reinterpret_cast< void * >( &context ) );

  QTime t;
  t.start();

  // do the labeling itself
  std::unique_ptr< pal::Problem > problem;
  try
  {
    problem = p.extractProblem( extent, mapBoundaryGeom );
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
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
  // https://issues.qgis.org/issues/11856
  QgsMapToPixel xform = mMapSettings.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel &xform = mMapSettings->mapToPixel();
#endif

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  // TODO mCandidates.clear();
  if ( settings.testFlag( QgsLabelingEngineSettings::DrawCandidates ) && problem )
  {
    painter->setBrush( Qt::NoBrush );
    for ( int i = 0; i < problem->getNumFeatures(); i++ )
    {
      for ( int j = 0; j < problem->getFeatureCandidateCount( i ); j++ )
      {
        pal::LabelPosition *lp = problem->getFeatureCandidate( i, j );

        QgsPalLabeling::drawLabelCandidateRect( lp, painter, &xform );
      }
    }
  }

  // find the solution
  QList<pal::LabelPosition *> labels = p.solveProblem( problem.get(), settings.testFlag( QgsLabelingEngineSettings::UseAllLabels ) );

  QgsDebugMsgLevel( QStringLiteral( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( labels.size() ), 4 );
  t.restart();

  if ( context.renderingStopped() )
  {
    return;
  }
  painter->setRenderHint( QPainter::Antialiasing );

  // sort labels
  std::sort( labels.begin(), labels.end(), QgsLabelSorter( mMapSettings ) );

  // draw the labels
  for ( pal::LabelPosition *label : qgis::as_const( labels ) )
  {
    if ( context.renderingStopped() )
      break;

    QgsLabelFeature *lf = label->getFeaturePart()->feature();
    if ( !lf )
    {
      continue;
    }

    lf->provider()->drawLabel( context, label );
  }

  // Reset composition mode for further drawing operations
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  QgsDebugMsgLevel( QStringLiteral( "LABELING draw:  %1 ms" ).arg( t.elapsed() ), 4 );
}

QgsLabelingResults *QgsLabelingEngine::takeResults()
{
  return mResults.release();
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
  , mFlags( DrawLabels )
  , mPlacement( QgsPalLayerSettings::AroundPoint )
  , mLinePlacementFlags( 0 )
  , mPriority( 0.5 )
  , mObstacleType( QgsPalLayerSettings::PolygonInterior )
  , mUpsidedownLabels( QgsPalLayerSettings::Upright )
{
}


//
// QgsLabelingUtils
//

QString QgsLabelingUtils::encodePredefinedPositionOrder( const QVector<QgsPalLayerSettings::PredefinedPointPosition> &positions )
{
  QStringList predefinedOrderString;
  Q_FOREACH ( QgsPalLayerSettings::PredefinedPointPosition position, positions )
  {
    switch ( position )
    {
      case QgsPalLayerSettings::TopLeft:
        predefinedOrderString << QStringLiteral( "TL" );
        break;
      case QgsPalLayerSettings::TopSlightlyLeft:
        predefinedOrderString << QStringLiteral( "TSL" );
        break;
      case QgsPalLayerSettings::TopMiddle:
        predefinedOrderString << QStringLiteral( "T" );
        break;
      case QgsPalLayerSettings::TopSlightlyRight:
        predefinedOrderString << QStringLiteral( "TSR" );
        break;
      case QgsPalLayerSettings::TopRight:
        predefinedOrderString << QStringLiteral( "TR" );
        break;
      case QgsPalLayerSettings::MiddleLeft:
        predefinedOrderString << QStringLiteral( "L" );
        break;
      case QgsPalLayerSettings::MiddleRight:
        predefinedOrderString << QStringLiteral( "R" );
        break;
      case QgsPalLayerSettings::BottomLeft:
        predefinedOrderString << QStringLiteral( "BL" );
        break;
      case QgsPalLayerSettings::BottomSlightlyLeft:
        predefinedOrderString << QStringLiteral( "BSL" );
        break;
      case QgsPalLayerSettings::BottomMiddle:
        predefinedOrderString << QStringLiteral( "B" );
        break;
      case QgsPalLayerSettings::BottomSlightlyRight:
        predefinedOrderString << QStringLiteral( "BSR" );
        break;
      case QgsPalLayerSettings::BottomRight:
        predefinedOrderString << QStringLiteral( "BR" );
        break;
    }
  }
  return predefinedOrderString.join( QStringLiteral( "," ) );
}

QVector<QgsPalLayerSettings::PredefinedPointPosition> QgsLabelingUtils::decodePredefinedPositionOrder( const QString &positionString )
{
  QVector<QgsPalLayerSettings::PredefinedPointPosition> result;
  QStringList predefinedOrderList = positionString.split( ',' );
  Q_FOREACH ( const QString &position, predefinedOrderList )
  {
    QString cleaned = position.trimmed().toUpper();
    if ( cleaned == QLatin1String( "TL" ) )
      result << QgsPalLayerSettings::TopLeft;
    else if ( cleaned == QLatin1String( "TSL" ) )
      result << QgsPalLayerSettings::TopSlightlyLeft;
    else if ( cleaned == QLatin1String( "T" ) )
      result << QgsPalLayerSettings::TopMiddle;
    else if ( cleaned == QLatin1String( "TSR" ) )
      result << QgsPalLayerSettings::TopSlightlyRight;
    else if ( cleaned == QLatin1String( "TR" ) )
      result << QgsPalLayerSettings::TopRight;
    else if ( cleaned == QLatin1String( "L" ) )
      result << QgsPalLayerSettings::MiddleLeft;
    else if ( cleaned == QLatin1String( "R" ) )
      result << QgsPalLayerSettings::MiddleRight;
    else if ( cleaned == QLatin1String( "BL" ) )
      result << QgsPalLayerSettings::BottomLeft;
    else if ( cleaned == QLatin1String( "BSL" ) )
      result << QgsPalLayerSettings::BottomSlightlyLeft;
    else if ( cleaned == QLatin1String( "B" ) )
      result << QgsPalLayerSettings::BottomMiddle;
    else if ( cleaned == QLatin1String( "BSR" ) )
      result << QgsPalLayerSettings::BottomSlightlyRight;
    else if ( cleaned == QLatin1String( "BR" ) )
      result << QgsPalLayerSettings::BottomRight;
  }
  return result;
}
