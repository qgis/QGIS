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

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsWkbTypes
 * \brief Handles storage of information regarding WKB types and their properties.
 * \since QGIS 2.10
 */

class CORE_EXPORT QgsWkbTypes
{
    Q_GADGET
  public:

    /**
     * The WKB type describes the number of dimensions a geometry has
     *
     * - Point
     * - LineString
     * - Polygon
     *
     * as well as the number of dimensions for each individual vertex
     *
     * - X (always)
     * - Y (always)
     * - Z (optional)
     * - M (measurement value, optional)
     *
     * it also has values for multi types, collections, unknown geometry,
     * null geometry, no geometry and curve support.
     *
     * These classes of geometry are often used for data sources to
     * communicate what kind of geometry should be expected for a given
     * geometry field. It is also used for tools or algorithms to decide
     * if they should be available for a given geometry type or act in
     * a different mode.
     */
    enum class Type SIP_MONKEYPATCH_SCOPEENUM : long
    {
      Unknown = 0,
      Point = 1,
      LineString = 2,
      Polygon = 3,
      Triangle = 17,
      MultiPoint = 4,
      MultiLineString = 5,
      MultiPolygon = 6,
      GeometryCollection = 7,
      CircularString = 8,
      CompoundCurve = 9,
      CurvePolygon = 10, //13, //should be 10. Seems to be correct in newer PostGIS versions
      MultiCurve = 11,
      MultiSurface = 12,
      NoGeometry = 100, //attributes only
      PointZ = 1001,
      LineStringZ = 1002,
      PolygonZ = 1003,
      TriangleZ = 1017,
      MultiPointZ = 1004,
      MultiLineStringZ = 1005,
      MultiPolygonZ = 1006,
      GeometryCollectionZ = 1007,
      CircularStringZ = 1008,
      CompoundCurveZ = 1009,
      CurvePolygonZ = 1010,
      MultiCurveZ = 1011,
      MultiSurfaceZ = 1012,
      PointM = 2001,
      LineStringM = 2002,
      PolygonM = 2003,
      TriangleM = 2017,
      MultiPointM = 2004,
      MultiLineStringM = 2005,
      MultiPolygonM = 2006,
      GeometryCollectionM = 2007,
      CircularStringM = 2008,
      CompoundCurveM = 2009,
      CurvePolygonM = 2010,
      MultiCurveM = 2011,
      MultiSurfaceM = 2012,
      PointZM = 3001,
      LineStringZM = 3002,
      PolygonZM = 3003,
      MultiPointZM = 3004,
      MultiLineStringZM = 3005,
      MultiPolygonZM = 3006,
      GeometryCollectionZM = 3007,
      CircularStringZM = 3008,
      CompoundCurveZM = 3009,
      CurvePolygonZM = 3010,
      MultiCurveZM = 3011,
      MultiSurfaceZM = 3012,
      TriangleZM = 3017,
      Point25D = 0x80000001,
      LineString25D,
      Polygon25D,
      MultiPoint25D,
      MultiLineString25D,
      MultiPolygon25D
    };
    Q_ENUM( Type )

    /**
     * The geometry types are used to group QgsWkbTypes::Type in a
     * coarse way.
     *
     * \see geometryType( QgsWkbTypes::Type )
     */
    enum class GeometryType SIP_MONKEYPATCH_SCOPEENUM : int
    {
      PointGeometry,
      LineGeometry,
      PolygonGeometry,
      UnknownGeometry,
      NullGeometry
    };
    Q_ENUM( GeometryType )

    /**
     * Returns the single type for a WKB type. For example, for MultiPolygon WKB types the single type would be Polygon.
     * \see isSingleType()
     * \see multiType()
     * \see curveType()
     * \see flatType()
     */
    static Type singleType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
        case Type::GeometryCollection:
        case Type::GeometryCollectionZ:
        case Type::GeometryCollectionM:
        case Type::GeometryCollectionZM:
          return Type::Unknown;

        case Type::Point:
        case Type::MultiPoint:
          return Type::Point;

        case Type::PointZ:
        case Type::MultiPointZ:
          return Type::PointZ;

        case Type::PointM:
        case Type::MultiPointM:
          return Type::PointM;

        case Type::PointZM:
        case Type::MultiPointZM:
          return Type::PointZM;

        case Type::LineString:
        case Type::MultiLineString:
          return Type::LineString;

