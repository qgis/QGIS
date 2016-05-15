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

#include "effectivearea.h"

EFFECTIVE_AREAS* initiate_effectivearea( const POINTARRAY *inpts )
{
  //LWDEBUG( 2, "Entered  initiate_effectivearea" );
  EFFECTIVE_AREAS *ea;
  ea = ( EFFECTIVE_AREAS* )lwalloc( sizeof( EFFECTIVE_AREAS ) );
  ea->initial_arealist = ( areanode* )lwalloc( inpts->npoints * sizeof( areanode ) );
  ea->res_arealist = ( double* )lwalloc( inpts->npoints * sizeof( double ) );
  ea->inpts = inpts;
  return ea;
}

void destroy_effectivearea( EFFECTIVE_AREAS *ea )
{
  lwfree( ea->initial_arealist );
  lwfree( ea->res_arealist );
  lwfree( ea );
}

static MINHEAP initiate_minheap( int npoints )
{
  MINHEAP tree;
  tree.key_array = ( areanode** )lwalloc( npoints * sizeof( void* ) );
  tree.maxSize = npoints;
  tree.usedSize = 0;
  return tree;
}

static void destroy_minheap( MINHEAP tree )
{
  lwfree( tree.key_array );
}

/**
 * Calculate the area of a triangle in 2d
 */
static double triarea2d( const double *P1, const double *P2, const double *P3 )
{
  return qAbs( 0.5 * (( P1[0] - P2[0] ) * ( P3[1] - P2[1] ) - ( P1[1] - P2[1] ) * ( P3[0] - P2[0] ) ) );
}

/**
 * Calculate the area of a triangle in 3d space
 */
static double triarea3d( const double *P1, const double *P2, const double *P3 )
{
  //LWDEBUG( 2, "Entered  triarea3d" );
  double ax, bx, ay, by, az, bz, cx, cy, cz, area;

  ax = P1[0] - P2[0];
  bx = P3[0] - P2[0];
  ay = P1[1] - P2[1];
  by = P3[1] - P2[1];
  az = P1[2] - P2[2];
  bz = P3[2] - P2[2];

  cx = ay * bz - az * by;
  cy = az * bx - ax * bz;
  cz = ax * by - ay * bx;

  area = qAbs( 0.5 * ( sqrt( cx * cx + cy * cy + cz * cz ) ) );
  return area;
}

/**
 * We create the minheap by ordering the minheap array by the areas in the areanode structs that the minheap keys refere to
 */
static int cmpfunc( const void * a, const void * b )
{
  double v1 = ( *( areanode** )a )->area;
  double v2 = ( *( areanode** )b )->area;

  /* qsort gives unpredictable results when comaping identical values.
   * If two values is the same we force returning the last point in hte point array.
   * That way we get the same ordering on diffreent machines and pllatforms
   */
  if ( v1 == v2 )
    return ( *( areanode** )a ) - ( *( areanode** )b );
  else
    return ( v1 > v2 ) ? 1 : -1;
}

/**
 * Sift Down
 */
static void down( MINHEAP *tree, areanode *arealist, int parent )
{
  //LWDEBUG( 2, "Entered  down" );
  areanode **treearray = tree->key_array;
  int left = parent * 2 + 1;
  int right = left + 1;
  areanode *tmp;
  int swap = parent;
  double leftarea = 0;
  double rightarea = 0;
  double parentarea = (( areanode* )treearray[parent] )->area;

  if ( left < tree->usedSize )
  {
    leftarea = (( areanode* )treearray[left] )->area;
    if ( parentarea > leftarea ) swap = left;
  }
  if ( right < tree->usedSize )
  {
    rightarea = (( areanode* )treearray[right] )->area;
    if ( rightarea < parentarea && rightarea < leftarea ) swap = right;
  }
  if ( swap > parent )
  {
    // ok, we have to swap something
    tmp = treearray[parent];
    treearray[parent] = treearray[swap];
    // Update reference
    (( areanode* )treearray[parent] )->treeindex = parent;
    treearray[swap] = tmp;
    // Update reference
    (( areanode* )treearray[swap] )->treeindex = swap;
    if ( swap < tree->usedSize ) down( tree, arealist, swap );
  }
}

