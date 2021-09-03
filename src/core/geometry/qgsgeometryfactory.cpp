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
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgstriangle.h"
#include "qgswkbtypes.h"
#include "qgslogger.h"

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkb( QgsConstWkbPtr &wkbPtr )
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
    Q_UNUSED( e )
    QgsDebugMsg( "WKB exception while reading header: " + e.what() );
    return nullptr;
  }
  wkbPtr -= 1 + sizeof( int );

  std::unique_ptr< QgsAbstractGeometry > geom = geomFromWkbType( type );

  if ( geom )
  {
    try
    {
      geom->fromWkb( wkbPtr );  // also updates wkbPtr
    }
    catch ( const QgsWkbException &e )
    {
      Q_UNUSED( e )
      QgsDebugMsg( "WKB exception: " + e.what() );
      geom.reset();
    }
  }

  return geom;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkt( const QString &text )
{
  const QString trimmed = text.trimmed();
  std::unique_ptr< QgsAbstractGeometry> geom;
  if ( trimmed.startsWith( QLatin1String( "Point" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsPoint >();
  }
  else if ( trimmed.startsWith( QLatin1String( "LineString" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsLineString >();
  }
  else if ( trimmed.startsWith( QLatin1String( "CircularString" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsCircularString >();
  }
  else if ( trimmed.startsWith( QLatin1String( "CompoundCurve" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsCompoundCurve>();
  }
  else if ( trimmed.startsWith( QLatin1String( "Polygon" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsPolygon >();
  }
  else if ( trimmed.startsWith( QLatin1String( "Triangle" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsTriangle >();
  }
  else if ( trimmed.startsWith( QLatin1String( "CurvePolygon" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsCurvePolygon >();
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiPoint" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsMultiPoint >();
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiCurve" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsMultiCurve >();
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiLineString" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsMultiLineString >();
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiSurface" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsMultiSurface >();
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiPolygon" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsMultiPolygon >();
  }
  else if ( trimmed.startsWith( QLatin1String( "GeometryCollection" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsGeometryCollection >();
  }

  if ( geom )
  {
    if ( !geom->fromWkt( text ) )
    {
      return nullptr;
    }
  }
  return geom;
}

std::unique_ptr< QgsAbstractGeometry > QgsGeometryFactory::fromPointXY( const QgsPointXY &point )
{
  return std::make_unique< QgsPoint >( point.x(), point.y() );
}

std::unique_ptr<QgsMultiPoint> QgsGeometryFactory::fromMultiPointXY( const QgsMultiPointXY &multipoint )
{
  std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >();
  QgsMultiPointXY::const_iterator ptIt = multipoint.constBegin();
  mp->reserve( multipoint.size() );
  for ( ; ptIt != multipoint.constEnd(); ++ptIt )
  {
    QgsPoint *pt = new QgsPoint( ptIt->x(), ptIt->y() );
    mp->addGeometry( pt );
  }
  return mp;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::fromPolylineXY( const QgsPolylineXY &polyline )
{
  return linestringFromPolyline( polyline );
}

std::unique_ptr<QgsMultiLineString> QgsGeometryFactory::fromMultiPolylineXY( const QgsMultiPolylineXY &multiline )
{
  std::unique_ptr< QgsMultiLineString > mLine = std::make_unique< QgsMultiLineString >();
  mLine->reserve( multiline.size() );
  for ( int i = 0; i < multiline.size(); ++i )
  {
    mLine->addGeometry( fromPolylineXY( multiline.at( i ) ).release() );
  }
  return mLine;
}

std::unique_ptr<QgsPolygon> QgsGeometryFactory::fromPolygonXY( const QgsPolygonXY &polygon )
{
  std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();

  QVector<QgsCurve *> holes;
  holes.reserve( polygon.size() );
  for ( int i = 0; i < polygon.size(); ++i )
  {
    std::unique_ptr< QgsLineString > l = linestringFromPolyline( polygon.at( i ) );
    l->close();

    if ( i == 0 )
    {
      poly->setExteriorRing( l.release() );
    }
    else
    {
      holes.push_back( l.release() );
    }
  }
  poly->setInteriorRings( holes );
  return poly;
}

std::unique_ptr< QgsMultiPolygon > QgsGeometryFactory::fromMultiPolygonXY( const QgsMultiPolygonXY &multipoly )
{
  std::unique_ptr< QgsMultiPolygon > mp = std::make_unique< QgsMultiPolygon >();
  mp->reserve( multipoly.size() );
  for ( int i = 0; i < multipoly.size(); ++i )
  {
    mp->addGeometry( fromPolygonXY( multipoly.at( i ) ).release() );
  }
  return mp;
}

std::unique_ptr<QgsLineString> QgsGeometryFactory::linestringFromPolyline( const QgsPolylineXY &polyline )
{
  const int size = polyline.size();
  QVector< double > x;
  x.resize( size );
  QVector< double > y;
  y.resize( size );
  double *destX = x.data();
  double *destY = y.data();
  const QgsPointXY *src = polyline.data();
  for ( int i = 0; i < size; ++i )
  {
    *destX++ = src->x();
    *destY++ = src->y();
    src++;
  }
  std::unique_ptr< QgsLineString > line = std::make_unique< QgsLineString >( x, y );
  return line;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkbType( QgsWkbTypes::Type t )
{
  const QgsWkbTypes::Type type = QgsWkbTypes::flatType( t );
  switch ( type )
  {
    case QgsWkbTypes::Point:
      return std::make_unique< QgsPoint >();
    case QgsWkbTypes::LineString:
      return std::make_unique< QgsLineString >();
    case QgsWkbTypes::CircularString:
      return std::make_unique< QgsCircularString >();
    case QgsWkbTypes::CompoundCurve:
      return std::make_unique< QgsCompoundCurve >();
    case QgsWkbTypes::Polygon:
      return std::make_unique< QgsPolygon >();
    case QgsWkbTypes::CurvePolygon:
      return std::make_unique< QgsCurvePolygon >();
    case QgsWkbTypes::MultiLineString:
      return std::make_unique< QgsMultiLineString >();
    case QgsWkbTypes::MultiPolygon:
      return std::make_unique< QgsMultiPolygon >();
    case QgsWkbTypes::MultiPoint:
      return std::make_unique< QgsMultiPoint >();
    case QgsWkbTypes::MultiCurve:
      return std::make_unique< QgsMultiCurve >();
    case QgsWkbTypes::MultiSurface:
      return std::make_unique< QgsMultiSurface >();
    case QgsWkbTypes::GeometryCollection:
      return std::make_unique< QgsGeometryCollection >();
    case QgsWkbTypes::Triangle:
      return std::make_unique< QgsTriangle >();
    default:
      return nullptr;
  }
}

std::unique_ptr<QgsGeometryCollection> QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Type t )
{
  const QgsWkbTypes::Type type = QgsWkbTypes::flatType( QgsWkbTypes::multiType( t ) );
  std::unique_ptr< QgsGeometryCollection > collect;
  switch ( type )
  {
    case QgsWkbTypes::MultiPoint:
      collect = std::make_unique< QgsMultiPoint >();
      break;
    case QgsWkbTypes::MultiLineString:
      collect = std::make_unique< QgsMultiLineString >();
      break;
    case QgsWkbTypes::MultiCurve:
      collect = std::make_unique< QgsMultiCurve >();
      break;
    case QgsWkbTypes::MultiPolygon:
      collect = std::make_unique< QgsMultiPolygon >();
      break;
    case QgsWkbTypes::MultiSurface:
      collect = std::make_unique< QgsMultiSurface >();
      break;
    case QgsWkbTypes::GeometryCollection:
      collect = std::make_unique< QgsGeometryCollection >();
      break;
    default:
      // should not be possible
      return nullptr;
  }
  if ( QgsWkbTypes::hasM( t ) )
    collect->addMValue();
  if ( QgsWkbTypes::hasZ( t ) )
    collect->addZValue();

  return collect;
}
