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
  Feature::Feature( Layer* l, const char* geom_id, PalGeometry* userG, double lx, double ly )
      : layer( l ), userGeom( userG ), label_x( lx ), label_y( ly ), distlabel( 0 ), labelInfo( NULL )
  {
    uid = new char[strlen( geom_id ) +1];
    strcpy( uid, geom_id );
  }

  Feature::~Feature()
  {
    delete[] uid;
  }

  ////////////

  FeaturePart::FeaturePart( Feature *feat, const GEOSGeometry* geom )
      : f( feat ), nbHoles( 0 ), holes( NULL )
  {
    // we'll remove const, but we won't modify that geometry
    the_geom = const_cast<GEOSGeometry*>( geom );
    ownsGeom = false; // geometry is owned by Feature class

    extractCoords( geom );

    holeOf = NULL;
    for ( int i = 0; i < nbHoles; i++ )
    {
      holes[i]->holeOf = this;
    }
  }


  FeaturePart::~FeaturePart()
  {
    // X and Y are deleted in PointSet

    if ( holes )
    {
      for ( int i = 0; i < nbHoles; i++ )
        delete holes[i];
      delete [] holes;
      holes = NULL;
    }

    if ( ownsGeom )
    {
      GEOSGeom_destroy( the_geom );
      the_geom = NULL;
    }
  }


  /*
   * \brief read coordinates from a GEOS geom
   */
  void FeaturePart::extractCoords( const GEOSGeometry* geom )
  {
    int i, j;

    const GEOSCoordSequence *coordSeq;

    type = GEOSGeomTypeId( geom );

    if ( type == GEOS_POLYGON )
    {
      if ( GEOSGetNumInteriorRings( geom ) > 0 )
      {
        // set nbHoles, holes member variables
        nbHoles = GEOSGetNumInteriorRings( geom );
        holes = new PointSet*[nbHoles];

        for ( i = 0; i < nbHoles; i++ )
        {
          holes[i] = new PointSet();
          holes[i]->holeOf = NULL;

          const GEOSGeometry* interior =  GEOSGetInteriorRingN( geom, i );
          holes[i]->nbPoints = GEOSGetNumCoordinates( interior );
          holes[i]->x = new double[holes[i]->nbPoints];
          holes[i]->y = new double[holes[i]->nbPoints];

          holes[i]->xmin = holes[i]->ymin = DBL_MAX;
          holes[i]->xmax = holes[i]->ymax = -DBL_MAX;

          coordSeq = GEOSGeom_getCoordSeq( interior );

          for ( j = 0; j < holes[i]->nbPoints; j++ )
          {
            GEOSCoordSeq_getX( coordSeq, j, &holes[i]->x[j] );
            GEOSCoordSeq_getY( coordSeq, j, &holes[i]->y[j] );

            holes[i]->xmax = holes[i]->x[j] > holes[i]->xmax ? holes[i]->x[j] : holes[i]->xmax;
            holes[i]->xmin = holes[i]->x[j] < holes[i]->xmin ? holes[i]->x[j] : holes[i]->xmin;

            holes[i]->ymax = holes[i]->y[j] > holes[i]->ymax ? holes[i]->y[j] : holes[i]->ymax;
            holes[i]->ymin = holes[i]->y[j] < holes[i]->ymin ? holes[i]->y[j] : holes[i]->ymin;
          }

          reorderPolygon( holes[i]->nbPoints, holes[i]->x, holes[i]->y );
        }
      }

      // use exterior ring for the extraction of coordinates that follows
      geom = GEOSGetExteriorRing( geom );
    }
    else
    {
      nbHoles = 0;
      holes = NULL;
    }

    // find out number of points
    nbPoints = GEOSGetNumCoordinates( geom );
    coordSeq = GEOSGeom_getCoordSeq( geom );

    // initialize bounding box
    xmin = ymin = DBL_MAX;
    xmax = ymax = -DBL_MAX;

    // initialize coordinate arrays
    x = new double[nbPoints];
    y = new double[nbPoints];

    for ( i = 0; i < nbPoints; i++ )
    {
      GEOSCoordSeq_getX( coordSeq, i, &x[i] );
      GEOSCoordSeq_getY( coordSeq, i, &y[i] );

      xmax = x[i] > xmax ? x[i] : xmax;
      xmin = x[i] < xmin ? x[i] : xmin;

      ymax = y[i] > ymax ? y[i] : ymax;
      ymin = y[i] < ymin ? y[i] : ymin;
    }
  }

  void FeaturePart::removeDuplicatePoints()
  {
    // TODO add simplify() process
    int new_nbPoints = nbPoints;
    bool *ok = new bool[new_nbPoints];
    int i, j;

    for ( i = 0; i < nbPoints; i++ )
    {
      ok[i] = true;
      j = ( i + 1 ) % nbPoints;
      if ( i == j )
        break;
      if ( vabs( x[i] - x[j] ) < 0.0000001 && vabs( y[i] - y[j] ) < 0.0000001 )
      {
        new_nbPoints--;
        ok[i] = false;
      }
    }

    if ( new_nbPoints < nbPoints )
    {
      double *new_x = new double[new_nbPoints];
      double *new_y = new double[new_nbPoints];
      for ( i = 0, j = 0; i < nbPoints; i++ )
      {
        if ( ok[i] )
        {
          new_x[j] = x[i];
          new_y[j] = y[i];
          j++;
        }
      }
      delete[] x;
      delete[] y;
      // interchange the point arrays
      x = new_x;
      y = new_y;
      nbPoints = new_nbPoints;
    }

    delete[] ok;
  }




  Layer *FeaturePart::getLayer()
  {
    return f->layer;
  }


  const char * FeaturePart::getUID()
  {
    return f->uid;
  }

  int FeaturePart::setPositionOverPoint( double x, double y, double scale, LabelPosition ***lPos, double delta_width )
  {
    int nbp = 3;
    *lPos = new LabelPosition *[nbp];

    // get from feature
    double label_x = f->label_x;
    double label_y = f->label_y;

    double lx = x - label_x / 2;
    double ly = y - label_y / 2;
    double cost = 0.0001;
    int id = 0;
    double alpha = 0;

    double offset = label_x / 4;

    // at the center
    ( *lPos )[0] = new LabelPosition( id, lx, ly, label_x, label_y, alpha, cost,  this );
    // shifted to the sides - with higher cost
    cost = 0.0021;
    ( *lPos )[1] = new LabelPosition( id, lx + offset, ly, label_x, label_y, alpha, cost,  this );
    ( *lPos )[2] = new LabelPosition( id, lx - offset, ly, label_x, label_y, alpha, cost,  this );
    return nbp;
  }

  int FeaturePart::setPositionForPoint( double x, double y, double scale, LabelPosition ***lPos, double delta_width )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (point) : " << layer->name << "/" << uid << std::endl;
