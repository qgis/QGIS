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
  { QgsWkbTypes::Unknown, WkbEntry( QLatin1String( "Unknown" ), false, QgsWkbTypes::Unknown, QgsWkbTypes::Unknown, QgsWkbTypes::Unknown, QgsWkbTypes::UnknownGeometry, false, false ) },
  { QgsWkbTypes::NoGeometry, WkbEntry( QLatin1String( "NoGeometry" ), false, QgsWkbTypes::NoGeometry, QgsWkbTypes::NoGeometry, QgsWkbTypes::NoGeometry, QgsWkbTypes::NullGeometry, false, false ) },
  //point
  {QgsWkbTypes::Point, WkbEntry( QLatin1String( "Point" ), false, QgsWkbTypes::MultiPoint, QgsWkbTypes::Point, QgsWkbTypes::Point, QgsWkbTypes::PointGeometry, false, false ) },
  {QgsWkbTypes::PointZ, WkbEntry( QLatin1String( "PointZ" ), false, QgsWkbTypes::MultiPointZ, QgsWkbTypes::PointZ, QgsWkbTypes::Point, QgsWkbTypes::PointGeometry, true, false ) },
  {QgsWkbTypes::PointM, WkbEntry( QLatin1String( "PointM" ), false, QgsWkbTypes::MultiPointM, QgsWkbTypes::PointM, QgsWkbTypes::Point, QgsWkbTypes::PointGeometry, false, true ) },
  {QgsWkbTypes::PointZM, WkbEntry( QLatin1String( "PointZM" ), false, QgsWkbTypes::MultiPointZM, QgsWkbTypes::PointZM, QgsWkbTypes::Point, QgsWkbTypes::PointGeometry, true, true ) },
  {QgsWkbTypes::Point25D, WkbEntry( QLatin1String( "Point25D" ), false, QgsWkbTypes::MultiPoint25D, QgsWkbTypes::Point25D, QgsWkbTypes::Point, QgsWkbTypes::PointGeometry, true, false ) },
  //linestring
  { QgsWkbTypes::LineString, WkbEntry( QLatin1String( "LineString" ), false, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineString, QgsWkbTypes::LineString, QgsWkbTypes::LineGeometry, false, false ) },
  { QgsWkbTypes::LineStringZ, WkbEntry( QLatin1String( "LineStringZ" ), false, QgsWkbTypes::MultiLineStringZ, QgsWkbTypes::LineStringZ, QgsWkbTypes::LineString, QgsWkbTypes::LineGeometry, true, false ) },
  { QgsWkbTypes::LineStringM, WkbEntry( QLatin1String( "LineStringM" ), false, QgsWkbTypes::MultiLineStringM, QgsWkbTypes::LineStringM, QgsWkbTypes::LineString, QgsWkbTypes::LineGeometry, false, true ) },
  { QgsWkbTypes::LineStringZM, WkbEntry( QLatin1String( "LineStringZM" ), false, QgsWkbTypes::MultiLineStringZM, QgsWkbTypes::LineStringZM, QgsWkbTypes::LineString, QgsWkbTypes::LineGeometry, true, true ) },
  { QgsWkbTypes::LineString25D, WkbEntry( QLatin1String( "LineString25D" ), false, QgsWkbTypes::MultiLineString25D, QgsWkbTypes::LineString25D, QgsWkbTypes::LineString, QgsWkbTypes::LineGeometry, true, false ) },
  //circularstring
  { QgsWkbTypes::CircularString, WkbEntry( QLatin1String( "CircularString" ), false, QgsWkbTypes::MultiCurve, QgsWkbTypes::CircularString, QgsWkbTypes::CircularString, QgsWkbTypes::LineGeometry, false, false ) },
  { QgsWkbTypes::CircularStringZ, WkbEntry( QLatin1String( "CircularStringZ" ), false, QgsWkbTypes::MultiCurveZ, QgsWkbTypes::CircularStringZ, QgsWkbTypes::CircularString, QgsWkbTypes::LineGeometry, true, false ) },
  { QgsWkbTypes::CircularStringM, WkbEntry( QLatin1String( "CircularStringM" ), false, QgsWkbTypes::MultiCurveM, QgsWkbTypes::CircularStringM, QgsWkbTypes::CircularString, QgsWkbTypes::LineGeometry, false, true ) },
  { QgsWkbTypes::CircularStringZM, WkbEntry( QLatin1String( "CircularStringZM" ), false, QgsWkbTypes::MultiCurveZM, QgsWkbTypes::CircularStringZM, QgsWkbTypes::CircularString, QgsWkbTypes::LineGeometry, true, true ) },
  //compoundcurve
  { QgsWkbTypes::CompoundCurve, WkbEntry( QLatin1String( "CompoundCurve" ), false, QgsWkbTypes::MultiCurve, QgsWkbTypes::CompoundCurve, QgsWkbTypes::CompoundCurve, QgsWkbTypes::LineGeometry, false, false ) },
  { QgsWkbTypes::CompoundCurveZ, WkbEntry( QLatin1String( "CompoundCurveZ" ), false, QgsWkbTypes::MultiCurveZ, QgsWkbTypes::CompoundCurveZ, QgsWkbTypes::CompoundCurve, QgsWkbTypes::LineGeometry, true, false ) },
  { QgsWkbTypes::CompoundCurveM, WkbEntry( QLatin1String( "CompoundCurveM" ), false, QgsWkbTypes::MultiCurveM, QgsWkbTypes::CompoundCurveM, QgsWkbTypes::CompoundCurve, QgsWkbTypes::LineGeometry, false, true ) },
  { QgsWkbTypes::CompoundCurveZM, WkbEntry( QLatin1String( "CompoundCurveZM" ), false, QgsWkbTypes::MultiCurveZM, QgsWkbTypes::CompoundCurveZM, QgsWkbTypes::CompoundCurve, QgsWkbTypes::LineGeometry, true, true ) },
  //polygon
  { QgsWkbTypes::Polygon, WkbEntry( QLatin1String( "Polygon" ), false, QgsWkbTypes::MultiPolygon, QgsWkbTypes::Polygon, QgsWkbTypes::Polygon, QgsWkbTypes::PolygonGeometry, false, false ) },
  { QgsWkbTypes::PolygonZ, WkbEntry( QLatin1String( "PolygonZ" ), false, QgsWkbTypes::MultiPolygonZ, QgsWkbTypes::PolygonZ, QgsWkbTypes::Polygon, QgsWkbTypes::PolygonGeometry, true, false ) },
  { QgsWkbTypes::PolygonM, WkbEntry( QLatin1String( "PolygonM" ), false, QgsWkbTypes::MultiPolygonM, QgsWkbTypes::PolygonM, QgsWkbTypes::Polygon, QgsWkbTypes::PolygonGeometry, false, true ) },
  { QgsWkbTypes::PolygonZM, WkbEntry( QLatin1String( "PolygonZM" ), false, QgsWkbTypes::MultiPolygonZM, QgsWkbTypes::PolygonZM, QgsWkbTypes::Polygon, QgsWkbTypes::PolygonGeometry, true, true ) },
  { QgsWkbTypes::Polygon25D, WkbEntry( QLatin1String( "Polygon25D" ), false, QgsWkbTypes::MultiPolygon25D, QgsWkbTypes::Polygon25D, QgsWkbTypes::Polygon, QgsWkbTypes::PolygonGeometry, true, false ) },
  //triangle
  { QgsWkbTypes::Triangle, WkbEntry( QLatin1String( "Triangle" ), false, QgsWkbTypes::Unknown, QgsWkbTypes::Triangle, QgsWkbTypes::Triangle, QgsWkbTypes::PolygonGeometry, false, false ) },
  { QgsWkbTypes::TriangleZ, WkbEntry( QLatin1String( "TriangleZ" ), false, QgsWkbTypes::Unknown, QgsWkbTypes::TriangleZ, QgsWkbTypes::Triangle, QgsWkbTypes::PolygonGeometry, true, false ) },
  { QgsWkbTypes::TriangleM, WkbEntry( QLatin1String( "TriangleM" ), false, QgsWkbTypes::Unknown, QgsWkbTypes::TriangleM, QgsWkbTypes::Triangle, QgsWkbTypes::PolygonGeometry, false, true ) },
  { QgsWkbTypes::TriangleZM, WkbEntry( QLatin1String( "TriangleZM" ), false, QgsWkbTypes::Unknown, QgsWkbTypes::TriangleZM, QgsWkbTypes::Triangle, QgsWkbTypes::PolygonGeometry, true, true ) },
  //curvepolygon
  { QgsWkbTypes::CurvePolygon, WkbEntry( QLatin1String( "CurvePolygon" ), false, QgsWkbTypes::MultiSurface, QgsWkbTypes::CurvePolygon, QgsWkbTypes::CurvePolygon, QgsWkbTypes::PolygonGeometry, false, false ) },
  { QgsWkbTypes::CurvePolygonZ, WkbEntry( QLatin1String( "CurvePolygonZ" ), false, QgsWkbTypes::MultiSurfaceZ, QgsWkbTypes::CurvePolygonZ, QgsWkbTypes::CurvePolygon, QgsWkbTypes::PolygonGeometry, true, false ) },
  { QgsWkbTypes::CurvePolygonM, WkbEntry( QLatin1String( "CurvePolygonM" ), false, QgsWkbTypes::MultiSurfaceM, QgsWkbTypes::CurvePolygonM, QgsWkbTypes::CurvePolygon, QgsWkbTypes::PolygonGeometry, false, true ) },
  { QgsWkbTypes::CurvePolygonZM, WkbEntry( QLatin1String( "CurvePolygonZM" ), false, QgsWkbTypes::MultiSurfaceZM, QgsWkbTypes::CurvePolygonZM, QgsWkbTypes::CurvePolygon, QgsWkbTypes::PolygonGeometry, true, true ) },
  //multipoint
  { QgsWkbTypes::MultiPoint, WkbEntry( QLatin1String( "MultiPoint" ), true, QgsWkbTypes::MultiPoint, QgsWkbTypes::Point, QgsWkbTypes::MultiPoint, QgsWkbTypes::PointGeometry, false, false ) },
  { QgsWkbTypes::MultiPointZ, WkbEntry( QLatin1String( "MultiPointZ" ), true, QgsWkbTypes::MultiPointZ, QgsWkbTypes::PointZ, QgsWkbTypes::MultiPoint, QgsWkbTypes::PointGeometry, true, false ) },
  { QgsWkbTypes::MultiPointM, WkbEntry( QLatin1String( "MultiPointM" ), true, QgsWkbTypes::MultiPointM, QgsWkbTypes::PointM, QgsWkbTypes::MultiPoint, QgsWkbTypes::PointGeometry, false, true ) },
  { QgsWkbTypes::MultiPointZM, WkbEntry( QLatin1String( "MultiPointZM" ), true, QgsWkbTypes::MultiPointZM, QgsWkbTypes::PointZM, QgsWkbTypes::MultiPoint, QgsWkbTypes::PointGeometry, true, true ) },
  { QgsWkbTypes::MultiPoint25D, WkbEntry( QLatin1String( "MultiPoint25D" ), true, QgsWkbTypes::MultiPoint25D, QgsWkbTypes::Point25D, QgsWkbTypes::MultiPoint, QgsWkbTypes::PointGeometry, true, false ) },
  //multiline
  { QgsWkbTypes::MultiLineString, WkbEntry( QLatin1String( "MultiLineString" ), true, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineString, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineGeometry, false, false ) },
  { QgsWkbTypes::MultiLineStringZ, WkbEntry( QLatin1String( "MultiLineStringZ" ), true, QgsWkbTypes::MultiLineStringZ, QgsWkbTypes::LineStringZ, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineGeometry, true, false ) },
  { QgsWkbTypes::MultiLineStringM, WkbEntry( QLatin1String( "MultiLineStringM" ), true, QgsWkbTypes::MultiLineStringM, QgsWkbTypes::LineStringM, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineGeometry, false, true ) },
  { QgsWkbTypes::MultiLineStringZM, WkbEntry( QLatin1String( "MultiLineStringZM" ), true, QgsWkbTypes::MultiLineStringZM, QgsWkbTypes::LineStringZM, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineGeometry, true, true ) },
  { QgsWkbTypes::MultiLineString25D, WkbEntry( QLatin1String( "MultiLineString25D" ), true, QgsWkbTypes::MultiLineString25D, QgsWkbTypes::LineString25D, QgsWkbTypes::MultiLineString, QgsWkbTypes::LineGeometry, true, false ) },
  //multicurve
  { QgsWkbTypes::MultiCurve, WkbEntry( QLatin1String( "MultiCurve" ), true, QgsWkbTypes::MultiCurve, QgsWkbTypes::CompoundCurve, QgsWkbTypes::MultiCurve, QgsWkbTypes::LineGeometry, false, false ) },
  { QgsWkbTypes::MultiCurveZ, WkbEntry( QLatin1String( "MultiCurveZ" ), true, QgsWkbTypes::MultiCurveZ, QgsWkbTypes::CompoundCurveZ, QgsWkbTypes::MultiCurve, QgsWkbTypes::LineGeometry, true, false ) },
  { QgsWkbTypes::MultiCurveM, WkbEntry( QLatin1String( "MultiCurveM" ), true, QgsWkbTypes::MultiCurveM, QgsWkbTypes::CompoundCurveM, QgsWkbTypes::MultiCurve, QgsWkbTypes::LineGeometry, false, true ) },
  { QgsWkbTypes::MultiCurveZM, WkbEntry( QLatin1String( "MultiCurveZM" ), true, QgsWkbTypes::MultiCurveZM, QgsWkbTypes::CompoundCurveZM, QgsWkbTypes::MultiCurve, QgsWkbTypes::LineGeometry, true, true ) },
  //multipolygon
  { QgsWkbTypes::MultiPolygon, WkbEntry( QLatin1String( "MultiPolygon" ), true, QgsWkbTypes::MultiPolygon, QgsWkbTypes::Polygon, QgsWkbTypes::MultiPolygon, QgsWkbTypes::PolygonGeometry, false, false ) },
  { QgsWkbTypes::MultiPolygonZ, WkbEntry( QLatin1String( "MultiPolygonZ" ), true, QgsWkbTypes::MultiPolygonZ, QgsWkbTypes::PolygonZ, QgsWkbTypes::MultiPolygon, QgsWkbTypes::PolygonGeometry, true, false ) },
  { QgsWkbTypes::MultiPolygonM, WkbEntry( QLatin1String( "MultiPolygonM" ), true, QgsWkbTypes::MultiPolygonM, QgsWkbTypes::PolygonM, QgsWkbTypes::MultiPolygon, QgsWkbTypes::PolygonGeometry, false, true ) },
  { QgsWkbTypes::MultiPolygonZM, WkbEntry( QLatin1String( "MultiPolygonZM" ), true, QgsWkbTypes::MultiPolygonZM, QgsWkbTypes::PolygonZM, QgsWkbTypes::MultiPolygon, QgsWkbTypes::PolygonGeometry, true, true ) },
  { QgsWkbTypes::MultiPolygon25D, WkbEntry( QLatin1String( "MultiPolygon25D" ), true, QgsWkbTypes::MultiPolygon25D, QgsWkbTypes::Polygon25D, QgsWkbTypes::MultiPolygon, QgsWkbTypes::PolygonGeometry, true, false ) },
  //multisurface
  { QgsWkbTypes::MultiSurface, WkbEntry( QLatin1String( "MultiSurface" ), true, QgsWkbTypes::MultiSurface, QgsWkbTypes::CurvePolygon, QgsWkbTypes::MultiSurface, QgsWkbTypes::PolygonGeometry, false, false ) },
  { QgsWkbTypes::MultiSurfaceZ, WkbEntry( QLatin1String( "MultiSurfaceZ" ), true, QgsWkbTypes::MultiSurfaceZ, QgsWkbTypes::CurvePolygonZ, QgsWkbTypes::MultiSurface, QgsWkbTypes::PolygonGeometry, true, false ) },
  { QgsWkbTypes::MultiSurfaceM, WkbEntry( QLatin1String( "MultiSurfaceM" ), true, QgsWkbTypes::MultiSurfaceM, QgsWkbTypes::CurvePolygonM, QgsWkbTypes::MultiSurface, QgsWkbTypes::PolygonGeometry, false, true ) },
  { QgsWkbTypes::MultiSurfaceZM, WkbEntry( QLatin1String( "MultiSurfaceZM" ), true, QgsWkbTypes::MultiSurfaceZM, QgsWkbTypes::CurvePolygonZM, QgsWkbTypes::MultiSurface, QgsWkbTypes::PolygonGeometry, true, true ) },
  //geometrycollection
  { QgsWkbTypes::GeometryCollection, WkbEntry( QLatin1String( "GeometryCollection" ), true, QgsWkbTypes::GeometryCollection, QgsWkbTypes::Unknown, QgsWkbTypes::GeometryCollection, QgsWkbTypes::UnknownGeometry, false, false ) },
  { QgsWkbTypes::GeometryCollectionZ, WkbEntry( QLatin1String( "GeometryCollectionZ" ), true, QgsWkbTypes::GeometryCollectionZ, QgsWkbTypes::Unknown, QgsWkbTypes::GeometryCollection, QgsWkbTypes::UnknownGeometry, true, false ) },
  { QgsWkbTypes::GeometryCollectionM, WkbEntry( QLatin1String( "GeometryCollectionM" ), true, QgsWkbTypes::GeometryCollectionM, QgsWkbTypes::Unknown, QgsWkbTypes::GeometryCollection, QgsWkbTypes::UnknownGeometry, false, true ) },
  { QgsWkbTypes::GeometryCollectionZM, WkbEntry( QLatin1String( "GeometryCollectionZM" ), true, QgsWkbTypes::GeometryCollectionZM, QgsWkbTypes::Unknown, QgsWkbTypes::GeometryCollection, QgsWkbTypes::UnknownGeometry, true, true ) },
} ) )

