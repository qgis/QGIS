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
#include <limits.h> //for INT_MAX

#include "qgslabelingenginev2.h"

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
    : nbLabelledLayers( 0 )
    , nblp( 0 )
    , all_nblp( 0 )
    , nbft( 0 )
    , displayAll( false )
    , labelPositionCost( nullptr )
    , nbOlap( nullptr )
    , featStartId( nullptr )
    , featNbLp( nullptr )
    , inactiveCost( nullptr )
    , sol( nullptr )
    , nbActive( 0 )
    , nbOverlap( 0.0 )
    , pal( nullptr )
{
  bbox[0] = 0;
  bbox[1] = 0;
  bbox[2] = 0;
  bbox[3] = 0;
  featWrap = nullptr;
  candidates = new RTree<LabelPosition*, double, 2, double>();
  candidates_sol = new RTree<LabelPosition*, double, 2, double>();
  candidates_subsol = nullptr;
}

Problem::~Problem()
{
  if ( sol )
  {
    if ( sol->s )
      delete[] sol->s;
    delete sol;
  }

  if ( featWrap )
    delete[] featWrap;
  if ( featStartId )
    delete[] featStartId;
  if ( featNbLp )
    delete[] featNbLp;

  qDeleteAll( mLabelPositions );
  mLabelPositions.clear();

  if ( inactiveCost )
    delete[] inactiveCost;

  delete candidates;
  delete candidates_sol;

  if ( candidates_subsol )
  {
    delete candidates_subsol;
  }
}

typedef struct
{
  int id;
  double inactiveCost;
  double nbOverlap;
} Ft;

inline bool borderSizeInc( void *l, void *r )
{
  return ( reinterpret_cast< SubPart* >( l ) )->borderSize > ( reinterpret_cast< SubPart* >( r ) )->borderSize;
}

void Problem::reduce()
{

  int i;
  int j;
  int k;

  int counter = 0;

  int lpid;

  bool *ok = new bool[nblp];
  bool run = true;

  for ( i = 0; i < nblp; i++ )
    ok[i] = false;


  double amin[2];
  double amax[2];
  LabelPosition *lp2;

  while ( run )
  {
    run = false;
    for ( i = 0; i < nbft; i++ )
    {
      // ok[i] = true;
      for ( j = 0; j < featNbLp[i]; j++ )  // foreach candidate
      {
        if ( !ok[featStartId[i] + j] )
        {
          if ( mLabelPositions.at( featStartId[i] + j )->getNumOverlaps() == 0 ) // if candidate has no overlap
          {
            run = true;
            ok[featStartId[i] + j] = true;
            // 1) remove worse candidates from candidates
            // 2) update nb_overlaps
            counter += featNbLp[i] - j - 1;

            for ( k = j + 1; k < featNbLp[i]; k++ )
            {

              lpid = featStartId[i] + k;
              ok[lpid] = true;
              lp2 = mLabelPositions.at( lpid );

              lp2->getBoundingBox( amin, amax );

              nbOverlap -= lp2->getNumOverlaps();
              candidates->Search( amin, amax, LabelPosition::removeOverlapCallback, reinterpret_cast< void* >( lp2 ) );
              lp2->removeFromIndex( candidates );
            }

            featNbLp[i] = j + 1;
            break;
          }
        }
      }
    }
  }

  this->nblp -= counter;
  delete[] ok;
}

void Problem::init_sol_empty()
{
  int i;

  if ( sol )
  {
    if ( sol->s )
      delete[] sol->s;
    delete sol;
  }

  sol = new Sol();
  sol->s = new int[nbft];

  for ( i = 0; i < nbft; i++ )
    sol->s[i] = -1;

  sol->cost = nbft;
}




typedef struct
{
  PriorityQueue *list;
  LabelPosition *lp;
  RTree <LabelPosition*, double, 2, double> *candidates;
} FalpContext;

bool falpCallback2( LabelPosition *lp, void* ctx )
{
  FalpContext* context = reinterpret_cast< FalpContext* >( ctx );
  LabelPosition *lp2 = context->lp;
  PriorityQueue *list = context->list;

  if ( lp->getId() != lp2->getId() && list->isIn( lp->getId() ) && lp->isInConflict( lp2 ) )
  {
    list->decreaseKey( lp->getId() );
  }
  return true;
}


void ignoreLabel( LabelPosition *lp, PriorityQueue *list, RTree <LabelPosition*, double, 2, double> *candidates )
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


bool falpCallback1( LabelPosition *lp, void* ctx )
{
  FalpContext* context = reinterpret_cast< FalpContext* >( ctx );
  LabelPosition *lp2 = context->lp;
  PriorityQueue *list = context->list;
  RTree <LabelPosition*, double, 2, double> *candidates = context->candidates;

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
  PriorityQueue *list;

  init_sol_empty();

  list = new PriorityQueue( nblp, all_nblp, true );

  double amin[2];
  double amax[2];

  FalpContext *context = new FalpContext();
  context->candidates = candidates;
  context->list = list;

  LabelPosition *lp;

  for ( i = 0; i < nbft; i++ )
    for ( j = 0; j < featNbLp[i]; j++ )
    {
      label = featStartId[i] + j;
      try
      {
        list->insert( label, mLabelPositions.at( label )->getNumOverlaps() );
      }
      catch ( pal::InternalException::Full )
      {
        continue;
      }
    }

  while ( list->getSize() > 0 ) // O (log size)
  {
    if ( pal->isCancelled() )
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
    sol->s[probFeatId] = label;

    for ( i = featStartId[probFeatId]; i < featStartId[probFeatId] + featNbLp[probFeatId]; i++ )
    {
      ignoreLabel( mLabelPositions.at( i ), list, candidates );
    }


    lp->getBoundingBox( amin, amax );

    context->lp = lp;
    candidates->Search( amin, amax, falpCallback1, reinterpret_cast< void* >( context ) );
    candidates_sol->Insert( amin, amax, lp );
  }

  delete context;




  if ( displayAll )
  {
    int nbOverlap;
    int start_p;
    LabelPosition* retainedLabel = nullptr;
    int p;

    for ( i = 0; i < nbft; i++ ) // forearch hidden feature
    {
      if ( sol->s[i] == -1 )
      {
        nbOverlap = INT_MAX;
        start_p = featStartId[i];
        for ( p = 0; p < featNbLp[i]; p++ )
        {
          lp = mLabelPositions.at( start_p + p );
          lp->resetNumOverlaps();

          lp->getBoundingBox( amin, amax );


          candidates_sol->Search( amin, amax, LabelPosition::countOverlapCallback, lp );

          if ( lp->getNumOverlaps() < nbOverlap )
          {
            retainedLabel = lp;
            nbOverlap = lp->getNumOverlaps();
          }
        }
        sol->s[i] = retainedLabel->getId();

        retainedLabel->insertIntoIndex( candidates_sol );

      }
    }
  }

  delete list;
}

