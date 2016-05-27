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

#include <QMap>
#include <QString>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsWKBTypes
 * \brief Handles storage of information regarding WKB types and their properties.
 * \note Added in version 2.10
 */

class CORE_EXPORT QgsWKBTypes
{
  public:

    enum Type
    {
      Unknown = 0,
      Point = 1,
      LineString = 2,
      Polygon = 3,
      MultiPoint = 4,
      MultiLineString = 5,
      MultiPolygon = 6,
      GeometryCollection = 7,
      CircularString = 8,
      CompoundCurve = 9,
      CurvePolygon =  10, //13, //should be 10. Seems to be correct in newer postgis versions
      MultiCurve = 11,
      MultiSurface = 12,
      NoGeometry = 100, //attributes only
      PointZ = 1001,
      LineStringZ = 1002,
      PolygonZ = 1003,
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
      Point25D = 0x80000001,
      LineString25D,
      Polygon25D,
      MultiPoint25D,
      MultiLineString25D,
      MultiPolygon25D
    };

    enum GeometryType
    {
      PointGeometry,
      LineGeometry,
      PolygonGeometry,
      UnknownGeometry,
      NullGeometry
    };

    /** Returns the single type for a WKB type. Eg, for MultiPolygon WKB types the single type would be Polygon.
     * @see isSingleType()
     * @see multiType()
     * @see flatType()
     */
    static Type singleType( Type type )
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

