/***************************************************************************
                         qgswkbtypes.h
                         -----------------------
    begin                : January 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWKBTYPES_H
#define QGSWKBTYPES_H

#include <QObject>
#include <QMap>
#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsWkbTypes
 * \brief Handles storage of information regarding WKB types and their properties.
 */

class CORE_EXPORT QgsWkbTypes
{
    Q_GADGET
  public:

    /**
     * Returns the single type for a WKB type. For example, for MultiPolygon WKB types the single type would be Polygon.
     * \see isSingleType()
     * \see multiType()
     * \see curveType()
     * \see flatType()
     */
    static Qgis::WkbType singleType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
        case Qgis::WkbType::GeometryCollection:
        case Qgis::WkbType::GeometryCollectionZ:
        case Qgis::WkbType::GeometryCollectionM:
        case Qgis::WkbType::GeometryCollectionZM:
          return Qgis::WkbType::Unknown;

        case Qgis::WkbType::Point:
        case Qgis::WkbType::MultiPoint:
          return Qgis::WkbType::Point;

        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::MultiPointZ:
          return Qgis::WkbType::PointZ;

        case Qgis::WkbType::PointM:
        case Qgis::WkbType::MultiPointM:
          return Qgis::WkbType::PointM;

        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::MultiPointZM:
          return Qgis::WkbType::PointZM;

        case Qgis::WkbType::LineString:
        case Qgis::WkbType::MultiLineString:
          return Qgis::WkbType::LineString;

        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::MultiLineStringZ:
          return Qgis::WkbType::LineStringZ;

        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::MultiLineStringM:
          return Qgis::WkbType::LineStringM;

        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::MultiLineStringZM:
          return Qgis::WkbType::LineStringZM;

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::MultiPolygon:
          return Qgis::WkbType::Polygon;

        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::MultiPolygonZ:
          return Qgis::WkbType::PolygonZ;

        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::MultiPolygonM:
          return Qgis::WkbType::PolygonM;

        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::MultiPolygonZM:
          return Qgis::WkbType::PolygonZM;

        case Qgis::WkbType::Triangle:
          return Qgis::WkbType::Triangle;

        case Qgis::WkbType::TriangleZ:
          return Qgis::WkbType::TriangleZ;

        case Qgis::WkbType::TriangleM:
          return Qgis::WkbType::TriangleM;

        case Qgis::WkbType::TriangleZM:
          return Qgis::WkbType::TriangleZM;

        case Qgis::WkbType::CircularString:
          return Qgis::WkbType::CircularString;

        case Qgis::WkbType::CircularStringZ:
          return Qgis::WkbType::CircularStringZ;

        case Qgis::WkbType::CircularStringM:
          return Qgis::WkbType::CircularStringM;

        case Qgis::WkbType::CircularStringZM:
          return Qgis::WkbType::CircularStringZM;

        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::MultiCurve:
          return Qgis::WkbType::CompoundCurve;

        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::MultiCurveZ:
          return Qgis::WkbType::CompoundCurveZ;

        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::MultiCurveM:
          return Qgis::WkbType::CompoundCurveM;

        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::MultiCurveZM:
          return Qgis::WkbType::CompoundCurveZM;

        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::MultiSurface:
          return Qgis::WkbType::CurvePolygon;

        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::MultiSurfaceZ:
          return Qgis::WkbType::CurvePolygonZ;

        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::MultiSurfaceM:
          return Qgis::WkbType::CurvePolygonM;

        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::MultiSurfaceZM:
          return Qgis::WkbType::CurvePolygonZM;

        case Qgis::WkbType::PolyhedralSurface:
          return Qgis::WkbType::PolyhedralSurface;

        case Qgis::WkbType::PolyhedralSurfaceZ:
          return Qgis::WkbType::PolyhedralSurfaceZ;

        case Qgis::WkbType::PolyhedralSurfaceM:
          return Qgis::WkbType::PolyhedralSurfaceM;

