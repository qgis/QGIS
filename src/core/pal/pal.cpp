/*
 *    libpal - Automated Placement of Labels Library
 *
 *    Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                             University of Applied Sciences, Western Switzerland
 *                             http://www.hes-so.ch
 *
 *    Contact:
 *        maxence.laurent <at> heig-vd <dot> ch
 *     or
 *        eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "qgsgeometry.h"
#include "pal.h"
#include "layer.h"
#include "palexception.h"
#include "palstat.h"
#include "rtree.hpp"
#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "pointset.h"
#include "internalexception.h"
#include "util.h"
#include <cfloat>

using namespace pal;

Pal::Pal()
{
  // do not init and exit GEOS - we do it inside QGIS
  //initGEOS( geosNotice, geosError );
}

void Pal::removeLayer( Layer *layer )
{
  if ( !layer )
    return;

  mMutex.lock();
  if ( QgsAbstractLabelProvider *key = mLayers.key( layer, nullptr ) )
  {
    mLayers.remove( key );
    delete layer;
  }
  mMutex.unlock();
}

Pal::~Pal()
{

  mMutex.lock();

  qDeleteAll( mLayers );
  mLayers.clear();
  mMutex.unlock();

  // do not init and exit GEOS - we do it inside QGIS
  //finishGEOS();
}

Layer *Pal::addLayer( QgsAbstractLabelProvider *provider, const QString &layerName, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, bool displayAll )
{
  mMutex.lock();

  Q_ASSERT( !mLayers.contains( provider ) );

  Layer *layer = new Layer( provider, layerName, arrangement, defaultPriority, active, toLabel, this, displayAll );
  mLayers.insert( provider, layer );
  mMutex.unlock();

  return layer;
}

typedef struct _featCbackCtx
{
  Layer *layer = nullptr;
  QLinkedList<Feats *> *fFeats;
  RTree<FeaturePart *, double, 2, double> *obstacles;
  RTree<LabelPosition *, double, 2, double> *candidates;
  QList<LabelPosition *> *positionsWithNoCandidates;
  const GEOSPreparedGeometry *mapBoundary = nullptr;
  Pal *pal = nullptr;
} FeatCallBackCtx;



/*
 * Callback function
 *
 * Extract a specific shape from indexes
 */
bool extractFeatCallback( FeaturePart *featurePart, void *ctx )
{
  double amin[2], amax[2];
  FeatCallBackCtx *context = reinterpret_cast< FeatCallBackCtx * >( ctx );

  // Holes of the feature are obstacles
  for ( int i = 0; i < featurePart->getNumSelfObstacles(); i++ )
  {
    featurePart->getSelfObstacle( i )->getBoundingBox( amin, amax );
    context->obstacles->Insert( amin, amax, featurePart->getSelfObstacle( i ) );

    if ( !featurePart->getSelfObstacle( i )->getHoleOf() )
    {
      //ERROR: SHOULD HAVE A PARENT!!!!!
    }
  }

  // generate candidates for the feature part
  QList< LabelPosition * > candidates = featurePart->createCandidates();

  // purge candidates that are outside the bbox

  QMutableListIterator< LabelPosition *> i( candidates );
  while ( i.hasNext() )
  {
    LabelPosition *pos = i.next();
    bool outside = false;

    if ( context->pal->getShowPartial() )
      outside = !pos->intersects( context->mapBoundary );
    else
      outside = !pos->within( context->mapBoundary );
    if ( outside )
    {
      i.remove();
      delete pos;
    }
    else   // this one is OK
    {
      pos->insertIntoIndex( context->candidates );
    }
  }

  if ( !candidates.empty() )
  {
    std::sort( candidates.begin(), candidates.end(), CostCalculator::candidateSortGrow );

    // valid features are added to fFeats
    Feats *ft = new Feats();
    ft->feature = featurePart;
    ft->shape = nullptr;
    ft->candidates = candidates;
    ft->priority = featurePart->calculatePriority();
    context->fFeats->append( ft );
  }
  else
  {
    // features with no candidates are recorded in the unlabeled feature list
    std::unique_ptr< LabelPosition > unplacedPosition = featurePart->createCandidatePointOnSurface( featurePart );
    if ( unplacedPosition )
      context->positionsWithNoCandidates->append( unplacedPosition.release() );
  }

  return true;
}

