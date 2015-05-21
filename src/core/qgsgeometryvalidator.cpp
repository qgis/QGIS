/***************************************************************************
  qgsgeometryvalidator.cpp - geometry validation thread
  -------------------------------------------------------------------
Date                 : 03.01.2012
Copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsgeometryvalidator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"

#include <QSettings>

QgsGeometryValidator::QgsGeometryValidator( const QgsGeometry *g, QList<QgsGeometry::Error> *errors )
    : QThread()
    , mErrors( errors )
    , mStop( false )
    , mErrorCount( 0 )
{
  Q_ASSERT( g );
  if ( g )
    mG = *g;
}

QgsGeometryValidator::~QgsGeometryValidator()
{
  stop();
  wait();
}

void QgsGeometryValidator::stop()
{
  mStop = true;
}

void QgsGeometryValidator::checkRingIntersections(
  int p0, int i0, const QgsPolyline &ring0,
  int p1, int i1, const QgsPolyline &ring1 )
{
  for ( int i = 0; !mStop && i < ring0.size() - 1; i++ )
  {
    QgsVector v = ring0[i+1] - ring0[i];

    for ( int j = 0; !mStop && j < ring1.size() - 1; j++ )
    {
      QgsVector w = ring1[j+1] - ring1[j];

      QgsPoint s;
      if ( intersectLines( ring0[i], v, ring1[j], w, s ) )
      {
        double d = -distLine2Point( ring0[i], v.perpVector(), s );

        if ( d >= 0 && d <= v.length() )
        {
          d = -distLine2Point( ring1[j], w.perpVector(), s );
          if ( d > 0 && d < w.length() )
          {
            QString msg = QObject::tr( "segment %1 of ring %2 of polygon %3 intersects segment %4 of ring %5 of polygon %6 at %7" )
                          .arg( i0 ).arg( i ).arg( p0 )
                          .arg( i1 ).arg( j ).arg( p1 )
                          .arg( s.toString() );
            QgsDebugMsg( msg );
            emit errorFound( QgsGeometry::Error( msg, s ) );
            mErrorCount++;
          }
        }
      }
    }
  }
}

void QgsGeometryValidator::validatePolyline( int i, QgsPolyline line, bool ring )
{
  if ( ring )
  {
    if ( line.size() < 4 )
    {
      QString msg = QObject::tr( "ring %1 with less than four points" ).arg( i );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }

    if ( line[0] != line[ line.size()-1 ] )
    {
      QString msg = QObject::tr( "ring %1 not closed" ).arg( i );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }
  }
  else if ( line.size() < 2 )
  {
    QString msg = QObject::tr( "line %1 with less than two points" ).arg( i );
    QgsDebugMsg( msg );
    emit errorFound( QgsGeometry::Error( msg ) );
    mErrorCount++;
    return;
  }

  int j = 0;
  while ( j < line.size() - 1 )
  {
    int n = 0;
    while ( j < line.size() - 1 && line[j] == line[j+1] )
    {
      line.remove( j );
      n++;
    }

    if ( n > 0 )
    {
      QString msg = QObject::tr( "line %1 contains %n duplicate node(s) at %2", "number of duplicate nodes", n ).arg( i ).arg( j );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg, line[j] ) );
      mErrorCount++;
    }

    j++;
  }

  for ( j = 0; !mStop && j < line.size() - 3; j++ )
  {
    QgsVector v = line[j+1] - line[j];
    double vl = v.length();

    int n = ( j == 0 && ring ) ? line.size() - 2 : line.size() - 1;

    for ( int k = j + 2; !mStop && k < n; k++ )
    {
      QgsVector w = line[k+1] - line[k];

      QgsPoint s;
      if ( !intersectLines( line[j], v, line[k], w, s ) )
        continue;

      double d = -distLine2Point( line[j], v.perpVector(), s );
      if ( d < 0 || d > vl )
        continue;

      d = -distLine2Point( line[k], w.perpVector(), s );
      if ( d <= 0 || d >= w.length() )
        continue;

      QString msg = QObject::tr( "segments %1 and %2 of line %3 intersect at %4" ).arg( j ).arg( k ).arg( i ).arg( s.toString() );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg, s ) );
      mErrorCount++;
    }
  }
}

void QgsGeometryValidator::validatePolygon( int idx, const QgsPolygon &polygon )
{
  // check if holes are inside polygon
  for ( int i = 1; !mStop && i < polygon.size(); i++ )
  {
    if ( !ringInRing( polygon[i], polygon[0] ) )
    {
      QString msg = QObject::tr( "ring %1 of polygon %2 not in exterior ring" ).arg( i ).arg( idx );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
    }
  }

  // check holes for intersections
  for ( int i = 1; !mStop && i < polygon.size(); i++ )
  {
    for ( int j = i + 1; !mStop && j < polygon.size(); j++ )
    {
      checkRingIntersections( idx, i, polygon[i], idx, j, polygon[j] );
    }
  }

  // check if rings are self-intersecting
  for ( int i = 0; !mStop && i < polygon.size(); i++ )
  {
    validatePolyline( i, polygon[i], true );
  }
}

void QgsGeometryValidator::run()
{
  mErrorCount = 0;
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
    ( (GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR>=3) || GEOS_VERSION_MAJOR>3)
  QSettings settings;
  if ( settings.value( "/qgis/digitizing/validate_geometries", 1 ).toInt() == 2 )
  {
    char *r = 0;
    const GEOSGeometry *g0 = mG.asGeos();
    GEOSContextHandle_t handle = QgsGeometry::getGEOSHandler();
    if ( !g0 )
    {
      emit errorFound( QgsGeometry::Error( QObject::tr( "GEOS error:could not produce geometry for GEOS (check log window)" ) ) );
    }
    else
    {
      GEOSGeometry *g1 = 0;
      if ( GEOSisValidDetail_r( handle, g0, GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE, &r, &g1 ) != 1 )
      {
        if ( g1 )
        {
          const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( handle, g1 );

          unsigned int n;
          if ( GEOSCoordSeq_getSize_r( handle, cs, &n ) && n == 1 )
          {
            double x, y;
            GEOSCoordSeq_getX_r( handle, cs, 0, &x );
            GEOSCoordSeq_getY_r( handle, cs, 0, &y );
            emit errorFound( QgsGeometry::Error( QObject::tr( "GEOS error:%1" ).arg( r ), QgsPoint( x, y ) ) );
            mErrorCount++;
          }

          GEOSGeom_destroy_r( handle, g1 );
        }
        else
        {
          emit errorFound( QgsGeometry::Error( QObject::tr( "GEOS error:%1" ).arg( r ) ) );
          mErrorCount++;
        }

        GEOSFree_r( handle, r );
      }
    }

    return;
  }
#endif

  QgsDebugMsg( "validation thread started." );

  switch ( mG.wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      break;

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
      validatePolyline( 0, mG.asPolyline() );
      break;

    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
    {
      QgsMultiPolyline mp = mG.asMultiPolyline();
      for ( int i = 0; !mStop && i < mp.size(); i++ )
        validatePolyline( i, mp[i] );
    }
    break;

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    {
      validatePolygon( 0, mG.asPolygon() );
    }
    break;

    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
    {
      QgsMultiPolygon mp = mG.asMultiPolygon();
      for ( int i = 0; !mStop && i < mp.size(); i++ )
      {
        validatePolygon( i, mp[i] );
      }

      for ( int i = 0; !mStop && i < mp.size(); i++ )
      {
        if ( mp[i].isEmpty() )
        {
          emit errorFound( QgsGeometry::Error( QObject::tr( "polygon %1 has no rings" ).arg( i ) ) );
          mErrorCount++;
          continue;
        }

        for ( int j = i + 1;  !mStop && j < mp.size(); j++ )
        {
          if ( mp[j].isEmpty() )
            continue;

          if ( ringInRing( mp[i][0], mp[j][0] ) )
          {
            emit errorFound( QgsGeometry::Error( QObject::tr( "polygon %1 inside polygon %2" ).arg( i ).arg( j ) ) );
            mErrorCount++;
          }
          else if ( ringInRing( mp[j][0], mp[i][0] ) )
          {
            emit errorFound( QgsGeometry::Error( QObject::tr( "polygon %1 inside polygon %2" ).arg( j ).arg( i ) ) );
            mErrorCount++;
          }
          else
          {
            checkRingIntersections( i, 0, mp[i][0], j, 0, mp[j][0] );
          }
        }
      }
    }
    break;

    case QGis::WKBNoGeometry:
    case QGis::WKBUnknown:
      QgsDebugMsg( QObject::tr( "Unknown geometry type" ) );
      emit errorFound( QgsGeometry::Error( QObject::tr( "Unknown geometry type %1" ).arg( mG.wkbType() ) ) );
      mErrorCount++;
      break;
  }

  QgsDebugMsg( "validation finished." );

  if ( mStop )
  {
    emit errorFound( QObject::tr( "Geometry validation was aborted." ) );
  }
  else if ( mErrorCount > 0 )
  {
    emit errorFound( QObject::tr( "Geometry has %1 errors." ).arg( mErrorCount ) );
  }
#if 0
  else
  {
    emit errorFound( QObject::tr( "Geometry is valid." ) );
  }
#endif
}

void QgsGeometryValidator::addError( QgsGeometry::Error e )
{
  if ( mErrors )
    *mErrors << e;
}

void QgsGeometryValidator::validateGeometry( const QgsGeometry *g, QList<QgsGeometry::Error> &errors )
{
  QgsGeometryValidator *gv = new QgsGeometryValidator( g, &errors );
  connect( gv, SIGNAL( errorFound( QgsGeometry::Error ) ), gv, SLOT( addError( QgsGeometry::Error ) ) );
  gv->run();
  gv->wait();
}

/**************************************************************************************
 *                                                                                    *
 * Helper MakeValid function from 'lwgeom_geos_clean.c' - potsgis repo                *
 * https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwgeom_geos_clean.c    *
 *                                                                                    *
/**************************************************************************************
 *                                                                                    *
 * PostGIS - Spatial Types for PostgreSQL                                             *
 * http://postgis.net                                                                 *
 *                                                                                    *
 * Copyright 2009-2010 Sandro Santilli <strk@keybit.net>                              *
 *                                                                                    *
 * This is free software; you can redistribute and/or modify it under                 *
 * the terms of the GNU General Public Licence. See the COPYING file.                 *
 *                                                                                    *
 **************************************************************************************/

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef lwrealloc
#define lwrealloc realloc
#endif
#ifndef lwalloc
#define lwalloc malloc
#endif
#ifndef lwfree
#define lwfree free
#endif

