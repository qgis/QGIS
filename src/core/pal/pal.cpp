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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//#define _VERBOSE_
//#define _EXPORT_MAP_
#include <QTime>

#define _CRT_SECURE_NO_DEPRECATE

#include <cstdarg>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cfloat>
#include <list>
//#include <geos/geom/Geometry.h>
#include <geos_c.h>

#include <pal/pal.h>
#include <pal/layer.h>
#include <pal/palexception.h>
#include <pal/palstat.h>

#include "linkedlist.hpp"
#include "rtree.hpp"

#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "pointset.h"
#include "simplemutex.h"
#include "util.h"

namespace pal
{

  void geosError( const char *fmt, ... )
  {
    va_list list;
    va_start( list, fmt );
    vfprintf( stderr, fmt, list );
  }

  void geosNotice( const char *fmt, ... )
  {
    va_list list;
    va_start( list, fmt );
    vfprintf( stdout, fmt, list );
  }

  Pal::Pal()
  {
    // do not init and exit GEOS - we do it inside QGIS
    //initGEOS( geosNotice, geosError );

    layers = new std::list<Layer*>();

    lyrsMutex = new SimpleMutex();

    ejChainDeg = 50;
    tenure = 10;
    candListSize = 0.2;

    tabuMinIt = 3;
    tabuMaxIt = 4;
    searchMethod = POPMUSIC_CHAIN;
    popmusic_r = 30;

    searchMethod = CHAIN;

    setSearch( CHAIN );

    dpi = 72;
    point_p = 8;
    line_p = 8;
    poly_p = 8;

    this->map_unit = pal::METER;

    std::cout.precision( 12 );
    std::cerr.precision( 12 );

  }

  std::list<Layer*> *Pal::getLayers()
  {
    // TODO make const ! or whatever else
    return layers;
  }

  Layer *Pal::getLayer( const char *lyrName )
  {
    lyrsMutex->lock();
    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end(); it++ )
      if ( strcmp(( *it )->name, lyrName ) == 0 )
      {
        lyrsMutex->unlock();
        return *it;
      }