typedef struct _obstaclebackCtx
{
  RTree<FeaturePart *, double, 2, double> *obstacles;
  int obstacleCount;
} ObstacleCallBackCtx;

/*
 * Callback function
 *
 * Extract obstacles from indexes
 */
bool extractObstaclesCallback( FeaturePart *ft_ptr, void *ctx )
{
  double amin[2], amax[2];
  ObstacleCallBackCtx *context = reinterpret_cast< ObstacleCallBackCtx * >( ctx );

  // insert into obstacles
  ft_ptr->getBoundingBox( amin, amax );
  context->obstacles->Insert( amin, amax, ft_ptr );
  context->obstacleCount++;
  return true;
}

typedef struct _filterContext
{
  RTree<LabelPosition *, double, 2, double> *cdtsIndex;
  Pal *pal = nullptr;
} FilterContext;

bool filteringCallback( FeaturePart *featurePart, void *ctx )
{

  RTree<LabelPosition *, double, 2, double> *cdtsIndex = ( reinterpret_cast< FilterContext * >( ctx ) )->cdtsIndex;
  Pal *pal = ( reinterpret_cast< FilterContext * >( ctx ) )->pal;

  if ( pal->isCanceled() )
    return false; // do not continue searching

  double amin[2], amax[2];
  featurePart->getBoundingBox( amin, amax );

  LabelPosition::PruneCtx pruneContext;
  pruneContext.obstacle = featurePart;
  pruneContext.pal = pal;
  cdtsIndex->Search( amin, amax, LabelPosition::pruneCallback, static_cast< void * >( &pruneContext ) );

  return true;
}