/* Return Nth vertex in GEOSGeometry as a POINT.
 * May return NULL if the geometry has NO vertexex.
 */
static GEOSGeometry* LWGEOM_GEOS_getPointN( const GEOSGeometry* g_in, uint32_t n )
{
  uint32_t dims;
  const GEOSCoordSequence* seq_in;
  GEOSCoordSeq seq_out;
  double val;
  uint32_t sz;
  int gn;
  GEOSGeometry* ret;

  switch ( GEOSGeomTypeId( g_in ) )
  {
    case GEOS_MULTIPOINT:
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION:
    {
      for ( gn = 0; gn < GEOSGetNumGeometries( g_in ); ++gn )
      {
        const GEOSGeometry* g = GEOSGetGeometryN( g_in, gn );
        ret = LWGEOM_GEOS_getPointN( g, n );
        if ( ret ) return ret;
      }
      break;
    }
    case GEOS_POLYGON:
    {
      ret = LWGEOM_GEOS_getPointN( GEOSGetExteriorRing( g_in ), n );
      if ( ret ) return ret;

      for ( gn = 0; gn < GEOSGetNumInteriorRings( g_in ); ++gn )
      {
        const GEOSGeometry* g = GEOSGetInteriorRingN( g_in, gn );
        ret = LWGEOM_GEOS_getPointN( g, n );
        if ( ret ) return ret;
      }
      break;
    }
    case GEOS_POINT:
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
      break;
  }

  seq_in = GEOSGeom_getCoordSeq( g_in );
  if ( !seq_in ) return NULL;
  if ( !GEOSCoordSeq_getSize( seq_in, &sz ) ) return NULL;
  if ( !sz ) return NULL;
  if ( !GEOSCoordSeq_getDimensions( seq_in, &dims ) ) return NULL;

  seq_out = GEOSCoordSeq_create( 1, dims );
  if ( !seq_out ) return NULL;

  if ( !GEOSCoordSeq_getX( seq_in, n, &val ) ) return NULL;
  if ( !GEOSCoordSeq_setX( seq_out, n, val ) ) return NULL;
  if ( !GEOSCoordSeq_getY( seq_in, n, &val ) ) return NULL;
  if ( !GEOSCoordSeq_setY( seq_out, n, val ) ) return NULL;
  if ( dims > 2 )
  {
    if ( !GEOSCoordSeq_getZ( seq_in, n, &val ) ) return NULL;
    if ( !GEOSCoordSeq_setZ( seq_out, n, val ) ) return NULL;
  }

  return GEOSGeom_createPoint( seq_out );
}

