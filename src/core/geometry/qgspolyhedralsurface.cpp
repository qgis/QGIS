/***************************************************************************
                         qgspolyhedralsurface.cpp
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

#include "qgspolyhedralsurface.h"
#include "qgsapplication.h"
#include "qgscurve.h"
#include "qgsfeedback.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgspolygon.h"
#include "qgsvertexid.h"
#include "qgswkbptr.h"
#include "qgsmultilinestring.h"
#include "qgsgeos.h"

#include <QPainter>
#include <QPainterPath>
#include <memory>
#include <nlohmann/json.hpp>

QgsPolyhedralSurface::QgsPolyhedralSurface()
{
  mWkbType = Qgis::WkbType::PolyhedralSurface;
}

QgsPolyhedralSurface::QgsPolyhedralSurface( const QgsMultiPolygon *multiPolygon )
{
  if ( multiPolygon->isEmpty() )
  {
    return;
  }

  mPatches.reserve( multiPolygon->numGeometries() );
  for ( int i = 0; i < multiPolygon->numGeometries(); ++i )
  {
    mPatches.append( multiPolygon->polygonN( i )->clone() );
  }

  setZMTypeFromSubGeometry( multiPolygon->polygonN( 0 ), Qgis::WkbType::PolyhedralSurface );
}

QgsPolyhedralSurface::~QgsPolyhedralSurface()
{
  QgsPolyhedralSurface::clear();
}

QgsPolyhedralSurface *QgsPolyhedralSurface::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsPolyhedralSurface >();
  result->mWkbType = mWkbType;
  return result.release();
}

QString QgsPolyhedralSurface::geometryType() const
{
  return QStringLiteral( "PolyhedralSurface" );
}

int QgsPolyhedralSurface::dimension() const
{
  return 2;
}

QgsPolyhedralSurface::QgsPolyhedralSurface( const QgsPolyhedralSurface &p )
  : QgsSurface( p )

{
  mWkbType = p.mWkbType;
  mPatches.reserve( p.mPatches.size() );

  for ( const QgsPolygon *patch : p.mPatches )
  {
    mPatches.push_back( patch->clone() );
  }

  mBoundingBox = p.mBoundingBox;
  mHasCachedValidity = p.mHasCachedValidity;
  mValidityFailureReason = p.mValidityFailureReason;
}

// cppcheck-suppress operatorEqVarError
QgsPolyhedralSurface &QgsPolyhedralSurface::operator=( const QgsPolyhedralSurface &p )
{
  if ( &p != this )
  {
    QgsSurface::operator=( p );
    mPatches.reserve( p.mPatches.size() );

    for ( const QgsPolygon *patch : p.mPatches )
    {
      mPatches.push_back( patch->clone() );
    }
  }
  return *this;
}

QgsPolyhedralSurface *QgsPolyhedralSurface::clone() const
{
  return new QgsPolyhedralSurface( *this );
}

void QgsPolyhedralSurface::clear()
{
  mWkbType = Qgis::WkbType::PolyhedralSurface;
  qDeleteAll( mPatches );
  mPatches.clear();
  clearCache();
}


bool QgsPolyhedralSurface::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();

  if ( !wkbPtr )
  {
    return false;
  }

  Qgis::WkbType type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != Qgis::WkbType::PolyhedralSurface )
  {
    return false;
  }
  mWkbType = type;

  int nPatches;
  wkbPtr >> nPatches;
  std::unique_ptr< QgsPolygon > currentPatch;
  for ( int i = 0; i < nPatches; ++i )
  {
    Qgis::WkbType polygonType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    Qgis::WkbType flatPolygonType = QgsWkbTypes::flatType( polygonType );
    if ( flatPolygonType == Qgis::WkbType::Polygon )
    {
      currentPatch.reset( new QgsPolygon() );
    }
    else
    {
      return false;
    }
    currentPatch->fromWkb( wkbPtr );  // also updates wkbPtr
    mPatches.append( currentPatch.release() );
  }

  return true;
}

bool QgsPolyhedralSurface::fromWkt( const QString &wkt )
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

  QString defaultChildWkbType = QStringLiteral( "Polygon%1%2" ).arg( is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType );
  for ( const QString &childWkt : blocks )
  {
    QPair<Qgis::WkbType, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    if ( QgsWkbTypes::flatType( childParts.first ) == Qgis::WkbType::Polygon )
    {
      mPatches.append( new QgsPolygon() );
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

QgsBox3D QgsPolyhedralSurface::calculateBoundingBox3D() const
{
  if ( mPatches.empty() )
  {
    return QgsBox3D();
  }

  QgsBox3D bbox = mPatches.at( 0 )->boundingBox3D();
  for ( int i = 1; i < mPatches.size(); ++i )
  {
    QgsBox3D polygonBox = mPatches.at( i )->boundingBox3D();
    bbox.combineWith( polygonBox );
  }
  return bbox;
}

int QgsPolyhedralSurface::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  for ( const QgsPolygon *patch : mPatches )
  {
    binarySize += patch->wkbSize( flags );
  }
  return binarySize;
}

QByteArray QgsPolyhedralSurface::asWkb( WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsPolyhedralSurface::wkbSize( flags ) );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mPatches.size() );
  for ( const QgsPolygon *patch : mPatches )
  {
    wkb << patch->asWkb( flags );
  }
  return wkbArray;
}

QString QgsPolyhedralSurface::asWkt( int precision ) const
{
  QString wkt = wktTypeStr();

  if ( isEmpty() )
    wkt += QLatin1String( " EMPTY" );
  else
  {
    wkt += QLatin1String( " (" );
    for ( const QgsPolygon *patch : mPatches )
    {
      QString childWkt = patch->asWkt( precision );
      if ( qgsgeometry_cast<const QgsPolygon *>( patch ) )
      {
        // Type names of linear geometries are omitted
        childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
      }
      wkt += childWkt + ',';
    }
    if ( wkt.endsWith( ',' ) )
    {
      wkt.chop( 1 ); // Remove last ','
    }
    wkt += ')';
  }
  return wkt;
}

QDomElement QgsPolyhedralSurface::asGml2( QDomDocument &, int, const QString &, const AxisOrder ) const
{
  QgsDebugError( QStringLiteral( "gml version 2 does not support PolyhedralSurface geometry" ) );
  return QDomElement();
}

QDomElement QgsPolyhedralSurface::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemPolyhedralSurface = doc.createElementNS( ns, QStringLiteral( "PolyhedralSurface" ) );

  if ( isEmpty() )
    return elemPolyhedralSurface;

  QDomElement elemPolygonPatches = doc.createElementNS( ns, QStringLiteral( "polygonPatches" ) );

  for ( QgsPolygon *patch : mPatches )
  {
    QDomElement elemPolygonPatch = patch->asGml3( doc, precision, ns, axisOrder );
    elemPolygonPatch.setTagName( "PolygonPatch" );

    elemPolygonPatches.appendChild( elemPolygonPatch );
  }
  elemPolyhedralSurface.appendChild( elemPolygonPatches );

  return elemPolyhedralSurface;
}

json QgsPolyhedralSurface::asJsonObject( int precision ) const
{
  // GeoJSON format does not support PolyhedralSurface geometry
  // Return a multipolygon instead;
  std::unique_ptr<QgsMultiPolygon> multiPolygon( toMultiPolygon() );
  return multiPolygon->asJsonObject( precision );
}

QString QgsPolyhedralSurface::asKml( int ) const
{
  QgsDebugError( QStringLiteral( "kml format does not support PolyhedralSurface geometry" ) );
  return QString( "" );
}

void QgsPolyhedralSurface::normalize()
{
  for ( QgsPolygon *patch : mPatches )
  {
    QgsCurve *exteriorRing = patch->exteriorRing();
    if ( !exteriorRing )
      continue;

    exteriorRing->normalize();

    if ( patch->numInteriorRings() > 0 )
    {
      QVector<QgsCurve *> interiorRings;
      for ( int i = 0, n = patch->numInteriorRings(); i < n; ++i )
      {
        interiorRings.push_back( patch->interiorRing( i )->clone() );
      }

      // sort rings
      std::sort( interiorRings.begin(), interiorRings.end(), []( const QgsCurve * a, const QgsCurve * b )
      {
        return a->compareTo( b ) > 0;
      } );

      patch->removeInteriorRings();
      for ( QgsCurve *curve : interiorRings )
        patch->addInteriorRing( curve );
    }
  }
}

double QgsPolyhedralSurface::area() const
{
  // sum area of patches
  double area = 0.0;
  for ( const QgsPolygon *patch : mPatches )
  {
    area += patch->area();
  }

  return area;
}

double QgsPolyhedralSurface::perimeter() const
{
  // sum perimeter of patches
  double perimeter = 0.0;
  for ( const QgsPolygon *patch : mPatches )
  {
    perimeter += patch->perimeter();
  }

  return perimeter;
}

QgsAbstractGeometry *QgsPolyhedralSurface::boundary() const
{
  std::unique_ptr< QgsMultiLineString > multiLine( new QgsMultiLineString() );
  multiLine->reserve( mPatches.size() );
  for ( QgsPolygon *polygon : mPatches )
  {
    std::unique_ptr<QgsAbstractGeometry> polygonBoundary( polygon->boundary() );
    if ( QgsLineString *lineStringBoundary = qgsgeometry_cast< QgsLineString * >( polygonBoundary.get() ) )
    {
      multiLine->addGeometry( lineStringBoundary->clone() );
    }
    else if ( QgsMultiLineString *multiLineStringBoundary = qgsgeometry_cast< QgsMultiLineString * >( polygonBoundary.get() ) )
    {
      for ( int j = 0; j < multiLineStringBoundary->numGeometries(); ++j )
      {
        multiLine->addGeometry( multiLineStringBoundary->geometryN( j )->clone() );
      }
    }
  }

  if ( multiLine->numGeometries() == 0 )
  {
    return nullptr;
  }
  return multiLine.release();
}

QgsPolyhedralSurface *QgsPolyhedralSurface::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing, bool removeRedundantPoints ) const
{
  std::unique_ptr< QgsPolyhedralSurface > surface( createEmptyWithSameType() );

  for ( QgsPolygon *patch : mPatches )
  {
    // exterior ring
    std::unique_ptr<QgsCurve> exteriorRing( static_cast< QgsCurve *>( patch->exteriorRing()->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing, removeRedundantPoints ) ) );
    if ( !exteriorRing )
    {
      return nullptr;
    }

    std::unique_ptr<QgsPolygon> gridifiedPatch = std::make_unique<QgsPolygon>();
    gridifiedPatch->setExteriorRing( exteriorRing.release() );

    //interior rings
    for ( int i = 0, n = patch->numInteriorRings(); i < n; ++i )
    {
      QgsCurve *interiorRing = patch->interiorRing( i );
      if ( !interiorRing )
        continue;

      std::unique_ptr<QgsCurve> gridifiedInterior( static_cast< QgsCurve * >( interiorRing->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing, removeRedundantPoints ) ) );
      if ( gridifiedInterior )
        gridifiedPatch->addInteriorRing( gridifiedInterior.release() );
    }

    surface->addPatch( gridifiedPatch.release() );
  }

  return surface.release();
}

QgsPolyhedralSurface *QgsPolyhedralSurface::simplifyByDistance( double tolerance ) const
{
  if ( isEmpty() )
    return nullptr;

  std::unique_ptr< QgsPolyhedralSurface > simplifiedGeom = std::make_unique< QgsPolyhedralSurface >();
  for ( QgsPolygon *polygon : mPatches )
  {
    std::unique_ptr<QgsCurvePolygon> polygonSimplified( polygon->simplifyByDistance( tolerance ) );
    simplifiedGeom->addPatch( polygonSimplified->surfaceToPolygon() );
  }
  return simplifiedGeom.release();
}

bool QgsPolyhedralSurface::removeDuplicateNodes( double epsilon, bool useZValues )
{
  bool result = false;

  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    if ( patch->removeDuplicateNodes( epsilon, useZValues ) )
    {
      result = true;
    }
  }
  return result;
}

bool QgsPolyhedralSurface::boundingBoxIntersects( const QgsBox3D &box3d ) const
{
  // if we already have the bounding box calculated, then this check is trivial!
  if ( !mBoundingBox.isNull() )
  {
    return mBoundingBox.intersects( box3d );
  }

  // loop through each patch and test the bounding box intersection.
  // This gives us a chance to use optimisations which may be present on the individual
  // ring geometry subclasses, and at worst it will cause a calculation of the bounding box
  // of each individual patch geometry which we would have to do anyway... (and these
  // bounding boxes are cached, so would be reused without additional expense)
  for ( const QgsPolygon *patch : mPatches )
  {
    if ( patch->boundingBoxIntersects( box3d ) )
      return true;
  }

  // even if we don't intersect the bounding box of any rings, we may still intersect the
  // bounding box of the overall polygon (we are considering worst case scenario here and
  // the polygon is invalid, with rings outside the exterior ring!)
  // so here we fall back to the non-optimised base class check which has to first calculate
  // the overall bounding box of the polygon..
  return QgsSurface::boundingBoxIntersects( box3d );
}

void QgsPolyhedralSurface::setPatches( const QVector<QgsPolygon *> &patches )
{
  qDeleteAll( mPatches );
  mPatches.clear();

  for ( QgsPolygon *patch : patches )
  {
    addPatch( patch );
  }

  clearCache();
}

void QgsPolyhedralSurface::addPatch( QgsPolygon *patch )
{
  if ( !patch )
    return;

  if ( mPatches.empty() )
  {
    setZMTypeFromSubGeometry( patch, Qgis::WkbType::PolyhedralSurface );
  }

  // Ensure dimensionality of patch matches polyhedral surface
  if ( !is3D() )
    patch->dropZValue();
  else if ( !patch->is3D() )
    patch->addZValue();

  if ( !isMeasure() )
    patch->dropMValue();
  else if ( !patch->isMeasure() )
    patch->addMValue();

  mPatches.append( patch );
  clearCache();
}

bool QgsPolyhedralSurface::removePatch( int patchIndex )
{
  if ( patchIndex < 0 || patchIndex >= mPatches.size() )
  {
    return false;
  }

  delete mPatches.takeAt( patchIndex );
  clearCache();
  return true;
}

QPainterPath QgsPolyhedralSurface::asQPainterPath() const
{
  QPainterPath painterPath;
  for ( const QgsPolygon *patch : mPatches )
  {
    QPainterPath patchPath = patch->asQPainterPath();
    patchPath.closeSubpath();
    painterPath.addPath( patchPath );
  }

  return painterPath;
}

void QgsPolyhedralSurface::draw( QPainter &p ) const
{
  if ( mPatches.empty() )
    return;

  for ( const QgsPolygon *patch : mPatches )
  {
    patch->draw( p );
  }
}

void QgsPolyhedralSurface::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsPolyhedralSurface::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->transform( t, zTranslate, zScale, mTranslate, mScale );
  }
  clearCache();
}

QgsCoordinateSequence QgsPolyhedralSurface::coordinateSequence() const
{
  QgsCoordinateSequence sequence;
  for ( const QgsPolygon *polygon : std::as_const( mPatches ) )
  {
    QgsCoordinateSequence polyCoords = polygon->coordinateSequence();
    QgsCoordinateSequence::const_iterator cIt = polyCoords.constBegin();
    for ( ; cIt != polyCoords.constEnd(); ++cIt )
    {
      sequence.push_back( *cIt );
    }
  }

  return sequence;
}

int QgsPolyhedralSurface::nCoordinates() const
{
  int count = 0;
  for ( const QgsPolygon *patch : mPatches )
  {
    count += patch->nCoordinates();
  }
  return count;
}

int QgsPolyhedralSurface::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part < 0 || id.part >= partCount() )
    return -1;

  int number = 0;
  for ( int i = 0; i < mPatches.count(); ++i )
  {
    if ( id.part == i )
    {
      int partNumber = mPatches.at( i )->vertexNumberFromVertexId( QgsVertexId( 0, id.ring, id.vertex ) );
      if ( partNumber == -1 )
      {
        return -1;
      }

      return number + partNumber;
    }
    else
    {
      number += mPatches.at( i )->nCoordinates();
    }
  }

  return -1; // should not happen
}

bool QgsPolyhedralSurface::isEmpty() const
{
  return mPatches.isEmpty();
}

double QgsPolyhedralSurface::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  QVector<QgsPolygon *> segmentList = mPatches;
  return QgsGeometryUtils::closestSegmentFromComponents( segmentList, QgsGeometryUtils::Part, pt, segmentPt,  vertexAfter, leftOf, epsilon );
}

bool QgsPolyhedralSurface::nextVertex( QgsVertexId &vId, QgsPoint &vertex ) const
{
  if ( vId.part < 0 )
  {
    vId.part = 0;
    vId.ring = -1;
    vId.vertex = -1;
  }

  if ( isEmpty() || vId.part >= partCount() )
  {
    return false;
  }

  QgsPolygon *patch = mPatches[vId.part];
  if ( patch->nextVertex( vId, vertex ) )
  {
    return true;
  }

  ++vId.part;
  vId.ring = 0;
  vId.vertex = -1;
  if ( vId.part >= partCount() )
  {
    return false;
  }
  patch = mPatches[vId.part];
  return patch->nextVertex( vId, vertex );
}

void QgsPolyhedralSurface::adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  if ( vertex.part < 0 || vertex.part >= partCount() )
  {
    previousVertex = QgsVertexId();
    nextVertex = QgsVertexId();
    return;
  }

  QgsPolygon *patch = mPatches[vertex.ring];
  patch->adjacentVertices( QgsVertexId( 0, 0, vertex.vertex ), previousVertex, nextVertex );
  return;
}

bool QgsPolyhedralSurface::insertVertex( QgsVertexId vId, const QgsPoint &vertex )
{
  if ( vId.part < 0 || vId.part >= partCount() )
  {
    return false;
  }

  QgsPolygon *patch = mPatches.at( vId.part );
  bool success = patch->insertVertex( QgsVertexId( 0, vId.ring, vId.vertex ), vertex );
  if ( success )
  {
    clearCache();
  }

  return success;
}

bool QgsPolyhedralSurface::moveVertex( QgsVertexId vId, const QgsPoint &newPos )
{
  if ( vId.part < 0 || vId.part >= partCount() )
  {
    return false;
  }

  QgsPolygon *patch = mPatches.at( vId.part );
  bool success = patch->moveVertex( QgsVertexId( 0, vId.ring, vId.vertex ), newPos );
  if ( success )
  {
    clearCache();
  }

  return success;
}

bool QgsPolyhedralSurface::deleteVertex( QgsVertexId vId )
{
  if ( vId.part < 0 || vId.part >= partCount() )
  {
    return false;
  }

  QgsPolygon *patch = mPatches.at( vId.part );
  bool success = patch->deleteVertex( QgsVertexId( 0, vId.ring, vId.vertex ) );
  if ( success )
  {
    // if the patch has lost its exterior ring, remove it
    if ( !patch->exteriorRing() )
    {
      delete mPatches.takeAt( vId.part );
    }
    clearCache();
  }

  return success;
}

bool QgsPolyhedralSurface::hasCurvedSegments() const
{
  return false;
}

QgsAbstractGeometry *QgsPolyhedralSurface::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  // This is only used by curves
  Q_UNUSED( tolerance )
  Q_UNUSED( toleranceType )
  return clone();
}

double QgsPolyhedralSurface::vertexAngle( QgsVertexId vertex ) const
{
  if ( vertex.part < 0 || vertex.part >= partCount() )
  {
    return 0.0;
  }

  QgsPolygon *patch = mPatches[vertex.part];
  return patch->vertexAngle( QgsVertexId( 0, vertex.ring, vertex.vertex ) );
}

int QgsPolyhedralSurface::vertexCount( int part, int ring ) const
{
  if ( part < 0 || part >= partCount() )
  {
    return 0;
  }

  QgsPolygon *patchPolygon = mPatches[part];
  QgsCurve *ringCurve = ring == 0 ? patchPolygon->exteriorRing() : patchPolygon->interiorRing( ring - 1 );
  if ( ringCurve )
  {
    return ringCurve->vertexCount();
  }

  return 0;
}

int QgsPolyhedralSurface::ringCount( int part ) const
{
  if ( part < 0 || part >= partCount() )
    return 0;

  return mPatches[part]->ringCount();
}

int QgsPolyhedralSurface::partCount() const
{
  return mPatches.size();
}

QgsPoint QgsPolyhedralSurface::vertexAt( QgsVertexId id ) const
{
  if ( id.part < 0 || id.part >= partCount() )
    return QgsPoint();

  return mPatches[id.part]->vertexAt( id );
}

double QgsPolyhedralSurface::segmentLength( QgsVertexId startVertex ) const
{
  if ( startVertex.part < 0 || startVertex.part >= partCount() )
  {
    return 0.0;
  }

  const QgsPolygon *patch = mPatches[startVertex.part];
  return patch->segmentLength( QgsVertexId( 0, startVertex.ring, startVertex.vertex ) );
}

bool QgsPolyhedralSurface::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
  {
    return false;
  }

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsPolyhedralSurface::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
  {
    return false;
  }

  mWkbType = QgsWkbTypes::addM( mWkbType );

  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsPolyhedralSurface::dropZValue()
{
  if ( !is3D() )
  {
    return false;
  }

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsPolyhedralSurface::dropMValue()
{
  if ( !isMeasure() )
  {
    return false;
  }

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->dropMValue();
  }
  clearCache();
  return true;
}

void QgsPolyhedralSurface::swapXy()
{
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->swapXy();
  }
  clearCache();
}

QgsMultiSurface *QgsPolyhedralSurface::toCurveType() const
{
  std::unique_ptr<QgsMultiSurface> multiSurface = std::make_unique< QgsMultiSurface >();
  multiSurface->reserve( mPatches.size() );
  for ( const QgsPolygon *polygon : std::as_const( mPatches ) )
  {
    multiSurface->addGeometry( polygon->clone() );
  }
  return multiSurface.release();
}

bool QgsPolyhedralSurface::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
{
  if ( !transformer )
    return false;

  bool res = true;

  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    res = patch->transform( transformer );
    if ( feedback && feedback->isCanceled() )
      res = false;

    if ( !res )
      break;
  }

  clearCache();
  return res;
}

QgsMultiPolygon *QgsPolyhedralSurface::toMultiPolygon() const
{
  std::unique_ptr<QgsMultiPolygon> multiPolygon = std::make_unique< QgsMultiPolygon >();
  multiPolygon->reserve( mPatches.size() );
  for ( const QgsPolygon *polygon : std::as_const( mPatches ) )
  {
    multiPolygon->addGeometry( polygon->clone() );
  }
  return multiPolygon.release();
}

void QgsPolyhedralSurface::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->filterVertices( filter );
  }

  clearCache();
}

void QgsPolyhedralSurface::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  for ( QgsPolygon *patch : std::as_const( mPatches ) )
  {
    patch->transformVertices( transform );
  }

  clearCache();
}

int QgsPolyhedralSurface::childCount() const
{
  return mPatches.count();
}

QgsAbstractGeometry *QgsPolyhedralSurface::childGeometry( int index ) const
{
  return mPatches.at( index );
}

int QgsPolyhedralSurface::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsPolyhedralSurface *otherPolySurface = qgsgeometry_cast<const QgsPolyhedralSurface *>( other );
  if ( !otherPolySurface )
    return -1;

  const int nPatches1 = mPatches.size();
  const int nPatches2 = otherPolySurface->mPatches.size();
  if ( nPatches1 < nPatches2 )
  {
    return -1;
  }
  if ( nPatches1 > nPatches2 )
  {
    return 1;
  }

  for ( int i = 0; i < nPatches1; i++ )
  {
    const int polygonComp = mPatches.at( i )->compareTo( otherPolySurface->mPatches.at( i ) );
    if ( polygonComp != 0 )
    {
      return polygonComp;
    }
  }

  return 0;
}

bool QgsPolyhedralSurface::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  if ( flags == 0 && mHasCachedValidity )
  {
    // use cached validity results
    error = mValidityFailureReason;
    return error.isEmpty();
  }

  if ( isEmpty() )
    return true;

  error.clear();

  // GEOS does not handle PolyhedralSurface, check the polygons one by one
  for ( int i = 0; i < mPatches.size(); ++i )
  {
    const QgsGeos geos( mPatches.at( i ), 0, Qgis::GeosCreationFlags() );
    const bool valid = geos.isValid( &error, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, nullptr );
    if ( !valid )
    {
      error = QStringLiteral( "Polygon %1 is invalid: %2" ).arg( QString::number( i ), error );
      break;
    }
  }

  //TODO: Also check that the polyhedral surface is connected
  // For example, see SFCGAL implementation:
  // https://gitlab.com/sfcgal/SFCGAL/-/blob/19e3ff0c9057542a0e271edfee873d5f8b220871/src/algorithm/isValid.cpp#L469

  const bool valid = error.isEmpty();
  if ( flags == 0 )
  {
    mValidityFailureReason = !valid ? error : QString();
    mHasCachedValidity = true;
  }

  return valid;
}
