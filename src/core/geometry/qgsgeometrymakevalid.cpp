/***************************************************************************
  qgsgeometrymakevalid.cpp
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// The code in this file has been ported from lwgeom library (part of PostGIS).
// See lwgeom_geos_clean.c for the original code by Sandro Santilli.
//
// Ideally one day the implementation will go to GEOS library...

#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgslogger.h"

#include "qgsgeometrycollection.h"
#include "qgsmultipoint.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"

#include <memory>

#if ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR<8 )

// ------------ BuildArea stuff ---------------------------------------------------------------------

typedef struct Face_t
{
  const GEOSGeometry *geom = nullptr;
  GEOSGeometry *env = nullptr;
  double envarea;
  struct Face_t *parent; // if this face is an hole of another one, or NULL
} Face;

static Face *newFace( const GEOSGeometry *g );
static void delFace( Face *f );
static unsigned int countParens( const Face *f );
static void findFaceHoles( Face **faces, int nfaces );

static Face *newFace( const GEOSGeometry *g )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  Face *f = new Face;
  f->geom = g;
  f->env = GEOSEnvelope_r( handle, f->geom );
  GEOSArea_r( handle, f->env, &f->envarea );
  f->parent = nullptr;
  return f;
}

static unsigned int countParens( const Face *f )
{
  unsigned int pcount = 0;
  while ( f->parent )
  {
    ++pcount;
    f = f->parent;
  }
  return pcount;
}

// Destroy the face and release memory associated with it
static void delFace( Face *f )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();
  GEOSGeom_destroy_r( handle, f->env );
  delete f;
}


static int compare_by_envarea( const void *g1, const void *g2 )
{
  Face *f1 = *( Face ** )g1;
  Face *f2 = *( Face ** )g2;
  double n1 = f1->envarea;
  double n2 = f2->envarea;

  if ( n1 < n2 ) return 1;
  if ( n1 > n2 ) return -1;
  return 0;
}

// Find holes of each face
static void findFaceHoles( Face **faces, int nfaces )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  // We sort by envelope area so that we know holes are only
  // after their shells
  qsort( faces, nfaces, sizeof( Face * ), compare_by_envarea );
  for ( int i = 0; i < nfaces; ++i )
  {
    Face *f = faces[i];
    int nholes = GEOSGetNumInteriorRings_r( handle, f->geom );
    for ( int h = 0; h < nholes; ++h )
    {
      const GEOSGeometry *hole = GEOSGetInteriorRingN_r( handle, f->geom, h );
      for ( int j = i + 1; j < nfaces; ++j )
      {
        Face *f2 = faces[j];
        if ( f2->parent )
          continue; // hole already assigned
        /* TODO: can be optimized as the ring would have the
         *       same vertices, possibly in different order.
         *       maybe comparing number of points could already be
         *       useful.
         */
        if ( GEOSEquals_r( handle, GEOSGetExteriorRing_r( handle, f2->geom ), hole ) )
        {
          f2->parent = f;
          break;
        }
      }
    }
  }
}

static GEOSGeometry *collectFacesWithEvenAncestors( Face **faces, int nfaces )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  unsigned int ngeoms = 0;
  GEOSGeometry **geoms = new GEOSGeometry*[nfaces];
  for ( int i = 0; i < nfaces; ++i )
  {
    Face *f = faces[i];
    if ( countParens( f ) % 2 )
      continue; // we skip odd parents geoms
    geoms[ngeoms++] = GEOSGeom_clone_r( handle, f->geom );
  }

  GEOSGeometry *ret = GEOSGeom_createCollection_r( handle, GEOS_MULTIPOLYGON, geoms, ngeoms );
  delete [] geoms;
  return ret;
}

