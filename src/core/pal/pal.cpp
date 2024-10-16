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
#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "pointset.h"
#include "internalexception.h"
#include "util.h"
#include "palrtree.h"
#include "qgslabelingengine.h"
#include "qgsrendercontext.h"
#include "qgssettingsentryimpl.h"
#include "qgsruntimeprofiler.h"

#include <cfloat>
#include <list>


using namespace pal;

const QgsSettingsEntryInteger *Pal::settingsRenderingLabelCandidatesLimitPoints = new QgsSettingsEntryInteger( QStringLiteral( "label-candidates-limit-points" ), sTreePal, 0 );
const QgsSettingsEntryInteger *Pal::settingsRenderingLabelCandidatesLimitLines = new QgsSettingsEntryInteger( QStringLiteral( "label-candidates-limit-lines" ), sTreePal, 0 );
const QgsSettingsEntryInteger *Pal::settingsRenderingLabelCandidatesLimitPolygons = new QgsSettingsEntryInteger( QStringLiteral( "label-candidates-limit-polygons" ), sTreePal, 0 );


Pal::Pal()
{
  mGlobalCandidatesLimitPoint = Pal::settingsRenderingLabelCandidatesLimitPoints->value();
  mGlobalCandidatesLimitLine = Pal::settingsRenderingLabelCandidatesLimitLines->value();
  mGlobalCandidatesLimitPolygon = Pal::settingsRenderingLabelCandidatesLimitPolygons->value();
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

Layer *Pal::addLayer( QgsAbstractLabelProvider *provider, const QString &layerName, Qgis::LabelPlacement arrangement, double defaultPriority, bool active, bool toLabel )
{
  mMutex.lock();

  Q_ASSERT( mLayers.find( provider ) == mLayers.end() );

  std::unique_ptr< Layer > layer = std::make_unique< Layer >( provider, layerName, arrangement, defaultPriority, active, toLabel, this );
  Layer *res = layer.get();
  mLayers.insert( std::pair<QgsAbstractLabelProvider *, std::unique_ptr< Layer >>( provider, std::move( layer ) ) );
  mMutex.unlock();

  // cppcheck-suppress returnDanglingLifetime
  return res;
}

std::unique_ptr<Problem> Pal::extractProblem( const QgsRectangle &extent, const QgsGeometry &mapBoundary, QgsRenderContext &context )
{
  QgsLabelingEngineFeedback *feedback = qobject_cast< QgsLabelingEngineFeedback * >( context.feedback() );

  std::unique_ptr< QgsScopedRuntimeProfile > extractionProfile;
  if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    extractionProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Placing labels" ), QStringLiteral( "rendering" ) );
  }

  // expand out the incoming buffer by 1000x -- that's the visible map extent, yet we may be getting features which exceed this extent
  // (while 1000x may seem excessive here, this value is only used for scaling coordinates in the spatial indexes
  // and the consequence of inserting coordinates outside this extent is worse than the consequence of setting this value too large.)
  const QgsRectangle maxCoordinateExtentForSpatialIndices = extent.buffered( std::max( extent.width(), extent.height() ) * 1000 );

  // to store obstacles
  PalRtree< FeaturePart > obstacles( maxCoordinateExtentForSpatialIndices );
  PalRtree< LabelPosition > allCandidatesFirstRound( maxCoordinateExtentForSpatialIndices );
  std::vector< FeaturePart * > allObstacleParts;
  std::unique_ptr< Problem > prob = std::make_unique< Problem >( maxCoordinateExtentForSpatialIndices );

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

  double step = !mLayers.empty() ? 100.0 / mLayers.size() : 1;
  int index = -1;
  std::unique_ptr< QgsScopedRuntimeProfile > candidateProfile;
  if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    candidateProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Generating label candidates" ), QStringLiteral( "rendering" ) );
  }

  for ( const auto &it : mLayers )
  {
    index++;
    if ( feedback )
      feedback->setProgress( index * step );

    Layer *layer = it.second.get();
    if ( !layer )
    {
      // invalid layer name
      continue;
    }

    // only select those who are active
    if ( !layer->active() )
      continue;

    if ( feedback )
      feedback->emit candidateCreationAboutToBegin( it.first );

    std::unique_ptr< QgsScopedRuntimeProfile > layerProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      layerProfile = std::make_unique< QgsScopedRuntimeProfile >( it.first->providerId(), QStringLiteral( "rendering" ) );
    }

    // check for connected features with the same label text and join them
    if ( layer->mergeConnectedLines() )
      layer->joinConnectedFeatures();

    if ( isCanceled() )
      return nullptr;

    layer->chopFeaturesAtRepeatDistance();

    if ( isCanceled() )
      return nullptr;

    QMutexLocker locker( &layer->mMutex );

    const double featureStep = !layer->mFeatureParts.empty() ? step / layer->mFeatureParts.size() : 1;
    std::size_t featureIndex = 0;
    // generate candidates for all features
    for ( const std::unique_ptr< FeaturePart > &featurePart : std::as_const( layer->mFeatureParts ) )
    {
      if ( feedback )
        feedback->setProgress( index * step + featureIndex * featureStep );
      featureIndex++;

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
          candidate->setGlobalId( mNextCandidateId++ );
        }

        std::sort( candidates.begin(), candidates.end(), CostCalculator::candidateSortGrow );

        // valid features are added to fFeats
        std::unique_ptr< Feats > ft = std::make_unique< Feats >();
        ft->feature = featurePart.get();
        ft->shape = nullptr;
        ft->candidates = std::move( candidates );
        ft->priority = featurePart->calculatePriority();
        features.emplace_back( std::move( ft ) );
      }
      else
      {
        // no candidates, so generate a default "point on surface" one
        std::unique_ptr< LabelPosition > unplacedPosition = featurePart->createCandidatePointOnSurface( featurePart.get() );
        if ( !unplacedPosition )
          continue;

        if ( featurePart->feature()->allowDegradedPlacement() )
        {
          // if we are allowing degraded placements, we throw the default candidate in too
          unplacedPosition->insertIntoIndex( allCandidatesFirstRound );
          unplacedPosition->setGlobalId( mNextCandidateId++ );
          candidates.emplace_back( std::move( unplacedPosition ) );

          // valid features are added to fFeats
          std::unique_ptr< Feats > ft = std::make_unique< Feats >();
          ft->feature = featurePart.get();
          ft->shape = nullptr;
          ft->candidates = std::move( candidates );
          ft->priority = featurePart->calculatePriority();
          features.emplace_back( std::move( ft ) );
        }
        else
        {
          // not displaying all labels for this layer, so it goes into the unlabeled feature list
          prob->positionsWithNoCandidates()->emplace_back( std::move( unplacedPosition ) );
        }
      }
    }
    if ( isCanceled() )
      return nullptr;

    // collate all layer obstacles
    for ( FeaturePart *obstaclePart : std::as_const( layer->mObstacleParts ) )
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

    if ( feedback )
      feedback->emit candidateCreationFinished( it.first );
  }

  candidateProfile.reset();

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
    if ( feedback )
      feedback->emit obstacleCostingAboutToBegin();

    std::unique_ptr< QgsScopedRuntimeProfile > costingProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      costingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Assigning label costs" ), QStringLiteral( "rendering" ) );
    }

    // Filtering label positions against obstacles
    index = -1;
    step = !allObstacleParts.empty() ? 100.0 / allObstacleParts.size() : 1;

    for ( FeaturePart *obstaclePart : allObstacleParts )
    {
      index++;
      if ( feedback )
        feedback->setProgress( step * index );

      if ( isCanceled() )
        break; // do not continue searching

      allCandidatesFirstRound.intersects( obstaclePart->boundingBox(), [obstaclePart, this]( const LabelPosition * candidatePosition ) -> bool
      {
        // test whether we should ignore this obstacle for the candidate. We do this if:
        // 1. it's not a hole, and the obstacle belongs to the same label feature as the candidate (e.g.,
        // features aren't obstacles for their own labels)
        // 2. it IS a hole, and the hole belongs to a different label feature to the candidate (e.g., holes
        // are ONLY obstacles for the labels of the feature they belong to)
        // 3. The label is set to "Always Allow" overlap mode
        if ( candidatePosition->getFeaturePart()->feature()->overlapHandling() == Qgis::LabelOverlapHandling::AllowOverlapAtNoCost
             || ( !obstaclePart->getHoleOf() && candidatePosition->getFeaturePart()->hasSameLabelFeatureAs( obstaclePart ) )
             || ( obstaclePart->getHoleOf() && !candidatePosition->getFeaturePart()->hasSameLabelFeatureAs( dynamic_cast< FeaturePart * >( obstaclePart->getHoleOf() ) ) ) )
        {
          return true;
        }

        CostCalculator::addObstacleCostPenalty( const_cast< LabelPosition * >( candidatePosition ), obstaclePart, this );

        return true;
      } );
    }

    if ( feedback )
      feedback->emit obstacleCostingFinished();
    costingProfile.reset();

    if ( isCanceled() )
    {
      return nullptr;
    }

    step = prob->mFeatureCount != 0 ? 100.0 / prob->mFeatureCount : 1;
    if ( feedback )
      feedback->emit calculatingConflictsAboutToBegin();

    std::unique_ptr< QgsScopedRuntimeProfile > conflictProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      conflictProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Calculating conflicts" ), QStringLiteral( "rendering" ) );
    }

    int idlp = 0;
    for ( std::size_t i = 0; i < prob->mFeatureCount; i++ ) /* for each feature into prob */
    {
      if ( feedback )
        feedback->setProgress( i * step );

      std::unique_ptr< Feats > feat = std::move( features.front() );
      features.pop_front();

      prob->mFeatStartId[i] = idlp;
      prob->mInactiveCost[i] = std::pow( 2, 10 - 10 * feat->priority );

      std::size_t maxCandidates = 0;
      switch ( feat->feature->getGeosType() )
      {
        case GEOS_POINT:
          // this is usually 0, i.e. no maximum
          maxCandidates = feat->feature->maximumPointCandidates();
          break;

        case GEOS_LINESTRING:
          maxCandidates = feat->feature->maximumLineCandidates();
          break;

        case GEOS_POLYGON:
          maxCandidates = std::max( static_cast< std::size_t >( 16 ), feat->feature->maximumPolygonCandidates() );
          break;
      }

      if ( isCanceled() )
        return nullptr;

      auto pruneHardConflicts = [&]
      {
        switch ( mPlacementVersion )
        {
          case Qgis::LabelPlacementEngineVersion::Version1:
            break;

          case Qgis::LabelPlacementEngineVersion::Version2:
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
              switch ( feat->feature->feature()->overlapHandling() )
              {
                case Qgis::LabelOverlapHandling::PreventOverlap:
                {
                  // we're going to end up removing ALL candidates for this label. Oh well, that's allowed. We just need to
                  // make sure we move this last candidate to the unplaced labels list
                  prob->positionsWithNoCandidates()->emplace_back( std::move( feat->candidates.front() ) );
                  feat->candidates.clear();
                  break;
                }

                case Qgis::LabelOverlapHandling::AllowOverlapIfRequired:
                case Qgis::LabelOverlapHandling::AllowOverlapAtNoCost:
                  // we can't avoid overlaps for this label, but in this mode we are allowing overlaps as a last resort.
                  // => don't discard this last remaining candidate.
                  break;
              }
            }
          }
        }
      };

      // if we're not showing all labels (including conflicts) for this layer, then we prune the candidates
      // upfront to avoid extra work...
      switch ( feat->feature->feature()->overlapHandling() )
      {
        case Qgis::LabelOverlapHandling::PreventOverlap:
          pruneHardConflicts();
          break;

        case Qgis::LabelOverlapHandling::AllowOverlapIfRequired:
        case Qgis::LabelOverlapHandling::AllowOverlapAtNoCost:
          break;
      }

      if ( feat->candidates.empty() )
        continue;

      // calculate final costs
      CostCalculator::finalizeCandidatesCosts( feat.get(), bbx, bby );

      // sort candidates list, best label to worst
      std::sort( feat->candidates.begin(), feat->candidates.end(), CostCalculator::candidateSortGrow );

      // but if we ARE showing all labels (including conflicts), let's go ahead and prune them now.
      // Since we've calculated all their costs and sorted them, if we've hit the situation that ALL
      // candidates have conflicts, then at least when we pick the first candidate to display it will be
      // the lowest cost (i.e. best possible) overlapping candidate...
      switch ( feat->feature->feature()->overlapHandling() )
      {
        case Qgis::LabelOverlapHandling::PreventOverlap:
          break;
        case Qgis::LabelOverlapHandling::AllowOverlapIfRequired:
        case Qgis::LabelOverlapHandling::AllowOverlapAtNoCost:
          pruneHardConflicts();
          break;
      }

      // only keep the 'maxCandidates' best candidates
      if ( maxCandidates > 0 && feat->candidates.size() > maxCandidates )
      {
        feat->candidates.resize( maxCandidates );
      }

      if ( isCanceled() )
        return nullptr;

      // update problem's # candidate
      prob->mFeatNbLp[i] = static_cast< int >( feat->candidates.size() );
      prob->mTotalCandidates += static_cast< int >( feat->candidates.size() );

      // add all candidates into a rtree (to speed up conflicts searching)
      for ( std::unique_ptr< LabelPosition > &candidate : feat->candidates )
      {
        candidate->insertIntoIndex( prob->allCandidatesIndex() );
        candidate->setProblemIds( static_cast< int >( i ), idlp++ );
      }
      features.emplace_back( std::move( feat ) );
    }

    if ( feedback )
      feedback->emit calculatingConflictsFinished();

    conflictProfile.reset();

    int nbOverlaps = 0;

    double amin[2];
    double amax[2];

    if ( feedback )
      feedback->emit finalizingCandidatesAboutToBegin();

    std::unique_ptr< QgsScopedRuntimeProfile > finalizingProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      finalizingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Finalizing labels" ), QStringLiteral( "rendering" ) );
    }

    index = -1;
    step = !features.empty() ? 100.0 / features.size() : 1;
    while ( !features.empty() ) // for each feature
    {
      index++;
      if ( feedback )
        feedback->setProgress( step * index );

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
        prob->allCandidatesIndex().intersects( QgsRectangle( amin[0], amin[1], amax[0], amax[1] ), [&lp, this]( const LabelPosition * lp2 )->bool
        {
          if ( candidatesAreConflicting( lp.get(), lp2 ) )
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

    if ( feedback )
      feedback->emit finalizingCandidatesFinished();

    finalizingProfile.reset();

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


QList<LabelPosition *> Pal::solveProblem( Problem *prob, QgsRenderContext &context, bool displayAll, QList<LabelPosition *> *unlabeled )
{
  QgsLabelingEngineFeedback *feedback = qobject_cast< QgsLabelingEngineFeedback * >( context.feedback() );

  if ( !prob )
    return QList<LabelPosition *>();


  std::unique_ptr< QgsScopedRuntimeProfile > calculatingProfile;
  if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
  {
    calculatingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Calculating optimal labeling" ), QStringLiteral( "rendering" ) );
  }

  if ( feedback )
    feedback->emit reductionAboutToBegin();

  {
    std::unique_ptr< QgsScopedRuntimeProfile > reductionProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      reductionProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Reducing labeling" ), QStringLiteral( "rendering" ) );
    }

    prob->reduce();
  }

  if ( feedback )
    feedback->emit reductionFinished();

  if ( feedback )
    feedback->emit solvingPlacementAboutToBegin();

  {
    std::unique_ptr< QgsScopedRuntimeProfile > solvingProfile;
    if ( context.flags() & Qgis::RenderContextFlag::RecordProfile )
    {
      solvingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Solving labeling" ), QStringLiteral( "rendering" ) );
    }
    try
    {
      prob->chainSearch( context );
    }
    catch ( InternalException::Empty & )
    {
      return QList<LabelPosition *>();
    }
  }

  if ( feedback )
    feedback->emit solvingPlacementFinished();

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

Qgis::LabelPlacementEngineVersion Pal::placementVersion() const
{
  return mPlacementVersion;
}

void Pal::setPlacementVersion( Qgis::LabelPlacementEngineVersion placementVersion )
{
  mPlacementVersion = placementVersion;
}

bool Pal::candidatesAreConflicting( const LabelPosition *lp1, const LabelPosition *lp2 ) const
{
  // we cache the value -- this can be costly to calculate, and we check this multiple times
  // per candidate during the labeling problem solving

  // conflicts are commutative - so we always store them in the cache using the smaller id as the first element of the key pair
  auto key = qMakePair( std::min( lp1->globalId(), lp2->globalId() ), std::max( lp1->globalId(), lp2->globalId() ) );
  auto it = mCandidateConflicts.constFind( key );
  if ( it != mCandidateConflicts.constEnd() )
    return *it;

  const bool res = lp1->isInConflict( lp2 );
  mCandidateConflicts.insert( key, res );
  return res;
}

int Pal::getMinIt() const
{
  return mTabuMaxIt;
}

int Pal::getMaxIt() const
{
  return mTabuMinIt;
}

bool Pal::showPartialLabels() const
{
  return mShowPartialLabels;
}
