/***************************************************************************
                           qgsgeometryimport.cpp
                         ------------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
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

#include "qgsgeometryimport.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgscurvepolygonv2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"
#include "qgsmulticurvev2.h"
#include "qgsmultilinestringv2.h"
#include "qgsmultipointv2.h"
#include "qgsmultipolygonv2.h"
#include "qgsmultisurfacev2.h"
#include "qgswkbtypes.h"

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkb( const unsigned char* wkb )
{
  if ( !wkb )
  {
    return 0;
  }

  //find out type (bytes 2-5)
  int type;
  memcpy( &type, wkb + 1, sizeof( int ) );
  QgsAbstractGeometryV2* geom = 0;

  geom = geomFromWkbType( QgsWKBTypes::Type( type ) );

#if 0
  type = QgsWKBTypes::flatType( QgsWKBTypes::Type( type ) );
  switch ( type )
  {
    case QgsWKBTypes::Point:
      geom = new QgsPointV2();
      break;
    case QgsWKBTypes::LineString:
      geom = new QgsLineStringV2();
      break;
    case QgsWKBTypes::CircularString:
      geom = new QgsCircularStringV2();
      break;
    case QgsWKBTypes::CompoundCurve:
      geom = new QgsCompoundCurveV2();
      break;
    case QgsWKBTypes::Polygon:
      geom = new QgsPolygonV2();
      break;
    case QgsWKBTypes::CurvePolygon:
      geom = new QgsCurvePolygonV2();
      break;
    case QgsWKBTypes::MultiLineString:
      geom = new QgsMultiLineStringV2();
      break;
    case QgsWKBTypes::MultiPolygon:
      geom = new QgsMultiPolygonV2();
      break;
    case QgsWKBTypes::MultiPoint:
      geom = new QgsMultiPointV2();
      break;
    default:
      geom = 0;
  }
#endif

  if ( geom )
  {
    geom->fromWkb( wkb );
  }
  return geom;
}

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkt( const QString& text )
{
  QgsAbstractGeometryV2* geom = 0;
  if ( text.startsWith( "Point", Qt::CaseInsensitive ) )
  {
    geom = new QgsPointV2();
  }
  else if ( text.startsWith( "LineString", Qt::CaseInsensitive ) )
  {
    geom = new QgsLineStringV2();
  }
  else if ( text .startsWith( "CircularString", Qt::CaseInsensitive ) )
  {
    geom = new QgsCircularStringV2();
  }
  else if ( text.startsWith( "CompoundCurve" , Qt::CaseInsensitive ) )
  {
    geom = new QgsCompoundCurveV2();
  }
  else if ( text.startsWith( "Polygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsPolygonV2();
  }
  else if ( text.startsWith( "CurvePolygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsCurvePolygonV2();
  }
  else if ( text.startsWith( "MultiPoint", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiPointV2();
  }
  else if ( text.startsWith( "MultiCurve", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiCurveV2();
  }
  else if ( text.startsWith( "MultiLineString", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiLineStringV2();
  }
  else if ( text.startsWith( "MultiSurface", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiSurfaceV2();
  }
  else if ( text.startsWith( "MultiPolygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiPolygonV2();
  }
  else if ( text.startsWith( "GeometryCollection", Qt::CaseInsensitive ) )
  {
    geom = new QgsGeometryCollectionV2();
  }

  if ( geom )
  {
    if ( !geom->fromWkt( text ) )
    {
      delete geom; return 0;
    }
  }
  return geom;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPoint( const QgsPoint& point )
{
  return new QgsPointV2( point.x(), point.y() );
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  QgsMultiPointV2* mp = new QgsMultiPointV2();
  QgsMultiPoint::const_iterator ptIt = multipoint.constBegin();
  for ( ; ptIt != multipoint.constEnd(); ++ptIt )
  {
    QgsPointV2* pt = new QgsPointV2( ptIt->x(), ptIt->y() );
    mp->addGeometry( pt );
  }
  return mp;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPolyline( const QgsPolyline& polyline )
{
  return linestringFromPolyline( polyline );
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QgsMultiLineStringV2* mLine = new QgsMultiLineStringV2();
  for ( int i = 0; i < multiline.size(); ++i )
  {
    mLine->addGeometry( fromPolyline( multiline.at( i ) ) );
  }
  return mLine;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromPolygon( const QgsPolygon& polygon )
{
  QgsPolygonV2* poly = new QgsPolygonV2();

  QList<QgsCurveV2*> holes;
  for ( int i = 0; i < polygon.size(); ++i )
  {
    QgsLineStringV2* l = linestringFromPolyline( polygon.at( i ) );
    l->close();

    if ( i == 0 )
    {
      poly->setExteriorRing( l );
    }
    else
    {
      holes.push_back( l );
    }
  }
  poly->setInteriorRings( holes );
  return poly;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  QgsMultiPolygonV2* mp = new QgsMultiPolygonV2();
  for ( int i = 0; i < multipoly.size(); ++i )
  {
    mp->addGeometry( fromPolygon( multipoly.at( i ) ) );
  }
  return mp;
}

QgsAbstractGeometryV2* QgsGeometryImport::fromRect( const QgsRectangle& rect )
{
  QgsPolyline ring;
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );

  QgsPolygon polygon;
  polygon.append( ring );

  return fromPolygon( polygon );
}

QgsLineStringV2* QgsGeometryImport::linestringFromPolyline( const QgsPolyline& polyline )
{
  QgsLineStringV2* line = new QgsLineStringV2();

  QList<QgsPointV2> points;
  QgsPolyline::const_iterator it = polyline.constBegin();
  for ( ; it != polyline.constEnd(); ++it )
  {
    points.append( QgsPointV2( it->x(), it->y() ) );
  }
  line->setPoints( points );
  return line;
}

QgsAbstractGeometryV2* QgsGeometryImport::geomFromWkbType( QgsWKBTypes::Type t )
{
  QgsWKBTypes::Type type = QgsWKBTypes::flatType( t );
  switch ( type )
  {
    case QgsWKBTypes::Point:
      return new QgsPointV2();
    case QgsWKBTypes::LineString:
      return new QgsLineStringV2();
    case QgsWKBTypes::CircularString:
      return new QgsCircularStringV2();
    case QgsWKBTypes::CompoundCurve:
      return new QgsCompoundCurveV2();
    case QgsWKBTypes::Polygon:
      return new QgsPolygonV2();
    case QgsWKBTypes::CurvePolygon:
      return new QgsCurvePolygonV2();
    case QgsWKBTypes::MultiLineString:
      return new QgsMultiLineStringV2();
    case QgsWKBTypes::MultiPolygon:
      return new QgsMultiPolygonV2();
    case QgsWKBTypes::MultiPoint:
      return new QgsMultiPointV2();
    default:
      return 0;
  }
}