Q_NOWARN_UNREACHABLE_PUSH
static GEOSGeometry *LWGEOM_GEOS_buildArea( const GEOSGeometry *geom_in, QString &errorMessage )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  GEOSGeometry *tmp = nullptr;
  GEOSGeometry *shp = nullptr;
  GEOSGeometry *geos_result = nullptr;
  int srid = GEOSGetSRID_r( handle, geom_in );

  GEOSGeometry const *vgeoms[1];
  vgeoms[0] = geom_in;
  try
  {
    geos_result = GEOSPolygonize_r( handle, vgeoms, 1 );
  }
  catch ( GEOSException &e )
  {
    errorMessage = QStringLiteral( "GEOSPolygonize(): %1" ).arg( e.what() );
    return nullptr;
  }

  // We should now have a collection
#if PARANOIA_LEVEL > 0
  if ( GEOSGeometryTypeId_r( handle, geos_result ) != COLLECTIONTYPE )
  {
    GEOSGeom_destroy_r( handle, geos_result );
    errorMessage = "Unexpected return from GEOSpolygonize";
    return nullptr;
  }
#endif

  int ngeoms = GEOSGetNumGeometries_r( handle, geos_result );

  // No geometries in collection, early out
  if ( ngeoms == 0 )
  {
    GEOSSetSRID_r( handle, geos_result, srid );
    return geos_result;
  }

  // Return first geometry if we only have one in collection,
  // to avoid the unnecessary Geometry clone below.
  if ( ngeoms == 1 )
  {
    tmp = ( GEOSGeometry * )GEOSGetGeometryN_r( handle, geos_result, 0 );
    if ( ! tmp )
    {
      GEOSGeom_destroy_r( handle, geos_result );
      return nullptr;
    }
    shp = GEOSGeom_clone_r( handle, tmp );
    GEOSGeom_destroy_r( handle, geos_result ); // only safe after the clone above
    GEOSSetSRID_r( handle, shp, srid );
    return shp;
  }

  /*
   * Polygonizer returns a polygon for each face in the built topology.
   *
   * This means that for any face with holes we'll have other faces
   * representing each hole. We can imagine a parent-child relationship
   * between these faces.
   *
   * In order to maximize the number of visible rings in output we
   * only use those faces which have an even number of parents.
   *
   * Example:
   *
   *   +---------------+
   *   |     L0        |  L0 has no parents
   *   |  +---------+  |
   *   |  |   L1    |  |  L1 is an hole of L0
   *   |  |  +---+  |  |
   *   |  |  |L2 |  |  |  L2 is an hole of L1 (which is an hole of L0)
   *   |  |  |   |  |  |
   *   |  |  +---+  |  |
   *   |  +---------+  |
   *   |               |
   *   +---------------+
   *
   * See http://trac.osgeo.org/postgis/ticket/1806
   *
   */

  // Prepare face structures for later analysis
  Face **geoms = new Face*[ngeoms];
  for ( int i = 0; i < ngeoms; ++i )
    geoms[i] = newFace( GEOSGetGeometryN_r( handle, geos_result, i ) );

  // Find faces representing other faces holes
  findFaceHoles( geoms, ngeoms );

  // Build a MultiPolygon composed only by faces with an
  // even number of ancestors
  tmp = collectFacesWithEvenAncestors( geoms, ngeoms );

  // Cleanup face structures
  for ( int i = 0; i < ngeoms; ++i )
    delFace( geoms[i] );
  delete [] geoms;

  // Faces referenced memory owned by geos_result.
  // It is safe to destroy geos_result after deleting them.
  GEOSGeom_destroy_r( handle, geos_result );

  // Run a single overlay operation to dissolve shared edges
  shp = GEOSUnionCascaded_r( handle, tmp );
  if ( !shp )
  {
    GEOSGeom_destroy_r( handle, tmp );
    return nullptr;
  }

  GEOSGeom_destroy_r( handle, tmp );

  GEOSSetSRID_r( handle, shp, srid );

  return shp;
}
Q_NOWARN_UNREACHABLE_POP