/*
 * Fully node given linework
 */
static GEOSGeometry* LWGEOM_GEOS_nodeLines( const GEOSGeometry* lines )
{
  GEOSGeometry* noded;
  GEOSGeometry* point;

  /*
   * Union with first geometry point, obtaining full noding
   * and dissolving of duplicated repeated points
   *
   * TODO: substitute this with UnaryUnion?
   */
  point = LWGEOM_GEOS_getPointN( lines, 0 );
  if ( !point ) return NULL;

  // LWDEBUGF(3, "Boundary point: %s", lwgeom_to_ewkt(GEOS2LWGEOM(point, 0)));

  noded = GEOSUnion( lines, point );

  if ( NULL == noded )
  {
    GEOSGeom_destroy( point );
    return NULL;
  }
  GEOSGeom_destroy( point );

  // LWDEBUGF(3, "LWGEOM_GEOS_nodeLines: in[%s] out[%s]", lwgeom_to_ewkt(GEOS2LWGEOM(lines, 0)), lwgeom_to_ewkt(GEOS2LWGEOM(noded, 0)));

  return noded;
}

/* 
 * Make valid the specified linestring.
 */
static GEOSGeometry* LWGEOM_GEOS_makeValidLine( const GEOSGeometry* gin )
{
  GEOSGeometry* gout = LWGEOM_GEOS_nodeLines( gin );
  return gout;
}