QgsWkbTypes::Type QgsWkbTypes::parseType( const QString &wktStr )
{
  const QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<QgsWkbTypes::Type, WkbEntry>::const_iterator it = sWkbEntries()->constBegin();
  for ( ; it != sWkbEntries()->constEnd(); ++it )
  {
    if ( it.value().mName.compare( typestr, Qt::CaseInsensitive ) == 0 )
    {
      return it.key();
    }
  }
  return Unknown;
}

QString QgsWkbTypes::displayString( Type type )
{
  const QMap< Type, WkbEntry >::const_iterator it = sWkbEntries()->constFind( type );
  if ( it == sWkbEntries()->constEnd() )
  {
    return QString();
  }
  return it->mName;
}

QString QgsWkbTypes::translatedDisplayString( Type type )
{
  switch ( type )
  {
    case Unknown: return QObject::tr( "Unknown" );
    case Point: return QObject::tr( "Point" );
    case LineString: return QObject::tr( "LineString" );
    case Polygon: return QObject::tr( "Polygon" );
    case Triangle: return QObject::tr( "Triangle" );
    case MultiPoint: return QObject::tr( "MultiPoint" );
    case MultiLineString: return QObject::tr( "MultiLine" );
    case MultiPolygon: return QObject::tr( "MultiPolygon" );
    case GeometryCollection: return QObject::tr( "GeometryCollection" );
    case CircularString: return QObject::tr( "CircularString" );
    case CompoundCurve: return QObject::tr( "CompoundCurve" );
    case CurvePolygon: return QObject::tr( "CurvePolygon" );
    case MultiCurve: return QObject::tr( "MultiCurve" );
    case MultiSurface: return QObject::tr( "MultiSurface" );
    case NoGeometry: return QObject::tr( "No Geometry" );
    case PointZ: return QObject::tr( "PointZ" );
    case LineStringZ: return QObject::tr( "LineStringZ" );
    case PolygonZ: return QObject::tr( "PolygonZ" );
    case TriangleZ: return QObject::tr( "TriangleZ" );
    case MultiPointZ: return QObject::tr( "MultiPointZ" );
    case MultiLineStringZ: return QObject::tr( "MultiLineZ" );
    case MultiPolygonZ: return QObject::tr( "MultiPolygonZ" );
    case GeometryCollectionZ: return QObject::tr( "GeometryCollectionZ" );
    case CircularStringZ: return QObject::tr( "CircularStringZ" );
    case CompoundCurveZ: return QObject::tr( "CompoundCurveZ" );
    case CurvePolygonZ: return QObject::tr( "CurvePolygonZ" );
    case MultiCurveZ: return QObject::tr( "MultiCurveZ" );
    case MultiSurfaceZ: return QObject::tr( "MultiSurfaceZ" );
    case PointM: return QObject::tr( "PointM" );
    case LineStringM: return QObject::tr( "LineStringM" );
    case PolygonM: return QObject::tr( "PolygonM" );
    case TriangleM: return QObject::tr( "TriangleM" );
    case MultiPointM: return QObject::tr( "MultiPointM" );
    case MultiLineStringM: return QObject::tr( "MultiLineStringM" );
    case MultiPolygonM: return QObject::tr( "MultiPolygonM" );
    case GeometryCollectionM: return QObject::tr( "GeometryCollectionM" );
    case CircularStringM: return QObject::tr( "CircularStringM" );
    case CompoundCurveM: return QObject::tr( "CompoundCurveM" );
    case CurvePolygonM: return QObject::tr( "CurvePolygonM" );
    case MultiCurveM: return QObject::tr( "MultiCurveM" );
    case MultiSurfaceM: return QObject::tr( "MultiSurfaceM" );
    case PointZM: return QObject::tr( "PointZM" );
    case LineStringZM: return QObject::tr( "LineStringZM" );
    case PolygonZM: return QObject::tr( "PolygonZM" );
    case MultiPointZM: return QObject::tr( "MultiPointZM" );
    case MultiLineStringZM: return QObject::tr( "MultiLineZM" );
    case MultiPolygonZM: return QObject::tr( "MultiPolygonZM" );
    case GeometryCollectionZM: return QObject::tr( "GeometryCollectionZM" );
    case CircularStringZM: return QObject::tr( "CircularStringZM" );
    case CompoundCurveZM: return QObject::tr( "CompoundCurveZM" );
    case CurvePolygonZM: return QObject::tr( "CurvePolygonZM" );
    case MultiCurveZM: return QObject::tr( "MultiCurveZM" );
    case MultiSurfaceZM: return QObject::tr( "MultiSurfaceZM" );
    case TriangleZM: return QObject::tr( "TriangleZM" );
    case Point25D: return QObject::tr( "Point25D" );
    case LineString25D: return QObject::tr( "LineString25D" );
    case Polygon25D: return QObject::tr( "Polygon25D" );
    case MultiPoint25D: return QObject::tr( "MultiPoint25D" );
    case MultiLineString25D: return QObject::tr( "MultiLineString25D" );
    case MultiPolygon25D: return QObject::tr( "MultiPolygon25D" );
  }
  return QString();
}

QString QgsWkbTypes::geometryDisplayString( QgsWkbTypes::GeometryType type )
{

  switch ( type )
  {
    case PointGeometry:
      return QStringLiteral( "Point" );
    case LineGeometry:
      return QStringLiteral( "Line" );
    case PolygonGeometry:
      return QStringLiteral( "Polygon" );
    case UnknownGeometry:
      return QStringLiteral( "Unknown geometry" );
    case NullGeometry:
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