    lyrsMutex->unlock();
    throw new PalException::UnknownLayer();
  }


  void Pal::removeLayer( Layer *layer )
  {
    lyrsMutex->lock();
    if ( layer )
    {
      layers->remove( layer );
      delete layer;
    }
    lyrsMutex->unlock();
  }


  Pal::~Pal()
  {

    lyrsMutex->lock();
    while ( layers->size() > 0 )
    {
      delete layers->front();
      layers->pop_front();
    }

    delete layers;
    delete lyrsMutex;

    // do not init and exit GEOS - we do it inside QGIS
    //finishGEOS();
  }


  Layer * Pal::addLayer( const char *lyrName, double min_scale, double max_scale, Arrangement arrangement, Units label_unit, double defaultPriority, bool obstacle, bool active, bool toLabel )
  {
    Layer *lyr;
    lyrsMutex->lock();

#ifdef _DEBUG_
    std::cout << "Pal::addLayer" << std::endl;
    std::cout << "lyrName:" << lyrName << std::endl;
    std::cout << "nbLayers:" << layers->size() << std::endl;
#endif

    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end(); it++ )
    {
      if ( strcmp(( *it )->name, lyrName ) == 0 )   // if layer already known
      {
        lyrsMutex->unlock();
        //There is already a layer with this name, so we just return the existing one.
        //Sometimes the same layer is added twice (e.g. datetime split with otf-reprojection)
        return *it;
      }
    }

    lyr = new Layer( lyrName, min_scale, max_scale, arrangement, label_unit, defaultPriority, obstacle, active, toLabel, this );
    layers->push_back( lyr );

    lyrsMutex->unlock();

    return lyr;
  }


  typedef struct _featCbackCtx
  {
    Layer *layer;
    double scale;
    LinkedList<Feats*> *fFeats;
    RTree<PointSet*, double, 2, double> *obstacles;
    RTree<LabelPosition*, double, 2, double> *candidates;
    double priority;
    double bbox_min[2];
    double bbox_max[2];
#ifdef _EXPORT_MAP_
    std::ofstream *svgmap;
#endif
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

#ifdef _EXPORT_MAP_
    bool svged = false; // is the feature has been written into the svg map ?
    int dpi = context->layer->pal->getDpi();
#endif


#ifdef _DEBUG_FULL_
    std::cout << "extract feat : " << ft_ptr->layer->name << "/" << ft_ptr->uid << std::endl;
#endif

    // all feature which are obstacle will be inserted into obstacles
    if ( context->layer->obstacle )
    {
      ft_ptr->getBoundingBox( amin, amax );
      context->obstacles->Insert( amin, amax, ft_ptr );
    }

    // first do some checks whether to extract candidates or not

    // feature has to be labeled ?
    if ( !context->layer->toLabel )
      return true;

    // are we in a valid scale range for the layer?
    if ( !context->layer->isScaleValid( context->scale ) )
      return true;

    // is the feature well defined ? // TODO Check epsilon
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
    LabelPosition** lPos = NULL;
    int nblp = ft_ptr->setPosition( context->scale, &lPos, context->bbox_min, context->bbox_max, ft_ptr, context->candidates
#ifdef _EXPORT_MAP_
                                    , *context->svgmap
#endif
                                  );

    if ( nblp > 0 )
    {
      // valid features are added to fFeats
      Feats *ft = new Feats();
      ft->feature = ft_ptr;
      ft->shape = NULL;
      ft->nblp = nblp;
      ft->lPos = lPos;
      ft->priority = context->priority;
      context->fFeats->push_back( ft );
    }
    else
    {
      // Others are deleted
      delete[] lPos;
    }

    return true;
  }




  typedef struct _filterContext
  {
    RTree<LabelPosition*, double, 2, double> *cdtsIndex;
    double scale;
    Pal* pal;
  } FilterContext;

  bool filteringCallback( PointSet *pset, void *ctx )
  {

    RTree<LabelPosition*, double, 2, double> *cdtsIndex = (( FilterContext* ) ctx )->cdtsIndex;
    double scale = (( FilterContext* ) ctx )->scale;
    Pal* pal = (( FilterContext* )ctx )->pal;

    double amin[2], amax[2];
    pset->getBoundingBox( amin, amax );

    LabelPosition::PruneCtx pruneContext;

    pruneContext.scale = scale;
    pruneContext.obstacle = pset;
    pruneContext.pal = pal;
    cdtsIndex->Search( amin, amax, LabelPosition::pruneCallback, ( void* ) &pruneContext );

    return true;
  }


  /**
  * \Brief Problem Factory
  * Select features from user's choice layers within
  * a specific bounding box
  * param nbLayers # wanted layers
  * param layersFactor layers importance
  * param layersName layers in problem
  * param lambda_min west bbox
  * param phi_min south bbox
  * param lambda_max east bbox
  * param phi_max north bbox
  * param scale the scale
  */
  Problem* Pal::extract( int nbLayers, char **layersName, double *layersFactor, double lambda_min, double phi_min, double lambda_max, double phi_max, double scale, std::ofstream *svgmap )
  {
    // to store obstacles
    RTree<PointSet*, double, 2, double> *obstacles = new RTree<PointSet*, double, 2, double>();

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


    prob->scale = scale;
    prob->pal = this;

    LinkedList<Feats*> *fFeats = new LinkedList<Feats*> ( ptrFeatsCompare );

    FeatCallBackCtx *context = new FeatCallBackCtx();
    context->fFeats = fFeats;
    context->scale = scale;
    context->obstacles = obstacles;
    context->candidates = prob->candidates;

    context->bbox_min[0] = amin[0];
    context->bbox_min[1] = amin[1];

    context->bbox_max[0] = amax[0];
    context->bbox_max[1] = amax[1];

#ifdef _EXPORT_MAP_
    context->svgmap = svgmap;
#endif

#ifdef _VERBOSE_
    std::cout <<  nbLayers << "/" << layers->size() << " layers to extract " << std::endl;
    std::cout << "scale is 1:" << scale << std::endl << std::endl;

#endif


    /* First step : extract feature from layers
     *
     * */
    int oldNbft = 0;
    Layer *layer;

    std::list<char*> *labLayers = new std::list<char*>();

    lyrsMutex->lock();
    for ( i = 0; i < nbLayers; i++ )
    {
      for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end(); it++ ) // iterate on pal->layers
      {
        layer = *it;
        // Only select those who are active and labellable (with scale constraint) or those who are active and which must be treated as obstaclewhich must be treated as obstacle
        if ( layer->active
             && ( layer->obstacle || ( layer->toLabel && layer->isScaleValid( scale ) ) ) )
        {

          // check if this selected layers has been selected by user
          if ( strcmp( layersName[i], layer->name ) == 0 )
          {
            // check for connected features with the same label text and join them
            if ( layer->getMergeConnectedLines() )
              layer->joinConnectedFeatures();

            context->layer = layer;
            context->priority = layersFactor[i];
            // lookup for feature (and generates candidates list)

#ifdef _EXPORT_MAP_
            *svgmap << "<g inkscape:label=\"" << layer->name << "\"" << std::endl
            <<  "    inkscape:groupmode=\"layer\"" << std::endl
            <<  "    id=\"" << layer->name << "\">" << std::endl << std::endl;
#endif

            context->layer->modMutex->lock();
            context->layer->rtree->Search( amin, amax, extractFeatCallback, ( void* ) context );
            context->layer->modMutex->unlock();

#ifdef _EXPORT_MAP_
            *svgmap  << "</g>" << std::endl << std::endl;
#endif

#ifdef _VERBOSE_
            std::cout << "Layer's name: " << layer->getName() << std::endl;
            std::cout << "     scale range: " << layer->getMinScale() << "->" << layer->getMaxScale() << std::endl;
            std::cout << "     active:" << layer->isToLabel() << std::endl;
            std::cout << "     obstacle:" << layer->isObstacle() << std::endl;
            std::cout << "     toLabel:" << layer->isToLabel() << std::endl;
            std::cout << "     # features: " << layer->getNbFeatures() << std::endl;
            std::cout << "     # extracted features: " << context->fFeats->size() - oldNbft << std::endl;
#endif
            if ( context->fFeats->size() - oldNbft > 0 )
            {
              char *name = new char[strlen( layer->getName() ) +1];
              strcpy( name, layer->getName() );
              labLayers->push_back( name );
            }
            oldNbft = context->fFeats->size();


            break;
          }
        }
      }
    }
    delete context;
    lyrsMutex->unlock();

    prob->nbLabelledLayers = labLayers->size();
    prob->labelledLayersName = new char*[prob->nbLabelledLayers];
    for ( i = 0; i < prob->nbLabelledLayers; i++ )
    {
      prob->labelledLayersName[i] = labLayers->front();
      labLayers->pop_front();
    }

    delete labLayers;

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
    filterCtx.scale = prob->scale;
    filterCtx.pal = this;
    obstacles->Search( amin, amax, filteringCallback, ( void* ) &filterCtx );


    int idlp = 0;
    for ( i = 0; i < prob->nbft; i++ ) /* foreach feature into prob */
    {
      feat = fFeats->pop_front();
#ifdef _DEBUG_FULL_
      std::cout << "Feature:" << feat->feature->layer->name << "/" << feat->feature->uid << std::endl;
#endif
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

#ifdef _DEBUG_FULL_
      std::cout << "All Cost are setted" << std::endl;
#endif
      // only keep the 'max_p' best candidates
      for ( j = max_p; j < feat->nblp; j++ )
      {
        // TODO remove from index
        feat->lPos[j]->removeFromIndex( prob->candidates );
        delete feat->lPos[j];
      }
      feat->nblp = max_p;

      // update problem's # candidate
      prob->featNbLp[i] = feat->nblp;
      prob->nblp += feat->nblp;

      // add all candidates into a rtree (to speed up conflicts searching)
      for ( j = 0; j < feat->nblp; j++, idlp++ )
      {
        lp = feat->lPos[j];
        //lp->insertIntoIndex(prob->candidates);
        lp->setProblemIds( i, idlp ); // bugfix #1 (maxence 10/23/2008)
      }
      fFeats->push_back( feat );
    }

