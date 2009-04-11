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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE


#if defined(_VERBOSE_) || (_DEBUG_)
#include <iostream>
#endif


#include <cmath>
#include <cstring>
#include <cfloat>

#include <pal/pal.h>
#include <pal/layer.h>

#include "linkedlist.hpp"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "pointset.h"
#include "simplemutex.h"
#include "util.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace pal
{

  Feature::Feature( Feat *feat, Layer *layer, int part, int nPart, PalGeometry *userGeom ) :
      layer( layer ), nPart( nPart ), part( part ), userGeom( userGeom )
  {

    this->uid = new char[strlen( feat->id ) +1];
    strcpy( this->uid, feat->id );

    label_x = -1;
    label_y = -1;

    xmin = feat->minmax[0];
    xmax = feat->minmax[2];
    ymin = feat->minmax[1];
    ymax = feat->minmax[3];

    nbPoints = feat->nbPoints;
    x = feat->x;
    y = feat->y;


    int i;


    nbSelfObs = feat->nbHoles;
    selfObs = feat->holes;

    holeOf = NULL;
    for ( i = 0;i < nbSelfObs;i++ )
    {
      selfObs[i]->holeOf = this;
    }


    this->type = feat->type;

#ifdef _DEBUG_
    std::cout << "Corrdinates for " << layer->name << "/" << uid << " :" << nbPoints << " pts" << std::endl;
    for ( i = 0;i < nbPoints;i++ )
    {
      std::cout << x[i] << ";" << y[i] << std::endl;
    }
#endif

    distlabel = 0;
    currentAccess = 0;

    accessMutex = new SimpleMutex();
  }


  Feature::~Feature()
  {
    if ( x || y )
    {
      std::cout << "Warning: coordinates not released: " << layer->name << "/" << uid << std::endl;
    }

    if ( uid )
    {
      delete[] uid;
    }

    if ( nbSelfObs )
    {
      int i;
      for ( i = 0;i < nbSelfObs;i++ )
      {
        if ( selfObs[i]->x || selfObs[i]->y )
        {
          std::cout << "Warning: hole coordinates not released" << std::endl;
        }
        delete selfObs[i];
      }
      delete[] selfObs;
    }

    delete accessMutex;
  }

  Layer *Feature::getLayer()
  {
    return layer;
  }


  const char * Feature::getUID()
  {
    return uid;
  }

  int Feature::setPositionForPoint( double x, double y, double scale, LabelPosition ***lPos, double delta_width )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (point) : " << layer->name << "/" << uid << std::endl;
#endif

    int dpi = layer->pal->dpi;


    double xrm;
    double yrm;

    xrm = unit_convert( label_x,
                        layer->label_unit,
                        layer->pal->map_unit,
                        dpi, scale, delta_width );

    yrm = unit_convert( label_y,
                        layer->label_unit,
                        layer->pal->map_unit,
                        dpi, scale, delta_width );

    int nbp = layer->pal->point_p;

    //std::cout << "Nbp : " << nbp << std::endl;

    int i;
    int icost = 0;
    int inc = 2;

    double alpha;
    double beta = 2 * M_PI / nbp; /* angle bw 2 pos */

    // uncomment for Wolff 2 position model test on RailwayStation
    //if (nbp==2)
    //   beta = M_PI/2;

    double distlabel = unit_convert( this->distlabel,
                                     pal::PIXEL,
                                     layer->pal->map_unit,
                                     dpi, scale, delta_width );

    double lx, ly; /* label pos */

    /* various alpha */
    double a90  = M_PI / 2;
    double a180 = M_PI;
    double a270 = a180 + a90;
    double a360 = 2 * M_PI;


    double gamma1, gamma2;

    if ( distlabel > 0 )
    {
      gamma1 = atan2( yrm / 2, distlabel + xrm / 2 );
      gamma2 = atan2( xrm / 2, distlabel + yrm / 2 );
    }
    else
    {
      gamma1 = gamma2 = a90 / 3.0;
    }


    if ( gamma1 > a90 / 3.0 )
      gamma1 = a90 / 3.0;

    if ( gamma2 > a90 / 3.0 )
      gamma2 = a90 / 3.0;


    if ( gamma1 == 0 || gamma2 == 0 )
    {
      std::cout << "Oups... label size error..." << std::endl;
    }

    *lPos = new LabelPosition *[nbp];

    for ( i = 0, alpha = M_PI / 4;i < nbp;i++, alpha += beta )
    {
      lx = x;
      ly = y;

      if ( alpha > a360 )
        alpha -= a360;

      if ( alpha < gamma1 || alpha > a360 - gamma1 )  // on the right
      {
        lx += distlabel;
        double iota = ( alpha + gamma1 );
        if ( iota > a360 - gamma1 )
          iota -= a360;

        //ly += -yrm/2.0 + tan(alpha)*(distlabel + xrm/2);
        ly += -yrm + yrm * iota / ( 2 * gamma1 );
      }
      else if ( alpha < a90 - gamma2 )  // top-right
      {
        lx += distlabel * cos( alpha );
        ly += distlabel * sin( alpha );
      }
      else if ( alpha < a90 + gamma2 ) // top
      {
        //lx += -xrm/2.0 - tan(alpha+a90)*(distlabel + yrm/2);
        lx += -xrm * ( alpha - a90 + gamma2 ) / ( 2 * gamma2 ) ;
        ly += distlabel;
      }
      else if ( alpha < a180 - gamma1 )  // top left
      {
        lx += distlabel * cos( alpha ) - xrm;
        ly += distlabel * sin( alpha );
      }
      else if ( alpha < a180 + gamma1 ) // left
      {
        lx += -distlabel - xrm;
        //ly += -yrm/2.0 - tan(alpha)*(distlabel + xrm/2);
        ly += - ( alpha - a180 + gamma1 ) * yrm / ( 2 * gamma1 );
      }
      else if ( alpha < a270 - gamma2 ) // down - left
      {
        lx += distlabel * cos( alpha ) - xrm;
        ly += distlabel * sin( alpha ) - yrm;
      }
      else if ( alpha < a270 + gamma2 ) // down
      {
        ly += -distlabel - yrm;
        //lx += -xrm/2.0 + tan(alpha+a90)*(distlabel + yrm/2);
        lx += -xrm + ( alpha - a270 + gamma2 ) * xrm / ( 2 * gamma2 );
      }
      else if ( alpha < a360 )
      {
        lx += distlabel * cos( alpha );
        ly += distlabel * sin( alpha ) - yrm;
      }

      double cost;

      if ( nbp == 1 )
        cost = 0.0001;
      else
        cost =  0.0001 + 0.0020 * double( icost ) / double( nbp - 1 );

      ( *lPos )[i] = new LabelPosition( i, lx, ly, xrm, yrm, 0, cost,  this );

      icost += inc;

      if ( icost == nbp )
      {
        icost = nbp - 1;
        inc = -2;
      }
      else if ( icost > nbp )
      {
        icost = nbp - 2;
        inc = -2;
      }

    }

    return nbp;
  }

