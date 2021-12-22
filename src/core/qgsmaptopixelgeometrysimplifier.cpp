/***************************************************************************
    qgsmaptopixelgeometrysimplifier.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <memory>

#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsrectangle.h"
#include "qgswkbptr.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometrycollection.h"
#include "qgsvertexid.h"

QgsMapToPixelSimplifier::QgsMapToPixelSimplifier( int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm )
  : mSimplifyFlags( simplifyFlags )
  , mSimplifyAlgorithm( simplifyAlgorithm )
  , mTolerance( tolerance )
{
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods

float QgsMapToPixelSimplifier::calculateLengthSquared2D( double x1, double y1, double x2, double y2 )
{
  const float vx = static_cast< float >( x2 - x1 );
  const float vy = static_cast< float >( y2 - y1 );

  return ( vx * vx ) + ( vy * vy );
}

bool QgsMapToPixelSimplifier::equalSnapToGrid( double x1, double y1, double x2, double y2, double gridOriginX, double gridOriginY, float gridInverseSizeXY )
{
  const int grid_x1 = std::round( ( x1 - gridOriginX ) * gridInverseSizeXY );
  const int grid_x2 = std::round( ( x2 - gridOriginX ) * gridInverseSizeXY );
  if ( grid_x1 != grid_x2 ) return false;

  const int grid_y1 = std::round( ( y1 - gridOriginY ) * gridInverseSizeXY );
  const int grid_y2 = std::round( ( y2 - gridOriginY ) * gridInverseSizeXY );
  return grid_y1 == grid_y2;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helper simplification methods for Visvalingam method

// It uses a refactored code of the liblwgeom implementation:
// https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/effectivearea.h
// https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/effectivearea.c

#include "simplify/effectivearea.h"

//////////////////////////////////////////////////////////////////////////////////////////////

//! Generalize the WKB-geometry using the BBOX of the original geometry
static std::unique_ptr< QgsAbstractGeometry > generalizeWkbGeometryByBoundingBox(
  QgsWkbTypes::Type wkbType,
  const QgsAbstractGeometry &geometry,
  const QgsRectangle &envelope,
  bool isRing )
{
  const unsigned int geometryType = QgsWkbTypes::singleType( QgsWkbTypes::flatType( wkbType ) );

  // If the geometry is already minimal skip the generalization
  const int minimumSize = geometryType == QgsWkbTypes::LineString ? 2 : 5;

  if ( geometry.nCoordinates() <= minimumSize )
  {
    return std::unique_ptr< QgsAbstractGeometry >( geometry.clone() );
  }

  const double x1 = envelope.xMinimum();
  const double y1 = envelope.yMinimum();
  const double x2 = envelope.xMaximum();
  const double y2 = envelope.yMaximum();

  // Write the generalized geometry
  if ( geometryType == QgsWkbTypes::LineString && !isRing )
  {
    return std::make_unique< QgsLineString >( QVector<double>() << x1 << x2, QVector<double>() << y1 << y2 );
  }
  else
  {
    std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString >(
        QVector< double >() << x1
        << x2
        << x2
        << x1
        << x1,
        QVector< double >() << y1
        << y1
        << y2
        << y2
        << y1 );
    if ( geometryType == QgsWkbTypes::LineString )
      return std::move( ext );
    else
    {
      std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();
      polygon->setExteriorRing( ext.release() );
      return std::move( polygon );
    }
  }
}

std::unique_ptr< QgsAbstractGeometry > QgsMapToPixelSimplifier::simplifyGeometry( int simplifyFlags,
    SimplifyAlgorithm simplifyAlgorithm,
    const QgsAbstractGeometry &geometry, double map2pixelTol,
    bool isaLinearRing )
{
  bool isGeneralizable = true;
  const QgsWkbTypes::Type wkbType = geometry.wkbType();

  // Can replace the geometry by its BBOX ?
  const QgsRectangle envelope = geometry.boundingBox();
  if ( ( simplifyFlags & QgsMapToPixelSimplifier::SimplifyEnvelope ) &&
       isGeneralizableByMapBoundingBox( envelope, map2pixelTol ) )
  {
    return generalizeWkbGeometryByBoundingBox( wkbType, geometry, envelope, isaLinearRing );
  }

  if ( !( simplifyFlags & QgsMapToPixelSimplifier::SimplifyGeometry ) )
    isGeneralizable = false;

  const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( wkbType );

  // Write the geometry
  if ( flatType == QgsWkbTypes::LineString || flatType == QgsWkbTypes::CircularString )
  {
    const QgsCurve &srcCurve = dynamic_cast<const QgsCurve &>( geometry );
    const int numPoints = srcCurve.numPoints();

    std::unique_ptr<QgsCurve> output;

    QVector< double > lineStringX;
    QVector< double > lineStringY;
    if ( flatType == QgsWkbTypes::LineString )
    {
      // if we are making a linestring, we do it in an optimised way by directly constructing
      // the final x/y vectors, which avoids calling the slower insertVertex method
      lineStringX.reserve( numPoints );
      lineStringY.reserve( numPoints );
    }
    else
    {
      output.reset( qgsgeometry_cast< QgsCurve * >( srcCurve.createEmptyWithSameType() ) );
    }

    double x = 0.0, y = 0.0, lastX = 0.0, lastY = 0.0;

    if ( numPoints <= ( isaLinearRing ? 4 : 2 ) )
      isGeneralizable = false;

    bool isLongSegment;
    bool hasLongSegments = false; //-> To avoid replace the simplified geometry by its BBOX when there are 'long' segments.

    // Check whether the LinearRing is really closed.
    if ( isaLinearRing )
    {
      isaLinearRing = qgsDoubleNear( srcCurve.xAt( 0 ), srcCurve.xAt( numPoints - 1 ) ) &&
                      qgsDoubleNear( srcCurve.yAt( 0 ), srcCurve.yAt( numPoints - 1 ) );
    }

    // Process each vertex...
    switch ( simplifyAlgorithm )
    {
      case SnapToGrid:
      {
        const double gridOriginX = envelope.xMinimum();
        const double gridOriginY = envelope.yMinimum();

        // Use a factor for the maximum displacement distance for simplification, similar as GeoServer does
        const float gridInverseSizeXY = map2pixelTol != 0 ? ( float )( 1.0f / ( 0.8 * map2pixelTol ) ) : 0.0f;

        const double *xData = nullptr;
        const double *yData = nullptr;
        if ( flatType == QgsWkbTypes::LineString )
        {
          xData = qgsgeometry_cast< const QgsLineString * >( &srcCurve )->xData();
          yData = qgsgeometry_cast< const QgsLineString * >( &srcCurve )->yData();
        }

        for ( int i = 0; i < numPoints; ++i )
        {
          if ( xData && yData )
          {
            x = *xData++;
            y = *yData++;
          }
          else
          {
            x = srcCurve.xAt( i );
            y = srcCurve.yAt( i );
          }

          if ( i == 0 ||
               !isGeneralizable ||
               !equalSnapToGrid( x, y, lastX, lastY, gridOriginX, gridOriginY, gridInverseSizeXY ) ||
               ( !isaLinearRing && ( i == 1 || i >= numPoints - 2 ) ) )
          {
            if ( output )
              output->insertVertex( QgsVertexId( 0, 0, output->numPoints() ), QgsPoint( x, y ) );
            else
            {
              lineStringX.append( x );
              lineStringY.append( y );
            }
            lastX = x;
            lastY = y;
          }
        }
        break;
      }

      case SnappedToGridGlobal:
      {
        output.reset( qgsgeometry_cast< QgsCurve * >( srcCurve.snappedToGrid( map2pixelTol, map2pixelTol ) ) );
        break;
      }

      case Visvalingam:
      {
        map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'Area' calculations.

        EFFECTIVE_AREAS ea( srcCurve );

        const int set_area = 0;
        ptarray_calc_areas( &ea, isaLinearRing ? 4 : 2, set_area, map2pixelTol );

        for ( int i = 0; i < numPoints; ++i )
        {
          if ( ea.res_arealist[ i ] > map2pixelTol )
          {
            if ( output )
              output->insertVertex( QgsVertexId( 0, 0, output->numPoints() ), ea.inpts.at( i ) );
            else
            {
              lineStringX.append( ea.inpts.at( i ).x() );
              lineStringY.append( ea.inpts.at( i ).y() );
            }
          }
        }
        break;
      }

      case Distance:
      {
        map2pixelTol *= map2pixelTol; //-> Use mappixelTol for 'LengthSquare' calculations.

        const double *xData = nullptr;
        const double *yData = nullptr;
        if ( flatType == QgsWkbTypes::LineString )
        {
          xData = qgsgeometry_cast< const QgsLineString * >( &srcCurve )->xData();
          yData = qgsgeometry_cast< const QgsLineString * >( &srcCurve )->yData();
        }

        for ( int i = 0; i < numPoints; ++i )
        {
          if ( xData && yData )
          {
            x = *xData++;
            y = *yData++;
          }
          else
          {
            x = srcCurve.xAt( i );
            y = srcCurve.yAt( i );
          }

          isLongSegment = false;

          if ( i == 0 ||
               !isGeneralizable ||
               ( isLongSegment = ( calculateLengthSquared2D( x, y, lastX, lastY ) > map2pixelTol ) ) ||
               ( !isaLinearRing && ( i == 1 || i >= numPoints - 2 ) ) )
          {
            if ( output )
              output->insertVertex( QgsVertexId( 0, 0, output->numPoints() ), QgsPoint( x, y ) );
            else
            {
              lineStringX.append( x );
              lineStringY.append( y );
            }
            lastX = x;
            lastY = y;

            hasLongSegments |= isLongSegment;
          }
        }
      }
    }

    if ( !output )
    {
      output = std::make_unique< QgsLineString >( lineStringX, lineStringY );
    }
    if ( output->numPoints() < ( isaLinearRing ? 4 : 2 ) )
    {
      // we simplified the geometry too much!
      if ( !hasLongSegments )
      {
        // approximate the geometry's shape by its bounding box
        // (rect for linear ring / one segment for line string)
        return generalizeWkbGeometryByBoundingBox( wkbType, geometry, envelope, isaLinearRing );
      }
      else
      {
        // Bad luck! The simplified geometry is invalid and approximation by bounding box
        // would create artifacts due to long segments.
        // We will return the original geometry
        return std::unique_ptr< QgsAbstractGeometry >( geometry.clone() );
      }
    }

    if ( isaLinearRing )
    {
      // make sure we keep the linear ring closed
      if ( !qgsDoubleNear( lastX, output->xAt( 0 ) ) || !qgsDoubleNear( lastY, output->yAt( 0 ) ) )
      {
        output->insertVertex( QgsVertexId( 0, 0, output->numPoints() ), QgsPoint( output->xAt( 0 ), output->yAt( 0 ) ) );
      }
    }

    return std::move( output );
  }
  else if ( flatType == QgsWkbTypes::Polygon )
  {
    const QgsPolygon &srcPolygon = dynamic_cast<const QgsPolygon &>( geometry );
    std::unique_ptr<QgsPolygon> polygon( new QgsPolygon() );
    std::unique_ptr<QgsAbstractGeometry> extRing = simplifyGeometry( simplifyFlags, simplifyAlgorithm, *srcPolygon.exteriorRing(), map2pixelTol, true );
    polygon->setExteriorRing( qgsgeometry_cast<QgsCurve *>( extRing.release() ) );
    for ( int i = 0; i < srcPolygon.numInteriorRings(); ++i )
    {
      const QgsCurve *sub = srcPolygon.interiorRing( i );
      std::unique_ptr< QgsAbstractGeometry > ring = simplifyGeometry( simplifyFlags, simplifyAlgorithm, *sub, map2pixelTol, true );
      polygon->addInteriorRing( qgsgeometry_cast<QgsCurve *>( ring.release() ) );
    }
    return std::move( polygon );
  }
  else if ( QgsWkbTypes::isMultiType( flatType ) )
  {
    const QgsGeometryCollection &srcCollection = dynamic_cast<const QgsGeometryCollection &>( geometry );
    std::unique_ptr<QgsGeometryCollection> collection( srcCollection.createEmptyWithSameType() );
    const int numGeoms = srcCollection.numGeometries();
    collection->reserve( numGeoms );
    for ( int i = 0; i < numGeoms; ++i )
    {
      const QgsAbstractGeometry *sub = srcCollection.geometryN( i );
      std::unique_ptr< QgsAbstractGeometry > part = simplifyGeometry( simplifyFlags, simplifyAlgorithm, *sub, map2pixelTol, false );
      collection->addGeometry( part.release() );
    }
    return std::move( collection );
  }
  return std::unique_ptr< QgsAbstractGeometry >( geometry.clone() );
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool QgsMapToPixelSimplifier::isGeneralizableByMapBoundingBox( const QgsRectangle &envelope, double map2pixelTol )
{
  // Can replace the geometry by its BBOX ?
  return envelope.width() < map2pixelTol && envelope.height() < map2pixelTol;
}

QgsGeometry QgsMapToPixelSimplifier::simplify( const QgsGeometry &geometry ) const
{
  if ( geometry.isNull() )
  {
    return QgsGeometry();
  }
  if ( mSimplifyFlags == QgsMapToPixelSimplifier::NoFlags )
  {
    return geometry;
  }

  // Check whether the geometry can be simplified using the map2pixel context
  const QgsWkbTypes::Type singleType = QgsWkbTypes::singleType( geometry.wkbType() );
  const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( singleType );
  if ( flatType == QgsWkbTypes::Point )
  {
    return geometry;
  }

  const bool isaLinearRing = flatType == QgsWkbTypes::Polygon;
  const int numPoints = geometry.constGet()->nCoordinates();

  if ( numPoints <= ( isaLinearRing ? 6 : 3 ) )
  {
    // No simplify simple geometries
    return geometry;
  }

  const QgsRectangle envelope = geometry.boundingBox();
  if ( std::max( envelope.width(), envelope.height() ) / numPoints > mTolerance * 2.0 )
  {
    //points are in average too far apart to lead to any significant simplification
    return geometry;
  }

  return QgsGeometry( simplifyGeometry( mSimplifyFlags, mSimplifyAlgorithm, *geometry.constGet(), mTolerance, false ) );
}

QgsAbstractGeometry *QgsMapToPixelSimplifier::simplify( const QgsAbstractGeometry *geometry ) const
{
  //
  // IMPORTANT!!!!!!!
  // We want to avoid any geometry cloning we possibly can here, which is why the
  // "fail" paths always return nullptr
  //

  if ( !geometry )
  {
    return nullptr;
  }
  if ( mSimplifyFlags == QgsMapToPixelSimplifier::NoFlags )
  {
    return nullptr;
  }

  // Check whether the geometry can be simplified using the map2pixel context
  const QgsWkbTypes::Type singleType = QgsWkbTypes::singleType( geometry->wkbType() );
  const QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( singleType );
  if ( flatType == QgsWkbTypes::Point )
  {
    return nullptr;
  }

  const bool isaLinearRing = flatType == QgsWkbTypes::Polygon;
  const int numPoints = geometry->nCoordinates();

  if ( numPoints <= ( isaLinearRing ? 6 : 3 ) )
  {
    // No simplify simple geometries
    return nullptr;
  }

  const QgsRectangle envelope = geometry->boundingBox();
  if ( std::max( envelope.width(), envelope.height() ) / numPoints > mTolerance * 2.0 )
  {
    //points are in average too far apart to lead to any significant simplification
    return nullptr;
  }

  return simplifyGeometry( mSimplifyFlags, mSimplifyAlgorithm, *geometry, mTolerance, false ).release();
}
