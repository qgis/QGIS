/*

 gg_advanced.c -- Gaia advanced geometric operations

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
#include <math.h>
#include <float.h>

#include <spatialite/sqlite3ext.h>
#include <spatialite/gaiageo.h>

GAIAGEO_DECLARE double
gaiaMeasureLength( double *coords, int vert )
{
  /* computes the total length */
  double lung = 0.0;
  double xx1;
  double xx2;
  double yy1;
  double yy2;
  double x;
  double y;
  double dist;
  int ind;
  for ( ind = 0; ind < vert; ind++ )
  {
    if ( ind == 0 )
    {
      gaiaGetPoint( coords, ind, &xx1, &yy1 );
    }
    else
    {
      gaiaGetPoint( coords, ind, &xx2, &yy2 );
      x = xx1 - xx2;
      y = yy1 - yy2;
      dist = sqrt(( x * x ) + ( y * y ) );
      lung += dist;
      xx1 = xx2;
      yy1 = yy2;
    }
  }
  return lung;
}

GAIAGEO_DECLARE double
gaiaMeasureArea( gaiaRingPtr ring )
{
  /* computes the area */
  int iv;
  double xx;
  double yy;
  double x;
  double y;
  double area = 0.0;
  if ( !ring )
    return 0.0;
  gaiaGetPoint( ring->Coords, 0, &xx, &yy );
  for ( iv = 1; iv < ring->Points; iv++ )
  {
    gaiaGetPoint( ring->Coords, iv, &x, &y );
    area += (( xx * y ) - ( x * yy ) );
    xx = x;
    yy = y;
  }
  area /= 2.0;
  return fabs( area );
}

GAIAGEO_DECLARE void
gaiaRingCentroid( gaiaRingPtr ring, double *rx, double *ry )
{
  /* computes the simple ring centroid */
  double cx = 0.0;
  double cy = 0.0;
  double xx;
  double yy;
  double x;
  double y;
  double coeff;
  double area;
  double term;
  int iv;
  if ( !ring )
  {
    *rx = DBL_MIN;
    *ry = DBL_MIN;
    return;
  }
  area = gaiaMeasureArea( ring );
  coeff = 1.0 / ( area * 6.0 );
  gaiaGetPoint( ring->Coords, 0, &xx, &yy );
  for ( iv = 0; iv < ring->Points; iv++ )
  {
    gaiaGetPoint( ring->Coords, iv, &x, &y );
    term = ( xx * y ) - ( x * yy );
    cx += ( xx + x ) * term;
    cy += ( yy + y ) * term;
    xx = x;
    yy = y;
  }
  *rx = fabs( cx * coeff );
  *ry = fabs( cy * coeff );
}

GAIAGEO_DECLARE void
gaiaClockwise( gaiaRingPtr p )
{
  /* determines clockwise or anticlockwise direction */
  int ind;
  int ix;
  double xx;
  double yy;
  double x;
  double y;
  double area = 0.0;
  gaiaGetPoint( p->Coords, 0, &x, &y );
  for ( ind = 0; ind < p->Points; ind++ )
  {
    gaiaGetPoint( p->Coords, ind, &xx, &yy );
    ix = ( ind + 1 ) % p->Points;
    gaiaGetPoint( p->Coords, ix, &x, &y );
    area += (( xx * y ) - ( x * yy ) );
  }
  area /= 2.0;
  if ( area >= 0.0 )
    p->Clockwise = 0;
  else
    p->Clockwise = 1;
}