    /** Returns the multi type for a WKB type. Eg, for Polygon WKB types the multi type would be MultiPolygon.
     * @see isMultiType()
     * @see singleType()
     * @see flatType()
     */
    static Type multiType( Type type )
    {
      switch ( type )
      {
        case Unknown:
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

    /** Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * Eg, for PolygonZM WKB types the single type would be Polygon.
     * @see singleType()
     * @see multiType()
     */
    static Type flatType( Type type )
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

    /** Returns the modified input geometry type according to hasZ / hasM */
    static Type zmType( Type type, bool hasZ, bool hasM )
    {
      type = flatType( type );
      if ( hasZ )
        type = static_cast<QgsWKBTypes::Type>( static_cast<quint32>( type ) + 1000 );
      if ( hasM )
        type = static_cast<QgsWKBTypes::Type>( static_cast<quint32>( type ) + 2000 );
      return type;
    }

    /** Attempts to extract the WKB type from a WKT string.
     * @param wktStr a valid WKT string
     */
    static Type parseType( const QString& wktStr );

    /** Returns true if the WKB type is a single type.
     * @see isMultiType()
     * @see singleType()
     */
    static bool isSingleType( Type type )
    {
      return ( type != Unknown && !isMultiType( type ) );
    }

    /** Returns true if the WKB type is a multi type.
     * @see isSingleType()
     * @see multiType()
     */
    static bool isMultiType( Type type )
    {
      switch ( type )
      {
        case Unknown:
        case Point:
        case LineString:
        case Polygon:
        case CircularString:
        case CompoundCurve:
        case CurvePolygon:
        case NoGeometry:
        case PointZ:
        case LineStringZ:
        case PolygonZ:
        case CircularStringZ:
        case CompoundCurveZ:
        case CurvePolygonZ:
        case PointM:
        case LineStringM:
        case PolygonM:
        case CircularStringM:
        case CompoundCurveM:
        case CurvePolygonM:
        case PointZM:
        case LineStringZM:
        case PolygonZM:
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

    /** Returns true if the WKB type is a curved type or can contain curved geometries.
     * @note added in QGIS 2.14
     */
    static bool isCurvedType( Type type )
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

    /** Returns the inherent dimension of the geometry type as an integer. Returned value will
     * always be less than or equal to the coordinate dimension.
     * @returns 0 for point geometries, 1 for line geometries, 2 for polygon geometries
     * Invalid geometry types will return a dimension of 0.
     * @see coordDimensions()
     */
    static int wkbDimensions( Type type )
    {
      GeometryType gtype = geometryType( type );
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

    /** Returns the coordinate dimension of the geometry type as an integer. Returned value will
     * be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
     * Invalid geometry types will return a dimension of 0.
     * @note added in QGIS 2.14
     * @see wkbDimensions()
     */
    static int coordDimensions( Type type )
    {
      if ( type == Unknown || type == NoGeometry )
        return 0;

      return 2 + hasZ( type ) + hasM( type );
    }

    /** Returns the geometry type for a WKB type, eg both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     */
    static GeometryType geometryType( Type type )
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
        case PolygonZ:
        case MultiPolygonZ:
        case PolygonM:
        case MultiPolygonM:
        case PolygonZM:
        case MultiPolygonZM:
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

    /** Returns a display string type for a WKB type, eg the geometry name used in WKT geometry representations.
     */
    static QString displayString( Type type );

    /** Tests whether a WKB type contains the z-dimension.
     * @returns true if type has z values
     * @see addZ()
     * @see hasM()
     */
    static bool hasZ( Type type )
    {
      switch ( type )
      {
        case PointZ:
        case LineStringZ:
        case PolygonZ:
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

    /** Tests whether a WKB type contains m values.
     * @returns true if type has m values
     * @see addM()
     * @see hasZ()
     */
    static bool hasM( Type type )
    {
      switch ( type )
      {
        case PointM:
        case LineStringM:
        case PolygonM:
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

    /** Adds the z dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addM()
     * @see dropZ()
     * @see hasZ()
     */
    static Type addZ( Type type )
    {
      if ( hasZ( type ) )
        return type;
      else if ( type == Unknown )
        return Unknown;
      else if ( type == NoGeometry )
        return NoGeometry;

      //upgrade with z dimension
      Type flat = flatType( type );
      if ( hasM( type ) )
        return static_cast< QgsWKBTypes::Type >( flat + 3000 );
      else
        return static_cast< QgsWKBTypes::Type >( flat + 1000 );
    }

    /** Adds the m dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addZ()
     * @see dropM()
     * @see hasM()
     */
    static Type addM( Type type )
    {
      if ( hasM( type ) )
        return type;
      else if ( type == Unknown )
        return Unknown;
      else if ( type == NoGeometry )
        return NoGeometry;
      else if ( type == Point25D ||
                type == LineString25D ||
                type == Polygon25D ||
                type == MultiPoint25D ||
                type == MultiLineString25D ||
                type == MultiPolygon25D )
        return type; //can't add M dimension to these types

      //upgrade with m dimension
      Type flat = flatType( type );
      if ( hasZ( type ) )
        return static_cast< QgsWKBTypes::Type >( flat + 3000 );
      else
        return static_cast< QgsWKBTypes::Type >( flat + 2000 );
    }

    /** Drops the z dimension (if present) for a WKB type and returns the new type.
     * @param type original type
     * @note added in QGIS 2.14
     * @see dropM()
     * @see addZ()
     */
    static Type dropZ( Type type )
    {
      if ( !hasZ( type ) )
        return type;

      QgsWKBTypes::Type returnType = flatType( type );
      if ( hasM( type ) )
        returnType = addM( returnType );
      return returnType;
    }

    /** Drops the m dimension (if present) for a WKB type and returns the new type.
     * @param type original type
     * @note added in QGIS 2.14
     * @see dropZ()
     * @see addM()
     */
    static Type dropM( Type type )
    {
      if ( !hasM( type ) )
        return type;

      QgsWKBTypes::Type returnType = flatType( type );
      if ( hasZ( type ) )
        returnType = addZ( returnType );
      return returnType;
    }

    /**
     * Will convert the 25D version of the flat type if supported or Unknown if not supported.
     * @param type The type to convert
     * @return the 25D version of the type or Unknown
     */
    static Type to25D( Type type )
    {
      QgsWKBTypes::Type flat = flatType( type );

      if ( flat >= Point && flat <= MultiPolygon )
        return static_cast< QgsWKBTypes::Type >( flat + 0x80000000 );
      else if ( type == QgsWKBTypes::NoGeometry )
        return QgsWKBTypes::NoGeometry;
      else
        return Unknown;
    }

  private:

    struct wkbEntry
    {
      wkbEntry( const QString& name, bool isMultiType, Type multiType, Type singleType, Type flatType, GeometryType geometryType,
                bool hasZ, bool hasM )
          : mName( name )
          , mIsMultiType( isMultiType )
          , mMultiType( multiType )
          , mSingleType( singleType )
          , mFlatType( flatType )
          , mGeometryType( geometryType )
          , mHasZ( hasZ )
          , mHasM( hasM )
      {}
      QString mName;
      bool mIsMultiType;
      Type mMultiType;
      Type mSingleType;
      Type mFlatType;
      GeometryType mGeometryType;
      bool mHasZ;
      bool mHasM;
    };

    static QMap<Type, wkbEntry> registerTypes();
    static QMap<Type, wkbEntry>* entries();
};

#endif // QGSWKBTYPES_H
