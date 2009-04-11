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

#ifndef _POINT_SET_H_
#define _POINT_SET_H_

#if defined(_VERBOSE_) || (_DEBUG_) || (_DEBUG_FULL_)
#include <iostream>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pointset.h"
#include "util.h"

#include <pal/pal.h>


#include "geomfunction.h"

namespace pal
{


  PointSet::PointSet()
  {
    nbPoints = cHullSize =  0;
    x = NULL;
    y = NULL;
    status = NULL;
    cHull = NULL;
    type = -1;
  }

  PointSet::PointSet( int nbPoints, double *x, double *y )
  {
    this->nbPoints = nbPoints;
    this->x = new double[nbPoints];
    this->y = new double[nbPoints];
    int i;

    for ( i = 0;i < nbPoints;i++ )
    {
      this->x[i] = x[i];
      this->y[i] = y[i];
    }
    type = GEOS_POLYGON;
    status = NULL;
    cHull = NULL;
  }

  PointSet::PointSet( double x, double y )
  {
    nbPoints = cHullSize =  1;
    this->x = new double[1];
    this->y = new double[1];
    this->x[0] = x;
    this->y[0] = y;

    status = NULL;
    cHull = NULL;
    parent = NULL;
    holeOf = NULL;

    type = GEOS_POINT;
  }

  PointSet::PointSet( PointSet &ps )
  {
    int i;

    nbPoints = ps.nbPoints;
    x = new double[nbPoints];
    y = new double[nbPoints];

    if ( ps.status )
      status = new int[nbPoints];
    else
      status = NULL;


    for ( i = 0;i < nbPoints;i++ )
    {
      x[i] = ps.x[i];
      y[i] = ps.y[i];
      if ( status )
        status[i] = ps.status[i];
    }

    if ( ps.cHull )
    {
      cHullSize = ps.cHullSize;
      for ( i = 0;i < cHullSize;i++ )
      {
        cHull[i] = ps.cHull[i];
      }
    }
    else
    {
      cHull = NULL;
      cHullSize = 0;
    }

    type = ps.type;

    holeOf = ps.holeOf;
  }

  PointSet::~PointSet()
  {
    if ( x )
      delete[] x;
    if ( y )
      delete[] y;

    if ( status )
      delete[] status;
    if ( cHull )
      delete[] cHull;
  }


  int PointSet::getPath( int start, int stop, int *path_val )
  {
    int nbPt = 0;
    int i;
#ifdef _DEBUG_FULL_
    std::cout << "start: " << start << std::endl;
    std::cout << "stop:  " << stop << std::endl;
    std::cout << "nbPoints:  " << nbPoints << std::endl;
#endif

    for ( i = start; i != stop; i = ( i + 1 ) % nbPoints, nbPt++ )
    {
      if ( status[i] < 10 && status[i] != *path_val )
        *path_val = 0;
    }
#ifdef _DEBUG_FULL_
    std::cout << "new nbPoints:  " << nbPt << std::endl;
#endif
    return nbPt;
  }



  PointSet * PointSet::extractPath( int path, int nbPtPath, int nbBboxPt, double bbx[4], double bby[4], Crossing *start, Crossing *stop, int startPt )
  {

    if ( path == -1 || path == 0 )
    {
      int k, j, l;
      PointSet *newShape = new PointSet();
      newShape->nbPoints = nbPtPath + nbBboxPt + 2;
      newShape->x = new double[newShape->nbPoints];
      newShape->y = new double[newShape->nbPoints];
      newShape->status = new int[newShape->nbPoints];
      newShape->type = GEOS_POLYGON;
      k = 0;

#ifdef _DEBUG_FULL_
      std::cout << "Keep # ot: " << newShape->nbPoints << " :: "  << path << std::endl;
      std::cout << "path point:" << nbPtPath << std::endl;
#endif
      // start => b
      // stop => a

      newShape->xmin = DBL_MAX;
      newShape->xmax = -DBL_MAX;
      newShape->ymin = DBL_MAX;
      newShape->ymax = -DBL_MAX;


      for ( k = 0, j = startPt; k < nbPtPath; j = ( j + 1 ) % nbPoints, k++ )
      {
        newShape->x[k] = x[j];
        newShape->y[k] = y[j];
        newShape->status[k] = status[j];
#ifdef _DEBUG_FULL_
        std::cout << "    " << x[j] << ";" << y[j] << std::endl;
#endif
        newShape->xmin = ( newShape->xmin < x[j] ? newShape->xmin : x[j] );
        newShape->xmax = ( newShape->xmax > x[j] ? newShape->xmax : x[j] );
        newShape->ymin = ( newShape->ymin < y[j] ? newShape->ymin : y[j] );
        newShape->ymax = ( newShape->ymax > y[j] ? newShape->ymax : y[j] );
      }

      /*
            if (start->pt != stop->pt){
               newShape->x[k] = x[j];
               newShape->y[k] = y[j];
               newShape->status[k] = status[j];
      #ifdef _DEBUG_
               std::cout << "    " << x[j] << ";" << y[j] << std::endl;
      #endif
               k++;
            }
      */

#ifdef _DEBUG_FULL_
      std::cout << "pta:" << std::endl;
#endif
      newShape->x[k] = stop->x;
      newShape->y[k] = stop->y;

      newShape->xmin = ( newShape->xmin < stop->x ? newShape->xmin : stop->x );
      newShape->xmax = ( newShape->xmax > stop->x ? newShape->xmax : stop->x );
      newShape->ymin = ( newShape->ymin < stop->y ? newShape->ymin : stop->y );
      newShape->ymax = ( newShape->ymax > stop->y ? newShape->ymax : stop->y );

      if ( stop->way == -1 )
        newShape->status[k] = stop->nextCorner + 10;
      else
        newShape->status[k] = (( stop->nextCorner + 3 ) % 4 ) + 10;
      k++;

#ifdef _DEBUG_FULL_
      std::cout << "    " << stop->x << ";" << stop->y << std::endl;
      std::cout << "corner:" << std::endl;
#endif
      for ( j = 0, l = stop->nextCorner;j < nbBboxPt;j++, l = ( l + stop->way + 4 ) % 4, k++ )
      {
        newShape->x[k] = bbx[l];
        newShape->y[k] = bby[l];
        newShape->status[k] = 15;
#ifdef _DEBUG_FULL_
        std::cout << "    " << bbx[l] << ";" << bby[l] << std::endl;
#endif
        newShape->xmin = ( newShape->xmin < bbx[l] ? newShape->xmin : bbx[l] );
        newShape->xmax = ( newShape->xmax > bbx[l] ? newShape->xmax : bbx[l] );
        newShape->ymin = ( newShape->ymin < bby[l] ? newShape->ymin : bby[l] );
        newShape->ymax = ( newShape->ymax > bby[l] ? newShape->ymax : bby[l] );
      }

      newShape->x[k] = start->x;
      newShape->y[k] = start->y;

      newShape->xmin = ( newShape->xmin < start->x ? newShape->xmin : start->x );
      newShape->xmax = ( newShape->xmax > start->x ? newShape->xmax : start->x );
      newShape->ymin = ( newShape->ymin < start->y ? newShape->ymin : start->y );
      newShape->ymax = ( newShape->ymax > start->y ? newShape->ymax : start->y );

      if ( start->way == -1 )
        newShape->status[k] = start->nextCorner + 10;
      else
        newShape->status[k] = (( start->nextCorner + 3 ) % 4 ) + 10;

#ifdef _DEBUG_FULL_
      std::cout << "ptb:" << std::endl;
      std::cout << "    " << start->x << ";" << start->y << std::endl;
      std::cout << "k: " << k << "    nbPoints: " << newShape->nbPoints << std::endl;
#endif
      return newShape;
    }
    else
      return NULL;
  }