        case Type::LineStringZ:
        case Type::MultiLineStringZ:
          return Type::LineStringZ;

        case Type::LineStringM:
        case Type::MultiLineStringM:
          return Type::LineStringM;

        case Type::LineStringZM:
        case Type::MultiLineStringZM:
          return Type::LineStringZM;

        case Type::Polygon:
        case Type::MultiPolygon:
          return Type::Polygon;

        case Type::PolygonZ:
        case Type::MultiPolygonZ:
          return Type::PolygonZ;

        case Type::PolygonM:
        case Type::MultiPolygonM:
          return Type::PolygonM;

        case Type::PolygonZM:
        case Type::MultiPolygonZM:
          return Type::PolygonZM;

        case Type::Triangle:
          // case MultiTriangle:
          return Type::Triangle;

        case Type::TriangleZ:
          // case MultiTriangleZ:
          return Type::TriangleZ;

        case Type::TriangleM:
          // case MultiTriangleM:
          return Type::TriangleM;

        case Type::TriangleZM:
          // case MultiTriangleZM:
          return Type::TriangleZM;

        case Type::CircularString:
          return Type::CircularString;

        case Type::CircularStringZ:
          return Type::CircularStringZ;

        case Type::CircularStringM:
          return Type::CircularStringM;

        case Type::CircularStringZM:
          return Type::CircularStringZM;

        case Type::CompoundCurve:
        case Type::MultiCurve:
          return Type::CompoundCurve;

        case Type::CompoundCurveZ:
        case Type::MultiCurveZ:
          return Type::CompoundCurveZ;

        case Type::CompoundCurveM:
        case Type::MultiCurveM:
          return Type::CompoundCurveM;

        case Type::CompoundCurveZM:
        case Type::MultiCurveZM:
          return Type::CompoundCurveZM;

        case Type::CurvePolygon:
        case Type::MultiSurface:
          return Type::CurvePolygon;

        case Type::CurvePolygonZ:
        case Type::MultiSurfaceZ:
          return Type::CurvePolygonZ;

        case Type::CurvePolygonM:
        case Type::MultiSurfaceM:
          return Type::CurvePolygonM;

        case Type::CurvePolygonZM:
        case Type::MultiSurfaceZM:
          return Type::CurvePolygonZM;

        case Type::NoGeometry:
          return Type::NoGeometry;

        case Type::Point25D:
        case Type::MultiPoint25D:
          return Type::Point25D;

        case Type::LineString25D:
        case Type::MultiLineString25D:
          return Type::LineString25D;

