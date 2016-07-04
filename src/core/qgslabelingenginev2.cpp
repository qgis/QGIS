/***************************************************************************
  qgslabelingenginev2.cpp
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

#include "qgslabelingenginev2.h"

#include "qgslogger.h"
#include "qgsproject.h"

#include "feature.h"
#include "labelposition.h"
#include "layer.h"
#include "pal.h"
#include "problem.h"



// helper function for checking for job cancellation within PAL
static bool _palIsCancelled( void* ctx )
{
  return ( reinterpret_cast< QgsRenderContext* >( ctx ) )->renderingStopped();
}

/** \ingroup core
 * \class QgsLabelSorter
 * Helper class for sorting labels into correct draw order
 */
class QgsLabelSorter
{
  public:

    explicit QgsLabelSorter( const QgsMapSettings& mapSettings )
        : mMapSettings( mapSettings )
    {}

    bool operator()( pal::LabelPosition* lp1, pal::LabelPosition* lp2 ) const
    {
      QgsLabelFeature* lf1 = lp1->getFeaturePart()->feature();
      QgsLabelFeature* lf2 = lp2->getFeaturePart()->feature();

      if ( !qgsDoubleNear( lf1->zIndex(), lf2->zIndex() ) )
        return lf1->zIndex() < lf2->zIndex();

      //equal z-index, so fallback to respecting layer render order
      int layer1Pos = mMapSettings.layers().indexOf( lf1->provider()->layerId() );
      int layer2Pos = mMapSettings.layers().indexOf( lf2->provider()->layerId() );
      if ( layer1Pos != layer2Pos && layer1Pos >= 0 && layer2Pos >= 0 )
        return layer1Pos > layer2Pos; //higher positions are rendered first

      //same layer, so render larger labels first
      return lf1->size().width() * lf1->size().height() > lf2->size().width() * lf2->size().height();
    }

  private:

    const QgsMapSettings& mMapSettings;
};


QgsLabelingEngineV2::QgsLabelingEngineV2()
    : mFlags( RenderOutlineLabels | UsePartialCandidates )
    , mSearchMethod( QgsPalLabeling::Chain )
    , mCandPoint( 8 )
    , mCandLine( 8 )
    , mCandPolygon( 8 )
    , mResults( nullptr )
{
  mResults = new QgsLabelingResults;
}

QgsLabelingEngineV2::~QgsLabelingEngineV2()
{
  delete mResults;
  qDeleteAll( mProviders );
  qDeleteAll( mSubProviders );
}

void QgsLabelingEngineV2::addProvider( QgsAbstractLabelProvider* provider )
{
  provider->setEngine( this );
  mProviders << provider;
}

void QgsLabelingEngineV2::removeProvider( QgsAbstractLabelProvider* provider )
{
  int idx = mProviders.indexOf( provider );
  if ( idx >= 0 )
  {
    delete mProviders.takeAt( idx );
  }
}

void QgsLabelingEngineV2::processProvider( QgsAbstractLabelProvider* provider, QgsRenderContext& context, pal::Pal& p )
{
  QgsAbstractLabelProvider::Flags flags = provider->flags();

  // create the pal layer
  pal::Layer* l = p.addLayer( provider,
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

  // set whether labels must fall completely within the polygon
  l->setFitInPolygonOnly( flags.testFlag( QgsAbstractLabelProvider::FitInPolygonOnly ) );

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
      Q_ASSERT( "unsupported upside-down label setting" && 0 );
      return;
  }
  l->setUpsidedownLabels( upsdnlabels );


  QList<QgsLabelFeature*> features = provider->labelFeatures( context );

  Q_FOREACH ( QgsLabelFeature* feature, features )
  {
    try
    {
      l->registerFeature( feature );
    }
    catch ( std::exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsgLevel( QString( "Ignoring feature %1 due PAL exception:" ).arg( feature->id() ) + QString::fromLatin1( e.what() ), 4 );
      continue;
    }
  }

  // any sub-providers?
  Q_FOREACH ( QgsAbstractLabelProvider* subProvider, provider->subProviders() )
  {
    mSubProviders << subProvider;
    processProvider( subProvider, context, p );
  }
}


