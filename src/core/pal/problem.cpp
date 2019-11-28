/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
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

#include "pal.h"
#include "palstat.h"
#include "layer.h"
#include "rtree.hpp"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "util.h"
#include "priorityqueue.h"
#include "internalexception.h"
#include <cfloat>
#include <limits> //for std::numeric_limits<int>::max()

#include "qgslabelingengine.h"

using namespace pal;

inline void delete_chain( Chain *chain )
{
  if ( chain )
  {
    delete[] chain->feat;
    delete[] chain->label;
    delete chain;
  }
}

Problem::Problem()
{
  mMapExtentBounds[0] = 0;
  mMapExtentBounds[1] = 0;
  mMapExtentBounds[2] = 0;
  mMapExtentBounds[3] = 0;
  mCandidatesIndex = new RTree<LabelPosition *, double, 2, double>();
  mActiveCandidatesIndex = new RTree<LabelPosition *, double, 2, double>();
}

Problem::~Problem()
{
  delete[] mFeatStartId;
  delete[] mFeatNbLp;

  qDeleteAll( mLabelPositions );
  mLabelPositions.clear();

  qDeleteAll( mPositionsWithNoCandidates );

  delete[] mInactiveCost;

  delete mCandidatesIndex;
  delete mActiveCandidatesIndex;
}


void Problem::reduce()
{

  int i;
  int j;
  int k;

  int counter = 0;

  int lpid;

  bool *ok = new bool[mTotalCandidates];
  bool run = true;

  for ( i = 0; i < mTotalCandidates; i++ )
    ok[i] = false;


  double amin[2];
  double amax[2];
  LabelPosition *lp2 = nullptr;

  while ( run )
  {
    run = false;
    for ( i = 0; i < static_cast< int >( mFeatureCount ); i++ )
    {
      // ok[i] = true;
      for ( j = 0; j < mFeatNbLp[i]; j++ )  // foreach candidate
      {
        if ( !ok[mFeatStartId[i] + j] )
        {
          if ( mLabelPositions.at( mFeatStartId[i] + j )->getNumOverlaps() == 0 ) // if candidate has no overlap
          {
            run = true;
            ok[mFeatStartId[i] + j] = true;
            // 1) remove worse candidates from candidates
            // 2) update nb_overlaps
            counter += mFeatNbLp[i] - j - 1;

            for ( k = j + 1; k < mFeatNbLp[i]; k++ )
            {

              lpid = mFeatStartId[i] + k;
              ok[lpid] = true;
              lp2 = mLabelPositions.at( lpid );

              lp2->getBoundingBox( amin, amax );

              mNbOverlap -= lp2->getNumOverlaps();
              mCandidatesIndex->Search( amin, amax, LabelPosition::removeOverlapCallback, reinterpret_cast< void * >( lp2 ) );
              lp2->removeFromIndex( mCandidatesIndex );
            }

            mFeatNbLp[i] = j + 1;
            break;
          }
        }
      }
    }
  }

  this->mTotalCandidates -= counter;
  delete[] ok;
}

struct FalpContext
{
  PriorityQueue *list = nullptr;
  LabelPosition *lp = nullptr;
  RTree <LabelPosition *, double, 2, double> *candidates;
} ;

bool falpCallback2( LabelPosition *lp, void *ctx )
{
  FalpContext *context = reinterpret_cast< FalpContext * >( ctx );
  LabelPosition *lp2 = context->lp;
  PriorityQueue *list = context->list;

  if ( lp->getId() != lp2->getId() && list->isIn( lp->getId() ) && lp->isInConflict( lp2 ) )
  {
    list->decreaseKey( lp->getId() );
  }
  return true;
}


void ignoreLabel( LabelPosition *lp, PriorityQueue *list, RTree <LabelPosition *, double, 2, double> *candidates )
{


  FalpContext *context = new FalpContext();
  context->candidates = nullptr;
  context->list = list;
  double amin[2];
  double amax[2];

  if ( list->isIn( lp->getId() ) )
  {
    list->remove( lp->getId() );

    lp->getBoundingBox( amin, amax );

    context->lp = lp;
    candidates->Search( amin, amax, falpCallback2, context );
  }

  delete context;
}