void Problem::popmusic()
{

  if ( nbft == 0 )
    return;

  int i;
  int seed;
  bool *ok = new bool[nbft];

  int r = pal->popmusic_r;

  SearchMethod searchMethod = pal->searchMethod;

  candidates_subsol = new RTree<LabelPosition*, double, 2, double>();

  double delta = 0.0;

  int it = 0;

  SubPart *current = nullptr;

  labelPositionCost = new double[all_nblp];
  nbOlap = new int[all_nblp];

  featWrap = new int[nbft];
  memset( featWrap, -1, sizeof( int ) *nbft );

  SubPart ** parts = new SubPart*[nbft];
  int *isIn = new int[nbft];

  memset( isIn, 0, sizeof( int ) *nbft );


  for ( i = 0; i < nbft; i++ )
  {
    parts[i] = subPart( r, i, isIn );
    ok[i] = false;
  }
  delete[] isIn;
  Util::sort( reinterpret_cast< void** >( parts ), nbft, borderSizeInc );
  //sort ((void**)parts, nbft, borderSizeDec);

  init_sol_falp();

  solution_cost();

  int popit = 0;

  seed = 0;
  while ( true )
  {
    it++;
    /* find the next seed not ok */
    for ( i = ( seed + 1 ) % nbft; ok[i] && i != seed; i = ( i + 1 ) % nbft )
      ;

    if ( i == seed && ok[seed] )
    {
      current = nullptr; // everything is OK :-)
      break;
    }
    else
    {
      seed = i;
      current = parts[seed];
    }

    // update sub part solution
    candidates_subsol->RemoveAll();

    for ( i = 0; i < current->subSize; i++ )
    {
      current->sol[i] = sol->s[current->sub[i]];
      if ( current->sol[i] != -1 )
      {
        mLabelPositions.at( current->sol[i] )->insertIntoIndex( candidates_subsol );
      }
    }

    switch ( searchMethod )
    {
        //case branch_and_bound :
        //delta = current->branch_and_bound_search();
        //   break;

      case POPMUSIC_TABU :
        delta = popmusic_tabu( current );
        break;
      case POPMUSIC_TABU_CHAIN :
        delta = popmusic_tabu_chain( current );
        break;
      case POPMUSIC_CHAIN :
        delta = popmusic_chain( current );
        break;
      default:
        delete[] ok;
        delete[] parts;
        return;
    }

    popit++;

    if ( delta > EPSILON )
    {
      /* Update solution */
      for ( i = 0; i < current->borderSize; i++ )
      {
        ok[current->sub[i]] = false;
      }

      for ( i = current->borderSize; i < current->subSize; i++ )
      {

        if ( sol->s[current->sub[i]] != -1 )
        {
          mLabelPositions.at( sol->s[current->sub[i]] )->removeFromIndex( candidates_sol );
        }

        sol->s[current->sub[i]] = current->sol[i];

        if ( current->sol[i] != -1 )
        {
          mLabelPositions.at( current->sol[i] )->insertIntoIndex( candidates_sol );
        }

        ok[current->sub[i]] = false;
      }
    }
    else  // not improved
    {
      ok[seed] = true;
    }
  }

  solution_cost();

  delete[] labelPositionCost;
  delete[] nbOlap;

  for ( i = 0; i < nbft; i++ )
  {
    delete[] parts[i]->sub;
    delete[] parts[i]->sol;
    delete parts[i];
  }
  delete[] parts;

  delete[] ok;

  return;
}

typedef struct
{
  QLinkedList<int> *queue;
  int *isIn;
  LabelPosition *lp;
} SubPartContext;

bool subPartCallback( LabelPosition *lp, void *ctx )
{
  SubPartContext* context = reinterpret_cast< SubPartContext* >( ctx );
  int *isIn = context->isIn;
  QLinkedList<int> *queue = context->queue;


  int id = lp->getProblemFeatureId();
  if ( !isIn[id] && lp->isInConflict( context->lp ) )
  {
    queue->append( id );
    isIn[id] = 1;
  }

  return true;
}

/* Select a sub part, expected size of r, from seed */
SubPart * Problem::subPart( int r, int featseed, int *isIn )
{
  QLinkedList<int> *queue = new QLinkedList<int>;
  QLinkedList<int> *ri = new QLinkedList<int>;

  int *sub;

  int id;
  int featS;
  int p;
  int i;

  int n = 0;
  int nb = 0;

  double amin[2];
  double amax[2];

  SubPartContext context;
  context.queue = queue;
  context.isIn = isIn;

  queue->append( featseed );
  isIn[featseed] = 1;

  LabelPosition *lp;

  while ( ri->size() < r && !queue->isEmpty() )
  {
    id = queue->takeFirst();
    ri->append( id );

    featS = featStartId[id];
    p = featNbLp[id];

    for ( i = featS; i < featS + p; i++ )  // foreach candidat of feature 'id'
    {
      lp = mLabelPositions.at( i );

      lp->getBoundingBox( amin, amax );

      context.lp = lp;
      candidates->Search( amin, amax, subPartCallback, reinterpret_cast< void* >( &context ) );
    }
  }

  nb = queue->size();
  n = ri->size();

  sub = new int[n+nb];

  i = 0;

  while ( !queue->isEmpty() )
  {
    sub[i] = queue->takeFirst();
    isIn[sub[i]] = 0;
    i++;
  }

  while ( !ri->isEmpty() )
  {
    sub[i] = ri->takeFirst();
    isIn[sub[i]] = 0;
    i++;
  }

  delete queue;
  delete ri;

  SubPart *subPart = new SubPart();

  subPart->probSize = n;
  subPart->borderSize = nb;
  subPart->subSize = n + nb;
  subPart->sub = sub;
  subPart->sol = new int [subPart->subSize];
  subPart->seed = featseed;
  return subPart;
}

double Problem::compute_feature_cost( SubPart *part, int feat_id, int label_id, int *nbOverlap )
{
  double cost;
  *nbOverlap = 0;

  LabelPosition::CountContext context;
  context.inactiveCost = inactiveCost;
  context.nbOv = nbOverlap;
  context.cost = &cost;

  double amin[2];
  double amax[2];
  LabelPosition *lp;

  cost = 0.0;

  if ( label_id >= 0 ) // is the feature displayed ?
  {
    lp = mLabelPositions.at( label_id );

    lp->getBoundingBox( amin, amax );

    context.lp = lp;
    candidates_subsol->Search( amin, amax, LabelPosition::countFullOverlapCallback, reinterpret_cast< void* >( &context ) );

    cost += lp->cost();
  }
  else
  {
    cost = inactiveCost[part->sub[feat_id]];
    //(*nbOverlap)++;
  }

  return cost;

}