  void PointSet::reduceLine( PointSet *line,
                             LinkedList<PointSet*> *shapes_final,
                             double bbx[4], double bby[4] )
  {
#ifdef _DEBUG_
    std::cout << "reduceLine" << std::endl;
#endif

    int i, j;
    int ba, bb;
    Crossing *crossing;
    double crossingX, crossingY;
    LinkedList<Crossing*> *crossings = new LinkedList<Crossing*> ( ptrCrossingCompare );

    int nbPoints = line->nbPoints;
    double *x = line->x;
    double *y = line->y;
    int *status = line->status;

#ifdef _DEBUG_FULL_
    std::cout << "Coord:" << std::endl;
    for ( i = 0;i < nbPoints;i++ )
    {
      std::cout << x[i] << ";" << y[i] << std::endl;
    }
#endif

    for ( i = 0, j = i + 1;i < nbPoints - 1;i++, j++ )
    {
      for ( ba = 0;ba < 4;ba++ ) // for each bbox segement
      {
        bb = ( ba + 1 ) % 4;

#ifdef _DEBUG_FULL_
        std::cout << "Test: " << i << "->" << j << " and " << ba << "-->" << bb  << std::endl;
#endif
        if ( computeSegIntersection( x[i], y[i], x[j], y[j],
                                     bbx[ba], bby[ba], bbx[bb], bby[bb],
                                     &crossingX, &crossingY ) ) // seg[i->j] crossing bbox[ba->bb]
        {

          crossing = new Crossing();
          crossing->pt = i;
          crossing->x = crossingX; // cross coordinate
          crossing->y = crossingY;
#ifdef _DEBUG_FULL_
          std::cout << "Crossing: " << std::endl;
          std::cout << "  cross point: " << crossingX << ";" << crossingY << std::endl;
          std::cout << "           pt: " << i  << "   (" << status[i] << ")" << std::endl;
#endif
          crossings->push_back( crossing );
        }

      }
    }


    if ( crossings->size() > 0 )
    {
      int start, stop;
      double startX, startY;
      double stopX, stopY;

      bool seg_complete = false;

      Crossing *crossing;
      startX = x[0];
      startY = y[0];
      start = 1;
      while ( crossings->size() > 0 )
      {
        crossing = crossings->pop_front();

        // TODO
        if ( status[crossing->pt] == -1 ) //  inside
        {
          stop = crossing->pt;
          stopX = crossing->x;
          stopY = crossing->y;
          PointSet *new_line = new PointSet();
          new_line->nbPoints = stop - start + 3;
          new_line->type = GEOS_LINESTRING;
          new_line->x = new double [new_line->nbPoints];
          new_line->y = new double [new_line->nbPoints];

          new_line->x[0] = startX;
          new_line->y[0] = startY;

          new_line->xmin = new_line->x[0];
          new_line->xmax = new_line->x[0];

          new_line->ymin = new_line->y[0];
          new_line->ymax = new_line->y[0];

          for ( j = start, i = 1;j <= stop;j++, i++ )
          {
            new_line->x[i] = x[j];
            new_line->y[i] = y[j];

            new_line->xmin = ( new_line->xmin < x[j] ? new_line->xmin : x[j] );
            new_line->xmax = ( new_line->xmax > x[j] ? new_line->xmax : x[j] );
            new_line->ymin = ( new_line->ymin < y[j] ? new_line->ymin : y[j] );
            new_line->ymax = ( new_line->ymax > y[j] ? new_line->ymax : y[j] );
          }
          new_line->x[new_line->nbPoints-1] = stopX;
          new_line->y[new_line->nbPoints-1] = stopY;

          new_line->xmin = ( new_line->xmin < stopX ? new_line->xmin : stopX );
          new_line->xmax = ( new_line->xmax > stopX ? new_line->xmax : stopX );
          new_line->ymin = ( new_line->ymin < stopY ? new_line->ymin : stopY );
          new_line->ymax = ( new_line->ymax > stopY ? new_line->ymax : stopY );

          shapes_final->push_back( new_line );
          seg_complete = true;
        }
        else
        {
          start = crossing->pt + 1;
          startX = crossing->x;
          startY = crossing->y;
          seg_complete = false;
        }
        delete crossing;
      }

      if ( !seg_complete )
      {
        PointSet * new_line = new PointSet();
        new_line->type = GEOS_LINESTRING;
        new_line->nbPoints = nbPoints - start + 1;
        new_line->x = new double[new_line->nbPoints];
        new_line->y = new double[new_line->nbPoints];
        new_line->x[0] = startX;
        new_line->y[0] = startY;

        new_line->xmin = startX;
        new_line->xmax = startX;

        new_line->ymin = startY;
        new_line->ymax = startY;

        for ( j = start, i = 1;j < nbPoints;j++, i++ )
        {
          new_line->x[i] = x[j];
          new_line->y[i] = y[j];

          new_line->xmin = ( new_line->xmin < x[j] ? new_line->xmin : x[j] );
          new_line->xmax = ( new_line->xmax > x[j] ? new_line->xmax : x[j] );
          new_line->ymin = ( new_line->ymin < y[j] ? new_line->ymin : y[j] );
          new_line->ymax = ( new_line->ymax > y[j] ? new_line->ymax : y[j] );
        }
        shapes_final->push_back( new_line );
      }

      delete line;
    }
    else
    {
      // line is out
#ifdef _DEBUG_FULL_
      std::cout << "Line is completely outside" << std::endl;
#endif
      delete line;
    }

    delete crossings;
  }

