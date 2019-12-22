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
#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "pointset.h"
#include "internalexception.h"
#include "util.h"
#include "palrtree.h"
#include "qgssettings.h"
#include <cfloat>
#include <list>

using namespace pal;

Pal::Pal()
{
  QgsSettings settings;
  mGlobalCandidatesLimitPoint = settings.value( QStringLiteral( "rendering/label_candidates_limit_points" ), 0, QgsSettings::Core ).toInt();
  mGlobalCandidatesLimitLine = settings.value( QStringLiteral( "rendering/label_candidates_limit_lines" ), 0, QgsSettings::Core ).toInt();
  mGlobalCandidatesLimitPolygon = settings.value( QStringLiteral( "rendering/label_candidates_limit_polygons" ), 0, QgsSettings::Core ).toInt();
}

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

std::unique_ptr<Problem> Pal::extract( const QgsRectangle &extent, const QgsGeometry &mapBoundary )
{
  // to store obstacles
  PalRtree< FeaturePart > obstacles;
  PalRtree< LabelPosition > allCandidatesFirstRound;
  std::vector< FeaturePart * > allObstacleParts;
  std::unique_ptr< Problem > prob = qgis::make_unique< Problem >();

  double bbx[4];
  double bby[4];

  bbx[0] = bbx[3] = prob->mMapExtentBounds[0] = extent.xMinimum();
  bby[0] = bby[1] = prob->mMapExtentBounds[1] = extent.yMinimum();
  bbx[1] = bbx[2] = prob->mMapExtentBounds[2] = extent.xMaximum();
  bby[2] = bby[3] = prob->mMapExtentBounds[3] = extent.yMaximum();

  prob->pal = this;

  std::list< std::unique_ptr< Feats > > features;

  // prepare map boundary
  geos::unique_ptr mapBoundaryGeos( QgsGeos::asGeos( mapBoundary ) );
  geos::prepared_unique_ptr mapBoundaryPrepared( GEOSPrepare_r( QgsGeos::getGEOSHandler(), mapBoundaryGeos.get() ) );

  int obstacleCount = 0;

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

    // generate candidates for all features
    for ( FeaturePart *featurePart : qgis::as_const( layer->mFeatureParts ) )
    {
      if ( isCanceled() )
        break;

      // Holes of the feature are obstacles
      for ( int i = 0; i < featurePart->getNumSelfObstacles(); i++ )
      {
        FeaturePart *selfObstacle =  featurePart->getSelfObstacle( i );
        obstacles.insert( selfObstacle, selfObstacle->boundingBox() );
        allObstacleParts.emplace_back( selfObstacle );

        if ( !featurePart->getSelfObstacle( i )->getHoleOf() )
        {
          //ERROR: SHOULD HAVE A PARENT!!!!!
        }
      }

      // generate candidates for the feature part
      std::vector< std::unique_ptr< LabelPosition > > candidates = featurePart->createCandidates( this );

      if ( isCanceled() )
        break;

      // purge candidates that are outside the bbox
      candidates.erase( std::remove_if( candidates.begin(), candidates.end(), [&mapBoundaryPrepared, this]( std::unique_ptr< LabelPosition > &candidate )
      {
        if ( showPartialLabels() )
          return !candidate->intersects( mapBoundaryPrepared.get() );
        else
          return !candidate->within( mapBoundaryPrepared.get() );
      } ), candidates.end() );

      if ( isCanceled() )
        break;

      if ( !candidates.empty() )
      {
        for ( std::unique_ptr< LabelPosition > &candidate : candidates )
        {
          candidate->insertIntoIndex( allCandidatesFirstRound );
        }

        std::sort( candidates.begin(), candidates.end(), CostCalculator::candidateSortGrow );

        // valid features are added to fFeats
        std::unique_ptr< Feats > ft = qgis::make_unique< Feats >();
        ft->feature = featurePart;
        ft->shape = nullptr;
        ft->candidates = std::move( candidates );
        ft->priority = featurePart->calculatePriority();
        features.emplace_back( std::move( ft ) );
      }
      else
      {
        // features with no candidates are recorded in the unlabeled feature list
        std::unique_ptr< LabelPosition > unplacedPosition = featurePart->createCandidatePointOnSurface( featurePart );
        if ( unplacedPosition )
          prob->positionsWithNoCandidates()->emplace_back( std::move( unplacedPosition ) );
      }
    }
    if ( isCanceled() )
      return nullptr;

    // collate all layer obstacles
    for ( FeaturePart *obstaclePart : qgis::as_const( layer->mObstacleParts ) )
    {
      if ( isCanceled() )
        break; // do not continue searching

      // insert into obstacles
      obstacles.insert( obstaclePart, obstaclePart->boundingBox() );
      allObstacleParts.emplace_back( obstaclePart );
      obstacleCount++;
    }

    if ( isCanceled() )
      return nullptr;

    locker.unlock();

    if ( features.size() - previousFeatureCount > 0 || obstacleCount > previousObstacleCount )
    {
      layersWithFeaturesInBBox << layer->name();
    }
    previousFeatureCount = features.size();
    previousObstacleCount = obstacleCount;
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
    for ( FeaturePart *obstaclePart : allObstacleParts )
    {
      if ( isCanceled() )
        break; // do not continue searching

      allCandidatesFirstRound.intersects( obstaclePart->boundingBox(), [obstaclePart, this]( const LabelPosition * candidatePosition ) -> bool
      {
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

        CostCalculator::addObstacleCostPenalty( const_cast< LabelPosition * >( candidatePosition ), obstaclePart, this );

        return true;
      } );
    }

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

      std::size_t max_p = 0;
      switch ( feat->feature->getGeosType() )
      {
        case GEOS_POINT:
          // this is usually 0, i.e. no maximum
          max_p = feat->feature->maximumPointCandidates();
          break;

        case GEOS_LINESTRING:
          max_p = feat->feature->maximumLineCandidates();
          break;

        case GEOS_POLYGON:
          max_p = feat->feature->maximumPolygonCandidates();
          break;
      }

      // sort candidates by cost, skip less interesting ones, calculate polygon costs (if using polygons)
      max_p = CostCalculator::finalizeCandidatesCosts( feat.get(), max_p, &obstacles, bbx, bby );

      if ( isCanceled() )
        return nullptr;

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
              return true;
            }
            return false;
          } ), feat->candidates.end() );

          if ( feat->candidates.size() == 1 && feat->candidates[ 0 ]->hasHardObstacleConflict() )
          {
            // we've ended up removing ALL candidates for this label. Oh well, that's allowed. We just need to
            // make sure we move this last candidate to the unplaced labels list
            prob->positionsWithNoCandidates()->emplace_back( std::move( feat->candidates.front() ) );
            feat->candidates.clear();
          }
        }
      }

      // only keep the 'max_p' best candidates
      while ( feat->candidates.size() > max_p )
      {
        feat->candidates.pop_back();
      }

      if ( isCanceled() )
        return nullptr;

      // update problem's # candidate
      prob->mFeatNbLp[i] = static_cast< int >( feat->candidates.size() );
      prob->mTotalCandidates += static_cast< int >( feat->candidates.size() );

      // add all candidates into a rtree (to speed up conflicts searching)
      for ( std::size_t j = 0; j < feat->candidates.size(); j++, idlp++ )
      {
        feat->candidates[ j ]->insertIntoIndex( prob->allCandidatesIndex() );
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
        prob->allCandidatesIndex().intersects( QgsRectangle( amin[0], amin[1], amax[0], amax[1] ), [&lp]( const LabelPosition * lp2 )->bool
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