GAIAGEO_DECLARE int
gaiaIsPointOnRingSurface( gaiaRingPtr ring, double pt_x, double pt_y )
{
  /* tests if a POINT falls inside a RING */
  int isInternal = 0;
  int cnt;
  int i;
  int j;
  double x;
  double y;
  double *vert_x;
  double *vert_y;
  double minx = DBL_MAX;
  double miny = DBL_MAX;
  double maxx = DBL_MIN;
  double maxy = DBL_MIN;
  cnt = ring->Points;
  cnt--;   /* ignoring last vertex because surely identical to the first one */
  if ( cnt < 2 )
    return 0;
  /* allocating and loading an array of vertices */
  vert_x = malloc( sizeof( double ) * ( cnt ) );
  vert_y = malloc( sizeof( double ) * ( cnt ) );
  for ( i = 0; i < cnt; i++ )
  {
    gaiaGetPoint( ring->Coords, i, &x, &y );
    vert_x[i] = x;
    vert_y[i] = y;
    if ( x < minx )
      minx = x;
    if ( x > maxx )
      maxx = x;
    if ( y < miny )
      miny = y;
    if ( y > maxy )
      maxy = y;
  }
  if ( x < minx || x > maxx )
    goto end;  /* outside the bounding box (x axis) */
  if ( y < miny || y > maxy )
    goto end;  /* outside the bounding box (y axis) */
  for ( i = 0, j = cnt - 1; i < cnt; j = i++ )
  {
    /* The definitive reference is "Point in Polyon Strategies" by
    /  Eric Haines [Gems IV]  pp. 24-46.
    /  The code in the Sedgewick book Algorithms (2nd Edition, p.354) is
    /  incorrect.
    */
    if (((( vert_y[i] <= pt_y ) && ( pt_y < vert_y[j] ) ) ||
         (( vert_y[j] <= pt_y ) && ( y < vert_y[i] ) ) ) &&
        ( pt_x < ( vert_x[j] - vert_x[i] ) *
          ( pt_y - vert_y[i] ) / ( vert_y[j] - vert_y[i] ) + vert_x[i] ) )
      isInternal = !isInternal;
  }
end:
  free( vert_x );
  free( vert_y );
  return isInternal;
}

GAIAGEO_DECLARE double
gaiaMinDistance( double x0, double y0, double *coords, int n_vert )
{
  /* computing minimal distance between a POINT and a linestring/ring */
  double x;
  double y;
  double ox;
  double oy;
  double lineMag;
  double u;
  double px;
  double py;
  double dist;
  double min_dist = DBL_MAX;
  int iv;
  if ( n_vert < 2 )
    return min_dist; /* not a valid linestring */
  /* computing distance from first vertex */
  ox = *( coords + 0 );
  oy = *( coords + 1 );
  min_dist = sqrt((( x0 - ox ) * ( x0 - ox ) ) + (( y0 - oy ) * ( y0 - oy ) ) );
  for ( iv = 1; iv < n_vert; iv++ )
  {
    /* segment start-end coordinates */
    gaiaGetPoint( coords, iv - 1, &ox, &oy );
    gaiaGetPoint( coords, iv, &x, &y );
    /* computing distance from vertex */
    dist = sqrt((( x0 - x ) * ( x0 - x ) ) + (( y0 - y ) * ( y0 - y ) ) );
    if ( dist < min_dist )
      min_dist = dist;
    /* computing a projection */
    lineMag = (( x - ox ) * ( x - ox ) ) + (( y - oy ) * ( y - oy ) );
    u = ((( x0 - ox ) * ( x - ox ) ) + (( y0 - oy ) * ( y - oy ) ) ) / lineMag;
    if ( u < 0.0 || u > 1.0 )
      ;   /* closest point does not fall within the line segment */
    else
    {
      px = ox + u * ( x - ox );
      py = oy + u * ( y - oy );
      dist = sqrt((( x0 - px ) * ( x0 - px ) ) + (( y0 - py ) * ( y0 - py ) ) );
      if ( dist < min_dist )
        min_dist = dist;
    }
  }
  return min_dist;
}

GAIAGEO_DECLARE int
gaiaIsPointOnPolygonSurface( gaiaPolygonPtr polyg, double x, double y )
{
  /* tests if a POINT falls inside a POLYGON */
  int ib;
  gaiaRingPtr ring = polyg->Exterior;
  if ( gaiaIsPointOnRingSurface( ring, x, y ) )
  {
    /* ok, the POINT falls inside the polygon */
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      ring = polyg->Interiors + ib;
      if ( gaiaIsPointOnRingSurface( ring, x, y ) )
      {
        /* no, the POINT fall inside some hole */
        return 0;
      }
    }
    return 1;
  }
  return 0;
}

