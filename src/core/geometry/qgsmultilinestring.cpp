/***************************************************************************
                        qgsmultilinestring.cpp
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultilinestring.h"
#include "qgsabstractgeometry.h"
#include "qgsapplication.h"
#include "qgscurve.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"

#include <nlohmann/json.hpp>
#include <QJsonObject>

QgsMultiLineString::QgsMultiLineString()
{
  mWkbType = Qgis::WkbType::MultiLineString;
}

QgsMultiLineString::QgsMultiLineString( const QList<QgsLineString> &linestrings )
{
  if ( linestrings.empty() )
    return;

  mGeometries.reserve( linestrings.size() );
  for ( const QgsLineString &line : linestrings )
  {
    mGeometries.append( line.clone() );
  }

  setZMTypeFromSubGeometry( &linestrings.at( 0 ), Qgis::WkbType::MultiLineString );
}

QgsMultiLineString::QgsMultiLineString( const QList<QgsLineString *> &linestrings )
{
  if ( linestrings.empty() )
    return;

  mGeometries.reserve( linestrings.size() );
  for ( QgsLineString *line : linestrings )
  {
    mGeometries.append( line );
  }

  setZMTypeFromSubGeometry( linestrings.at( 0 ), Qgis::WkbType::MultiLineString );
}

QgsLineString *QgsMultiLineString::lineStringN( int index )
{
  return qgsgeometry_cast< QgsLineString * >( geometryN( index ) );
}

const QgsLineString *QgsMultiLineString::lineStringN( int index ) const
{
  return qgsgeometry_cast< const QgsLineString * >( geometryN( index ) );
}

QString QgsMultiLineString::geometryType() const
{
  return QStringLiteral( "MultiLineString" );
}

QgsMultiLineString *QgsMultiLineString::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsMultiLineString >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsMultiLineString *QgsMultiLineString::clone() const
{
  return new QgsMultiLineString( *this );
}

void QgsMultiLineString::clear()
{
  QgsMultiCurve::clear();
  mWkbType = Qgis::WkbType::MultiLineString;
}

bool QgsMultiLineString::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt, QVector<QgsAbstractGeometry *>() << new QgsLineString, QStringLiteral( "LineString" ) );
}

QDomElement QgsMultiLineString::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  QDomElement elemMultiLineString = doc.createElementNS( ns, QStringLiteral( "MultiLineString" ) );

  if ( isEmpty() )
    return elemMultiLineString;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( geom ) )
    {
      QDomElement elemLineStringMember = doc.createElementNS( ns, QStringLiteral( "lineStringMember" ) );
      elemLineStringMember.appendChild( lineString->asGml2( doc, precision, ns, axisOrder ) );
      elemMultiLineString.appendChild( elemLineStringMember );
    }
  }

  return elemMultiLineString;
}

QDomElement QgsMultiLineString::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemMultiCurve = doc.createElementNS( ns, QStringLiteral( "MultiCurve" ) );

  if ( isEmpty() )
    return elemMultiCurve;

  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( const QgsLineString *lineString = qgsgeometry_cast<const QgsLineString *>( geom ) )
    {
      QDomElement elemCurveMember = doc.createElementNS( ns, QStringLiteral( "curveMember" ) );
      elemCurveMember.appendChild( lineString->asGml3( doc, precision, ns, axisOrder ) );
      elemMultiCurve.appendChild( elemCurveMember );
    }
  }

  return elemMultiCurve;
}

json QgsMultiLineString::asJsonObject( int precision ) const
{
  json coordinates( json::array( ) );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( qgsgeometry_cast<const QgsCurve *>( geom ) )
    {
      const QgsLineString *lineString = static_cast<const QgsLineString *>( geom );
      QgsPointSequence pts;
      lineString->points( pts );
      coordinates.push_back( QgsGeometryUtils::pointsToJson( pts, precision ) );
    }
  }
  return
  {
    { "type",  "MultiLineString" },
    { "coordinates", coordinates }
  };
}

bool QgsMultiLineString::addGeometry( QgsAbstractGeometry *g )
{
  if ( !dynamic_cast<QgsLineString *>( g ) )
  {
    delete g;
    return false;
  }

  if ( mGeometries.empty() )
  {
    setZMTypeFromSubGeometry( g, Qgis::WkbType::MultiLineString );
  }
  if ( is3D() && !g->is3D() )
    g->addZValue();
  else if ( !is3D() && g->is3D() )
    g->dropZValue();
  if ( isMeasure() && !g->isMeasure() )
    g->addMValue();
  else if ( !isMeasure() && g->isMeasure() )
    g->dropMValue();
  return QgsGeometryCollection::addGeometry( g ); // NOLINT(bugprone-parent-virtual-call) clazy:exclude=skipped-base-method
}

bool QgsMultiLineString::addGeometries( const QVector<QgsAbstractGeometry *> &geometries )
{
  for ( QgsAbstractGeometry *g : geometries )
  {
    if ( !qgsgeometry_cast<QgsLineString *>( g ) )
    {
      qDeleteAll( geometries );
      return false;
    }
  }

  if ( mGeometries.empty() && !geometries.empty() )
  {
    setZMTypeFromSubGeometry( geometries.at( 0 ), Qgis::WkbType::MultiLineString );
  }
  mGeometries.reserve( mGeometries.size() + geometries.size() );
  for ( QgsAbstractGeometry *g : geometries )
  {
    if ( is3D() && !g->is3D() )
      g->addZValue();
    else if ( !is3D() && g->is3D() )
      g->dropZValue();
    if ( isMeasure() && !g->isMeasure() )
      g->addMValue();
    else if ( !isMeasure() && g->isMeasure() )
      g->dropMValue();
    mGeometries.append( g );
  }

  clearCache();
  return true;
}

bool QgsMultiLineString::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g || QgsWkbTypes::flatType( g->wkbType() ) != Qgis::WkbType::LineString )
  {
    delete g;
    return false;
  }

  return QgsMultiCurve::insertGeometry( g, index );
}

QgsMultiLineString *QgsMultiLineString::simplifyByDistance( double tolerance ) const
{
  std::unique_ptr< QgsMultiLineString > result = std::make_unique< QgsMultiLineString >();
  result->reserve( mGeometries.size() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    result->addGeometry( mGeometries.at( i )->simplifyByDistance( tolerance ) );
  }
  return result.release();
}

QgsMultiCurve *QgsMultiLineString::toCurveType() const
{
  QgsMultiCurve *multiCurve = new QgsMultiCurve();
  multiCurve->reserve( mGeometries.size() );
  for ( int i = 0; i < mGeometries.size(); ++i )
  {
    multiCurve->addGeometry( mGeometries.at( i )->toCurveType() );
  }
  return multiCurve;
}

bool QgsMultiLineString::wktOmitChildType() const
{
  return true;
}

QgsMultiLineString *QgsMultiLineString::measuredLine( double start, double end ) const
{
  std::unique_ptr< QgsMultiLineString > result = std::make_unique< QgsMultiLineString >();
  if ( isEmpty() )
  {
    result->convertTo( QgsWkbTypes::addM( mWkbType ) );
    return result.release();
  }

  /* Calculate the total length of the line */
  const double length{this->length()};
  const double range{end - start};
  double lengthSoFar{0.0};

  result->reserve( numGeometries() );
  for ( int i = 0; i < numGeometries(); i++ )
  {
    const double subLength{geometryN( i )->length()};

    const double subStart{ ( start + range *lengthSoFar / length ) };
    const double subEnd{ ( start + range * ( lengthSoFar + subLength ) / length ) };

    std::unique_ptr< QgsLineString > measuredLine = qgsgeometry_cast<QgsLineString *>( geometryN( i ) )->measuredLine( subStart, subEnd );
    result->addGeometry( measuredLine.release() );

    lengthSoFar += subLength;
  }

  return result.release();
}
