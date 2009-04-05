/*

 gg_transform.c -- Gaia PROJ.4 wrapping

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

#include <stdio.h>
#include <string.h>

#if OMIT_PROJ == 0  /* including PROJ.4 */
#include <proj_api.h>
#endif

#include <spatialite/sqlite3ext.h>
#include <spatialite/gaiageo.h>

GAIAGEO_DECLARE void
gaiaShiftCoords( gaiaGeomCollPtr geom, double shift_x, double shift_y )
{
  /* returns a geometry that is the old old geometry with required shifting applied to coordinates */
  int ib;
  int iv;
  double x;
  double y;
  gaiaPointPtr point;
  gaiaPolygonPtr polyg;
  gaiaLinestringPtr line;
  gaiaRingPtr ring;
  if ( !geom )
    return;
  point = geom->FirstPoint;
  while ( point )
  {
    /* shifting POINTs */
    point->X += shift_x;
    point->Y += shift_y;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* shifting LINESTRINGs */
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      x += shift_x;
      y += shift_y;
      gaiaSetPoint( line->Coords, iv, x, y );
    }
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* shifting POLYGONs */
    ring = polyg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* shifting the EXTERIOR RING */
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      x += shift_x;
      y += shift_y;
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      /* shifting the INTERIOR RINGs */
      ring = polyg->Interiors + ib;
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        gaiaGetPoint( ring->Coords, iv, &x, &y );
        x += shift_x;
        y += shift_y;
        gaiaSetPoint( ring->Coords, iv, x, y );
      }
    }
    polyg = polyg->Next;
  }
  gaiaMbrGeometry( geom );
}

GAIAGEO_DECLARE void
gaiaScaleCoords( gaiaGeomCollPtr geom, double scale_x, double scale_y )
{
  /* returns a geometry that is the old old geometry with required scaling applied to coordinates */
  int ib;
  int iv;
  double x;
  double y;
  gaiaPointPtr point;
  gaiaPolygonPtr polyg;
  gaiaLinestringPtr line;
  gaiaRingPtr ring;
  if ( !geom )
    return;
  point = geom->FirstPoint;
  while ( point )
  {
    /* scaling POINTs */
    point->X *= scale_x;
    point->Y *= scale_y;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* scaling LINESTRINGs */
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      x *= scale_x;
      y *= scale_y;
      gaiaSetPoint( line->Coords, iv, x, y );
    }
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* scaling POLYGONs */
    ring = polyg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* scaling the EXTERIOR RING */
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      x *= scale_x;
      y *= scale_y;
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      /* scaling the INTERIOR RINGs */
      ring = polyg->Interiors + ib;
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        gaiaGetPoint( ring->Coords, iv, &x, &y );
        x *= scale_x;
        y *= scale_y;
        gaiaSetPoint( ring->Coords, iv, x, y );
      }
    }
    polyg = polyg->Next;
  }
  gaiaMbrGeometry( geom );
}

GAIAGEO_DECLARE void
gaiaRotateCoords( gaiaGeomCollPtr geom, double angle )
{
  /* returns a geometry that is the old old geometry with required rotation applied to coordinates */
  int ib;
  int iv;
  double x;
  double y;
  double nx;
  double ny;
  double rad = angle * 0.0174532925199432958;
  double cosine = cos( rad );
  double sine = sin( rad );
  gaiaPointPtr point;
  gaiaPolygonPtr polyg;
  gaiaLinestringPtr line;
  gaiaRingPtr ring;
  if ( !geom )
    return;
  point = geom->FirstPoint;
  while ( point )
  {
    /* shifting POINTs */
    x = point->X;
    y = point->Y;
    point->X = ( x * cosine ) + ( y * sine );
    point->Y = ( y * cosine ) - ( x * sine );
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* rotating LINESTRINGs */
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      nx = ( x * cosine ) + ( y * sine );
      ny = ( y * cosine ) - ( x * sine );
      gaiaSetPoint( line->Coords, iv, nx, ny );
    }
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* rotating POLYGONs */
    ring = polyg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* rotating the EXTERIOR RING */
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      nx = ( x * cosine ) + ( y * sine );
      ny = ( y * cosine ) - ( x * sine );
      gaiaSetPoint( ring->Coords, iv, nx, ny );
    }
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      /* rotating the INTERIOR RINGs */
      ring = polyg->Interiors + ib;
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        gaiaGetPoint( ring->Coords, iv, &x, &y );
        nx = ( x * cosine ) + ( y * sine );
        ny = ( y * cosine ) - ( x * sine );
        gaiaSetPoint( ring->Coords, iv, nx, ny );
      }
    }
    polyg = polyg->Next;
  }
  gaiaMbrGeometry( geom );
}

