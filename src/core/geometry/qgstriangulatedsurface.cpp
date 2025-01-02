/***************************************************************************
                         qgstriangulatedsurface.cpp
                         ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstriangulatedsurface.h"
#include "qgspolyhedralsurface.h"
#include "qgslogger.h"
#include "qgstriangle.h"
#include "qgsvertexid.h"
#include "qgsgeometryutils.h"

#include <memory>
#include <nlohmann/json.hpp>

QgsTriangulatedSurface::QgsTriangulatedSurface()
{
  mWkbType = Qgis::WkbType::TIN;
}

QgsTriangulatedSurface::~QgsTriangulatedSurface()
{
  QgsTriangulatedSurface::clear();
}

QgsTriangulatedSurface *QgsTriangulatedSurface::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsTriangulatedSurface >();
  result->mWkbType = mWkbType;
  return result.release();
}

QString QgsTriangulatedSurface::geometryType() const
{
  return QStringLiteral( "TIN" );
}

QgsTriangulatedSurface::QgsTriangulatedSurface( const QgsTriangulatedSurface &p )
  : QgsPolyhedralSurface( p )

{
  mWkbType = p.mWkbType;
}

// cppcheck-suppress operatorEqVarError
QgsTriangulatedSurface &QgsTriangulatedSurface::operator=( const QgsTriangulatedSurface &p )
{
  if ( &p != this )
  {
    QgsPolyhedralSurface::operator=( p );
  }
  return *this;
}

QgsTriangulatedSurface *QgsTriangulatedSurface::clone() const
{
  return new QgsTriangulatedSurface( *this );
}

void QgsTriangulatedSurface::clear()
{
  mWkbType = Qgis::WkbType::TIN;
  qDeleteAll( mPatches );
  mPatches.clear();
  clearCache();
}


bool QgsTriangulatedSurface::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();

  if ( !wkbPtr )
  {
    return false;
  }

  Qgis::WkbType type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != Qgis::WkbType::TIN )
  {
    return false;
  }
  mWkbType = type;

  int nTriangles;
  wkbPtr >> nTriangles;
  std::unique_ptr< QgsTriangle > currentTriangle;
  for ( int i = 0; i < nTriangles; ++i )
  {
    Qgis::WkbType triangleType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    Qgis::WkbType flatTriangleType = QgsWkbTypes::flatType( triangleType );
    if ( flatTriangleType == Qgis::WkbType::Triangle )
    {
      currentTriangle.reset( new QgsTriangle() );
    }
    else
    {
      return false;
    }
    currentTriangle->fromWkb( wkbPtr );  // also updates wkbPtr
    mPatches.append( currentTriangle.release() );
  }

  return true;
}

bool QgsTriangulatedSurface::fromWkt( const QString &wkt )
{
  clear();

  QPair<Qgis::WkbType, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != Qgis::GeometryType::Polygon )
    return false;

  mWkbType = parts.first;

  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.remove( '(' ).remove( ')' ).simplified().remove( ' ' );
  if ( ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 ) ||
       secondWithoutParentheses.isEmpty() )
    return true;

  QString defaultChildWkbType = QStringLiteral( "Triangle%1%2" ).arg( is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType );
  for ( const QString &childWkt : blocks )
  {
    QPair<Qgis::WkbType, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    if ( QgsWkbTypes::flatType( childParts.first ) == Qgis::WkbType::Triangle )
    {
      mPatches.append( new QgsTriangle() );
    }
    else
    {
      clear();
      return false;
    }

    if ( !mPatches.back()->fromWkt( childWkt ) )
    {
      clear();
      return false;
    }
  }

  return true;
}

QDomElement QgsTriangulatedSurface::asGml2( QDomDocument &, int, const QString &, const AxisOrder ) const
{
  QgsDebugError( QStringLiteral( "gml version 2 does not support TIN geometry" ) );
  return QDomElement();
}

QDomElement QgsTriangulatedSurface::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemTriangulatedSurface = doc.createElementNS( ns, QStringLiteral( "TriangulatedSurface" ) );

  if ( isEmpty() )
    return elemTriangulatedSurface;

  QDomElement elemPatches = doc.createElementNS( ns, QStringLiteral( "patches" ) );
  for ( QgsPolygon *patch : mPatches )
  {
    QgsTriangle *triangle = qgsgeometry_cast<QgsTriangle *>( patch );
    if ( !triangle )
      continue;

    QDomElement elemTriangle = triangle->asGml3( doc, precision, ns, axisOrder );
    elemPatches.appendChild( elemTriangle );
  }
  elemTriangulatedSurface.appendChild( elemPatches );

  return elemTriangulatedSurface;
}

QString QgsTriangulatedSurface::asKml( int ) const
{
  QgsDebugError( QStringLiteral( "kml format does not support TIN geometry" ) );
  return QString( "" );
}

void QgsTriangulatedSurface::normalize()
{
  for ( QgsPolygon *patch : mPatches )
  {
    QgsTriangle *triangle = qgsgeometry_cast<QgsTriangle *>( patch );
    if ( !triangle )
      continue;

    QgsCurve *exteriorRing = triangle->exteriorRing();
    if ( !exteriorRing )
      continue;

    exteriorRing->normalize();
  }
}

QgsTriangulatedSurface *QgsTriangulatedSurface::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing, bool removeRedundantPoints ) const
{
  std::unique_ptr< QgsTriangulatedSurface > surface( createEmptyWithSameType() );

  for ( QgsPolygon *patch : mPatches )
  {
    QgsTriangle *triangle = qgsgeometry_cast<QgsTriangle *>( patch );
    if ( !triangle )
      continue;

    std::unique_ptr<QgsCurve> exteriorRing( qgsgeometry_cast< QgsCurve *>( triangle->exteriorRing()->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing, removeRedundantPoints ) ) );
    if ( !exteriorRing )
    {
      return nullptr;
    }

    std::unique_ptr<QgsTriangle> gridifiedTriangle = std::make_unique<QgsTriangle>();
    gridifiedTriangle->setExteriorRing( exteriorRing.release() );
    surface->addPatch( gridifiedTriangle.release() );
  }

  return surface.release();
}

void QgsTriangulatedSurface::setTriangles( const QVector<QgsTriangle *> &triangles )
{
  qDeleteAll( mPatches );
  mPatches.clear();

  for ( QgsTriangle *triangle : triangles )
  {
    addPatch( triangle );
  }

  clearCache();
}

void QgsTriangulatedSurface::addPatch( QgsPolygon *patch )
{
  QgsTriangle *triangle = qgsgeometry_cast<QgsTriangle *>( patch );
  if ( !triangle )
  {
    if ( patch )
    {
      delete patch;
    }
    return;
  }

  if ( mPatches.empty() )
  {
    setZMTypeFromSubGeometry( patch, Qgis::WkbType::TIN );
  }

  // Ensure dimensionality of patch matches TIN
  if ( !is3D() )
    triangle->dropZValue();
  else if ( !triangle->is3D() )
    triangle->addZValue();

  if ( !isMeasure() )
    triangle->dropMValue();
  else if ( !triangle->isMeasure() )
    triangle->addMValue();

  mPatches.append( triangle );
  clearCache();
}

void QgsTriangulatedSurface::addTriangle( QgsTriangle *triangle )
{
  addPatch( triangle );
}

QgsTriangle *QgsTriangulatedSurface::triangleN( int index )
{
  return qgsgeometry_cast< QgsTriangle * >( patchN( index ) );
}

const QgsTriangle *QgsTriangulatedSurface::triangleN( int index ) const
{
  return qgsgeometry_cast< const QgsTriangle * >( patchN( index ) );
}

bool QgsTriangulatedSurface::insertVertex( QgsVertexId vId, const QgsPoint &vertex )
{
  Q_UNUSED( vId )
  Q_UNUSED( vertex )
  return false;
}

bool QgsTriangulatedSurface::deleteVertex( QgsVertexId vId )
{
  Q_UNUSED( vId )
  return false;
}

int QgsTriangulatedSurface::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsTriangulatedSurface *otherTriangulatedSurface = qgsgeometry_cast<const QgsTriangulatedSurface *>( other );
  if ( !otherTriangulatedSurface )
    return -1;

  const int nTriangles1 = mPatches.size();
  const int nTriangles2 = otherTriangulatedSurface->mPatches.size();
  if ( nTriangles1 < nTriangles2 )
  {
    return -1;
  }
  if ( nTriangles1 > nTriangles2 )
  {
    return 1;
  }

  for ( int i = 0; i < nTriangles1; i++ )
  {
    const int triangleComp = mPatches.at( i )->compareTo( otherTriangulatedSurface->mPatches.at( i ) );
    if ( triangleComp != 0 )
    {
      return triangleComp;
    }
  }

  return 0;
}
