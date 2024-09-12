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
#include "qgspolyhedralsurface.h"
#include "qgstriangulatedsurface.h"
#include "qgstriangle.h"
#include "qgswkbtypes.h"
#include "qgslogger.h"

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
    return nullptr;

  //find out type (bytes 2-5)
  Qgis::WkbType type = Qgis::WkbType::Unknown;
  try
  {
    type = wkbPtr.readHeader();
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e )
    QgsDebugError( "WKB exception while reading header: " + e.what() );
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
      QgsDebugError( "WKB exception: " + e.what() );
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
  else if ( trimmed.startsWith( QLatin1String( "PolyhedralSurface" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsPolyhedralSurface >();
  }
  else if ( trimmed.startsWith( QLatin1String( "TIN" ), Qt::CaseInsensitive ) )
  {
    geom = std::make_unique< QgsTriangulatedSurface >();
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

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkbType( Qgis::WkbType t )
{
  const Qgis::WkbType type = QgsWkbTypes::flatType( t );
  switch ( type )
  {
    case Qgis::WkbType::Point:
      return std::make_unique< QgsPoint >();
    case Qgis::WkbType::LineString:
      return std::make_unique< QgsLineString >();
    case Qgis::WkbType::CircularString:
      return std::make_unique< QgsCircularString >();
    case Qgis::WkbType::CompoundCurve:
      return std::make_unique< QgsCompoundCurve >();
    case Qgis::WkbType::Polygon:
      return std::make_unique< QgsPolygon >();
    case Qgis::WkbType::CurvePolygon:
      return std::make_unique< QgsCurvePolygon >();
    case Qgis::WkbType::MultiLineString:
      return std::make_unique< QgsMultiLineString >();
    case Qgis::WkbType::MultiPolygon:
      return std::make_unique< QgsMultiPolygon >();
    case Qgis::WkbType::MultiPoint:
      return std::make_unique< QgsMultiPoint >();
    case Qgis::WkbType::MultiCurve:
      return std::make_unique< QgsMultiCurve >();
    case Qgis::WkbType::MultiSurface:
      return std::make_unique< QgsMultiSurface >();
    case Qgis::WkbType::GeometryCollection:
      return std::make_unique< QgsGeometryCollection >();
    case Qgis::WkbType::Triangle:
      return std::make_unique< QgsTriangle >();
    case Qgis::WkbType::PolyhedralSurface:
      return std::make_unique< QgsPolyhedralSurface >();
    case Qgis::WkbType::TIN:
      return std::make_unique< QgsTriangulatedSurface >();
    default:
      return nullptr;
  }
}

std::unique_ptr<QgsGeometryCollection> QgsGeometryFactory::createCollectionOfType( Qgis::WkbType t )
{
  const Qgis::WkbType type = QgsWkbTypes::flatType( QgsWkbTypes::multiType( t ) );
  std::unique_ptr< QgsGeometryCollection > collect;
  switch ( type )
  {
    case Qgis::WkbType::MultiPoint:
      collect = std::make_unique< QgsMultiPoint >();
      break;
    case Qgis::WkbType::MultiLineString:
      collect = std::make_unique< QgsMultiLineString >();
      break;
    case Qgis::WkbType::MultiCurve:
      collect = std::make_unique< QgsMultiCurve >();
      break;
    case Qgis::WkbType::MultiPolygon:
      collect = std::make_unique< QgsMultiPolygon >();
      break;
    case Qgis::WkbType::MultiSurface:
      collect = std::make_unique< QgsMultiSurface >();
      break;
    case Qgis::WkbType::GeometryCollection:
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