/* 
 * Make valid the specified multi-linestring.
 */
static GEOSGeometry* LWGEOM_GEOS_makeValidMultiLine( const GEOSGeometry* gin )
{
  GEOSGeometry** lines;
  GEOSGeometry** points;
  GEOSGeometry* mline_out = 0;
  GEOSGeometry* mpoint_out = 0;
  GEOSGeometry* gout = 0;
  uint32_t nlines = 0, nlines_alloc;
  uint32_t npoints = 0;
  uint32_t ngeoms = 0, nsubgeoms;
  uint32_t i, j;

  ngeoms = GEOSGetNumGeometries( gin );
  nlines_alloc = ngeoms;
  lines  = ( GEOSGeometry** )lwalloc( sizeof( GEOSGeometry* ) * nlines_alloc );
  points = ( GEOSGeometry** )lwalloc( sizeof( GEOSGeometry* ) * ngeoms );

  for ( i = 0; i < ngeoms; ++i )
  {
    const GEOSGeometry* g = GEOSGetGeometryN( gin, i );
    GEOSGeometry* vg;
    vg = LWGEOM_GEOS_makeValidLine( g );

    if ( GEOSisEmpty( vg ) )
    {
      /* we don't care about this one */
      GEOSGeom_destroy( vg );
      continue;
    }
    if ( GEOSGeomTypeId( vg ) == GEOS_POINT )
    {
      points[ npoints++ ] = vg;
    }
    else if ( GEOSGeomTypeId( vg ) == GEOS_LINESTRING )
    {
      lines[ nlines++ ] = vg;
    }
    else if ( GEOSGeomTypeId( vg ) == GEOS_MULTILINESTRING )
    {
      nsubgeoms = GEOSGetNumGeometries( vg );
      nlines_alloc += nsubgeoms;
      lines = ( GEOSGeometry** )lwrealloc( lines, sizeof( GEOSGeometry* ) * nlines_alloc );

      for ( j = 0; j < nsubgeoms; ++j )
      {
        const GEOSGeometry* gc = GEOSGetGeometryN( vg, j );

        /* NOTE: ownership of the cloned geoms will be taken by final collection */
        lines[nlines++] = GEOSGeom_clone( gc );
      }
    }
    else
    {
      /* NOTE: return from GEOSGeomType will leak but we really don't expect this to happen */
      QgsDebugMsg( QString( "unexpected geom type returned by GEOS_makeValid: %1" ).arg( GEOSGeomType( vg ) ) );
    }
  }

  if ( npoints )
  {
    if ( npoints > 1 )
    {
      mpoint_out = GEOSGeom_createCollection( GEOS_MULTIPOINT, points, npoints );
    }
    else
    {
      mpoint_out = points[0];
    }
  }
  if ( nlines )
  {
    if ( nlines > 1 )
    {
      mline_out = GEOSGeom_createCollection( GEOS_MULTILINESTRING, lines, nlines );
    }
    else
    {
      mline_out = lines[0];
    }
  }
  lwfree( lines );

  if ( mline_out && mpoint_out )
  {
    points[0] = mline_out;
    points[1] = mpoint_out;
    gout = GEOSGeom_createCollection( GEOS_GEOMETRYCOLLECTION, points, 2 );
  }
  else if ( mline_out )
  {
    gout = mline_out;
  }
  else if ( mpoint_out )
  {
    gout = mpoint_out;
  }
  lwfree( points );

  return gout;
}