std::unique_ptr<Problem> Pal::extract( const QgsRectangle &extent, const QgsGeometry &mapBoundary )
{
  // to store obstacles
  RTree<FeaturePart *, double, 2, double> obstacles;
  std::unique_ptr< Problem > prob = qgis::make_unique< Problem >();

  int i, j;

  double bbx[4];
  double bby[4];

  double amin[2];
  double amax[2];

  int max_p = 0;

  LabelPosition *lp = nullptr;

  bbx[0] = bbx[3] = amin[0] = prob->bbox[0] = extent.xMinimum();
  bby[0] = bby[1] = amin[1] = prob->bbox[1] = extent.yMinimum();
  bbx[1] = bbx[2] = amax[0] = prob->bbox[2] = extent.xMaximum();
  bby[2] = bby[3] = amax[1] = prob->bbox[3] = extent.yMaximum();

  prob->pal = this;

  QLinkedList<Feats *> fFeats;

  FeatCallBackCtx context;

  // prepare map boundary
  geos::unique_ptr mapBoundaryGeos( QgsGeos::asGeos( mapBoundary ) );
  geos::prepared_unique_ptr mapBoundaryPrepared( GEOSPrepare_r( QgsGeos::getGEOSHandler(), mapBoundaryGeos.get() ) );

  context.fFeats = &fFeats;
  context.obstacles = &obstacles;
  context.candidates = prob->candidates;
  context.positionsWithNoCandidates = prob->positionsWithNoCandidates();
  context.mapBoundary = mapBoundaryPrepared.get();
  context.pal = this;

  ObstacleCallBackCtx obstacleContext;
  obstacleContext.obstacles = &obstacles;
  obstacleContext.obstacleCount = 0;

  // first step : extract features from layers

  int previousFeatureCount = 0;
  int previousObstacleCount = 0;

  QStringList layersWithFeaturesInBBox;

  mMutex.lock();
  const auto constMLayers = mLayers;
  for ( Layer *layer : constMLayers )
  {
    if ( !layer )
    {
      // invalid layer name
      continue;
    }

    // only select those who are active
    if ( !layer->active() )
      continue;

    // check for connected features with the same label text and join them
    if ( layer->mergeConnectedLines() )
      layer->joinConnectedFeatures();

    layer->chopFeaturesAtRepeatDistance();

    layer->mMutex.lock();

    // find features within bounding box and generate candidates list
    context.layer = layer;
    layer->mFeatureIndex->Search( amin, amax, extractFeatCallback, static_cast< void * >( &context ) );
    // find obstacles within bounding box
    layer->mObstacleIndex->Search( amin, amax, extractObstaclesCallback, static_cast< void * >( &obstacleContext ) );

    layer->mMutex.unlock();

    if ( context.fFeats->size() - previousFeatureCount > 0 || obstacleContext.obstacleCount > previousObstacleCount )
    {
      layersWithFeaturesInBBox << layer->name();
    }
    previousFeatureCount = context.fFeats->size();
    previousObstacleCount = obstacleContext.obstacleCount;
  }
  mMutex.unlock();

  prob->mLayerCount = layersWithFeaturesInBBox.size();
  prob->labelledLayersName = layersWithFeaturesInBBox;

  prob->mFeatureCount = fFeats.size();
  prob->mTotalCandidates = 0;
  prob->featNbLp = new int [prob->mFeatureCount];
  prob->featStartId = new int [prob->mFeatureCount];
  prob->inactiveCost = new double[prob->mFeatureCount];


  if ( !fFeats.isEmpty() )
  {
    Feats *feat = nullptr;

    // Filtering label positions against obstacles
    amin[0] = amin[1] = std::numeric_limits<double>::lowest();
    amax[0] = amax[1] = std::numeric_limits<double>::max();
    FilterContext filterCtx;
    filterCtx.cdtsIndex = prob->candidates;
    filterCtx.pal = this;
    obstacles.Search( amin, amax, filteringCallback, static_cast< void * >( &filterCtx ) );

    if ( isCanceled() )
    {
      for ( Feats *feat : qgis::as_const( fFeats ) )
      {
        qDeleteAll( feat->candidates );
        feat->candidates.clear();
      }

      qDeleteAll( fFeats );
      return nullptr;
    }

    int idlp = 0;
    for ( i = 0; i < prob->mFeatureCount; i++ ) /* foreach feature into prob */
    {
      feat = fFeats.takeFirst();

      prob->featStartId[i] = idlp;
      prob->inactiveCost[i] = std::pow( 2, 10 - 10 * feat->priority );

      switch ( feat->feature->getGeosType() )
      {
        case GEOS_POINT:
          max_p = feat->feature->layer()->maximumPointLabelCandidates();
          break;
        case GEOS_LINESTRING:
          max_p = feat->feature->layer()->maximumLineLabelCandidates();
          break;
        case GEOS_POLYGON:
          max_p = feat->feature->layer()->maximumPolygonLabelCandidates();
          break;
      }

      // sort candidates by cost, skip less interesting ones, calculate polygon costs (if using polygons)
      max_p = CostCalculator::finalizeCandidatesCosts( feat, max_p, &obstacles, bbx, bby );

      // only keep the 'max_p' best candidates
      while ( feat->candidates.count() > max_p )
      {
        // TODO remove from index
        feat->candidates.constLast()->removeFromIndex( prob->candidates );
        delete feat->candidates.takeLast();
      }

      // update problem's # candidate
      prob->featNbLp[i] = feat->candidates.count();
      prob->mTotalCandidates += feat->candidates.count();

      // add all candidates into a rtree (to speed up conflicts searching)
      for ( j = 0; j < feat->candidates.count(); j++, idlp++ )
      {
        lp = feat->candidates.at( j );
        //lp->insertIntoIndex(prob->candidates);
        lp->setProblemIds( i, idlp ); // bugfix #1 (maxence 10/23/2008)
      }
      fFeats.append( feat );
    }

    int nbOverlaps = 0;

    while ( !fFeats.isEmpty() ) // foreach feature
    {
      if ( isCanceled() )
      {
        for ( Feats *feat : qgis::as_const( fFeats ) )
        {
          qDeleteAll( feat->candidates );
          feat->candidates.clear();
        }

        qDeleteAll( fFeats );
        return nullptr;
      }

      feat = fFeats.takeFirst();
      while ( !feat->candidates.isEmpty() ) // foreach label candidate
      {
        lp = feat->candidates.takeFirst();
        lp->resetNumOverlaps();

        // make sure that candidate's cost is less than 1
        lp->validateCost();

        prob->addCandidatePosition( lp );
        //prob->feat[idlp] = j;

        lp->getBoundingBox( amin, amax );

        // lookup for overlapping candidate
        prob->candidates->Search( amin, amax, LabelPosition::countOverlapCallback, static_cast< void * >( lp ) );

        nbOverlaps += lp->getNumOverlaps();
      }
      delete feat;
    }
    nbOverlaps /= 2;
    prob->all_nblp = prob->mTotalCandidates;
    prob->nbOverlap = nbOverlaps;
  }

  return prob;
}