#ifdef _DEBUG_FULL_
    std::cout << "Malloc problem...." << std::endl;
#endif


    idlp = 0;
    int nbOverlaps = 0;
    prob->labelpositions = new LabelPosition*[prob->nblp];
    //prob->feat = new int[prob->nblp];

#ifdef _DEBUG_FULL_
    std::cout << "problem malloc'd" << std::endl;
#endif


    j = 0;
    while ( fFeats->size() > 0 ) // foreach feature
    {
      feat = fFeats->pop_front();
      for ( i = 0; i < feat->nblp; i++, idlp++ )  // foreach label candidate
      {
        lp = feat->lPos[i];
        lp->resetNumOverlaps();

        // make sure that candidate's cost is less than 1
        lp->validateCost();

        prob->labelpositions[idlp] = lp;
        //prob->feat[idlp] = j;

        lp->getBoundingBox( amin, amax );

        // lookup for overlapping candidate
        prob->candidates->Search( amin, amax, LabelPosition::countOverlapCallback, ( void* ) lp );

        nbOverlaps += lp->getNumOverlaps();
#ifdef _DEBUG_FULL_
        std::cout << "Nb overlap for " << idlp << "/" << prob->nblp - 1 << " : " << lp->nbOverlap << std::endl;
#endif
      }
      j++;
      delete[] feat->lPos;
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
    std::cerr << scale << "\t"
              << prob->nbft << "\t"
              << prob->nblp << "\t"
              << prob->nbOverlap << "\t";
#endif

    return prob;
  }

  std::list<LabelPosition*>* Pal::labeller( double scale, double bbox[4], PalStat **stats, bool displayAll )
  {

#ifdef _DEBUG_FULL_
    std::cout << "LABELLER (active)" << std::endl;
#endif
    int i;

    lyrsMutex->lock();
    int nbLayers = layers->size();

    char **layersName = new char*[nbLayers];
    double *priorities = new double[nbLayers];
    Layer *layer;
    i = 0;
    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end(); it++ )
    {
      layer = *it;
      layersName[i] = layer->name;
      priorities[i] = layer->defaultPriority;
      i++;
    }
    lyrsMutex->unlock();

    std::list<LabelPosition*> * solution = labeller( nbLayers, layersName, priorities, scale, bbox, stats, displayAll );

    delete[] layersName;
    delete[] priorities;
    return solution;
  }


  /*
   * BIG MACHINE
   */
  std::list<LabelPosition*>* Pal::labeller( int nbLayers, char **layersName , double *layersFactor, double scale, double bbox[4], PalStat **stats, bool displayAll )
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