/**
 * Sift Up
 */
static void up( MINHEAP *tree, areanode *arealist, int c )
{
  //LWDEBUG( 2, "Entered  up" );
  areanode *tmp;

  Q_UNUSED( arealist );

  areanode **treearray = tree->key_array;
  int parent = ( c - 1 ) / 2;

  while ((( areanode* )treearray[c] )->area < (( areanode* )treearray[parent] )->area )
  {
    // ok, we have to swap
    tmp = treearray[parent];
    treearray[parent] = treearray[c];
    // Update reference
    (( areanode* )treearray[parent] )->treeindex = parent;
    treearray[c] = tmp;
    // Update reference
    (( areanode* )treearray[c] )->treeindex = c;
    c = parent;
    parent = ( c - 1 ) / 2;
  }
}

/**
 * Get a reference to the point with the smallest effective area from the root of the min heap
 */
static areanode* minheap_pop( MINHEAP *tree, areanode *arealist )
{
  //LWDEBUG( 2, "Entered  minheap_pop" );
  areanode *res = tree->key_array[0];

  // put last value first
  tree->key_array[0] = tree->key_array[( tree->usedSize ) - 1];
  (( areanode* )tree->key_array[0] )->treeindex = 0;

  tree->usedSize--;
  down( tree, arealist, 0 );
  return res;
}

/**
 * The member of the minheap at index idx is changed. Update the tree and make restore the heap property
 */
static void minheap_update( MINHEAP *tree, areanode *arealist, int idx )
{
  areanode **treearray = tree->key_array;
  int parent = ( idx - 1 ) / 2;

  if ((( areanode* )treearray[idx] )->area < (( areanode* )treearray[parent] )->area )
    up( tree, arealist, idx );
  else
    down( tree, arealist, idx );
}

/**
 * To get the effective area, we have to check what area a point results in when all smaller areas are eliminated
 */
static void tune_areas( EFFECTIVE_AREAS *ea, int avoid_collaps, int set_area, double trshld )
{
  //LWDEBUG( 2, "Entered  tune_areas" );
  const double *P1;
  const double *P2;
  const double *P3;
  double area;
  int go_on = 1;
  double check_order_min_area = 0;

  int npoints = ea->inpts->npoints;
  int i;
  int current, before_current, after_current;

  MINHEAP tree = initiate_minheap( npoints );

  int is3d = FLAGS_GET_Z( ea->inpts->flags );

  // Add all keys (index in initial_arealist) into minheap array
  for ( i = 0; i < npoints; i++ )
  {
    tree.key_array[i] = ea->initial_arealist + i;
    //LWDEBUGF( 2, "add nr %d, with area %lf, and %lf", i, ea->initial_arealist[i].area, tree.key_array[i]->area );
  }
  tree.usedSize = npoints;

  // order the keys by area, small to big
  qsort( tree.key_array, npoints, sizeof( void* ), cmpfunc );

  // We have to put references to our tree in our point-list
  for ( i = 0; i < npoints; i++ )
  {
    (( areanode* )tree.key_array[i] )->treeindex = i;
    //LWDEBUGF( 4, "Check ordering qsort gives, area=%lf and belong to point %d", (( areanode* )tree.key_array[i] )->area, tree.key_array[i] - ea->initial_arealist );
  }

  // Ok, now we have a minHeap, just need to keep it
  i = 0;
  while ( go_on )
  {
    // Get a reference to the point with the currently smallest effective area
    current = minheap_pop( &tree, ea->initial_arealist ) - ea->initial_arealist;

    // We have found the smallest area. That is the resulting effective area for the "current" point
    if ( i < npoints - avoid_collaps )
      ea->res_arealist[current] = ea->initial_arealist[current].area;
    else
      ea->res_arealist[current] = FLT_MAX;

    if ( ea->res_arealist[current] < check_order_min_area )
      lwerror( "Oh no, this is a bug. For some reason the minHeap returned our points in the wrong order. Please file a ticket in PostGIS ticket system, or send a mial at the mailing list.Returned area = %lf, and last area = %lf", ea->res_arealist[current], check_order_min_area );

    check_order_min_area = ea->res_arealist[current];

    // The found smallest area point is now regarded as elimnated and we have to recalculate the area the adjacent (ignoring earlier elimnated points) points gives

    // Find point before and after
    before_current = ea->initial_arealist[current].prev;
    after_current  = ea->initial_arealist[current].next;

    P2 = ( double* )getPoint_internal( ea->inpts, before_current );
    P3 = ( double* )getPoint_internal( ea->inpts, after_current );

    // Check if point before current point is the first in the point array.
    if ( before_current > 0 )
    {
      P1 = ( double* )getPoint_internal( ea->inpts, ea->initial_arealist[before_current].prev );

      if ( is3d )
        area = triarea3d( P1, P2, P3 );
      else
        area = triarea2d( P1, P2, P3 );

      ea->initial_arealist[before_current].area = FP_MAX( area, ea->res_arealist[current] );
      minheap_update( &tree, ea->initial_arealist, ea->initial_arealist[before_current].treeindex );
    }
    if ( after_current < npoints - 1 ) // Check if point after current point is the last in the point array.
    {
      P1 = P2;
      P2 = P3;
      P3 = ( double* )getPoint_internal( ea->inpts, ea->initial_arealist[after_current].next );

      if ( is3d )
        area = triarea3d( P1, P2, P3 );
      else
        area = triarea2d( P1, P2, P3 );

      ea->initial_arealist[after_current].area = FP_MAX( area, ea->res_arealist[current] );
      minheap_update( &tree, ea->initial_arealist, ea->initial_arealist[after_current].treeindex );
    }

    // rearrange the nodes so the eliminated point will be ingored on the next run
    ea->initial_arealist[before_current].next = ea->initial_arealist[current].next;
    ea->initial_arealist[after_current ].prev = ea->initial_arealist[current].prev;

    // Check if we are finnished
    if (( !set_area && ea->res_arealist[current] > trshld ) || ( ea->initial_arealist[0].next == ( npoints - 1 ) ) )
      go_on = 0;

    i++;
  }
  destroy_minheap( tree );
}