        case Type::Polygon25D:
        case Type::MultiPolygon25D:
          return Type::Polygon25D;

      }
      return Type::Unknown;
    }

    /**
     * Returns the multi type for a WKB type. For example, for Polygon WKB types the multi type would be MultiPolygon.
     * \see isMultiType()
     * \see singleType()
     * \see curveType()
     * \see flatType()
     */
    static Type multiType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
          return Type::Unknown;

        // until we support TIN types, use multipolygon
        case Type::Triangle:
          return Type::MultiPolygon;

        case Type::TriangleZ:
          return Type::MultiPolygonZ;

        case Type::TriangleM:
          return Type::MultiPolygonM;

        case Type::TriangleZM:
          return Type::MultiPolygonZM;

        case Type::GeometryCollection:
          return Type::GeometryCollection;

        case Type::GeometryCollectionZ:
          return Type::GeometryCollectionZ;

        case Type::GeometryCollectionM:
          return Type::GeometryCollectionM;

        case Type::GeometryCollectionZM:
          return Type::GeometryCollectionZM;

        case Type::Point:
        case Type::MultiPoint:
          return Type::MultiPoint;

        case Type::PointZ:
        case Type::MultiPointZ:
          return Type::MultiPointZ;

        case Type::PointM:
        case Type::MultiPointM:
          return Type::MultiPointM;

        case Type::PointZM:
        case Type::MultiPointZM:
          return Type::MultiPointZM;

        case Type::LineString:
        case Type::MultiLineString:
          return Type::MultiLineString;

        case Type::LineStringZ:
        case Type::MultiLineStringZ:
          return Type::MultiLineStringZ;

        case Type::LineStringM:
        case Type::MultiLineStringM:
          return Type::MultiLineStringM;

        case Type::LineStringZM:
        case Type::MultiLineStringZM:
          return Type::MultiLineStringZM;

        case Type::Polygon:
        case Type::MultiPolygon:
          return Type::MultiPolygon;

        case Type::PolygonZ:
        case Type::MultiPolygonZ:
          return Type::MultiPolygonZ;

        case Type::PolygonM:
        case Type::MultiPolygonM:
          return Type::MultiPolygonM;

        case Type::PolygonZM:
        case Type::MultiPolygonZM:
          return Type::MultiPolygonZM;

        case Type::CompoundCurve:
        case Type::CircularString:
        case Type::MultiCurve:
          return Type::MultiCurve;

        case Type::CompoundCurveZ:
        case Type::CircularStringZ:
        case Type::MultiCurveZ:
          return Type::MultiCurveZ;

        case Type::CompoundCurveM:
        case Type::CircularStringM:
        case Type::MultiCurveM:
          return Type::MultiCurveM;

        case Type::CompoundCurveZM:
        case Type::CircularStringZM:
        case Type::MultiCurveZM:
          return Type::MultiCurveZM;

        case Type::CurvePolygon:
        case Type::MultiSurface:
          return Type::MultiSurface;

        case Type::CurvePolygonZ:
        case Type::MultiSurfaceZ:
          return Type::MultiSurfaceZ;

        case Type::CurvePolygonM:
        case Type::MultiSurfaceM:
          return Type::MultiSurfaceM;

        case Type::CurvePolygonZM:
        case Type::MultiSurfaceZM:
          return Type::MultiSurfaceZM;

        case Type::NoGeometry:
          return Type::NoGeometry;

        case Type::Point25D:
        case Type::MultiPoint25D:
          return Type::MultiPoint25D;

        case Type::LineString25D:
        case Type::MultiLineString25D:
          return Type::MultiLineString25D;

        case Type::Polygon25D:
        case Type::MultiPolygon25D:
          return Type::MultiPolygon25D;
      }
      return Type::Unknown;
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
    static Type curveType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
        case Type::Triangle:
        case Type::TriangleZ:
        case Type::TriangleM:
        case Type::TriangleZM:
          return Type::Unknown;

        case Type::GeometryCollection:
          return Type::GeometryCollection;

        case Type::GeometryCollectionZ:
          return Type::GeometryCollectionZ;

        case Type::GeometryCollectionM:
          return Type::GeometryCollectionM;

        case Type::GeometryCollectionZM:
          return Type::GeometryCollectionZM;

        case Type::Point:
          return Type::Point;

        case Type::MultiPoint:
          return Type::MultiPoint;

        case Type::PointZ:
          return Type::PointZ;

        case Type::MultiPointZ:
          return Type::MultiPointZ;

        case Type::PointM:
          return Type::PointM;

        case Type::MultiPointM:
          return Type::MultiPointM;

        case Type::PointZM:
          return Type::PointZM;

        case Type::MultiPointZM:
          return Type::MultiPointZM;

        case Type::LineString:
        case Type::CompoundCurve:
        case Type::CircularString:
          return Type::CompoundCurve;

        case Type::MultiLineString:
        case Type::MultiCurve:
          return Type::MultiCurve;

        case Type::LineStringZ:
        case Type::CompoundCurveZ:
        case Type::CircularStringZ:
        case Type::LineString25D:
          return Type::CompoundCurveZ;

        case Type::MultiLineStringZ:
        case Type::MultiCurveZ:
        case Type::MultiLineString25D:
          return Type::MultiCurveZ;

        case Type::LineStringM:
        case Type::CompoundCurveM:
        case Type::CircularStringM:
          return Type::CompoundCurveM;

        case Type::MultiLineStringM:
        case Type::MultiCurveM:
          return Type::MultiCurveM;

        case Type::LineStringZM:
        case Type::CompoundCurveZM:
        case Type::CircularStringZM:
          return Type::CompoundCurveZM;

        case Type::MultiLineStringZM:
        case Type::MultiCurveZM:
          return Type::MultiCurveZM;

        case Type::Polygon:
        case Type::CurvePolygon:
          return Type::CurvePolygon;

        case Type::MultiPolygon:
        case Type::MultiSurface:
          return Type::MultiSurface;

        case Type::PolygonZ:
        case Type::CurvePolygonZ:
        case Type::Polygon25D:
          return Type::CurvePolygonZ;

        case Type::MultiPolygonZ:
        case Type::MultiSurfaceZ:
        case Type::MultiPolygon25D:
          return Type::MultiSurfaceZ;

        case Type::PolygonM:
        case Type::CurvePolygonM:
          return Type::CurvePolygonM;

        case Type::MultiPolygonM:
        case Type::MultiSurfaceM:
          return Type::MultiSurfaceM;

        case Type::PolygonZM:
        case Type::CurvePolygonZM:
          return Type::CurvePolygonZM;

        case Type::MultiPolygonZM:
        case Type::MultiSurfaceZM:
          return Type::MultiSurfaceZM;

        case Type::NoGeometry:
          return Type::NoGeometry;

        case Type::Point25D:
        case Type::MultiPoint25D:
          return Type::MultiPoint25D;
      }
      return Type::Unknown;
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
    static Type linearType( Type type )
    {
      switch ( type )
      {

        case Type::CircularString:
        case Type::CompoundCurve:
          return Type::LineString;

        case Type::CircularStringM:
        case Type::CompoundCurveM:
          return Type::LineStringM;

        case Type::CircularStringZ:
        case Type::CompoundCurveZ:
          return Type::LineStringZ;

        case Type::CircularStringZM:
        case Type::CompoundCurveZM:
          return Type::LineStringZM;

        case Type::MultiCurve:
          return Type::MultiLineString;

        case Type::MultiCurveM:
          return Type::MultiLineStringM;

        case Type::MultiCurveZ:
          return Type::MultiLineStringZ;

        case Type::MultiCurveZM:
          return Type::MultiLineStringZM;

        case Type::CurvePolygon:
          return Type::Polygon;

        case Type::CurvePolygonM:
          return Type::PolygonM;

        case Type::CurvePolygonZ:
          return Type::PolygonZ;

        case Type::CurvePolygonZM:
          return Type::PolygonZM;

        case Type::MultiSurface:
          return Type::MultiPolygon;

        case Type::MultiSurfaceM:
          return Type::MultiPolygonM;

        case Type::MultiSurfaceZ:
          return Type::MultiPolygonZ;

        case Type::MultiSurfaceZM:
          return Type:: MultiPolygonZM;

        case Type::GeometryCollection:
        case Type::GeometryCollectionM:
        case Type::GeometryCollectionZ:
        case Type::GeometryCollectionZM:
        case Type::LineString:
        case Type::LineString25D:
        case Type::LineStringM:
        case Type::LineStringZ:
        case Type::LineStringZM:
        case Type::MultiLineString:
        case Type::MultiLineString25D:
        case Type::MultiLineStringM:
        case Type::MultiLineStringZ:
        case Type::MultiLineStringZM:
        case Type::MultiPoint:
        case Type::MultiPoint25D:
        case Type::MultiPointM:
        case Type::MultiPointZ:
        case Type::MultiPointZM:
        case Type::MultiPolygon:
        case Type::MultiPolygon25D:
        case Type::MultiPolygonM:
        case Type::MultiPolygonZ:
        case Type:: MultiPolygonZM:
        case Type::NoGeometry:
        case Type::Point:
        case Type::Point25D:
        case Type::PointM:
        case Type::PointZ:
        case Type::PointZM:
        case Type::Polygon:
        case Type::Polygon25D:
        case Type::PolygonM:
        case Type::PolygonZ:
        case Type::PolygonZM:
        case Type::Triangle:
        case Type::TriangleM:
        case Type::TriangleZ:
        case Type::TriangleZM:
        case Type::Unknown:
          return type;

      }
      return Type::Unknown;
    }

    /**
     * Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * For example, for PolygonZM WKB types the single type would be Polygon.
     * \see singleType()
     * \see multiType()
     * \see curveType()
     */
    static Type flatType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
          return Type::Unknown;

        case Type::Point:
        case Type::PointZ:
        case Type::PointM:
        case Type::PointZM:
        case Type::Point25D:
          return Type::Point;

        case Type::LineString:
        case Type::LineStringZ:
        case Type::LineStringM:
        case Type::LineStringZM:
        case Type::LineString25D:
          return Type::LineString;

        case Type::Polygon:
        case Type::PolygonZ:
        case Type::PolygonM:
        case Type::PolygonZM:
        case Type::Polygon25D:
          return Type::Polygon;

        case Type::Triangle:
        case Type::TriangleZ:
        case Type::TriangleM:
        case Type::TriangleZM:
          return Type::Triangle;

        case Type::MultiPoint:
        case Type::MultiPointZ:
        case Type::MultiPointM:
        case Type::MultiPointZM:
        case Type::MultiPoint25D:
          return Type::MultiPoint;

        case Type::MultiLineString:
        case Type::MultiLineStringZ:
        case Type::MultiLineStringM:
        case Type::MultiLineStringZM:
        case Type::MultiLineString25D:
          return Type::MultiLineString;

        case Type::MultiPolygon:
        case Type::MultiPolygonZ:
        case Type::MultiPolygonM:
        case Type:: MultiPolygonZM:
        case Type::MultiPolygon25D:
          return Type::MultiPolygon;

        case Type::GeometryCollection:
        case Type::GeometryCollectionZ:
        case Type::GeometryCollectionM:
        case Type::GeometryCollectionZM:
          return Type::GeometryCollection;

        case Type::CircularString:
        case Type::CircularStringZ:
        case Type::CircularStringM:
        case Type::CircularStringZM:
          return Type::CircularString;

        case Type::CompoundCurve:
        case Type::CompoundCurveZ:
        case Type::CompoundCurveM:
        case Type::CompoundCurveZM:
          return Type::CompoundCurve;

        case Type::MultiCurve:
        case Type::MultiCurveZ:
        case Type::MultiCurveM:
        case Type::MultiCurveZM:
          return Type::MultiCurve;

        case Type::CurvePolygon:
        case Type::CurvePolygonZ:
        case Type::CurvePolygonM:
        case Type::CurvePolygonZM:
          return Type::CurvePolygon;

        case Type::MultiSurface:
        case Type::MultiSurfaceZ:
        case Type::MultiSurfaceM:
        case Type::MultiSurfaceZM:
          return Type::MultiSurface;

        case Type::NoGeometry:
          return Type::NoGeometry;

      }
      return Type::Unknown;
    }

    //! Returns the modified input geometry type according to hasZ / hasM
    static Type zmType( Type type, bool hasZ, bool hasM )
    {
      type = flatType( type );
      if ( hasZ )
        type = static_cast<QgsWkbTypes::Type>( static_cast<quint32>( type ) + 1000 );
      if ( hasM )
        type = static_cast<QgsWkbTypes::Type>( static_cast<quint32>( type ) + 2000 );
      return type;
    }

    /**
     * Attempts to extract the WKB type from a WKT string.
     * \param wktStr a valid WKT string
     */
    static Type parseType( const QString &wktStr );

    /**
     * Returns TRUE if the WKB type is a single type.
     * \see isMultiType()
     * \see singleType()
     */
    static bool isSingleType( Type type )
    {
      return ( type != Type::Unknown && !isMultiType( type ) );
    }

    /**
     * Returns TRUE if the WKB type is a multi type.
     * \see isSingleType()
     * \see multiType()
     */
    static bool isMultiType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
        case Type::Point:
        case Type::LineString:
        case Type::Polygon:
        case Type::Triangle:
        case Type::CircularString:
        case Type::CompoundCurve:
        case Type::CurvePolygon:
        case Type::NoGeometry:
        case Type::PointZ:
        case Type::LineStringZ:
        case Type::PolygonZ:
        case Type::TriangleZ:
        case Type::CircularStringZ:
        case Type::CompoundCurveZ:
        case Type::CurvePolygonZ:
        case Type::PointM:
        case Type::LineStringM:
        case Type::PolygonM:
        case Type::TriangleM:
        case Type::CircularStringM:
        case Type::CompoundCurveM:
        case Type::CurvePolygonM:
        case Type::PointZM:
        case Type::LineStringZM:
        case Type::PolygonZM:
        case Type::TriangleZM:
        case Type::CircularStringZM:
        case Type::CompoundCurveZM:
        case Type::CurvePolygonZM:
        case Type::Point25D:
        case Type::LineString25D:
        case Type::Polygon25D:
          return false;

        default:
          return true;

      }
    }

    /**
     * Returns TRUE if the WKB type is a curved type or can contain curved geometries.
     * \since QGIS 2.14
     */
    static bool isCurvedType( Type type )
    {
      switch ( flatType( type ) )
      {
        case Type::CircularString:
        case Type::CompoundCurve:
        case Type::CurvePolygon:
        case Type::MultiCurve:
        case Type::MultiSurface:
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
    static int wkbDimensions( Type type )
    {
      GeometryType gtype = geometryType( type );
      switch ( gtype )
      {
        case GeometryType::LineGeometry:
          return 1;
        case GeometryType::PolygonGeometry:
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
     * \since QGIS 2.14
     */
    static int coordDimensions( Type type )
    {
      if ( type == Type::Unknown || type == Type::NoGeometry )
        return 0;

      return 2 + hasZ( type ) + hasM( type );
    }

    /**
     * Returns the geometry type for a WKB type, e.g., both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     * GeometryCollections are reported as QgsWkbTypes::GeometryType::UnknownGeometry.
     */
    static GeometryType geometryType( Type type )
    {
      switch ( type )
      {
        case Type::Unknown:
        case Type::GeometryCollection:
        case Type::GeometryCollectionZ:
        case Type::GeometryCollectionM:
        case Type::GeometryCollectionZM:
          return GeometryType::UnknownGeometry;

        case Type::Point:
        case Type::MultiPoint:
        case Type::PointZ:
        case Type::MultiPointZ:
        case Type::PointM:
        case Type::MultiPointM:
        case Type::PointZM:
        case Type::MultiPointZM:
        case Type::Point25D:
        case Type::MultiPoint25D:
          return GeometryType::PointGeometry;

        case Type::LineString:
        case Type::MultiLineString:
        case Type::LineStringZ:
        case Type::MultiLineStringZ:
        case Type::LineStringM:
        case Type::MultiLineStringM:
        case Type::LineStringZM:
        case Type::MultiLineStringZM:
        case Type::LineString25D:
        case Type::MultiLineString25D:
        case Type::CircularString:
        case Type::CompoundCurve:
        case Type::MultiCurve:
        case Type::CircularStringZ:
        case Type::CompoundCurveZ:
        case Type::MultiCurveZ:
        case Type::CircularStringM:
        case Type::CompoundCurveM:
        case Type::MultiCurveM:
        case Type::CircularStringZM:
        case Type::CompoundCurveZM:
        case Type::MultiCurveZM:
          return GeometryType::LineGeometry;

        case Type::Polygon:
        case Type::MultiPolygon:
        case Type::Triangle:
        case Type::PolygonZ:
        case Type::TriangleZ:
        case Type::MultiPolygonZ:
        case Type::PolygonM:
        case Type::TriangleM:
        case Type::MultiPolygonM:
        case Type::PolygonZM:
        case Type:: MultiPolygonZM:
        case Type::TriangleZM:
        case Type::Polygon25D:
        case Type::MultiPolygon25D:
        case Type::CurvePolygon:
        case Type::MultiSurface:
        case Type::CurvePolygonZ:
        case Type::MultiSurfaceZ:
        case Type::CurvePolygonM:
        case Type::MultiSurfaceM:
        case Type::CurvePolygonZM:
        case Type::MultiSurfaceZM:
          return GeometryType::PolygonGeometry;

        case Type::NoGeometry:
          return GeometryType::NullGeometry;
      }

      return GeometryType::UnknownGeometry;
    }

    /**
     * Returns a display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
     */
    static QString displayString( Type type );

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
     * \since QGIS 3.0
     */
    static QString geometryDisplayString( GeometryType type );

    /**
     * Tests whether a WKB type contains the z-dimension.
     * \returns TRUE if type has z values
     * \see addZ()
     * \see hasM()
     */
    static bool hasZ( Type type )
    {
      switch ( type )
      {
        case Type::PointZ:
        case Type::LineStringZ:
        case Type::PolygonZ:
        case Type::TriangleZ:
        case Type::MultiPointZ:
        case Type::MultiLineStringZ:
        case Type::MultiPolygonZ:
        case Type::GeometryCollectionZ:
        case Type::CircularStringZ:
        case Type::CompoundCurveZ:
        case Type::CurvePolygonZ:
        case Type::MultiCurveZ:
        case Type::MultiSurfaceZ:
        case Type::PointZM:
        case Type::LineStringZM:
        case Type::PolygonZM:
        case Type::TriangleZM:
        case Type::MultiPointZM:
        case Type::MultiLineStringZM:
        case Type:: MultiPolygonZM:
        case Type::GeometryCollectionZM:
        case Type::CircularStringZM:
        case Type::CompoundCurveZM:
        case Type::CurvePolygonZM:
        case Type::MultiCurveZM:
        case Type::MultiSurfaceZM:
        case Type::Point25D:
        case Type::LineString25D:
        case Type::Polygon25D:
        case Type::MultiPoint25D:
        case Type::MultiLineString25D:
        case Type::MultiPolygon25D:
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
    static bool hasM( Type type )
    {
      switch ( type )
      {
        case Type::PointM:
        case Type::LineStringM:
        case Type::PolygonM:
        case Type::TriangleM:
        case Type::MultiPointM:
        case Type::MultiLineStringM:
        case Type::MultiPolygonM:
        case Type::GeometryCollectionM:
        case Type::CircularStringM:
        case Type::CompoundCurveM:
        case Type::CurvePolygonM:
        case Type::MultiCurveM:
        case Type::MultiSurfaceM:
        case Type::PointZM:
        case Type::LineStringZM:
        case Type::PolygonZM:
        case Type::TriangleZM:
        case Type::MultiPointZM:
        case Type::MultiLineStringZM:
        case Type:: MultiPolygonZM:
        case Type::GeometryCollectionZM:
        case Type::CircularStringZM:
        case Type::CompoundCurveZM:
        case Type::CurvePolygonZM:
        case Type::MultiCurveZM:
        case Type::MultiSurfaceZM:
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
     * \since QGIS 2.12
     */
    static Type addZ( Type type )
    {
      if ( hasZ( type ) )
        return type;
      else if ( type == Type::Unknown )
        return Type::Unknown;
      else if ( type == Type::NoGeometry )
        return Type::NoGeometry;

      //upgrade with z dimension
      Type flat = flatType( type );
      if ( hasM( type ) )
        return static_cast< QgsWkbTypes::Type >( static_cast<int>( flat ) + 3000 );
      else
        return static_cast< QgsWkbTypes::Type >( static_cast<int>( flat ) + 1000 );
    }

    /**
     * Adds the m dimension to a WKB type and returns the new type
     * \param type original type
     * \see addZ()
     * \see dropM()
     * \see hasM()
     * \since QGIS 2.12
     */
    static Type addM( Type type )
    {
      if ( hasM( type ) )
        return type;
      else if ( type == Type::Unknown )
        return Type::Unknown;
      else if ( type == Type::NoGeometry )
        return Type::NoGeometry;
      else if ( type == Type::Point25D ||
                type == Type::LineString25D ||
                type == Type::Polygon25D ||
                type == Type::MultiPoint25D ||
                type == Type::MultiLineString25D ||
                type == Type::MultiPolygon25D )
        return type; //can't add M dimension to these types

      //upgrade with m dimension
      Type flat = flatType( type );
      if ( hasZ( type ) )
        return static_cast< QgsWkbTypes::Type >( static_cast<int>( flat ) + 3000 );
      else
        return static_cast< QgsWkbTypes::Type >( static_cast<int>( flat ) + 2000 );
    }

    /**
     * Drops the z dimension (if present) for a WKB type and returns the new type.
     * \param type original type
     * \see dropM()
     * \see addZ()
     * \since QGIS 2.14
     */
    static Type dropZ( Type type )
    {
      if ( !hasZ( type ) )
        return type;

      QgsWkbTypes::Type returnType = flatType( type );
      if ( hasM( type ) )
        returnType = addM( returnType );
      return returnType;
    }

    /**
     * Drops the m dimension (if present) for a WKB type and returns the new type.
     * \param type original type
     * \see dropZ()
     * \see addM()
     * \since QGIS 2.14
     */
    static Type dropM( Type type )
    {
      if ( !hasM( type ) )
        return type;

      QgsWkbTypes::Type returnType = flatType( type );
      if ( hasZ( type ) )
        returnType = addZ( returnType );
      return returnType;
    }

    /**
     * Will convert the 25D version of the flat type if supported or Unknown if not supported.
     * \param type The type to convert
     * \returns the 25D version of the type or Unknown
     */
    static Type to25D( Type type )
    {
      QgsWkbTypes::Type flat = flatType( type );

      if ( static_cast<int>( flat ) >= static_cast<int>( Type::Point ) && static_cast<int>( flat ) <= static_cast<int>( Type::MultiPolygon ) )
        return static_cast< QgsWkbTypes::Type >( static_cast<unsigned>( static_cast<int>( flat ) ) + 0x80000000U );
      else if ( type == QgsWkbTypes::Type::NoGeometry )
        return QgsWkbTypes::Type::NoGeometry;
      else
        return Type::Unknown;
    }

};

#endif // QGSWKBTYPES_H
