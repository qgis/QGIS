/***************************************************************************
                         qgswkbtypes.cpp
                         ---------------
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

#include "qgswkbtypes.h"

#include "moc_qgswkbtypes.cpp"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/


struct WkbEntry
{
  WkbEntry( const QString &name, bool isMultiType, Qgis::WkbType multiType, Qgis::WkbType singleType, Qgis::WkbType flatType, Qgis::GeometryType geometryType,
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
  Qgis::WkbType mMultiType;
  Qgis::WkbType mSingleType;
  Qgis::WkbType mFlatType;
  Qgis::GeometryType mGeometryType;
  bool mHasZ;
  bool mHasM;
};

typedef  QMap<Qgis::WkbType, WkbEntry> WkbEntries;

Q_GLOBAL_STATIC_WITH_ARGS( WkbEntries, sWkbEntries, (
{
  //register the known wkb types
  { Qgis::WkbType::Unknown, WkbEntry( "Unknown"_L1, false, Qgis::WkbType::Unknown, Qgis::WkbType::Unknown, Qgis::WkbType::Unknown, Qgis::GeometryType::Unknown, false, false ) },
  { Qgis::WkbType::NoGeometry, WkbEntry( "NoGeometry"_L1, false, Qgis::WkbType::NoGeometry, Qgis::WkbType::NoGeometry, Qgis::WkbType::NoGeometry, Qgis::GeometryType::Null, false, false ) },
  //point
  {Qgis::WkbType::Point, WkbEntry( "Point"_L1, false, Qgis::WkbType::MultiPoint, Qgis::WkbType::Point, Qgis::WkbType::Point, Qgis::GeometryType::Point, false, false ) },
  {Qgis::WkbType::PointZ, WkbEntry( "PointZ"_L1, false, Qgis::WkbType::MultiPointZ, Qgis::WkbType::PointZ, Qgis::WkbType::Point, Qgis::GeometryType::Point, true, false ) },
  {Qgis::WkbType::PointM, WkbEntry( "PointM"_L1, false, Qgis::WkbType::MultiPointM, Qgis::WkbType::PointM, Qgis::WkbType::Point, Qgis::GeometryType::Point, false, true ) },
  {Qgis::WkbType::PointZM, WkbEntry( "PointZM"_L1, false, Qgis::WkbType::MultiPointZM, Qgis::WkbType::PointZM, Qgis::WkbType::Point, Qgis::GeometryType::Point, true, true ) },
  {Qgis::WkbType::Point25D, WkbEntry( "Point25D"_L1, false, Qgis::WkbType::MultiPoint25D, Qgis::WkbType::Point25D, Qgis::WkbType::Point, Qgis::GeometryType::Point, true, false ) },
  //linestring
  { Qgis::WkbType::LineString, WkbEntry( "LineString"_L1, false, Qgis::WkbType::MultiLineString, Qgis::WkbType::LineString, Qgis::WkbType::LineString, Qgis::GeometryType::Line, false, false ) },
  { Qgis::WkbType::LineStringZ, WkbEntry( "LineStringZ"_L1, false, Qgis::WkbType::MultiLineStringZ, Qgis::WkbType::LineStringZ, Qgis::WkbType::LineString, Qgis::GeometryType::Line, true, false ) },
  { Qgis::WkbType::LineStringM, WkbEntry( "LineStringM"_L1, false, Qgis::WkbType::MultiLineStringM, Qgis::WkbType::LineStringM, Qgis::WkbType::LineString, Qgis::GeometryType::Line, false, true ) },
  { Qgis::WkbType::LineStringZM, WkbEntry( "LineStringZM"_L1, false, Qgis::WkbType::MultiLineStringZM, Qgis::WkbType::LineStringZM, Qgis::WkbType::LineString, Qgis::GeometryType::Line, true, true ) },
  { Qgis::WkbType::LineString25D, WkbEntry( "LineString25D"_L1, false, Qgis::WkbType::MultiLineString25D, Qgis::WkbType::LineString25D, Qgis::WkbType::LineString, Qgis::GeometryType::Line, true, false ) },
  //circularstring
  { Qgis::WkbType::CircularString, WkbEntry( "CircularString"_L1, false, Qgis::WkbType::MultiCurve, Qgis::WkbType::CircularString, Qgis::WkbType::CircularString, Qgis::GeometryType::Line, false, false ) },
  { Qgis::WkbType::CircularStringZ, WkbEntry( "CircularStringZ"_L1, false, Qgis::WkbType::MultiCurveZ, Qgis::WkbType::CircularStringZ, Qgis::WkbType::CircularString, Qgis::GeometryType::Line, true, false ) },
  { Qgis::WkbType::CircularStringM, WkbEntry( "CircularStringM"_L1, false, Qgis::WkbType::MultiCurveM, Qgis::WkbType::CircularStringM, Qgis::WkbType::CircularString, Qgis::GeometryType::Line, false, true ) },
  { Qgis::WkbType::CircularStringZM, WkbEntry( "CircularStringZM"_L1, false, Qgis::WkbType::MultiCurveZM, Qgis::WkbType::CircularStringZM, Qgis::WkbType::CircularString, Qgis::GeometryType::Line, true, true ) },
  //compoundcurve
  { Qgis::WkbType::CompoundCurve, WkbEntry( "CompoundCurve"_L1, false, Qgis::WkbType::MultiCurve, Qgis::WkbType::CompoundCurve, Qgis::WkbType::CompoundCurve, Qgis::GeometryType::Line, false, false ) },
  { Qgis::WkbType::CompoundCurveZ, WkbEntry( "CompoundCurveZ"_L1, false, Qgis::WkbType::MultiCurveZ, Qgis::WkbType::CompoundCurveZ, Qgis::WkbType::CompoundCurve, Qgis::GeometryType::Line, true, false ) },
  { Qgis::WkbType::CompoundCurveM, WkbEntry( "CompoundCurveM"_L1, false, Qgis::WkbType::MultiCurveM, Qgis::WkbType::CompoundCurveM, Qgis::WkbType::CompoundCurve, Qgis::GeometryType::Line, false, true ) },
  { Qgis::WkbType::CompoundCurveZM, WkbEntry( "CompoundCurveZM"_L1, false, Qgis::WkbType::MultiCurveZM, Qgis::WkbType::CompoundCurveZM, Qgis::WkbType::CompoundCurve, Qgis::GeometryType::Line, true, true ) },
  //polygonQgis::WkbTypes
  { Qgis::WkbType::Polygon, WkbEntry( "Polygon"_L1, false, Qgis::WkbType::MultiPolygon, Qgis::WkbType::Polygon, Qgis::WkbType::Polygon, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::PolygonZ, WkbEntry( "PolygonZ"_L1, false, Qgis::WkbType::MultiPolygonZ, Qgis::WkbType::PolygonZ, Qgis::WkbType::Polygon, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::PolygonM, WkbEntry( "PolygonM"_L1, false, Qgis::WkbType::MultiPolygonM, Qgis::WkbType::PolygonM, Qgis::WkbType::Polygon, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::PolygonZM, WkbEntry( "PolygonZM"_L1, false, Qgis::WkbType::MultiPolygonZM, Qgis::WkbType::PolygonZM, Qgis::WkbType::Polygon, Qgis::GeometryType::Polygon, true, true ) },
  { Qgis::WkbType::Polygon25D, WkbEntry( "Polygon25D"_L1, false, Qgis::WkbType::MultiPolygon25D, Qgis::WkbType::Polygon25D, Qgis::WkbType::Polygon, Qgis::GeometryType::Polygon, true, false ) },
  //triangle
  { Qgis::WkbType::Triangle, WkbEntry( "Triangle"_L1, false, Qgis::WkbType::Unknown, Qgis::WkbType::Triangle, Qgis::WkbType::Triangle, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::TriangleZ, WkbEntry( "TriangleZ"_L1, false, Qgis::WkbType::Unknown, Qgis::WkbType::TriangleZ, Qgis::WkbType::Triangle, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::TriangleM, WkbEntry( "TriangleM"_L1, false, Qgis::WkbType::Unknown, Qgis::WkbType::TriangleM, Qgis::WkbType::Triangle, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::TriangleZM, WkbEntry( "TriangleZM"_L1, false, Qgis::WkbType::Unknown, Qgis::WkbType::TriangleZM, Qgis::WkbType::Triangle, Qgis::GeometryType::Polygon, true, true ) },
  //curvepolygon
  { Qgis::WkbType::CurvePolygon, WkbEntry( "CurvePolygon"_L1, false, Qgis::WkbType::MultiSurface, Qgis::WkbType::CurvePolygon, Qgis::WkbType::CurvePolygon, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::CurvePolygonZ, WkbEntry( "CurvePolygonZ"_L1, false, Qgis::WkbType::MultiSurfaceZ, Qgis::WkbType::CurvePolygonZ, Qgis::WkbType::CurvePolygon, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::CurvePolygonM, WkbEntry( "CurvePolygonM"_L1, false, Qgis::WkbType::MultiSurfaceM, Qgis::WkbType::CurvePolygonM, Qgis::WkbType::CurvePolygon, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::CurvePolygonZM, WkbEntry( "CurvePolygonZM"_L1, false, Qgis::WkbType::MultiSurfaceZM, Qgis::WkbType::CurvePolygonZM, Qgis::WkbType::CurvePolygon, Qgis::GeometryType::Polygon, true, true ) },
  //polyhedralsurface
  { Qgis::WkbType::PolyhedralSurface, WkbEntry( "PolyhedralSurface"_L1, false, Qgis::WkbType::MultiPolygon, Qgis::WkbType::PolyhedralSurface, Qgis::WkbType::PolyhedralSurface, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::PolyhedralSurfaceZ, WkbEntry( "PolyhedralSurfaceZ"_L1, false, Qgis::WkbType::MultiPolygonZ, Qgis::WkbType::PolyhedralSurfaceZ, Qgis::WkbType::PolyhedralSurface, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::PolyhedralSurfaceM, WkbEntry( "PolyhedralSurfaceM"_L1, false, Qgis::WkbType::MultiPolygonM, Qgis::WkbType::PolyhedralSurfaceM, Qgis::WkbType::PolyhedralSurface, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::PolyhedralSurfaceZM, WkbEntry( "PolyhedralSurfaceZM"_L1, false, Qgis::WkbType::MultiPolygonZM, Qgis::WkbType::PolyhedralSurfaceZM, Qgis::WkbType::PolyhedralSurface, Qgis::GeometryType::Polygon, true, true ) },
  //TIN
  { Qgis::WkbType::TIN, WkbEntry( "TIN"_L1, false, Qgis::WkbType::MultiPolygon, Qgis::WkbType::TIN, Qgis::WkbType::TIN, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::TINZ, WkbEntry( "TINZ"_L1, false, Qgis::WkbType::MultiPolygonZ, Qgis::WkbType::TIN, Qgis::WkbType::TINZ, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::TINM, WkbEntry( "TINM"_L1, false, Qgis::WkbType::MultiPolygonM, Qgis::WkbType::TIN, Qgis::WkbType::TINM, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::TINZM, WkbEntry( "TINZM"_L1, false, Qgis::WkbType::MultiPolygonZM, Qgis::WkbType::TIN, Qgis::WkbType::TINZM, Qgis::GeometryType::Polygon, true, true ) },
  //multipoint
  { Qgis::WkbType::MultiPoint, WkbEntry( "MultiPoint"_L1, true, Qgis::WkbType::MultiPoint, Qgis::WkbType::Point, Qgis::WkbType::MultiPoint, Qgis::GeometryType::Point, false, false ) },
  { Qgis::WkbType::MultiPointZ, WkbEntry( "MultiPointZ"_L1, true, Qgis::WkbType::MultiPointZ, Qgis::WkbType::PointZ, Qgis::WkbType::MultiPoint, Qgis::GeometryType::Point, true, false ) },
  { Qgis::WkbType::MultiPointM, WkbEntry( "MultiPointM"_L1, true, Qgis::WkbType::MultiPointM, Qgis::WkbType::PointM, Qgis::WkbType::MultiPoint, Qgis::GeometryType::Point, false, true ) },
  { Qgis::WkbType::MultiPointZM, WkbEntry( "MultiPointZM"_L1, true, Qgis::WkbType::MultiPointZM, Qgis::WkbType::PointZM, Qgis::WkbType::MultiPoint, Qgis::GeometryType::Point, true, true ) },
  { Qgis::WkbType::MultiPoint25D, WkbEntry( "MultiPoint25D"_L1, true, Qgis::WkbType::MultiPoint25D, Qgis::WkbType::Point25D, Qgis::WkbType::MultiPoint, Qgis::GeometryType::Point, true, false ) },
  //multiline
  {Qgis::WkbType::MultiLineString, WkbEntry( "MultiLineString"_L1, true, Qgis::WkbType::MultiLineString, Qgis::WkbType::LineString, Qgis::WkbType::MultiLineString, Qgis::GeometryType::Line, false, false ) },
  {Qgis::WkbType::MultiLineStringZ, WkbEntry( "MultiLineStringZ"_L1, true, Qgis::WkbType::MultiLineStringZ, Qgis::WkbType::LineStringZ, Qgis::WkbType::MultiLineString, Qgis::GeometryType::Line, true, false ) },
  {Qgis::WkbType::MultiLineStringM, WkbEntry( "MultiLineStringM"_L1, true, Qgis::WkbType::MultiLineStringM, Qgis::WkbType::LineStringM, Qgis::WkbType::MultiLineString, Qgis::GeometryType::Line, false, true ) },
  {Qgis::WkbType::MultiLineStringZM, WkbEntry( "MultiLineStringZM"_L1, true, Qgis::WkbType::MultiLineStringZM, Qgis::WkbType::LineStringZM, Qgis::WkbType::MultiLineString, Qgis::GeometryType::Line, true, true ) },
  {Qgis::WkbType::MultiLineString25D, WkbEntry( "MultiLineString25D"_L1, true, Qgis::WkbType::MultiLineString25D, Qgis::WkbType::LineString25D, Qgis::WkbType::MultiLineString, Qgis::GeometryType::Line, true, false ) },
  //multicurve
  {Qgis::WkbType::MultiCurve, WkbEntry( "MultiCurve"_L1, true, Qgis::WkbType::MultiCurve, Qgis::WkbType::CompoundCurve, Qgis::WkbType::MultiCurve, Qgis::GeometryType::Line, false, false ) },
  {Qgis::WkbType::MultiCurveZ, WkbEntry( "MultiCurveZ"_L1, true, Qgis::WkbType::MultiCurveZ, Qgis::WkbType::CompoundCurveZ, Qgis::WkbType::MultiCurve, Qgis::GeometryType::Line, true, false ) },
  {Qgis::WkbType::MultiCurveM, WkbEntry( "MultiCurveM"_L1, true, Qgis::WkbType::MultiCurveM, Qgis::WkbType::CompoundCurveM, Qgis::WkbType::MultiCurve, Qgis::GeometryType::Line, false, true ) },
  {Qgis::WkbType::MultiCurveZM, WkbEntry( "MultiCurveZM"_L1, true, Qgis::WkbType::MultiCurveZM, Qgis::WkbType::CompoundCurveZM, Qgis::WkbType::MultiCurve, Qgis::GeometryType::Line, true, true ) },
  //multipolygon
  { Qgis::WkbType::MultiPolygon, WkbEntry( "MultiPolygon"_L1, true, Qgis::WkbType::MultiPolygon, Qgis::WkbType::Polygon, Qgis::WkbType::MultiPolygon, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::MultiPolygonZ, WkbEntry( "MultiPolygonZ"_L1, true, Qgis::WkbType::MultiPolygonZ, Qgis::WkbType::PolygonZ, Qgis::WkbType::MultiPolygon, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::MultiPolygonM, WkbEntry( "MultiPolygonM"_L1, true, Qgis::WkbType::MultiPolygonM, Qgis::WkbType::PolygonM, Qgis::WkbType::MultiPolygon, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::MultiPolygonZM, WkbEntry( "MultiPolygonZM"_L1, true, Qgis::WkbType::MultiPolygonZM, Qgis::WkbType::PolygonZM, Qgis::WkbType::MultiPolygon, Qgis::GeometryType::Polygon, true, true ) },
  { Qgis::WkbType::MultiPolygon25D, WkbEntry( "MultiPolygon25D"_L1, true, Qgis::WkbType::MultiPolygon25D, Qgis::WkbType::Polygon25D, Qgis::WkbType::MultiPolygon, Qgis::GeometryType::Polygon, true, false ) },
  //multisurface
  { Qgis::WkbType::MultiSurface, WkbEntry( "MultiSurface"_L1, true, Qgis::WkbType::MultiSurface, Qgis::WkbType::CurvePolygon, Qgis::WkbType::MultiSurface, Qgis::GeometryType::Polygon, false, false ) },
  { Qgis::WkbType::MultiSurfaceZ, WkbEntry( "MultiSurfaceZ"_L1, true, Qgis::WkbType::MultiSurfaceZ, Qgis::WkbType::CurvePolygonZ, Qgis::WkbType::MultiSurface, Qgis::GeometryType::Polygon, true, false ) },
  { Qgis::WkbType::MultiSurfaceM, WkbEntry( "MultiSurfaceM"_L1, true, Qgis::WkbType::MultiSurfaceM, Qgis::WkbType::CurvePolygonM, Qgis::WkbType::MultiSurface, Qgis::GeometryType::Polygon, false, true ) },
  { Qgis::WkbType::MultiSurfaceZM, WkbEntry( "MultiSurfaceZM"_L1, true, Qgis::WkbType::MultiSurfaceZM, Qgis::WkbType::CurvePolygonZM, Qgis::WkbType::MultiSurface, Qgis::GeometryType::Polygon, true, true ) },
  //geometrycollection
  { Qgis::WkbType::GeometryCollection, WkbEntry( "GeometryCollection"_L1, true, Qgis::WkbType::GeometryCollection, Qgis::WkbType::Unknown, Qgis::WkbType::GeometryCollection, Qgis::GeometryType::Unknown, false, false ) },
  { Qgis::WkbType::GeometryCollectionZ, WkbEntry( "GeometryCollectionZ"_L1, true, Qgis::WkbType::GeometryCollectionZ, Qgis::WkbType::Unknown, Qgis::WkbType::GeometryCollection, Qgis::GeometryType::Unknown, true, false ) },
  { Qgis::WkbType::GeometryCollectionM, WkbEntry( "GeometryCollectionM"_L1, true, Qgis::WkbType::GeometryCollectionM, Qgis::WkbType::Unknown, Qgis::WkbType::GeometryCollection, Qgis::GeometryType::Unknown, false, true ) },
  { Qgis::WkbType::GeometryCollectionZM, WkbEntry( "GeometryCollectionZM"_L1, true, Qgis::WkbType::GeometryCollectionZM, Qgis::WkbType::Unknown, Qgis::WkbType::GeometryCollection, Qgis::GeometryType::Unknown, true, true ) },
} ) )

Qgis::WkbType QgsWkbTypes::parseType( const QString &wktStr )
{
  const QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<Qgis::WkbType, WkbEntry>::const_iterator it = sWkbEntries()->constBegin();
  for ( ; it != sWkbEntries()->constEnd(); ++it )
  {
    if ( it.value().mName.compare( typestr, Qt::CaseInsensitive ) == 0 )
    {
      return it.key();
    }
  }
  return Qgis::WkbType::Unknown;
}

QString QgsWkbTypes::displayString( Qgis::WkbType type )
{
  const QMap< Qgis::WkbType, WkbEntry >::const_iterator it = sWkbEntries()->constFind( type );
  if ( it == sWkbEntries()->constEnd() )
  {
    return QString();
  }
  return it->mName;
}

QString QgsWkbTypes::translatedDisplayString( Qgis::WkbType type )
{
  switch ( type )
  {
    case Qgis::WkbType::Unknown: return QObject::tr( "Unknown" );
    case Qgis::WkbType::Point: return QObject::tr( "Point" );
    case Qgis::WkbType::LineString: return QObject::tr( "LineString" );
    case Qgis::WkbType::Polygon: return QObject::tr( "Polygon" );
    case Qgis::WkbType::Triangle: return QObject::tr( "Triangle" );
    case Qgis::WkbType::PolyhedralSurface: return QObject::tr( "PolyhedralSurface" );
    case Qgis::WkbType::TIN: return QObject::tr( "TIN" );
    case Qgis::WkbType::MultiPoint: return QObject::tr( "MultiPoint" );
    case Qgis::WkbType::MultiLineString: return QObject::tr( "MultiLine" );
    case Qgis::WkbType::MultiPolygon: return QObject::tr( "MultiPolygon" );
    case Qgis::WkbType::GeometryCollection: return QObject::tr( "GeometryCollection" );
    case Qgis::WkbType::CircularString: return QObject::tr( "CircularString" );
    case Qgis::WkbType::CompoundCurve: return QObject::tr( "CompoundCurve" );
    case Qgis::WkbType::CurvePolygon: return QObject::tr( "CurvePolygon" );
    case Qgis::WkbType::MultiCurve: return QObject::tr( "MultiCurve" );
    case Qgis::WkbType::MultiSurface: return QObject::tr( "MultiSurface" );
    case Qgis::WkbType::NoGeometry: return QObject::tr( "No Geometry" );
    case Qgis::WkbType::PointZ: return QObject::tr( "PointZ" );
    case Qgis::WkbType::LineStringZ: return QObject::tr( "LineStringZ" );
    case Qgis::WkbType::PolygonZ: return QObject::tr( "PolygonZ" );
    case Qgis::WkbType::TriangleZ: return QObject::tr( "TriangleZ" );
    case Qgis::WkbType::PolyhedralSurfaceZ: return QObject::tr( "PolyhedralSurfaceZ" );
    case Qgis::WkbType::TINZ: return QObject::tr( "TINZ" );
    case Qgis::WkbType::MultiPointZ: return QObject::tr( "MultiPointZ" );
    case Qgis::WkbType::MultiLineStringZ: return QObject::tr( "MultiLineZ" );
    case Qgis::WkbType::MultiPolygonZ: return QObject::tr( "MultiPolygonZ" );
    case Qgis::WkbType::GeometryCollectionZ: return QObject::tr( "GeometryCollectionZ" );
    case Qgis::WkbType::CircularStringZ: return QObject::tr( "CircularStringZ" );
    case Qgis::WkbType::CompoundCurveZ: return QObject::tr( "CompoundCurveZ" );
    case Qgis::WkbType::CurvePolygonZ: return QObject::tr( "CurvePolygonZ" );
    case Qgis::WkbType::MultiCurveZ: return QObject::tr( "MultiCurveZ" );
    case Qgis::WkbType::MultiSurfaceZ: return QObject::tr( "MultiSurfaceZ" );
    case Qgis::WkbType::PointM: return QObject::tr( "PointM" );
    case Qgis::WkbType::LineStringM: return QObject::tr( "LineStringM" );
    case Qgis::WkbType::PolygonM: return QObject::tr( "PolygonM" );
    case Qgis::WkbType::TriangleM: return QObject::tr( "TriangleM" );
    case Qgis::WkbType::PolyhedralSurfaceM: return QObject::tr( "PolyhedralSurfaceM" );
    case Qgis::WkbType::TINM: return QObject::tr( "TINM" );
    case Qgis::WkbType::MultiPointM: return QObject::tr( "MultiPointM" );
    case Qgis::WkbType::MultiLineStringM: return QObject::tr( "MultiLineStringM" );
    case Qgis::WkbType::MultiPolygonM: return QObject::tr( "MultiPolygonM" );
    case Qgis::WkbType::GeometryCollectionM: return QObject::tr( "GeometryCollectionM" );
    case Qgis::WkbType::CircularStringM: return QObject::tr( "CircularStringM" );
    case Qgis::WkbType::CompoundCurveM: return QObject::tr( "CompoundCurveM" );
    case Qgis::WkbType::CurvePolygonM: return QObject::tr( "CurvePolygonM" );
    case Qgis::WkbType::MultiCurveM: return QObject::tr( "MultiCurveM" );
    case Qgis::WkbType::MultiSurfaceM: return QObject::tr( "MultiSurfaceM" );
    case Qgis::WkbType::PointZM: return QObject::tr( "PointZM" );
    case Qgis::WkbType::LineStringZM: return QObject::tr( "LineStringZM" );
    case Qgis::WkbType::PolygonZM: return QObject::tr( "PolygonZM" );
    case Qgis::WkbType::PolyhedralSurfaceZM: return QObject::tr( "PolyhedralSurfaceZM" );
    case Qgis::WkbType::TINZM: return QObject::tr( "TINZM" );
    case Qgis::WkbType::MultiPointZM: return QObject::tr( "MultiPointZM" );
    case Qgis::WkbType::MultiLineStringZM: return QObject::tr( "MultiLineZM" );
    case Qgis::WkbType::MultiPolygonZM: return QObject::tr( "MultiPolygonZM" );
    case Qgis::WkbType::GeometryCollectionZM: return QObject::tr( "GeometryCollectionZM" );
    case Qgis::WkbType::CircularStringZM: return QObject::tr( "CircularStringZM" );
    case Qgis::WkbType::CompoundCurveZM: return QObject::tr( "CompoundCurveZM" );
    case Qgis::WkbType::CurvePolygonZM: return QObject::tr( "CurvePolygonZM" );
    case Qgis::WkbType::MultiCurveZM: return QObject::tr( "MultiCurveZM" );
    case Qgis::WkbType::MultiSurfaceZM: return QObject::tr( "MultiSurfaceZM" );
    case Qgis::WkbType::TriangleZM: return QObject::tr( "TriangleZM" );
    case Qgis::WkbType::Point25D: return QObject::tr( "Point25D" );
    case Qgis::WkbType::LineString25D: return QObject::tr( "LineString25D" );
    case Qgis::WkbType::Polygon25D: return QObject::tr( "Polygon25D" );
    case Qgis::WkbType::MultiPoint25D: return QObject::tr( "MultiPoint25D" );
    case Qgis::WkbType::MultiLineString25D: return QObject::tr( "MultiLineString25D" );
    case Qgis::WkbType::MultiPolygon25D: return QObject::tr( "MultiPolygon25D" );
  }
  return QString();
}

QString QgsWkbTypes::geometryDisplayString( Qgis::GeometryType type )
{

  switch ( type )
  {
    case Qgis::GeometryType::Point:
      return u"Point"_s;
    case Qgis::GeometryType::Line:
      return u"Line"_s;
    case Qgis::GeometryType::Polygon:
      return u"Polygon"_s;
    case Qgis::GeometryType::Unknown:
      return u"Unknown geometry"_s;
    case Qgis::GeometryType::Null:
      return u"No geometry"_s;
    default:
      return u"Invalid type"_s;
  }


}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/