bool falpCallback1( LabelPosition *lp, void *ctx )
{
  FalpContext *context = reinterpret_cast< FalpContext * >( ctx );
  LabelPosition *lp2 = context->lp;
  PriorityQueue *list = context->list;
  RTree <LabelPosition *, double, 2, double> *candidates = context->candidates;

  if ( lp2->isInConflict( lp ) )
  {
    ignoreLabel( lp, list, candidates );
  }
  return true;
}



/* Better initial solution
 * Step one FALP (Yamamoto, Camara, Lorena 2005)
 */
void Problem::init_sol_falp()
{
  int i, j;
  int label;
  PriorityQueue *list = nullptr;

  mSol.init( mFeatureCount );

  list = new PriorityQueue( mTotalCandidates, mAllNblp, true );

  double amin[2];
  double amax[2];

  FalpContext *context = new FalpContext();
  context->candidates = mCandidatesIndex;
  context->list = list;

  LabelPosition *lp = nullptr;

  for ( i = 0; i < static_cast< int >( mFeatureCount ); i++ )
    for ( j = 0; j < mFeatNbLp[i]; j++ )
    {
      label = mFeatStartId[i] + j;
      try
      {
        list->insert( label, mLabelPositions.at( label )->getNumOverlaps() );
      }
      catch ( pal::InternalException::Full & )
      {
        continue;
      }
    }

  while ( list->getSize() > 0 ) // O (log size)
  {
    if ( pal->isCanceled() )
    {
      delete context;
      delete list;
      return;
    }

    label = list->getBest();   // O (log size)


    lp = mLabelPositions.at( label );

    if ( lp->getId() != label )
    {
      //error
    }

    int probFeatId = lp->getProblemFeatureId();
    mSol.activeLabelIds[probFeatId] = label;

    for ( i = mFeatStartId[probFeatId]; i < mFeatStartId[probFeatId] + mFeatNbLp[probFeatId]; i++ )
    {
      ignoreLabel( mLabelPositions.at( i ), list, mCandidatesIndex );
    }


    lp->getBoundingBox( amin, amax );

    context->lp = lp;
    mCandidatesIndex->Search( amin, amax, falpCallback1, reinterpret_cast< void * >( context ) );
    mActiveCandidatesIndex->Insert( amin, amax, lp );
  }

  delete context;




  if ( mDisplayAll )
  {
    int nbOverlap;
    int start_p;
    LabelPosition *retainedLabel = nullptr;
    int p;

    for ( std::size_t i = 0; i < mFeatureCount; i++ ) // forearch hidden feature
    {
      if ( mSol.activeLabelIds[i] == -1 )
      {
        nbOverlap = std::numeric_limits<int>::max();
        start_p = mFeatStartId[i];
        for ( p = 0; p < mFeatNbLp[i]; p++ )
        {
          lp = mLabelPositions.at( start_p + p );
          lp->resetNumOverlaps();

          lp->getBoundingBox( amin, amax );


          mActiveCandidatesIndex->Search( amin, amax, LabelPosition::countOverlapCallback, lp );

          if ( lp->getNumOverlaps() < nbOverlap )
          {
            retainedLabel = lp;
            nbOverlap = lp->getNumOverlaps();
          }
        }
        mSol.activeLabelIds[i] = retainedLabel->getId();

        retainedLabel->insertIntoIndex( mActiveCandidatesIndex );

      }
    }
  }

  delete list;
}


struct ChainContext
{
  LabelPosition *lp = nullptr;
  std::vector< int > *tmpsol = nullptr;
  int *featWrap = nullptr;
  int *feat = nullptr;
  int borderSize;
  QLinkedList<ElemTrans *> *currentChain;
  QLinkedList<int> *conflicts;
  double *delta_tmp = nullptr;
  double *inactiveCost = nullptr;

} ;