  /**
   * \brief takes shapes from shapes_toProcess, compute intersection with bbox
   * and puts new shapes into shapes_final
   *
   */
  void PointSet::reducePolygon( PointSet *shape_toProcess, LinkedList<PointSet*> *shapes_final, double bbx[4], double bby[4] )
  {
//#define _DEBUG_
//#define _DEBUG_FULL_

#ifdef _DEBUG_
    std::cout << "reducePolygon" << std::endl;
#endif

    int i, j;

    int nbCrossingIter = 0;


    LinkedList<PointSet*> *shapes_toProcess = new LinkedList<PointSet*> ( ptrPSetCompare );
    LinkedList<Crossing*> *crossings = new LinkedList<Crossing*> ( ptrCrossingCompare );

    shapes_toProcess->push_back( shape_toProcess );

    // only keep inside bbox part of feature
    PointSet *shape;
    while ( shapes_toProcess->size() > 0 ) // foreach feature in toProcess
    {
      nbCrossingIter++;
      double crossingX, crossingY;

      int ba, bb;
      int k;
      Crossing *crossing;

      shape = shapes_toProcess->pop_front();

#ifdef _DEBUG_FULL_
      std::cout << "ShapeToReduce:" << std::endl;
#endif

      // search point where polygon and bbox cross
      for ( i = 0;i < shape->nbPoints;i++ ) // foreach polygon segment
      {
#ifdef _DEBUG_FULL_
        std::cout << shape->x[i] << ";" << shape->y[i] << std::endl;
#endif
        j = ( i + 1 ) % shape->nbPoints;
        for ( ba = 0;ba < 4;ba++ ) // for each bbox segement
        {
          bb = ( ba + 1 ) % 4;

          if ( computeSegIntersection( shape->x[i], shape->y[i], shape->x[j], shape->y[j],
                                       bbx[ba], bby[ba], bbx[bb], bby[bb],
                                       &crossingX, &crossingY ) ) // seg[i->j] crossing bbox[ba->bb]
          {

            if ( vabs( cross_product( bbx[ba], bby[ba], bbx[bb], bby[bb], shape->x[i], shape->y[i] ) ) > 0.00001 )
            {
              // perhaps shape had  previuosly been splitted
              //   => new points generated by this operation are on the bbox border
              //   => avoid to consider this cross as valid ('status' for these new points are == bboxSegNumber + 10 and 15 for bbox corner
              if ( shape->status[i] != ba + 10 && shape->status[j] != ba + 10
                   && shape->status[i] != 15 && shape->status[j] != 15 )
              {

                crossing = new Crossing();
                crossing->pt = i;
                crossing->x = crossingX; // cross coordinate
                crossing->y = crossingY;
                crossing->seg = ba; // wich bbox edge ? 0=south, 1 = east, 2 = west, 3=west

                // cross->nextCorner denote on which bbox corner we fall first when following bbox inside the polygon
                if ( cross_product( shape->x[crossing->pt], shape->y[crossing->pt],
                                    shape->x[( crossing->pt+1 ) %shape->nbPoints],
                                    shape->y[( crossing->pt+1 ) %shape->nbPoints],
                                    bbx[ba], bby[ba] ) > 0 )
                {
                  crossing->nextCorner = ba; // next corner is ba
                  crossing->way = -1; // have to go counter-clockwise
                }
                else
                {
                  crossing->nextCorner = bb; // next corner is bb
                  crossing->way = 1; // go clockwise
                }
                crossings->push_back( crossing );
                //break;
              }
            }
          }
        }
      }


      i = 0;
      int crossingTabSize = crossings->size();
      Crossing ** crossingTab = new Crossing*[crossingTabSize];

      // put crosses in crossTab
      while ( crossings->size() > 0 )
      {
        crossing = crossings->pop_front();
        crossingTab[i] = crossing;
        i++;
      }

      if ( crossingTabSize > 1 )
      {
        double initialDist;
        double nextX;
        double nextY;
        double startX;
        double startY;

        initialDist = 0;

        // sort cross points, order counterclockwise by following bbox from down-left corner
        for ( k = 0;k < 4;k++ )
        {
          startX = bbx[k];
          startY = bby[k];
          nextX = bbx[( k+1 ) %4];
          nextY = bby[( k+1 ) %4];

          for ( j = 0;j < crossingTabSize;j++ )
          {
            // if the crossing point is colinear with bbox seg
            if ( crossingTab[j]->seg == k )
            {
              crossingTab[j]->d = initialDist + dist_euc2d_sq( startX, startY, crossingTab[j]->x, crossingTab[j]->y );
            }
          }
          initialDist += dist_euc2d_sq( startX, startY, nextX, nextY );
        }
        sort(( void** ) crossingTab, crossingTabSize, crossingDist );

        // crossing are sorted

#ifdef _DEBUG_FULL_
        for ( i = 0;i < crossingTabSize;i++ )
        {
          std::cout << "Crossing # " << i << ":" << std::endl;
          std::cout << "   pt: " << crossingTab[i]->pt << std::endl;
          std::cout << "   d: " << crossingTab[i]->d << std::endl;
          std::cout << "   (x,y): (" << crossingTab[i]->x << "," << crossingTab[i]->y << ")" << std::endl;
          std::cout << "   next corner: " << crossingTab[i]->nextCorner << std::endl;
          std::cout << "   way: " << crossingTab[i]->way << std::endl;
          std::cout << std::endl;
        }
#endif


        // select tros crosses
        Crossing *a = crossingTab[0];
        Crossing *b = crossingTab[( a->way + crossingTabSize ) %crossingTabSize];

        if ( vabs( a->x - b->x ) > 0.00001 || vabs( a->y - b->y ) > 0.00001 )
        {
#ifdef _DEBUG_FULL_
          std::cout << "crossing: " << a->x << ";" << a->y << std::endl;
          std::cout << "d: " << a->d << std::endl;
          std::cout << "a: 0           " << " b: " << ( a->way + crossingTabSize ) % crossingTabSize << std::endl;
#endif


          int cornerA = a->nextCorner;
          j = 1;
          while ( cornerA != b->nextCorner )
          {
            j++;
            cornerA = ( cornerA + a->way + 4 ) % 4;
          }

          if ( j == 4 )
          {
            if ( isSegIntersects( a->x, a->y,
                                  bbx[a->nextCorner], bby[a->nextCorner],
                                  shape->x[b->pt], shape->y[b->pt],
                                  shape->x[( b->pt + 1 ) %shape->nbPoints], shape->y[( b->pt + 1 ) %shape->nbPoints] ) )
            {
              j = 0;
            }
          }

          int nbBboxPoint = j;
          int nbPtPathA = 0;
          int nbPtPathB = 0;

#ifdef _DEBUG_FULL_
          std::cout << std::endl;
#endif

          int path_a;
          int path_b;

          if ( shape->status[a->pt] == -1 ) // ok
          {
            path_a = -1; // a is inside so...
            path_b = 1; // b is outside
          }
          else if ( shape->status[b->pt] == -1 ) // ok
          {
            path_a = 1;
            path_b = -1;
          }
          else if ( shape->status[a->pt] == 1 && shape->status[b->pt] == 1 )
          {
            path_a = -1;
            path_b = -1;
          }
          else if ( shape->status[a->pt] == 1 )
          {
            path_a = 1;
            path_b = -1;
          }
          else if ( shape->status[b->pt] == 1 )
          {
            path_a = -1;
            path_b = 1;
          }
          else
          {
            std::cout << "OULALA C EST UN PROBLEMEM CA!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            path_a = 0;
            path_b = 0;
          }

#ifdef _DEBUG_FULL_
          std::cout << "Path_a: " << path_a << std::endl;
          std::cout << "Path_b: " << path_b << std::endl;
#endif

          nbPtPathA = shape->getPath(( b->pt + 1 ) % shape->nbPoints,
                                     a->pt,
                                     &path_a );

          nbPtPathB = shape->getPath(( a->pt + 1 ) % shape->nbPoints,
                                     ( b->pt + 1 ) % shape->nbPoints,
                                     &path_b );

          if ( a->pt != b->pt )
          {
            nbPtPathA++;
          }

#ifdef _DEBUG_FULL_
          std::cout << "Path_a: " << path_a << std::endl;
          std::cout << "Path_b: " << path_b << std::endl;
#endif

          PointSet *newShape = NULL;
          Crossing *tmp;
          if (( a->pt == b->pt ) && ( a->way == 1 ) )
          {
            tmp = a;
            a = b;
            b = tmp;
          }
          // split shape into two new shape
	  newShape = shape->extractPath( path_a, nbPtPathA, nbBboxPoint, bbx, bby, b, a, ( b->pt + 1 ) % shape->nbPoints );
          if ( newShape )
          {
            if ( path_a == -1 ) // new shape inside => push into shapes_final
            {
              shapes_final->push_back( newShape );
            }
            else   // to drop or to resplit...
            {
              shapes_toProcess->push_back( newShape );
            }
          }

          newShape = shape->extractPath( path_b, nbPtPathB, nbBboxPoint, bbx, bby, a, b, ( a->pt + 1 ) % shape->nbPoints );
          if ( newShape )
          {
            if ( path_b == -1 )
            {
              shapes_final->push_back( newShape );
            }
            else
            {
              shapes_toProcess->push_back( newShape );
            }
          }

          delete shape;
        }
        else
        {
          if ( vabs( a->x - shape->x[a->pt] ) < 0.00001 && vabs( a->y - shape->y[a->pt] ) < 0.00001 )
          {
            shape->status[a->pt] = a->seg + 10;
          }
          if ( vabs( a->x - shape->x[( a->pt+1 ) %shape->nbPoints] ) < 0.00001 && vabs( a->y - shape->y[( a->pt+1 ) %shape->nbPoints] ) < 0.00001 )
          {
            shape->status[a->pt] = a->seg + 10;
          }
          if ( vabs( a->x - shape->x[b->pt] ) < 0.00001 && vabs( a->y - shape->y[b->pt] ) < 0.00001 )
          {
            shape->status[a->pt] = a->seg + 10;
          }
          if ( vabs( a->x - shape->x[( b->pt+1 ) %shape->nbPoints] ) < 0.00001 && vabs( a->y - shape->y[( b->pt+1 ) %shape->nbPoints] ) < 0.00001 )
          {
            shape->status[a->pt] = a->seg + 10;
          }

          shapes_toProcess->push_back( shape );
        }
      }
      else if ( nbCrossingIter == 1 || crossingTabSize == 1 )  // First shape dont cross the bbox => outside or bbox is inside shape
      {
        bool check = false, ok = false;

        if ( crossingTabSize == 1 )
        {
          std::cout << "WARN : one crossing !!!!! " << std::endl;
          bool in = true;
          bool out = true;
          for ( i = 0;i < shape->nbPoints;i++ )
          {
#ifdef _DEBUG_FULL_
            std::cout << shape->x[i] << ";" << shape->y[i] << ";" << shape->status[i] << std::endl;
#endif
            if ( shape->status[i] == -1 )
            {
              out = false;
            }
            else if ( shape->status[i] == 1 )
            {
              in = false;
            }
          }

          if ( in )
          {
            // shape is totally inside
            shapes_final->push_back( shape );
            ok = true;
          }
          else
            check = true;

        }
        else
          check = true;

        if ( check && isPointInPolygon( shape->nbPoints,
                                        shape->x, shape->y,
                                        ( bbx[0] + bbx[2] ) / 2,
                                        ( bby[0] + bby[2] ) / 2 ) )
        {
          // bbox is inside the shape => reduce shape to bbox
          delete[] shape->status;
          delete[] shape->x;
          delete[] shape->y;
          shape->status = new int[4];
          shape->x = new double[4];
          shape->y = new double[4];
          shape->nbPoints = 4;
          for ( i = 0;i < 4;i++ )
          {
            shape->status[i] = 0;
            shape->x[i] = bbx[i];
            shape->y[i] = bby[i];
          }
          shapes_final->push_back( shape ); // keep the bbox as shape
        }
        else if ( ok == false )
          delete shape;
      }
      else
      {
        int stat = -1;
        for ( i = 0;i < shape->nbPoints;i++ )
        {
          if ( shape->status[i] == 1 )
          {
            stat = 1;
            break;
          }
        }
        if ( stat > 0 )
          delete shape;
        else
          shapes_final->push_back( shape );
      }

      for ( i = 0;i < crossingTabSize;i++ )
        delete crossingTab[i];
      delete[] crossingTab;
    }

    delete shapes_toProcess;
    delete crossings;
  }
//#undef _DEBUG_
//#undef _DEBUG_FULL_

