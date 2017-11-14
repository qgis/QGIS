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

#include "qgsabstractgeometry.h"
#include "qgscurve.h"
#include "qgspoint.h"

#define SIP_NO_FILE

#ifndef _EFFECTIVEAREA_H
#define _EFFECTIVEAREA_H 1


#define LWDEBUG //
#define LWDEBUGF //
#define FP_MAX std::max
#define FLAGS_GET_Z( flags ) ( ( flags ) & 0x01 )
#define LW_MSG_MAXLEN 256
#define lwalloc qgsMalloc
#define lwfree qgsFree
#define lwerror qWarning


/**
 * This structure is placed in an array with one member per point.
 * It has links into the minheap rtee and keeps track of eliminated points.
 */
struct areanode
{
  double area;
  int treeindex;
  int prev;
  int next;
};

/**
 * This structure holds a minheap tree that is used to keep track of what points
 * that has the smallest effective area.
 * When eliminating points the neighbor points has its effective area affected
 * and the minheap helps to resort efficient.
 */
struct MINHEAP
{
  int maxSize;
  int usedSize;
  areanode **key_array = nullptr;
};

/**
 * Structure to hold point array and its arealist.
 */
struct EFFECTIVE_AREAS
{
  EFFECTIVE_AREAS( const QgsCurve &curve )
    : is3d( curve.is3D() )
  {
    curve.points( inpts );
    initial_arealist = new areanode[ inpts.size()];
    res_arealist = new double[ inpts.size()];
  }

  ~EFFECTIVE_AREAS()
  {
    delete [] initial_arealist;
    delete [] res_arealist;
  }

  EFFECTIVE_AREAS( const EFFECTIVE_AREAS &other ) = delete;
  EFFECTIVE_AREAS &operator=( const EFFECTIVE_AREAS &other ) = delete;

  bool is3d;
  QgsPointSequence inpts;
  areanode *initial_arealist = nullptr;
  double *res_arealist = nullptr;


};

void ptarray_calc_areas( EFFECTIVE_AREAS *ea, int avoid_collaps, int set_area, double trshld );

#endif /* _EFFECTIVEAREA_H */