#endif

    int dpi = f->layer->pal->dpi;


    double xrm;
    double yrm;
    double distlabel = f->distlabel;

    xrm = unit_convert( f->label_x,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        dpi, scale, delta_width );

    yrm = unit_convert( f->label_y,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        dpi, scale, delta_width );

    int nbp = f->layer->pal->point_p;

    //std::cout << "Nbp : " << nbp << std::endl;

    int i;
    int icost = 0;
    int inc = 2;

    double alpha;
    double beta = 2 * M_PI / nbp; /* angle bw 2 pos */

    // uncomment for Wolff 2 position model test on RailwayStation
    //if (nbp==2)
    //   beta = M_PI/2;

    /*double distlabel =  unit_convert( this->distlabel,
                                     pal::PIXEL,
                                     layer->pal->map_unit,
                                     dpi, scale, delta_width );*/

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

    for ( i = 0, alpha = M_PI / 4; i < nbp; i++, alpha += beta )
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
  int FeaturePart::setPositionForLine( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width )
  {
#ifdef _DEBUG_
    std::cout << "SetPosition (line) : " << layer->name << "/" << uid << std::endl;
#endif
    int i;
    int dpi = f->layer->pal->dpi;
    double xrm, yrm;
    double distlabel = f->distlabel;

    xrm = unit_convert( f->label_x,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        dpi, scale, delta_width );

    yrm = unit_convert( f->label_y,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        dpi, scale, delta_width );


    /*double distlabel = unit_convert( this->distlabel,
                                     pal::PIXEL,
                                     layer->pal->map_unit,
                                     dpi, scale, delta_width );*/


    double *d; // segments lengths distance bw pt[i] && pt[i+1]
    double *ad;  // absolute distance bw pt[0] and pt[i] along the line
    double ll; // line length
    double dist;
    double bx, by, ex, ey;
    int nbls;
    double alpha;
    double cost;

    unsigned long flags = f->layer->getArrangementFlags();
    if ( flags == 0 )
      flags = FLAG_ON_LINE; // default flag
    bool reversed = false;

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
    for ( i = 0; i < line->nbPoints - 1; i++ )
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
        cost = ( 1 - cost ) / 100; // < 0.0001, 0.01 > (but 0.005 is already pretty much)

      // penalize positions which are further from the line's midpoint
      double costCenter = vabs( ll / 2 - ( l + xrm / 2 ) ) / ll; // <0, 0.5>
      cost += costCenter / 1000;  // < 0, 0.0005 >

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
      if ( f->layer->arrangement == P_LINE )
      {
        //std::cout << alpha*180/M_PI << std::endl;
        if ( flags & FLAG_MAP_ORIENTATION )
          reversed = ( alpha >= M_PI / 2 || alpha < -M_PI / 2 );

        if (( !reversed && ( flags & FLAG_ABOVE_LINE ) ) || ( reversed && ( flags & FLAG_BELOW_LINE ) ) )
          positions->push_back( new LabelPosition( i, bx + cos( beta ) *distlabel , by + sin( beta ) *distlabel, xrm, yrm, alpha, cost, this ) ); // Line
        if (( !reversed && ( flags & FLAG_BELOW_LINE ) ) || ( reversed && ( flags & FLAG_ABOVE_LINE ) ) )
          positions->push_back( new LabelPosition( i, bx - cos( beta ) *( distlabel + yrm ) , by - sin( beta ) *( distlabel + yrm ), xrm, yrm, alpha, cost, this ) );   // Line
        if ( flags & FLAG_ON_LINE )
          positions->push_back( new LabelPosition( i, bx - yrm*cos( beta ) / 2, by - yrm*sin( beta ) / 2, xrm, yrm, alpha, cost, this ) ); // Line
      }
      else if ( f->layer->arrangement == P_HORIZ )
      {
        positions->push_back( new LabelPosition( i, bx - xrm / 2, by - yrm / 2, xrm, yrm, 0, cost, this ) ); // Line
        //positions->push_back( new LabelPosition(i, bx -yrm/2, by - yrm*sin(beta)/2, xrm, yrm, alpha, cost, this, line)); // Line
      }
      else
      {
        // an invalid arrangement?
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


  LabelPosition* FeaturePart::curvedPlacementAtOffset( PointSet* path_positions, double* path_distances, int orientation, int index, double distance )
  {
    // Check that the given distance is on the given index and find the correct index and distance if not
    while ( distance < 0 && index > 1 )
    {
      index--;
      distance += path_distances[index];
    }

    if ( index <= 1 && distance < 0 ) // We've gone off the start, fail out
    {
      std::cerr << "err1" << std::endl;
      return NULL;
    }

    // Same thing, checking if we go off the end
    while ( index < path_positions->nbPoints && distance > path_distances[index] )
    {
      distance -= path_distances[index];
      index += 1;
    }
    if ( index >= path_positions->nbPoints )
    {
      std::cerr << "err2" << std::endl;
      return NULL;
    }

    // Keep track of the initial index,distance incase we need to re-call get_placement_offset
    int initial_index = index;
    double initial_distance = distance;

    double string_height = f->labelInfo->label_height;
    double old_x = path_positions->x[index-1];
    double old_y = path_positions->y[index-1];

    double new_x = path_positions->x[index];
    double new_y = path_positions->y[index];

    double dx = new_x - old_x;
    double dy = new_y - old_y;

    double segment_length = path_distances[index];
    if ( segment_length == 0 )
    {
      // Not allowed to place across on 0 length segments or discontinuities
      std::cerr << "err3" << std::endl;
      return NULL;
    }

    LabelPosition* slp = NULL;
    LabelPosition* slp_tmp = NULL;
    // current_placement = placement_result()
    double angle = atan2( -dy, dx );

    bool orientation_forced = ( orientation != 0 ); // Whether the orientation was set by the caller
    if ( !orientation_forced )
      orientation = ( angle > 0.55 * M_PI || angle < -0.45 * M_PI ? -1 : 1 );

    int upside_down_char_count = 0; // Count of characters that are placed upside down.

    for ( int i = 0; i < f->labelInfo->char_num; i++ )
    {
      double last_character_angle = angle;

      // grab the next character according to the orientation
      LabelInfo::CharacterInfo& ci = ( orientation > 0 ? f->labelInfo->char_info[i] : f->labelInfo->char_info[f->labelInfo->char_num-i-1] );

      // Coordinates this character will start at
      if ( segment_length == 0 )
      {
        // Not allowed to place across on 0 length segments or discontinuities
        std::cerr << "err4" << std::endl;
        return NULL;
      }

      double start_x = old_x + dx * distance / segment_length;
      double start_y = old_y + dy * distance / segment_length;
      // Coordinates this character ends at, calculated below
      double end_x = 0;
      double end_y = 0;

      //std::cerr << "segment len " << segment_length << "   distance " << distance << std::endl;
      if ( segment_length - distance  >= ci.width )
      {
        // if the distance remaining in this segment is enough, we just go further along the segment
        distance += ci.width;
        end_x = old_x + dx * distance / segment_length;
        end_y = old_y + dy * distance / segment_length;
      }
      else
      {
        // If there isn't enough distance left on this segment
        // then we need to search until we find the line segment that ends further than ci.width away
        do
        {
          old_x = new_x;
          old_y = new_y;
          index++;
          if ( index >= path_positions->nbPoints ) // Bail out if we run off the end of the shape
          {
            //std::cerr << "err5" << std::endl;
            return NULL;
          }
          new_x = path_positions->x[index];
          new_y = path_positions->y[index];
          dx = new_x - old_x;
          dy = new_y - old_y;
          segment_length = path_distances[index];

          //std::cerr << "-> " << sqrt(pow(start_x - new_x,2) + pow(start_y - new_y,2)) << " vs " << ci.width << std::endl;

        }
        while ( sqrt( pow( start_x - new_x, 2 ) + pow( start_y - new_y, 2 ) ) < ci.width ); // Distance from start_ to new_

        // Calculate the position to place the end of the character on
        findLineCircleIntersection( start_x, start_y, ci.width, old_x, old_y, new_x, new_y, end_x, end_y );

        // Need to calculate distance on the new segment
        distance = sqrt( pow( old_x - end_x, 2 ) + pow( old_y - end_y, 2 ) );
      }

      // Calculate angle from the start of the character to the end based on start_/end_ position
      angle = atan2( start_y - end_y, end_x - start_x );
      //angle = atan2(end_y-start_y, end_x-start_x);

      // Test last_character_angle vs angle
      // since our rendering angle has changed then check against our
      // max allowable angle change.
      double angle_delta = last_character_angle - angle;
      // normalise between -180 and 180
      while ( angle_delta > M_PI ) angle_delta -= 2 * M_PI;
      while ( angle_delta < -M_PI ) angle_delta += 2 * M_PI;
      if ( f->labelInfo->max_char_angle_delta > 0 && fabs( angle_delta ) > f->labelInfo->max_char_angle_delta*( M_PI / 180 ) )
      {
        std::cerr << "err6" << std::endl;
        return NULL;
      }

      double render_angle = angle;

      double render_x = start_x;
      double render_y = start_y;

      // Center the text on the line
      //render_x -= ((string_height/2.0) - 1.0)*math.cos(render_angle+math.pi/2)
      //render_y += ((string_height/2.0) - 1.0)*math.sin(render_angle+math.pi/2)

      if ( orientation < 0 )
      {
        // rotate in place
        render_x += ci.width * cos( render_angle ); //- (string_height-2)*sin(render_angle);
        render_y -= ci.width * sin( render_angle ); //+ (string_height-2)*cos(render_angle);
        render_angle += M_PI;
      }

      //std::cerr << "adding part: " << render_x << "  " << render_y << std::endl;
      LabelPosition* tmp = new LabelPosition( 0, render_x /*- xBase*/, render_y /*- yBase*/, ci.width, string_height, -render_angle, 0.0001, this );
      tmp->setPartId( orientation > 0 ? i : f->labelInfo->char_num - i - 1 );
      if ( slp == NULL )
        slp = tmp;
      else
        slp_tmp->setNextPart( tmp );
      slp_tmp = tmp;

      //current_placement.add_node(ci.character,render_x, -render_y, render_angle);
      //current_placement.add_node(ci.character,render_x - current_placement.starting_x, render_y - current_placement.starting_y, render_angle)

      // Normalise to 0 <= angle < 2PI
      while ( render_angle >= 2*M_PI ) render_angle -= 2 * M_PI;
      while ( render_angle < 0 ) render_angle += 2 * M_PI;

      if ( render_angle > M_PI / 2 && render_angle < 1.5*M_PI )
        upside_down_char_count++;
    }
    // END FOR

    // If we placed too many characters upside down
    if ( upside_down_char_count >= f->labelInfo->char_num / 2.0 )
    {
      // if we auto-detected the orientation then retry with the opposite orientation
      if ( !orientation_forced )
      {
        orientation = -orientation;
        slp = curvedPlacementAtOffset( path_positions, path_distances, orientation, initial_index, initial_distance );
      }
      else
      {
        // Otherwise we have failed to find a placement
        //std::cerr << "err7" << std::endl;
        return NULL;
      }
    }

    return slp;
  }

  static LabelPosition* _createCurvedCandidate( LabelPosition* lp, double angle, double dist )
  {
    LabelPosition* newLp = new LabelPosition( *lp );
    newLp->offsetPosition( dist*cos( angle + M_PI / 2 ), dist*sin( angle + M_PI / 2 ) );
    return newLp;
  }

  int FeaturePart::setPositionForLineCurved( LabelPosition ***lPos, PointSet* mapShape )
  {
    // label info must be present
    if ( f->labelInfo == NULL || f->labelInfo->char_num == 0 )
      return 0;

    // distance calculation
    double* path_distances = new double[mapShape->nbPoints];
    double total_distance = 0;
    double old_x = -1.0, old_y = -1.0;
    for ( int i = 0; i < mapShape->nbPoints; i++ )
    {
      if ( i == 0 )
        path_distances[i] = 0;
      else
        path_distances[i] = sqrt( pow( old_x - mapShape->x[i], 2 ) + pow( old_y - mapShape->y[i], 2 ) );
      old_x = mapShape->x[i];
      old_y = mapShape->y[i];

      total_distance += path_distances[i];
    }

    if ( total_distance == 0 )
      return 0;

    LinkedList<LabelPosition*> *positions = new LinkedList<LabelPosition*> ( ptrLPosCompare );
    double delta = max( f->labelInfo->label_height, total_distance / 10.0 );

    unsigned long flags = f->layer->getArrangementFlags();
    if ( flags == 0 )
      flags = FLAG_ON_LINE; // default flag

    // generate curved labels
    std::cerr << "------" << std::endl;
    for ( int i = 0; i*delta < total_distance; i++ )
    {
      LabelPosition* slp = curvedPlacementAtOffset( mapShape, path_distances, 0, 1, i * delta );

      if ( slp )
      {
        // evaluate cost
        double angle_diff = 0.0, angle_last = 0.0, diff;
        LabelPosition* tmp = slp;
        double sin_avg = 0, cos_avg = 0;
        while ( tmp )
        {
          if ( tmp != slp ) // not first?
          {
            diff = fabs( tmp->getAlpha() - angle_last );
            if ( diff > 2*M_PI ) diff -= 2 * M_PI;
            diff = min( diff, 2 * M_PI - diff ); // difference 350 deg is actually just 10 deg...
            angle_diff += diff;
          }

          sin_avg += sin( tmp->getAlpha() );
          cos_avg += cos( tmp->getAlpha() );
          angle_last = tmp->getAlpha();
          tmp = tmp->getNextPart();
        }

        double angle_diff_avg = angle_diff / ( f->labelInfo->char_num - 1 ); // <0, pi> but pi/8 is much already
        double cost = angle_diff_avg / 100; // <0, 0.031 > but usually <0, 0.003 >
        if ( cost < 0.0001 ) cost = 0.0001;

        // penalize positions which are further from the line's midpoint
        double labelCenter = ( i * delta ) + f->label_x / 2;
        double costCenter = vabs( total_distance / 2 - labelCenter ) / total_distance; // <0, 0.5>
        cost += costCenter / 1000;  // < 0, 0.0005 >
        //std::cerr << "cost " << angle_diff << " vs " << costCenter << std::endl;
        slp->setCost( cost );


        // average angle is calculated with respect to periodicity of angles
        double angle_avg = atan2( sin_avg / f->labelInfo->char_num, cos_avg / f->labelInfo->char_num );
        // displacement
        if ( flags & FLAG_ABOVE_LINE )
          positions->push_back( _createCurvedCandidate( slp, angle_avg, f->distlabel ) );
        if ( flags & FLAG_ON_LINE )
          positions->push_back( _createCurvedCandidate( slp, angle_avg, -f->labelInfo->label_height / 2 ) );
        if ( flags & FLAG_BELOW_LINE )
          positions->push_back( _createCurvedCandidate( slp, angle_avg, -f->labelInfo->label_height - f->distlabel ) );

        // delete original candidate
        delete slp;
      }
    }


    int nbp = positions->size();
    ( *lPos ) = new LabelPosition*[nbp];
    for ( int i = 0; i < nbp; i++ )
    {
      ( *lPos )[i] = positions->pop_front();
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

  int FeaturePart::setPositionForPolygon( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (polygon) : " << layer->name << "/" << uid << std::endl;
#endif

    int i;
    int j;

    double xrm;
    double yrm;

    xrm = unit_convert( f->label_x,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        f->layer->pal->dpi, scale, delta_width );

    yrm = unit_convert( f->label_y,
                        f->layer->label_unit,
                        f->layer->pal->map_unit,
                        f->layer->pal->dpi, scale, delta_width );

    //print();

    //LinkedList<PointSet*> *shapes_toCut;
    LinkedList<PointSet*> *shapes_toProcess;
    LinkedList<PointSet*> *shapes_final;

    //shapes_toCut     = new LinkedList<PointSet*>(ptrPSetCompare);
    shapes_toProcess = new LinkedList<PointSet*> ( ptrPSetCompare );
    shapes_final     = new LinkedList<PointSet*> ( ptrPSetCompare );

    mapShape->parent = NULL;

    shapes_toProcess->push_back( mapShape );

    splitPolygons( shapes_toProcess, shapes_final, xrm, yrm, f->uid );


    delete shapes_toProcess;

    int nbp;

    if ( shapes_final->size() > 0 )
    {
      LinkedList<LabelPosition*> *positions = new LinkedList<LabelPosition*> ( ptrLPosCompare );
      int it;

      int id = 0; // ids for candidates
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
        for ( bbid = 0; bbid < j; bbid++ )
        {
          CHullBox *box = boxes[bbid];

          if (( box->length * box->width ) > ( xmax - xmin ) *( ymax - ymin ) *5 )
          {
            std::cout << "Very Large BBOX (should never occurs : bug-report please)" << std::endl;
            std::cout << "   Box size:  " << box->length << "/" << box->width << std::endl;
            std::cout << "   Alpha:     " << alpha << "   " << alpha * 180 / M_PI << std::endl;
            std::cout << "   Dx;Dy:     " << dx << "   " << dy  << std::endl;
            std::cout << "   LabelSizerm: " << xrm << "   " << yrm  << std::endl;
            std::cout << "   LabelSizeUn: " << f->label_x << "   " << f->label_y << std::endl;
            continue;
          }

#ifdef _DEBUG_FULL_
          std::cout << "New BBox : " << bbid << std::endl;
          for ( i = 0; i < 4; i++ )
          {
            std::cout << box->x[i] << "\t" << box->y[i] << std::endl;
          }
#endif

          bool enoughPlace = false;
          if ( f->layer->getArrangement() == P_FREE )
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

          if ( f->layer->getArrangement() == P_HORIZ || enoughPlace )
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

          for ( px = px0; px <= box->width; px += dx )
          {
            for ( py = py0; py <= box->length; py += dy )
            {

              rx = cos( box->alpha ) * px + cos( box->alpha - M_PI / 2 ) * py;
              ry = sin( box->alpha ) * px + sin( box->alpha - M_PI / 2 ) * py;

              rx += box->x[0];
              ry += box->y[0];

              // Only accept candidate that center is in the polygon
              if ( isPointInPolygon( mapShape->nbPoints, mapShape->x, mapShape->y, rx,  ry ) )
              {
                // cost is set to minimal value, evaluated later
                positions->push_back( new LabelPosition( id++, rx - dlx, ry - dly , xrm, yrm, alpha, 0.0001, this ) ); // Polygon
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
      for ( i = 0; i < nbp; i++ )
      {
        ( *lPos )[i] = positions->pop_front();
      }

      for ( bbid = 0; bbid < j; bbid++ )
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

  void FeaturePart::print()
  {
    int i, j;
    std::cout << "Geometry id : " << f->uid << std::endl;
    std::cout << "Type: " << type << std::endl;
    if ( x && y )
    {
      for ( i = 0; i < nbPoints; i++ )
        std::cout << x[i] << ", " << y[i] << std::endl;
      std::cout << "Obstacle: " << nbHoles << std::endl;
      for ( i = 0; i < nbHoles; i++ )
      {
        std::cout << "  obs " << i << std::endl;
        for ( j = 0; j < holes[i]->nbPoints; j++ )
        {
          std::cout << holes[i]->x[j] << ";" << holes[i]->y[j] << std::endl;
        }
      }
    }

    std::cout << std::endl;
  }

  int FeaturePart::setPosition( double scale, LabelPosition ***lPos,
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
        if ( f->layer->getArrangement() == P_POINT_OVER )
          nbp = setPositionOverPoint( x[0], y[0], scale, lPos, delta );
        else
          nbp = setPositionForPoint( x[0], y[0], scale, lPos, delta );
        break;
      case GEOS_LINESTRING:
        if ( f->layer->getArrangement() == P_CURVED )
          nbp = setPositionForLineCurved( lPos, mapShape );
        else
          nbp = setPositionForLine( scale, lPos, mapShape, delta );
        break;

      case GEOS_POLYGON:
        switch ( f->layer->getArrangement() )
        {
          case P_POINT:
          case P_POINT_OVER:
            double cx, cy;
            mapShape->getCentroid( cx, cy );
            if ( f->layer->getArrangement() == P_POINT_OVER )
              nbp = setPositionOverPoint( cx, cy, scale, lPos, delta );
            else
              nbp = setPositionForPoint( cx, cy, scale, lPos, delta );
            break;
          case P_LINE:
            nbp = setPositionForLine( scale, lPos, mapShape, delta );
            break;
          default:
            nbp = setPositionForPolygon( scale, lPos, mapShape, delta );
            break;
        }
    }

    int rnbp = nbp;

    // purge candidates that are outside the bbox
    for ( i = 0; i < nbp; i++ )
    {
      if ( !( *lPos )[i]->isIn( bbox ) )
      {
        rnbp--;
        ( *lPos )[i]->setCost( DBL_MAX ); // infinite cost => do not use
      }
      else   // this one is OK
      {
        ( *lPos )[i]->insertIntoIndex( candidates );
      }
    }

    sort(( void** )( *lPos ), nbp, LabelPosition::costGrow );

    for ( i = rnbp; i < nbp; i++ )
    {
      delete( *lPos )[i];
    }

    return rnbp;
  }

  void FeaturePart::addSizePenalty( int nbp, LabelPosition** lPos, double bbx[4], double bby[4] )
  {
    int geomType = GEOSGeomTypeId( the_geom );

    double sizeCost = 0;
    if ( geomType == GEOS_LINESTRING )
    {
      double length;
      if ( GEOSLength( the_geom, &length ) != 1 )
        return; // failed to calculate length
      double bbox_length = max( bbx[2] - bbx[0], bby[2] - bby[0] );
      if ( length >= bbox_length / 4 )
        return; // the line is longer than quarter of height or width - don't penalize it

      sizeCost = 1 - ( length / ( bbox_length / 4 ) ); // < 0,1 >
    }
    else if ( geomType == GEOS_POLYGON )
    {
      double area;
      if ( GEOSArea( the_geom, &area ) != 1 )
        return;
      double bbox_area = ( bbx[2] - bbx[0] ) * ( bby[2] - bby[0] );
      if ( area >= bbox_area / 16 )
        return; // covers more than 1/16 of our view - don't penalize it

      sizeCost = 1 - ( area / ( bbox_area / 16 ) ); // < 0, 1 >
    }
    else
      return; // no size penalty for points

    //std::cout << "size cost " << sizeCost << std::endl;

    // apply the penalty
    for ( int i = 0; i < nbp; i++ )
    {
      lPos[i]->setCost( lPos[i]->getCost() + sizeCost / 100 );
    }
  }

  bool FeaturePart::isConnected( FeaturePart* p2 )
  {
    return ( GEOSTouches( the_geom, p2->the_geom ) == 1 );
  }

  bool FeaturePart::mergeWithFeaturePart( FeaturePart* other )
  {
    GEOSGeometry* g1 = GEOSGeom_clone( the_geom );
    GEOSGeometry* g2 = GEOSGeom_clone( other->the_geom );
    GEOSGeometry* geoms[2] = { g1, g2 };
    GEOSGeometry* g = GEOSGeom_createCollection( GEOS_MULTILINESTRING, geoms, 2 );
    GEOSGeometry* gTmp = GEOSLineMerge( g );
    GEOSGeom_destroy( g );

    if ( GEOSGeomTypeId( gTmp ) != GEOS_LINESTRING )
    {
      // sometimes it's not possible to merge lines (e.g. they don't touch at endpoints)
      GEOSGeom_destroy( gTmp );
      return false;
    }

    if ( ownsGeom ) // delete old geometry if we own it
      GEOSGeom_destroy( the_geom );
    // set up new geometry
    the_geom = gTmp;
    ownsGeom = true;

    deleteCoords();
    extractCoords( the_geom );
    return true;
  }

} // end namespace pal