GAIAGEO_DECLARE int
gaiaIntersect( double *x0, double *y0, double x1, double y1, double x2,
               double y2, double x3, double y3, double x4, double y4 )
{
  /* computes intersection [if any] between two line segments
  /  the intersection POINT has coordinates (x0, y0)
  /  first line is identified by(x1, y1)  and (x2, y2)
  /  second line is identified by (x3, y3) and (x4, y4)
  */
  double x;
  double y;
  double a1;
  double b1;
  double c1;
  double a2;
  double b2;
  double c2;
  double m1;
  double m2;
  double p;
  double det_inv;
  double minx1;
  double miny1;
  double maxx1;
  double maxy1;
  double minx2;
  double miny2;
  double maxx2;
  double maxy2;
  int ok1 = 0;
  int ok2 = 0;
  /* building line segment's MBRs */
  if ( x2 < x1 )
  {
    minx1 = x2;
    maxx1 = x1;
  }
  else
  {
    minx1 = x1;
    maxx1 = x2;
  }
  if ( y2 < y1 )
  {
    miny1 = y2;
    maxy1 = y1;
  }
  else
  {
    miny1 = y1;
    maxy1 = y2;
  }
  if ( x4 < x3 )
  {
    minx2 = x4;
    maxx2 = x3;
  }
  else
  {
    minx2 = x3;
    maxx2 = x4;
  }
  if ( y4 < y3 )
  {
    miny2 = y4;
    maxy2 = y3;
  }
  else
  {
    miny2 = y3;
    maxy2 = y4;
  }
  /* checkinkg MBRs first */
  if ( minx1 >= maxx2 )
    return 0;
  if ( miny1 >= maxy2 )
    return 0;
  if ( maxx1 <= minx2 )
    return 0;
  if ( maxy1 <= miny2 )
    return 0;
  if ( minx2 >= maxx1 )
    return 0;
  if ( miny2 >= maxy1 )
    return 0;
  if ( maxx2 <= minx1 )
    return 0;
  if ( maxy2 <= miny1 )
    return 0;
  /* there is an MBRs intersection - proceeding */
  if (( x2 - x1 ) != 0.0 )
    m1 = ( y2 - y1 ) / ( x2 - x1 );
  else
    m1 = DBL_MAX;
  if (( x4 - x3 ) != 0 )
    m2 = ( y4 - y3 ) / ( x4 - x3 );
  else
    m2 = DBL_MAX;
  if ( m1 == m2 )  /* parallel lines */
    return 0;
  if ( m1 == DBL_MAX )
    c1 = y1;
  else
    c1 = ( y1 - m1 * x1 );
  if ( m2 == DBL_MAX )
    c2 = y3;
  else
    c2 = ( y3 - m2 * x3 );
  if ( m1 == DBL_MAX )
  {
    x = x1;
    p = m2 * x1;
    y = p + c2;  /*  first line is vertical */
    goto check_bbox;
  }
  if ( m2 == DBL_MAX )
  {
    x = x3;
    p = m1 * x3;
    y = p + c1;  /* second line is vertical */
    goto check_bbox;
  }
  a1 = m1;
  a2 = m2;
  b1 = -1;
  b2 = -1;
  det_inv = 1 / ( a1 * b2 - a2 * b1 );
  x = (( b1 * c2 - b2 * c1 ) * det_inv );
  y = (( a2 * c1 - a1 * c2 ) * det_inv );
  /* now checking if intersection falls within both segment boundaries */
check_bbox:
  if ( x >= minx1 && x <= maxx1 && y >= miny1 && y <= maxy1 )
    ok1 = 1;
  if ( x >= minx2 && x <= maxx2 && y >= miny2 && y <= maxy2 )
    ok2 = 1;
  if ( ok1 && ok2 )
  {
    /* intersection point falls within the segments */
    *x0 = x;
    *y0 = y;
    return 1;
  }
  return 0;
}