double Problem::compute_subsolution_cost( SubPart *part, int *s, int *nbOverlap )
{
  int i;
  double cost = 0.0;
  int nbO = 0;

  *nbOverlap = 0;

  for ( i = 0; i < part->subSize; i++ )
  {
    cost += compute_feature_cost( part, i, s[i], &nbO );
    *nbOverlap += nbO;
  }

  return cost;
}



typedef struct _Triple
{
  int feat_id;
  int label_id;
  double cost;
  int nbOverlap;
} Triple;


bool decreaseCost( void *tl, void *tr )
{
  return ( reinterpret_cast< Triple* >( tl ) )->cost < ( reinterpret_cast< Triple* >( tr ) )->cost;
}

inline void actualizeTabuCandidateList( int m, int iteration, int nbOverlap, int *candidateListSize,
                                        double candidateBaseFactor, double *candidateFactor,
                                        int minCandidateListSize, double reductionFactor,
                                        int minTabuTSize, double tabuFactor, int *tenure, int n )
{

  if ( *candidateFactor > candidateBaseFactor )
    *candidateFactor /= reductionFactor;

  if ( iteration % m == 0 )
  {
    *tenure = minTabuTSize + static_cast< int >( tabuFactor * nbOverlap );
    *candidateListSize = minCandidateListSize + static_cast< int >( *candidateFactor * nbOverlap );

    if ( *candidateListSize > n )
      *candidateListSize = n;
  }

}


inline void actualizeCandidateList( int nbOverlap, int *candidateListSize, double candidateBaseFactor,
                                    double *candidateFactor, int minCandidateListSize, double growingFactor, int n )
{

  candidateBaseFactor += 0;

  if ( *candidateListSize < n )
    *candidateFactor = *candidateFactor * growingFactor;
  *candidateListSize = minCandidateListSize + static_cast< int >( *candidateFactor * nbOverlap );

  if ( *candidateListSize > n )
    *candidateListSize = n;
}




typedef struct
{
  LabelPosition *lp;
  Triple **candidates;
  double *labelPositionCost;
  int *nbOlap;
  double diff_cost;
  int *featWrap;
  int *sol;
  int borderSize;
} UpdateContext;

bool updateCandidatesCost( LabelPosition *lp, void *context )
{
  UpdateContext *ctx = reinterpret_cast< UpdateContext* >( context );

  if ( ctx->lp->isInConflict( lp ) )
  {
    ctx->labelPositionCost[lp->getId()] += ctx->diff_cost;
    if ( ctx->diff_cost > 0 )
      ctx->nbOlap[lp->getId()]++;
    else
      ctx->nbOlap[lp->getId()]--;

    int feat_id = ctx->featWrap[ctx->lp->getProblemFeatureId()];
    int feat_id2;
    if ( feat_id >= 0 && ctx->sol[feat_id] == lp->getId() ) // this label is in use
    {
      if (( feat_id2 = feat_id - ctx->borderSize ) >= 0 )
      {
        ctx->candidates[feat_id2]->cost += ctx->diff_cost;
        ctx->candidates[feat_id2]->nbOverlap--;
      }
    }
  }
  return true;
}