// Return Nth vertex in GEOSGeometry as a POINT.
// May return NULL if the geometry has NO vertexex.
static GEOSGeometry *LWGEOM_GEOS_getPointN( const GEOSGeometry *g_in, uint32_t n )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  uint32_t dims;
  const GEOSCoordSequence *seq_in = nullptr;
  GEOSCoordSeq seq_out;
  double val;
  uint32_t sz;
  int gn;
  GEOSGeometry *ret = nullptr;

  switch ( GEOSGeomTypeId_r( handle, g_in ) )
  {
    case GEOS_MULTIPOINT:
    case GEOS_MULTILINESTRING:
    case GEOS_MULTIPOLYGON:
    case GEOS_GEOMETRYCOLLECTION:
    {
      for ( gn = 0; gn < GEOSGetNumGeometries_r( handle, g_in ); ++gn )
      {
        const GEOSGeometry *g = GEOSGetGeometryN_r( handle, g_in, gn );
        ret = LWGEOM_GEOS_getPointN( g, n );
        if ( ret ) return ret;
      }
      break;
    }

    case GEOS_POLYGON:
    {
      ret = LWGEOM_GEOS_getPointN( GEOSGetExteriorRing_r( handle, g_in ), n );
      if ( ret ) return ret;
      for ( gn = 0; gn < GEOSGetNumInteriorRings_r( handle, g_in ); ++gn )
      {
        const GEOSGeometry *g = GEOSGetInteriorRingN_r( handle, g_in, gn );
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

  seq_in = GEOSGeom_getCoordSeq_r( handle, g_in );
  if ( ! seq_in ) return nullptr;
  if ( ! GEOSCoordSeq_getSize_r( handle, seq_in, &sz ) ) return nullptr;
  if ( ! sz ) return nullptr;

  if ( ! GEOSCoordSeq_getDimensions_r( handle, seq_in, &dims ) ) return nullptr;

  seq_out = GEOSCoordSeq_create_r( handle, 1, dims );
  if ( ! seq_out ) return nullptr;

  if ( ! GEOSCoordSeq_getX_r( handle, seq_in, n, &val ) ) return nullptr;
  if ( ! GEOSCoordSeq_setX_r( handle, seq_out, n, val ) ) return nullptr;
  if ( ! GEOSCoordSeq_getY_r( handle, seq_in, n, &val ) ) return nullptr;
  if ( ! GEOSCoordSeq_setY_r( handle, seq_out, n, val ) ) return nullptr;
  if ( dims > 2 )
  {
    if ( ! GEOSCoordSeq_getZ_r( handle, seq_in, n, &val ) ) return nullptr;
    if ( ! GEOSCoordSeq_setZ_r( handle, seq_out, n, val ) ) return nullptr;
  }

  return GEOSGeom_createPoint_r( handle, seq_out );
}


static bool lwline_make_geos_friendly( QgsLineString *line );
static bool lwpoly_make_geos_friendly( QgsPolygon *poly );
static bool lwcollection_make_geos_friendly( QgsGeometryCollection *g );


// Ensure the geometry is "structurally" valid (enough for GEOS to accept it)
static bool lwgeom_make_geos_friendly( QgsAbstractGeometry *geom )
{
  QgsDebugMsgLevel( QStringLiteral( "lwgeom_make_geos_friendly enter (type %1)" ).arg( geom->wkbType() ), 3 );
  switch ( QgsWkbTypes::flatType( geom->wkbType() ) )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::MultiPoint:
      // a point is always valid
      return true;
      break;

    case QgsWkbTypes::LineString:
      // lines need at least 2 points
      return lwline_make_geos_friendly( qgsgeometry_cast<QgsLineString *>( geom ) );
      break;

    case QgsWkbTypes::Polygon:
      // polygons need all rings closed and with npoints > 3
      return lwpoly_make_geos_friendly( qgsgeometry_cast<QgsPolygon *>( geom ) );
      break;

    case QgsWkbTypes::MultiLineString:
    case QgsWkbTypes::MultiPolygon:
    case QgsWkbTypes::GeometryCollection:
      return lwcollection_make_geos_friendly( qgsgeometry_cast<QgsGeometryCollection *>( geom ) );
      break;

    default:
      QgsDebugMsg( QStringLiteral( "lwgeom_make_geos_friendly: unsupported input geometry type: %1" ).arg( geom->wkbType() ) );
      break;
  }
  return false;
}


static bool ring_make_geos_friendly( QgsCurve *ring )
{
  if ( ring->nCoordinates() == 0 )
    return false;

  // earlier we allowed in only geometries with straight segments
  QgsLineString *linestring = qgsgeometry_cast<QgsLineString *>( ring );
  Q_ASSERT_X( linestring, "ring_make_geos_friendly", "ring was not a linestring" );

  // close the ring if not already closed (2d only)

  QgsPoint p1 = linestring->startPoint(), p2 = linestring->endPoint();
  if ( p1.x() != p2.x() || p1.y() != p2.y() )
    linestring->addVertex( p1 );

  // must have at least 4 coordinates to be accepted by GEOS

  while ( linestring->nCoordinates() < 4 )
    linestring->addVertex( p1 );

  return true;
}

// Make sure all rings are closed and have > 3 points.
static bool lwpoly_make_geos_friendly( QgsPolygon *poly )
{
  // If the polygon has no rings there's nothing to do
  // TODO: in qgis representation there always is exterior ring
  //if ( ! poly->nrings ) return true;

  // All rings must be closed and have > 3 points
  for ( int i = 0; i < poly->numInteriorRings(); i++ )
  {
    if ( !ring_make_geos_friendly( qgsgeometry_cast<QgsCurve *>( poly->interiorRing( i ) ) ) )
      return false;
  }

  return true;
}

// Need NO or >1 points. Duplicate first if only one.
static bool lwline_make_geos_friendly( QgsLineString *line )
{
  if ( line->numPoints() == 1 ) // 0 is fine, 2 is fine
  {
    line->addVertex( line->startPoint() );
  }
  return true;
}

static bool lwcollection_make_geos_friendly( QgsGeometryCollection *g )
{
  for ( int i = 0; i < g->numGeometries(); i++ )
  {
    if ( !lwgeom_make_geos_friendly( g->geometryN( i ) ) )
      return false;
  }

  return true;
}


// Fully node given linework
static GEOSGeometry *LWGEOM_GEOS_nodeLines( const GEOSGeometry *lines )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  // Union with first geometry point, obtaining full noding
  // and dissolving of duplicated repeated points
  //
  // TODO: substitute this with UnaryUnion?

  GEOSGeometry *point = LWGEOM_GEOS_getPointN( lines, 0 );
  if ( ! point )
    return nullptr;

  GEOSGeometry *noded = nullptr;
  try
  {
    noded = GEOSUnion_r( handle, lines, point );
  }
  catch ( GEOSException & )
  {
    // no need to do anything here - we'll return nullptr anyway
  }
  GEOSGeom_destroy_r( handle, point );
  return noded;
}