void Pal::registerCancellationCallback( Pal::FnIsCanceled fnCanceled, void *context )
{
  fnIsCanceled = fnCanceled;
  fnIsCanceledContext = context;
}

std::unique_ptr<Problem> Pal::extractProblem( const QgsRectangle &extent, const QgsGeometry &mapBoundary )
{
  return extract( extent, mapBoundary );
}

QList<LabelPosition *> Pal::solveProblem( Problem *prob, bool displayAll, QList<LabelPosition *> *unlabeled )
{
  if ( !prob )
    return QList<LabelPosition *>();

  prob->reduce();

  try
  {
    prob->chain_search();
  }
  catch ( InternalException::Empty & )
  {
    return QList<LabelPosition *>();
  }

  return prob->getSolution( displayAll, unlabeled );
}


void Pal::setPointP( int point_p )
{
  if ( point_p > 0 )
    this->point_p = point_p;
}

void Pal::setLineP( int line_p )
{
  if ( line_p > 0 )
    this->line_p = line_p;
}

void Pal::setPolyP( int poly_p )
{
  if ( poly_p > 0 )
    this->poly_p = poly_p;
}


void Pal::setMinIt( int min_it )
{
  if ( min_it >= 0 )
    tabuMinIt = min_it;
}

void Pal::setMaxIt( int max_it )
{
  if ( max_it > 0 )
    tabuMaxIt = max_it;
}

void Pal::setPopmusicR( int r )
{
  if ( r > 0 )
    popmusic_r = r;
}

void Pal::setEjChainDeg( int degree )
{
  this->ejChainDeg = degree;
}

void Pal::setTenure( int tenure )
{
  this->tenure = tenure;
}

void Pal::setCandListSize( double fact )
{
  this->candListSize = fact;
}

void Pal::setShowPartial( bool show )
{
  this->showPartial = show;
}

int Pal::getPointP()
{
  return point_p;
}

int Pal::getLineP()
{
  return line_p;
}

int Pal::getPolyP()
{
  return poly_p;
}

QgsLabelingEngineSettings::PlacementEngineVersion Pal::placementVersion() const
{
  return mPlacementVersion;
}

void Pal::setPlacementVersion( QgsLabelingEngineSettings::PlacementEngineVersion placementVersion )
{
  mPlacementVersion = placementVersion;
}

int Pal::getMinIt()
{
  return tabuMaxIt;
}

int Pal::getMaxIt()
{
  return tabuMinIt;
}

bool Pal::getShowPartial()
{
  return showPartial;
}
