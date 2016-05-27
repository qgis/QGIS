/**********************************************************************
 *
 * PostGIS - Spatial Types for PostgreSQL
 * http://postgis.net
 *
 * PostGIS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * PostGIS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
 *
 **********************************************************************
 *
 * Copyright 2014 Nicklas Avén
 *
 **********************************************************************/

#ifndef _EFFECTIVEAREA_H
#define _EFFECTIVEAREA_H 1

/**
 * Structure to hold the info of a POINT array.
 * This structure accommodates the original PostGIS POINTARRAY declaration
 * to use from a QGis WKB-stream.
 */
typedef struct
{
  double *pointlist;
  int dimension;
  int flags;
  int npoints;
} POINTARRAY;

/**
 * This structure is placed in an array with one member per point.
 * It has links into the minheap rtee and kepps track of eliminated points.
 */
typedef struct
{
  double area;
  int treeindex;
  int prev;
  int next;
} areanode;

/**
 * This structure holds a minheap tree that is used to keep track of what points
 * that has the smallest effective area.
 * When elliminating points the neighbor points has its effective area affected
 * and the minheap helps to resort efficient.
 */
typedef struct
{
  int maxSize;
  int usedSize;
  areanode **key_array;
} MINHEAP;

/**
 * Structure to hold pointarray and it's arealist.
 */
typedef struct
{
  const POINTARRAY *inpts;
  areanode *initial_arealist;
  double *res_arealist;
} EFFECTIVE_AREAS;

EFFECTIVE_AREAS* initiate_effectivearea( const POINTARRAY *inpts );

void destroy_effectivearea( EFFECTIVE_AREAS *ea );

void ptarray_calc_areas( EFFECTIVE_AREAS *ea, int avoid_collaps, int set_area, double trshld );

double* getPoint_internal( const POINTARRAY* inpts, int pointIndex );

#endif /* _EFFECTIVEAREA_H */