// Will return NULL on error (expect error handler being called by then)
Q_NOWARN_UNREACHABLE_PUSH
static GEOSGeometry *LWGEOM_GEOS_makeValidPolygon( const GEOSGeometry *gin, QString &errorMessage )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  GEOSGeom gout;
  GEOSGeom geos_bound;
  GEOSGeom geos_cut_edges, geos_area, collapse_points;
  GEOSGeometry *vgeoms[3]; // One for area, one for cut-edges
  unsigned int nvgeoms = 0;

  Q_ASSERT( GEOSGeomTypeId_r( handle, gin ) == GEOS_POLYGON ||
            GEOSGeomTypeId_r( handle, gin ) == GEOS_MULTIPOLYGON );

  geos_bound = GEOSBoundary_r( handle, gin );
  if ( !geos_bound )
    return nullptr;

  // Use noded boundaries as initial "cut" edges

  geos_cut_edges = LWGEOM_GEOS_nodeLines( geos_bound );
  if ( !geos_cut_edges )
  {
    GEOSGeom_destroy_r( handle, geos_bound );
    errorMessage = QStringLiteral( "LWGEOM_GEOS_nodeLines() failed" );
    return nullptr;
  }

  // NOTE: the noding process may drop lines collapsing to points.
  //       We want to retrieve any of those
  {
    GEOSGeometry *pi = nullptr;
    GEOSGeometry *po = nullptr;

    try
    {
      pi = GEOSGeom_extractUniquePoints_r( handle, geos_bound );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_bound );
      errorMessage = QStringLiteral( "GEOSGeom_extractUniquePoints(): %1" ).arg( e.what() );
      return nullptr;
    }

    try
    {
      po = GEOSGeom_extractUniquePoints_r( handle, geos_cut_edges );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_bound );
      GEOSGeom_destroy_r( handle, pi );
      errorMessage = QStringLiteral( "GEOSGeom_extractUniquePoints(): %1" ).arg( e.what() );
      return nullptr;
    }

    try
    {
      collapse_points = GEOSDifference_r( handle, pi, po );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_bound );
      GEOSGeom_destroy_r( handle, pi );
      GEOSGeom_destroy_r( handle, po );
      errorMessage = QStringLiteral( "GEOSDifference(): %1" ).arg( e.what() );
      return nullptr;
    }

    GEOSGeom_destroy_r( handle, pi );
    GEOSGeom_destroy_r( handle, po );
  }
  GEOSGeom_destroy_r( handle, geos_bound );

  // And use an empty geometry as initial "area"
  try
  {
    geos_area = GEOSGeom_createEmptyPolygon_r( handle );
  }
  catch ( GEOSException &e )
  {
    errorMessage = QStringLiteral( "GEOSGeom_createEmptyPolygon(): %1" ).arg( e.what() );
    GEOSGeom_destroy_r( handle, geos_cut_edges );
    return nullptr;
  }

  // See if an area can be build with the remaining edges
  // and if it can, symdifference with the original area.
  // Iterate this until no more polygons can be created
  // with left-over edges.
  while ( GEOSGetNumGeometries_r( handle, geos_cut_edges ) )
  {
    GEOSGeometry *new_area = nullptr;
    GEOSGeometry *new_area_bound = nullptr;
    GEOSGeometry *symdif = nullptr;
    GEOSGeometry *new_cut_edges = nullptr;

    // ASSUMPTION: cut_edges should already be fully noded

    try
    {
      new_area = LWGEOM_GEOS_buildArea( geos_cut_edges, errorMessage );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_cut_edges );
      GEOSGeom_destroy_r( handle, geos_area );
      errorMessage = QStringLiteral( "LWGEOM_GEOS_buildArea() threw an error: %1" ).arg( e.what() );
      return nullptr;
    }

    if ( GEOSisEmpty_r( handle, new_area ) )
    {
      // no more rings can be build with the edges
      GEOSGeom_destroy_r( handle, new_area );
      break;
    }

    // We succeeded in building a ring !

    // Save the new ring boundaries first (to compute
    // further cut edges later)
    try
    {
      new_area_bound = GEOSBoundary_r( handle, new_area );
    }
    catch ( GEOSException &e )
    {
      // We did check for empty area already so
      // this must be some other error
      errorMessage = QStringLiteral( "GEOSBoundary() threw an error: %1" ).arg( e.what() );
      GEOSGeom_destroy_r( handle, new_area );
      GEOSGeom_destroy_r( handle, geos_area );
      return nullptr;
    }

    // Now symdiff new and old area
    try
    {
      symdif = GEOSSymDifference_r( handle, geos_area, new_area );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_cut_edges );
      GEOSGeom_destroy_r( handle, new_area );
      GEOSGeom_destroy_r( handle, new_area_bound );
      GEOSGeom_destroy_r( handle, geos_area );
      errorMessage = QStringLiteral( "GEOSSymDifference() threw an error: %1" ).arg( e.what() );
      return nullptr;
    }

    GEOSGeom_destroy_r( handle, geos_area );
    GEOSGeom_destroy_r( handle, new_area );
    geos_area = symdif;
    symdif = nullptr;

    // Now let's re-set geos_cut_edges with what's left
    // from the original boundary.
    // ASSUMPTION: only the previous cut-edges can be
    //             left, so we don't need to reconsider
    //             the whole original boundaries
    //
    // NOTE: this is an expensive operation.

    try
    {
      new_cut_edges = GEOSDifference_r( handle, geos_cut_edges, new_area_bound );
    }
    catch ( GEOSException &e )
    {
      GEOSGeom_destroy_r( handle, geos_cut_edges );
      GEOSGeom_destroy_r( handle, new_area_bound );
      GEOSGeom_destroy_r( handle, geos_area );
      errorMessage = QStringLiteral( "GEOSDifference() threw an error: %1" ).arg( e.what() );
      return nullptr;
    }
    GEOSGeom_destroy_r( handle, geos_cut_edges );
    GEOSGeom_destroy_r( handle, new_area_bound );
    geos_cut_edges = new_cut_edges;
  }

  if ( !GEOSisEmpty_r( handle, geos_area ) )
  {
    vgeoms[nvgeoms++] = geos_area;
  }
  else
  {
    GEOSGeom_destroy_r( handle, geos_area );
  }

  if ( !GEOSisEmpty_r( handle, geos_cut_edges ) )
  {
    vgeoms[nvgeoms++] = geos_cut_edges;
  }
  else
  {
    GEOSGeom_destroy_r( handle, geos_cut_edges );
  }

  if ( !GEOSisEmpty_r( handle, collapse_points ) )
  {
    vgeoms[nvgeoms++] = collapse_points;
  }
  else
  {
    GEOSGeom_destroy_r( handle, collapse_points );
  }

  if ( 1 == nvgeoms )
  {
    // Return cut edges
    gout = vgeoms[0];
  }
  else
  {
    // Collect areas and lines (if any line)
    try
    {
      gout = GEOSGeom_createCollection_r( handle, GEOS_GEOMETRYCOLLECTION, vgeoms, nvgeoms );
    }
    catch ( GEOSException &e )
    {
      errorMessage = QStringLiteral( "GEOSGeom_createCollection() threw an error: %1" ).arg( e.what() );
      // TODO: cleanup!
      return nullptr;
    }
  }

  return gout;

}
Q_NOWARN_UNREACHABLE_PUSH

