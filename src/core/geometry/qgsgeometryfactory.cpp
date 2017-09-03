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
    Q_UNUSED( e );
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
      Q_UNUSED( e );
      QgsDebugMsg( "WKB exception: " + e.what() );
      geom.reset();
    }
  }

  return geom;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkt( const QString &text )
{
  QString trimmed = text.trimmed();
  std::unique_ptr< QgsAbstractGeometry> geom;
  if ( trimmed.startsWith( QLatin1String( "Point" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsPoint() );
  }
  else if ( trimmed.startsWith( QLatin1String( "LineString" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsLineString() );
  }
  else if ( trimmed.startsWith( QLatin1String( "CircularString" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsCircularString() );
  }
  else if ( trimmed.startsWith( QLatin1String( "CompoundCurve" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsCompoundCurve() );
  }
  else if ( trimmed.startsWith( QLatin1String( "Polygon" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsPolygonV2() );
  }
  else if ( trimmed.startsWith( QLatin1String( "CurvePolygon" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsCurvePolygon() );
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiPoint" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsMultiPointV2() );
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiCurve" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsMultiCurve() );
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiLineString" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsMultiLineString() );
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiSurface" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsMultiSurface() );
  }
  else if ( trimmed.startsWith( QLatin1String( "MultiPolygon" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsMultiPolygonV2() );
  }
  else if ( trimmed.startsWith( QLatin1String( "GeometryCollection" ), Qt::CaseInsensitive ) )
  {
    geom.reset( new QgsGeometryCollection() );
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

std::unique_ptr< QgsAbstractGeometry > QgsGeometryFactory::fromPoint( const QgsPointXY &point )
{
  return std::unique_ptr< QgsAbstractGeometry >( new QgsPoint( point.x(), point.y() ) );
}

std::unique_ptr<QgsMultiPointV2> QgsGeometryFactory::fromMultiPoint( const QgsMultiPoint &multipoint )
{
  std::unique_ptr< QgsMultiPointV2 > mp( new QgsMultiPointV2() );
  QgsMultiPoint::const_iterator ptIt = multipoint.constBegin();
  for ( ; ptIt != multipoint.constEnd(); ++ptIt )
  {
    QgsPoint *pt = new QgsPoint( ptIt->x(), ptIt->y() );
    mp->addGeometry( pt );
  }
  return mp;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::fromPolyline( const QgsPolyline &polyline )
{
  return linestringFromPolyline( polyline );
}

std::unique_ptr<QgsMultiLineString> QgsGeometryFactory::fromMultiPolyline( const QgsMultiPolyline &multiline )
{
  std::unique_ptr< QgsMultiLineString > mLine( new QgsMultiLineString() );
  for ( int i = 0; i < multiline.size(); ++i )
  {
    mLine->addGeometry( fromPolyline( multiline.at( i ) ).release() );
  }
  return mLine;
}

std::unique_ptr<QgsPolygonV2> QgsGeometryFactory::fromPolygon( const QgsPolygon &polygon )
{
  std::unique_ptr< QgsPolygonV2 > poly( new QgsPolygonV2() );

  QList<QgsCurve *> holes;
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

std::unique_ptr< QgsMultiPolygonV2 > QgsGeometryFactory::fromMultiPolygon( const QgsMultiPolygon &multipoly )
{
  std::unique_ptr< QgsMultiPolygonV2 > mp( new QgsMultiPolygonV2() );
  for ( int i = 0; i < multipoly.size(); ++i )
  {
    mp->addGeometry( fromPolygon( multipoly.at( i ) ).release() );
  }
  return mp;
}

std::unique_ptr<QgsPolygonV2> QgsGeometryFactory::fromRect( const QgsRectangle &rect )
{
  QgsPolyline ring;
  ring.append( QgsPointXY( rect.xMinimum(), rect.yMinimum() ) );
  ring.append( QgsPointXY( rect.xMaximum(), rect.yMinimum() ) );
  ring.append( QgsPointXY( rect.xMaximum(), rect.yMaximum() ) );
  ring.append( QgsPointXY( rect.xMinimum(), rect.yMaximum() ) );
  ring.append( QgsPointXY( rect.xMinimum(), rect.yMinimum() ) );

  QgsPolygon polygon;
  polygon.append( ring );

  return fromPolygon( polygon );
}

std::unique_ptr<QgsLineString> QgsGeometryFactory::linestringFromPolyline( const QgsPolyline &polyline )
{
  QVector< double > x;
  x.reserve( polyline.size() );
  QVector< double > y;
  y.reserve( polyline.size() );
  QgsPolyline::const_iterator it = polyline.constBegin();
  for ( ; it != polyline.constEnd(); ++it )
  {
    x << it->x();
    y << it->y();
  }
  std::unique_ptr< QgsLineString > line( new QgsLineString( x, y ) );
  return line;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryFactory::geomFromWkbType( QgsWkbTypes::Type t )
{
  QgsWkbTypes::Type type = QgsWkbTypes::flatType( t );
  switch ( type )
  {
    case QgsWkbTypes::Point:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsPoint() );
    case QgsWkbTypes::LineString:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsLineString() );
    case QgsWkbTypes::CircularString:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsCircularString() );
    case QgsWkbTypes::CompoundCurve:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsCompoundCurve() );
    case QgsWkbTypes::Polygon:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsPolygonV2() );
    case QgsWkbTypes::CurvePolygon:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsCurvePolygon() );
    case QgsWkbTypes::MultiLineString:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsMultiLineString() );
    case QgsWkbTypes::MultiPolygon:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsMultiPolygonV2() );
    case QgsWkbTypes::MultiPoint:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsMultiPointV2() );
    case QgsWkbTypes::MultiCurve:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsMultiCurve() );
    case QgsWkbTypes::MultiSurface:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsMultiSurface() );
    case QgsWkbTypes::GeometryCollection:
      return std::unique_ptr<QgsAbstractGeometry>( new QgsGeometryCollection() );
    default:
      return nullptr;
  }
}

std::unique_ptr<QgsGeometryCollection> QgsGeometryFactory::createCollectionOfType( QgsWkbTypes::Type t )
{
  QgsWkbTypes::Type type = QgsWkbTypes::flatType( QgsWkbTypes::multiType( t ) );
  std::unique_ptr< QgsGeometryCollection > collect;
  switch ( type )
  {
    case QgsWkbTypes::MultiPoint:
      collect.reset( new QgsMultiPointV2() );
      break;
    case QgsWkbTypes::MultiLineString:
      collect.reset( new QgsMultiLineString() );
      break;
    case QgsWkbTypes::MultiCurve:
      collect.reset( new QgsMultiCurve() );
      break;
    case QgsWkbTypes::MultiPolygon:
      collect.reset( new QgsMultiPolygonV2() );
      break;
    case QgsWkbTypes::MultiSurface:
      collect.reset( new QgsMultiSurface() );
      break;
    case QgsWkbTypes::GeometryCollection:
      collect.reset( new QgsGeometryCollection() );
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