double Problem::popmusic_tabu( SubPart *part )
{
  int probSize = part->probSize;
  int borderSize = part->borderSize;
  int subSize = part->subSize;
  int *sub = part->sub;
  int *sol = part->sol;

  Triple **candidateList = new Triple*[probSize];
  Triple **candidateListUnsorted = new Triple*[probSize];

  int * best_sol = new int[subSize];

  double cur_cost;
  double best_cost;
  double initial_cost;

  int *tabu_list = new int[probSize];

  int i;
  int j;

  int itwImp;
  int it = 0;
  int max_it;
  int stop_it;

  double delta;
  double delta_min;
  bool authorized;

  int feat_id;
  int feat_sub_id;
  int label_id;
  int p;

  int choosed_feat;
  int choosed_label;
  int candidateId;

  int nbOverlap = 0;
  //int nbOverlapLabel;


  int tenure = 10;  //
  int m = 50; // m   [10;50]

  int minTabuTSize = 9; // min_t [2;10]
  double tabuFactor = 0.5; // p_t [0.1;0.8]

  int minCandidateListSize = 18; // min_c   [2;20]

  double candidateBaseFactor = 0.73; // p_base  [0.1;0.8]

  double growingFactor = 15; // fa  [5;20]
  double reductionFactor = 1.3; // f_r [1.1;1.5]

  int candidateListSize = minCandidateListSize;
  double candidateFactor = candidateBaseFactor;


  int first_lp;

  //double EPSILON = 0.000001;
  max_it = probSize * pal->tabuMaxIt;
  itwImp = probSize * pal->tabuMinIt;
  stop_it = itwImp;

  cur_cost = 0.0;
  nbOverlap = 0;


  int lp;
  for ( i = 0; i < subSize; i++ )
    featWrap[sub[i]] = i;

  for ( i = 0; i < subSize; i++ )
  {
    j = featStartId[sub[i]];
    for ( lp = 0; lp < featNbLp[sub[i]]; lp++ )
    {
      it = j + lp;
      labelPositionCost[it] = compute_feature_cost( part, i, it, & ( nbOlap[it] ) );
    }
  }

  first_lp = ( displayAll ? 0 : -1 );
  for ( i = 0; i < probSize; i++ )
  {

    tabu_list[i] = -1; // nothing is tabu

    candidateList[i] = new Triple();
    candidateList[i]->feat_id = i + borderSize;
    candidateList[i]->label_id = sol[i+borderSize];

    candidateListUnsorted[i] = candidateList[i];

    if ( sol[i+borderSize] >= 0 )
    {
      j = sol[i+borderSize];
      candidateList[i]->cost = labelPositionCost[j];
      candidateList[i]->nbOverlap = nbOlap[j];
    }
    else
    {
      candidateList[i]->cost = inactiveCost[sub[i+borderSize]];
      candidateList[i]->nbOverlap = 1;
    }

  }


  for ( i = 0; i < subSize; i++ )
  {
    if ( sol[i] == -1 )
    {
      cur_cost += inactiveCost[sub[i]];
    }
    else
    {
      nbOverlap += nbOlap[sol[i]];
      cur_cost += labelPositionCost[sol[i]];
    }
  }

  Util::sort( reinterpret_cast< void** >( candidateList ), probSize, decreaseCost );

  best_cost = cur_cost;
  initial_cost = cur_cost;
  memcpy( best_sol, sol, sizeof( int ) *( subSize ) );

  // START TABU

  it = 0;
  while ( it < stop_it && best_cost >= EPSILON )
  {
    actualizeTabuCandidateList( m, it, nbOverlap, &candidateListSize, candidateBaseFactor, &candidateFactor, minCandidateListSize, reductionFactor, minTabuTSize, tabuFactor, &tenure, probSize );
    delta_min     = DBL_MAX;
    choosed_feat  = -1;
    choosed_label = -2;
    candidateId   = -1;

    // foreach retained Candidate
    for ( i = 0; i < candidateListSize; i++ )
    {
      feat_id     = candidateList[i]->feat_id;
      feat_sub_id = sub[feat_id];
      label_id    = candidateList[i]->label_id;

      int oldPos  = ( label_id < 0 ? -1 : label_id - featStartId[feat_sub_id] );


      p = featNbLp[feat_sub_id];

      /* label -1 means inactive feature */
      // foreach labelposition of feature minus the one in the solution
      for ( j = first_lp; j < p; j++ )
      {
        if ( j != oldPos )
        {

          if ( sol[feat_id] < 0 )
          {
            delta = -inactiveCost[feat_sub_id];
          }
          else
          {
            delta = -labelPositionCost[sol[feat_id]];
            delta -= nbOlap[sol[feat_id]] * ( inactiveCost[feat_sub_id] + mLabelPositions.at( label_id )->cost() );
          }

          if ( j >= 0 )
          {
            delta += labelPositionCost[featStartId[feat_sub_id] + j];
            delta += nbOlap[featStartId[feat_sub_id] + j] * ( inactiveCost[feat_sub_id] + mLabelPositions.at( featStartId[feat_sub_id] + j )->cost() );
          }
          else
          {
            delta += inactiveCost[feat_sub_id];
          }

          // move is authorized wether the feat isn't taboo or whether the move give a new best solution
          authorized = ( tabu_list[feat_id - borderSize] <= it ) || ( cur_cost + delta < best_cost );

          if ( delta < delta_min && authorized )
          {
            choosed_feat = feat_id;

            if ( j == -1 )
              choosed_label = -1;
            else
              choosed_label = featStartId[feat_sub_id] + j;

            delta_min = delta;
            candidateId = i;
          }
        }
      }
    }

    // if a modification has been retained
    if ( choosed_feat >= 0 )
    {
      // update the solution and update tabu list
      int old_label = sol[choosed_feat];

      tabu_list[choosed_feat-borderSize] = it + tenure;
      sol[choosed_feat] = choosed_label;
      candidateList[candidateId]->label_id = choosed_label;

      if ( old_label != -1 )
        mLabelPositions.at( old_label )->removeFromIndex( candidates_subsol );

      /* re-compute all labelpositioncost that overlap with old an new label */
      double local_inactive = inactiveCost[sub[choosed_feat]];

      if ( choosed_label != -1 )
      {
        candidateList[candidateId]->cost = labelPositionCost[choosed_label];
        candidateList[candidateId]->nbOverlap = nbOlap[choosed_label];
      }
      else
      {
        candidateList[candidateId]->cost = local_inactive;
        candidateList[candidateId]->nbOverlap = 1;
      }

      cur_cost += delta_min;

      double amin[2];
      double amax[2];
      LabelPosition *lp;

      UpdateContext context;

      context.candidates = candidateListUnsorted;
      context.labelPositionCost = labelPositionCost;
      context.nbOlap = nbOlap;
      context.featWrap = featWrap;
      context.sol = sol;
      context.borderSize = borderSize;

      if ( old_label >= 0 )
      {
        lp = mLabelPositions.at( old_label );

        lp->getBoundingBox( amin, amax );

        context.diff_cost = -local_inactive - lp->cost();
        context.lp = lp;

        candidates->Search( amin, amax, updateCandidatesCost, &context );
      }

      if ( choosed_label >= 0 )
      {
        lp = mLabelPositions.at( choosed_label );

        lp->getBoundingBox( amin, amax );

        context.diff_cost = local_inactive + lp->cost();
        context.lp = lp;


        candidates->Search( amin, amax, updateCandidatesCost, &context );

        lp->insertIntoIndex( candidates_subsol );
      }

      Util::sort( reinterpret_cast< void** >( candidateList ), probSize, decreaseCost );

      if ( best_cost - cur_cost > EPSILON ) // new best sol
      {
        best_cost = cur_cost;
        memcpy( best_sol, sol, sizeof( int ) *( subSize ) );
        stop_it = it + itwImp;
        if ( stop_it > max_it )
          stop_it = max_it;
      }
    }
    else
    {
      /* no feature was selected : increase candidate list size*/
      actualizeCandidateList( nbOverlap, &candidateListSize, candidateBaseFactor,
                              &candidateFactor, minCandidateListSize, growingFactor, probSize );
    }
    it++;
  }

  memcpy( sol, best_sol, sizeof( int ) *( subSize ) );

  for ( i = 0; i < subSize; i++ )
    featWrap[sub[i]] = -1;

  for ( i = 0; i < probSize; i++ )
    delete candidateList[i];

  delete[] candidateList;
  delete[] candidateListUnsorted;
  delete[] best_sol;
  delete[] tabu_list;

  /* Return delta */
  return initial_cost - best_cost;
}





typedef struct
{
  LabelPosition *lp;
  int *tmpsol;
  int *featWrap;
  int *feat;
  int borderSize;
  QLinkedList<ElemTrans*> *currentChain;
  QLinkedList<int> *conflicts;
  double *delta_tmp;
  double *inactiveCost;

} ChainContext;