static GEOSGeometry *LWGEOM_GEOS_makeValidLine( const GEOSGeometry *gin, QString &errorMessage )
{
  Q_UNUSED( errorMessage )
  return LWGEOM_GEOS_nodeLines( gin );
}

static GEOSGeometry *LWGEOM_GEOS_makeValidMultiLine( const GEOSGeometry *gin, QString &errorMessage )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  int ngeoms = GEOSGetNumGeometries_r( handle, gin );
  uint32_t nlines_alloc = ngeoms;
  QVector<GEOSGeometry *> lines;
  QVector<GEOSGeometry *> points;
  lines.reserve( nlines_alloc );
  points.reserve( ngeoms );

  for ( int i = 0; i < ngeoms; ++i )
  {
    const GEOSGeometry *g = GEOSGetGeometryN_r( handle, gin, i );
    GEOSGeometry *vg = LWGEOM_GEOS_makeValidLine( g, errorMessage );
    if ( GEOSisEmpty_r( handle, vg ) )
    {
      // we don't care about this one
      GEOSGeom_destroy_r( handle, vg );
    }
    if ( GEOSGeomTypeId_r( handle, vg ) == GEOS_POINT )
    {
      points.append( vg );
    }
    else if ( GEOSGeomTypeId_r( handle, vg ) == GEOS_LINESTRING )
    {
      lines.append( vg );
    }
    else if ( GEOSGeomTypeId_r( handle, vg ) == GEOS_MULTILINESTRING )
    {
      int nsubgeoms = GEOSGetNumGeometries_r( handle, vg );
      nlines_alloc += nsubgeoms;
      lines.reserve( nlines_alloc );
      for ( int j = 0; j < nsubgeoms; ++j )
      {
        // NOTE: ownership of the cloned geoms will be taken by final collection
        lines.append( GEOSGeom_clone_r( handle, GEOSGetGeometryN_r( handle, vg, j ) ) );
      }
    }
    else
    {
      // NOTE: return from GEOSGeomType will leak but we really don't expect this to happen
      errorMessage = QStringLiteral( "unexpected geom type returned by LWGEOM_GEOS_makeValid: %1" ).arg( GEOSGeomTypeId_r( handle, vg ) );
    }
  }

  GEOSGeometry *mpoint_out = nullptr;
  if ( !points.isEmpty() )
  {
    if ( points.count() > 1 )
    {
      mpoint_out = GEOSGeom_createCollection_r( handle, GEOS_MULTIPOINT, points.data(), points.count() );
    }
    else
    {
      mpoint_out = points[0];
    }
  }

  GEOSGeometry *mline_out = nullptr;
  if ( !lines.isEmpty() )
  {
    if ( lines.count() > 1 )
    {
      mline_out = GEOSGeom_createCollection_r( handle, GEOS_MULTILINESTRING, lines.data(), lines.count() );
    }
    else
    {
      mline_out = lines[0];
    }
  }

  if ( mline_out && mpoint_out )
  {
    GEOSGeometry *collection[2];
    collection[0] = mline_out;
    collection[1] = mpoint_out;
    return GEOSGeom_createCollection_r( handle, GEOS_GEOMETRYCOLLECTION, collection, 2 );
  }
  else if ( mline_out )
  {
    return mline_out;
  }
  else if ( mpoint_out )
  {
    return mpoint_out;
  }
  else
  {
    return nullptr;
  }
}


