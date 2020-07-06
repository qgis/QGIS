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

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/


struct WkbEntry
{
  WkbEntry( const QString &name, bool isMultiType, QgsWkbTypes::Type multiType, QgsWkbTypes::Type singleType, QgsWkbTypes::Type flatType, QgsWkbTypes::GeometryType geometryType,
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
  QgsWkbTypes::Type mMultiType;
  QgsWkbTypes::Type mSingleType;
  QgsWkbTypes::Type mFlatType;
  QgsWkbTypes::GeometryType mGeometryType;
  bool mHasZ;
  bool mHasM;
};

typedef  QMap<QgsWkbTypes::Type, WkbEntry> WkbEntries;

Q_GLOBAL_STATIC_WITH_ARGS( WkbEntries, sWkbEntries, (
{
  //register the known wkb types
  { QgsWkbTypes::Type::Unknown, WkbEntry( QLatin1String( "Unknown" ), false, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::Unknown, QgsWkbTypes::GeometryType::UnknownGeometry, false, false ) },
  { QgsWkbTypes::Type::NoGeometry, WkbEntry( QLatin1String( "NoGeometry" ), false, QgsWkbTypes::Type::NoGeometry, QgsWkbTypes::Type::NoGeometry, QgsWkbTypes::Type::NoGeometry, QgsWkbTypes::GeometryType::NullGeometry, false, false ) },
  //point
  {QgsWkbTypes::Type::Point, WkbEntry( QLatin1String( "Point" ), false, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::Type::Point, QgsWkbTypes::Type::Point, QgsWkbTypes::GeometryType::PointGeometry, false, false ) },
  {QgsWkbTypes::Type::PointZ, WkbEntry( QLatin1String( "PointZ" ), false, QgsWkbTypes::Type::MultiPointZ, QgsWkbTypes::Type::PointZ, QgsWkbTypes::Type::Point, QgsWkbTypes::GeometryType::PointGeometry, true, false ) },
  {QgsWkbTypes::Type::PointM, WkbEntry( QLatin1String( "PointM" ), false, QgsWkbTypes::Type::MultiPointM, QgsWkbTypes::Type::PointM, QgsWkbTypes::Type::Point, QgsWkbTypes::GeometryType::PointGeometry, false, true ) },
  {QgsWkbTypes::Type::PointZM, WkbEntry( QLatin1String( "PointZM" ), false, QgsWkbTypes::Type::MultiPointZM, QgsWkbTypes::Type::PointZM, QgsWkbTypes::Type::Point, QgsWkbTypes::GeometryType::PointGeometry, true, true ) },
  {QgsWkbTypes::Type::Point25D, WkbEntry( QLatin1String( "Point25D" ), false, QgsWkbTypes::Type::MultiPoint25D, QgsWkbTypes::Type::Point25D, QgsWkbTypes::Type::Point, QgsWkbTypes::GeometryType::PointGeometry, true, false ) },
  //linestring
  { QgsWkbTypes::Type::LineString, WkbEntry( QLatin1String( "LineString" ), false, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::Type::LineString, QgsWkbTypes::Type::LineString, QgsWkbTypes::GeometryType::LineGeometry, false, false ) },
  { QgsWkbTypes::Type::LineStringZ, WkbEntry( QLatin1String( "LineStringZ" ), false, QgsWkbTypes::Type::MultiLineStringZ, QgsWkbTypes::Type::LineStringZ, QgsWkbTypes::Type::LineString, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  { QgsWkbTypes::Type::LineStringM, WkbEntry( QLatin1String( "LineStringM" ), false, QgsWkbTypes::Type::MultiLineStringM, QgsWkbTypes::Type::LineStringM, QgsWkbTypes::Type::LineString, QgsWkbTypes::GeometryType::LineGeometry, false, true ) },
  { QgsWkbTypes::Type::LineStringZM, WkbEntry( QLatin1String( "LineStringZM" ), false, QgsWkbTypes::Type::MultiLineStringZM, QgsWkbTypes::Type::LineStringZM, QgsWkbTypes::Type::LineString, QgsWkbTypes::GeometryType::LineGeometry, true, true ) },
  { QgsWkbTypes::Type::LineString25D, WkbEntry( QLatin1String( "LineString25D" ), false, QgsWkbTypes::Type::MultiLineString25D, QgsWkbTypes::Type::LineString25D, QgsWkbTypes::Type::LineString, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  //circularstring
  { QgsWkbTypes::Type::CircularString, WkbEntry( QLatin1String( "CircularString" ), false, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::Type::CircularString, QgsWkbTypes::Type::CircularString, QgsWkbTypes::GeometryType::LineGeometry, false, false ) },
  { QgsWkbTypes::Type::CircularStringZ, WkbEntry( QLatin1String( "CircularStringZ" ), false, QgsWkbTypes::Type::MultiCurveZ, QgsWkbTypes::Type::CircularStringZ, QgsWkbTypes::Type::CircularString, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  { QgsWkbTypes::Type::CircularStringM, WkbEntry( QLatin1String( "CircularStringM" ), false, QgsWkbTypes::Type::MultiCurveM, QgsWkbTypes::Type::CircularStringM, QgsWkbTypes::Type::CircularString, QgsWkbTypes::GeometryType::LineGeometry, false, true ) },
  { QgsWkbTypes::Type::CircularStringZM, WkbEntry( QLatin1String( "CircularStringZM" ), false, QgsWkbTypes::Type::MultiCurveZM, QgsWkbTypes::Type::CircularStringZM, QgsWkbTypes::Type::CircularString, QgsWkbTypes::GeometryType::LineGeometry, true, true ) },
  //compoundcurve
  { QgsWkbTypes::Type::CompoundCurve, WkbEntry( QLatin1String( "CompoundCurve" ), false, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::GeometryType::LineGeometry, false, false ) },
  { QgsWkbTypes::Type::CompoundCurveZ, WkbEntry( QLatin1String( "CompoundCurveZ" ), false, QgsWkbTypes::Type::MultiCurveZ, QgsWkbTypes::Type::CompoundCurveZ, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  { QgsWkbTypes::Type::CompoundCurveM, WkbEntry( QLatin1String( "CompoundCurveM" ), false, QgsWkbTypes::Type::MultiCurveM, QgsWkbTypes::Type::CompoundCurveM, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::GeometryType::LineGeometry, false, true ) },
  { QgsWkbTypes::Type::CompoundCurveZM, WkbEntry( QLatin1String( "CompoundCurveZM" ), false, QgsWkbTypes::Type::MultiCurveZM, QgsWkbTypes::Type::CompoundCurveZM, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::GeometryType::LineGeometry, true, true ) },
  //polygon
  { QgsWkbTypes::Type::Polygon, WkbEntry( QLatin1String( "Polygon" ), false, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::Type::Polygon, QgsWkbTypes::Type::Polygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, false ) },
  { QgsWkbTypes::Type::PolygonZ, WkbEntry( QLatin1String( "PolygonZ" ), false, QgsWkbTypes::Type::MultiPolygonZ, QgsWkbTypes::Type::PolygonZ, QgsWkbTypes::Type::Polygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  { QgsWkbTypes::Type::PolygonM, WkbEntry( QLatin1String( "PolygonM" ), false, QgsWkbTypes::Type::MultiPolygonM, QgsWkbTypes::Type::PolygonM, QgsWkbTypes::Type::Polygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, true ) },
  { QgsWkbTypes::Type::PolygonZM, WkbEntry( QLatin1String( "PolygonZM" ), false, QgsWkbTypes::Type::MultiPolygonZM, QgsWkbTypes::Type::PolygonZM, QgsWkbTypes::Type::Polygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, true ) },
  { QgsWkbTypes::Type::Polygon25D, WkbEntry( QLatin1String( "Polygon25D" ), false, QgsWkbTypes::Type::MultiPolygon25D, QgsWkbTypes::Type::Polygon25D, QgsWkbTypes::Type::Polygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  //triangle
  { QgsWkbTypes::Type::Triangle, WkbEntry( QLatin1String( "Triangle" ), false, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::Triangle, QgsWkbTypes::Type::Triangle, QgsWkbTypes::GeometryType::PolygonGeometry, false, false ) },
  { QgsWkbTypes::Type::TriangleZ, WkbEntry( QLatin1String( "TriangleZ" ), false, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::TriangleZ, QgsWkbTypes::Type::Triangle, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  { QgsWkbTypes::Type::TriangleM, WkbEntry( QLatin1String( "TriangleM" ), false, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::TriangleM, QgsWkbTypes::Type::Triangle, QgsWkbTypes::GeometryType::PolygonGeometry, false, true ) },
  { QgsWkbTypes::Type::TriangleZM, WkbEntry( QLatin1String( "TriangleZM" ), false, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::TriangleZM, QgsWkbTypes::Type::Triangle, QgsWkbTypes::GeometryType::PolygonGeometry, true, true ) },
  //curvepolygon
  { QgsWkbTypes::Type::CurvePolygon, WkbEntry( QLatin1String( "CurvePolygon" ), false, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, false ) },
  { QgsWkbTypes::Type::CurvePolygonZ, WkbEntry( QLatin1String( "CurvePolygonZ" ), false, QgsWkbTypes::Type::MultiSurfaceZ, QgsWkbTypes::Type::CurvePolygonZ, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  { QgsWkbTypes::Type::CurvePolygonM, WkbEntry( QLatin1String( "CurvePolygonM" ), false, QgsWkbTypes::Type::MultiSurfaceM, QgsWkbTypes::Type::CurvePolygonM, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, true ) },
  { QgsWkbTypes::Type::CurvePolygonZM, WkbEntry( QLatin1String( "CurvePolygonZM" ), false, QgsWkbTypes::Type::MultiSurfaceZM, QgsWkbTypes::Type::CurvePolygonZM, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, true ) },
  //multipoint
  { QgsWkbTypes::Type::MultiPoint, WkbEntry( QLatin1String( "MultiPoint" ), true, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::Type::Point, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::GeometryType::PointGeometry, false, false ) },
  { QgsWkbTypes::Type::MultiPointZ, WkbEntry( QLatin1String( "MultiPointZ" ), true, QgsWkbTypes::Type::MultiPointZ, QgsWkbTypes::Type::PointZ, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::GeometryType::PointGeometry, true, false ) },
  { QgsWkbTypes::Type::MultiPointM, WkbEntry( QLatin1String( "MultiPointM" ), true, QgsWkbTypes::Type::MultiPointM, QgsWkbTypes::Type::PointM, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::GeometryType::PointGeometry, false, true ) },
  { QgsWkbTypes::Type::MultiPointZM, WkbEntry( QLatin1String( "MultiPointZM" ), true, QgsWkbTypes::Type::MultiPointZM, QgsWkbTypes::Type::PointZM, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::GeometryType::PointGeometry, true, true ) },
  { QgsWkbTypes::Type::MultiPoint25D, WkbEntry( QLatin1String( "MultiPoint25D" ), true, QgsWkbTypes::Type::MultiPoint25D, QgsWkbTypes::Type::Point25D, QgsWkbTypes::Type::MultiPoint, QgsWkbTypes::GeometryType::PointGeometry, true, false ) },
  //multiline
  { QgsWkbTypes::Type::MultiLineString, WkbEntry( QLatin1String( "MultiLineString" ), true, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::Type::LineString, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::GeometryType::LineGeometry, false, false ) },
  { QgsWkbTypes::Type::MultiLineStringZ, WkbEntry( QLatin1String( "MultiLineStringZ" ), true, QgsWkbTypes::Type::MultiLineStringZ, QgsWkbTypes::Type::LineStringZ, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  { QgsWkbTypes::Type::MultiLineStringM, WkbEntry( QLatin1String( "MultiLineStringM" ), true, QgsWkbTypes::Type::MultiLineStringM, QgsWkbTypes::Type::LineStringM, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::GeometryType::LineGeometry, false, true ) },
  { QgsWkbTypes::Type::MultiLineStringZM, WkbEntry( QLatin1String( "MultiLineStringZM" ), true, QgsWkbTypes::Type::MultiLineStringZM, QgsWkbTypes::Type::LineStringZM, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::GeometryType::LineGeometry, true, true ) },
  { QgsWkbTypes::Type::MultiLineString25D, WkbEntry( QLatin1String( "MultiLineString25D" ), true, QgsWkbTypes::Type::MultiLineString25D, QgsWkbTypes::Type::LineString25D, QgsWkbTypes::Type::MultiLineString, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  //multicurve
  { QgsWkbTypes::Type::MultiCurve, WkbEntry( QLatin1String( "MultiCurve" ), true, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::Type::CompoundCurve, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::GeometryType::LineGeometry, false, false ) },
  { QgsWkbTypes::Type::MultiCurveZ, WkbEntry( QLatin1String( "MultiCurveZ" ), true, QgsWkbTypes::Type::MultiCurveZ, QgsWkbTypes::Type::CompoundCurveZ, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::GeometryType::LineGeometry, true, false ) },
  { QgsWkbTypes::Type::MultiCurveM, WkbEntry( QLatin1String( "MultiCurveM" ), true, QgsWkbTypes::Type::MultiCurveM, QgsWkbTypes::Type::CompoundCurveM, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::GeometryType::LineGeometry, false, true ) },
  { QgsWkbTypes::Type::MultiCurveZM, WkbEntry( QLatin1String( "MultiCurveZM" ), true, QgsWkbTypes::Type::MultiCurveZM, QgsWkbTypes::Type::CompoundCurveZM, QgsWkbTypes::Type::MultiCurve, QgsWkbTypes::GeometryType::LineGeometry, true, true ) },
  //multipolygon
  { QgsWkbTypes::Type::MultiPolygon, WkbEntry( QLatin1String( "MultiPolygon" ), true, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::Type::Polygon, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, false ) },
  { QgsWkbTypes::Type::MultiPolygonZ, WkbEntry( QLatin1String( "MultiPolygonZ" ), true, QgsWkbTypes::Type::MultiPolygonZ, QgsWkbTypes::Type::PolygonZ, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  { QgsWkbTypes::Type::MultiPolygonM, WkbEntry( QLatin1String( "MultiPolygonM" ), true, QgsWkbTypes::Type::MultiPolygonM, QgsWkbTypes::Type::PolygonM, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::GeometryType::PolygonGeometry, false, true ) },
  { QgsWkbTypes::Type::MultiPolygonZM, WkbEntry( QLatin1String( "MultiPolygonZM" ), true, QgsWkbTypes::Type::MultiPolygonZM, QgsWkbTypes::Type::PolygonZM, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, true ) },
  { QgsWkbTypes::Type::MultiPolygon25D, WkbEntry( QLatin1String( "MultiPolygon25D" ), true, QgsWkbTypes::Type::MultiPolygon25D, QgsWkbTypes::Type::Polygon25D, QgsWkbTypes::Type::MultiPolygon, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  //multisurface
  { QgsWkbTypes::Type::MultiSurface, WkbEntry( QLatin1String( "MultiSurface" ), true, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::Type::CurvePolygon, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::GeometryType::PolygonGeometry, false, false ) },
  { QgsWkbTypes::Type::MultiSurfaceZ, WkbEntry( QLatin1String( "MultiSurfaceZ" ), true, QgsWkbTypes::Type::MultiSurfaceZ, QgsWkbTypes::Type::CurvePolygonZ, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::GeometryType::PolygonGeometry, true, false ) },
  { QgsWkbTypes::Type::MultiSurfaceM, WkbEntry( QLatin1String( "MultiSurfaceM" ), true, QgsWkbTypes::Type::MultiSurfaceM, QgsWkbTypes::Type::CurvePolygonM, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::GeometryType::PolygonGeometry, false, true ) },
  { QgsWkbTypes::Type::MultiSurfaceZM, WkbEntry( QLatin1String( "MultiSurfaceZM" ), true, QgsWkbTypes::Type::MultiSurfaceZM, QgsWkbTypes::Type::CurvePolygonZM, QgsWkbTypes::Type::MultiSurface, QgsWkbTypes::GeometryType::PolygonGeometry, true, true ) },
  //geometrycollection
  { QgsWkbTypes::Type::GeometryCollection, WkbEntry( QLatin1String( "GeometryCollection" ), true, QgsWkbTypes::Type::GeometryCollection, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::GeometryCollection, QgsWkbTypes::GeometryType::UnknownGeometry, false, false ) },
  { QgsWkbTypes::Type::GeometryCollectionZ, WkbEntry( QLatin1String( "GeometryCollectionZ" ), true, QgsWkbTypes::Type::GeometryCollectionZ, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::GeometryCollection, QgsWkbTypes::GeometryType::UnknownGeometry, true, false ) },
  { QgsWkbTypes::Type::GeometryCollectionM, WkbEntry( QLatin1String( "GeometryCollectionM" ), true, QgsWkbTypes::Type::GeometryCollectionM, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::GeometryCollection, QgsWkbTypes::GeometryType::UnknownGeometry, false, true ) },
  { QgsWkbTypes::Type::GeometryCollectionZM, WkbEntry( QLatin1String( "GeometryCollectionZM" ), true, QgsWkbTypes::Type::GeometryCollectionZM, QgsWkbTypes::Type::Unknown, QgsWkbTypes::Type::GeometryCollection, QgsWkbTypes::GeometryType::UnknownGeometry, true, true ) },
} ) )

QgsWkbTypes::Type QgsWkbTypes::parseType( const QString &wktStr )
{
  QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<QgsWkbTypes::Type, WkbEntry>::const_iterator it = sWkbEntries()->constBegin();
  for ( ; it != sWkbEntries()->constEnd(); ++it )
  {
    if ( it.value().mName.compare( typestr, Qt::CaseInsensitive ) == 0 )
    {
      return it.key();
    }
  }
  return Type::Unknown;
}

QString QgsWkbTypes::displayString( Type type )
{
  QMap< Type, WkbEntry >::const_iterator it = sWkbEntries()->constFind( type );
  if ( it == sWkbEntries()->constEnd() )
  {
    return QString();
  }
  return it->mName;
}

QString QgsWkbTypes::geometryDisplayString( QgsWkbTypes::GeometryType type )
{

  switch ( type )
  {
    case GeometryType::PointGeometry:
      return QStringLiteral( "Point" );
    case GeometryType::LineGeometry:
      return QStringLiteral( "Line" );
    case GeometryType::PolygonGeometry:
      return QStringLiteral( "Polygon" );
    case GeometryType::UnknownGeometry:
      return QStringLiteral( "Unknown geometry" );
    case GeometryType::NullGeometry:
      return QStringLiteral( "No geometry" );
    default:
      return QStringLiteral( "Invalid type" );
  }


}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/