void QgsLabelingEngineV2::run( QgsRenderContext& context )
{
  pal::Pal p;

  pal::SearchMethod s;
  switch ( mSearchMethod )
  {
    default:
    case QgsPalLabeling::Chain:
      s = pal::CHAIN;
      break;
    case QgsPalLabeling::Popmusic_Tabu:
      s = pal::POPMUSIC_TABU;
      break;
    case QgsPalLabeling::Popmusic_Chain:
      s = pal::POPMUSIC_CHAIN;
      break;
    case QgsPalLabeling::Popmusic_Tabu_Chain:
      s = pal::POPMUSIC_TABU_CHAIN;
      break;
    case QgsPalLabeling::Falp:
      s = pal::FALP;
      break;
  }
  p.setSearch( s );

  // set number of candidates generated per feature
  p.setPointP( mCandPoint );
  p.setLineP( mCandLine );
  p.setPolyP( mCandPolygon );

  p.setShowPartial( mFlags.testFlag( UsePartialCandidates ) );


  // for each provider: get labels and register them in PAL
  Q_FOREACH ( QgsAbstractLabelProvider* provider, mProviders )
  {
    processProvider( provider, context, p );
  }


  // NOW DO THE LAYOUT (from QgsPalLabeling::drawLabeling)

  QPainter* painter = context.painter();

  QgsGeometry* extentGeom( QgsGeometry::fromRect( mMapSettings.visibleExtent() ) );
  if ( !qgsDoubleNear( mMapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom->rotate( -mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
  }

  QgsRectangle extent = extentGeom->boundingBox();
  delete extentGeom;

  p.registerCancellationCallback( &_palIsCancelled, reinterpret_cast< void* >( &context ) );

  QTime t;
  t.start();

  // do the labeling itself
  double bbox[] = { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() };

  QList<pal::LabelPosition*>* labels;
  pal::Problem *problem;
  try
  {
    problem = p.extractProblem( bbox );
  }
  catch ( std::exception& e )
  {
    Q_UNUSED( e );
    QgsDebugMsgLevel( "PAL EXCEPTION :-( " + QString::fromLatin1( e.what() ), 4 );
    return;
  }


  if ( context.renderingStopped() )
  {
    delete problem;
    return; // it has been cancelled
  }

#if 1 // XXX strk
  // features are pre-rotated but not scaled/translated,
  // so we only disable rotation here. Ideally, they'd be
  // also pre-scaled/translated, as suggested here:
  // http://hub.qgis.org/issues/11856
  QgsMapToPixel xform = mMapSettings.mapToPixel();
  xform.setMapRotation( 0, 0, 0 );
#else
  const QgsMapToPixel& xform = mMapSettings->mapToPixel();
#endif

  // draw rectangles with all candidates
  // this is done before actual solution of the problem
  // before number of candidates gets reduced
  // TODO mCandidates.clear();
  if ( mFlags.testFlag( DrawCandidates ) && problem )
  {
    painter->setBrush( Qt::NoBrush );
    for ( int i = 0; i < problem->getNumFeatures(); i++ )
    {
      for ( int j = 0; j < problem->getFeatureCandidateCount( i ); j++ )
      {
        pal::LabelPosition* lp = problem->getFeatureCandidate( i, j );

        QgsPalLabeling::drawLabelCandidateRect( lp, painter, &xform );
      }
    }
  }

  // find the solution
  labels = p.solveProblem( problem, mFlags.testFlag( UseAllLabels ) );

  QgsDebugMsgLevel( QString( "LABELING work:  %1 ms ... labels# %2" ).arg( t.elapsed() ).arg( labels->size() ), 4 );
  t.restart();

  if ( context.renderingStopped() )
  {
    delete problem;
    delete labels;
    return;
  }
  painter->setRenderHint( QPainter::Antialiasing );

  // sort labels
  qSort( labels->begin(), labels->end(), QgsLabelSorter( mMapSettings ) );

  // draw the labels
  QList<pal::LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it )
  {
    if ( context.renderingStopped() )
      break;

    QgsLabelFeature* lf = ( *it )->getFeaturePart()->feature();
    if ( !lf )
    {
      continue;
    }

    lf->provider()->drawLabel( context, *it );
  }

  // Reset composition mode for further drawing operations
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  QgsDebugMsgLevel( QString( "LABELING draw:  %1 ms" ).arg( t.elapsed() ), 4 );

  delete problem;
  delete labels;


}

QgsLabelingResults* QgsLabelingEngineV2::takeResults()
{
  QgsLabelingResults* res = mResults;
  mResults = nullptr;
  return res;
}


void QgsLabelingEngineV2::readSettingsFromProject()
{
  bool saved = false;
  QgsProject* prj = QgsProject::instance();
  mSearchMethod = static_cast< QgsPalLabeling::Search >( prj->readNumEntry( "PAL", "/SearchMethod", static_cast< int >( QgsPalLabeling::Chain ), &saved ) );
  mCandPoint = prj->readNumEntry( "PAL", "/CandidatesPoint", 8, &saved );
  mCandLine = prj->readNumEntry( "PAL", "/CandidatesLine", 8, &saved );
  mCandPolygon = prj->readNumEntry( "PAL", "/CandidatesPolygon", 8, &saved );

  mFlags = nullptr;
  if ( prj->readBoolEntry( "PAL", "/ShowingCandidates", false, &saved ) ) mFlags |= DrawCandidates;
  if ( prj->readBoolEntry( "PAL", "/DrawRectOnly", false, &saved ) ) mFlags |= DrawLabelRectOnly;
  if ( prj->readBoolEntry( "PAL", "/ShowingShadowRects", false, &saved ) ) mFlags |= DrawShadowRects;
  if ( prj->readBoolEntry( "PAL", "/ShowingAllLabels", false, &saved ) ) mFlags |= UseAllLabels;
  if ( prj->readBoolEntry( "PAL", "/ShowingPartialsLabels", true, &saved ) ) mFlags |= UsePartialCandidates;
  if ( prj->readBoolEntry( "PAL", "/DrawOutlineLabels", true, &saved ) ) mFlags |= RenderOutlineLabels;
}

void QgsLabelingEngineV2::writeSettingsToProject()
{
  QgsProject::instance()->writeEntry( "PAL", "/SearchMethod", static_cast< int >( mSearchMethod ) );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPoint", mCandPoint );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesLine", mCandLine );
  QgsProject::instance()->writeEntry( "PAL", "/CandidatesPolygon", mCandPolygon );

  QgsProject::instance()->writeEntry( "PAL", "/ShowingCandidates", mFlags.testFlag( DrawCandidates ) );
  QgsProject::instance()->writeEntry( "PAL", "/DrawRectOnly", mFlags.testFlag( DrawLabelRectOnly ) );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingShadowRects", mFlags.testFlag( DrawShadowRects ) );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingAllLabels", mFlags.testFlag( UseAllLabels ) );
  QgsProject::instance()->writeEntry( "PAL", "/ShowingPartialsLabels", mFlags.testFlag( UsePartialCandidates ) );
  QgsProject::instance()->writeEntry( "PAL", "/DrawOutlineLabels", mFlags.testFlag( RenderOutlineLabels ) );
}