GAIAGEO_DECLARE void
gaiaReflectCoords( gaiaGeomCollPtr geom, int x_axis, int y_axis )
{
  /* returns a geometry that is the old old geometry with required reflection applied to coordinates */
  int ib;
  int iv;
  double x;
  double y;
  gaiaPointPtr point;
  gaiaPolygonPtr polyg;
  gaiaLinestringPtr line;
  gaiaRingPtr ring;
  if ( !geom )
    return;
  point = geom->FirstPoint;
  while ( point )
  {
    /* reflecting POINTs */
    if ( x_axis )
      point->X *= -1.0;
    if ( y_axis )
      point->Y *= -1.0;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* reflecting LINESTRINGs */
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      if ( x_axis )
        x *= -1.0;
      if ( y_axis )
        y *= -1.0;
      gaiaSetPoint( line->Coords, iv, x, y );
    }
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* reflecting POLYGONs */
    ring = polyg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* reflecting the EXTERIOR RING */
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      if ( x_axis )
        x *= -1.0;
      if ( y_axis )
        y *= -1.0;
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      /* reflecting the INTERIOR RINGs */
      ring = polyg->Interiors + ib;
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        gaiaGetPoint( ring->Coords, iv, &x, &y );
        if ( x_axis )
          x *= -1.0;
        if ( y_axis )
          y *= -1.0;
        gaiaSetPoint( ring->Coords, iv, x, y );
      }
    }
    polyg = polyg->Next;
  }
  gaiaMbrGeometry( geom );
}

GAIAGEO_DECLARE void
gaiaSwapCoords( gaiaGeomCollPtr geom )
{
  /* returns a geometry that is the old old geometry with swapped x- and y-coordinates */
  int ib;
  int iv;
  double x;
  double y;
  double sv;
  gaiaPointPtr point;
  gaiaPolygonPtr polyg;
  gaiaLinestringPtr line;
  gaiaRingPtr ring;
  if ( !geom )
    return;
  point = geom->FirstPoint;
  while ( point )
  {
    /* swapping POINTs */
    sv = point->X;
    point->X = point->Y;
    point->Y = sv;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* swapping LINESTRINGs */
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      sv = x;
      x = y;
      y = sv;
      gaiaSetPoint( line->Coords, iv, x, y );
    }
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* swapping POLYGONs */
    ring = polyg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* shifting the EXTERIOR RING */
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      sv = x;
      x = y;
      y = sv;
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    for ( ib = 0; ib < polyg->NumInteriors; ib++ )
    {
      /* swapping the INTERIOR RINGs */
      ring = polyg->Interiors + ib;
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        gaiaGetPoint( ring->Coords, iv, &x, &y );
        sv = x;
        x = y;
        y = sv;
        gaiaSetPoint( ring->Coords, iv, x, y );
      }
    }
    polyg = polyg->Next;
  }
  gaiaMbrGeometry( geom );
}

#if OMIT_PROJ == 0  /* including PROJ.4 */

static int
gaiaIsLongLat( char *str )
{
  /* checks if we have to do with ANGLES if +proj=longlat is defined */
  if ( strstr( str, "+proj=longlat" ) != NULL )
    return 1;
  return 0;
}

GAIAGEO_DECLARE double
gaiaRadsToDegs( double rads )
{
  /* converts an ANGLE from radians to degrees */
  return rads * RAD_TO_DEG;
}

