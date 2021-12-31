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
    enum Type
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
    enum GeometryType
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
    static Type singleType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
        case GeometryCollection:
        case GeometryCollectionZ:
        case GeometryCollectionM:
        case GeometryCollectionZM:
          return Unknown;

        case Point:
        case MultiPoint:
          return Point;

        case PointZ:
        case MultiPointZ:
          return PointZ;

        case PointM:
        case MultiPointM:
          return PointM;

        case PointZM:
        case MultiPointZM:
          return PointZM;

        case LineString:
        case MultiLineString:
          return LineString;

        case LineStringZ:
        case MultiLineStringZ:
          return LineStringZ;

        case LineStringM:
        case MultiLineStringM:
          return LineStringM;

        case LineStringZM:
        case MultiLineStringZM:
          return LineStringZM;

        case Polygon:
        case MultiPolygon:
          return Polygon;

        case PolygonZ:
        case MultiPolygonZ:
          return PolygonZ;

        case PolygonM:
        case MultiPolygonM:
          return PolygonM;

        case PolygonZM:
        case MultiPolygonZM:
          return PolygonZM;

        case Triangle:
          // case MultiTriangle:
          return Triangle;

        case TriangleZ:
          // case MultiTriangleZ:
          return TriangleZ;

        case TriangleM:
          // case MultiTriangleM:
          return TriangleM;

        case TriangleZM:
          // case MultiTriangleZM:
          return TriangleZM;

        case CircularString:
          return CircularString;

        case CircularStringZ:
          return CircularStringZ;

        case CircularStringM:
          return CircularStringM;

        case CircularStringZM:
          return CircularStringZM;

        case CompoundCurve:
        case MultiCurve:
          return CompoundCurve;

        case CompoundCurveZ:
        case MultiCurveZ:
          return CompoundCurveZ;

        case CompoundCurveM:
        case MultiCurveM:
          return CompoundCurveM;

        case CompoundCurveZM:
        case MultiCurveZM:
          return CompoundCurveZM;

        case CurvePolygon:
        case MultiSurface:
          return CurvePolygon;

        case CurvePolygonZ:
        case MultiSurfaceZ:
          return CurvePolygonZ;

        case CurvePolygonM:
        case MultiSurfaceM:
          return CurvePolygonM;

        case CurvePolygonZM:
        case MultiSurfaceZM:
          return CurvePolygonZM;

        case NoGeometry:
          return NoGeometry;

        case Point25D:
        case MultiPoint25D:
          return Point25D;

        case LineString25D:
        case MultiLineString25D:
          return LineString25D;

        case Polygon25D:
        case MultiPolygon25D:
          return Polygon25D;

      }
      return Unknown;
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
    static Type multiType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
          return Unknown;

        // until we support TIN types, use multipolygon
        case Triangle:
          return MultiPolygon;

        case TriangleZ:
          return MultiPolygonZ;

        case TriangleM:
          return MultiPolygonM;

        case TriangleZM:
          return MultiPolygonZM;

        case GeometryCollection:
          return GeometryCollection;

        case GeometryCollectionZ:
          return GeometryCollectionZ;

        case GeometryCollectionM:
          return GeometryCollectionM;

        case GeometryCollectionZM:
          return GeometryCollectionZM;

        case Point:
        case MultiPoint:
          return MultiPoint;

        case PointZ:
        case MultiPointZ:
          return MultiPointZ;

        case PointM:
        case MultiPointM:
          return MultiPointM;

        case PointZM:
        case MultiPointZM:
          return MultiPointZM;

        case LineString:
        case MultiLineString:
          return MultiLineString;

        case LineStringZ:
        case MultiLineStringZ:
          return MultiLineStringZ;

        case LineStringM:
        case MultiLineStringM:
          return MultiLineStringM;

        case LineStringZM:
        case MultiLineStringZM:
          return MultiLineStringZM;

        case Polygon:
        case MultiPolygon:
          return MultiPolygon;

        case PolygonZ:
        case MultiPolygonZ:
          return MultiPolygonZ;

        case PolygonM:
        case MultiPolygonM:
          return MultiPolygonM;

        case PolygonZM:
        case MultiPolygonZM:
          return MultiPolygonZM;

        case CompoundCurve:
        case CircularString:
        case MultiCurve:
          return MultiCurve;

        case CompoundCurveZ:
        case CircularStringZ:
        case MultiCurveZ:
          return MultiCurveZ;

        case CompoundCurveM:
        case CircularStringM:
        case MultiCurveM:
          return MultiCurveM;

        case CompoundCurveZM:
        case CircularStringZM:
        case MultiCurveZM:
          return MultiCurveZM;

        case CurvePolygon:
        case MultiSurface:
          return MultiSurface;

        case CurvePolygonZ:
        case MultiSurfaceZ:
          return MultiSurfaceZ;

        case CurvePolygonM:
        case MultiSurfaceM:
          return MultiSurfaceM;

        case CurvePolygonZM:
        case MultiSurfaceZM:
          return MultiSurfaceZM;

        case NoGeometry:
          return NoGeometry;

        case Point25D:
        case MultiPoint25D:
          return MultiPoint25D;

        case LineString25D:
        case MultiLineString25D:
          return MultiLineString25D;

        case Polygon25D:
        case MultiPolygon25D:
          return MultiPolygon25D;
      }
      return Unknown;
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
    static Type promoteNonPointTypesToMulti( Type type ) SIP_HOLDGIL
    {
      switch ( geometryType( type ) )
      {
        case QgsWkbTypes::PointGeometry:
        case QgsWkbTypes::UnknownGeometry:
        case QgsWkbTypes::NullGeometry:
          return type;

        case QgsWkbTypes::LineGeometry:
        case QgsWkbTypes::PolygonGeometry:
          return multiType( type );
      }
      return Unknown;
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
    static Type curveType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
        case Triangle:
        case TriangleZ:
        case TriangleM:
        case TriangleZM:
          return Unknown;

        case GeometryCollection:
          return GeometryCollection;

        case GeometryCollectionZ:
          return GeometryCollectionZ;

        case GeometryCollectionM:
          return GeometryCollectionM;

        case GeometryCollectionZM:
          return GeometryCollectionZM;

        case Point:
          return Point;

        case MultiPoint:
          return MultiPoint;

        case PointZ:
          return PointZ;

        case MultiPointZ:
          return MultiPointZ;

        case PointM:
          return PointM;

        case MultiPointM:
          return MultiPointM;

        case PointZM:
          return PointZM;

        case MultiPointZM:
          return MultiPointZM;

        case LineString:
        case CompoundCurve:
        case CircularString:
          return CompoundCurve;

        case MultiLineString:
        case MultiCurve:
          return MultiCurve;

        case LineStringZ:
        case CompoundCurveZ:
        case CircularStringZ:
        case LineString25D:
          return CompoundCurveZ;

        case MultiLineStringZ:
        case MultiCurveZ:
        case MultiLineString25D:
          return MultiCurveZ;

        case LineStringM:
        case CompoundCurveM:
        case CircularStringM:
          return CompoundCurveM;

        case MultiLineStringM:
        case MultiCurveM:
          return MultiCurveM;

        case LineStringZM:
        case CompoundCurveZM:
        case CircularStringZM:
          return CompoundCurveZM;

        case MultiLineStringZM:
        case MultiCurveZM:
          return MultiCurveZM;

        case Polygon:
        case CurvePolygon:
          return CurvePolygon;

        case MultiPolygon:
        case MultiSurface:
          return MultiSurface;

        case PolygonZ:
        case CurvePolygonZ:
        case Polygon25D:
          return CurvePolygonZ;

        case MultiPolygonZ:
        case MultiSurfaceZ:
        case MultiPolygon25D:
          return MultiSurfaceZ;

        case PolygonM:
        case CurvePolygonM:
          return CurvePolygonM;

        case MultiPolygonM:
        case MultiSurfaceM:
          return MultiSurfaceM;

        case PolygonZM:
        case CurvePolygonZM:
          return CurvePolygonZM;

        case MultiPolygonZM:
        case MultiSurfaceZM:
          return MultiSurfaceZM;

        case NoGeometry:
          return NoGeometry;

        case Point25D:
        case MultiPoint25D:
          return MultiPoint25D;
      }
      return Unknown;
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
    static Type linearType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {

        case CircularString:
        case CompoundCurve:
          return LineString;

        case CircularStringM:
        case CompoundCurveM:
          return LineStringM;

        case CircularStringZ:
        case CompoundCurveZ:
          return LineStringZ;

        case CircularStringZM:
        case CompoundCurveZM:
          return LineStringZM;

        case MultiCurve:
          return MultiLineString;

        case MultiCurveM:
          return MultiLineStringM;

        case MultiCurveZ:
          return MultiLineStringZ;

        case MultiCurveZM:
          return MultiLineStringZM;

        case CurvePolygon:
          return Polygon;

        case CurvePolygonM:
          return PolygonM;

        case CurvePolygonZ:
          return PolygonZ;

        case CurvePolygonZM:
          return PolygonZM;

        case MultiSurface:
          return MultiPolygon;

        case MultiSurfaceM:
          return MultiPolygonM;

        case MultiSurfaceZ:
          return MultiPolygonZ;

        case MultiSurfaceZM:
          return MultiPolygonZM;

        case GeometryCollection:
        case GeometryCollectionM:
        case GeometryCollectionZ:
        case GeometryCollectionZM:
        case LineString:
        case LineString25D:
        case LineStringM:
        case LineStringZ:
        case LineStringZM:
        case MultiLineString:
        case MultiLineString25D:
        case MultiLineStringM:
        case MultiLineStringZ:
        case MultiLineStringZM:
        case MultiPoint:
        case MultiPoint25D:
        case MultiPointM:
        case MultiPointZ:
        case MultiPointZM:
        case MultiPolygon:
        case MultiPolygon25D:
        case MultiPolygonM:
        case MultiPolygonZ:
        case MultiPolygonZM:
        case NoGeometry:
        case Point:
        case Point25D:
        case PointM:
        case PointZ:
        case PointZM:
        case Polygon:
        case Polygon25D:
        case PolygonM:
        case PolygonZ:
        case PolygonZM:
        case Triangle:
        case TriangleM:
        case TriangleZ:
        case TriangleZM:
        case Unknown:
          return type;

      }
      return Unknown;
    }

    /**
     * Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * For example, for PolygonZM WKB types the single type would be Polygon.
     * \see singleType()
     * \see multiType()
     * \see curveType()
     */
    static Type flatType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
          return Unknown;

        case Point:
        case PointZ:
        case PointM:
        case PointZM:
        case Point25D:
          return Point;

        case LineString:
        case LineStringZ:
        case LineStringM:
        case LineStringZM:
        case LineString25D:
          return LineString;

        case Polygon:
        case PolygonZ:
        case PolygonM:
        case PolygonZM:
        case Polygon25D:
          return Polygon;

        case Triangle:
        case TriangleZ:
        case TriangleM:
        case TriangleZM:
          return Triangle;

        case MultiPoint:
        case MultiPointZ:
        case MultiPointM:
        case MultiPointZM:
        case MultiPoint25D:
          return MultiPoint;

        case MultiLineString:
        case MultiLineStringZ:
        case MultiLineStringM:
        case MultiLineStringZM:
        case MultiLineString25D:
          return MultiLineString;

        case MultiPolygon:
        case MultiPolygonZ:
        case MultiPolygonM:
        case MultiPolygonZM:
        case MultiPolygon25D:
          return MultiPolygon;

        case GeometryCollection:
        case GeometryCollectionZ:
        case GeometryCollectionM:
        case GeometryCollectionZM:
          return GeometryCollection;

        case CircularString:
        case CircularStringZ:
        case CircularStringM:
        case CircularStringZM:
          return CircularString;

        case CompoundCurve:
        case CompoundCurveZ:
        case CompoundCurveM:
        case CompoundCurveZM:
          return CompoundCurve;

        case MultiCurve:
        case MultiCurveZ:
        case MultiCurveM:
        case MultiCurveZM:
          return MultiCurve;

        case CurvePolygon:
        case CurvePolygonZ:
        case CurvePolygonM:
        case CurvePolygonZM:
          return CurvePolygon;

        case MultiSurface:
        case MultiSurfaceZ:
        case MultiSurfaceM:
        case MultiSurfaceZM:
          return MultiSurface;

        case NoGeometry:
          return NoGeometry;

      }
      return Unknown;
    }

    //! Returns the modified input geometry type according to hasZ / hasM
    static Type zmType( Type type, bool hasZ, bool hasM ) SIP_HOLDGIL
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
    static bool isSingleType( Type type ) SIP_HOLDGIL
    {
      return ( type != Unknown && !isMultiType( type ) );
    }

    /**
     * Returns TRUE if the WKB type is a multi type.
     * \see isSingleType()
     * \see multiType()
     */
    static bool isMultiType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
        case Point:
        case LineString:
        case Polygon:
        case Triangle:
        case CircularString:
        case CompoundCurve:
        case CurvePolygon:
        case NoGeometry:
        case PointZ:
        case LineStringZ:
        case PolygonZ:
        case TriangleZ:
        case CircularStringZ:
        case CompoundCurveZ:
        case CurvePolygonZ:
        case PointM:
        case LineStringM:
        case PolygonM:
        case TriangleM:
        case CircularStringM:
        case CompoundCurveM:
        case CurvePolygonM:
        case PointZM:
        case LineStringZM:
        case PolygonZM:
        case TriangleZM:
        case CircularStringZM:
        case CompoundCurveZM:
        case CurvePolygonZM:
        case Point25D:
        case LineString25D:
        case Polygon25D:
          return false;

        default:
          return true;

      }
    }

    /**
     * Returns TRUE if the WKB type is a curved type or can contain curved geometries.
     * \since QGIS 2.14
     */
    static bool isCurvedType( Type type ) SIP_HOLDGIL
    {
      switch ( flatType( type ) )
      {
        case CircularString:
        case CompoundCurve:
        case CurvePolygon:
        case MultiCurve:
        case MultiSurface:
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
    static int wkbDimensions( Type type ) SIP_HOLDGIL
    {
      const GeometryType gtype = geometryType( type );
      switch ( gtype )
      {
        case LineGeometry:
          return 1;
        case PolygonGeometry:
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
    static int coordDimensions( Type type ) SIP_HOLDGIL
    {
      if ( type == Unknown || type == NoGeometry )
        return 0;

      return 2 + hasZ( type ) + hasM( type );
    }

    /**
     * Returns the geometry type for a WKB type, e.g., both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     * GeometryCollections are reported as QgsWkbTypes::UnknownGeometry.
     */
    static GeometryType geometryType( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case Unknown:
        case GeometryCollection:
        case GeometryCollectionZ:
        case GeometryCollectionM:
        case GeometryCollectionZM:
          return UnknownGeometry;

        case Point:
        case MultiPoint:
        case PointZ:
        case MultiPointZ:
        case PointM:
        case MultiPointM:
        case PointZM:
        case MultiPointZM:
        case Point25D:
        case MultiPoint25D:
          return PointGeometry;

        case LineString:
        case MultiLineString:
        case LineStringZ:
        case MultiLineStringZ:
        case LineStringM:
        case MultiLineStringM:
        case LineStringZM:
        case MultiLineStringZM:
        case LineString25D:
        case MultiLineString25D:
        case CircularString:
        case CompoundCurve:
        case MultiCurve:
        case CircularStringZ:
        case CompoundCurveZ:
        case MultiCurveZ:
        case CircularStringM:
        case CompoundCurveM:
        case MultiCurveM:
        case CircularStringZM:
        case CompoundCurveZM:
        case MultiCurveZM:
          return LineGeometry;

        case Polygon:
        case MultiPolygon:
        case Triangle:
        case PolygonZ:
        case TriangleZ:
        case MultiPolygonZ:
        case PolygonM:
        case TriangleM:
        case MultiPolygonM:
        case PolygonZM:
        case MultiPolygonZM:
        case TriangleZM:
        case Polygon25D:
        case MultiPolygon25D:
        case CurvePolygon:
        case MultiSurface:
        case CurvePolygonZ:
        case MultiSurfaceZ:
        case CurvePolygonM:
        case MultiSurfaceM:
        case CurvePolygonZM:
        case MultiSurfaceZM:
          return PolygonGeometry;

        case NoGeometry:
          return NullGeometry;
      }

      return UnknownGeometry;
    }

    /**
     * Returns a non-translated display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
     */
    static QString displayString( Type type ) SIP_HOLDGIL;

    /**
     * Returns a translated display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
     *
     * \since QGIS 3.18
     */
    static QString translatedDisplayString( Type type ) SIP_HOLDGIL;

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
    static QString geometryDisplayString( GeometryType type ) SIP_HOLDGIL;

    /**
     * Tests whether a WKB type contains the z-dimension.
     * \returns TRUE if type has z values
     * \see addZ()
     * \see hasM()
     */
    static bool hasZ( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case PointZ:
        case LineStringZ:
        case PolygonZ:
        case TriangleZ:
        case MultiPointZ:
        case MultiLineStringZ:
        case MultiPolygonZ:
        case GeometryCollectionZ:
        case CircularStringZ:
        case CompoundCurveZ:
        case CurvePolygonZ:
        case MultiCurveZ:
        case MultiSurfaceZ:
        case PointZM:
        case LineStringZM:
        case PolygonZM:
        case TriangleZM:
        case MultiPointZM:
        case MultiLineStringZM:
        case MultiPolygonZM:
        case GeometryCollectionZM:
        case CircularStringZM:
        case CompoundCurveZM:
        case CurvePolygonZM:
        case MultiCurveZM:
        case MultiSurfaceZM:
        case Point25D:
        case LineString25D:
        case Polygon25D:
        case MultiPoint25D:
        case MultiLineString25D:
        case MultiPolygon25D:
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
    static bool hasM( Type type ) SIP_HOLDGIL
    {
      switch ( type )
      {
        case PointM:
        case LineStringM:
        case PolygonM:
        case TriangleM:
        case MultiPointM:
        case MultiLineStringM:
        case MultiPolygonM:
        case GeometryCollectionM:
        case CircularStringM:
        case CompoundCurveM:
        case CurvePolygonM:
        case MultiCurveM:
        case MultiSurfaceM:
        case PointZM:
        case LineStringZM:
        case PolygonZM:
        case TriangleZM:
        case MultiPointZM:
        case MultiLineStringZM:
        case MultiPolygonZM:
        case GeometryCollectionZM:
        case CircularStringZM:
        case CompoundCurveZM:
        case CurvePolygonZM:
        case MultiCurveZM:
        case MultiSurfaceZM:
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
    static Type addZ( Type type ) SIP_HOLDGIL
    {
      if ( hasZ( type ) )
        return type;
      else if ( type == Unknown )
        return Unknown;
      else if ( type == NoGeometry )
        return NoGeometry;

      //upgrade with z dimension
      const Type flat = flatType( type );
      if ( hasM( type ) )
        return static_cast< QgsWkbTypes::Type >( flat + 3000 );
      else
        return static_cast< QgsWkbTypes::Type >( flat + 1000 );
    }

    /**
     * Adds the m dimension to a WKB type and returns the new type
     * \param type original type
     * \see addZ()
     * \see dropM()
     * \see hasM()
     * \since QGIS 2.12
     */
    static Type addM( Type type ) SIP_HOLDGIL
    {
      if ( hasM( type ) )
        return type;
      else if ( type == Unknown )
        return Unknown;
      else if ( type == NoGeometry )
        return NoGeometry;
      else if ( type == Point25D )
        return PointZM;
      else if ( type == LineString25D )
        return LineStringZM;
      else if ( type == Polygon25D )
        return PolygonZM;
      else if ( type == MultiPoint25D )
        return MultiPointZM;
      else if ( type == MultiLineString25D )
        return MultiLineStringZM;
      else if ( type == MultiPolygon25D )
        return MultiPolygonZM;

      //upgrade with m dimension
      const Type flat = flatType( type );
      if ( hasZ( type ) )
        return static_cast< QgsWkbTypes::Type >( flat + 3000 );
      else
        return static_cast< QgsWkbTypes::Type >( flat + 2000 );
    }

    /**
     * Drops the z dimension (if present) for a WKB type and returns the new type.
     * \param type original type
     * \see dropM()
     * \see addZ()
     * \since QGIS 2.14
     */
    static Type dropZ( Type type ) SIP_HOLDGIL
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
    static Type dropM( Type type ) SIP_HOLDGIL
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
    static Type to25D( Type type ) SIP_HOLDGIL
    {
      const QgsWkbTypes::Type flat = flatType( type );

      if ( flat >= Point && flat <= MultiPolygon )
        return static_cast< QgsWkbTypes::Type >( static_cast<unsigned>( flat ) + 0x80000000U );
      else if ( type == QgsWkbTypes::NoGeometry )
        return QgsWkbTypes::NoGeometry;
      else
        return Unknown;
    }

};

#endif // QGSWKBTYPES_H