/* Make valid the specified polygon.
 * The original LWGEOM_GEOS_makeValidPolygon function implements other code 
 * but GEOSBuffer seems to work fine.
 */
static GEOSGeometry* LWGEOM_GEOS_makeValidPolygon( const GEOSGeometry* gin )
{
  GEOSGeometry* gout = GEOSBuffer( gin, 0, 0 );
  return gout;
}

static GEOSGeometry* LWGEOM_GEOS_makeValid( const GEOSGeometry* );

/* 
 * Make valid the specified geometry collection.
 */
static GEOSGeometry* LWGEOM_GEOS_makeValidCollection( const GEOSGeometry* gin )
{
  unsigned int nvgeoms;
  GEOSGeometry **vgeoms;
  GEOSGeom gout;
  unsigned int i;

  nvgeoms = GEOSGetNumGeometries( gin );
  if ( nvgeoms == -1 ) 
  {
    // lwerror("GEOSGetNumGeometries: %s", lwgeom_geos_errmsg);
    return 0;
  }

  vgeoms = ( GEOSGeometry** )lwalloc( sizeof( GEOSGeometry* ) * nvgeoms );
  if ( !vgeoms ) 
  {
    // lwerror("LWGEOM_GEOS_makeValidCollection: out of memory");
    return 0;
  }

  for ( i = 0; i < nvgeoms; ++i ) 
  {
    vgeoms[i] = LWGEOM_GEOS_makeValid( GEOSGetGeometryN( gin, i ) );

    if ( !vgeoms[i] ) 
    {
      while ( i-- ) GEOSGeom_destroy( vgeoms[i] );
      lwfree( vgeoms );

      /* we expect lwerror being called already by makeValid */
      return NULL;
    }
  }

  /* Collect areas and lines (if any line) */
  gout = GEOSGeom_createCollection( GEOS_GEOMETRYCOLLECTION, vgeoms, nvgeoms );

  if ( !gout ) /* an exception again */
  {
    /* cleanup and throw */
    for ( i = 0; i < nvgeoms; ++i ) GEOSGeom_destroy( vgeoms[i] );
    lwfree( vgeoms );

    // lwerror("GEOSGeom_createCollection() threw an error: %s", lwgeom_geos_errmsg);
    return NULL;
  }
  lwfree( vgeoms );

  return gout;
}

