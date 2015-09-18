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

//#define _VERBOSE_

#define _CRT_SECURE_NO_DEPRECATE

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
#include "util.h"
#include <QTime>
#include <cstdarg>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <list>

namespace pal
{
  GEOSContextHandle_t geosContext()
  {
    return QgsGeometry::getGEOSHandler();
  }

  Pal::Pal()
  {
    // do not init and exit GEOS - we do it inside QGIS
    //initGEOS( geosNotice, geosError );

    fnIsCancelled = 0;
    fnIsCancelledContext = 0;

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

    std::cout.precision( 12 );
    std::cerr.precision( 12 );

  }

  QList<Layer*> Pal::getLayers()
  {
    // TODO make const ! or whatever else
    return mLayers.values();
  }

  Layer *Pal::getLayer( const QString& layerName )
  {
    mMutex.lock();
    if ( !mLayers.contains( layerName ) )
    {
      mMutex.unlock();
      throw new PalException::UnknownLayer();
    }

    Layer* result = mLayers.value( layerName );
    mMutex.unlock();
    return result;
  }

  void Pal::removeLayer( Layer *layer )
  {
    if ( !layer )
      return;

    mMutex.lock();
    QString key = mLayers.key( layer, QString() );
    if ( !key.isEmpty() )
    {
      mLayers.remove( key );
    }
    delete layer;
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

  Layer* Pal::addLayer( const QString &layerName, Arrangement arrangement, double defaultPriority, bool obstacle, bool active, bool toLabel, bool displayAll )
  {
    mMutex.lock();

    //check if layer is already known
    if ( mLayers.contains( layerName ) )
    {
      mMutex.unlock();
      //There is already a layer with this name, so we just return the existing one.
      //Sometimes the same layer is added twice (e.g. datetime split with otf-reprojection)
      return mLayers.value( layerName );
    }

    Layer* layer = new Layer( layerName, arrangement, defaultPriority, obstacle, active, toLabel, this, displayAll );
    mLayers.insert( layerName, layer );
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

    FeatCallBackCtx *context = ( FeatCallBackCtx* ) ctx;

#ifdef _DEBUG_FULL_
    std::cout << "extract feat : " << ft_ptr->getLayer()->getName() << "/" << ft_ptr->getUID() << std::endl;
#endif

    // all feature which are obstacle will be inserted into obstacles
    if ( ft_ptr->getFeature()->isObstacle() )
    {
      ft_ptr->getBoundingBox( amin, amax );
      context->obstacles->Insert( amin, amax, ft_ptr );
    }

    // first do some checks whether to extract candidates or not

    // feature has to be labeled?
    if ( !context->layer->labelLayer() )
      return true;

    // is the feature well defined?  TODO Check epsilon
    if ( ft_ptr->getLabelWidth() < 0.0000001 || ft_ptr->getLabelHeight() < 0.0000001 )
      return true;

    // OK, everything's fine, let's process the feature part

    // Holes of the feature are obstacles
    for ( int i = 0; i < ft_ptr->getNumSelfObstacles(); i++ )
    {
      ft_ptr->getSelfObstacle( i )->getBoundingBox( amin, amax );
      context->obstacles->Insert( amin, amax, ft_ptr->getSelfObstacle( i ) );

      if ( !ft_ptr->getSelfObstacle( i )->getHoleOf() )
      {
        std::cout << "ERROR: SHOULD HAVE A PARENT!!!!!" << std::endl;
      }
    }

    // generate candidates for the feature part
    QList< LabelPosition* > lPos;
    if ( ft_ptr->setPosition( lPos, context->bbox_min, context->bbox_max, ft_ptr, context->candidates ) )
    {
      // valid features are added to fFeats
      Feats *ft = new Feats();
      ft->feature = ft_ptr;
      ft->shape = NULL;
      ft->lPos = lPos;
      ft->priority = ft_ptr->getFeature()->calculatePriority();
      context->fFeats->append( ft );
    }
    else
    {
      // Others are deleted
      qDeleteAll( lPos );
    }

    return true;
  }




  typedef struct _filterContext
  {
    RTree<LabelPosition*, double, 2, double> *cdtsIndex;
    Pal* pal;
  } FilterContext;

  bool filteringCallback( FeaturePart *featurePart, void *ctx )
  {

    RTree<LabelPosition*, double, 2, double> *cdtsIndex = (( FilterContext* ) ctx )->cdtsIndex;
    Pal* pal = (( FilterContext* )ctx )->pal;

    if ( pal->isCancelled() )
      return false; // do not continue searching

    double amin[2], amax[2];
    featurePart->getBoundingBox( amin, amax );

    LabelPosition::PruneCtx pruneContext;
    pruneContext.obstacle = featurePart;
    pruneContext.pal = pal;
    cdtsIndex->Search( amin, amax, LabelPosition::pruneCallback, ( void* ) &pruneContext );

    return true;
  }

  Problem* Pal::extract( const QStringList& layerNames, double lambda_min, double phi_min, double lambda_max, double phi_max )
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

    FeatCallBackCtx *context = new FeatCallBackCtx();
    context->fFeats = fFeats;
    context->obstacles = obstacles;
    context->candidates = prob->candidates;

    context->bbox_min[0] = amin[0];
    context->bbox_min[1] = amin[1];

    context->bbox_max[0] = amax[0];
    context->bbox_max[1] = amax[1];

    // first step : extract features from layers

    int previousFeatureCount = 0;
    Layer *layer;

    QStringList layersWithFeaturesInBBox;

    mMutex.lock();
    Q_FOREACH ( const QString& layerName, layerNames )
    {
      layer = mLayers.value( layerName, 0 );
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

      // find features within bounding box and generate candidates list
      context->layer = layer;
      context->layer->mMutex.lock();
      context->layer->rtree->Search( amin, amax, extractFeatCallback, ( void* ) context );
      context->layer->mMutex.unlock();

      if ( context->fFeats->size() - previousFeatureCount > 0 )
      {
        layersWithFeaturesInBBox << layer->name();
      }
      previousFeatureCount = context->fFeats->size();
    }
    delete context;
    mMutex.unlock();

    prob->nbLabelledLayers = layersWithFeaturesInBBox.size();
    prob->labelledLayersName = layersWithFeaturesInBBox;

    if ( fFeats->size() == 0 )
    {
#ifdef _VERBOSE_
      std::cout << std::endl << "Empty problem" << std::endl;
#endif
      delete fFeats;
      delete prob;
      delete obstacles;
      return NULL;
    }

    prob->nbft = fFeats->size();
    prob->nblp = 0;
    prob->featNbLp = new int [prob->nbft];
    prob->featStartId = new int [prob->nbft];
    prob->inactiveCost = new double[prob->nbft];

    Feats *feat;

#ifdef _VERBOSE_
    std::cout << "FIRST NBFT : " << prob->nbft << std::endl;
#endif

    // Filtering label positions against obstacles
    amin[0] = amin[1] = -DBL_MAX;
    amax[0] = amax[1] = DBL_MAX;
    FilterContext filterCtx;
    filterCtx.cdtsIndex = prob->candidates;
    filterCtx.pal = this;
    obstacles->Search( amin, amax, filteringCallback, ( void* ) &filterCtx );

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
      return 0;
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
        lp = feat->lPos[j];
        //lp->insertIntoIndex(prob->candidates);
        lp->setProblemIds( i, idlp ); // bugfix #1 (maxence 10/23/2008)
      }
      fFeats->append( feat );
    }

    int nbOverlaps = 0;

    while ( fFeats->size() > 0 ) // foreach feature
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
        return 0;
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
        prob->candidates->Search( amin, amax, LabelPosition::countOverlapCallback, ( void* ) lp );

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