bool chainCallback( LabelPosition *lp, void *context )
{
  ChainContext *ctx = reinterpret_cast< ChainContext * >( context );

  if ( lp->isInConflict( ctx->lp ) )
  {
    int feat, rfeat;
    bool sub = nullptr != ctx->featWrap;

    feat = lp->getProblemFeatureId();
    if ( sub )
    {
      rfeat = feat;
      feat = ctx->featWrap[feat];
    }
    else
      rfeat = feat;

    if ( feat >= 0 && ( *ctx->tmpsol )[feat] == lp->getId() )
    {
      if ( sub && feat < ctx->borderSize )
      {
        throw - 2;
      }
    }

    // is there any cycles ?
    QLinkedList< ElemTrans * >::iterator cur;
    for ( cur = ctx->currentChain->begin(); cur != ctx->currentChain->end(); ++cur )
    {
      if ( ( *cur )->feat == feat )
      {
        throw - 1;
      }
    }

    if ( !ctx->conflicts->contains( feat ) )
    {
      ctx->conflicts->append( feat );
      *ctx->delta_tmp += lp->cost() + ctx->inactiveCost[rfeat];
    }
  }
  return true;
}

inline Chain *Problem::chain( int seed )
{

  int i;
  int j;

  int lid;

  double delta;
  double delta_min;
  double delta_best = std::numeric_limits<double>::max();
  double delta_tmp;

  int next_seed;
  int retainedLabel;
  Chain *retainedChain = nullptr;

  int max_degree = pal->mEjChainDeg;

  int seedNbLp;

  QLinkedList<ElemTrans *> currentChain;
  QLinkedList<int> conflicts;

  std::vector< int > tmpsol( mSol.activeLabelIds );

  LabelPosition *lp = nullptr;
  double amin[2];
  double amax[2];

  ChainContext context;
  context.featWrap = nullptr;
  context.borderSize = 0;
  context.tmpsol = &tmpsol;
  context.inactiveCost = mInactiveCost;
  context.feat = nullptr;
  context.currentChain = &currentChain;
  context.conflicts = &conflicts;
  context.delta_tmp = &delta_tmp;

  delta = 0;
  while ( seed != -1 )
  {
    seedNbLp = mFeatNbLp[seed];
    delta_min = std::numeric_limits<double>::max();

    next_seed = -1;
    retainedLabel = -2;

    // sol[seed] is ejected
    if ( tmpsol[seed] == -1 )
      delta -= mInactiveCost[seed];
    else
      delta -= mLabelPositions.at( tmpsol[seed] )->cost();

    for ( i = -1; i < seedNbLp; i++ )
    {
      try
      {
        // Skip active label !
        if ( !( tmpsol[seed] == -1 && i == -1 ) && i + mFeatStartId[seed] != tmpsol[seed] )
        {
          if ( i != -1 ) // new_label
          {
            lid = mFeatStartId[seed] + i;
            delta_tmp = delta;

            lp = mLabelPositions.at( lid );

            // evaluate conflicts graph in solution after moving seed's label
            lp->getBoundingBox( amin, amax );

            context.lp = lp;

            mActiveCandidatesIndex->Search( amin, amax, chainCallback, reinterpret_cast< void * >( &context ) );

            // no conflict -> end of chain
            if ( conflicts.isEmpty() )
            {
              if ( !retainedChain || delta + lp->cost() < delta_best )
              {
                if ( retainedChain )
                {
                  delete[] retainedChain->label;
                  delete[] retainedChain->feat;
                }
                else
                {
                  retainedChain = new Chain();
                }

                delta_best = delta + lp->cost();

                retainedChain->degree = currentChain.size() + 1;
                retainedChain->feat  = new int[retainedChain->degree];
                retainedChain->label = new int[retainedChain->degree];
                QLinkedList<ElemTrans *>::iterator current = currentChain.begin();
                ElemTrans *move = nullptr;
                j = 0;
                while ( current != currentChain.end() )
                {
                  move = *current;
                  retainedChain->feat[j]  = move->feat;
                  retainedChain->label[j] = move->new_label;
                  ++current;
                  ++j;
                }
                retainedChain->feat[j] = seed;
                retainedChain->label[j] = lid;
                retainedChain->delta = delta + lp->cost();
              }
            }

            // another feature can be ejected
            else if ( conflicts.size() == 1 )
            {
              if ( delta_tmp < delta_min )
              {
                delta_min = delta_tmp;
                retainedLabel = lid;
                next_seed = conflicts.takeFirst();
              }
              else
              {
                conflicts.takeFirst();
              }
            }
            else
            {

              // A lot of conflict : make them inactive and store chain
              Chain *newChain = new Chain();
              newChain->degree = currentChain.size() + 1 + conflicts.size();
              newChain->feat  = new int[newChain->degree];
              newChain->label = new int[newChain->degree];
              QLinkedList<ElemTrans *>::iterator current = currentChain.begin();
              ElemTrans *move = nullptr;
              j = 0;

              while ( current != currentChain.end() )
              {
                move = *current;
                newChain->feat[j]  = move->feat;
                newChain->label[j] = move->new_label;
                ++current;
                ++j;
              }

              // add the current candidates into the chain
              newChain->feat[j] = seed;
              newChain->label[j] = lid;
              newChain->delta = delta + mLabelPositions.at( newChain->label[j] )->cost();
              j++;

              // hide all conflictual candidates
              while ( !conflicts.isEmpty() )
              {
                int ftid = conflicts.takeFirst();
                newChain->feat[j] = ftid;
                newChain->label[j] = -1;
                newChain->delta += mInactiveCost[ftid];
                j++;
              }

              if ( newChain->delta < delta_best )
              {
                if ( retainedChain )
                  delete_chain( retainedChain );

                delta_best = newChain->delta;
                retainedChain = newChain;
              }
              else
              {
                delete_chain( newChain );
              }
            }

          }
          else   // Current label == -1   end of chain ...
          {
            if ( !retainedChain || delta + mInactiveCost[seed] < delta_best )
            {
              if ( retainedChain )
              {
                delete[] retainedChain->label;
                delete[] retainedChain->feat;
              }
              else
                retainedChain = new Chain();

              delta_best = delta + mInactiveCost[seed];

              retainedChain->degree = currentChain.size() + 1;
              retainedChain->feat  = new int[retainedChain->degree];
              retainedChain->label = new int[retainedChain->degree];
              QLinkedList<ElemTrans *>::iterator current = currentChain.begin();
              ElemTrans *move = nullptr;
              j = 0;
              while ( current != currentChain.end() )
              {
                move = *current;
                retainedChain->feat[j]  = move->feat;
                retainedChain->label[j] = move->new_label;
                ++current;
                ++j;
              }
              retainedChain->feat[j] = seed;
              retainedChain->label[j] = -1;
              retainedChain->delta = delta + mInactiveCost[seed];
            }
          }
        }
      }
      catch ( int i )
      {
        Q_UNUSED( i )
        conflicts.clear();
      }
    } // end foreach labelposition

    if ( next_seed == -1 )
    {
      seed = -1;
    }
    else if ( currentChain.size() > max_degree )
    {
      // Max degree reached
      seed = -1;
    }
    else
    {
      ElemTrans *et = new ElemTrans();
      et->feat  = seed;
      et->old_label = tmpsol[seed];
      et->new_label = retainedLabel;
      currentChain.append( et );

      if ( et->old_label != -1 )
      {
        mLabelPositions.at( et->old_label )->removeFromIndex( mActiveCandidatesIndex );
      }

      if ( et->new_label != -1 )
      {
        mLabelPositions.at( et->new_label )->insertIntoIndex( mActiveCandidatesIndex );
      }


      tmpsol[seed] = retainedLabel;
      delta += mLabelPositions.at( retainedLabel )->cost();
      seed = next_seed;
    }
  }

  while ( !currentChain.isEmpty() )
  {
    std::unique_ptr< ElemTrans > et( currentChain.takeFirst() );

    if ( et->new_label != -1 )
    {
      mLabelPositions.at( et->new_label )->removeFromIndex( mActiveCandidatesIndex );
    }

    if ( et->old_label != -1 )
    {
      mLabelPositions.at( et->old_label )->insertIntoIndex( mActiveCandidatesIndex );
    }
  }

  return retainedChain;
}

