/*

 gg_relations.c -- Gaia spatial relations

 version 2.3, 2008 October 13

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------

 Version: MPL 1.1/GPL 2.0/LGPL 2.1

 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri

Portions created by the Initial Developer are Copyright (C) 2008
the Initial Developer. All Rights Reserved.

Contributor(s):

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.

*/

#include <stdlib.h>
#include <stdio.h>

#if OMIT_GEOS == 0  /* including GEOS */
#include <geos_c.h>
#endif

#include <spatialite/sqlite3ext.h>
#include <spatialite/gaiageo.h>

static int
check_point( double *coords, int points, double x, double y )
{
  /* checks if [X,Y] point is defined into this coordinate array [Linestring or Ring] */
  int iv;
  double xx;
  double yy;
  for ( iv = 0; iv < points; iv++ )
  {
    gaiaGetPoint( coords, iv, &xx, &yy );
    if ( xx == x && yy == y )
      return 1;
  }
  return 0;
}

GAIAGEO_DECLARE int
gaiaLinestringEquals( gaiaLinestringPtr line1, gaiaLinestringPtr line2 )
{
  /* checks if two Linestrings are "spatially equal" */
  int iv;
  double x;
  double y;
  if ( line1->Points != line2->Points )
    return 0;
  for ( iv = 0; iv < line1->Points; iv++ )
  {
    gaiaGetPoint( line1->Coords, iv, &x, &y );
    if ( !check_point( line2->Coords, line2->Points, x, y ) )
      return 0;
  }
  return 1;
}

GAIAGEO_DECLARE int
gaiaPolygonEquals( gaiaPolygonPtr polyg1, gaiaPolygonPtr polyg2 )
{
  /* checks if two Polygons are "spatially equal" */
  int ib;
  int ib2;
  int iv;
  int ok2;
  double x;
  double y;
  gaiaRingPtr ring1;
  gaiaRingPtr ring2;
  if ( polyg1->NumInteriors != polyg2->NumInteriors )
    return 0;
  /* checking the EXTERIOR RINGs */
  ring1 = polyg1->Exterior;
  ring2 = polyg2->Exterior;
  if ( ring1->Points != ring2->Points )
    return 0;
  for ( iv = 0; iv < ring1->Points; iv++ )
  {
    gaiaGetPoint( ring1->Coords, iv, &x, &y );
    if ( !check_point( ring2->Coords, ring2->Points, x, y ) )
      return 0;
  }
  for ( ib = 0; ib < polyg1->NumInteriors; ib++ )
  {
    /* checking the INTERIOR RINGS */
    int ok = 0;
    ring1 = polyg1->Interiors + ib;
    for ( ib2 = 0; ib2 < polyg2->NumInteriors; ib2++ )
    {
      ok2 = 1;
      ring2 = polyg2->Interiors + ib2;
      for ( iv = 0; iv < ring1->Points; iv++ )
      {
        gaiaGetPoint( ring1->Coords, iv, &x, &y );
        if ( !check_point( ring2->Coords, ring2->Points, x, y ) )
        {
          ok2 = 0;
          break;
        }
      }
      if ( ok2 )
      {
        ok = 1;
        break;
      }
    }
    if ( !ok )
      return 0;
  }
  return 1;
}

#if OMIT_GEOS == 0  /* including GEOS */