#ifdef _VERBOSE_
    std::cout << "nbOverlap: " << prob->nbOverlap << std::endl;
    std::cerr << prob->nbft << "\t"
              << prob->nblp << "\t"
              << prob->nbOverlap << "\t";
#endif

    return prob;
  }

  std::list<LabelPosition*>* Pal::labeller( double bbox[4], PalStat **stats, bool displayAll )
  {
    return labeller( mLayers.keys(), bbox, stats, displayAll );
  }

  /*
   * BIG MACHINE
   */
  std::list<LabelPosition*>* Pal::labeller( const QStringList& layerNames, double bbox[4], PalStat **stats, bool displayAll )
  {
#ifdef _DEBUG_
    std::cout << "LABELLER (selection)" << std::endl;
#endif

    Problem *prob;

    SearchMethod old_searchMethod = searchMethod;

    if ( displayAll )
    {
      setSearch( POPMUSIC_TABU );
    }

#ifdef _VERBOSE_
    clock_t start = clock();
    double create_time;
    std::cout << std::endl << "bbox: " << bbox[0] << " " << bbox[1] << " " << bbox[2] << " " << bbox[3] << std::endl;
#endif

    QTime t;
    t.start();

    // First, extract the problem
    if (( prob = extract( layerNames, bbox[0], bbox[1], bbox[2], bbox[3] ) ) == NULL )
    {
      // nothing to be done => return an empty result set
      if ( stats )
        ( *stats ) = new PalStat();
      return new std::list<LabelPosition*>();
    }

    std::cout << "PAL EXTRACT: " << t.elapsed() / 1000.0 << " s" << std::endl;
    t.restart();

    // reduce number of candidates
    // (remove candidates which surely won't be used)
    prob->reduce();

#ifdef _VERBOSE_
    std::cerr << prob->nblp << "\t"
              << prob->nbOverlap;
#endif


    prob->displayAll = displayAll;

#ifdef _VERBOSE_
    create_time = double( clock() - start ) / double( CLOCKS_PER_SEC );

    std::cout << std::endl << "Problem : " << prob->nblp << " candidates for " << prob->nbft << " features makes " << prob->nbOverlap << " overlaps" << std::endl;
    std::cout << std::endl << "Times:"  << std::endl << "    to create problem:  " << create_time << std::endl;
#endif

    // search a solution
    if ( searchMethod == FALP )
      prob->init_sol_falp();
    else if ( searchMethod == CHAIN )
      prob->chain_search();
    else
      prob->popmusic();

    std::cout << "PAL SEARCH (" << searchMethod << "): " << t.elapsed() / 1000.0 << " s" << std::endl;
    t.restart();

    // Post-Optimization
    //prob->post_optimization();


    std::list<LabelPosition*> * solution = prob->getSolution( displayAll );

    if ( stats )
      *stats = prob->getStats();

#ifdef _VERBOSE_
    clock_t total_time =  clock() - start;
    std::cout << "    Total time: " << double( total_time ) / double( CLOCKS_PER_SEC ) << std::endl;
    std::cerr << "\t" << create_time << "\t" << double( total_time ) / double( CLOCKS_PER_SEC ) << std::endl;
#endif

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
    return extract( mLayers.keys(), bbox[0], bbox[1], bbox[2], bbox[3] );
  }

  std::list<LabelPosition*>* Pal::solveProblem( Problem* prob, bool displayAll )
  {
    if ( prob == NULL )
      return new std::list<LabelPosition*>();

    prob->reduce();

    if ( searchMethod == FALP )
      prob->init_sol_falp();
    else if ( searchMethod == CHAIN )
      prob->chain_search();
    else
      prob->popmusic();

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
      default:
        std::cerr << "Unknown search method..." << std::endl;
    }
  }

} // namespace pal