////

QgsAbstractLabelProvider*QgsLabelFeature::provider() const
{
  return mLayer ? mLayer->provider() : nullptr;

}

QgsAbstractLabelProvider::QgsAbstractLabelProvider( const QString& layerId, const QString& providerId )
    : mEngine( nullptr )
    , mLayerId( layerId )
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

QString QgsLabelingUtils::encodePredefinedPositionOrder( const QVector<QgsPalLayerSettings::PredefinedPointPosition>& positions )
{
  QStringList predefinedOrderString;
  Q_FOREACH ( QgsPalLayerSettings::PredefinedPointPosition position, positions )
  {
    switch ( position )
    {
      case QgsPalLayerSettings::TopLeft:
        predefinedOrderString << "TL";
        break;
      case QgsPalLayerSettings::TopSlightlyLeft:
        predefinedOrderString << "TSL";
        break;
      case QgsPalLayerSettings::TopMiddle:
        predefinedOrderString << "T";
        break;
      case QgsPalLayerSettings::TopSlightlyRight:
        predefinedOrderString << "TSR";
        break;
      case QgsPalLayerSettings::TopRight:
        predefinedOrderString << "TR";
        break;
      case QgsPalLayerSettings::MiddleLeft:
        predefinedOrderString << "L";
        break;
      case QgsPalLayerSettings::MiddleRight:
        predefinedOrderString << "R";
        break;
      case QgsPalLayerSettings::BottomLeft:
        predefinedOrderString << "BL";
        break;
      case QgsPalLayerSettings::BottomSlightlyLeft:
        predefinedOrderString << "BSL";
        break;
      case QgsPalLayerSettings::BottomMiddle:
        predefinedOrderString << "B";
        break;
      case QgsPalLayerSettings::BottomSlightlyRight:
        predefinedOrderString << "BSR";
        break;
      case QgsPalLayerSettings::BottomRight:
        predefinedOrderString << "BR";
        break;
    }
  }
  return predefinedOrderString.join( "," );
}

QVector<QgsPalLayerSettings::PredefinedPointPosition> QgsLabelingUtils::decodePredefinedPositionOrder( const QString& positionString )
{
  QVector<QgsPalLayerSettings::PredefinedPointPosition> result;
  QStringList predefinedOrderList = positionString.split( ',' );
  Q_FOREACH ( const QString& position, predefinedOrderList )
  {
    QString cleaned = position.trimmed().toUpper();
    if ( cleaned == "TL" )
      result << QgsPalLayerSettings::TopLeft;
    else if ( cleaned == "TSL" )
      result << QgsPalLayerSettings::TopSlightlyLeft;
    else if ( cleaned == "T" )
      result << QgsPalLayerSettings::TopMiddle;
    else if ( cleaned == "TSR" )
      result << QgsPalLayerSettings::TopSlightlyRight;
    else if ( cleaned == "TR" )
      result << QgsPalLayerSettings::TopRight;
    else if ( cleaned == "L" )
      result << QgsPalLayerSettings::MiddleLeft;
    else if ( cleaned == "R" )
      result << QgsPalLayerSettings::MiddleRight;
    else if ( cleaned == "BL" )
      result << QgsPalLayerSettings::BottomLeft;
    else if ( cleaned == "BSL" )
      result << QgsPalLayerSettings::BottomSlightlyLeft;
    else if ( cleaned == "B" )
      result << QgsPalLayerSettings::BottomMiddle;
    else if ( cleaned == "BSR" )
      result << QgsPalLayerSettings::BottomSlightlyRight;
    else if ( cleaned == "BR" )
      result << QgsPalLayerSettings::BottomRight;
  }
  return result;
}
