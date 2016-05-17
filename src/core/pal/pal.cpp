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

GEOSContextHandle_t pal::geosContext()
{
  return QgsGeometry::getGEOSHandler();
}

Pal::Pal()
{
  // do not init and exit GEOS - we do it inside QGIS
  //initGEOS( geosNotice, geosError );

  fnIsCancelled = nullptr;
  fnIsCancelledContext = nullptr;

  ejChainDeg = 50;
  tenure = 10;
  candListSize = 0.2;

  tabuMinIt = 3;
  tabuMaxIt = 4;
  searchMethod = POPMUSIC_CHAIN;
  popmusic_r = 30;

  searchMethod = CHAIN;

  setSearch( CHAIN );

  point_p = 8;
  line_p = 8;
  poly_p = 8;

  showPartial = true;
}

void Pal::removeLayer( Layer *layer )
{
  if ( !layer )
    return;

  mMutex.lock();
  if ( QgsAbstractLabelProvider* key = mLayers.key( layer, nullptr ) )
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

Layer* Pal::addLayer( QgsAbstractLabelProvider* provider, const QString& layerName, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, bool displayAll )
{
  mMutex.lock();

  Q_ASSERT( !mLayers.contains( provider ) );

  Layer* layer = new Layer( provider, layerName, arrangement, defaultPriority, active, toLabel, this, displayAll );
  mLayers.insert( provider, layer );
  mMutex.unlock();

  return layer;
}

typedef struct _featCbackCtx
{
  Layer *layer;
  QLinkedList<Feats*>* fFeats;
  RTree<FeaturePart*, double, 2, double> *obstacles;
  RTree<LabelPosition*, double, 2, double> *candidates;
  double bbox_min[2];
  double bbox_max[2];
} FeatCallBackCtx;



/*
 * Callback function
 *
 * Extract a specific shape from indexes
 */
bool extractFeatCallback( FeaturePart *ft_ptr, void *ctx )
{
  double amin[2], amax[2];
  FeatCallBackCtx *context = reinterpret_cast< FeatCallBackCtx* >( ctx );

  // Holes of the feature are obstacles
  for ( int i = 0; i < ft_ptr->getNumSelfObstacles(); i++ )
  {
    ft_ptr->getSelfObstacle( i )->getBoundingBox( amin, amax );
    context->obstacles->Insert( amin, amax, ft_ptr->getSelfObstacle( i ) );

    if ( !ft_ptr->getSelfObstacle( i )->getHoleOf() )
    {
      //ERROR: SHOULD HAVE A PARENT!!!!!
    }
  }

  // generate candidates for the feature part
  QList< LabelPosition* > lPos;
  if ( ft_ptr->createCandidates( lPos, context->bbox_min, context->bbox_max, ft_ptr, context->candidates ) )
  {
    // valid features are added to fFeats
    Feats *ft = new Feats();
    ft->feature = ft_ptr;
    ft->shape = nullptr;
    ft->lPos = lPos;
    ft->priority = ft_ptr->calculatePriority();
    context->fFeats->append( ft );
  }
  else
  {
    // Others are deleted
    qDeleteAll( lPos );
  }

  return true;
}

typedef struct _obstaclebackCtx
{
  RTree<FeaturePart*, double, 2, double> *obstacles;
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
  ObstacleCallBackCtx *context = reinterpret_cast< ObstacleCallBackCtx* >( ctx );

  // insert into obstacles
  ft_ptr->getBoundingBox( amin, amax );
  context->obstacles->Insert( amin, amax, ft_ptr );
  context->obstacleCount++;
  return true;
}

typedef struct _filterContext
{
  RTree<LabelPosition*, double, 2, double> *cdtsIndex;
  Pal* pal;
} FilterContext;

bool filteringCallback( FeaturePart *featurePart, void *ctx )
{

  RTree<LabelPosition*, double, 2, double> *cdtsIndex = ( reinterpret_cast< FilterContext* >( ctx ) )->cdtsIndex;
  Pal* pal = ( reinterpret_cast< FilterContext* >( ctx ) )->pal;

  if ( pal->isCancelled() )
    return false; // do not continue searching

  double amin[2], amax[2];
  featurePart->getBoundingBox( amin, amax );

  LabelPosition::PruneCtx pruneContext;
  pruneContext.obstacle = featurePart;
  pruneContext.pal = pal;
  cdtsIndex->Search( amin, amax, LabelPosition::pruneCallback, static_cast< void* >( &pruneContext ) );

  return true;
}

Problem* Pal::extract( double lambda_min, double phi_min, double lambda_max, double phi_max )
{
  // to store obstacles
  RTree<FeaturePart*, double, 2, double> *obstacles = new RTree<FeaturePart*, double, 2, double>();

  Problem *prob = new Problem();

  int i, j;

  double bbx[4];
  double bby[4];

  double amin[2];
  double amax[2];

  int max_p = 0;

  LabelPosition* lp;

  bbx[0] = bbx[3] = amin[0] = prob->bbox[0] = lambda_min;
  bby[0] = bby[1] = amin[1] = prob->bbox[1] = phi_min;
  bbx[1] = bbx[2] = amax[0] = prob->bbox[2] = lambda_max;
  bby[2] = bby[3] = amax[1] = prob->bbox[3] = phi_max;

  prob->pal = this;

  QLinkedList<Feats*> *fFeats = new QLinkedList<Feats*>;

  FeatCallBackCtx context;
  context.fFeats = fFeats;
  context.obstacles = obstacles;
  context.candidates = prob->candidates;
  context.bbox_min[0] = amin[0];
  context.bbox_min[1] = amin[1];
  context.bbox_max[0] = amax[0];
  context.bbox_max[1] = amax[1];

  ObstacleCallBackCtx obstacleContext;
  obstacleContext.obstacles = obstacles;
  obstacleContext.obstacleCount = 0;

  // first step : extract features from layers

  int previousFeatureCount = 0;
  int previousObstacleCount = 0;

  QStringList layersWithFeaturesInBBox;

  mMutex.lock();
  Q_FOREACH ( Layer* layer, mLayers )
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
    layer->mFeatureIndex->Search( amin, amax, extractFeatCallback, static_cast< void* >( &context ) );
    // find obstacles within bounding box
    layer->mObstacleIndex->Search( amin, amax, extractObstaclesCallback, static_cast< void* >( &obstacleContext ) );

    layer->mMutex.unlock();

    if ( context.fFeats->size() - previousFeatureCount > 0 || obstacleContext.obstacleCount > previousObstacleCount )
    {
      layersWithFeaturesInBBox << layer->name();
    }
    previousFeatureCount = context.fFeats->size();
    previousObstacleCount = obstacleContext.obstacleCount;
  }
  mMutex.unlock();

  prob->nbLabelledLayers = layersWithFeaturesInBBox.size();
  prob->labelledLayersName = layersWithFeaturesInBBox;

  if ( fFeats->isEmpty() )
  {
    delete fFeats;
    delete prob;
    delete obstacles;
    return nullptr;
  }

  prob->nbft = fFeats->size();
  prob->nblp = 0;
  prob->featNbLp = new int [prob->nbft];
  prob->featStartId = new int [prob->nbft];
  prob->inactiveCost = new double[prob->nbft];

  Feats *feat;

  // Filtering label positions against obstacles
  amin[0] = amin[1] = -DBL_MAX;
  amax[0] = amax[1] = DBL_MAX;
  FilterContext filterCtx;
  filterCtx.cdtsIndex = prob->candidates;
  filterCtx.pal = this;
  obstacles->Search( amin, amax, filteringCallback, static_cast< void* >( &filterCtx ) );

  if ( isCancelled() )
  {
    Q_FOREACH ( Feats* feat, *fFeats )
    {
      qDeleteAll( feat->lPos );
      feat->lPos.clear();
    }

    qDeleteAll( *fFeats );
    delete fFeats;
    delete prob;
    delete obstacles;
    return nullptr;
  }

  int idlp = 0;
  for ( i = 0; i < prob->nbft; i++ ) /* foreach feature into prob */
  {
    feat = fFeats->takeFirst();

    prob->featStartId[i] = idlp;
    prob->inactiveCost[i] = pow( 2, 10 - 10 * feat->priority );

    switch ( feat->feature->getGeosType() )
    {
      case GEOS_POINT:
        max_p = point_p;
        break;
      case GEOS_LINESTRING:
        max_p = line_p;
        break;
      case GEOS_POLYGON:
        max_p = poly_p;
        break;
    }

    // sort candidates by cost, skip less interesting ones, calculate polygon costs (if using polygons)
    max_p = CostCalculator::finalizeCandidatesCosts( feat, max_p, obstacles, bbx, bby );

    // only keep the 'max_p' best candidates
    while ( feat->lPos.count() > max_p )
    {
      // TODO remove from index
      feat->lPos.last()->removeFromIndex( prob->candidates );
      delete feat->lPos.takeLast();
    }

    // update problem's # candidate
    prob->featNbLp[i] = feat->lPos.count();
    prob->nblp += feat->lPos.count();

    // add all candidates into a rtree (to speed up conflicts searching)
    for ( j = 0; j < feat->lPos.count(); j++, idlp++ )
    {
      lp = feat->lPos.at( j );
      //lp->insertIntoIndex(prob->candidates);
      lp->setProblemIds( i, idlp ); // bugfix #1 (maxence 10/23/2008)
    }
    fFeats->append( feat );
  }

  int nbOverlaps = 0;

  while ( !fFeats->isEmpty() ) // foreach feature
  {
    if ( isCancelled() )
    {
      Q_FOREACH ( Feats* feat, *fFeats )
      {
        qDeleteAll( feat->lPos );
        feat->lPos.clear();
      }

      qDeleteAll( *fFeats );
      delete fFeats;
      delete prob;
      delete obstacles;
      return nullptr;
    }

    feat = fFeats->takeFirst();
    while ( !feat->lPos.isEmpty() ) // foreach label candidate
    {
      lp = feat->lPos.takeFirst();
      lp->resetNumOverlaps();

      // make sure that candidate's cost is less than 1
      lp->validateCost();

      prob->addCandidatePosition( lp );
      //prob->feat[idlp] = j;

      lp->getBoundingBox( amin, amax );

      // lookup for overlapping candidate
      prob->candidates->Search( amin, amax, LabelPosition::countOverlapCallback, static_cast< void* >( lp ) );

      nbOverlaps += lp->getNumOverlaps();
    }
    delete feat;
  }
  delete fFeats;

  //delete candidates;
  delete obstacles;

  nbOverlaps /= 2;
  prob->all_nblp = prob->nblp;
  prob->nbOverlap = nbOverlaps;

  return prob;
}