/* 
 * Make valid the specified geometry.
 */
static GEOSGeometry* LWGEOM_GEOS_makeValid( const GEOSGeometry* gin )
{
  GEOSGeometry* gout;
  char ret_char;

  /*
   * Step 2: return what we got so far if already valid
   */
  ret_char = GEOSisValid( gin );

  if ( ret_char == 2 )
  {
    /* I don't think should ever happen */
    // lwerror("GEOSisValid(): %s", lwgeom_geos_errmsg);
    return NULL;
  }
  else if ( ret_char )
  {
    // LWDEBUGF(3, "Geometry [%s] is valid. ", lwgeom_to_ewkt(GEOS2LWGEOM(gin, 0)));

    /* It's valid at this step, return what we have */
    return GEOSGeom_clone( gin );
  }

  // LWDEBUGF(3, "Geometry [%s] is still not valid: %s. Will try to clean up further.", lwgeom_to_ewkt(GEOS2LWGEOM(gin, 0)), lwgeom_geos_errmsg);

  /*
   * Step 3 : make what we got valid
   */
  switch ( GEOSGeomTypeId( gin ) )
  {
    case GEOS_MULTIPOINT:
    case GEOS_POINT:
      /* points are always valid, but we might have invalid ordinate values */
      // lwnotice("PUNTUAL geometry resulted invalid to GEOS -- dunno how to clean that up");
      return NULL;
      break;

    case GEOS_LINESTRING:
      gout = LWGEOM_GEOS_makeValidLine( gin );

      if ( !gout )  /* an exception or something */
      {
        /* cleanup and throw */
        // lwerror("%s", lwgeom_geos_errmsg);
        return NULL;
      }
      break; /* we've done */

    case GEOS_MULTILINESTRING:
      gout = LWGEOM_GEOS_makeValidMultiLine( gin );

      if ( !gout )  /* an exception or something */
      {
        /* cleanup and throw */
        // lwerror("%s", lwgeom_geos_errmsg);
        return NULL;
      }
      break; /* we've done */

    case GEOS_POLYGON:
    case GEOS_MULTIPOLYGON:
    {
      gout = LWGEOM_GEOS_makeValidPolygon( gin );

      if ( !gout )  /* an exception or something */
      {
        /* cleanup and throw */
        // lwerror("%s", lwgeom_geos_errmsg);
        return NULL;
      }
      break; /* we've done */
    }
    case GEOS_GEOMETRYCOLLECTION:
    {
      gout = LWGEOM_GEOS_makeValidCollection( gin );

      if ( !gout )  /* an exception or something */
      {
        /* cleanup and throw */
        // lwerror("%s", lwgeom_geos_errmsg);
        return NULL;
      }
      break; /* we've done */
    }
    default:
    {
      char* typname = GEOSGeomType( gin );

      // lwnotice("ST_MakeValid: doesn't support geometry type: %s", typname);
      GEOSFree(typname);
      return NULL;
      break;
    }
  }

  #if _DEBUG
  /*
   * Now check if every point of input is also found
   * in output, or abort by returning NULL
   *
   * Input geometry was lwgeom_in
   */
  {
    const int paranoia = 2;

    /* TODO: check if the result is valid */
    if (paranoia)
    {
      int loss;
      GEOSGeometry *pi, *po, *pd;

      /* TODO: handle some errors here...
       * Lack of exceptions is annoying indeed,
       * I'm getting old --strk;
       */
      pi = GEOSGeom_extractUniquePoints( gin );
      po = GEOSGeom_extractUniquePoints( gout );
      pd = GEOSDifference( pi, po ); /* input points - output points */
      GEOSGeom_destroy( pi );
      GEOSGeom_destroy( po );
      loss = !GEOSisEmpty( pd );
      GEOSGeom_destroy( pd );
      if ( loss )
      {
        // lwnotice("Vertices lost in LWGEOM_GEOS_makeValid");
        /* return NULL */
      }
    }
  }
  #endif

  return gout;
}

