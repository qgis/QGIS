/***************************************************************************
                           qgsgeometryfactory.cpp
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

#include "qgsgeometryfactory.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgspointv2.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgswkbtypes.h"
#include "qgslogger.h"

QgsAbstractGeometry* QgsGeometryFactory::geomFromWkb( QgsConstWkbPtr wkbPtr )
{
  if ( !wkbPtr )
    return nullptr;

  //find out type (bytes 2-5)
  QgsWkbTypes::Type type = QgsWkbTypes::Unknown;
  try
  {
    type = wkbPtr.readHeader();
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "WKB exception while reading header: " + e.what() );
    return nullptr;
  }
  wkbPtr -= 1 + sizeof( int );

  QgsAbstractGeometry* geom = nullptr;

  geom = geomFromWkbType( type );

  if ( geom )
  {
    try
    {
      geom->fromWkb( wkbPtr );
    }
    catch ( const QgsWkbException &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( "WKB exception: " + e.what() );
      delete geom;
      geom = nullptr;
    }
  }

  return geom;
}

QgsAbstractGeometry* QgsGeometryFactory::geomFromWkt( const QString& text )
{
  QgsAbstractGeometry* geom = nullptr;
  if ( text.startsWith( "Point", Qt::CaseInsensitive ) )
  {
    geom = new QgsPointV2();
  }
  else if ( text.startsWith( "LineString", Qt::CaseInsensitive ) )
  {
    geom = new QgsLineString();
  }
  else if ( text.startsWith( "CircularString", Qt::CaseInsensitive ) )
  {
    geom = new QgsCircularString();
  }
  else if ( text.startsWith( "CompoundCurve" , Qt::CaseInsensitive ) )
  {
    geom = new QgsCompoundCurve();
  }
  else if ( text.startsWith( "Polygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsPolygonV2();
  }
  else if ( text.startsWith( "CurvePolygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsCurvePolygon();
  }
  else if ( text.startsWith( "MultiPoint", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiPointV2();
  }
  else if ( text.startsWith( "MultiCurve", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiCurve();
  }
  else if ( text.startsWith( "MultiLineString", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiLineString();
  }
  else if ( text.startsWith( "MultiSurface", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiSurface();
  }
  else if ( text.startsWith( "MultiPolygon", Qt::CaseInsensitive ) )
  {
    geom = new QgsMultiPolygonV2();
  }
  else if ( text.startsWith( "GeometryCollection", Qt::CaseInsensitive ) )
  {
    geom = new QgsGeometryCollection();
  }

  if ( geom )
  {
    if ( !geom->fromWkt( text ) )
    {
      delete geom;
      return nullptr;
    }
  }
  return geom;
}

QgsAbstractGeometry* QgsGeometryFactory::fromPoint( const QgsPoint& point )
{
  return new QgsPointV2( point.x(), point.y() );
}

QgsAbstractGeometry* QgsGeometryFactory::fromMultiPoint( const QgsMultiPoint& multipoint )
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

QgsAbstractGeometry* QgsGeometryFactory::fromPolyline( const QgsPolyline& polyline )
{
  return linestringFromPolyline( polyline );
}

QgsAbstractGeometry* QgsGeometryFactory::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QgsMultiLineString* mLine = new QgsMultiLineString();
  for ( int i = 0; i < multiline.size(); ++i )
  {
    mLine->addGeometry( fromPolyline( multiline.at( i ) ) );
  }
  return mLine;
}

QgsAbstractGeometry* QgsGeometryFactory::fromPolygon( const QgsPolygon& polygon )
{
  QgsPolygonV2* poly = new QgsPolygonV2();

  QList<QgsCurve*> holes;
  for ( int i = 0; i < polygon.size(); ++i )
  {
    QgsLineString* l = linestringFromPolyline( polygon.at( i ) );
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

QgsAbstractGeometry* QgsGeometryFactory::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  QgsMultiPolygonV2* mp = new QgsMultiPolygonV2();
  for ( int i = 0; i < multipoly.size(); ++i )
  {
    mp->addGeometry( fromPolygon( multipoly.at( i ) ) );
  }
  return mp;
}

QgsAbstractGeometry* QgsGeometryFactory::fromRect( const QgsRectangle& rect )
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

QgsLineString* QgsGeometryFactory::linestringFromPolyline( const QgsPolyline& polyline )
{
  QgsLineString* line = new QgsLineString();

  QgsPointSequence points;
  QgsPolyline::const_iterator it = polyline.constBegin();
  for ( ; it != polyline.constEnd(); ++it )
  {
    points.append( QgsPointV2( it->x(), it->y() ) );
  }
  line->setPoints( points );
  return line;
}

QgsAbstractGeometry* QgsGeometryFactory::geomFromWkbType( QgsWkbTypes::Type t )
{
  QgsWkbTypes::Type type = QgsWkbTypes::flatType( t );
  switch ( type )
  {
    case QgsWkbTypes::Point:
      return new QgsPointV2();
    case QgsWkbTypes::LineString:
      return new QgsLineString();
    case QgsWkbTypes::CircularString:
      return new QgsCircularString();
    case QgsWkbTypes::CompoundCurve:
      return new QgsCompoundCurve();
    case QgsWkbTypes::Polygon:
      return new QgsPolygonV2();
    case QgsWkbTypes::CurvePolygon:
      return new QgsCurvePolygon();
    case QgsWkbTypes::MultiLineString:
      return new QgsMultiLineString();
    case QgsWkbTypes::MultiPolygon:
      return new QgsMultiPolygonV2();
    case QgsWkbTypes::MultiPoint:
      return new QgsMultiPointV2();
    case QgsWkbTypes::MultiCurve:
      return new QgsMultiCurve();
    case QgsWkbTypes::MultiSurface:
      return new QgsMultiSurface();
    case QgsWkbTypes::GeometryCollection:
      return new QgsGeometryCollection();
    default:
      return nullptr;
  }
}