static GEOSGeometry *LWGEOM_GEOS_makeValid( const GEOSGeometry *gin, QString &errorMessage );

// Will return NULL on error (expect error handler being called by then)
static GEOSGeometry *LWGEOM_GEOS_makeValidCollection( const GEOSGeometry *gin, QString &errorMessage )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  int nvgeoms = GEOSGetNumGeometries_r( handle, gin );
  if ( nvgeoms == -1 )
  {
    errorMessage = QStringLiteral( "GEOSGetNumGeometries: %1" ).arg( QLatin1String( "?" ) );
    return nullptr;
  }

  QVector<GEOSGeometry *> vgeoms( nvgeoms );
  for ( int i = 0; i < nvgeoms; ++i )
  {
    vgeoms[i] = LWGEOM_GEOS_makeValid( GEOSGetGeometryN_r( handle, gin, i ), errorMessage );
    if ( ! vgeoms[i] )
    {
      while ( i-- )
        GEOSGeom_destroy_r( handle, vgeoms[i] );
      // we expect lwerror being called already by makeValid
      return nullptr;
    }
  }

  // Collect areas and lines (if any line)
  try
  {
    return GEOSGeom_createCollection_r( handle, GEOS_GEOMETRYCOLLECTION, vgeoms.data(), nvgeoms );
  }
  catch ( GEOSException &e )
  {
    // cleanup and throw
    for ( int i = 0; i < nvgeoms; ++i )
      GEOSGeom_destroy_r( handle, vgeoms[i] );
    errorMessage = QStringLiteral( "GEOSGeom_createCollection() threw an error: %1" ).arg( e.what() );
    return nullptr;
  }
}