  PointSet* PointSet::extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty )
  {

    int i, j;

    PointSet *newShape = new PointSet();

    newShape->type = GEOS_POLYGON;

    newShape->nbPoints = nbPtSh;

    newShape->x = new double[newShape->nbPoints];
    newShape->y = new double[newShape->nbPoints];
    newShape->status = NULL;
#ifdef _DEBUG_FULL_
    std::cout << "New shape: ";
#endif
    // new shape # 1 from imin to imax
    for ( j = 0, i = imin;i != ( imax + 1 ) % nbPoints;i = ( i + 1 ) % nbPoints, j++ )
    {
      newShape->x[j] = x[i];
      newShape->y[j] = y[i];
#ifdef _DEBUG_FULL_
      std::cout << x[i] << ";" << y[i] << std::endl;
#endif
    }
    // is the cutting point a new one ?
    if ( fps != fpe )
    {
      // yes => so add it
      newShape->x[j] = fptx;
      newShape->y[j] = fpty;
#ifdef _DEBUG_FULL_
      std::cout << fptx << ";" << fpty << std::endl;
#endif
    }

#ifdef _DEBUG_FULL_
    std::cout << "J = " << j << "/" << newShape->nbPoints << std::endl;
    std::cout << std::endl;
    std::cout << "This:    " << this << std::endl;
#endif

    return newShape;
  }


  void PointSet::splitPolygons( LinkedList<PointSet*> *shapes_toProcess,
                                LinkedList<PointSet*> *shapes_final,
                                double xrm, double yrm , char *uid )
  {
#ifdef _DEBUG_
    std::cout << "splitPolygons: " << uid << std::endl;
#endif
    int i, j;

    int nbp;
    double *x;
    double *y;

    int *pts;

    int *cHull;
    int cHullSize;

    double cp;
    double bestcp = 0;

    double bestArea = 0;
    double area;

    double base;
    double b, c;
    double s;

    int ihs;
    int ihn;

    int ips;
    int ipn;

    int holeS = -1;  // hole start and end points
    int holeE = -1;

    int retainedPt = -1;
    int pt = 0;

    double labelArea = xrm * yrm;

    PointSet *shape;

    while ( shapes_toProcess->size() > 0 )
    {
#ifdef _DEBUG_FULL_
      std::cout << "Shape popping()" << std::endl;
#endif
      shape = shapes_toProcess->pop_front();

      x = shape->x;
      y = shape->y;
      nbp = shape->nbPoints;
      pts = new int[nbp];

#ifdef _DEBUG_FULL_
      std::cout << "nbp: " << nbp << std::endl;
      std::cout << " PtSet: ";
#endif

      for ( i = 0;i < nbp;i++ )
      {
        pts[i] = i;
#ifdef _DEBUG_FULL_
        std::cout << x[i] << ";" << y[i] << std::endl;
#endif
      }

#ifdef _DEBUG_FULL_
      std::cout << std::endl;
#endif

      // conpute convex hull
      shape->cHullSize = convexHullId( pts, x, y, nbp, shape->cHull );

      cHull = shape->cHull;
      cHullSize = shape->cHullSize;


#ifdef _DEBUG_FULL_
      std::cout << " CHull: ";
      for ( i = 0;i < cHullSize;i++ )
      {
        std::cout << cHull[i] << " ";
      }
      std::cout << std::endl;
#endif

      bestArea = 0;
      retainedPt = -1;

      // lookup for a hole
      for ( ihs = 0;ihs < cHullSize;ihs++ )
      {
        // ihs->ihn => cHull'seg
        ihn = ( ihs + 1 ) % cHullSize;

        ips = cHull[ihs];
        ipn = ( ips + 1 ) % nbp;
        if ( ipn != cHull[ihn] ) // next point on shape is not the next point on cHull => there is a hole here !
        {
          bestcp = 0;
          pt = -1;
          // lookup for the deepest point in the hole
          for ( i = ips;i != cHull[ihn];i = ( i + 1 ) % nbp )
          {
            cp = vabs( cross_product( x[cHull[ihs]], y[cHull[ihs]],
                                      x[cHull[ihn]], y[cHull[ihn]],
                                      x[i], y[i] ) );
            if ( cp - bestcp > EPSILON )
            {
              bestcp = cp;
              pt = i;
            }
          }

#ifdef _DEBUG_FULL_
          std::cout << "Deeper POint: " << pt << " between " << ips << " and " << cHull[ihn]  << std::endl;
#endif
          if ( pt  != -1 )
          {
            // compute the ihs->ihn->pt triangle's area
            base = dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                               x[cHull[ihn]], y[cHull[ihn]] );

            b = dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                            x[pt], y[pt] );

            c = dist_euc2d( x[cHull[ihn]], y[cHull[ihn]],
                            x[pt], y[pt] );

            s = ( base + b + c ) / 2; // s = half perimeter
            area = s * ( s - base ) * ( s - b ) * ( s - c );
            if ( area < 0 )
              area = -area;

            // retain the biggest area
            if ( area - bestArea > EPSILON )
            {
              bestArea = area;
              retainedPt = pt;
              holeS = ihs;
              holeE = ihn;
            }
          }
        }
      }

      // we have a hole, its area, and the deppest point in hole
      // we're going to find the second point to cup the shape
      // holeS = hole starting point
      // holeE = hole ending point
      // retainedPt = deppest point in hole
      // bestArea = area of triangle HoleS->holeE->retainedPoint
      bestArea = sqrt( bestArea );
      double cx, cy, dx, dy, ex, ey, fx, fy, seg_length, ptx = 0, pty = 0, fptx = 0, fpty = 0;
      int ps = -1, pe = -1, fps = -1, fpe = -1;
      if ( retainedPt >= 0 && bestArea > labelArea ) // there is a hole so we'll cut the shape in two new shape (only if hole area is bigger than twice labelArea)
      {
#ifdef _DEBUG_FULL_
        std::cout << "Trou: " << retainedPt << std::endl;
        std::cout << "Hole: " << cHull[holeS] << " -> " << cHull[holeE] << std::endl;
        std::cout << "iterate from " << ( cHull[holeE] + 1 ) % nbp << "   to " << ( cHull[holeS] - 1 + nbp ) % nbp << std::endl;
#endif
        c = DBL_MAX;


        // iterate on all shape points except points which are in the hole
        double isValid;
        int k, l;
        for ( i = ( cHull[holeE] + 1 ) % nbp;i != ( cHull[holeS] - 1 + nbp ) % nbp;i = j )
        {
          j = ( i + 1 ) % nbp; // i->j is shape segment not in hole

          // compute distance between retainedPoint and segment
          // whether perpendicular distance (if retaindPoint is fronting segment i->j)
          // or distance between retainedPt and i or j (choose the nearest)
          seg_length = dist_euc2d( x[i], y[i], x[j], y[j] );
          cx = ( x[i] + x[j] ) / 2.0;
          cy = ( y[i] + y[j] ) / 2.0;
          dx = cy - y[i];
          dy = cx - x[i];

          ex = cx - dx;
          ey = cy + dy;
          fx = cx + dx;
          fy = cy - dy;
#ifdef _DEBUG_FULL_
          std::cout << "D: " << dx << " " << dy << std::endl;
          std::cout << "seg_length: " << seg_length << std::endl;
#endif
          if ( seg_length < EPSILON || vabs(( b = cross_product( ex, ey, fx, fy, x[retainedPt], y[retainedPt] ) / ( seg_length ) ) ) > ( seg_length / 2 ) )    // retainedPt is not fronting i->j
          {
            if (( ex = dist_euc2d_sq( x[i], y[i], x[retainedPt], y[retainedPt] ) ) < ( ey = dist_euc2d_sq( x[j], y[j], x[retainedPt], y[retainedPt] ) ) )
            {
              b = ex;
              ps = i;
              pe = i;
            }
            else
            {
              b = ey;
              ps = j;
              pe = j;
            }
          }
          else   // point fronting i->j => compute pependicular distance  => create a new point
          {
            b = cross_product( x[i], y[i], x[j], y[j],  x[retainedPt], y[retainedPt] ) / seg_length;
            b *= b;
            ps = i;
            pe = j;

            if ( !computeLineIntersection( x[i], y[i], x[j], y[j], x[retainedPt], y[retainedPt], x[retainedPt] - dx, y[retainedPt] + dy, &ptx, &pty ) )
            {
              std::cout << "Oups ... il devrait par tomber la..." << std::endl;
            }
#ifdef _DEBUG_FULL_
            std::cout << "intersection : " << x[i] << " " <<  y[i] << " " << x[j] << " " <<  y[j] << " " <<  x[retainedPt] << " " <<  y[retainedPt] << " " <<  x[retainedPt] - dx << " " <<  y[retainedPt] + dy << std::endl;
            std::cout << "   =>   " << ptx << ";" << pty << std::endl;
            std::cout << "   cxy>   " << cx << ";" << cy << std::endl;
            std::cout << "   dxy>   " << dx << ";" << dy << std::endl;
#endif
          }

          isValid = true;
          double pointX, pointY;
          if ( ps == pe )
          {
            pointX = x[pe];
            pointY = y[pe];
          }
          else
          {
            pointX = ptx;
            pointY = pty;
          }

          for ( k = cHull[holeS];k != cHull[holeE];k = ( k + 1 ) % nbp )
          {
            l = ( k + 1 ) % nbp;
            //std::cout << "test " << k << " " << l << std::endl;
            if ( isSegIntersects( x[retainedPt], y[retainedPt], pointX, pointY, x[k], y[k], x[l], y[l] ) )
            {
              isValid = false;
              //std::cout << "Invalid point" << pe << ps << std::endl;
              break;
            }
          }


          if ( isValid && b < c )
          {
            //std::cout << "new point: " << ps << " " << pe << std::endl;
            c = b;
            fps = ps;
            fpe = pe;
            fptx = ptx;
            fpty = pty;
          }
        }  // for point which are not in hole

#ifdef _DEBUG_FULL_
        std::cout << " cut from " << retainedPt << " to ";
        if ( fps == fpe )
          std::cout << "point " << fps << std::endl;
        else
        {
          std::cout << "new point (" << fptx << ";" << fpty << "     between " << fps << " and " << fpe << std::endl;
        }
#endif

        // we will cut the shapeu in two new shapes, one from [retainedPoint] to [newPoint] and one form [newPoint] to [retainedPoint]
        int imin = retainedPt;
        int imax = ((( fps < retainedPt && fpe < retainedPt ) || ( fps > retainedPt && fpe > retainedPt ) ) ? min( fps, fpe ) : max( fps, fpe ) );

        int nbPtSh1, nbPtSh2; // how many points in new shapes ?
        if ( imax > imin )
          nbPtSh1 = imax - imin + 1 + ( fpe != fps );
        else
          nbPtSh1 = imax + nbp - imin + 1 + ( fpe != fps );

        if (( imax == fps ? fpe : fps ) < imin )
          nbPtSh2 = imin - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );
        else
          nbPtSh2 = imin + nbp - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );


#ifdef _DEBUG_FULL_
        std::cout << "imin: " << imin << "    imax:" << imax << std::endl;
#endif
        if ( retainedPt == -1 || fps == -1 || fpe == -1 )
        {
#ifdef _DEBUG_
          std::cout << std::endl << "Failed to split feature !!! (uid=" << uid << ")" << std::endl;
#endif
          if ( shape->parent )
            delete shape;
        }
        // check for useless spliting
        else if ( imax == imin || nbPtSh1 <= 2 || nbPtSh2 <= 2 || nbPtSh1 == nbp  || nbPtSh2 == nbp )
        {
          shapes_final->push_back( shape );
        }
        else
        {

          PointSet *newShape = shape->extractShape( nbPtSh1, imin, imax, fps, fpe, fptx, fpty );

          if ( shape->parent )
            newShape->parent = shape->parent;
          else
            newShape->parent = shape;

#ifdef _DEBUG_FULL_
          int i = 0;
          std::cout << "push back:" <<  std::endl;
          for ( i = 0;i < newShape->nbPoints;i++ )
          {
            std::cout << newShape->x[i] << ";" << newShape->y[i] << std::endl;
          }
#endif

          shapes_toProcess->push_back( newShape );

          if ( imax == fps )
            imax = fpe;
          else
            imax = fps;

          newShape = shape->extractShape( nbPtSh2, imax, imin, fps, fpe, fptx, fpty );

          if ( shape->parent )
            newShape->parent = shape->parent;
          else
            newShape->parent = shape;

#ifdef _DEBUG_FULL_
          std::cout << "push back:" <<  std::endl;
          for ( i = 0;i < newShape->nbPoints;i++ )
          {
            std::cout << newShape->x[i] << ";" << newShape->y[i] << std::endl;
          }
#endif
          shapes_toProcess->push_back( newShape );

          if ( shape->parent )
            delete shape;
        }
      }
      else
      {
#ifdef _DEBUG_FULL_
        std::cout << "Put shape into shapes_final" << std::endl;
#endif
        shapes_final->push_back( shape );
      }
      delete[] pts;
    }
  }


  PointSet * PointSet::createProblemSpecificPointSet( double bbx[4], double bby[4], bool *outside, bool *inside )
  {
    int i;
#ifdef _DEBUG_FULL_
    std::cout << "CreateProblemSpecific:" << std::endl;
#endif
    PointSet *shape = new PointSet();
    shape->status = new int[nbPoints];
    shape->x = new double[nbPoints];
    shape->y = new double[nbPoints];
    shape->nbPoints = nbPoints;
    shape->type = type;

    shape->xmin = xmin;
    shape->xmax = xmax;
    shape->ymin = ymin;
    shape->ymax = ymax;

    *outside = true;
    *inside = true;

    // Build PointSet from geom => (x,y) coord, status[i] -> -1 inside, 1 outside , 0 on border

    double epsilon = 0.000001;
    for ( i = 0;i < nbPoints;i++ )
    {
      shape->x[i] = this->x[i];
      shape->y[i] = this->y[i];

      double cp0 = cross_product( bbx[0], bby[0], bbx[1], bby[1],
                                  shape->x[i], shape->y[i] );
      double cp1 = cross_product( bbx[1], bby[1], bbx[2], bby[2],
                                  shape->x[i], shape->y[i] );
      double cp2 = cross_product( bbx[2], bby[2], bbx[3], bby[3],
                                  shape->x[i], shape->y[i] );
      double cp3 = cross_product( bbx[3], bby[3], bbx[0], bby[0],
                                  shape->x[i], shape->y[i] );

      if ( cp0 > epsilon && cp1 > epsilon && cp2 > epsilon && cp3 > epsilon )
      {
        shape->status[i] = -1;  // point is inside the map extent
        *outside = false;
      }
      else if ( vabs( cp0 ) < epsilon && cp1 > epsilon && cp2 > epsilon && cp3 > epsilon )
      {
        shape->status[i] = 10;  // Point is on the bottom border
        *outside = false;
      }
      else if ( cp0 > epsilon && vabs( cp1 ) < epsilon && cp2 > epsilon && cp3 > epsilon )
      {
        shape->status[i] = 11; // point is on the right border
        *outside = false;
      }
      else if ( cp0 > epsilon && cp1 > epsilon && vabs( cp2 ) < epsilon && cp3 > epsilon )
      {
        shape->status[i] = 12; // point is on the top border
        *outside = false;
      }
      else if ( cp0 > epsilon && cp1 > epsilon && cp2 > epsilon && vabs( cp3 ) < epsilon )
      {
        shape->status[i] = 13; // point is on the left border
        *outside = false;
      }
      else if ( vabs( cp0 ) < epsilon && vabs( cp1 ) < epsilon && cp2 > epsilon && cp3 > epsilon )
      {
        shape->status[i] = 15; // point is on the bottom right corner
        *outside = false;
      }
      else if ( cp0 > epsilon && vabs( cp1 ) < epsilon && vabs( cp2 ) < epsilon && cp3 > epsilon )
      {
        shape->status[i] = 15; // point is on the top right corner
        *outside = false;
      }
      else if ( cp0 > epsilon && cp1 > epsilon && vabs( cp2 ) < epsilon && vabs( cp3 ) < epsilon )
      {
        shape->status[i] = 15; // point is on the top left corner
        *outside = false;
      }
      else if ( vabs( cp0 ) < epsilon && cp1 > epsilon && cp2 > epsilon && vabs( cp3 ) < epsilon )
      {
        shape->status[i] = 15; // point is on the bottom left corner
        *outside = false;
      }
      else
      {
        shape->status[i] = 1; // point is not in the map extent
        *inside = false;
      }
#ifdef _DEBUG_FULL_
      std::cout << shape->x[i] << ";" << shape->y[i] << ";" << shape->status[i] << ";" << cp0 << ";" << cp1 << ";" << cp2 << ";" << cp3 << std::endl;
#endif
    }

    shape->holeOf = NULL;
    shape->parent = NULL;

    return shape;
  }




  CHullBox * PointSet::compute_chull_bbox()
  {
    int i;
    int j;

    double bbox[4]; // xmin, ymin, xmax, ymax

    double alpha;
    int alpha_d;

    double alpha_seg;

    double dref;
    double d1, d2;

    double bb[16];   // {ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy, gx, gy, hx, hy}}

    double cp;
    double best_cp;
    double distNearestPoint;
    int nearestPoint;

    double area;
    double width;
    double length;

    double best_area = DBL_MAX;
    double best_alpha = -1;
    double best_bb[16];
    double best_length = 0;
    double best_width = 0;


    bbox[0] =   DBL_MAX;
    bbox[1] =   DBL_MAX;
    bbox[2] = - DBL_MAX;
    bbox[3] = - DBL_MAX;

#ifdef _DEBUG_
    std::cout << "Compute_chull_bbox" << std::endl;
#endif

    for ( i = 0;i < cHullSize;i++ )
    {
#ifdef _DEBUG_FULL_
      std::cout << x[cHull[i]] << ";" << y[cHull[i]] << std::endl;
#endif
      if ( x[cHull[i]] < bbox[0] )
        bbox[0] = x[cHull[i]];

      if ( x[cHull[i]] > bbox[2] )
        bbox[2] = x[cHull[i]];

      if ( y[cHull[i]] < bbox[1] )
        bbox[1] = y[cHull[i]];

      if ( y[cHull[i]] > bbox[3] )
        bbox[3] = y[cHull[i]];
    }


    dref = bbox[2] - bbox[0];

    for ( alpha_d = 0; alpha_d < 90;alpha_d++ )
    {
      alpha = alpha_d *  M_PI / 180.0;
      d1 = cos( alpha ) * dref;
      d2 = sin( alpha ) * dref;

      bb[0]  = bbox[0];
      bb[1]  = bbox[3]; // ax, ay

      bb[4]  = bbox[0];
      bb[5]  = bbox[1]; // cx, cy

      bb[8]  = bbox[2];
      bb[9]  = bbox[1]; // ex, ey

      bb[12] = bbox[2];
      bb[13] = bbox[3]; // gx, gy


      bb[2]  = bb[0] + d1;
      bb[3]  = bb[1] + d2; // bx, by
      bb[6]  = bb[4] - d2;
      bb[7]  = bb[5] + d1; // dx, dy
      bb[10] = bb[8] - d1;
      bb[11] = bb[9] - d2; // fx, fy
      bb[14] = bb[12] + d2;
      bb[15] = bb[13] - d1; // hx, hy

      // adjust all points
      for ( i = 0;i < 16;i += 4 )
      {

        alpha_seg = (( i / 4 > 0 ? ( i / 4 ) - 1 : 3 ) ) * M_PI / 2 + alpha;

        best_cp = DBL_MAX;
        nearestPoint = -1;
        for ( j = 0;j < nbPoints;j++ )
        {
          cp = cross_product( bb[i+2], bb[i+3], bb[i], bb[i+1], x[cHull[j]], y[cHull[j]] );
          if ( cp < best_cp )
          {
            best_cp = cp;
            nearestPoint = cHull[j];
          }
        }

        distNearestPoint = best_cp / dref;

        d1 = cos( alpha_seg ) * distNearestPoint;
        d2 = sin( alpha_seg ) * distNearestPoint;

        bb[i]   += d1; // x
        bb[i+1] += d2; // y
        bb[i+2] += d1; // x
        bb[i+3] += d2; // y
      }

      // compute and compare AREA
      width = cross_product( bb[6], bb[7], bb[4], bb[5], bb[12], bb[13] ) / dref;
      length = cross_product( bb[2], bb[3], bb[0], bb[1], bb[8], bb[9] ) / dref;

      area = width * length;

      if ( area < 0 )
        area *= -1;


      if ( best_area - area > EPSILON )
      {
        best_area = area;
        best_length = length;
        best_width = width;
        best_alpha = alpha;
        memcpy( best_bb, bb, sizeof( double ) *16 );
      }
    }

    // best bbox is defined

    CHullBox * finalBb = new CHullBox();

    for ( i = 0;i < 16;i = i + 4 )
    {
      computeLineIntersection( best_bb[i], best_bb[i+1], best_bb[i+2], best_bb[i+3],
                               best_bb[( i+4 ) %16], best_bb[( i+5 ) %16], best_bb[( i+6 ) %16], best_bb[( i+7 ) %16],
                               &finalBb->x[int ( i/4 )], &finalBb->y[int ( i/4 )] );
    }

    finalBb->alpha = best_alpha;
    finalBb->width = best_width;
    finalBb->length = best_length;

#ifdef _DEBUG_FULL_
    std::cout << "FINAL" << std::endl;
    std::cout << "Length : " << best_length << std::endl;
    std::cout << "Width : " << best_width << std::endl;
    std::cout << "Alpha: " << best_alpha << "    " << best_alpha*180 / M_PI << std::endl;
    for ( i = 0;i < 4;i++ )
    {
      std::cout << finalBb->x[0] << " " << finalBb->y[0] << " ";
    }
    std::cout << std::endl;
#endif

    return finalBb;
  }

