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
    static Type singleType( Type type );

    /** Returns the multi type for a WKB type. Eg, for Polygon WKB types the multi type would be MultiPolygon.
     * @see isMultiType()
     * @see singleType()
     * @see flatType()
     */
    static Type multiType( Type type );

    /** Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * Eg, for PolygonZM WKB types the single type would be Polygon.
     * @see singleType()
     * @see multiType()
     */
    static Type flatType( Type type );

    /** Attempts to extract the WKB type from a WKT string.
     * @param wktStr a valid WKT string
     */
    static Type parseType( const QString& wktStr );

    /** Returns true if the WKB type is a single type.
     * @see isMultiType()
     * @see singleType()
     */
    static bool isSingleType( Type type );

    /** Returns true if the WKB type is a multi type.
     * @see isSingleType()
     * @see multiType()
     */
    static bool isMultiType( Type type );

    /** Returns the inherent dimension of the geometry type as an integer. Returned value will
     * always be less than or equal to the coordinate dimension.
     * @returns 0 for point geometries, 1 for line geometries, 2 for polygon geometries
     * Invalid geometry types will return a dimension of 0.
     * @see coordDimensions()
     */
    static int wkbDimensions( Type type );

    /** Returns the coordinate dimension of the geometry type as an integer. Returned value will
     * be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
     * Invalid geometry types will return a dimension of 0.
     * @note added in QGIS 2.14
     * @see wkbDimensions()
     */
    static int coordDimensions( Type type );

    /** Returns the geometry type for a WKB type, eg both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     */
    static GeometryType geometryType( Type type );

    /** Returns a display string type for a WKB type, eg the geometry name used in WKT geometry representations.
     */
    static QString displayString( Type type );

    /** Tests whether a WKB type contains the z-dimension.
     * @returns true if type has z values
     * @see addZ()
     * @see hasM()
     */
    static bool hasZ( Type type );

    /** Tests whether a WKB type contains m values.
     * @returns true if type has m values
     * @see addM()
     * @see hasZ()
     */
    static bool hasM( Type type );

    /** Adds the z dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addM()
     * @see hasZ()
     */
    static Type addZ( Type type );

    /** Adds the m dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addZ()
     * @see hasM()
     */
    static Type addM( Type type );

  private:

    struct wkbEntry
    {
      wkbEntry( const QString& name, bool isMultiType, Type multiType, Type singleType, Type flatType, GeometryType geometryType,
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

    static QMap<Type, wkbEntry> registerTypes();
    static QMap<Type, wkbEntry>* entries();
};

#endif // QGSWKBTYPES_H