static GEOSGeometry *LWGEOM_GEOS_makeValid( const GEOSGeometry *gin, QString &errorMessage )
{
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  // return what we got so far if already valid
  Q_NOWARN_UNREACHABLE_PUSH
  try
  {
    if ( GEOSisValid_r( handle, gin ) )
    {
      // It's valid at this step, return what we have
      return GEOSGeom_clone_r( handle, gin );
    }
  }
  catch ( GEOSException &e )
  {
    // I don't think should ever happen
    errorMessage = QStringLiteral( "GEOSisValid(): %1" ).arg( e.what() );
    return nullptr;
  }
  Q_NOWARN_UNREACHABLE_POP

  // make what we got valid

  switch ( GEOSGeomTypeId_r( handle, gin ) )
  {
    case GEOS_MULTIPOINT:
    case GEOS_POINT:
      // points are always valid, but we might have invalid ordinate values
      QgsDebugMsg( QStringLiteral( "PUNTUAL geometry resulted invalid to GEOS -- dunno how to clean that up" ) );
      return nullptr;

    case GEOS_LINESTRING:
      return LWGEOM_GEOS_makeValidLine( gin, errorMessage );

    case GEOS_MULTILINESTRING:
      return LWGEOM_GEOS_makeValidMultiLine( gin, errorMessage );

    case GEOS_POLYGON:
    case GEOS_MULTIPOLYGON:
      return LWGEOM_GEOS_makeValidPolygon( gin, errorMessage );

    case GEOS_GEOMETRYCOLLECTION:
      return LWGEOM_GEOS_makeValidCollection( gin, errorMessage );

    default:
      errorMessage = QStringLiteral( "ST_MakeValid: doesn't support geometry type: %1" ).arg( GEOSGeomTypeId_r( handle, gin ) );
      return nullptr;
  }
}