// TODO work with squared distance by remonving call to sqrt or dist_euc2d
  int Feature::setPositionForLine( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width )
  {
#ifdef _DEBUG_
    std::cout << "SetPosition (line) : " << layer->name << "/" << uid << std::endl;
#endif
    int i;
    int dpi = layer->pal->dpi;
    double xrm, yrm;

    xrm = unit_convert( label_x,
                        layer->label_unit,
                        layer->pal->map_unit,
                        dpi, scale, delta_width );

    yrm = unit_convert( label_y,
                        layer->label_unit,
                        layer->pal->map_unit,
                        dpi, scale, delta_width );


    double distlabel = unit_convert( this->distlabel,
                                     pal::PIXEL,
                                     layer->pal->map_unit,
                                     dpi, scale, delta_width );


    double *d; // segments lengths distance bw pt[i] && pt[i+1]
    double *ad;  // absolute distance bw pt[0] and pt[i] along the line
    double ll; // line length
    double dist;
    double bx, by, ex, ey;
    int nbls;
    double alpha;
    double cost;

    //LinkedList<PointSet*> *shapes_final;

    //shapes_final     = new LinkedList<PointSet*>(ptrPSetCompare);

    LinkedList<LabelPosition*> *positions = new LinkedList<LabelPosition*> ( ptrLPosCompare );

    int nbPoints;
    double *x;
    double *y;

    PointSet * line = mapShape;
#ifdef _DEBUG_FULL_
    std::cout << "New line of " << line->nbPoints << " points with label " << xrm << "x" << yrm << std::endl;
#endif

    nbPoints = line->nbPoints;
    x = line->x;
    y = line->y;

    d = new double[nbPoints-1];
    ad = new double[nbPoints];

    ll = 0.0; // line length
    for ( i = 0;i < line->nbPoints - 1;i++ )
    {
      if ( i == 0 )
        ad[i] = 0;
      else
        ad[i] = ad[i-1] + d[i-1];

      d[i] = dist_euc2d( x[i], y[i], x[i+1], y[i+1] );
      ll += d[i];
    }

    ad[line->nbPoints-1] = ll;


    nbls = ( int )( ll / xrm ); // ratio bw line length and label width

#ifdef _DEBUG_FULL_
    std::cout << "line length :" << ll << std::endl;
    std::cout << "nblp :" << nbls << std::endl;
#endif

    dist = ( ll - xrm );

    double l;

    if ( nbls > 0 )
    {
      //dist /= nbls;
      l = 0;
      dist = min( yrm, xrm );
    }
    else   // line length < label with => centering label position
    {
      l = - ( xrm - ll ) / 2.0;
      dist = xrm;
      ll = xrm;
    }

    double birdfly;
    double beta;
    i = 0;
    //for (i=0;i<nbp;i++){
#ifdef _DEBUG_FULL_
    std::cout << l << " / " << ll - xrm << std::endl;
#endif
    while ( l < ll - xrm )
    {
      // => bx, by
      line->getPoint( d, ad, l, &bx, &by );
      // same but l = l+xrm
      line->getPoint( d, ad, l + xrm, &ex, &ey );

      // Label is bigger than line ...
      if ( l < 0 )
        birdfly = sqrt(( x[nbPoints-1] - x[0] ) * ( x[nbPoints-1] - x[0] )
                       + ( y[nbPoints-1] - y[0] ) * ( y[nbPoints-1] - y[0] ) );
      else
        birdfly = sqrt(( ex - bx ) * ( ex - bx ) + ( ey - by ) * ( ey - by ) );

      cost = birdfly / xrm;
      if ( cost > 0.98 )
        cost = 0.0001;
      else
        cost = ( 1 - cost ) / 100;


      if (( vabs( ey - by ) < EPSILON ) && ( vabs( ex - bx ) < EPSILON ) )
      {
        std::cout << "EPSILON " << EPSILON << std::endl;
        std::cout << "b: " << bx << ";" << by << std::endl;
        std::cout << "e: " << ex << ";" << ey << std::endl;
        alpha = 0.0;
      }
      else
        alpha = atan2( ey - by, ex - bx );

      beta = alpha + M_PI / 2;

#ifdef _DEBUG_FULL_
      std::cout << "  Create new label" << std::endl;
#endif
      if ( layer->arrangement == P_LINE_AROUND )
      {
        positions->push_back( new LabelPosition( i, bx + cos( beta ) *distlabel , by + sin( beta ) *distlabel, xrm, yrm, alpha, cost, this ) ); // Line
        positions->push_back( new LabelPosition( i, bx - cos( beta ) * ( distlabel + yrm ) , by - sin( beta ) * ( distlabel + yrm ), xrm, yrm, alpha, cost, this ) ); // Line
      }
      /*else if (layer->arrangement == P_HORIZ){ // TODO add P_HORIZ
         positions->push_back (new LabelPosition (i, bx -yrm/2, by - yrm*sin(beta)/2, xrm, yrm, alpha, cost, this, line)); // Line
        line->aliveCandidates++;
      }*/
      else
      {
        positions->push_back( new LabelPosition( i, bx - yrm*cos( beta ) / 2, by - yrm*sin( beta ) / 2, xrm, yrm, alpha, cost, this ) ); // Line
      }

      l += dist;

      i++;

      if ( nbls == 0 )
        break;
    }

    //delete line;

    delete[] d;
    delete[] ad;

    int nbp = positions->size();
    *lPos = new LabelPosition *[nbp];
    i = 0;
    while ( positions->size() > 0 )
    {
      ( *lPos )[i] = positions->pop_front();
      i++;
    }

    delete positions;

    return nbp;
  }


  /*
   *             seg 2
   *     pt3 ____________pt2
   *        ¦            ¦
   *        ¦            ¦
   * seg 3  ¦    BBOX    ¦ seg 1
   *        ¦            ¦
   *        ¦____________¦
   *     pt0    seg 0    pt1
   *
   */

  int Feature::setPositionForPolygon( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (polygon) : " << layer->name << "/" << uid << std::endl;
#endif

    int i;
    int j;

    double xrm;
    double yrm;

    xrm = unit_convert( label_x,
                        layer->label_unit,
                        layer->pal->map_unit,
                        layer->pal->dpi, scale, delta_width );

    yrm = unit_convert( label_y,
                        layer->label_unit,
                        layer->pal->map_unit,
                        layer->pal->dpi, scale, delta_width );

    //print();

    //LinkedList<PointSet*> *shapes_toCut;
    LinkedList<PointSet*> *shapes_toProcess;
    LinkedList<PointSet*> *shapes_final;

    //shapes_toCut     = new LinkedList<PointSet*>(ptrPSetCompare);
    shapes_toProcess = new LinkedList<PointSet*> ( ptrPSetCompare );
    shapes_final     = new LinkedList<PointSet*> ( ptrPSetCompare );

    mapShape->parent = NULL;

    shapes_toProcess->push_back( mapShape );

    splitPolygons( shapes_toProcess, shapes_final, xrm, yrm, uid );


    delete shapes_toProcess;

    int nbp;

    if ( shapes_final->size() > 0 )
    {
      LinkedList<LabelPosition*> *positions = new LinkedList<LabelPosition*> ( ptrLPosCompare );
      int it;

      double dlx, dly; // delta from label center and bottom-left corner
      double alpha = 0.0; // rotation for the label
      double px, py;
      double dx;
      double dy;
      int bbid;
      double beta;
      double diago = sqrt( xrm * xrm / 4.0 + yrm * yrm / 4 );
      double rx, ry;
      CHullBox **boxes = new CHullBox*[shapes_final->size()];
      j = 0;

      // Compute bounding box foreach finalShape
      while ( shapes_final->size() > 0 )
      {
        PointSet *shape = shapes_final->pop_front();
        boxes[j] = shape->compute_chull_bbox();

        if ( shape->parent )
          delete shape;

        j++;
      }

      it = 0;
      dx = dy = min( yrm, xrm ) / 2;

      int num_try = 0;
      int max_try = 10;
      do
      {
        for ( bbid = 0;bbid < j;bbid++ )
        {
          CHullBox *box = boxes[bbid];

          if (( box->length * box->width ) > ( xmax - xmin ) * ( ymax - ymin ) *5 )
          {
            std::cout << "Very Large BBOX (should never occurs : bug-report please)" << std::endl;
            std::cout << "   Box size:  " << box->length << "/" << box->width << std::endl;
            std::cout << "   Alpha:     " << alpha << "   " << alpha * 180 / M_PI << std::endl;
            std::cout << "   Dx;Dy:     " << dx << "   " << dy  << std::endl;
            std::cout << "   LabelSizerm: " << xrm << "   " << yrm  << std::endl;
            std::cout << "   LabelSizeUn: " << label_x << "   " << label_y << std::endl;
            continue;
          }

#ifdef _DEBUG_FULL_
          std::cout << "New BBox : " << bbid << std::endl;
          for ( i = 0;i < 4;i++ )
          {
            std::cout << box->x[i] << "\t" << box->y[i] << std::endl;
          }
#endif

          bool enoughPlace = false;
          if ( layer->getArrangement() == P_FREE )
          {
            enoughPlace = true;
            px = ( box->x[0] + box->x[2] ) / 2 - xrm;
            py = ( box->y[0] + box->y[2] ) / 2 - yrm;
            int i, j;

            // Virtual label: center on bbox center, label size = 2x original size
            // alpha = 0.
            // If all corner are in bbox then place candidates horizontaly
            for ( rx = px, i = 0; i < 2; rx = rx + 2 * xrm, i++ )
            {
              for ( ry = py, j = 0; j < 2; ry = ry + 2 * yrm, j++ )
              {
                // TODO should test with the polyone insteand of the bbox
                if ( !isPointInPolygon( 4, box->x, box->y, rx, ry ) )
                {
                  enoughPlace = false;
                  break;
                }
              }
              if ( !enoughPlace )
              {
                break;
              }
            }

          } // arrangement== FREE ?

          if ( layer->getArrangement() == P_HORIZ || enoughPlace )
          {
            alpha = 0.0; // HORIZ
          }
          else if ( box->length > 1.5*xrm && box->width > 1.5*xrm )
          {
            if ( box->alpha <= M_PI / 4 )
            {
              alpha = box->alpha;
            }
            else
            {
              alpha = box->alpha - M_PI / 2;
            }
          }
          else if ( box->length > box->width )
          {
            alpha = box->alpha - M_PI / 2;
          }
          else
          {
            alpha = box->alpha;
          }

          beta  = atan2( yrm, xrm ) + alpha;


          //alpha = box->alpha;

          // delta from label center and down-left corner
          dlx = cos( beta ) * diago;
          dly = sin( beta ) * diago;


          double px0, py0;

          px0 = box->width / 2.0;
          py0 = box->length / 2.0;

          px0 -= ceil( px0 / dx ) * dx;
          py0 -= ceil( py0 / dy ) * dy;

          for ( px = px0;px <= box->width;px += dx )
          {
            for ( py = py0;py <= box->length;py += dy )
            {

              rx = cos( box->alpha ) * px + cos( box->alpha - M_PI / 2 ) * py;
              ry = sin( box->alpha ) * px + sin( box->alpha - M_PI / 2 ) * py;

              rx += box->x[0];
              ry += box->y[0];

              // Only accept candidate that center is in the polygon
              if ( isPointInPolygon( mapShape->nbPoints, mapShape->x, mapShape->y, rx,  ry ) )
              {
                positions->push_back( new LabelPosition( 0, rx - dlx, ry - dly , xrm, yrm, alpha, 0.0001, this ) ); // Polygon
              }
            }
          }
        } // forall box

        nbp = positions->size();
        if ( nbp == 0 )
        {
          dx /= 2;
          dy /= 2;
          num_try++;
        }
      }
      while ( nbp == 0 && num_try < max_try );

      nbp = positions->size();

      ( *lPos ) = new LabelPosition*[nbp];
      for ( i = 0;i < nbp;i++ )
      {
        ( *lPos )[i] = positions->pop_front();
        ( *lPos )[i]->id = i;
      }

      for ( bbid = 0;bbid < j;bbid++ )
      {
        delete boxes[bbid];
      }

      delete[] boxes;
      delete positions;
    }
    else
    {
      nbp = 0;
    }

    delete shapes_final;

#ifdef _DEBUG_FULL_
    std::cout << "NbLabelPosition: " << nbp << std::endl;
#endif
    return nbp;
  }

  void Feature::print()
  {
    int i, j;
    std::cout << "Geometry id : " << uid << std::endl;
    std::cout << "Type: " << type << std::endl;
    if ( x && y )
    {
      for ( i = 0;i < nbPoints;i++ )
        std::cout << x[i] << ", " << y[i] << std::endl;
      std::cout << "Obstacle: " << nbSelfObs << std::endl;
      for ( i = 0;i < nbSelfObs;i++ )
      {
        std::cout << "  obs " << i << std::endl;
        for ( j = 0;j < selfObs[i]->nbPoints;j++ )
        {
          std::cout << selfObs[i]->x[j] << ";" << selfObs[i]->y[j] << std::endl;
        }
      }
    }

    std::cout << std::endl;
  }

  int Feature::setPosition( double scale, LabelPosition ***lPos,
                            double bbox_min[2], double bbox_max[2],
                            PointSet *mapShape, RTree<LabelPosition*, double, 2, double> *candidates
#ifdef _EXPORT_MAP_
                            , std::ofstream &svgmap
#endif
                          )
  {
    int nbp = 0;
    int i;
    double bbox[4];

#ifdef _EXPORT_MAP_
    int dpi = layer->pal->getDpi();
#endif

    bbox[0] = bbox_min[0];
    bbox[1] = bbox_min[1];
    bbox[2] = bbox_max[0];
    bbox[3] = bbox_max[1];

    double delta = bbox_max[0] - bbox_min[0];

    switch ( type )
    {
      case GEOS_POINT:
        fetchCoordinates();
        nbp = setPositionForPoint( x[0], y[0], scale, lPos, delta );
#ifdef _EXPORT_MAP_
        toSVGPath( nbPoints, type, x, y, dpi , scale,
                   convert2pt( bbox_min[0], scale, dpi ),
                   convert2pt( bbox_max[1], scale, dpi ),
                   layer->name, uid, svgmap );
#endif
        releaseCoordinates();
        break;
      case GEOS_LINESTRING:
        nbp = setPositionForLine( scale, lPos, mapShape, delta );
        break;

      case GEOS_POLYGON:
        switch ( layer->getArrangement() )
        {
          case P_POINT:
            double cx, cy;
            mapShape->getCentroid( cx, cy );
            nbp = setPositionForPoint( cx, cy, scale, lPos, delta );
            break;
          case P_LINE:
          case P_LINE_AROUND:
            nbp = setPositionForLine( scale, lPos, mapShape, delta );
            break;
          default:
            nbp = setPositionForPolygon( scale, lPos, mapShape, delta );
            break;
        }
    }

    int rnbp = nbp;

    // purge candidates that are outside the bbox
    for ( i = 0;i < nbp;i++ )
    {
      if ( !( *lPos )[i]->isIn( bbox ) )
      {
        rnbp--;
        ( *lPos )[i]->cost = DBL_MAX;
      }
      else   // this one is OK
      {
        ( *lPos )[i]->insertIntoIndex( candidates );
      }
    }

    sort(( void** )( *lPos ), nbp, costGrow );

    for ( i = rnbp;i < nbp;i++ )
    {
      delete( *lPos )[i];
    }

    return rnbp;
  }


  void Feature::fetchCoordinates()
  {
    accessMutex->lock();
    layer->pal->tmpTime -= clock();
    if ( !x && !y )
    {
      //std::cout << "fetch feat " << layer->name << "/" << uid << std::endl;
      the_geom = userGeom->getGeosGeometry();
      LinkedList<Feat*> *feats = splitGeom( the_geom, this->uid );
      int id = 0;
      while ( feats->size() > 0 )
      {
        Feat *f = feats->pop_front();
        if ( id == this->part )
        {
          x = f->x;
          y = f->y;
          int i;
          for ( i = 0;i < nbSelfObs;i++ )
          {
            selfObs[i]->x = f->holes[i]->x;
            selfObs[i]->y = f->holes[i]->y;
            f->holes[i]->x = NULL;
            f->holes[i]->y = NULL;
            delete f->holes[i];
            selfObs[i]->holeOf = this;
          }
          if ( f->holes )
            delete[] f->holes;
          delete f;
        }
        else
        {
          delete[] f->x;
          delete[] f->y;
          int i;
          for ( i = 0;i < f->nbHoles;i++ )
            delete f->holes[i];
          if ( f->holes )
            delete[] f->holes;
          delete f;
        }

        id++;
      }
      delete feats;
    }
    currentAccess++;
    layer->pal->tmpTime += clock();
    accessMutex->unlock();
  }


  void Feature::deleteCoord()
  {
    if ( x && y )
    {
      int i;
      delete[] x;
      delete[] y;
      x = NULL;
      y = NULL;
      for ( i = 0;i < nbSelfObs;i++ )
      {
        delete[] selfObs[i]->x;
        delete[] selfObs[i]->y;
        selfObs[i]->x = NULL;
        selfObs[i]->y = NULL;
      }
    }
  }



  void Feature::releaseCoordinates()
  {
    accessMutex->lock();
    //std::cout << "release (" << currentAccess << ")" << std::endl;
    if ( x && y && currentAccess == 1 )
    {
      deleteCoord();
      userGeom->releaseGeosGeometry( the_geom );
    }
    currentAccess--;
    accessMutex->unlock();
  }

} // end namespace pal