/*
 * BIG MACHINE
 */
QList<LabelPosition*>* Pal::labeller( double bbox[4], PalStat **stats, bool displayAll )
{
  Problem *prob;

  SearchMethod old_searchMethod = searchMethod;

  if ( displayAll )
  {
    setSearch( POPMUSIC_TABU );
  }

  // First, extract the problem
  if ( !( prob = extract( bbox[0], bbox[1], bbox[2], bbox[3] ) ) )
  {
    // nothing to be done => return an empty result set
    if ( stats )
      ( *stats ) = new PalStat();
    return new QList<LabelPosition*>();
  }

  // reduce number of candidates
  // (remove candidates which surely won't be used)
  prob->reduce();
  prob->displayAll = displayAll;

  // search a solution
  if ( searchMethod == FALP )
    prob->init_sol_falp();
  else if ( searchMethod == CHAIN )
    prob->chain_search();
  else
    prob->popmusic();

  // Post-Optimization
  //prob->post_optimization();


  QList<LabelPosition*> * solution = prob->getSolution( displayAll );

  if ( stats )
    *stats = prob->getStats();

  delete prob;

  if ( displayAll )
  {
    setSearch( old_searchMethod );
  }

  return solution;
}

void Pal::registerCancellationCallback( Pal::FnIsCancelled fnCancelled, void *context )
{
  fnIsCancelled = fnCancelled;
  fnIsCancelledContext = context;
}

