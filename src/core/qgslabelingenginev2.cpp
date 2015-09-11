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
#include "qgspalgeometry.h"

#include "feature.h"
#include "labelposition.h"
#include "layer.h"
#include "pal.h"
#include "problem.h"



// helper function for checking for job cancellation within PAL
static bool _palIsCancelled( void* ctx )
{
  return (( QgsRenderContext* ) ctx )->renderingStopped();
}


QgsLabelingEngineV2::QgsLabelingEngineV2( const QgsMapSettings& mapSettings )
    : mMapSettings( mapSettings )
    , mFlags( RenderOutlineLabels | UsePartialCandidates )
    , mSearchMethod( QgsPalLabeling::Chain )
    , mCandPoint( 8 )
    , mCandLine( 8 )
    , mCandPolygon( 8 )
    , mResults( 0 )
{
  mResults = new QgsLabelingResults;
}

QgsLabelingEngineV2::~QgsLabelingEngineV2()
{
  delete mResults;
  qDeleteAll( mProviders );
}

void QgsLabelingEngineV2::addProvider( QgsAbstractLabelProvider* provider )
{
  provider->setEngine( this );
  mProviders << provider;
}

void QgsLabelingEngineV2::run( QgsRenderContext& context )
{
  pal::Pal p;

  SearchMethod s;
  switch ( mSearchMethod )
  {
    default:
    case QgsPalLabeling::Chain: s = CHAIN; break;
    case QgsPalLabeling::Popmusic_Tabu: s = POPMUSIC_TABU; break;
    case QgsPalLabeling::Popmusic_Chain: s = POPMUSIC_CHAIN; break;
    case QgsPalLabeling::Popmusic_Tabu_Chain: s = POPMUSIC_TABU_CHAIN; break;
    case QgsPalLabeling::Falp: s = FALP; break;
  }
  p.setSearch( s );

  // set number of candidates generated per feature
  p.setPointP( mCandPoint );
  p.setLineP( mCandLine );
  p.setPolyP( mCandPolygon );

  p.setShowPartial( mFlags.testFlag( UsePartialCandidates ) );


  // for each provider: get labels and register them in PAL
  foreach ( QgsAbstractLabelProvider* provider, mProviders )
  {
    // how to place the labels
    pal::Arrangement arrangement;
    switch ( provider->placement() )
    {
      case QgsPalLayerSettings::AroundPoint: arrangement = pal::P_POINT; break;
      case QgsPalLayerSettings::OverPoint:   arrangement = pal::P_POINT_OVER; break;
      case QgsPalLayerSettings::Line:        arrangement = pal::P_LINE; break;
      case QgsPalLayerSettings::Curved:      arrangement = pal::P_CURVED; break;
      case QgsPalLayerSettings::Horizontal:  arrangement = pal::P_HORIZ; break;
      case QgsPalLayerSettings::Free:        arrangement = pal::P_FREE; break;
      default: Q_ASSERT( "unsupported placement" && 0 ); return;
    }

    QgsAbstractLabelProvider::Flags flags = provider->flags();

    // create the pal layer
    pal::Layer* l = p.addLayer( provider->id(),
                                arrangement,
                                provider->priority(),
                                flags.testFlag( QgsAbstractLabelProvider::GeometriesAreObstacles ),
                                true,
                                flags.testFlag( QgsAbstractLabelProvider::DrawLabels ),
                                flags.testFlag( QgsAbstractLabelProvider::DrawAllLabels ) );

    // extra flags for placement of labels for linestrings
    l->setArrangementFlags(( LineArrangementFlags ) provider->linePlacementFlags() );

    // set label mode (label per feature is the default)
    l->setLabelMode( flags.testFlag( QgsAbstractLabelProvider::LabelPerFeaturePart ) ? pal::Layer::LabelPerFeaturePart : pal::Layer::LabelPerFeature );

    // set whether adjacent lines should be merged
    l->setMergeConnectedLines( flags.testFlag( QgsAbstractLabelProvider::MergeConnectedLines ) );

    // set obstacle type
    switch ( provider->obstacleType() )
    {
      case QgsPalLayerSettings::PolygonInterior:
        l->setObstacleType( pal::PolygonInterior );
        break;
      case QgsPalLayerSettings::PolygonBoundary:
        l->setObstacleType( pal::PolygonBoundary );
        break;
    }

    // set whether location of centroid must be inside of polygons
    l->setCentroidInside( flags.testFlag( QgsAbstractLabelProvider::CentroidMustBeInside ) );

    // set whether labels must fall completely within the polygon
    l->setFitInPolygonOnly( flags.testFlag( QgsAbstractLabelProvider::FitInPolygonOnly ) );

    // set how to show upside-down labels
    pal::Layer::UpsideDownLabels upsdnlabels;
    switch ( provider->upsidedownLabels() )
    {
      case QgsPalLayerSettings::Upright:     upsdnlabels = pal::Layer::Upright; break;
      case QgsPalLayerSettings::ShowDefined: upsdnlabels = pal::Layer::ShowDefined; break;
      case QgsPalLayerSettings::ShowAll:     upsdnlabels = pal::Layer::ShowAll; break;
      default: Q_ASSERT( "unsupported upside-down label setting" && 0 ); return;
    }
    l->setUpsidedownLabels( upsdnlabels );


    QList<QgsLabelFeature*> features = provider->labelFeatures( mMapSettings, context );

    foreach ( QgsLabelFeature* feature, features )
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
  }


  // NOW DO THE LAYOUT (from QgsPalLabeling::drawLabeling)

  QPainter* painter = const_cast<QgsRenderContext&>( context ).painter();

  QgsGeometry* extentGeom( QgsGeometry::fromRect( mMapSettings.visibleExtent() ) );
  if ( !qgsDoubleNear( mMapSettings.rotation(), 0.0 ) )
  {
    //PAL features are prerotated, so extent also needs to be unrotated
    extentGeom->rotate( -mMapSettings.rotation(), mMapSettings.visibleExtent().center() );
  }

  QgsRectangle extent = extentGeom->boundingBox();
  delete extentGeom;

  p.registerCancellationCallback( &_palIsCancelled, ( void* ) &context );

  QTime t;
  t.start();

  // do the labeling itself
  double bbox[] = { extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() };

  std::list<pal::LabelPosition*>* labels;
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

  // draw the labels
  std::list<pal::LabelPosition*>::iterator it = labels->begin();
  for ( ; it != labels->end(); ++it )
  {
    if ( context.renderingStopped() )
      break;

    QgsPalGeometry* palGeometry = dynamic_cast< QgsPalGeometry* >(( *it )->getFeaturePart()->getUserGeometry() );
    if ( !palGeometry )
    {
      continue;
    }

    //layer names
    QString layerName = ( *it )->getLayerName();

    providerById( layerName )->drawLabel( context, *it );
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
  mResults = 0;
  return res;
}

QgsAbstractLabelProvider* QgsLabelingEngineV2::providerById( const QString& id )
{
  Q_FOREACH ( QgsAbstractLabelProvider* provider, mProviders )
  {
    if ( provider->id() == id )
      return provider;
  }
  return 0;
}



////



QgsLabelFeature::QgsLabelFeature( QString id, QgsPalGeometry* geometry, const QSizeF& size )
    : mId( id )
    , mGeometry( geometry )
    , mSize( size )
    , mPriority( -1 )
    , mHasFixedPosition( false )
    , mHasFixedAngle( false )
    , mHasFixedQuadrant( false )
    , mDistLabel( 0 )
    , mRepeatDistance( 0 )
    , mAlwaysShow( false )
    , mIsObstacle( false )
    , mObstacleFactor( 1 )
{
}

QgsAbstractLabelProvider::QgsAbstractLabelProvider()
    : mEngine( 0 )
    , mFlags( DrawLabels )
    , mPlacement( QgsPalLayerSettings::AroundPoint )
    , mLinePlacementFlags( 0 )
    , mPriority( 0.5 )
    , mObstacleType( QgsPalLayerSettings::PolygonInterior )
    , mUpsidedownLabels( QgsPalLayerSettings::Upright )
{
}