/***********************************************************************/

GEOSGeometry* QgsGeometryValidator::makeValidGeometry( const GEOSGeometry *g )
{
  if ( g )
  {
    GEOSGeometry* validGeos = LWGEOM_GEOS_makeValid( g );
    return validGeos;
  }
  return NULL;
}

QgsGeometry* QgsGeometryValidator::makeValidGeometry( const QgsGeometry *g )
{
  QGis::WkbType flatType = QGis::flatType( g->wkbType() );

  if ( flatType == QGis::WKBPoint || flatType == QGis::WKBUnknown || flatType == QGis::WKBNoGeometry )
  {
    return QgsGeometry::fromPoint( g->asPoint() );
  }
  else
  {
    const GEOSGeometry* geos = g->asGeos();

    if ( geos )
    {
      GEOSGeometry* validGeos = QgsGeometryValidator::makeValidGeometry( geos );

      if ( validGeos )
      {
        QgsGeometry* validGeom = new QgsGeometry;
        validGeom->fromGeos( validGeos );
        return validGeom;
      }
    }
    return NULL;
  }
}

//
// distance of point q from line through p in direction v
// return >0  => q lies left of the line
//        <0  => q lies right of the line
//
double QgsGeometryValidator::distLine2Point( QgsPoint p, QgsVector v, QgsPoint q )
{
  if ( v.length() == 0 )
  {
    throw QgsException( QObject::tr( "invalid line" ) );
  }

  return ( v.x()*( q.y() - p.y() ) - v.y()*( q.x() - p.x() ) ) / v.length();
}

bool QgsGeometryValidator::intersectLines( QgsPoint p, QgsVector v, QgsPoint q, QgsVector w, QgsPoint &s )
{
  double d = v.y() * w.x() - v.x() * w.y();

  if ( d == 0 )
    return false;

  double dx = q.x() - p.x();
  double dy = q.y() - p.y();
  double k = ( dy * w.x() - dx * w.y() ) / d;

  s = p + v * k;

  return true;
}

bool QgsGeometryValidator::pointInRing( const QgsPolyline &ring, const QgsPoint &p )
{
  bool inside = false;
  int j = ring.size() - 1;

  for ( int i = 0; !mStop && i < ring.size(); i++ )
  {
    if ( ring[i].x() == p.x() && ring[i].y() == p.y() )
      return true;

    if (( ring[i].y() < p.y() && ring[j].y() >= p.y() ) ||
        ( ring[j].y() < p.y() && ring[i].y() >= p.y() ) )
    {
      if ( ring[i].x() + ( p.y() - ring[i].y() ) / ( ring[j].y() - ring[i].y() )*( ring[j].x() - ring[i].x() ) <= p.x() )
        inside = !inside;
    }

    j = i;
  }

  return inside;
}

bool QgsGeometryValidator::ringInRing( const QgsPolyline &inside, const QgsPolyline &outside )
{
  for ( int i = 0; !mStop && i < inside.size(); i++ )
  {
    if ( !pointInRing( outside, inside[i] ) )
      return false;
  }

  return true;
}