#ifdef _EXPORT_MAP_
    // TODO this is not secure
    std::ofstream svgmap( "pal-map.svg" );

    svgmap << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << std::endl
    << "<svg" << std::endl
    << "xmlns:dc=\"http://purl.org/dc/elements/1.1/\"" << std::endl
    << "xmlns:cc=\"http://creativecommons.org/ns#\"" << std::endl
    << "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"" << std::endl
    << "xmlns:svg=\"http://www.w3.org/2000/svg\"" << std::endl
    << "xmlns=\"http://www.w3.org/2000/svg\"" << std::endl
    << "xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"" << std::endl
    << "xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"" << std::endl
    << "width=\"" << convert2pt( bbox[2] - bbox[0], scale, dpi )  << "\"" << std::endl
    << "height=\"" << convert2pt( bbox[3] - bbox[1], scale, dpi )  << "\">" << std::endl; // TODO xmax ymax
#endif

    QTime t;
    t.start();

    // First, extract the problem
    // TODO which is the minimum scale ? (> 0, >= 0, >= 1, >1 )
    if ( scale < 1 || ( prob = extract( nbLayers, layersName, layersFactor, bbox[0], bbox[1], bbox[2], bbox[3], scale,
#ifdef _EXPORT_MAP_
                                        & svgmap
#else
                                        NULL
#endif
                                      ) ) == NULL )
    {

#ifdef _VERBOSE_
      if ( scale < 1 )
        std::cout << "Scale is 1:" << scale << std::endl;
      else
        std::cout << "empty problem... finishing" << std::endl;
#endif

#ifdef _EXPORT_MAP_
      svgmap << "</svg>" << std::endl;
      svgmap.close();
#endif

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

#ifdef _EXPORT_MAP_
    prob->drawLabels( svgmap );
    svgmap << "</svg>" << std::endl;
    svgmap.close();
#endif

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

  Problem* Pal::extractProblem( double scale, double bbox[4] )
  {
    // find out: nbLayers, layersName, layersFactor
    lyrsMutex->lock();
    int nbLayers = layers->size();

    char **layersName = new char*[nbLayers];
    double *priorities = new double[nbLayers];
    Layer *layer;
    int i = 0;
    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end(); it++ )
    {
      layer = *it;
      layersName[i] = layer->name;
      priorities[i] = layer->defaultPriority;
      i++;
    }
    lyrsMutex->unlock();

    Problem* prob = extract( nbLayers, layersName, priorities, bbox[0], bbox[1], bbox[2], bbox[3], scale, NULL );

    delete[] layersName;
    delete[] priorities;

    return prob;
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


  void Pal::setDpi( int dpi )
  {
    if ( dpi > 0 )
      this->dpi = dpi;
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

  int Pal::getDpi()
  {
    return dpi;
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


  /**
   * \brief get current map unit
   */
  Units Pal::getMapUnit()
  {
    return map_unit;
  }

  /**
   * \brief set map unit
   */
  void Pal::setMapUnit( Units map_unit )
  {
    if ( map_unit == pal::PIXEL || map_unit == pal::METER
         || map_unit == pal::FOOT || map_unit == pal::DEGREE )
    {
      this->map_unit = map_unit;
    }
  }



} // namespace pal