#if 0
  double PointSet::getDistInside( double px, double py )
  {

    double dist[8];
    double rpx[8];
    double rpy[8];
    bool ok[8];

    if ( !isPointInPolygon( nbPoints, x, y, px, py ) )
    {
      double d = getDist( px, py );
      //std::cout << "Outside : " << d << std::endl;
      if ( d < 0 )
      {
        d = -( d * d * d * d );
      }
      else
      {
        d = d * d * d * d;
      }
      return d;
    }

    int i;

    double width = ( xmax - xmin ) * 2;
    double height = ( ymax - ymin ) * 2;

    /*
               1  0   7
                \ | /
              2 --x -- 6
                / | \
              3   4  5
    */

    // Compute references points
    for ( i = 0;i < 8;i++ )
    {
      dist[i] = DBL_MAX;
      ok[i] = false;
      rpx[i] = px;
      rpy[i] = py;
    }

    rpx[0] += 0;
    rpy[0] += height;

    rpx[1] -= width;
    rpy[1] += height;

    rpx[2] -= width;
    rpy[2] += 0;

    rpx[3] -= width;
    rpy[3] -= height;

    rpx[4] += 0;
    rpy[4] -= height;

    rpx[5] += width;
    rpy[5] -= height;

    rpx[6] += width;
    rpy[6] += 0;

    rpx[7] += width;
    rpy[7] += height;

    int j, k;
    for ( i = 0;i < nbPoints;i++ )
    {
      j = ( i + 1 ) % nbPoints;

      for ( k = 0;k < 8;k++ )
      {
        double ix, iy;
        if ( computeSegIntersection( px, py, rpx[k], rpy[k], x[i], y[i], x[j], y[j], &ix, &iy ) )
        {
          double d = dist_euc2d_sq( px, py, ix, iy );
          if ( dist[k] > d )
          {
            dist[k] = d;
            ok[k] = true;
            //std::cout << "new dist for " << k << ": " << dist[k] << std::endl;
          }
        }
      }
    }

    double a, b, c, d;


    for ( i = 0;i < 8;i++ )
    {
      if ( !ok[i] )
      {
        std::cout << "ERROR!!!!!!!!!!!!!!!!!" << std::endl;
        dist[i] = 0;
      }
      //std::cout << "dist[" << i << "]: " << dist[i] << std::endl;
    }

    a = min( dist[0], dist[4] );
    b = min( dist[1], dist[5] );
    c = min( dist[2], dist[6] );
    d = min( dist[3], dist[7] );
    /*
        std::cout << "a: " << a << std::endl;
        std::cout << "b: " << b << std::endl;
        std::cout << "c: " << c << std::endl;
        std::cout << "d: " << d << std::endl;
      */
    //a = (a+b+c+d)/4.0;

    //a = min(a,b);
    //c = min(c,d);
    //return min(a,c);


    return ( a*b*c*d );
  }
