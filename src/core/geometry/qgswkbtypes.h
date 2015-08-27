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

/** Class to store information about wkb types.*/
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

    struct wkbEntry
    {
      wkbEntry( QString name, bool isMultiType, Type multiType, Type singleType, Type flatType, GeometryType geometryType,
                bool hasZ, bool hasM ):
          mName( name ), mIsMultiType( isMultiType ), mMultiType( multiType ), mSingleType( singleType ), mFlatType( flatType ), mGeometryType( geometryType ),
          mHasZ( hasZ ), mHasM( hasM ) {}
      QString mName;
      bool mIsMultiType;
      Type mMultiType;
      Type mSingleType;
      Type mFlatType;
      GeometryType mGeometryType;
      bool mHasZ;
      bool mHasM;
    };

    static Type singleType( Type type );
    static Type multiType( Type type );
    static Type flatType( Type type );
    static Type parseType( const QString& wktStr );
    static bool isSingleType( Type type );
    static bool isMultiType( Type type );
    static int wkbDimensions( Type type );
    static GeometryType geometryType( Type type );
    static QString displayString( Type type );
    static bool hasZ( Type type );
    static bool hasM( Type type );

  private:
    static QMap<Type, wkbEntry> registerTypes();
    static QMap<Type, wkbEntry>* entries();
};

#endif // QGSWKBTYPES_H