bool chainCallback( LabelPosition *lp, void *context )
{
  ChainContext *ctx = reinterpret_cast< ChainContext* >( context );

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

    if ( feat >= 0 && ctx->tmpsol[feat] == lp->getId() )
    {
      if ( sub && feat < ctx->borderSize )
      {
        throw - 2;
      }
    }

    // is there any cycles ?
    QLinkedList< ElemTrans* >::iterator cur;
    for ( cur = ctx->currentChain->begin(); cur != ctx->currentChain->end(); ++cur )
    {
      if (( *cur )->feat == feat )
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

inline Chain *Problem::chain( SubPart *part, int seed )
{

  int i;
  int j;

  int lid;

  //int probSize   = part->probSize;
  int borderSize = part->borderSize;
  int subSize    = part->subSize;
  int *sub       = part->sub;
  int *sol       = part->sol;
  int subseed;

  double delta;
  double delta_min;
  double delta_best = DBL_MAX;
  double delta_tmp;

  int next_seed;
  int retainedLabel;
  Chain *retainedChain = nullptr;

  int max_degree = pal->ejChainDeg;

  int seedNbLp;

  QLinkedList<ElemTrans*> *currentChain = new QLinkedList<ElemTrans*>;
  QLinkedList<int> *conflicts = new QLinkedList<int>;

  int *tmpsol = new int[subSize];
  memcpy( tmpsol, sol, sizeof( int ) *subSize );

  LabelPosition *lp;
  double amin[2];
  double amax[2];

  ChainContext context;
  context.featWrap = featWrap;
  context.borderSize = borderSize;
  context.tmpsol = tmpsol;
  context.inactiveCost = inactiveCost;
  context.feat = nullptr;
  context.currentChain = currentChain;
  context.conflicts = conflicts;
  context.delta_tmp = &delta_tmp;

  delta = 0;
  while ( seed != -1 )
  {
    subseed = sub[seed];
    seedNbLp = featNbLp[subseed];
    delta_min = DBL_MAX;
    next_seed = -1;
    retainedLabel = -2;


    if ( tmpsol[seed] == -1 )
      delta -= inactiveCost[subseed];
    else
      delta -= mLabelPositions.at( tmpsol[seed] )->cost();

    // TODO modify to handle displayAll param
    for ( i = -1; i < seedNbLp; i++ )
    {
      try
      {
        // Skip active label !
        if ( !( tmpsol[seed] == -1 && i == -1 ) && i + featStartId[subseed] != tmpsol[seed] )
        {
          if ( i != -1 )
          {
            lid = featStartId[subseed] + i;
            delta_tmp = delta;

            lp = mLabelPositions.at( lid );

            // evaluate conflicts graph in solution after moving seed's label
            lp->getBoundingBox( amin, amax );

            context.lp = lp;

            // search ative conflicts and count them
            candidates_subsol->Search( amin, amax, chainCallback, reinterpret_cast< void* >( &context ) );

            // no conflict -> end of chain
            if ( conflicts->isEmpty() )
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
                  retainedChain = new Chain(); // HERE
                }

                delta_best = delta + lp->cost();

                retainedChain->degree = currentChain->size() + 1;
                retainedChain->feat  = new int[retainedChain->degree]; // HERE
                retainedChain->label = new int[retainedChain->degree]; // HERE
                QLinkedList< ElemTrans* >::iterator current = currentChain->begin();
                ElemTrans* move;
                j = 0;
                while ( current != currentChain->end() )
                {
                  move = *current;
                  retainedChain->feat[j]  = move->feat;
                  retainedChain->label[j] = move->new_label;
                  ++current;
                  ++j;
                }
                retainedChain->feat[j] = seed;
                retainedChain->label[j] = lid;
                retainedChain->delta = delta + mLabelPositions.at( retainedChain->label[j] )->cost();
              }
            }

            // another feature can be ejected
            else if ( conflicts->size() == 1 )
            {
              if ( delta_tmp < delta_min )
              {
                delta_min = delta_tmp;
                retainedLabel = lid;
                next_seed =  conflicts->takeFirst();
              }
              else
              {
                conflicts->takeFirst();
              }
            }
            else
            {

              // A lot of conflict : make them inactive and store chain
              Chain *newChain = new Chain();  // HERE
              newChain->degree = currentChain->size() + 1 + conflicts->size();
              newChain->feat  = new int[newChain->degree]; // HERE
              newChain->label = new int[newChain->degree]; // HERE
              QLinkedList<ElemTrans*>::iterator current = currentChain->begin();
              ElemTrans* move;
              j = 0;
              while ( current != currentChain->end() )
              {
                move = *current;
                newChain->feat[j]  = move->feat;
                newChain->label[j] = move->new_label;
                ++current;
                ++j;
              }

              newChain->feat[j] = seed;
              newChain->label[j] = lid;
              newChain->delta = delta + mLabelPositions.at( newChain->label[j] )->cost();
              j++;


              while ( !conflicts->isEmpty() )
              {
                int ftid = conflicts->takeFirst();
                newChain->feat[j] = ftid;
                newChain->label[j] = -1;
                newChain->delta += inactiveCost[sub[ftid]];
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
            if ( !retainedChain || delta + inactiveCost[subseed] < delta_best )
            {
              if ( retainedChain )
              {
                delete[] retainedChain->label;
                delete[] retainedChain->feat;
              }
              else
                retainedChain = new Chain(); // HERE

              delta_best = delta + inactiveCost[subseed];

              retainedChain->degree = currentChain->size() + 1;
              retainedChain->feat  = new int[retainedChain->degree]; // HERE
              retainedChain->label = new int[retainedChain->degree]; // HERE
              QLinkedList<ElemTrans*>::iterator current = currentChain->begin();
              ElemTrans* move;
              j = 0;
              while ( current != currentChain->end() )
              {
                move = *current;
                retainedChain->feat[j]  = move->feat;
                retainedChain->label[j] = move->new_label;
                ++current;
                ++j;
              }
              retainedChain->feat[j] = seed;
              retainedChain->label[j] = -1;
              retainedChain->delta = delta + inactiveCost[subseed];
            }
          }
        }
      }
      catch ( int i )
      {
        Q_UNUSED( i );
        conflicts->clear();
      }
    } // end foreach labelposition

    if ( next_seed == -1 )
    {
      seed = -1;
    }
    else if ( currentChain->size() > max_degree )
    {
      seed = -1;
    }
    else
    {
      ElemTrans *et = new ElemTrans();
      et->feat  = seed;
      et->old_label = tmpsol[seed];
      et->new_label = retainedLabel;
      currentChain->append( et );

      if ( et->old_label != -1 )
      {
        mLabelPositions.at( et->old_label )->removeFromIndex( candidates_subsol );
      }

      if ( et->new_label != -1 )
      {
        mLabelPositions.at( et->new_label )->insertIntoIndex( candidates_subsol );
      }

      tmpsol[seed] = retainedLabel;
      delta += mLabelPositions.at( retainedLabel )->cost();
      seed = next_seed;
    }
  }

  while ( !currentChain->isEmpty() )
  {
    ElemTrans* et =  currentChain->takeFirst();

    if ( et->new_label != -1 )
    {
      mLabelPositions.at( et->new_label )->removeFromIndex( candidates_subsol );
    }

    if ( et->old_label != -1 )
    {
      mLabelPositions.at( et->old_label )->insertIntoIndex( candidates_subsol );
    }

    delete et;
  }
  delete currentChain;

  delete[] tmpsol;
  delete conflicts;


  return retainedChain;
}


inline Chain *Problem::chain( int seed )
{

  int i;
  int j;

  int lid;

  double delta;
  double delta_min;
  double delta_best = DBL_MAX;
  double delta_tmp;

  int next_seed;
  int retainedLabel;
  Chain *retainedChain = nullptr;

  int max_degree = pal->ejChainDeg;

  int seedNbLp;

  QLinkedList<ElemTrans*> *currentChain = new QLinkedList<ElemTrans*>;
  QLinkedList<int> *conflicts = new QLinkedList<int>;

  int *tmpsol = new int[nbft];
  memcpy( tmpsol, sol->s, sizeof( int ) *nbft );

  LabelPosition *lp;
  double amin[2];
  double amax[2];

  ChainContext context;
  context.featWrap = nullptr;
  context.borderSize = 0;
  context.tmpsol = tmpsol;
  context.inactiveCost = inactiveCost;
  context.feat = nullptr;
  context.currentChain = currentChain;
  context.conflicts = conflicts;
  context.delta_tmp = &delta_tmp;

  delta = 0;
  while ( seed != -1 )
  {
    seedNbLp = featNbLp[seed];
    delta_min = DBL_MAX;

    next_seed = -1;
    retainedLabel = -2;

    // sol[seed] is ejected
    if ( tmpsol[seed] == -1 )
      delta -= inactiveCost[seed];
    else
      delta -= mLabelPositions.at( tmpsol[seed] )->cost();

    for ( i = -1; i < seedNbLp; i++ )
    {
      try
      {
        // Skip active label !
        if ( !( tmpsol[seed] == -1 && i == -1 ) && i + featStartId[seed] != tmpsol[seed] )
        {
          if ( i != -1 ) // new_label
          {
            lid = featStartId[seed] + i;
            delta_tmp = delta;

            lp = mLabelPositions.at( lid );

            // evaluate conflicts graph in solution after moving seed's label
            lp->getBoundingBox( amin, amax );

            context.lp = lp;

            candidates_sol->Search( amin, amax, chainCallback, reinterpret_cast< void* >( &context ) );

            // no conflict -> end of chain
            if ( conflicts->isEmpty() )
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

                retainedChain->degree = currentChain->size() + 1;
                retainedChain->feat  = new int[retainedChain->degree];
                retainedChain->label = new int[retainedChain->degree];
                QLinkedList<ElemTrans*>::iterator current = currentChain->begin();
                ElemTrans* move;
                j = 0;
                while ( current != currentChain->end() )
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
            else if ( conflicts->size() == 1 )
            {
              if ( delta_tmp < delta_min )
              {
                delta_min = delta_tmp;
                retainedLabel = lid;
                next_seed =  conflicts->takeFirst();
              }
              else
              {
                conflicts->takeFirst();
              }
            }
            else
            {

              // A lot of conflict : make them inactive and store chain
              Chain *newChain = new Chain();
              newChain->degree = currentChain->size() + 1 + conflicts->size();
              newChain->feat  = new int[newChain->degree];
              newChain->label = new int[newChain->degree];
              QLinkedList<ElemTrans*>::iterator current = currentChain->begin();
              ElemTrans* move;
              j = 0;

              while ( current != currentChain->end() )
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
              while ( !conflicts->isEmpty() )
              {
                int ftid = conflicts->takeFirst();
                newChain->feat[j] = ftid;
                newChain->label[j] = -1;
                newChain->delta += inactiveCost[ftid];
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
            if ( !retainedChain || delta + inactiveCost[seed] < delta_best )
            {
              if ( retainedChain )
              {
                delete[] retainedChain->label;
                delete[] retainedChain->feat;
              }
              else
                retainedChain = new Chain();

              delta_best = delta + inactiveCost[seed];

              retainedChain->degree = currentChain->size() + 1;
              retainedChain->feat  = new int[retainedChain->degree];
              retainedChain->label = new int[retainedChain->degree];
              QLinkedList<ElemTrans*>::iterator current = currentChain->begin();
              ElemTrans* move;
              j = 0;
              while ( current != currentChain->end() )
              {
                move = *current;
                retainedChain->feat[j]  = move->feat;
                retainedChain->label[j] = move->new_label;
                ++current;
                ++j;
              }
              retainedChain->feat[j] = seed;
              retainedChain->label[j] = -1;
              retainedChain->delta = delta + inactiveCost[seed];
            }
          }
        }
      }
      catch ( int i )
      {
        Q_UNUSED( i );
        conflicts->clear();
      }
    } // end foreach labelposition

    if ( next_seed == -1 )
    {
      seed = -1;
    }
    else if ( currentChain->size() > max_degree )
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
      currentChain->append( et );

      if ( et->old_label != -1 )
      {
        mLabelPositions.at( et->old_label )->removeFromIndex( candidates_sol );
      }

      if ( et->new_label != -1 )
      {
        mLabelPositions.at( et->new_label )->insertIntoIndex( candidates_sol );
      }


      tmpsol[seed] = retainedLabel;
      delta += mLabelPositions.at( retainedLabel )->cost();
      seed = next_seed;
    }
  }


  while ( !currentChain->isEmpty() )
  {
    ElemTrans* et =  currentChain->takeFirst();

    if ( et->new_label != -1 )
    {
      mLabelPositions.at( et->new_label )->removeFromIndex( candidates_sol );
    }

    if ( et->old_label != -1 )
    {
      mLabelPositions.at( et->old_label )->insertIntoIndex( candidates_sol );
    }

    delete et;
  }
  delete currentChain;

  delete[] tmpsol;
  delete conflicts;


  return retainedChain;
}

double Problem::popmusic_chain( SubPart *part )
{
  int i;
  //int j;

  int probSize   = part->probSize;
  int borderSize = part->borderSize;
  int subSize    = part->subSize;
  int *sub       = part->sub;
  int *sol       = part->sol;

  int *best_sol = new int[subSize];

  for ( i = 0; i < subSize; i++ )
  {
    featWrap[sub[i]] = i;
    best_sol[i] = sol[i];
  }

  double initial_cost;
  double cur_cost = 0;
  double best_cost = 0;

  // int nbOverlap = 0;

  int seed;

  int featOv;

  int lid;
  int fid;

  int *tabu_list = new int[subSize];

  Chain *current_chain = nullptr;

  //int itC;

  int it;
  int stop_it;
  int maxit;
  int itwimp; // iteration without improvment

  int tenure = pal->tenure;

  for ( i = 0; i < subSize; i++ )
  {
    cur_cost += compute_feature_cost( part, i, sol[i], &featOv );
    // nbOverlap += featOv;
  }

  initial_cost = cur_cost;
  best_cost = cur_cost;

  it = 0;

  maxit = probSize * pal->tabuMaxIt;

  itwimp = probSize * pal->tabuMinIt;

  stop_it = itwimp;

  // feature on border always are tabu
  for ( i = 0; i < borderSize; i++ )
    tabu_list[i] = maxit;   // border always are taboo

  for ( i = 0; i < probSize; i++ )
    tabu_list[i+borderSize] = -1; // others aren't

  while ( it < stop_it )
  {
    seed = ( it % probSize ) + borderSize;

    current_chain = chain( part, seed );
    if ( current_chain )
    {

      /* we accept a modification only if the seed is not tabu or
       * if the nmodification will generate a new best solution */
      if ( tabu_list[seed] < it || ( cur_cost + current_chain->delta ) - best_cost < 0.0 )
      {

        for ( i = 0; i < current_chain->degree; i++ )
        {
          fid = current_chain->feat[i];
          lid = current_chain->label[i];

          if ( sol[fid] >= 0 )
          {
            mLabelPositions.at( sol[fid] )->removeFromIndex( candidates_subsol );
          }
          sol[fid] = lid;

          if ( sol[fid] >= 0 )
          {
            mLabelPositions.at( lid )->insertIntoIndex( candidates_subsol );
          }

          tabu_list[fid] = it + tenure;
        }

        cur_cost += current_chain->delta;

        /* check if new solution is a new best solution */
        if ( best_cost - cur_cost > EPSILON )
        {
          best_cost = cur_cost;
          memcpy( best_sol, sol, sizeof( int ) *subSize );

          stop_it = ( it + itwimp > maxit ? maxit : it + itwimp );
        }
      }
      delete_chain( current_chain );
    }
    it++;
  }

  memcpy( sol, best_sol, sizeof( int ) *subSize );

  /*
  for (i=borderSize;i<subSize;i++){
     chain =  chain (part, i);
     if (chain){
        if (chain->delta < 0.0){
           best_cost += chain->delta;
           for (j=0;j<chain->degree;j++){
              fid = chain->feat[j];
              lid = chain->label[j];
              sol[fid] = lid;
           }
        }

        delete_chain(chain);
     }
  }
  */

  for ( i = 0; i < subSize; i++ )
    featWrap[sub[i]] = -1;

  delete[] best_sol;
  delete[] tabu_list;


  return initial_cost - best_cost;
}

double Problem::popmusic_tabu_chain( SubPart *part )
{
  int i;

  int probSize   = part->probSize;
  int borderSize = part->borderSize;
  int subSize    = part->subSize;
  int *sub       = part->sub;
  int *sol       = part->sol;

  int *best_sol = new int[subSize];

  for ( i = 0; i < subSize; i++ )
  {
    featWrap[sub[i]] = i;
  }

  double initial_cost;
  double cur_cost = 0;
  double best_cost = 0;

  // int nbOverlap = 0;

  int seed;

  int featOv;

  int lid;
  int fid;

  int *tabu_list = new int[subSize];

  Chain *retainedChain = nullptr;
  Chain *current_chain = nullptr;

  int itC;

  int it;
  int stop_it;
  int maxit;
  int itwimp;

  int tenure = pal->tenure;

  //int deltaIt = 0;

  Triple **candidates = new Triple*[probSize];
  Triple **candidatesUnsorted = new Triple*[probSize];

  for ( i = 0; i < subSize; i++ )
  {
    cur_cost += compute_feature_cost( part, i, sol[i], &featOv );
    // nbOverlap += featOv;
  }

  initial_cost = cur_cost;
  best_cost = cur_cost;

  it = 0;

  maxit = probSize * pal->tabuMaxIt;

  itwimp = probSize * pal->tabuMinIt;

  stop_it = itwimp;

  for ( i = 0; i < borderSize; i++ )
    tabu_list[i] = maxit;


  for ( i = 0; i < probSize; i++ )
  {
    tabu_list[i+borderSize] = -1;

    candidates[i] = new Triple();
    candidates[i]->feat_id = i + borderSize;
    candidatesUnsorted[i] = candidates[i];

    candidates[i]->cost = ( sol[i+borderSize] == -1 ? inactiveCost[i+borderSize] : mLabelPositions.at( sol[i+borderSize] )->cost() );
  }

  Util::sort( reinterpret_cast< void** >( candidates ), probSize, decreaseCost );

  int candidateListSize;
  candidateListSize = int ( pal->candListSize * static_cast< double >( probSize ) + 0.5 );

  if ( candidateListSize > probSize )
    candidateListSize = probSize;

  while ( it < stop_it )
  {
    retainedChain = nullptr;

    for ( itC = 0; itC < candidateListSize; itC++ )
    {
      seed = candidates[itC]->feat_id;

      current_chain = chain( part, seed );

      if ( current_chain )
      {
        // seed is not taboo or chain give us a new best solution
        if ( tabu_list[seed] < it || ( cur_cost + current_chain->delta ) - best_cost < 0.0 )
        {
          if ( !retainedChain )
          {
            retainedChain = current_chain;
          }
          else if ( current_chain->delta - retainedChain->delta < EPSILON )
          {
            delete_chain( retainedChain );
            retainedChain = current_chain;
          }
          else
          {
            delete_chain( current_chain );
          }
        }
        else
        {
          delete_chain( current_chain );
        }
      }
    } // end foreach candidates

    if ( retainedChain /*&& retainedChain->delta <= 0*/ )
    {
      for ( i = 0; i < retainedChain->degree; i++ )
      {
        fid = retainedChain->feat[i];
        lid = retainedChain->label[i];

        if ( sol[fid] >= 0 )
          mLabelPositions.at( sol[fid] )->removeFromIndex( candidates_subsol );

        sol[fid] = lid;

        if ( lid >= 0 )
          mLabelPositions.at( lid )->insertIntoIndex( candidates_subsol );

        tabu_list[fid] = it + tenure;
        candidatesUnsorted[fid-borderSize]->cost = ( lid == -1 ? inactiveCost[sub[fid]] : mLabelPositions.at( lid )->cost() );

      }

      cur_cost += retainedChain->delta;

      delete_chain( retainedChain );

      if ( best_cost - cur_cost > EPSILON )
      {
        best_cost = cur_cost;
        memcpy( best_sol, sol, sizeof( int ) * subSize );

        stop_it = ( it + itwimp > maxit ? maxit : it + itwimp );
      }
      Util::sort( reinterpret_cast< void** >( candidates ), probSize, decreaseCost );
    }
    it++;
  }

  memcpy( sol, best_sol, sizeof( int ) *subSize );

  for ( i = 0; i < probSize; i++ )
    delete candidates[i];

  delete[] candidates;
  delete[] candidatesUnsorted;

  for ( i = 0; i < subSize; i++ )
    featWrap[sub[i]] = -1;

  delete[] best_sol;
  delete[] tabu_list;


  return initial_cost - best_cost;
}

bool checkCallback( LabelPosition *lp, void *ctx )
{
  QLinkedList<LabelPosition*> *list = reinterpret_cast< QLinkedList<LabelPosition*>* >( ctx );
  list->append( lp );

  return true;
}


void Problem::check_solution()
{
  int *solution = new int[nbft];

  double amin[2];
  double amax[2];

  amin[0] = bbox[0];
  amin[1] = bbox[1];
  amax[0] = bbox[2];
  amax[1] = bbox[3];

  QLinkedList<LabelPosition*> *list = new QLinkedList<LabelPosition*>;

  candidates_sol->Search( amin, amax, checkCallback, reinterpret_cast< void* >( list ) );

  int i;
  int nbActive = 0;
  for ( i = 0; i < nbft; i++ )
  {
    solution[i] = -1;
    if ( sol->s[i] >= 0 )
      nbActive++;
  }

  if ( list->size() != nbActive )
  {
    // Error in solution !!!!
  }

  while ( !list->isEmpty() )
  {
    LabelPosition *lp = list->takeFirst();
    int probFeatId = lp->getProblemFeatureId();

    solution[probFeatId] = lp->getId();
  }

  delete [] solution;
}

typedef struct _nokContext
{
  LabelPosition *lp;
  bool *ok;
  int *wrap;
} NokContext;

bool nokCallback( LabelPosition *lp, void *context )
{
  NokContext* ctx = reinterpret_cast< NokContext*>( context );
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

  if ( nbft == 0 )
    return;

  int i;
  int seed;
  bool *ok = new bool[nbft];
  int fid;
  int lid;
  int popit = 0;
  double amin[2];
  double amax[2];

  NokContext context;
  context.ok = ok;
  context.wrap = nullptr;

  Chain *retainedChain;

  featWrap = nullptr;

  for ( i = 0; i < nbft; i++ )
  {
    ok[i] = false;
  }

  //initialization();
  init_sol_falp();

  //check_solution();
  solution_cost();

  int iter = 0;

  while ( true )
  {

    //check_solution();

    for ( seed = ( iter + 1 ) % nbft;
          ok[seed] && seed != iter;
          seed = ( seed + 1 ) % nbft )
      ;

    // All seeds are OK
    if ( seed == iter )
    {
      break;
    }

    iter = ( iter + 1 ) % nbft;
    retainedChain = chain( seed );

    if ( retainedChain && retainedChain->delta < - EPSILON )
    {
      // apply modification
      for ( i = 0; i < retainedChain->degree; i++ )
      {
        fid = retainedChain->feat[i];
        lid = retainedChain->label[i];

        if ( sol->s[fid] >= 0 )
        {
          LabelPosition *old = mLabelPositions.at( sol->s[fid] );
          old->removeFromIndex( candidates_sol );

          old->getBoundingBox( amin, amax );

          context.lp = old;
          candidates->Search( amin, amax, nokCallback, &context );
        }

        sol->s[fid] = lid;

        if ( sol->s[fid] >= 0 )
        {
          mLabelPositions.at( lid )->insertIntoIndex( candidates_sol );
        }

        ok[fid] = false;
      }
      sol->cost += retainedChain->delta;
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

  return;
}

bool Problem::compareLabelArea( pal::LabelPosition* l1, pal::LabelPosition* l2 )
{
  return l1->getWidth() * l1->getHeight() > l2->getWidth() * l2->getHeight();
}

QList<LabelPosition*> * Problem::getSolution( bool returnInactive )
{

  int i;
  QList<LabelPosition*> *solList = new QList<LabelPosition*>();

  if ( nbft == 0 )
  {
    return solList;
  }

  for ( i = 0; i < nbft; i++ )
  {
    if ( sol->s[i] != -1 )
    {
      solList->push_back( mLabelPositions.at( sol->s[i] ) ); // active labels
    }
    else if ( returnInactive
              || mLabelPositions.at( featStartId[i] )->getFeaturePart()->layer()->displayAll()
              || mLabelPositions.at( featStartId[i] )->getFeaturePart()->getAlwaysShow() )
    {
      solList->push_back( mLabelPositions.at( featStartId[i] ) ); // unplaced label
    }
  }

  // if features collide, order by size, so smaller ones appear on top
  if ( returnInactive )
  {
    qSort( solList->begin(), solList->end(), compareLabelArea );
  }

  return solList;
}

PalStat * Problem::getStats()
{
  int i, j;

  PalStat *stats = new PalStat();

  stats->nbObjects = nbft;
  stats->nbLabelledObjects = 0;

  stats->nbLayers = nbLabelledLayers;
  stats->layersNbObjects = new int[stats->nbLayers];
  stats->layersNbLabelledObjects = new int[stats->nbLayers];

  for ( i = 0; i < stats->nbLayers; i++ )
  {
    stats->layersName << labelledLayersName[i];
    stats->layersNbObjects[i] = 0;
    stats->layersNbLabelledObjects[i] = 0;
  }

  QString lyrName;
  int k;
  for ( i = 0; i < nbft; i++ )
  {
    lyrName = mLabelPositions.at( featStartId[i] )->getFeaturePart()->feature()->provider()->name();
    k = -1;
    for ( j = 0; j < stats->nbLayers; j++ )
    {
      if ( lyrName == stats->layersName[j] )
      {
        k = j;
        break;
      }
    }
    if ( k != -1 )
    {
      stats->layersNbObjects[k]++;
      if ( sol->s[i] >= 0 )
      {
        stats->layersNbLabelledObjects[k]++;
        stats->nbLabelledObjects++;
      }
    }
    else
    {
      // unknown layer
    }
  }

  return stats;
}

void Problem::solution_cost()
{
  sol->cost = 0.0;
  nbActive = 0;

  int nbOv;

  int i;

  LabelPosition::CountContext context;
  context.inactiveCost = inactiveCost;
  context.nbOv = &nbOv;
  context.cost = &sol->cost;
  double amin[2];
  double amax[2];
  LabelPosition *lp;

  int nbHidden = 0;

  for ( i = 0; i < nbft; i++ )
  {
    if ( sol->s[i] == -1 )
    {
      sol->cost += inactiveCost[i];
      nbHidden++;
    }
    else
    {
      nbOv = 0;
      lp = mLabelPositions.at( sol->s[i] );

      lp->getBoundingBox( amin, amax );

      context.lp = lp;
      candidates_sol->Search( amin, amax, LabelPosition::countFullOverlapCallback, &context );

      sol->cost += lp->cost();

      if ( nbOv == 0 )
        nbActive++;
    }
  }
}
