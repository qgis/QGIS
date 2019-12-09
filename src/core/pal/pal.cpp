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
#include <list>

using namespace pal;

Pal::Pal() = default;

Pal::~Pal() = default;

void Pal::removeLayer( Layer *layer )
{
  if ( !layer )
    return;

  mMutex.lock();

  for ( auto it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    if ( it->second.get() == layer )
    {
      mLayers.erase( it );
      break;
    }
  }
  mMutex.unlock();
}

Layer *Pal::addLayer( QgsAbstractLabelProvider *provider, const QString &layerName, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, bool displayAll )
{
  mMutex.lock();

  Q_ASSERT( mLayers.find( provider ) == mLayers.end() );

  std::unique_ptr< Layer > layer = qgis::make_unique< Layer >( provider, layerName, arrangement, defaultPriority, active, toLabel, this, displayAll );
  Layer *res = layer.get();
  mLayers.insert( std::pair<QgsAbstractLabelProvider *, std::unique_ptr< Layer >>( provider, std::move( layer ) ) );
  mMutex.unlock();

  return res;
}

struct FeatCallBackCtx
{
  Layer *layer = nullptr;

  std::list< std::unique_ptr< Feats > > *features = nullptr;

  QgsGenericSpatialIndex< FeaturePart > *obstacleIndex = nullptr;
  QgsGenericSpatialIndex<LabelPosition> *allCandidateIndex = nullptr;
  std::vector< std::unique_ptr< LabelPosition > > *positionsWithNoCandidates = nullptr;
  const GEOSPreparedGeometry *mapBoundary = nullptr;
  Pal *pal = nullptr;
};


struct ObstacleCallBackCtx
{
  QgsGenericSpatialIndex< FeaturePart > *obstacleIndex = nullptr;
  int obstacleCount = 0;
  Pal *pal = nullptr;
};

struct FilterContext
{
  QgsGenericSpatialIndex<LabelPosition> *allCandidatesIndex = nullptr;
  Pal *pal = nullptr;
};