#endif

  double PointSet::getDist( double px, double py, double *rx, double *ry )
  {
    if ( nbPoints == 1 || type == GEOS_POINT )
    {
      if ( rx && ry )
      {
        *rx = x[0];
        *ry = y[0];
      }
      return dist_euc2d_sq( x[0], y[0], px, py );
    }

    int a, b;
    int nbP = ( type == GEOS_POLYGON ? nbPoints : nbPoints - 1 );

    double best_dist = DBL_MAX;
    double d;

    for ( a = 0;a < nbP;a++ )
    {
      b = ( a + 1 ) % nbPoints;

      double px2, py2;
      px2 = px - y[b] + y[a];
      py2 = py + x[b] - x[a];
      double ix, iy;

      // (px,py)->(px2,py2) is a line perpendicular to a->b
      // Check the line p->p2 cross the segment a->b
      if ( computeLineSegIntersection( px, py, px2, py2,
                                       x[a], y[a], x[b], y[b],
                                       &ix, &iy ) )
      {
        d = dist_euc2d_sq( px, py, ix, iy );
      }
      else
      {
        double d1 = dist_euc2d_sq( x[a], y[a], px, py );
        double d2 = dist_euc2d_sq( x[b], y[b], px, py );
        if ( d1 < d2 )
        {
          d = d1;
          ix = x[a];
          iy = y[a];
        }
        else
        {
          d = d2;
          ix = x[b];
          iy = y[b];
        }
      }

      if ( d < best_dist )
      {
        best_dist = d;
        if ( rx && ry )
        {
          *rx = ix;
          *ry = iy;
        }
      }
    } // end for (a in nbPoints)

    return best_dist;
  }



  void PointSet::getCentroid( double &px, double &py )
  {
    double ix, iy;
    double mesh = min(( xmax - xmin ) / 10, ( ymax - ymin ) / 10 );
    bool ptOk = false;

    while ( !ptOk )
    {
      double best = -DBL_MAX;
      double dist;
      for ( ix = xmin;ix < xmax;ix = ix + mesh )
      {
        for ( iy = ymin;iy < ymax;iy = iy + mesh )
        {
          dist = getDist( ix, iy, NULL, NULL );
          if ( dist > 0 && dist > best )
          {
            ptOk = true;
            best = dist;
            px = ix;
            py = iy;
          }
        }
      }
      mesh /= 10;
    }
  }

} // end namespace

#endif
