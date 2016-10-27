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

QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry>* QgsWkbTypes::entries()
{
  static QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry> entries = registerTypes();
  return &entries;
}

QgsWkbTypes::Type QgsWkbTypes::parseType( const QString &wktStr )
{
  QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry>* knownTypes = entries();
  QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry>::const_iterator it = knownTypes->constBegin();
  for ( ; it != knownTypes->constEnd(); ++it )
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
  QMap< Type, wkbEntry >::const_iterator it = entries()->constFind( type );
  if ( it == entries()->constEnd() )
  {
    return QString::null;
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

QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry> QgsWkbTypes::registerTypes()
{
  QMap<QgsWkbTypes::Type, QgsWkbTypes::wkbEntry> entries;
  //register the known wkb types
  entries.insert( Unknown, wkbEntry( QStringLiteral( "Unknown" ), false, Unknown, Unknown, Unknown, UnknownGeometry, false, false ) );
  entries.insert( NoGeometry, wkbEntry( QStringLiteral( "NoGeometry" ), false, NoGeometry, NoGeometry, NoGeometry, NullGeometry, false, false ) );
  //point
  entries.insert( Point, wkbEntry( QStringLiteral( "Point" ), false, MultiPoint, Point, Point, PointGeometry, false, false ) );
  entries.insert( PointZ, wkbEntry( QStringLiteral( "PointZ" ), false, MultiPointZ, PointZ, Point, PointGeometry, true, false ) );
  entries.insert( PointM, wkbEntry( QStringLiteral( "PointM" ), false, MultiPointM, PointM, Point, PointGeometry, false, true ) );
  entries.insert( PointZM, wkbEntry( QStringLiteral( "PointZM" ), false, MultiPointZM, PointZM, Point, PointGeometry, true, true ) );
  entries.insert( Point25D, wkbEntry( QStringLiteral( "Point25D" ), false, MultiPoint25D, Point25D, Point, PointGeometry, true, false ) );
  //linestring
  entries.insert( LineString, wkbEntry( QStringLiteral( "LineString" ), false, MultiLineString, LineString, LineString, LineGeometry, false, false ) );
  entries.insert( LineStringZ, wkbEntry( QStringLiteral( "LineStringZ" ), false, MultiLineStringZ, LineStringZ, LineString, LineGeometry, true, false ) );
  entries.insert( LineStringM, wkbEntry( QStringLiteral( "LineStringM" ), false, MultiLineStringM, LineStringM, LineString, LineGeometry, false, true ) );
  entries.insert( LineStringZM, wkbEntry( QStringLiteral( "LineStringZM" ), false, MultiLineStringZM, LineStringZM, LineString, LineGeometry, true, true ) );
  entries.insert( LineString25D, wkbEntry( QStringLiteral( "LineString25D" ), false, MultiLineString25D, LineString25D, LineString, LineGeometry, true, false ) );
  //circularstring
  entries.insert( CircularString, wkbEntry( QStringLiteral( "CircularString" ), false, MultiCurve, CircularString, CircularString, LineGeometry, false, false ) );
  entries.insert( CircularStringZ, wkbEntry( QStringLiteral( "CircularStringZ" ), false, MultiCurveZ, CircularStringZ, CircularString, LineGeometry, true, false ) );
  entries.insert( CircularStringM, wkbEntry( QStringLiteral( "CircularStringM" ), false, MultiCurveM, CircularStringM, CircularString, LineGeometry, false, true ) );
  entries.insert( CircularStringZM, wkbEntry( QStringLiteral( "CircularStringZM" ), false, MultiCurveZM, CircularStringZM, CircularString, LineGeometry, true, true ) );
  //compoundcurve
  entries.insert( CompoundCurve, wkbEntry( QStringLiteral( "CompoundCurve" ), false, MultiCurve, CompoundCurve, CompoundCurve, LineGeometry, false, false ) );
  entries.insert( CompoundCurveZ, wkbEntry( QStringLiteral( "CompoundCurveZ" ), false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineGeometry, true, false ) );
  entries.insert( CompoundCurveM, wkbEntry( QStringLiteral( "CompoundCurveM" ), false, MultiCurveM, CompoundCurveM, CompoundCurve, LineGeometry, false, true ) );
  entries.insert( CompoundCurveZM, wkbEntry( QStringLiteral( "CompoundCurveZM" ), false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineGeometry, true, true ) );
  //polygon
  entries.insert( Polygon, wkbEntry( QStringLiteral( "Polygon" ), false, MultiPolygon, Polygon, Polygon, PolygonGeometry, false, false ) );
  entries.insert( PolygonZ, wkbEntry( QStringLiteral( "PolygonZ" ), false, MultiPolygonZ, PolygonZ, Polygon, PolygonGeometry, true, false ) );
  entries.insert( PolygonM, wkbEntry( QStringLiteral( "PolygonM" ), false, MultiPolygonM, PolygonM, Polygon, PolygonGeometry, false, true ) );
  entries.insert( PolygonZM, wkbEntry( QStringLiteral( "PolygonZM" ), false, MultiPolygonZM, PolygonZM, Polygon, PolygonGeometry, true, true ) );
  entries.insert( Polygon25D, wkbEntry( QStringLiteral( "Polygon25D" ), false, MultiPolygon25D, Polygon25D, Polygon, PolygonGeometry, true, false ) );
  //curvepolygon
  entries.insert( CurvePolygon, wkbEntry( QStringLiteral( "CurvePolygon" ), false, MultiSurface, CurvePolygon, CurvePolygon, PolygonGeometry, false, false ) );
  entries.insert( CurvePolygonZ, wkbEntry( QStringLiteral( "CurvePolygonZ" ), false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonGeometry, true, false ) );
  entries.insert( CurvePolygonM, wkbEntry( QStringLiteral( "CurvePolygonM" ), false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonGeometry, false, true ) );
  entries.insert( CurvePolygonZM, wkbEntry( QStringLiteral( "CurvePolygonZM" ), false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonGeometry, true, true ) );
  //multipoint
  entries.insert( MultiPoint, wkbEntry( QStringLiteral( "MultiPoint" ), true, MultiPoint, Point, MultiPoint, PointGeometry, false, false ) );
  entries.insert( MultiPointZ, wkbEntry( QStringLiteral( "MultiPointZ" ), true, MultiPointZ, PointZ, MultiPoint, PointGeometry, true, false ) );
  entries.insert( MultiPointM, wkbEntry( QStringLiteral( "MultiPointM" ), true, MultiPointM, PointM, MultiPoint, PointGeometry, false, true ) );
  entries.insert( MultiPointZM, wkbEntry( QStringLiteral( "MultiPointZM" ), true, MultiPointZM, PointZM, MultiPoint, PointGeometry, true, true ) );
  entries.insert( MultiPoint25D, wkbEntry( QStringLiteral( "MultiPoint25D" ), true, MultiPoint25D, Point25D, MultiPoint, PointGeometry, true, false ) );
  //multiline
  entries.insert( MultiLineString, wkbEntry( QStringLiteral( "MultiLineString" ), true, MultiLineString, LineString, MultiLineString, LineGeometry, false, false ) );
  entries.insert( MultiLineStringZ, wkbEntry( QStringLiteral( "MultiLineStringZ" ), true, MultiLineStringZ, LineStringZ, MultiLineString, LineGeometry, true, false ) );
  entries.insert( MultiLineStringM, wkbEntry( QStringLiteral( "MultiLineStringM" ), true, MultiLineStringM, LineStringM, MultiLineString, LineGeometry, false, true ) );
  entries.insert( MultiLineStringZM, wkbEntry( QStringLiteral( "MultiLineStringZM" ), true, MultiLineStringZM, LineStringZM, MultiLineString, LineGeometry, true, true ) );
  entries.insert( MultiLineString25D, wkbEntry( QStringLiteral( "MultiLineString25D" ), true, MultiLineString25D, LineString25D, MultiLineString, LineGeometry, true, false ) );
  //multicurve
  entries.insert( MultiCurve, wkbEntry( QStringLiteral( "MultiCurve" ), true, MultiCurve, CompoundCurve, MultiCurve, LineGeometry, false, false ) );
  entries.insert( MultiCurveZ, wkbEntry( QStringLiteral( "MultiCurveZ" ), true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineGeometry, true, false ) );
  entries.insert( MultiCurveM, wkbEntry( QStringLiteral( "MultiCurveM" ), true, MultiCurveM, CompoundCurveM, MultiCurve, LineGeometry, false, true ) );
  entries.insert( MultiCurveZM, wkbEntry( QStringLiteral( "MultiCurveZM" ), true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineGeometry, true, true ) );
  //multipolygon
  entries.insert( MultiPolygon, wkbEntry( QStringLiteral( "MultiPolygon" ), true, MultiPolygon, Polygon, MultiPolygon, PolygonGeometry, false, false ) );
  entries.insert( MultiPolygonZ, wkbEntry( QStringLiteral( "MultiPolygonZ" ), true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonGeometry, true, false ) );
  entries.insert( MultiPolygonM, wkbEntry( QStringLiteral( "MultiPolygonM" ), true, MultiPolygonM, PolygonM, MultiPolygon, PolygonGeometry, false, true ) );
  entries.insert( MultiPolygonZM, wkbEntry( QStringLiteral( "MultiPolygonZM" ), true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonGeometry, true, true ) );
  entries.insert( MultiPolygon25D, wkbEntry( QStringLiteral( "MultiPolygon25D" ), true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonGeometry, true, false ) );
  //multisurface
  entries.insert( MultiSurface, wkbEntry( QStringLiteral( "MultiSurface" ), true, MultiSurface, CurvePolygon, MultiSurface, PolygonGeometry, false, false ) );
  entries.insert( MultiSurfaceZ, wkbEntry( QStringLiteral( "MultiSurfaceZ" ), true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonGeometry, true, false ) );
  entries.insert( MultiSurfaceM, wkbEntry( QStringLiteral( "MultiSurfaceM" ), true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonGeometry, false, true ) );
  entries.insert( MultiSurfaceZM, wkbEntry( QStringLiteral( "MultiSurfaceZM" ), true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonGeometry, true, true ) );
  //geometrycollection
  entries.insert( GeometryCollection, wkbEntry( QStringLiteral( "GeometryCollection" ), true, GeometryCollection, Unknown, GeometryCollection, UnknownGeometry, false, false ) );
  entries.insert( GeometryCollectionZ, wkbEntry( QStringLiteral( "GeometryCollectionZ" ), true, GeometryCollectionZ, Unknown, GeometryCollection, UnknownGeometry, true, false ) );
  entries.insert( GeometryCollectionM, wkbEntry( QStringLiteral( "GeometryCollectionM" ), true, GeometryCollectionM, Unknown, GeometryCollection, UnknownGeometry, false, true ) );
  entries.insert( GeometryCollectionZM, wkbEntry( QStringLiteral( "GeometryCollectionZM" ), true, GeometryCollectionZM, Unknown, GeometryCollection, UnknownGeometry, true, true ) );
  return entries;
}