GAIAGEO_DECLARE double
gaiaDegsToRads( double degs )
{
  /* converts an ANGLE from degrees to radians */
  return degs * DEG_TO_RAD;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTransform( gaiaGeomCollPtr org, char *proj_from, char *proj_to )
{
  /* creates a new GEOMETRY reprojecting coordinates from the original one */
  int ib;
  int cnt;
  int i;
  double *xx;
  double *yy;
  double *zz;
  double x;
  double y;
  int error = 0;
  int from_angle;
  int to_angle;
  gaiaPointPtr pt;
  gaiaLinestringPtr ln;
  gaiaLinestringPtr dst_ln;
  gaiaPolygonPtr pg;
  gaiaPolygonPtr dst_pg;
  gaiaRingPtr rng;
  gaiaRingPtr dst_rng;
  gaiaGeomCollPtr dst = gaiaAllocGeomColl();
  projPJ from_cs;
  projPJ to_cs;
  /* setting up projection parameters */
  from_angle = gaiaIsLongLat( proj_from );
  to_angle = gaiaIsLongLat( proj_to );
  from_cs = pj_init_plus( proj_from );
  if ( !from_cs )
    return dst;
  to_cs = pj_init_plus( proj_to );
  if ( !to_cs )
    return dst;
  cnt = 0;
  pt = org->FirstPoint;
  while ( pt )
  {
    /* counting POINTs */
    cnt++;
    pt = pt->Next;
  }
  if ( cnt )
  {
    /* reprojecting POINTs */
    xx = malloc( sizeof( double ) * cnt );
    yy = malloc( sizeof( double ) * cnt );
    zz = malloc( sizeof( double ) * cnt );
    i = 0;
    pt = org->FirstPoint;
    while ( pt )
    {
      /* inserting points to be converted in temporary arrays */
      if ( from_angle )
      {
        xx[i] = gaiaDegsToRads( pt->X );
        yy[i] = gaiaDegsToRads( pt->Y );
      }
      else
      {
        xx[i] = pt->X;
        yy[i] = pt->Y;
      }
      zz[i] = 0.0;
      i++;
      pt = pt->Next;
    }
    /* applying reprojection        */
    if ( pj_transform( from_cs, to_cs, cnt, 0, xx, yy, zz ) == 0 )
    {
      /* inserting the reprojected POINTs in the new GEOMETRY */
      for ( i = 0; i < cnt; i++ )
      {
        if ( to_angle )
        {
          x = gaiaRadsToDegs( xx[i] );
          y = gaiaRadsToDegs( yy[i] );
        }
        else
        {
          x = xx[i];
          y = yy[i];
        }
        gaiaAddPointToGeomColl( dst, x, y );
      }
    }
    else
      error = 1;
    free( xx );
    free( yy );
    free( zz );
  }
  if ( error )
    goto stop;
  ln = org->FirstLinestring;
  while ( ln )
  {
    /* reprojecting LINESTRINGs */
    cnt = ln->Points;
    xx = malloc( sizeof( double ) * cnt );
    yy = malloc( sizeof( double ) * cnt );
    zz = malloc( sizeof( double ) * cnt );
    for ( i = 0; i < cnt; i++ )
    {
      /* inserting points to be converted in temporary arrays */
      gaiaGetPoint( ln->Coords, i, &x, &y );
      if ( from_angle )
      {
        xx[i] = gaiaDegsToRads( x );
        yy[i] = gaiaDegsToRads( y );
      }
      else
      {
        xx[i] = x;
        yy[i] = y;
      }
      zz[i] = 0.0;
    }
    /* applying reprojection        */
    if ( pj_transform( from_cs, to_cs, cnt, 0, xx, yy, zz ) == 0 )
    {
      /* inserting the reprojected LINESTRING in the new GEOMETRY */
      dst_ln = gaiaAddLinestringToGeomColl( dst, cnt );
      for ( i = 0; i < cnt; i++ )
      {
        /* setting LINESTRING points */
        if ( to_angle )
        {
          x = gaiaRadsToDegs( xx[i] );
          y = gaiaRadsToDegs( yy[i] );
        }
        else
        {
          x = xx[i];
          y = yy[i];
        }
        gaiaSetPoint( dst_ln->Coords, i, x, y );
      }
    }
    else
      error = 1;
    free( xx );
    free( yy );
    free( zz );
    if ( error )
      goto stop;
    ln = ln->Next;
  }
  pg = org->FirstPolygon;
  while ( pg )
  {
    /* reprojecting POLYGONs */
    rng = pg->Exterior;
    cnt = rng->Points;
    dst_pg = gaiaAddPolygonToGeomColl( dst, cnt, pg->NumInteriors );
    xx = malloc( sizeof( double ) * cnt );
    yy = malloc( sizeof( double ) * cnt );
    zz = malloc( sizeof( double ) * cnt );
    for ( i = 0; i < cnt; i++ )
    {
      /* inserting points to be converted in temporary arrays [EXTERIOR RING] */
      gaiaGetPoint( rng->Coords, i, &x, &y );
      if ( from_angle )
      {
        xx[i] = gaiaDegsToRads( x );
        yy[i] = gaiaDegsToRads( y );
      }
      else
      {
        xx[i] = x;
        yy[i] = y;
      }
      zz[i] = 0.0;
    }
    /* applying reprojection        */
    if ( pj_transform( from_cs, to_cs, cnt, 0, xx, yy, zz ) == 0 )
    {
      /* inserting the reprojected POLYGON in the new GEOMETRY */
      dst_rng = dst_pg->Exterior;
      for ( i = 0; i < cnt; i++ )
      {
        /* setting EXTERIOR RING points */
        if ( to_angle )
        {
          x = gaiaRadsToDegs( xx[i] );
          y = gaiaRadsToDegs( yy[i] );
        }
        else
        {
          x = xx[i];
          y = yy[i];
        }
        gaiaSetPoint( dst_rng->Coords, i, x, y );
      }
    }
    else
      error = 1;
    free( xx );
    free( yy );
    free( zz );
    if ( error )
      goto stop;
    for ( ib = 0; ib < pg->NumInteriors; ib++ )
    {
      /* processing INTERIOR RINGS */
      rng = pg->Interiors + ib;
      cnt = rng->Points;
      xx = malloc( sizeof( double ) * cnt );
      yy = malloc( sizeof( double ) * cnt );
      zz = malloc( sizeof( double ) * cnt );
      for ( i = 0; i < cnt; i++ )
      {
        /* inserting points to be converted in temporary arrays [INTERIOR RING] */
        gaiaGetPoint( rng->Coords, i, &x, &y );
        if ( from_angle )
        {
          xx[i] = gaiaDegsToRads( x );
          yy[i] = gaiaDegsToRads( y );
        }
        else
        {
          xx[i] = x;
          yy[i] = y;
        }
        zz[i] = 0.0;
      }
      /* applying reprojection        */
      if ( pj_transform( from_cs, to_cs, cnt, 0, xx, yy, zz ) == 0 )
      {
        /* inserting the reprojected POLYGON in the new GEOMETRY */
        dst_rng = dst_pg->Interiors + ib;
        dst_rng->Points = cnt;
        dst_rng->Coords =
          malloc( sizeof( double ) * ( dst_rng->Points * 2 ) );
        for ( i = 0; i < cnt; i++ )
        {
          /* setting INTERIOR RING points */
          if ( to_angle )
          {
            x = gaiaRadsToDegs( xx[i] );
            y = gaiaRadsToDegs( yy[i] );
          }
          else
          {
            x = xx[i];
            y = yy[i];
          }
          gaiaSetPoint( dst_rng->Coords, i, x, y );
        }
      }
      else
        error = 1;
      free( xx );
      free( yy );
      free( zz );
      if ( error )
        goto stop;
    }
    pg = pg->Next;
  }
  /* destroying the PROJ4 params */
stop:
  pj_free( from_cs );
  pj_free( to_cs );
  if ( error )
  {
    /* some error occurred */
    gaiaPointPtr pP;
    gaiaPointPtr pPn;
    gaiaLinestringPtr pL;
    gaiaLinestringPtr pLn;
    gaiaPolygonPtr pA;
    gaiaPolygonPtr pAn;
    pP = dst->FirstPoint;
    while ( pP != NULL )
    {
      pPn = pP->Next;
      gaiaFreePoint( pP );
      pP = pPn;
    }
    pL = dst->FirstLinestring;
    while ( pL != NULL )
    {
      pLn = pL->Next;
      gaiaFreeLinestring( pL );
      pL = pLn;
    }
    pA = dst->FirstPolygon;
    while ( pA != NULL )
    {
      pAn = pA->Next;
      gaiaFreePolygon( pA );
      pA = pAn;
    }
    dst->FirstPoint = NULL;
    dst->LastPoint = NULL;
    dst->FirstLinestring = NULL;
    dst->LastLinestring = NULL;
    dst->FirstPolygon = NULL;
    dst->LastPolygon = NULL;
  }
  return dst;
}

#endif /* end including PROJ.4 */