std::unique_ptr< QgsAbstractGeometry > _qgis_lwgeom_make_valid( const QgsAbstractGeometry *lwgeom_in, QString &errorMessage )
{
  //bool is3d = FLAGS_GET_Z(lwgeom_in->flags);

  // try to convert to GEOS, if impossible, clean that up first
  // otherwise (adding only duplicates of existing points)
  GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();

  geos::unique_ptr geosgeom = QgsGeos::asGeos( lwgeom_in );
  if ( !geosgeom )
  {
    QgsDebugMsgLevel( QStringLiteral( "Original geom can't be converted to GEOS - will try cleaning that up first" ), 3 );

    std::unique_ptr<QgsAbstractGeometry> lwgeom_in_clone( lwgeom_in->clone() );
    if ( !lwgeom_make_geos_friendly( lwgeom_in_clone.get() ) )
    {
      QgsDebugMsg( QStringLiteral( "Could not make a valid geometry out of input" ) );
    }

    // try again as we did cleanup now
    // TODO: invoke LWGEOM2GEOS directly with autoclean ?
    geosgeom = QgsGeos::asGeos( lwgeom_in_clone.get() );

    if ( ! geosgeom )
    {
      errorMessage = QStringLiteral( "Could not convert QGIS geom to GEOS" );
      return nullptr;
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "original geom converted to GEOS" ), 4 );
  }

  GEOSGeometry *geosout = LWGEOM_GEOS_makeValid( geosgeom.get(), errorMessage );
  if ( !geosout )
    return nullptr;

  std::unique_ptr< QgsAbstractGeometry > lwgeom_out = QgsGeos::fromGeos( geosout );
  GEOSGeom_destroy_r( handle, geosout );
  if ( !lwgeom_out )
    return nullptr;

  // force multi-type if we had a multi-type before
  if ( QgsWkbTypes::isMultiType( lwgeom_in->wkbType() ) && !QgsWkbTypes::isMultiType( lwgeom_out->wkbType() ) )
  {
    QgsGeometryCollection *collection = nullptr;
    switch ( QgsWkbTypes::multiType( lwgeom_out->wkbType() ) )
    {
      case QgsWkbTypes::MultiPoint:
        collection = new QgsMultiPoint();
        break;
      case QgsWkbTypes::MultiLineString:
        collection = new QgsMultiLineString();
        break;
      case QgsWkbTypes::MultiPolygon:
        collection = new QgsMultiPolygon();
        break;
      default:
        collection = new QgsGeometryCollection();
        break;
    }
    collection->addGeometry( lwgeom_out.release() ); // takes ownership
    lwgeom_out.reset( collection );
  }

  return lwgeom_out;
}

#endif