        case Qgis::WkbType::PolyhedralSurfaceZM:
          return Qgis::WkbType::PolyhedralSurfaceZM;

        case Qgis::WkbType::TIN:
          return Qgis::WkbType::TIN;

        case Qgis::WkbType::TINZ:
          return Qgis::WkbType::TINZ;

        case Qgis::WkbType::TINM:
          return Qgis::WkbType::TINM;

        case Qgis::WkbType::TINZM:
          return Qgis::WkbType::TINZM;

        case Qgis::WkbType::NoGeometry:
          return Qgis::WkbType::NoGeometry;

        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::MultiPoint25D:
          return Qgis::WkbType::Point25D;

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::MultiLineString25D:
          return Qgis::WkbType::LineString25D;

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::MultiPolygon25D:
          return Qgis::WkbType::Polygon25D;

      }
      return Qgis::WkbType::Unknown;
    }

    /**
     * Returns the multi type for a WKB type. For example, for Polygon WKB types the multi type would be MultiPolygon.
     *
     * \see promoteNonPointTypesToMulti()
     * \see isMultiType()
     * \see singleType()
     * \see curveType()
     * \see flatType()
     */
    static Qgis::WkbType multiType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
          return Qgis::WkbType::Unknown;

        case Qgis::WkbType::Triangle:
          return Qgis::WkbType::MultiPolygon;

        case Qgis::WkbType::TriangleZ:
          return Qgis::WkbType::MultiPolygonZ;

        case Qgis::WkbType::TriangleM:
          return Qgis::WkbType::MultiPolygonM;

        case Qgis::WkbType::TriangleZM:
          return Qgis::WkbType::MultiPolygonZM;

        case Qgis::WkbType::GeometryCollection:
          return Qgis::WkbType::GeometryCollection;

        case Qgis::WkbType::GeometryCollectionZ:
          return Qgis::WkbType::GeometryCollectionZ;

        case Qgis::WkbType::GeometryCollectionM:
          return Qgis::WkbType::GeometryCollectionM;

        case Qgis::WkbType::GeometryCollectionZM:
          return Qgis::WkbType::GeometryCollectionZM;

        case Qgis::WkbType::Point:
        case Qgis::WkbType::MultiPoint:
          return Qgis::WkbType::MultiPoint;

        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::MultiPointZ:
          return Qgis::WkbType::MultiPointZ;

        case Qgis::WkbType::PointM:
        case Qgis::WkbType::MultiPointM:
          return Qgis::WkbType::MultiPointM;

        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::MultiPointZM:
          return Qgis::WkbType::MultiPointZM;

        case Qgis::WkbType::LineString:
        case Qgis::WkbType::MultiLineString:
          return Qgis::WkbType::MultiLineString;

        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::MultiLineStringZ:
          return Qgis::WkbType::MultiLineStringZ;

        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::MultiLineStringM:
          return Qgis::WkbType::MultiLineStringM;

        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::MultiLineStringZM:
          return Qgis::WkbType::MultiLineStringZM;

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::TIN:
          return Qgis::WkbType::MultiPolygon;

        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::TINZ:
          return Qgis::WkbType::MultiPolygonZ;

        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::TINM:
          return Qgis::WkbType::MultiPolygonM;

        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
          return Qgis::WkbType::MultiPolygonZM;

        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::MultiCurve:
          return Qgis::WkbType::MultiCurve;

        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::MultiCurveZ:
          return Qgis::WkbType::MultiCurveZ;

        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::MultiCurveM:
          return Qgis::WkbType::MultiCurveM;

        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::MultiCurveZM:
          return Qgis::WkbType::MultiCurveZM;

        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::MultiSurface:
          return Qgis::WkbType::MultiSurface;

        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::MultiSurfaceZ:
          return Qgis::WkbType::MultiSurfaceZ;

        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::MultiSurfaceM:
          return Qgis::WkbType::MultiSurfaceM;

        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::MultiSurfaceZM:
          return Qgis::WkbType::MultiSurfaceZM;

        case Qgis::WkbType::NoGeometry:
          return Qgis::WkbType::NoGeometry;

        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::MultiPoint25D:
          return Qgis::WkbType::MultiPoint25D;

        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::MultiLineString25D:
          return Qgis::WkbType::MultiLineString25D;

        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::MultiPolygon25D:
          return Qgis::WkbType::MultiPolygon25D;
      }
      return Qgis::WkbType::Unknown;
    }


    /**
     * Promotes a WKB geometry type to its multi-type equivalent, with the exception of point geometry types.
     *
     * Specifically, this method should be used to determine the most-permissive possible resultant WKB type which can result
     * from subtracting parts of a geometry. A single-point geometry type can never become a multi-point geometry type as
     * a result of a subtraction, but a single-line or single-polygon geometry CAN become a multipart geometry as a result of subtracting
     * portions of the geometry.
     *
     * \see multiType()
     * \see singleType()
     * \since QGIS 3.24
     */
    static Qgis::WkbType promoteNonPointTypesToMulti( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( geometryType( type ) )
      {
        case Qgis::GeometryType::Point:
        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          return type;

        case Qgis::GeometryType::Line:
        case Qgis::GeometryType::Polygon:
          return multiType( type );
      }
      return Qgis::WkbType::Unknown;
    }

    /**
     * Returns the curve type for a WKB type. For example, for Polygon WKB types the curve type would be CurvePolygon.
     *
     * \note Returns `CompoundCurve` for `CircularString` (and its Z/M variants)
     *
     * \see linearType()
     * \see isMultiType()
     * \see isCurvedType()
     * \see singleType()
     * \see flatType()
     * \see multiType()
     *
     * \since QGIS 3.10
     */
    static Qgis::WkbType curveType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
        case Qgis::WkbType::Triangle:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::TriangleZM:
          return Qgis::WkbType::Unknown;

        case Qgis::WkbType::GeometryCollection:
          return Qgis::WkbType::GeometryCollection;

        case Qgis::WkbType::GeometryCollectionZ:
          return Qgis::WkbType::GeometryCollectionZ;

        case Qgis::WkbType::GeometryCollectionM:
          return Qgis::WkbType::GeometryCollectionM;

        case Qgis::WkbType::GeometryCollectionZM:
          return Qgis::WkbType::GeometryCollectionZM;

        case Qgis::WkbType::Point:
          return Qgis::WkbType::Point;

        case Qgis::WkbType::MultiPoint:
          return Qgis::WkbType::MultiPoint;

        case Qgis::WkbType::PointZ:
          return Qgis::WkbType::PointZ;

        case Qgis::WkbType::MultiPointZ:
          return Qgis::WkbType::MultiPointZ;

        case Qgis::WkbType::PointM:
          return Qgis::WkbType::PointM;

        case Qgis::WkbType::MultiPointM:
          return Qgis::WkbType::MultiPointM;

        case Qgis::WkbType::PointZM:
          return Qgis::WkbType::PointZM;

        case Qgis::WkbType::MultiPointZM:
          return Qgis::WkbType::MultiPointZM;

        case Qgis::WkbType::LineString:
        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::CircularString:
          return Qgis::WkbType::CompoundCurve;

        case Qgis::WkbType::MultiLineString:
        case Qgis::WkbType::MultiCurve:
          return Qgis::WkbType::MultiCurve;

        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::LineString25D:
          return Qgis::WkbType::CompoundCurveZ;

        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::MultiLineString25D:
          return Qgis::WkbType::MultiCurveZ;

        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::CircularStringM:
          return Qgis::WkbType::CompoundCurveM;

        case Qgis::WkbType::MultiLineStringM:
        case Qgis::WkbType::MultiCurveM:
          return Qgis::WkbType::MultiCurveM;

        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::CircularStringZM:
          return Qgis::WkbType::CompoundCurveZM;

        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::MultiCurveZM:
          return Qgis::WkbType::MultiCurveZM;

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::CurvePolygon:
          return Qgis::WkbType::CurvePolygon;

        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::TIN:
          return Qgis::WkbType::MultiSurface;

        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::Polygon25D:
          return Qgis::WkbType::CurvePolygonZ;

        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::MultiSurfaceZ:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::MultiPolygon25D:
          return Qgis::WkbType::MultiSurfaceZ;

        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::CurvePolygonM:
          return Qgis::WkbType::CurvePolygonM;

        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::MultiSurfaceM:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::TINM:
          return Qgis::WkbType::MultiSurfaceM;

        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::CurvePolygonZM:
          return Qgis::WkbType::CurvePolygonZM;

        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::MultiSurfaceZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
          return Qgis::WkbType::MultiSurfaceZM;

        case Qgis::WkbType::NoGeometry:
          return Qgis::WkbType::NoGeometry;

        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::MultiPoint25D:
          return Qgis::WkbType::MultiPoint25D;
      }
      return Qgis::WkbType::Unknown;
    }

    /**
     * Returns the linear type for a WKB type. For example, for a CompoundCurve, the linear type would be LineString.
     *
     * \see curveType()
     * \see isMultiType()
     * \see isCurvedType()
     * \see singleType()
     * \see flatType()
     * \see multiType()
     *
     * \since QGIS 3.14
     */
    static Qgis::WkbType linearType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {

        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::CompoundCurve:
          return Qgis::WkbType::LineString;

        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::CompoundCurveM:
          return Qgis::WkbType::LineStringM;

        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::CompoundCurveZ:
          return Qgis::WkbType::LineStringZ;

        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::CompoundCurveZM:
          return Qgis::WkbType::LineStringZM;

        case Qgis::WkbType::MultiCurve:
          return Qgis::WkbType::MultiLineString;

        case Qgis::WkbType::MultiCurveM:
          return Qgis::WkbType::MultiLineStringM;

        case Qgis::WkbType::MultiCurveZ:
          return Qgis::WkbType::MultiLineStringZ;

        case Qgis::WkbType::MultiCurveZM:
          return Qgis::WkbType::MultiLineStringZM;

        case Qgis::WkbType::CurvePolygon:
          return Qgis::WkbType::Polygon;

        case Qgis::WkbType::CurvePolygonM:
          return Qgis::WkbType::PolygonM;

        case Qgis::WkbType::CurvePolygonZ:
          return Qgis::WkbType::PolygonZ;

        case Qgis::WkbType::CurvePolygonZM:
          return Qgis::WkbType::PolygonZM;

        case Qgis::WkbType::MultiSurface:
          return Qgis::WkbType::MultiPolygon;

        case Qgis::WkbType::MultiSurfaceM:
          return Qgis::WkbType::MultiPolygonM;

        case Qgis::WkbType::MultiSurfaceZ:
          return Qgis::WkbType::MultiPolygonZ;

        case Qgis::WkbType::MultiSurfaceZM:
          return Qgis::WkbType::MultiPolygonZM;

        case Qgis::WkbType::GeometryCollection:
        case Qgis::WkbType::GeometryCollectionM:
        case Qgis::WkbType::GeometryCollectionZ:
        case Qgis::WkbType::GeometryCollectionZM:
        case Qgis::WkbType::LineString:
        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::MultiLineString:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiLineStringM:
        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::MultiPoint:
        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiPointM:
        case Qgis::WkbType::MultiPointZ:
        case Qgis::WkbType::MultiPointZM:
        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::NoGeometry:
        case Qgis::WkbType::Point:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::PointM:
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TIN:
        case Qgis::WkbType::TINM:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::TINZM:
        case Qgis::WkbType::Triangle:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::TriangleZM:
        case Qgis::WkbType::Unknown:
          return type;

      }
      return Qgis::WkbType::Unknown;
    }

    /**
     * Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * For example, for PolygonZM WKB types the single type would be Polygon.
     * \see singleType()
     * \see multiType()
     * \see curveType()
     */
    static Qgis::WkbType flatType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
          return Qgis::WkbType::Unknown;

        case Qgis::WkbType::Point:
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::PointM:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::Point25D:
          return Qgis::WkbType::Point;

        case Qgis::WkbType::LineString:
        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::LineString25D:
          return Qgis::WkbType::LineString;

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::Polygon25D:
          return Qgis::WkbType::Polygon;

        case Qgis::WkbType::Triangle:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::TriangleZM:
          return Qgis::WkbType::Triangle;

        case Qgis::WkbType::MultiPoint:
        case Qgis::WkbType::MultiPointZ:
        case Qgis::WkbType::MultiPointM:
        case Qgis::WkbType::MultiPointZM:
        case Qgis::WkbType::MultiPoint25D:
          return Qgis::WkbType::MultiPoint;

        case Qgis::WkbType::MultiLineString:
        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::MultiLineStringM:
        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::MultiLineString25D:
          return Qgis::WkbType::MultiLineString;

        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::MultiPolygon25D:
          return Qgis::WkbType::MultiPolygon;

        case Qgis::WkbType::GeometryCollection:
        case Qgis::WkbType::GeometryCollectionZ:
        case Qgis::WkbType::GeometryCollectionM:
        case Qgis::WkbType::GeometryCollectionZM:
          return Qgis::WkbType::GeometryCollection;

        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::CircularStringZM:
          return Qgis::WkbType::CircularString;

        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::CompoundCurveZM:
          return Qgis::WkbType::CompoundCurve;

        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::MultiCurveM:
        case Qgis::WkbType::MultiCurveZM:
          return Qgis::WkbType::MultiCurve;

        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::CurvePolygonZM:
          return Qgis::WkbType::CurvePolygon;

        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::MultiSurfaceZ:
        case Qgis::WkbType::MultiSurfaceM:
        case Qgis::WkbType::MultiSurfaceZM:
          return Qgis::WkbType::MultiSurface;

        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
          return Qgis::WkbType::PolyhedralSurface;

        case Qgis::WkbType::TIN:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::TINM:
        case Qgis::WkbType::TINZM:
          return Qgis::WkbType::TIN;

        case Qgis::WkbType::NoGeometry:
          return Qgis::WkbType::NoGeometry;

      }
      return Qgis::WkbType::Unknown;
    }

    //! Returns the modified input geometry type according to hasZ / hasM
    static Qgis::WkbType zmType( Qgis::WkbType type, bool hasZ, bool hasM ) SIP_HOLDGIL
    {
      type = flatType( type );
      if ( hasZ )
        type = static_cast<Qgis::WkbType>( static_cast<quint32>( type ) + 1000 );
      if ( hasM )
        type = static_cast<Qgis::WkbType>( static_cast<quint32>( type ) + 2000 );
      return type;
    }

    /**
     * Attempts to extract the WKB type from a WKT string.
     * \param wktStr a valid WKT string
     */
    static Qgis::WkbType parseType( const QString &wktStr );

    /**
     * Returns TRUE if the WKB type is a single type.
     * \see isMultiType()
     * \see singleType()
     */
    static bool isSingleType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      return ( type != Qgis::WkbType::Unknown && !isMultiType( type ) );
    }

    /**
     * Returns TRUE if the WKB type is a multi type.
     * \see isSingleType()
     * \see multiType()
     */
    static bool isMultiType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
        case Qgis::WkbType::Point:
        case Qgis::WkbType::LineString:
        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::Triangle:
        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::TIN:
        case Qgis::WkbType::NoGeometry:
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::PointM:
        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::TINM:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::TriangleZM:
        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::Polygon25D:
          return false;

        default:
          return true;

      }
    }

    /**
     * Returns TRUE if the WKB type is a curved type or can contain curved geometries.
     */
    static bool isCurvedType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( flatType( type ) )
      {
        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::MultiSurface:
          return true;

        default:
          return false;
      }
    }

    /**
     * Returns the inherent dimension of the geometry type as an integer. Returned value will
     * always be less than or equal to the coordinate dimension.
     * \returns 0 for point geometries, 1 for line geometries, 2 for polygon geometries
     * Invalid geometry types will return a dimension of 0.
     * \see coordDimensions()
     */
    static int wkbDimensions( Qgis::WkbType type ) SIP_HOLDGIL
    {
      const Qgis::GeometryType gtype = geometryType( type );
      switch ( gtype )
      {
        case Qgis::GeometryType::Line:
          return 1;
        case Qgis::GeometryType::Polygon:
          return 2;
        default: //point, no geometry, unknown geometry
          return 0;
      }
    }

    /**
     * Returns the coordinate dimension of the geometry type as an integer. Returned value will
     * be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
     * Invalid geometry types will return a dimension of 0.
     * \see wkbDimensions()
     */
    static int coordDimensions( Qgis::WkbType type ) SIP_HOLDGIL
    {
      if ( type == Qgis::WkbType::Unknown || type == Qgis::WkbType::NoGeometry )
        return 0;

      return 2 + hasZ( type ) + hasM( type );
    }

    /**
     * Returns the geometry type for a WKB type, e.g., both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     * GeometryCollections are reported as Qgis::GeometryType::Unknown.
     */
    static Qgis::GeometryType geometryType( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::Unknown:
        case Qgis::WkbType::GeometryCollection:
        case Qgis::WkbType::GeometryCollectionZ:
        case Qgis::WkbType::GeometryCollectionM:
        case Qgis::WkbType::GeometryCollectionZM:
          return Qgis::GeometryType::Unknown;

        case Qgis::WkbType::Point:
        case Qgis::WkbType::MultiPoint:
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::MultiPointZ:
        case Qgis::WkbType::PointM:
        case Qgis::WkbType::MultiPointM:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::MultiPointZM:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::MultiPoint25D:
          return Qgis::GeometryType::Point;

        case Qgis::WkbType::LineString:
        case Qgis::WkbType::MultiLineString:
        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::MultiLineStringM:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::CircularString:
        case Qgis::WkbType::CompoundCurve:
        case Qgis::WkbType::MultiCurve:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::MultiCurveM:
        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::MultiCurveZM:
          return Qgis::GeometryType::Line;

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::Triangle:
        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::TriangleZM:
        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::MultiPolygon25D:
        case Qgis::WkbType::CurvePolygon:
        case Qgis::WkbType::MultiSurface:
        case Qgis::WkbType::PolyhedralSurface:
        case Qgis::WkbType::TIN:
        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::MultiSurfaceZ:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::MultiSurfaceM:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::TINM:
        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::MultiSurfaceZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
          return Qgis::GeometryType::Polygon;

        case Qgis::WkbType::NoGeometry:
          return Qgis::GeometryType::Null;
      }

      return Qgis::GeometryType::Unknown;
    }

    /**
     * Returns a non-translated display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
     */
    static QString displayString( Qgis::WkbType type ) SIP_HOLDGIL;

    /**
     * Returns a translated display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
     *
     * \since QGIS 3.18
     */
    static QString translatedDisplayString( Qgis::WkbType type ) SIP_HOLDGIL;

    /**
     * Returns a display string for a geometry type.
     *
     * This will return one of the following strings:
     *
     * - Point
     * - Line
     * - Polygon
     * - Unknown Geometry
     * - No Geometry
     * - Invalid Geometry
     *
     */
    static QString geometryDisplayString( Qgis::GeometryType type ) SIP_HOLDGIL;

    /**
     * Tests whether a WKB type contains the z-dimension.
     * \returns TRUE if type has z values
     * \see addZ()
     * \see hasM()
     */
    static bool hasZ( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::PointZ:
        case Qgis::WkbType::LineStringZ:
        case Qgis::WkbType::PolygonZ:
        case Qgis::WkbType::PolyhedralSurfaceZ:
        case Qgis::WkbType::TINZ:
        case Qgis::WkbType::TriangleZ:
        case Qgis::WkbType::MultiPointZ:
        case Qgis::WkbType::MultiLineStringZ:
        case Qgis::WkbType::MultiPolygonZ:
        case Qgis::WkbType::GeometryCollectionZ:
        case Qgis::WkbType::CircularStringZ:
        case Qgis::WkbType::CompoundCurveZ:
        case Qgis::WkbType::CurvePolygonZ:
        case Qgis::WkbType::MultiCurveZ:
        case Qgis::WkbType::MultiSurfaceZ:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
        case Qgis::WkbType::TriangleZM:
        case Qgis::WkbType::MultiPointZM:
        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::GeometryCollectionZM:
        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::MultiCurveZM:
        case Qgis::WkbType::MultiSurfaceZM:
        case Qgis::WkbType::Point25D:
        case Qgis::WkbType::LineString25D:
        case Qgis::WkbType::Polygon25D:
        case Qgis::WkbType::MultiPoint25D:
        case Qgis::WkbType::MultiLineString25D:
        case Qgis::WkbType::MultiPolygon25D:
          return true;

        default:
          return false;

      }
    }

    /**
     * Tests whether a WKB type contains m values.
     * \returns TRUE if type has m values
     * \see addM()
     * \see hasZ()
     */
    static bool hasM( Qgis::WkbType type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Qgis::WkbType::PointM:
        case Qgis::WkbType::LineStringM:
        case Qgis::WkbType::PolygonM:
        case Qgis::WkbType::PolyhedralSurfaceM:
        case Qgis::WkbType::TINM:
        case Qgis::WkbType::TriangleM:
        case Qgis::WkbType::MultiPointM:
        case Qgis::WkbType::MultiLineStringM:
        case Qgis::WkbType::MultiPolygonM:
        case Qgis::WkbType::GeometryCollectionM:
        case Qgis::WkbType::CircularStringM:
        case Qgis::WkbType::CompoundCurveM:
        case Qgis::WkbType::CurvePolygonM:
        case Qgis::WkbType::MultiCurveM:
        case Qgis::WkbType::MultiSurfaceM:
        case Qgis::WkbType::PointZM:
        case Qgis::WkbType::LineStringZM:
        case Qgis::WkbType::PolygonZM:
        case Qgis::WkbType::PolyhedralSurfaceZM:
        case Qgis::WkbType::TINZM:
        case Qgis::WkbType::TriangleZM:
        case Qgis::WkbType::MultiPointZM:
        case Qgis::WkbType::MultiLineStringZM:
        case Qgis::WkbType::MultiPolygonZM:
        case Qgis::WkbType::GeometryCollectionZM:
        case Qgis::WkbType::CircularStringZM:
        case Qgis::WkbType::CompoundCurveZM:
        case Qgis::WkbType::CurvePolygonZM:
        case Qgis::WkbType::MultiCurveZM:
        case Qgis::WkbType::MultiSurfaceZM:
          return true;

        default:
          return false;

      }
    }

    /**
     * Adds the z dimension to a WKB type and returns the new type
     * \param type original type
     * \see addM()
     * \see dropZ()
     * \see hasZ()
     */
    static Qgis::WkbType addZ( Qgis::WkbType type ) SIP_HOLDGIL
    {
      if ( hasZ( type ) )
        return type;
      else if ( type == Qgis::WkbType::Unknown )
        return Qgis::WkbType::Unknown;
      else if ( type == Qgis::WkbType::NoGeometry )
        return Qgis::WkbType::NoGeometry;

      //upgrade with z dimension
      const Qgis::WkbType flat = flatType( type );
      if ( hasM( type ) )
        return static_cast< Qgis::WkbType >( static_cast< quint32>( flat ) + 3000 );
      else
        return static_cast<Qgis::WkbType >( static_cast< quint32>( flat ) + 1000 );
    }

    /**
     * Adds the m dimension to a WKB type and returns the new type
     * \param type original type
     * \see addZ()
     * \see dropM()
     * \see hasM()
     */
    static Qgis::WkbType addM( Qgis::WkbType type ) SIP_HOLDGIL
    {
      if ( hasM( type ) )
        return type;
      else if ( type == Qgis::WkbType::Unknown )
        return Qgis::WkbType::Unknown;
      else if ( type == Qgis::WkbType::NoGeometry )
        return Qgis::WkbType::NoGeometry;
      else if ( type == Qgis::WkbType::Point25D )
        return Qgis::WkbType::PointZM;
      else if ( type == Qgis::WkbType::LineString25D )
        return Qgis::WkbType::LineStringZM;
      else if ( type == Qgis::WkbType::Polygon25D )
        return Qgis::WkbType::PolygonZM;
      else if ( type == Qgis::WkbType::MultiPoint25D )
        return Qgis::WkbType::MultiPointZM;
      else if ( type == Qgis::WkbType::MultiLineString25D )
        return Qgis::WkbType::MultiLineStringZM;
      else if ( type == Qgis::WkbType::MultiPolygon25D )
        return Qgis::WkbType::MultiPolygonZM;

      //upgrade with m dimension
      const Qgis::WkbType flat = flatType( type );
      if ( hasZ( type ) )
        return static_cast< Qgis::WkbType >( static_cast< quint32 >( flat ) + 3000 );
      else
        return static_cast< Qgis::WkbType >( static_cast< quint32 >( flat ) + 2000 );
    }

    /**
     * Drops the z dimension (if present) for a WKB type and returns the new type.
     * \param type original type
     * \see dropM()
     * \see addZ()
     */
    static Qgis::WkbType dropZ( Qgis::WkbType type ) SIP_HOLDGIL
    {
      if ( !hasZ( type ) )
        return type;

      Qgis::WkbType returnType = flatType( type );
      if ( hasM( type ) )
        returnType = addM( returnType );
      return returnType;
    }

    /**
     * Drops the m dimension (if present) for a WKB type and returns the new type.
     * \param type original type
     * \see dropZ()
     * \see addM()
     */
    static Qgis::WkbType dropM( Qgis::WkbType type ) SIP_HOLDGIL
    {
      if ( !hasM( type ) )
        return type;

      Qgis::WkbType returnType = flatType( type );
      if ( hasZ( type ) )
        returnType = addZ( returnType );
      return returnType;
    }

    /**
     * Will convert the 25D version of the flat type if supported or Unknown if not supported.
     * \param type The type to convert
     * \returns the 25D version of the type or Unknown
     */
    static Qgis::WkbType to25D( Qgis::WkbType type ) SIP_HOLDGIL
    {
      const Qgis::WkbType flat = flatType( type );

      if ( static_cast< quint32 >( flat ) >= static_cast< quint32>( Qgis::WkbType::Point ) && static_cast< quint32 >( flat ) <= static_cast< quint32>( Qgis::WkbType::MultiPolygon ) )
        return static_cast< Qgis::WkbType >( static_cast< quint32 >( flat ) + 0x80000000U );
      else if ( type == Qgis::WkbType::NoGeometry )
        return Qgis::WkbType::NoGeometry;
      else
        return Qgis::WkbType::Unknown;
    }

};

#endif // QGSWKBTYPES_H