std::unique_ptr<Problem> Pal::extract( const QgsRectangle &extent, const QgsGeometry &mapBoundary )
{
  // to store obstacles
  QgsGenericSpatialIndex< FeaturePart > obstacles;
  std::unique_ptr< Problem > prob = qgis::make_unique< Problem >();

  double bbx[4];
  double bby[4];

  double amin[2];
  double amax[2];

  std::size_t max_p = 0;

  bbx[0] = bbx[3] = amin[0] = prob->mMapExtentBounds[0] = extent.xMinimum();
  bby[0] = bby[1] = amin[1] = prob->mMapExtentBounds[1] = extent.yMinimum();
  bbx[1] = bbx[2] = amax[0] = prob->mMapExtentBounds[2] = extent.xMaximum();
  bby[2] = bby[3] = amax[1] = prob->mMapExtentBounds[3] = extent.yMaximum();

  prob->pal = this;

  std::list< std::unique_ptr< Feats > > features;

  FeatCallBackCtx context;

  // prepare map boundary
  geos::unique_ptr mapBoundaryGeos( QgsGeos::asGeos( mapBoundary ) );
  geos::prepared_unique_ptr mapBoundaryPrepared( GEOSPrepare_r( QgsGeos::getGEOSHandler(), mapBoundaryGeos.get() ) );

  context.features = &features;
  context.obstacleIndex = &obstacles;
  context.allCandidateIndex = &prob->mAllCandidatesIndex;
  context.positionsWithNoCandidates = prob->positionsWithNoCandidates();
  context.mapBoundary = mapBoundaryPrepared.get();
  context.pal = this;

  ObstacleCallBackCtx obstacleContext;
  obstacleContext.obstacleIndex = &obstacles;
  obstacleContext.obstacleCount = 0;
  obstacleContext.pal = this;

  // first step : extract features from layers

  std::size_t previousFeatureCount = 0;
  int previousObstacleCount = 0;

  QStringList layersWithFeaturesInBBox;

  QMutexLocker palLocker( &mMutex );
  for ( const auto &it : mLayers )
  {
    Layer *layer = it.second.get();
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

    if ( isCanceled() )
      return nullptr;

    layer->chopFeaturesAtRepeatDistance();

    if ( isCanceled() )
      return nullptr;

    QMutexLocker locker( &layer->mMutex );

    // find features within bounding box and generate candidates list
    context.layer = layer;
    layer->mFeatureIndex.intersects( QgsRectangle( amin[ 0], amin[1], amax[0], amax[1] ), [&context]( const  FeaturePart * constFeaturePart )->bool
    {
      FeaturePart *featurePart = const_cast< FeaturePart * >( constFeaturePart );

      if ( context.pal->isCanceled() )
        return false;

      // Holes of the feature are obstacles
      for ( int i = 0; i < featurePart->getNumSelfObstacles(); i++ )
      {
        context.obstacleIndex->insertData( featurePart->getSelfObstacle( i ), featurePart->getSelfObstacle( i )->boundingBox() );

        if ( !featurePart->getSelfObstacle( i )->getHoleOf() )
        {
          //ERROR: SHOULD HAVE A PARENT!!!!!
        }
      }

      // generate candidates for the feature part
      std::vector< std::unique_ptr< LabelPosition > > candidates = featurePart->createCandidates( context.pal );

      if ( context.pal->isCanceled() )
        return false;

      // purge candidates that are outside the bbox
      candidates.erase( std::remove_if( candidates.begin(), candidates.end(), [&context]( std::unique_ptr< LabelPosition > &candidate )
      {
        if ( context.pal->showPartialLabels() )
          return !candidate->intersects( context.mapBoundary );
        else
          return !candidate->within( context.mapBoundary );
      } ), candidates.end() );

      if ( context.pal->isCanceled() )
        return false;

      if ( !candidates.empty() )
      {
        for ( std::unique_ptr< LabelPosition > &candidate : candidates )
        {
          candidate->insertIntoIndex( *context.allCandidateIndex );
        }

        std::sort( candidates.begin(), candidates.end(), CostCalculator::candidateSortGrow );

        // valid features are added to fFeats
        std::unique_ptr< Feats > ft = qgis::make_unique< Feats >();
        ft->feature = featurePart;
        ft->shape = nullptr;
        ft->candidates = std::move( candidates );
        ft->priority = featurePart->calculatePriority();
        context.features->emplace_back( std::move( ft ) );
      }
      else
      {
        // features with no candidates are recorded in the unlabeled feature list
        std::unique_ptr< LabelPosition > unplacedPosition = featurePart->createCandidatePointOnSurface( featurePart );
        if ( unplacedPosition )
          context.positionsWithNoCandidates->emplace_back( std::move( unplacedPosition ) );
      }

      return true;
    } );
    if ( isCanceled() )
      return nullptr;

    // find obstacles within bounding box
    layer->mObstacleIndex.intersects( QgsRectangle( amin[0], amin[1], amax[0], amax[1] ), [&obstacleContext]( const FeaturePart * featurePart )->bool
    {
      if ( obstacleContext.pal->isCanceled() )
        return false; // do not continue searching

      // insert into obstacles
      obstacleContext.obstacleIndex->insertData( featurePart, featurePart->boundingBox() );
      obstacleContext.obstacleCount++;
      return true;
    } );
    if ( isCanceled() )
      return nullptr;

    locker.unlock();

    if ( context.features->size() - previousFeatureCount > 0 || obstacleContext.obstacleCount > previousObstacleCount )
    {
      layersWithFeaturesInBBox << layer->name();
    }
    previousFeatureCount = context.features->size();
    previousObstacleCount = obstacleContext.obstacleCount;
  }
  palLocker.unlock();

  if ( isCanceled() )
    return nullptr;

  prob->mLayerCount = layersWithFeaturesInBBox.size();
  prob->labelledLayersName = layersWithFeaturesInBBox;

  prob->mFeatureCount = features.size();
  prob->mTotalCandidates = 0;
  prob->mFeatNbLp.resize( prob->mFeatureCount );
  prob->mFeatStartId.resize( prob->mFeatureCount );
  prob->mInactiveCost.resize( prob->mFeatureCount );

  if ( !features.empty() )
  {
    // Filtering label positions against obstacles
    amin[0] = amin[1] = std::numeric_limits<double>::lowest();
    amax[0] = amax[1] = std::numeric_limits<double>::max();
    FilterContext filterCtx;
    filterCtx.allCandidatesIndex = &prob->mAllCandidatesIndex;
    filterCtx.pal = this;
    obstacles.intersects( QgsRectangle( amin[0], amin[1], amax[0], amax[1] ), [&filterCtx]( const FeaturePart * part )->bool
    {
      if ( filterCtx.pal->isCanceled() )
        return false; // do not continue searching

      LabelPosition::PruneCtx pruneContext;
      pruneContext.obstacle = const_cast< FeaturePart * >( part );
      pruneContext.pal = filterCtx.pal;
      filterCtx.allCandidatesIndex->intersects( part->boundingBox(), [&pruneContext]( const LabelPosition * candidatePosition ) -> bool{
        FeaturePart *obstaclePart = pruneContext.obstacle;

        // test whether we should ignore this obstacle for the candidate. We do this if:
        // 1. it's not a hole, and the obstacle belongs to the same label feature as the candidate (e.g.,
        // features aren't obstacles for their own labels)
        // 2. it IS a hole, and the hole belongs to a different label feature to the candidate (e.g., holes
        // are ONLY obstacles for the labels of the feature they belong to)
        if ( ( !obstaclePart->getHoleOf() && candidatePosition->getFeaturePart()->hasSameLabelFeatureAs( obstaclePart ) )
             || ( obstaclePart->getHoleOf() && !candidatePosition->getFeaturePart()->hasSameLabelFeatureAs( dynamic_cast< FeaturePart * >( obstaclePart->getHoleOf() ) ) ) )
        {
          return true;
        }

        CostCalculator::addObstacleCostPenalty( const_cast< LabelPosition * >( candidatePosition ), obstaclePart, pruneContext.pal );

        return true;
      } );

      return true;
    } );

    if ( isCanceled() )
    {
      return nullptr;
    }

    int idlp = 0;
    for ( std::size_t i = 0; i < prob->mFeatureCount; i++ ) /* foreach feature into prob */
    {
      std::unique_ptr< Feats > feat = std::move( features.front() );
      features.pop_front();

      prob->mFeatStartId[i] = idlp;
      prob->mInactiveCost[i] = std::pow( 2, 10 - 10 * feat->priority );

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
      max_p = CostCalculator::finalizeCandidatesCosts( feat.get(), max_p, &obstacles, bbx, bby );

      if ( isCanceled() )
        return nullptr;

      // only keep the 'max_p' best candidates
      while ( feat->candidates.size() > max_p )
      {
        // TODO remove from index
        feat->candidates.back()->removeFromIndex( prob->mAllCandidatesIndex );
        feat->candidates.pop_back();
      }

      switch ( mPlacementVersion )
      {
        case QgsLabelingEngineSettings::PlacementEngineVersion1:
          break;

        case QgsLabelingEngineSettings::PlacementEngineVersion2:
        {
          // v2 placement rips out candidates where the candidate cost is too high when compared to
          // their inactive cost

          // note, we start this at the SECOND candidate (you'll see why after this loop)
          feat->candidates.erase( std::remove_if( feat->candidates.begin() + 1, feat->candidates.end(), [ & ]( std::unique_ptr< LabelPosition > &candidate )
          {
            if ( candidate->hasHardObstacleConflict() )
            {
              candidate->removeFromIndex( prob->mAllCandidatesIndex );
              return true;
            }
            return false;
          } ), feat->candidates.end() );

          if ( feat->candidates.size() == 1 && feat->candidates[ 0 ]->hasHardObstacleConflict() )
          {
            // we've ended up removing ALL candidates for this label. Oh well, that's allowed. We just need to
            // make sure we move this last candidate to the unplaced labels list
            feat->candidates.front()->removeFromIndex( prob->mAllCandidatesIndex );
            prob->positionsWithNoCandidates()->emplace_back( std::move( feat->candidates.front() ) );
            feat->candidates.clear();
          }
        }
      }

      if ( isCanceled() )
        return nullptr;

      // update problem's # candidate
      prob->mFeatNbLp[i] = static_cast< int >( feat->candidates.size() );
      prob->mTotalCandidates += static_cast< int >( feat->candidates.size() );

      // add all candidates into a rtree (to speed up conflicts searching)
      for ( std::size_t j = 0; j < feat->candidates.size(); j++, idlp++ )
      {
        //lp->insertIntoIndex(prob->candidates);
        feat->candidates[ j ]->setProblemIds( static_cast< int >( i ), idlp );
      }
      features.emplace_back( std::move( feat ) );
    }

    int nbOverlaps = 0;

    double amin[2];
    double amax[2];
    while ( !features.empty() ) // foreach feature
    {
      if ( isCanceled() )
        return nullptr;

      std::unique_ptr< Feats > feat = std::move( features.front() );
      features.pop_front();

      for ( std::unique_ptr< LabelPosition > &candidate : feat->candidates )
      {
        std::unique_ptr< LabelPosition > lp = std::move( candidate );

        lp->resetNumOverlaps();

        // make sure that candidate's cost is less than 1
        lp->validateCost();

        //prob->feat[idlp] = j;

        // lookup for overlapping candidate
        lp->getBoundingBox( amin, amax );
        prob->mAllCandidatesIndex.intersects( QgsRectangle( amin[0], amin[1], amax[0], amax[1] ), [&lp]( const LabelPosition * lp2 )->bool
        {
          if ( lp->isInConflict( lp2 ) )
          {
            lp->incrementNumOverlaps();
          }

          return true;

        } );

        nbOverlaps += lp->getNumOverlaps();

        prob->addCandidatePosition( std::move( lp ) );

        if ( isCanceled() )
          return nullptr;
      }
    }
    nbOverlaps /= 2;
    prob->mAllNblp = prob->mTotalCandidates;
    prob->mNbOverlap = nbOverlaps;
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


void Pal::setMaximumNumberOfPointCandidates( int candidates )
{
  if ( candidates > 0 )
    this->mMaxPointCandidates = candidates;
}

void Pal::setMaximumNumberOfLineCandidates( int line_p )
{
  if ( line_p > 0 )
    this->mMaxLineCandidates = line_p;
}

void Pal::setMaximumNumberOfPolygonCandidates( int poly_p )
{
  if ( poly_p > 0 )
    this->mMaxPolyCandidates = poly_p;
}


void Pal::setMinIt( int min_it )
{
  if ( min_it >= 0 )
    mTabuMinIt = min_it;
}

void Pal::setMaxIt( int max_it )
{
  if ( max_it > 0 )
    mTabuMaxIt = max_it;
}

void Pal::setPopmusicR( int r )
{
  if ( r > 0 )
    mPopmusicR = r;
}

void Pal::setEjChainDeg( int degree )
{
  this->mEjChainDeg = degree;
}

void Pal::setTenure( int tenure )
{
  this->mTenure = tenure;
}

void Pal::setCandListSize( double fact )
{
  this->mCandListSize = fact;
}

void Pal::setShowPartialLabels( bool show )
{
  this->mShowPartialLabels = show;
}

int Pal::maximumNumberOfPointCandidates() const
{
  return mMaxPointCandidates;
}

int Pal::maximumNumberOfLineCandidates() const
{
  return mMaxLineCandidates;
}

int Pal::maximumNumberOfPolygonCandidates() const
{
  return mMaxPolyCandidates;
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
  return mTabuMaxIt;
}

int Pal::getMaxIt()
{
  return mTabuMinIt;
}

bool Pal::showPartialLabels() const
{
  return mShowPartialLabels;
}
