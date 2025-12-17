/***************************************************************************
                         qgsnurbsutils.cpp
                         -----------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnurbsutils.h"

#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsnurbscurve.h"
#include "qgsvertexid.h"

bool QgsNurbsUtils::containsNurbsCurve( const QgsAbstractGeometry *geom )
{
  if ( !geom )
    return false;

  if ( qgsgeometry_cast<const QgsNurbsCurve *>( geom ) )
    return true;

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast<const QgsGeometryCollection *>( geom ) )
  {
    for ( int i = 0; i < gc->numGeometries(); ++i )
    {
      if ( containsNurbsCurve( gc->geometryN( i ) ) )
        return true;
    }
  }

  if ( const QgsCurvePolygon *cp = qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
  {
    if ( containsNurbsCurve( cp->exteriorRing() ) )
      return true;
    for ( int i = 0; i < cp->numInteriorRings(); ++i )
    {
      if ( containsNurbsCurve( cp->interiorRing( i ) ) )
        return true;
    }
  }

  if ( const QgsCompoundCurve *cc = qgsgeometry_cast<const QgsCompoundCurve *>( geom ) )
  {
    for ( int i = 0; i < cc->nCurves(); ++i )
    {
      if ( containsNurbsCurve( cc->curveAt( i ) ) )
        return true;
    }
  }

  return false;
}

const QgsNurbsCurve *QgsNurbsUtils::extractNurbsCurve( const QgsAbstractGeometry *geom )
{
  if ( !geom )
    return nullptr;

  if ( const QgsNurbsCurve *nurbs = qgsgeometry_cast<const QgsNurbsCurve *>( geom ) )
    return nurbs;

  if ( const QgsGeometryCollection *gc = qgsgeometry_cast<const QgsGeometryCollection *>( geom ) )
  {
    for ( int i = 0; i < gc->numGeometries(); ++i )
    {
      const QgsNurbsCurve *nurbs = extractNurbsCurve( gc->geometryN( i ) );
      if ( nurbs )
        return nurbs;
    }
  }

  if ( const QgsCurvePolygon *cp = qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
  {
    const QgsNurbsCurve *nurbs = extractNurbsCurve( cp->exteriorRing() );
    if ( nurbs )
      return nurbs;
    for ( int i = 0; i < cp->numInteriorRings(); ++i )
    {
      nurbs = extractNurbsCurve( cp->interiorRing( i ) );
      if ( nurbs )
        return nurbs;
    }
  }

  if ( const QgsCompoundCurve *cc = qgsgeometry_cast<const QgsCompoundCurve *>( geom ) )
  {
    for ( int i = 0; i < cc->nCurves(); ++i )
    {
      const QgsNurbsCurve *nurbs = extractNurbsCurve( cc->curveAt( i ) );
      if ( nurbs )
        return nurbs;
    }
  }

  return nullptr;
}

bool QgsNurbsUtils::isPolyBezier( const QgsNurbsCurve *nurbs )
{
  if ( !nurbs )
    return false;

  // A poly-Bézier is degree 3 and has (n-1) divisible by 3
  // where n is the number of control points
  // This means: n = 3*segments + 1, so n-1 = 3*segments, so (n-1) % 3 == 0
  const int n = nurbs->controlPoints().size();
  return nurbs->degree() == 3 && n >= 4 && ( n - 1 ) % 3 == 0;
}

const QgsNurbsCurve *QgsNurbsUtils::findNurbsCurveForVertex( const QgsAbstractGeometry *geom, const QgsVertexId &vid, int &localIndex )
{
  if ( !geom )
    return nullptr;

  // Direct NURBS curve
  if ( const QgsNurbsCurve *nurbs = qgsgeometry_cast<const QgsNurbsCurve *>( geom ) )
  {
    localIndex = vid.vertex;
    return nurbs;
  }

  // Compound curve - find the curve containing this vertex
  if ( const QgsCompoundCurve *compound = qgsgeometry_cast<const QgsCompoundCurve *>( geom ) )
  {
    int vertexOffset = 0;
    for ( int i = 0; i < compound->nCurves(); ++i )
    {
      const QgsCurve *curve = compound->curveAt( i );
      const int curveVertexCount = curve->numPoints();

      // Check if vertex is in this curve (accounting for shared endpoints)
      const int adjustedCount = ( i == compound->nCurves() - 1 ) ? curveVertexCount : curveVertexCount - 1;
      if ( vid.vertex < vertexOffset + adjustedCount )
      {
        if ( const QgsNurbsCurve *nurbs = qgsgeometry_cast<const QgsNurbsCurve *>( curve ) )
        {
          localIndex = vid.vertex - vertexOffset;
          return nurbs;
        }
        return nullptr;
      }
      vertexOffset += adjustedCount;
    }
  }

  // Curve polygon - check exterior and interior rings
  if ( const QgsCurvePolygon *polygon = qgsgeometry_cast<const QgsCurvePolygon *>( geom ) )
  {
    const QgsCurve *ring = nullptr;
    if ( vid.ring == 0 )
      ring = polygon->exteriorRing();
    else if ( vid.ring > 0 && vid.ring <= polygon->numInteriorRings() )
      ring = polygon->interiorRing( vid.ring - 1 );

    if ( ring )
      return findNurbsCurveForVertex( ring, QgsVertexId( 0, 0, vid.vertex ), localIndex );
  }

  // Geometry collection
  if ( const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geom ) )
  {
    if ( vid.part >= 0 && vid.part < collection->numGeometries() )
    {
      return findNurbsCurveForVertex( collection->geometryN( vid.part ), QgsVertexId( 0, vid.ring, vid.vertex ), localIndex );
    }
  }

  return nullptr;
}

QgsNurbsCurve *QgsNurbsUtils::findMutableNurbsCurveForVertex( QgsAbstractGeometry *geom, const QgsVertexId &vid, int &localIndex )
{
  if ( !geom )
    return nullptr;

  // Direct NURBS curve
  if ( QgsNurbsCurve *nurbs = qgsgeometry_cast<QgsNurbsCurve *>( geom ) )
  {
    localIndex = vid.vertex;
    return nurbs;
  }

  // Compound curve - find the curve containing this vertex
  if ( const QgsCompoundCurve *compound = qgsgeometry_cast<const QgsCompoundCurve *>( geom ) )
  {
    int vertexOffset = 0;
    for ( int i = 0; i < compound->nCurves(); ++i )
    {
      const QgsCurve *curve = compound->curveAt( i );
      const int curveVertexCount = curve->numPoints();

      // Check if vertex is in this curve (accounting for shared endpoints)
      const int adjustedCount = ( i == compound->nCurves() - 1 ) ? curveVertexCount : curveVertexCount - 1;
      if ( vid.vertex < vertexOffset + adjustedCount )
      {
        if ( const QgsNurbsCurve *nurbs = qgsgeometry_cast<const QgsNurbsCurve *>( curve ) )
        {
          localIndex = vid.vertex - vertexOffset;
          // Return non-const by casting via the original geometry parameter
          return const_cast<QgsNurbsCurve *>( nurbs );
        }
        return nullptr;
      }
      vertexOffset += adjustedCount;
    }
  }

  // Curve polygon - check exterior and interior rings
  if ( QgsCurvePolygon *polygon = qgsgeometry_cast<QgsCurvePolygon *>( geom ) )
  {
    QgsCurve *ring = nullptr;
    if ( vid.ring == 0 )
      ring = polygon->exteriorRing();
    else if ( vid.ring > 0 && vid.ring <= polygon->numInteriorRings() )
      ring = polygon->interiorRing( vid.ring - 1 );

    if ( ring )
      return findMutableNurbsCurveForVertex( ring, QgsVertexId( 0, 0, vid.vertex ), localIndex );
  }

  // Geometry collection
  if ( QgsGeometryCollection *collection = qgsgeometry_cast<QgsGeometryCollection *>( geom ) )
  {
    if ( vid.part >= 0 && vid.part < collection->numGeometries() )
    {
      return findMutableNurbsCurveForVertex( collection->geometryN( vid.part ), QgsVertexId( 0, vid.ring, vid.vertex ), localIndex );
    }
  }

  return nullptr;
}
