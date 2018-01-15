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

const QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry> QgsWkbTypes::ENTRIES
{
  //register the known wkb types
  { Unknown, wkbEntry( QStringLiteral( "Unknown" ), false, Unknown, Unknown, Unknown, UnknownGeometry, false, false ) },
  { NoGeometry, wkbEntry( QStringLiteral( "NoGeometry" ), false, NoGeometry, NoGeometry, NoGeometry, NullGeometry, false, false ) },
  //point
  { Point, wkbEntry( QStringLiteral( "Point" ), false, MultiPoint, Point, Point, PointGeometry, false, false ) },
  { PointZ, wkbEntry( QStringLiteral( "PointZ" ), false, MultiPointZ, PointZ, Point, PointGeometry, true, false ) },
  { PointM, wkbEntry( QStringLiteral( "PointM" ), false, MultiPointM, PointM, Point, PointGeometry, false, true ) },
  { PointZM, wkbEntry( QStringLiteral( "PointZM" ), false, MultiPointZM, PointZM, Point, PointGeometry, true, true ) },
  { Point25D, wkbEntry( QStringLiteral( "Point25D" ), false, MultiPoint25D, Point25D, Point, PointGeometry, true, false ) },
  //linestring
  { LineString, wkbEntry( QStringLiteral( "LineString" ), false, MultiLineString, LineString, LineString, LineGeometry, false, false ) },
  { LineStringZ, wkbEntry( QStringLiteral( "LineStringZ" ), false, MultiLineStringZ, LineStringZ, LineString, LineGeometry, true, false ) },
  { LineStringM, wkbEntry( QStringLiteral( "LineStringM" ), false, MultiLineStringM, LineStringM, LineString, LineGeometry, false, true ) },
  { LineStringZM, wkbEntry( QStringLiteral( "LineStringZM" ), false, MultiLineStringZM, LineStringZM, LineString, LineGeometry, true, true ) },
  { LineString25D, wkbEntry( QStringLiteral( "LineString25D" ), false, MultiLineString25D, LineString25D, LineString, LineGeometry, true, false ) },
  //circularstring
  { CircularString, wkbEntry( QStringLiteral( "CircularString" ), false, MultiCurve, CircularString, CircularString, LineGeometry, false, false ) },
  { CircularStringZ, wkbEntry( QStringLiteral( "CircularStringZ" ), false, MultiCurveZ, CircularStringZ, CircularString, LineGeometry, true, false ) },
  { CircularStringM, wkbEntry( QStringLiteral( "CircularStringM" ), false, MultiCurveM, CircularStringM, CircularString, LineGeometry, false, true ) },
  { CircularStringZM, wkbEntry( QStringLiteral( "CircularStringZM" ), false, MultiCurveZM, CircularStringZM, CircularString, LineGeometry, true, true ) },
  //compoundcurve
  { CompoundCurve, wkbEntry( QStringLiteral( "CompoundCurve" ), false, MultiCurve, CompoundCurve, CompoundCurve, LineGeometry, false, false ) },
  { CompoundCurveZ, wkbEntry( QStringLiteral( "CompoundCurveZ" ), false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineGeometry, true, false ) },
  { CompoundCurveM, wkbEntry( QStringLiteral( "CompoundCurveM" ), false, MultiCurveM, CompoundCurveM, CompoundCurve, LineGeometry, false, true ) },
  { CompoundCurveZM, wkbEntry( QStringLiteral( "CompoundCurveZM" ), false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineGeometry, true, true ) },
  //polygon
  { Polygon, wkbEntry( QStringLiteral( "Polygon" ), false, MultiPolygon, Polygon, Polygon, PolygonGeometry, false, false ) },
  { PolygonZ, wkbEntry( QStringLiteral( "PolygonZ" ), false, MultiPolygonZ, PolygonZ, Polygon, PolygonGeometry, true, false ) },
  { PolygonM, wkbEntry( QStringLiteral( "PolygonM" ), false, MultiPolygonM, PolygonM, Polygon, PolygonGeometry, false, true ) },
  { PolygonZM, wkbEntry( QStringLiteral( "PolygonZM" ), false, MultiPolygonZM, PolygonZM, Polygon, PolygonGeometry, true, true ) },
  { Polygon25D, wkbEntry( QStringLiteral( "Polygon25D" ), false, MultiPolygon25D, Polygon25D, Polygon, PolygonGeometry, true, false ) },
  //triangle
  { Triangle, wkbEntry( QStringLiteral( "Triangle" ), false, Unknown, Triangle, Triangle, PolygonGeometry, false, false ) },
  { TriangleZ, wkbEntry( QStringLiteral( "TriangleZ" ), false, Unknown, TriangleZ, Triangle, PolygonGeometry, true, false ) },
  { TriangleM, wkbEntry( QStringLiteral( "TriangleM" ), false, Unknown, TriangleM, Triangle, PolygonGeometry, false, true ) },
  { TriangleZM, wkbEntry( QStringLiteral( "TriangleZM" ), false, Unknown, TriangleZM, Triangle, PolygonGeometry, true, true ) },
  //curvepolygon
  { CurvePolygon, wkbEntry( QStringLiteral( "CurvePolygon" ), false, MultiSurface, CurvePolygon, CurvePolygon, PolygonGeometry, false, false ) },
  { CurvePolygonZ, wkbEntry( QStringLiteral( "CurvePolygonZ" ), false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonGeometry, true, false ) },
  { CurvePolygonM, wkbEntry( QStringLiteral( "CurvePolygonM" ), false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonGeometry, false, true ) },
  { CurvePolygonZM, wkbEntry( QStringLiteral( "CurvePolygonZM" ), false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonGeometry, true, true ) },
  //multipoint
  { MultiPoint, wkbEntry( QStringLiteral( "MultiPoint" ), true, MultiPoint, Point, MultiPoint, PointGeometry, false, false ) },
  { MultiPointZ, wkbEntry( QStringLiteral( "MultiPointZ" ), true, MultiPointZ, PointZ, MultiPoint, PointGeometry, true, false ) },
  { MultiPointM, wkbEntry( QStringLiteral( "MultiPointM" ), true, MultiPointM, PointM, MultiPoint, PointGeometry, false, true ) },
  { MultiPointZM, wkbEntry( QStringLiteral( "MultiPointZM" ), true, MultiPointZM, PointZM, MultiPoint, PointGeometry, true, true ) },
  { MultiPoint25D, wkbEntry( QStringLiteral( "MultiPoint25D" ), true, MultiPoint25D, Point25D, MultiPoint, PointGeometry, true, false ) },
  //multiline
  { MultiLineString, wkbEntry( QStringLiteral( "MultiLineString" ), true, MultiLineString, LineString, MultiLineString, LineGeometry, false, false ) },
  { MultiLineStringZ, wkbEntry( QStringLiteral( "MultiLineStringZ" ), true, MultiLineStringZ, LineStringZ, MultiLineString, LineGeometry, true, false ) },
  { MultiLineStringM, wkbEntry( QStringLiteral( "MultiLineStringM" ), true, MultiLineStringM, LineStringM, MultiLineString, LineGeometry, false, true ) },
  { MultiLineStringZM, wkbEntry( QStringLiteral( "MultiLineStringZM" ), true, MultiLineStringZM, LineStringZM, MultiLineString, LineGeometry, true, true ) },
  { MultiLineString25D, wkbEntry( QStringLiteral( "MultiLineString25D" ), true, MultiLineString25D, LineString25D, MultiLineString, LineGeometry, true, false ) },
  //multicurve
  { MultiCurve, wkbEntry( QStringLiteral( "MultiCurve" ), true, MultiCurve, CompoundCurve, MultiCurve, LineGeometry, false, false ) },
  { MultiCurveZ, wkbEntry( QStringLiteral( "MultiCurveZ" ), true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineGeometry, true, false ) },
  { MultiCurveM, wkbEntry( QStringLiteral( "MultiCurveM" ), true, MultiCurveM, CompoundCurveM, MultiCurve, LineGeometry, false, true ) },
  { MultiCurveZM, wkbEntry( QStringLiteral( "MultiCurveZM" ), true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineGeometry, true, true ) },
  //multipolygon
  { MultiPolygon, wkbEntry( QStringLiteral( "MultiPolygon" ), true, MultiPolygon, Polygon, MultiPolygon, PolygonGeometry, false, false ) },
  { MultiPolygonZ, wkbEntry( QStringLiteral( "MultiPolygonZ" ), true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonGeometry, true, false ) },
  { MultiPolygonM, wkbEntry( QStringLiteral( "MultiPolygonM" ), true, MultiPolygonM, PolygonM, MultiPolygon, PolygonGeometry, false, true ) },
  { MultiPolygonZM, wkbEntry( QStringLiteral( "MultiPolygonZM" ), true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonGeometry, true, true ) },
  { MultiPolygon25D, wkbEntry( QStringLiteral( "MultiPolygon25D" ), true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonGeometry, true, false ) },
  //multisurface
  { MultiSurface, wkbEntry( QStringLiteral( "MultiSurface" ), true, MultiSurface, CurvePolygon, MultiSurface, PolygonGeometry, false, false ) },
  { MultiSurfaceZ, wkbEntry( QStringLiteral( "MultiSurfaceZ" ), true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonGeometry, true, false ) },
  { MultiSurfaceM, wkbEntry( QStringLiteral( "MultiSurfaceM" ), true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonGeometry, false, true ) },
  { MultiSurfaceZM, wkbEntry( QStringLiteral( "MultiSurfaceZM" ), true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonGeometry, true, true ) },
  //geometrycollection
  { GeometryCollection, wkbEntry( QStringLiteral( "GeometryCollection" ), true, GeometryCollection, Unknown, GeometryCollection, UnknownGeometry, false, false ) },
  { GeometryCollectionZ, wkbEntry( QStringLiteral( "GeometryCollectionZ" ), true, GeometryCollectionZ, Unknown, GeometryCollection, UnknownGeometry, true, false ) },
  { GeometryCollectionM, wkbEntry( QStringLiteral( "GeometryCollectionM" ), true, GeometryCollectionM, Unknown, GeometryCollection, UnknownGeometry, false, true ) },
  { GeometryCollectionZM, wkbEntry( QStringLiteral( "GeometryCollectionZM" ), true, GeometryCollectionZM, Unknown, GeometryCollection, UnknownGeometry, true, true ) },
};

QgsWkbTypes::Type QgsWkbTypes::parseType( const QString &wktStr )
{
  QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry>::const_iterator it = ENTRIES.constBegin();
  for ( ; it != ENTRIES.constEnd(); ++it )
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
  QMap< Type, wkbEntry >::const_iterator it = ENTRIES.constFind( type );
  if ( it == ENTRIES.constEnd() )
  {
    return QString();
  }
  return it->mName;
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
