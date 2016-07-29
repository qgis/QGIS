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

QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry>* QgsWKBTypes::entries()
{
  static QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> entries = registerTypes();
  return &entries;
}

QgsWKBTypes::Type QgsWKBTypes::parseType( const QString &wktStr )
{
  QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().remove( ' ' );

  QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry>* knownTypes = entries();
  QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry>::const_iterator it = knownTypes->constBegin();
  for ( ; it != knownTypes->constEnd(); ++it )
  {
    if ( it.value().mName.compare( typestr, Qt::CaseInsensitive ) == 0 )
    {
      return it.key();
    }
  }
  return Unknown;
}

QString QgsWKBTypes::displayString( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->constFind( type );
  if ( it == entries()->constEnd() )
  {
    return QString::null;
  }
  return it->mName;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> QgsWKBTypes::registerTypes()
{
  QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> entries;
  //register the known wkb types
  entries.insert( Unknown, wkbEntry( "Unknown", false, Unknown, Unknown, Unknown, UnknownGeometry, false, false ) );
  entries.insert( NoGeometry, wkbEntry( "NoGeometry", false, NoGeometry, NoGeometry, NoGeometry, NullGeometry, false, false ) );
  //point
  entries.insert( Point, wkbEntry( "Point", false, MultiPoint, Point, Point, PointGeometry, false, false ) );
  entries.insert( PointZ, wkbEntry( "PointZ", false, MultiPointZ, PointZ, Point, PointGeometry, true, false ) );
  entries.insert( PointM, wkbEntry( "PointM", false, MultiPointM, PointM, Point, PointGeometry, false, true ) );
  entries.insert( PointZM, wkbEntry( "PointZM", false, MultiPointZM, PointZM, Point, PointGeometry, true, true ) );
  entries.insert( Point25D, wkbEntry( "Point25D", false, MultiPoint25D, Point25D, Point, PointGeometry, true, false ) );
  //linestring
  entries.insert( LineString, wkbEntry( "LineString", false, MultiLineString, LineString, LineString, LineGeometry, false, false ) );
  entries.insert( LineStringZ, wkbEntry( "LineStringZ", false, MultiLineStringZ, LineStringZ, LineString, LineGeometry, true, false ) );
  entries.insert( LineStringM, wkbEntry( "LineStringM", false, MultiLineStringM, LineStringM, LineString, LineGeometry, false, true ) );
  entries.insert( LineStringZM, wkbEntry( "LineStringZM", false, MultiLineStringZM, LineStringZM, LineString, LineGeometry, true, true ) );
  entries.insert( LineString25D, wkbEntry( "LineString25D", false, MultiLineString25D, LineString25D, LineString, LineGeometry, true, false ) );
  //circularstring
  entries.insert( CircularString, wkbEntry( "CircularString", false, MultiCurve, CircularString, CircularString, LineGeometry, false, false ) );
  entries.insert( CircularStringZ, wkbEntry( "CircularStringZ", false, MultiCurveZ, CircularStringZ, CircularString, LineGeometry, true, false ) );
  entries.insert( CircularStringM, wkbEntry( "CircularStringM", false, MultiCurveM, CircularStringM, CircularString, LineGeometry, false, true ) );
  entries.insert( CircularStringZM, wkbEntry( "CircularStringZM", false, MultiCurveZM, CircularStringZM, CircularString, LineGeometry, true, true ) );
  //compoundcurve
  entries.insert( CompoundCurve, wkbEntry( "CompoundCurve", false, MultiCurve, CompoundCurve, CompoundCurve, LineGeometry, false, false ) );
  entries.insert( CompoundCurveZ, wkbEntry( "CompoundCurveZ", false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineGeometry, true, false ) );
  entries.insert( CompoundCurveM, wkbEntry( "CompoundCurveM", false, MultiCurveM, CompoundCurveM, CompoundCurve, LineGeometry, false, true ) );
  entries.insert( CompoundCurveZM, wkbEntry( "CompoundCurveZM", false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineGeometry, true, true ) );
  //polygon
  entries.insert( Polygon, wkbEntry( "Polygon", false, MultiPolygon, Polygon, Polygon, PolygonGeometry, false, false ) );
  entries.insert( PolygonZ, wkbEntry( "PolygonZ", false, MultiPolygonZ, PolygonZ, Polygon, PolygonGeometry, true, false ) );
  entries.insert( PolygonM, wkbEntry( "PolygonM", false, MultiPolygonM, PolygonM, Polygon, PolygonGeometry, false, true ) );
  entries.insert( PolygonZM, wkbEntry( "PolygonZM", false, MultiPolygonZM, PolygonZM, Polygon, PolygonGeometry, true, true ) );
  entries.insert( Polygon25D, wkbEntry( "Polygon25D", false, MultiPolygon25D, Polygon25D, Polygon, PolygonGeometry, true, false ) );
  //curvepolygon
  entries.insert( CurvePolygon, wkbEntry( "CurvePolygon", false, MultiSurface, CurvePolygon, CurvePolygon, PolygonGeometry, false, false ) );
  entries.insert( CurvePolygonZ, wkbEntry( "CurvePolygonZ", false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonGeometry, true, false ) );
  entries.insert( CurvePolygonM, wkbEntry( "CurvePolygonM", false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonGeometry, false, true ) );
  entries.insert( CurvePolygonZM, wkbEntry( "CurvePolygonZM", false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonGeometry, true, true ) );
  //multipoint
  entries.insert( MultiPoint, wkbEntry( "MultiPoint", true, MultiPoint, Point, MultiPoint, PointGeometry, false, false ) );
  entries.insert( MultiPointZ, wkbEntry( "MultiPointZ", true, MultiPointZ, PointZ, MultiPoint, PointGeometry, true, false ) );
  entries.insert( MultiPointM, wkbEntry( "MultiPointM", true, MultiPointM, PointM, MultiPoint, PointGeometry, false, true ) );
  entries.insert( MultiPointZM, wkbEntry( "MultiPointZM", true, MultiPointZM, PointZM, MultiPoint, PointGeometry, true, true ) );
  entries.insert( MultiPoint25D, wkbEntry( "MultiPoint25D", true, MultiPoint25D, Point25D, MultiPoint, PointGeometry, true, false ) );
  //multiline
  entries.insert( MultiLineString, wkbEntry( "MultiLineString", true, MultiLineString, LineString, MultiLineString, LineGeometry, false, false ) );
  entries.insert( MultiLineStringZ, wkbEntry( "MultiLineStringZ", true, MultiLineStringZ, LineStringZ, MultiLineString, LineGeometry, true, false ) );
  entries.insert( MultiLineStringM, wkbEntry( "MultiLineStringM", true, MultiLineStringM, LineStringM, MultiLineString, LineGeometry, false, true ) );
  entries.insert( MultiLineStringZM, wkbEntry( "MultiLineStringZM", true, MultiLineStringZM, LineStringZM, MultiLineString, LineGeometry, true, true ) );
  entries.insert( MultiLineString25D, wkbEntry( "MultiLineString25D", true, MultiLineString25D, LineString25D, MultiLineString, LineGeometry, true, false ) );
  //multicurve
  entries.insert( MultiCurve, wkbEntry( "MultiCurve", true, MultiCurve, CompoundCurve, MultiCurve, LineGeometry, false, false ) );
  entries.insert( MultiCurveZ, wkbEntry( "MultiCurveZ", true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineGeometry, true, false ) );
  entries.insert( MultiCurveM, wkbEntry( "MultiCurveM", true, MultiCurveM, CompoundCurveM, MultiCurve, LineGeometry, false, true ) );
  entries.insert( MultiCurveZM, wkbEntry( "MultiCurveZM", true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineGeometry, true, true ) );
  //multipolygon
  entries.insert( MultiPolygon, wkbEntry( "MultiPolygon", true, MultiPolygon, Polygon, MultiPolygon, PolygonGeometry, false, false ) );
  entries.insert( MultiPolygonZ, wkbEntry( "MultiPolygonZ", true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonGeometry, true, false ) );
  entries.insert( MultiPolygonM, wkbEntry( "MultiPolygonM", true, MultiPolygonM, PolygonM, MultiPolygon, PolygonGeometry, false, true ) );
  entries.insert( MultiPolygonZM, wkbEntry( "MultiPolygonZM", true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonGeometry, true, true ) );
  entries.insert( MultiPolygon25D, wkbEntry( "MultiPolygon25D", true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonGeometry, true, false ) );
  //multisurface
  entries.insert( MultiSurface, wkbEntry( "MultiSurface", true, MultiSurface, CurvePolygon, MultiSurface, PolygonGeometry, false, false ) );
  entries.insert( MultiSurfaceZ, wkbEntry( "MultiSurfaceZ", true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonGeometry, true, false ) );
  entries.insert( MultiSurfaceM, wkbEntry( "MultiSurfaceM", true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonGeometry, false, true ) );
  entries.insert( MultiSurfaceZM, wkbEntry( "MultiSurfaceZM", true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonGeometry, true, true ) );
  //geometrycollection
  entries.insert( GeometryCollection, wkbEntry( "GeometryCollection", true, GeometryCollection, Unknown, GeometryCollection, UnknownGeometry, false, false ) );
  entries.insert( GeometryCollectionZ, wkbEntry( "GeometryCollectionZ", true, GeometryCollectionZ, Unknown, GeometryCollection, UnknownGeometry, true, false ) );
  entries.insert( GeometryCollectionM, wkbEntry( "GeometryCollectionM", true, GeometryCollectionM, Unknown, GeometryCollection, UnknownGeometry, false, true ) );
  entries.insert( GeometryCollectionZM, wkbEntry( "GeometryCollectionZM", true, GeometryCollectionZM, Unknown, GeometryCollection, UnknownGeometry, true, true ) );
  return entries;
}