struct NokContext
{
  LabelPosition *lp = nullptr;
  bool *ok = nullptr;
  int *wrap = nullptr;
};

bool nokCallback( LabelPosition *lp, void *context )
{
  NokContext *ctx = reinterpret_cast< NokContext *>( context );
  LabelPosition *lp2 = ctx->lp;
  bool *ok = ctx->ok;
  int *wrap = ctx->wrap;

  if ( lp2->isInConflict( lp ) )
  {
    if ( wrap )
    {
      ok[wrap[lp->getProblemFeatureId()]] = false;
    }
    else
    {
      ok[lp->getProblemFeatureId()] = false;
    }
  }

  return true;
}

void Problem::chain_search()
{
  if ( mFeatureCount == 0 )
    return;

  int i;
  int seed;
  bool *ok = new bool[mFeatureCount];
  int fid;
  int lid;
  int popit = 0;
  double amin[2];
  double amax[2];

  NokContext context;
  context.ok = ok;
  context.wrap = nullptr;

  Chain *retainedChain = nullptr;

  std::fill( ok, ok + mFeatureCount, false );

  //initialization();
  init_sol_falp();

  //check_solution();
  solution_cost();

  int iter = 0;

  while ( true )
  {

    //check_solution();

    for ( seed = ( iter + 1 ) % mFeatureCount;
          ok[seed] && seed != iter;
          seed = ( seed + 1 ) % mFeatureCount )
      ;

    // All seeds are OK
    if ( seed == iter )
    {
      break;
    }

    iter = ( iter + 1 ) % mFeatureCount;
    retainedChain = chain( seed );

    if ( retainedChain && retainedChain->delta < - EPSILON )
    {
      // apply modification
      for ( i = 0; i < retainedChain->degree; i++ )
      {
        fid = retainedChain->feat[i];
        lid = retainedChain->label[i];

        if ( mSol.activeLabelIds[fid] >= 0 )
        {
          LabelPosition *old = mLabelPositions.at( mSol.activeLabelIds[fid] );
          old->removeFromIndex( mActiveCandidatesIndex );

          old->getBoundingBox( amin, amax );

          context.lp = old;
          mCandidatesIndex->Search( amin, amax, nokCallback, &context );
        }

        mSol.activeLabelIds[fid] = lid;

        if ( mSol.activeLabelIds[fid] >= 0 )
        {
          mLabelPositions.at( lid )->insertIntoIndex( mActiveCandidatesIndex );
        }

        ok[fid] = false;
      }
      mSol.totalCost += retainedChain->delta;
    }
    else
    {
      // no chain or the one is not god enough
      ok[seed] = true;
    }

    delete_chain( retainedChain );
    popit++;
  }

  solution_cost();
  delete[] ok;
}