Problem* Pal::extractProblem( double bbox[4] )
{
  return extract( bbox[0], bbox[1], bbox[2], bbox[3] );
}

QList<LabelPosition*>* Pal::solveProblem( Problem* prob, bool displayAll )
{
  if ( !prob )
    return new QList<LabelPosition*>();

  prob->reduce();

  try
  {
    if ( searchMethod == FALP )
      prob->init_sol_falp();
    else if ( searchMethod == CHAIN )
      prob->chain_search();
    else
      prob->popmusic();
  }
  catch ( InternalException::Empty )
  {
    return new QList<LabelPosition*>();
  }

  return prob->getSolution( displayAll );
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

SearchMethod Pal::getSearch()
{
  return searchMethod;
}

void Pal::setSearch( SearchMethod method )
{
  switch ( method )
  {
    case POPMUSIC_CHAIN:
      searchMethod = method;
      popmusic_r = 30;
      tabuMinIt = 2;
      tabuMaxIt = 4;
      tenure = 10;
      ejChainDeg = 50;
      candListSize = 0.2;
      break;
    case CHAIN:
      searchMethod      = method;
      ejChainDeg         = 50;
      break;
    case POPMUSIC_TABU:
      searchMethod = method;
      popmusic_r = 25;
      tabuMinIt = 2;
      tabuMaxIt = 4;
      tenure = 10;
      ejChainDeg = 50;
      candListSize = 0.2;
      break;
    case POPMUSIC_TABU_CHAIN:
      searchMethod = method;
      popmusic_r = 25;
      tabuMinIt = 2;
      tabuMaxIt = 4;
      tenure = 10;
      ejChainDeg = 50;
      candListSize = 0.2;
      break;
    case FALP:
      searchMethod = method;
      break;
  }
}