/**
 * We calculate the effective area for the first time
 */
void ptarray_calc_areas( EFFECTIVE_AREAS *ea, int avoid_collaps, int set_area, double trshld )
{
  //LWDEBUG( 2, "Entered  ptarray_calc_areas" );
  int i;
  int npoints = ea->inpts->npoints;
  int is3d = FLAGS_GET_Z( ea->inpts->flags );
  double area;

  const double *P1;
  const double *P2;
  const double *P3;
  P1 = ( double* )getPoint_internal( ea->inpts, 0 );
  P2 = ( double* )getPoint_internal( ea->inpts, 1 );

  // The first and last point shall always have the maximum effective area. We use float max to not make trouble for bbox
  ea->initial_arealist[0].area = ea->initial_arealist[npoints - 1].area = FLT_MAX;
  ea->res_arealist[0] = ea->res_arealist[npoints - 1] = FLT_MAX;

  ea->initial_arealist[0].next = 1;
  ea->initial_arealist[0].prev = 0;

  for ( i = 1; i < ( npoints ) - 1; i++ )
  {
    ea->initial_arealist[i].next = i + 1;
    ea->initial_arealist[i].prev = i - 1;
    P3 = ( double* )getPoint_internal( ea->inpts, i + 1 );

    if ( is3d )
      area = triarea3d( P1, P2, P3 );
    else
      area = triarea2d( P1, P2, P3 );

    //LWDEBUGF( 4, "Write area %lf to point %d on address %p", area, i, &( ea->initial_arealist[i].area ) );
    ea->initial_arealist[i].area = area;
    P1 = P2;
    P2 = P3;
  }
  ea->initial_arealist[npoints - 1].next = npoints - 1;
  ea->initial_arealist[npoints - 1].prev = npoints - 2;

  for ( i = 1; i < ( npoints ) - 1; i++ )
  {
    ea->res_arealist[i] = FLT_MAX;
  }
  tune_areas( ea, avoid_collaps, set_area, trshld );
}
