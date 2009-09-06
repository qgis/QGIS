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

#define _CRT_SECURE_NO_DEPRECATE

#include <cstdarg>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cfloat>
#include <ctime>
#include <list>
//#include <geos/geom/Geometry.h>
#include <geos_c.h>

#include <pal/pal.h>
#include <pal/layer.h>
#include <pal/palexception.h>

#include "linkedlist.hpp"
#include "rtree.hpp"

#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "problem.h"
#include "pointset.h"
#include "simplemutex.h"
#include "util.h"

namespace pal
{

  typedef struct
  {
    //LabelPosition *lp;
    double scale;
    Pal* pal;
    PointSet *obstacle;
  } PruneCtx;

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
    initGEOS( geosNotice, geosError );

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

    tmpTime = 0;
  }

  std::list<Layer*> *Pal::getLayers()
  {
    // TODO make const ! or whatever else
    return layers;
  }

  Layer *Pal::getLayer( const char *lyrName )
  {
    lyrsMutex->lock();
    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end();it++ )
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

    std::cout << "Acces/Concvert time: " << ( double ) tmpTime / CLOCKS_PER_SEC << std::endl;

    lyrsMutex->lock();
    while ( layers->size() > 0 )
    {
      delete layers->front();
      layers->pop_front();
    }

    delete layers;
    delete lyrsMutex;

    finishGEOS();
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

    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end();it++ )
    {
      if ( strcmp(( *it )->name, lyrName ) == 0 )   // if layer already known
      {
        lyrsMutex->unlock();
        throw new PalException::LayerExists();
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
  bool extractFeatCallback( Feature *ft_ptr, void *ctx )
  {

    double min[2];
    double max[2];

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
      min[0] = ft_ptr->xmin;
      min[1] = ft_ptr->ymin;
      max[0] = ft_ptr->xmax;
      max[1] = ft_ptr->ymax;
      context->obstacles->Insert( min, max, ft_ptr );

      ft_ptr->fetchCoordinates();
    }


    // feature has to be labeled ?
    if ( context->layer->toLabel )
    {
      // is the feature well defined ? // TODO Check epsilon
      if ( ft_ptr->label_x > 0.0000001 && ft_ptr->label_y > 0.0000001 )
      {

        int i;
        // Hole of the feature are obstacles
        for ( i = 0;i < ft_ptr->nbSelfObs;i++ )
        {
          min[0] = ft_ptr->selfObs[i]->xmin;
          min[1] = ft_ptr->selfObs[i]->ymin;
          max[0] = ft_ptr->selfObs[i]->xmax;
          max[1] = ft_ptr->selfObs[i]->ymax;
          context->obstacles->Insert( min, max, ft_ptr->selfObs[i] );

          if ( !ft_ptr->selfObs[i]->holeOf )
          {
            std::cout << "ERROR: SHOULD HAVE A PARENT!!!!!" << std::endl;
          }
        }


        LinkedList<Feats*> *feats = new LinkedList<Feats*> ( ptrFeatsCompare );

        if (( ft_ptr->type == GEOS_LINESTRING )
            || ft_ptr->type == GEOS_POLYGON )
        {

          double bbx[4], bby[4];

          bbx[0] = context->bbox_min[0];   bbx[1] = context->bbox_max[0];
          bbx[2] = context->bbox_max[0];   bbx[3] = context->bbox_min[0];

          bby[0] = context->bbox_min[1];   bby[1] = context->bbox_min[1];
          bby[2] = context->bbox_max[1];   bby[3] = context->bbox_max[1];

          LinkedList<PointSet*> *shapes = new LinkedList<PointSet*> ( ptrPSetCompare );
          bool outside, inside;

          // Fetch coordinates
          ft_ptr->fetchCoordinates();
          PointSet *shape = ft_ptr->createProblemSpecificPointSet( bbx, bby, &outside, &inside );
          ft_ptr->releaseCoordinates();




          if ( inside )
          {
            // no extra treatment required
            shapes->push_back( shape );
          }
          else
          {
            // feature isn't completly in the math
            if ( ft_ptr->type == GEOS_LINESTRING )
              PointSet::reduceLine( shape, shapes, bbx, bby );
            else
            {
              PointSet::reducePolygon( shape, shapes, bbx, bby );
            }
          }

          while ( shapes->size() > 0 )
          {
            shape = shapes->pop_front();
            Feats *ft = new Feats();
            ft->feature = ft_ptr;
            ft->shape = shape;
            feats->push_back( ft );

#ifdef _EXPORT_MAP_
            if ( !svged )
            {
              toSVGPath( shape->nbPoints, shape->type, shape->x, shape->y,
                         dpi , context->scale,
                         convert2pt( context->bbox_min[0], context->scale, dpi ),
                         convert2pt( context->bbox_max[1], context->scale, dpi ),
                         context->layer->name, ft_ptr->uid, *context->svgmap );
            }
#endif
          }
          delete shapes;
        }
        else
        {
          // Feat is a point
          Feats *ft = new Feats();
          ft->feature = ft_ptr;
          ft->shape = NULL;
          feats->push_back( ft );
        }

        // for earch feature part extracted : generate candidates
        while ( feats->size() > 0 )
        {
          Feats *ft = feats->pop_front();

#ifdef _DEBUG_
          std::cout << "Compute candidates for feat " <<  ft->feature->layer->name << "/" << ft->feature->uid << std::endl;
#endif
          ft->nblp = ft->feature->setPosition( context->scale, & ( ft->lPos ), context->bbox_min, context->bbox_max, ft->shape, context->candidates
#ifdef _EXPORT_MAP_
                                               , *context->svgmap
#endif
                                             );

          delete ft->shape;
          ft->shape = NULL;

          if ( ft->nblp > 0 )
          {
            // valid features are added to fFeats
            ft->priority = context->priority;
            context->fFeats->push_back( ft );
#ifdef _DEBUG_
            std::cout << ft->nblp << " labelPositions for feature : " << ft->feature->layer->name << "/" << ft->feature->uid << std::endl;
#endif
          }
          else
          {
            // Others are deleted
#ifdef _VERBOSE_
            std::cout << "Unable to generate labelPosition for feature : " << ft->feature->layer->name << "/" << ft->feature->uid << std::endl;
#endif
            delete[] ft->lPos;
            delete ft;
          }
        }
        delete feats;
      }
      else   // check labelsize
      {
#ifdef _VERBOSE_
        std::cerr << "Feature " <<  ft_ptr->layer->name << "/" << ft_ptr->uid << " is skipped (label size = 0)" << std::endl;
#endif
      }
    }
    return true;
  }


  double dist_pointToLabel( double xp, double yp, LabelPosition *lp )
  {

    int i;
    int j;

    double mx[4];
    double my[4];

    double dist_min = DBL_MAX;
    double dist;

    for ( i = 0;i < 4;i++ )
    {
      j = ( i + 1 ) % 4;
      mx[i] = ( lp->x[i] + lp->x[j] ) / 2.0;
      my[i] = ( lp->y[i] + lp->y[j] ) / 2.0;
    }


    if ( vabs( cross_product( mx[0], my[0], my[2], my[2], xp, yp ) / lp->h ) < lp->w / 2 )
    {
      dist = cross_product( lp->x[0], lp->y[0], lp->x[1], lp->y[1], xp, yp ) / lp->w;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;

      dist = cross_product( lp->x[2], lp->y[2], lp->x[3], lp->y[3], xp, yp ) / lp->w;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    if ( vabs( cross_product( mx[1], my[1], my[3], my[3], xp, yp ) / lp->w ) < lp->h / 2 )
    {
      dist = cross_product( lp->x[1], lp->y[1], lp->x[2], lp->y[2], xp, yp ) / lp->h;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;

      dist = cross_product( lp->x[3], lp->y[3], lp->x[0], lp->y[0], xp, yp ) / lp->h;
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    for ( i = 0;i < 4;i++ )
    {
      dist = dist_euc2d( lp->x[i], lp->y[i], xp, yp );
      if ( vabs( dist ) < vabs( dist_min ) )
        dist_min = dist;
    }

    return dist_min;
  }


  /*
   *  Check wheter the candidate in ctx overlap with obstacle feat
   */
  bool pruneLabelPositionCallback( LabelPosition *lp, void *ctx )
  {

    PointSet *feat = (( PruneCtx* ) ctx )->obstacle;
    double scale = (( PruneCtx* ) ctx )->scale;
    Pal* pal = (( PruneCtx* ) ctx )->pal;

    if (( feat == lp->feature ) || ( feat->holeOf && feat->holeOf != lp->feature ) )
    {
      return true;
    }

    int n;

    int i, j;
    double ca, cb;

    n = 0;

    //if (!feat->holeOf)
    //((Feature*)feat)->fetchCoordinates();

    double dist;

    double distlabel = unit_convert( double( lp->feature->distlabel ),
                                     pal::PIXEL,
                                     pal->map_unit,
                                     pal->dpi, scale, 1 );



    switch ( feat->type )
    {
        //case geos::geom::GEOS_POINT:
      case GEOS_POINT:

#ifdef _DEBUG_FULL
        std::cout << "    POINT" << std::endl;
#endif

        dist = dist_pointToLabel( feat->x[0], feat->y[0], lp );

        if ( dist < 0 )
          n = 2;
        else if ( dist < distlabel )
          n = 1;
        else
          n = 0;

        break;

        //case geos::geom::GEOS_LINESTRING:
      case GEOS_LINESTRING:
#ifdef _DEBUG_FULL
        std::cout << "    LINE" << std::endl;
#endif
        // Is one of label's boarder cross the line ?
        for ( i = 0;i < 4;i++ )
        {
          for ( j = 0;j < feat->nbPoints - 1;j++ )
          {
            ca = cross_product( lp->x[i], lp->y[i], lp->x[( i+1 ) %4], lp->y[( i+1 ) %4],
                                feat->x[j], feat->y[j] );
            cb = cross_product( lp->x[i], lp->y[i], lp->x[( i+1 ) %4], lp->y[( i+1 ) %4],
                                feat->x[j+1], feat->y[j+1] );

            if (( ca < 0 && cb > 0 ) || ( ca > 0 && cb < 0 ) )
            {
              ca = cross_product( feat->x[j], feat->y[j], feat->x[j+1], feat->y[j+1],
                                  lp->x[i], lp->y[i] );
              cb = cross_product( feat->x[j], feat->y[j], feat->x[j+1], feat->y[j+1],
                                  lp->x[( i+1 ) %4], lp->y[( i+1 ) %4] );
              if (( ca < 0 && cb > 0 ) || ( ca > 0 && cb < 0 ) )
              {
                n = 1;
                i = 4;
                break;
              }
            }
          }
        }
        break;
        //case geos::geom::GEOS_POLYGON:
      case GEOS_POLYGON:
#ifdef _DEBUG_FULL
        std::cout << "    POLY" << std::endl;
#endif
        n =  nbLabelPointInPolygon( feat->nbPoints, feat->x, feat->y, lp->x, lp->y );

        //n<1?n=0:n=1;
        break;
    }

    // label cost is penalized
    lp->cost += double( n );

    //if (!feat->holeOf)
    //((Feature*)feat)->releaseCoordinates();

    return true;
  }

  bool releaseCallback( PointSet *pset, void *ctx )
  {
    if ( pset->holeOf == NULL )
    {
      (( Feature* ) pset )->releaseCoordinates();
    }
    return true;
  }


  void releaseAllInIndex( RTree<PointSet*, double, 2, double> *obstacles )
  {
    /*
    RTree<PointSet*, double, 2, double>::Iterator it;
    for (obstacles->GetFirst(it) ; it.IsNotNull(); obstacles->GetNext(it)){
        if (!(*it)->holeOf){
            std::cout << "Release obs:" << ((Feature*)(*it))->layer->getName() << "/" << ((Feature*)(*it))->uid << std::endl;
            ((Feature*)(*it))->releaseCoordinates();
        }
    }
    */


    double amin[2];
    double amax[2];

    amin[0] = amin[1] = -DBL_MAX;
    amax[0] = amax[1] = DBL_MAX;

    obstacles->Search( amin, amax, releaseCallback, NULL );
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

    if ( pset->holeOf == NULL )
    {
      (( Feature* ) pset )->fetchCoordinates();
    }
    else
    {
      (( Feature* ) pset->holeOf )->fetchCoordinates();
    }

    double amin[2], amax[2];

    amin[0] = pset->xmin;
    amin[1] = pset->ymin;
    amax[0] = pset->xmax;
    amax[1] = pset->ymax;

    PruneCtx pruneContext;

    pruneContext.scale = scale;
    pruneContext.obstacle = pset;
    pruneContext.pal = pal;
    cdtsIndex->Search( amin, amax, pruneLabelPositionCallback, ( void* ) &pruneContext );

    if ( pset->holeOf == NULL )
    {
      (( Feature* ) pset )->releaseCoordinates();
    }
    else
    {
      (( Feature* ) pset->holeOf )->releaseCoordinates();
    }

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

    int i, j, c;

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
    for ( i = 0;i < nbLayers;i++ )
    {
      for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end();it++ ) // iterate on pal->layers
      {
        layer = *it;
        // Only select those who are active and labellable (with scale constraint) or those who are active and which must be treated as obstaclewhich must be treated as obstacle
        if ( layer->active
             && ( layer->obstacle || ( layer->toLabel && layer->isScaleValid( scale ) ) ) )
        {

          // check if this selected layers has been selected by user
          if ( strcmp( layersName[i], layer->name ) == 0 )
          {
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
    for ( i = 0;i < prob->nbLabelledLayers;i++ )
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
      releaseAllInIndex( obstacles );
      delete obstacles;
      return NULL;
    }

    prob->nbft = fFeats->size();
    prob->nblp = 0;
    prob->featNbLp = new int [prob->nbft];
    prob->featStartId = new int [prob->nbft];
    prob->inactiveCost = new double[prob->nbft];

    Feats *feat;

    std::cout << "FIRSST NBFT : " << prob->nbft << std::endl;

    // Filtering label positions against obstacles
    amin[0] = amin[1] = -DBL_MAX;
    amax[0] = amax[1] = DBL_MAX;
    FilterContext filterCtx;
    filterCtx.cdtsIndex = prob->candidates;
    filterCtx.scale = prob->scale;
    filterCtx.pal = this;
    obstacles->Search( amin, amax, filteringCallback, ( void* ) &filterCtx );


    int idlp = 0;
    for ( i = 0;i < prob->nbft;i++ ) /* foreach feature into prob */
    {
      feat = fFeats->pop_front();
#ifdef _DEBUG_FULL_
      std::cout << "Feature:" << feat->feature->layer->name << "/" << feat->feature->uid << std::endl;
#endif
      prob->featStartId[i] = idlp;
      prob->inactiveCost[i] = pow( 2, 10 - 10 * feat->priority );

      switch ( feat->feature->type )
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

      // If candidates list is smaller than expected
      if ( max_p > feat->nblp )
        max_p = feat->nblp;
      //
      // sort candidates list, best label to worst
      sort(( void** ) feat->lPos, feat->nblp, costGrow );

      // try to exclude all conflitual labels (good ones have cost < 1 by pruning)
      double discrim = 0.0;
      int stop;
      do
      {
        discrim += 1.0;
        for ( stop = 0;stop < feat->nblp && feat->lPos[stop]->cost < discrim;stop++ );
      }
      while ( stop == 0 && discrim < feat->lPos[feat->nblp-1]->cost + 2.0 );

      if ( discrim > 1.5 )
      {
        int k;
        for ( k = 0;k < stop;k++ )
          feat->lPos[k]->cost = 0.0021;
      }

      if ( max_p > stop )
        max_p = stop;

#ifdef _DEBUG_FULL_
      std::cout << "Nblabel kept for feat " << feat->feature->uid << "/" << feat->feature->layer->name << ": " << max_p << "/" << feat->nblp << std::endl;
#endif

      // Sets costs for candidates of polygon
      if ( feat->feature->type == GEOS_POLYGON && ( feat->feature->layer->arrangement == P_FREE || feat->feature->layer->arrangement == P_HORIZ ) )
        LabelPosition::setCost( stop, feat->lPos, max_p, obstacles, bbx, bby );

#ifdef _DEBUG_FULL_
      std::cout << "All Cost are setted" << std::endl;
#endif
      // only keep the 'max_p' best candidates
      for ( j = max_p;j < feat->nblp;j++ )
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
      for ( j = 0;j < feat->nblp;j++, idlp++ )
      {
        lp = feat->lPos[j];
        //lp->insertIntoIndex(prob->candidates);
        lp->id = idlp;
        lp->probFeat = i; // bugfix #1 (maxence 10/23/2008)
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
      for ( i = 0;i < feat->nblp;i++, idlp++ )  // foreach label candidate
      {
        lp = feat->lPos[i];
        lp->nbOverlap = 0;

        if ( lp->cost >= 1 )
        {
          std::cout << " Warning: lp->cost == " << lp->cost << " (from feat: " << lp->feature->uid << "/" << lp->feature->layer->name << ")" << std::endl;
          lp->cost -= int ( lp->cost ); // label cost up to 1
        }

        prob->labelpositions[idlp] = lp;
        //prob->feat[idlp] = j;


        amin[0] = DBL_MAX;
        amax[0] = -DBL_MAX;
        amin[1] = DBL_MAX;
        amax[1] = -DBL_MAX;
        for ( c = 0;c < 4;c++ )
        {
          if ( lp->x[c] < amin[0] )
            amin[0] = lp->x[c];
          if ( lp->x[c] > amax[0] )
            amax[0] = lp->x[c];
          if ( lp->y[c] < amin[1] )
            amin[1] = lp->y[c];
          if ( lp->y[c] > amax[1] )
            amax[1] = lp->y[c];
        }
        // lookup for overlapping candidate
        prob->candidates->Search( amin, amax, countOverlapCallback, ( void* ) lp );

        nbOverlaps += lp->nbOverlap;
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
    releaseAllInIndex( obstacles );
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

    prob->reduce();

#ifdef _VERBOSE_
    std::cerr << prob->nblp << "\t"
              << prob->nbOverlap;
#endif

    return prob;
  }

  std::list<Label*>* Pal::labeller( double scale, double bbox[4], PalStat **stats, bool displayAll )
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
    for ( std::list<Layer*>::iterator it = layers->begin(); it != layers->end();it++ )
    {
      layer = *it;
      layersName[i] = layer->name;
      priorities[i] = layer->defaultPriority;
      i++;
    }
    lyrsMutex->unlock();

    std::list<Label*> * solution = labeller( nbLayers, layersName, priorities, scale, bbox, stats, displayAll );

    delete[] layersName;
    delete[] priorities;
    return solution;
  }


  /*
   * BIG MACHINE
   */
  std::list<Label*>* Pal::labeller( int nbLayers, char **layersName , double *layersFactor, double scale, double bbox[4], PalStat **stats, bool displayAll )
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
      return new std::list<Label*>();
    }

    prob->displayAll = displayAll;

#ifdef _VERBOSE_
    create_time = double( clock() - start ) / double( CLOCKS_PER_SEC );

    std::cout << std::endl << "Problem : " << prob->nblp << " candidates for " << prob->nbft << " features makes " << prob->nbOverlap << " overlaps" << std::endl;
    std::cout << std::endl << "Times:"  << std::endl << "    to create problem:  " << create_time << std::endl;
#endif

    // search a solution
    if ( searchMethod != CHAIN )
      prob->popmusic();
    else
      prob->chain_search();

    // Post-Optimization
    //prob->post_optimization();


    std::list<Label*> * solution = prob->getSolution( displayAll );

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