QList<LabelPosition *> Problem::getSolution( bool returnInactive, QList<LabelPosition *> *unlabeled )
{
  QList<LabelPosition *> solList;

  for ( std::size_t i = 0; i < mFeatureCount; i++ )
  {
    if ( mSol.activeLabelIds[i] != -1 )
    {
      solList.push_back( mLabelPositions.at( mSol.activeLabelIds[i] ) ); // active labels
    }
    else if ( returnInactive
              || mLabelPositions.at( mFeatStartId[i] )->getFeaturePart()->layer()->displayAll()
              || mLabelPositions.at( mFeatStartId[i] )->getFeaturePart()->alwaysShow() )
    {
      solList.push_back( mLabelPositions.at( mFeatStartId[i] ) ); // unplaced label
    }
    else if ( unlabeled )
    {
      unlabeled->push_back( mLabelPositions.at( mFeatStartId[i] ) );
    }
  }

  // unlabeled features also include those with no candidates
  if ( unlabeled )
    unlabeled->append( mPositionsWithNoCandidates );

  return solList;
}

void Problem::solution_cost()
{
  mSol.totalCost = 0.0;

  int nbOv;

  LabelPosition::CountContext context;
  context.inactiveCost = mInactiveCost;
  context.nbOv = &nbOv;
  context.cost = &mSol.totalCost;
  double amin[2];
  double amax[2];
  LabelPosition *lp = nullptr;

  int nbHidden = 0;

  for ( std::size_t i = 0; i < mFeatureCount; i++ )
  {
    if ( mSol.activeLabelIds[i] == -1 )
    {
      mSol.totalCost += mInactiveCost[i];
      nbHidden++;
    }
    else
    {
      nbOv = 0;
      lp = mLabelPositions.at( mSol.activeLabelIds[i] );

      lp->getBoundingBox( amin, amax );

      context.lp = lp;
      mActiveCandidatesIndex->Search( amin, amax, LabelPosition::countFullOverlapCallback, &context );

      mSol.totalCost += lp->cost();
    }
  }
}
