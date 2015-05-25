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

QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry>* QgsWKBTypes::entries()
{
  static QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> entries = registerTypes();
  return &entries;
}

QgsWKBTypes::Type QgsWKBTypes::singleType( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() || it.key() == Unknown )
  {
    return Unknown;
  }
  return ( it->mSingleType );
}

QgsWKBTypes::Type QgsWKBTypes::multiType( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() || it.key() == Unknown )
  {
    return Unknown;
  }
  return it->mMultiType;
}

QgsWKBTypes::Type QgsWKBTypes::flatType( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() || it.key() == Unknown )
  {
    return Unknown;
  }
  return it->mFlatType;
}

QgsWKBTypes::Type QgsWKBTypes::parseType( const QString &wktStr )
{
  QString typestr = wktStr.left( wktStr.indexOf( '(' ) ).simplified().replace( " ", "" );
  foreach ( const Type& type, entries()->keys() )
  {
    QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
    if ( it != entries()->constEnd() && it.value().mName.compare( typestr, Qt::CaseInsensitive ) == 0 )
    {
      return type;
    }
  }
  return Unknown;
}

bool QgsWKBTypes::isSingleType( Type type )
{
  return ( type != Unknown && !isMultiType( type ) );
}

bool QgsWKBTypes::isMultiType( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() )
  {
    return Unknown;
  }
  return it->mIsMultiType;
}

int QgsWKBTypes::wkbDimensions( Type type )
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

QgsWKBTypes::GeometryType QgsWKBTypes::geometryType( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() )
  {
    return UnknownGeometry;
  }
  return it->mGeometryType;
}

QString QgsWKBTypes::displayString( Type type )
{
  QMap< Type, wkbEntry >::const_iterator it = entries()->find( type );
  if ( it == entries()->constEnd() )
  {
    return QString::null;
  }
  return it->mName;
}

QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> QgsWKBTypes::registerTypes()
{
  QMap<QgsWKBTypes::Type, QgsWKBTypes::wkbEntry> entries;
  //register the known wkb types
  entries.insert( Unknown, wkbEntry( "Unknown", false, Unknown, Unknown, Unknown, UnknownGeometry ) );
  entries.insert( NoGeometry, wkbEntry( "NoGeometry", false, NoGeometry, NoGeometry, NoGeometry, NullGeometry ) );
  //point
  entries.insert( Point, wkbEntry( "Point", false, MultiPoint, Point, Point, PointGeometry ) );
  entries.insert( PointZ, wkbEntry( "PointZ", false, MultiPointZ, PointZ, Point, PointGeometry ) );
  entries.insert( PointM, wkbEntry( "PointM", false, MultiPointM, PointM, Point, PointGeometry ) );
  entries.insert( PointZM, wkbEntry( "PointZM", false, MultiPointZM, PointZM, Point, PointGeometry ) );
  entries.insert( Point25D, wkbEntry( "Point25D", false, MultiPoint25D, Point25D, Point, PointGeometry ) );
  //linestring
  entries.insert( LineString, wkbEntry( "LineString", false, MultiLineString, LineString, LineString, LineGeometry ) );
  entries.insert( LineStringZ, wkbEntry( "LineStringZ", false, MultiLineStringZ, LineStringZ, LineString, LineGeometry ) );
  entries.insert( LineStringM, wkbEntry( "LineStringM", false, MultiLineStringM, LineStringM, LineString, LineGeometry ) );
  entries.insert( LineStringZM, wkbEntry( "LineStringZM", false, MultiLineStringZM, LineStringZM, LineString, LineGeometry ) );
  entries.insert( LineString25D, wkbEntry( "LineString25D", false, MultiLineString25D, LineString25D, LineString, LineGeometry ) );
  //circularstring
  entries.insert( CircularString, wkbEntry( "CircularString", false, MultiCurve, CircularString, CircularString, LineGeometry ) );
  entries.insert( CircularStringZ, wkbEntry( "CircularStringZ", false, MultiCurveZ, CircularStringZ, CircularString, LineGeometry ) );
  entries.insert( CircularStringM, wkbEntry( "CircularStringM", false, MultiCurveM, CircularStringM, CircularString, LineGeometry ) );
  entries.insert( CircularStringZM, wkbEntry( "CircularStringZM", false, MultiCurveZM, CircularStringZM, CircularString, LineGeometry ) );
  //compoundcurve
  entries.insert( CompoundCurve, wkbEntry( "CompoundCurve", false, MultiCurve, CompoundCurve, CompoundCurve, LineGeometry ) );
  entries.insert( CompoundCurveZ, wkbEntry( "CompoundCurveZ", false, MultiCurveZ, CompoundCurveZ, CompoundCurve, LineGeometry ) );
  entries.insert( CompoundCurveM, wkbEntry( "CompoundCurveM", false, MultiCurveM, CompoundCurveM, CompoundCurve, LineGeometry ) );
  entries.insert( CompoundCurveZM, wkbEntry( "CompoundCurveZM", false, MultiCurveZM, CompoundCurveZM, CompoundCurve, LineGeometry ) );
  //polygon
  entries.insert( Polygon, wkbEntry( "Polygon", false, MultiPolygon, Polygon, Polygon, PolygonGeometry ) );
  entries.insert( PolygonZ, wkbEntry( "PolygonZ", false, MultiPolygonZ, PolygonZ, Polygon, PolygonGeometry ) );
  entries.insert( PolygonM, wkbEntry( "PolygonM", false, MultiPolygonM, PolygonM, Polygon, PolygonGeometry ) );
  entries.insert( PolygonZM, wkbEntry( "PolygonZM", false, MultiPolygonZM, PolygonZM, Polygon, PolygonGeometry ) );
  entries.insert( Polygon25D, wkbEntry( "Polygon25D", false, MultiPolygon25D, Polygon25D, Polygon, PolygonGeometry ) );
  //curvepolygon
  entries.insert( CurvePolygon, wkbEntry( "CurvePolygon", false, MultiSurface, CurvePolygon, CurvePolygon, PolygonGeometry ) );
  entries.insert( CurvePolygonZ, wkbEntry( "CurvePolygonZ", false, MultiSurfaceZ, CurvePolygonZ, CurvePolygon, PolygonGeometry ) );
  entries.insert( CurvePolygonM, wkbEntry( "CurvePolygonM", false, MultiSurfaceM, CurvePolygonM, CurvePolygon, PolygonGeometry ) );
  entries.insert( CurvePolygonZM, wkbEntry( "CurvePolygonZM", false, MultiSurfaceZM, CurvePolygonZM, CurvePolygon, PolygonGeometry ) );
  //multipoint
  entries.insert( MultiPoint, wkbEntry( "MultiPoint", true, MultiPoint, Point, MultiPoint, PointGeometry ) );
  entries.insert( MultiPointZ, wkbEntry( "MultiPointZ", true, MultiPointZ, PointZ, MultiPoint, PointGeometry ) );
  entries.insert( MultiPointM, wkbEntry( "MultiPointM", true, MultiPointM, PointM, MultiPoint, PointGeometry ) );
  entries.insert( MultiPointZM, wkbEntry( "MultiPointZM", true, MultiPointZM, PointZM, MultiPoint, PointGeometry ) );
  entries.insert( MultiPoint25D, wkbEntry( "MultiPoint25D", true, MultiPoint25D, Point25D, MultiPoint, PointGeometry ) );
  //multiline
  entries.insert( MultiLineString, wkbEntry( "MultiLineString", true, MultiLineString, LineString, MultiLineString, LineGeometry ) );
  entries.insert( MultiLineStringZ, wkbEntry( "MultiLineStringZ", true, MultiLineStringZ, LineStringZ, MultiLineString, LineGeometry ) );
  entries.insert( MultiLineStringM, wkbEntry( "MultiLineStringM", true, MultiLineStringM, LineStringM, MultiLineString, LineGeometry ) );
  entries.insert( MultiLineStringZM, wkbEntry( "MultiLineStringZM", true, MultiLineStringZM, LineStringZM, MultiLineString, LineGeometry ) );
  entries.insert( MultiLineString25D, wkbEntry( "MultiLineString25D", true, MultiLineString25D, LineString25D, MultiLineString, LineGeometry ) );
  //multicurve
  entries.insert( MultiCurve, wkbEntry( "MultiCurve", true, MultiCurve, CompoundCurve, MultiCurve, LineGeometry ) );
  entries.insert( MultiCurveZ, wkbEntry( "MultiCurveZ", true, MultiCurveZ, CompoundCurveZ, MultiCurve, LineGeometry ) );
  entries.insert( MultiCurveM, wkbEntry( "MultiCurveM", true, MultiCurveM, CompoundCurveM, MultiCurve, LineGeometry ) );
  entries.insert( MultiCurveZM, wkbEntry( "MultiCurveZM", true, MultiCurveZM, CompoundCurveZM, MultiCurve, LineGeometry ) );
  //multipolygon
  entries.insert( MultiPolygon, wkbEntry( "MultiPolygon", true, MultiPolygon, Polygon, MultiPolygon, PolygonGeometry ) );
  entries.insert( MultiPolygonZ, wkbEntry( "MultiPolygonZ", true, MultiPolygonZ, PolygonZ, MultiPolygon, PolygonGeometry ) );
  entries.insert( MultiPolygonM, wkbEntry( "MultiPolygonM", true, MultiPolygonM, PolygonM, MultiPolygon, PolygonGeometry ) );
  entries.insert( MultiPolygonZM, wkbEntry( "MultiPolygonZM", true, MultiPolygonZM, PolygonZM, MultiPolygon, PolygonGeometry ) );
  entries.insert( MultiPolygon25D, wkbEntry( "MultiPolygon25D", true, MultiPolygon25D, Polygon25D, MultiPolygon, PolygonGeometry ) );
  //multisurface
  entries.insert( MultiSurface, wkbEntry( "MultiSurface", true, MultiSurface, CurvePolygon, MultiSurface, PolygonGeometry ) );
  entries.insert( MultiSurfaceZ, wkbEntry( "MultiSurfaceZ", true, MultiSurfaceZ, CurvePolygonZ, MultiSurface, PolygonGeometry ) );
  entries.insert( MultiSurfaceM, wkbEntry( "MultiSurfaceM", true, MultiSurfaceM, CurvePolygonM, MultiSurface, PolygonGeometry ) );
  entries.insert( MultiSurfaceZM, wkbEntry( "MultiSurfaceZM", true, MultiSurfaceZM, CurvePolygonZM, MultiSurface, PolygonGeometry ) );
  //geometrycollection
  entries.insert( GeometryCollection, wkbEntry( "GeometryCollection", true, GeometryCollection, Unknown, GeometryCollection, UnknownGeometry ) );
  entries.insert( GeometryCollectionZ, wkbEntry( "GeometryCollectionZ", true, GeometryCollectionZ, Unknown, GeometryCollection, UnknownGeometry ) );
  entries.insert( GeometryCollectionM, wkbEntry( "GeometryCollectionM", true, GeometryCollectionM, Unknown, GeometryCollection, UnknownGeometry ) );
  entries.insert( GeometryCollectionZM, wkbEntry( "GeometryCollectionZM", true, GeometryCollectionZM, Unknown, GeometryCollection, UnknownGeometry ) );
  return entries;
}