GAIAGEO_DECLARE int
gaiaGeomCollEquals( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries are "spatially equal" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSEquals( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollIntersects( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries do "spatially intersects" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSIntersects( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDisjoint( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries are "spatially disjoint" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSDisjoint( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollOverlaps( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries do "spatially overlaps" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSOverlaps( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCrosses( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries do "spatially crosses" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSCrosses( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollTouches( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if two Geometries do "spatially touches" */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSTouches( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollWithin( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if GEOM-1 is completely contained within GEOM-2 */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSWithin( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollContains( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* checks if GEOM-1 completely contains GEOM-2 */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSContains( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollRelate( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
                    const char *pattern )
{
  /* checks if if GEOM-1 and GEOM-2 have a spatial relationship as specified by the pattern Matrix */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return -1;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSRelatePattern( g1, g2, pattern );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLength( gaiaGeomCollPtr geom, double *xlength )
{
  /* computes the total length for this Geometry */
  double length;
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g;
  if ( !geom )
    return 0;
  gaiaToWkb( geom, &p_result, &len );
  g = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSLength( g, &length );
  GEOSGeom_destroy( g );
  if ( ret )
    *xlength = length;
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollArea( gaiaGeomCollPtr geom, double *xarea )
{
  /* computes the total area for this Geometry */
  double area;
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g;
  if ( !geom )
    return 0;
  gaiaToWkb( geom, &p_result, &len );
  g = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSArea( g, &area );
  GEOSGeom_destroy( g );
  if ( ret )
    *xarea = area;
  return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDistance( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
                      double *xdist )
{
  /* computes the minimum distance intercurring between GEOM-1 and GEOM-2 */
  double dist;
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom1 || !geom2 )
    return 0;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSDistance( g1, g2, &dist );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  if ( ret )
    *xdist = dist;
  return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryIntersection( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* builds a new geometry representing the "spatial intersection" of GEOM-1 and GEOM-2 */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr geo;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  GEOSGeometry *g3;
  if ( !geom1 || !geom2 )
    return NULL;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g3 = GEOSIntersection( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  if ( !g3 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g3, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g3 );
    return NULL;
  }
  geo = gaiaFromWkb( p_result, ( int ) tlen );
  if ( geo == NULL )
  {
    free( p_result );
    return NULL;
  }
  geo->Srid = GEOSGetSRID( g3 );
  GEOSGeom_destroy( g3 );
  free( p_result );
  return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryUnion( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* builds a new geometry representing the "spatial union" of GEOM-1 and GEOM-2 */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr geo;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  GEOSGeometry *g3;
  if ( !geom1 || !geom2 )
    return NULL;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g3 = GEOSUnion( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  if ( !g3 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g3, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g3 );
    return NULL;
  }
  geo = gaiaFromWkb( p_result, ( int ) tlen );
  if ( geo == NULL )
  {
    free( p_result );
    return NULL;
  }
  geo->Srid = GEOSGetSRID( g3 );
  GEOSGeom_destroy( g3 );
  free( p_result );
  return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryDifference( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* builds a new geometry representing the "spatial difference" of GEOM-1 and GEOM-2 */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr geo;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  GEOSGeometry *g3;
  if ( !geom1 || !geom2 )
    return NULL;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g3 = GEOSDifference( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  if ( !g3 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g3, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g3 );
    return NULL;
  }
  geo = gaiaFromWkb( p_result, ( int ) tlen );
  if ( geo == NULL )
  {
    free( p_result );
    return NULL;
  }
  geo->Srid = GEOSGetSRID( g3 );
  GEOSGeom_destroy( g3 );
  free( p_result );
  return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometrySymDifference( gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2 )
{
  /* builds a new geometry representing the "spatial symmetric difference" of GEOM-1 and GEOM-2 */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr geo;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  GEOSGeometry *g3;
  if ( !geom1 || !geom2 )
    return NULL;
  gaiaToWkb( geom1, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  gaiaToWkb( geom2, &p_result, &len );
  g2 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g3 = GEOSSymDifference( g1, g2 );
  GEOSGeom_destroy( g1 );
  GEOSGeom_destroy( g2 );
  if ( !g3 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g3, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g3 );
    return NULL;
  }
  geo = gaiaFromWkb( p_result, ( int ) tlen );
  if ( geo == NULL )
  {
    free( p_result );
    return NULL;
  }
  geo->Srid = GEOSGetSRID( g3 );
  GEOSGeom_destroy( g3 );
  free( p_result );
  return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaBoundary( gaiaGeomCollPtr geom )
{
  /* builds a new geometry representing the conbinatorial boundary of GEOM */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr geo;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return NULL;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSBoundary( g1 );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return NULL;
  }
  geo = gaiaFromWkb( p_result, ( int ) tlen );
  if ( geo == NULL )
  {
    free( p_result );
    return NULL;
  }
  geo->Srid = GEOSGetSRID( g2 );
  GEOSGeom_destroy( g2 );
  free( p_result );
  return geo;
}

GAIAGEO_DECLARE int
gaiaGeomCollCentroid( gaiaGeomCollPtr geom, double *x, double *y )
{
  /* returns a Point representing the centroid for this Geometry */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return 0;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSGetCentroid( g1 );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return 0;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return 0;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  if ( !result )
  {
    free( p_result );
    return 0;
  }
  free( p_result );
  if ( result->FirstPoint )
  {
    *x = result->FirstPoint->X;
    *y = result->FirstPoint->Y;
    gaiaFreeGeomColl( result );
    return 1;
  }
  gaiaFreeGeomColl( result );
  return 0;
}

GAIAGEO_DECLARE int
gaiaGetPointOnSurface( gaiaGeomCollPtr geom, double *x, double *y )
{
  /* returns a Point guaranteed to lie on the Surface */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return 0;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSPointOnSurface( g1 );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return 0;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return 0;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  if ( !result )
  {
    free( p_result );
    return 0;
  }
  free( p_result );
  if ( result->FirstPoint )
  {
    *x = result->FirstPoint->X;
    *y = result->FirstPoint->Y;
    gaiaFreeGeomColl( result );
    return 1;
  }
  gaiaFreeGeomColl( result );
  return 0;
}

GAIAGEO_DECLARE int
gaiaIsSimple( gaiaGeomCollPtr geom )
{
  /* checks if this GEOMETRYCOLLECTION is a simple one */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g;
  if ( !geom )
    return -1;
  gaiaToWkb( geom, &p_result, &len );
  g = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSisSimple( g );
  GEOSGeom_destroy( g );
  if ( ret == 2 )
    return -1;
  return ret;
}

GAIAGEO_DECLARE int
gaiaIsRing( gaiaLinestringPtr line )
{
  /* checks if this LINESTRING can be a valid RING */
  gaiaGeomCollPtr geo;
  gaiaLinestringPtr line2;
  int ret;
  int len;
  int iv;
  double x;
  double y;
  unsigned char *p_result = NULL;
  GEOSGeometry *g;
  if ( !line )
    return -1;
  geo = gaiaAllocGeomColl();
  line2 = gaiaAddLinestringToGeomColl( geo, line->Points );
  for ( iv = 0; iv < line2->Points; iv++ )
  {
    gaiaGetPoint( line->Coords, iv, &x, &y );
    gaiaSetPoint( line2->Coords, iv, x, y );
  }
  gaiaToWkb( geo, &p_result, &len );
  gaiaFreeGeomColl( geo );
  g = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSisRing( g );
  GEOSGeom_destroy( g );
  if ( ret == 2 )
    return -1;
  return ret;
}

GAIAGEO_DECLARE int
gaiaIsValid( gaiaGeomCollPtr geom )
{
  /* checks if this GEOMETRYCOLLECTION is a valid one */
  int ret;
  int len;
  unsigned char *p_result = NULL;
  GEOSGeometry *g;
  if ( !geom )
    return -1;
  gaiaToWkb( geom, &p_result, &len );
  g = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  ret = GEOSisValid( g );
  GEOSGeom_destroy( g );
  if ( ret == 2 )
    return -1;
  return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplify( gaiaGeomCollPtr geom, double tolerance )
{
  /* builds a simplified geometry using the Douglas-Peuker algorihtm */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return NULL;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSSimplify( g1, tolerance );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return NULL;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  free( p_result );
  result->Srid = geom->Srid;
  return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplifyPreserveTopology( gaiaGeomCollPtr geom, double tolerance )
{
  /* builds a simplified geometry using the Douglas-Peuker algorihtm [preserving topology] */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return NULL;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSTopologyPreserveSimplify( g1, tolerance );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return NULL;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  free( p_result );
  result->Srid = geom->Srid;
  return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaConvexHull( gaiaGeomCollPtr geom )
{
  /* builds a geometry that is the convex hull of GEOM */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return NULL;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSConvexHull( g1 );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return NULL;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  free( p_result );
  result->Srid = geom->Srid;
  return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollBuffer( gaiaGeomCollPtr geom, double radius, int points )
{
  /* builds a geometry that is the GIS buffer of GEOM */
  int len;
  size_t tlen;
  unsigned char *p_result = NULL;
  gaiaGeomCollPtr result;
  GEOSGeometry *g1;
  GEOSGeometry *g2;
  if ( !geom )
    return NULL;
  gaiaToWkb( geom, &p_result, &len );
  g1 = GEOSGeomFromWKB_buf( p_result, len );
  free( p_result );
  g2 = GEOSBuffer( g1, radius, points );
  GEOSGeom_destroy( g1 );
  if ( !g2 )
    return NULL;
  p_result = GEOSGeomToWKB_buf( g2, &tlen );
  if ( !p_result )
  {
    GEOSGeom_destroy( g2 );
    return NULL;
  }
  GEOSGeom_destroy( g2 );
  result = gaiaFromWkb( p_result, ( int ) tlen );
  free( p_result );
  result->Srid = geom->Srid;
  return result;
}

#endif /* end including GEOS */
