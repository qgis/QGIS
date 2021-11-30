/***************************************************************************
                         qgscurvepolygon.cpp
                         ---------------------
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

#include "qgscurvepolygon.h"
#include "qgsapplication.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgswkbptr.h"
#include "qgsmulticurve.h"
#include "qgsfeedback.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <memory>
#include <nlohmann/json.hpp>

QgsCurvePolygon::QgsCurvePolygon()
{
  mWkbType = QgsWkbTypes::CurvePolygon;
}

QgsCurvePolygon::~QgsCurvePolygon()
{
  clear();
}

QgsCurvePolygon *QgsCurvePolygon::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsCurvePolygon >();
  result->mWkbType = mWkbType;
  return result.release();
}

QString QgsCurvePolygon::geometryType() const
{
  return QStringLiteral( "CurvePolygon" );
}

int QgsCurvePolygon::dimension() const
{
  return 2;
}

QgsCurvePolygon::QgsCurvePolygon( const QgsCurvePolygon &p )
  : QgsSurface( p )

{
  mWkbType = p.mWkbType;
  if ( p.mExteriorRing )
  {
    mExteriorRing.reset( p.mExteriorRing->clone() );
  }

  for ( const QgsCurve *ring : p.mInteriorRings )
  {
    mInteriorRings.push_back( ring->clone() );
  }
}

QgsCurvePolygon &QgsCurvePolygon::operator=( const QgsCurvePolygon &p )
{
  if ( &p != this )
  {
    clearCache();
    QgsSurface::operator=( p );
    if ( p.mExteriorRing )
    {
      mExteriorRing.reset( p.mExteriorRing->clone() );
    }

    for ( const QgsCurve *ring : p.mInteriorRings )
    {
      mInteriorRings.push_back( ring->clone() );
    }
  }
  return *this;
}

bool QgsCurvePolygon::operator==( const QgsAbstractGeometry &other ) const
{
  const QgsCurvePolygon *otherPolygon = qgsgeometry_cast< const QgsCurvePolygon * >( &other );
  if ( !otherPolygon )
    return false;

  //run cheap checks first
  if ( mWkbType != otherPolygon->mWkbType )
    return false;

  if ( ( !mExteriorRing && otherPolygon->mExteriorRing ) || ( mExteriorRing && !otherPolygon->mExteriorRing ) )
    return false;

  if ( mInteriorRings.count() != otherPolygon->mInteriorRings.count() )
    return false;

  // compare rings
  if ( mExteriorRing && otherPolygon->mExteriorRing )
  {
    if ( *mExteriorRing != *otherPolygon->mExteriorRing )
      return false;
  }

  for ( int i = 0; i < mInteriorRings.count(); ++i )
  {
    if ( ( !mInteriorRings.at( i ) && otherPolygon->mInteriorRings.at( i ) ) ||
         ( mInteriorRings.at( i ) && !otherPolygon->mInteriorRings.at( i ) ) )
      return false;

    if ( mInteriorRings.at( i ) && otherPolygon->mInteriorRings.at( i ) &&
         *mInteriorRings.at( i ) != *otherPolygon->mInteriorRings.at( i ) )
      return false;
  }

  return true;
}

bool QgsCurvePolygon::operator!=( const QgsAbstractGeometry &other ) const
{
  return !operator==( other );
}

QgsCurvePolygon *QgsCurvePolygon::clone() const
{
  return new QgsCurvePolygon( *this );
}

void QgsCurvePolygon::clear()
{
  mWkbType = QgsWkbTypes::CurvePolygon;
  mExteriorRing.reset();
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
  clearCache();
}


bool QgsCurvePolygon::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::CurvePolygon )
  {
    return false;
  }
  mWkbType = type;

  int nRings;
  wkbPtr >> nRings;
  std::unique_ptr< QgsCurve > currentCurve;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsWkbTypes::Type curveType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( curveType );
    if ( flatCurveType == QgsWkbTypes::LineString )
    {
      currentCurve.reset( new QgsLineString() );
    }
    else if ( flatCurveType == QgsWkbTypes::CircularString )
    {
      currentCurve.reset( new QgsCircularString() );
    }
    else if ( flatCurveType == QgsWkbTypes::CompoundCurve )
    {
      currentCurve.reset( new QgsCompoundCurve() );
    }
    else
    {
      return false;
    }
    currentCurve->fromWkb( wkbPtr );  // also updates wkbPtr
    if ( i == 0 )
    {
      mExteriorRing = std::move( currentCurve );
    }
    else
    {
      mInteriorRings.append( currentCurve.release() );
    }
  }

  return true;
}

bool QgsCurvePolygon::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != QgsWkbTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.remove( '(' ).remove( ')' ).simplified().remove( ' ' );
  if ( ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 ) ||
       secondWithoutParentheses.isEmpty() )
    return true;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType );
  for ( const QString &childWkt : blocks )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( childParts.first );
    if ( flatCurveType == QgsWkbTypes::LineString )
      mInteriorRings.append( new QgsLineString() );
    else if ( flatCurveType == QgsWkbTypes::CircularString )
      mInteriorRings.append( new QgsCircularString() );
    else if ( flatCurveType == QgsWkbTypes::CompoundCurve )
      mInteriorRings.append( new QgsCompoundCurve() );
    else
    {
      clear();
      return false;
    }
    if ( !mInteriorRings.back()->fromWkt( childWkt ) )
    {
      clear();
      return false;
    }
  }

  if ( mInteriorRings.isEmpty() )
  {
    clear();
    return false;
  }

  mExteriorRing.reset( mInteriorRings.takeFirst() );

  //scan through rings and check if dimensionality of rings is different to CurvePolygon.
  //if so, update the type dimensionality of the CurvePolygon to match
  bool hasZ = false;
  bool hasM = false;
  if ( mExteriorRing )
  {
    hasZ = hasZ || mExteriorRing->is3D();
    hasM = hasM || mExteriorRing->isMeasure();
  }
  for ( const QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    hasZ = hasZ || curve->is3D();
    hasM = hasM || curve->isMeasure();
    if ( hasZ && hasM )
      break;
  }
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

QgsRectangle QgsCurvePolygon::calculateBoundingBox() const
{
  if ( mExteriorRing )
  {
    return mExteriorRing->boundingBox();
  }
  return QgsRectangle();
}

int QgsCurvePolygon::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  if ( mExteriorRing )
  {
    binarySize += mExteriorRing->wkbSize( flags );
  }
  for ( const QgsCurve *curve : mInteriorRings )
  {
    binarySize += curve->wkbSize( flags );
  }
  return binarySize;
}

QByteArray QgsCurvePolygon::asWkb( WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsCurvePolygon::wkbSize( flags ) );
  QgsWkbPtr wkbPtr( wkbArray );
  wkbPtr << static_cast<char>( QgsApplication::endian() );
  wkbPtr << static_cast<quint32>( wkbType() );
  wkbPtr << static_cast<quint32>( ( mExteriorRing ? 1 : 0 ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    wkbPtr << mExteriorRing->asWkb( flags );
  }
  for ( const QgsCurve *curve : mInteriorRings )
  {
    wkbPtr << curve->asWkb( flags );
  }
  return wkbArray;
}

QString QgsCurvePolygon::asWkt( int precision ) const
{
  QString wkt = wktTypeStr();

  if ( isEmpty() )
    wkt += QLatin1String( " EMPTY" );
  else
  {
    wkt += QLatin1String( " (" );
    if ( mExteriorRing )
    {
      QString childWkt = mExteriorRing->asWkt( precision );
      if ( qgsgeometry_cast<QgsLineString *>( mExteriorRing.get() ) )
      {
        // Type names of linear geometries are omitted
        childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
      }
      wkt += childWkt + ',';
    }
    for ( const QgsCurve *curve : mInteriorRings )
    {
      QString childWkt = curve->asWkt( precision );
      if ( qgsgeometry_cast<const QgsLineString *>( curve ) )
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

QDomElement QgsCurvePolygon::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  // GML2 does not support curves
  QDomElement elemPolygon = doc.createElementNS( ns, QStringLiteral( "Polygon" ) );

  if ( isEmpty() )
    return elemPolygon;

  QDomElement elemOuterBoundaryIs = doc.createElementNS( ns, QStringLiteral( "outerBoundaryIs" ) );
  std::unique_ptr< QgsLineString > exteriorLineString( exteriorRing()->curveToLine() );
  QDomElement outerRing = exteriorLineString->asGml2( doc, precision, ns, axisOrder );
  outerRing.toElement().setTagName( QStringLiteral( "LinearRing" ) );
  elemOuterBoundaryIs.appendChild( outerRing );
  elemPolygon.appendChild( elemOuterBoundaryIs );
  std::unique_ptr< QgsLineString > interiorLineString;
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QDomElement elemInnerBoundaryIs = doc.createElementNS( ns, QStringLiteral( "innerBoundaryIs" ) );
    interiorLineString.reset( interiorRing( i )->curveToLine() );
    QDomElement innerRing = interiorLineString->asGml2( doc, precision, ns, axisOrder );
    innerRing.toElement().setTagName( QStringLiteral( "LinearRing" ) );
    elemInnerBoundaryIs.appendChild( innerRing );
    elemPolygon.appendChild( elemInnerBoundaryIs );
  }
  return elemPolygon;
}

QDomElement QgsCurvePolygon::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemCurvePolygon = doc.createElementNS( ns, QStringLiteral( "Polygon" ) );

  if ( isEmpty() )
    return elemCurvePolygon;

  QDomElement elemExterior = doc.createElementNS( ns, QStringLiteral( "exterior" ) );
  QDomElement curveElem = exteriorRing()->asGml3( doc, precision, ns, axisOrder );
  if ( curveElem.tagName() == QLatin1String( "LineString" ) )
  {
    curveElem.setTagName( QStringLiteral( "LinearRing" ) );
  }
  elemExterior.appendChild( curveElem );
  elemCurvePolygon.appendChild( elemExterior );

  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QDomElement elemInterior = doc.createElementNS( ns, QStringLiteral( "interior" ) );
    QDomElement innerRing = interiorRing( i )->asGml3( doc, precision, ns, axisOrder );
    if ( innerRing.tagName() == QLatin1String( "LineString" ) )
    {
      innerRing.setTagName( QStringLiteral( "LinearRing" ) );
    }
    elemInterior.appendChild( innerRing );
    elemCurvePolygon.appendChild( elemInterior );
  }
  return elemCurvePolygon;
}

json QgsCurvePolygon::asJsonObject( int precision ) const
{
  json coordinates( json::array( ) );
  if ( auto *lExteriorRing = exteriorRing() )
  {
    std::unique_ptr< QgsLineString > exteriorLineString( lExteriorRing->curveToLine() );
    QgsPointSequence exteriorPts;
    exteriorLineString->points( exteriorPts );
    coordinates.push_back( QgsGeometryUtils::pointsToJson( exteriorPts, precision ) );

    std::unique_ptr< QgsLineString > interiorLineString;
    for ( int i = 0, n = numInteriorRings(); i < n; ++i )
    {
      interiorLineString.reset( interiorRing( i )->curveToLine() );
      QgsPointSequence interiorPts;
      interiorLineString->points( interiorPts );
      coordinates.push_back( QgsGeometryUtils::pointsToJson( interiorPts, precision ) );
    }
  }
  return
  {
    {  "type", "Polygon"  },
    { "coordinates", coordinates }
  };
}

QString QgsCurvePolygon::asKml( int precision ) const
{
  QString kml;
  kml.append( QLatin1String( "<Polygon>" ) );
  if ( mExteriorRing )
  {
    kml.append( QLatin1String( "<outerBoundaryIs>" ) );
    kml.append( mExteriorRing->asKml( precision ) );
    kml.append( QLatin1String( "</outerBoundaryIs>" ) );
  }
  const QVector<QgsCurve *> &interiorRings = mInteriorRings;
  for ( const QgsCurve *ring : interiorRings )
  {
    kml.append( QLatin1String( "<innerBoundaryIs>" ) );
    kml.append( ring->asKml( precision ) );
    kml.append( QLatin1String( "</innerBoundaryIs>" ) );
  }
  kml.append( QLatin1String( "</Polygon>" ) );
  return kml;
}

void QgsCurvePolygon::normalize()
{
  // normalize rings
  if ( mExteriorRing )
    mExteriorRing->normalize();

  for ( QgsCurve *ring : std::as_const( mInteriorRings ) )
  {
    ring->normalize();
  }

  // sort rings
  std::sort( mInteriorRings.begin(), mInteriorRings.end(), []( const QgsCurve * a, const QgsCurve * b )
  {
    return a->compareTo( b ) > 0;
  } );

  // normalize ring orientation
  forceRHR();
}

double QgsCurvePolygon::area() const
{
  if ( !mExteriorRing )
  {
    return 0.0;
  }

  double totalArea = 0.0;

  if ( mExteriorRing->isRing() )
  {
    double area = 0.0;
    mExteriorRing->sumUpArea( area );
    totalArea += std::fabs( area );
  }

  for ( const QgsCurve *ring : mInteriorRings )
  {
    double area = 0.0;
    if ( ring->isRing() )
    {
      ring->sumUpArea( area );
      totalArea -= std::fabs( area );
    }
  }
  return totalArea;
}

double QgsCurvePolygon::perimeter() const
{
  if ( !mExteriorRing )
    return 0.0;

  //sum perimeter of rings
  double perimeter = mExteriorRing->length();
  for ( const QgsCurve *ring : mInteriorRings )
  {
    perimeter += ring->length();
  }
  return perimeter;
}

double QgsCurvePolygon::roundness() const
{
  const double p = perimeter();
  if ( qgsDoubleNear( p, 0.0 ) )
    return 0.0;

  return 4.0 * M_PI * area() / pow( p, 2.0 );
}

QgsPolygon *QgsCurvePolygon::surfaceToPolygon() const
{
  std::unique_ptr< QgsPolygon > polygon( new QgsPolygon() );
  if ( !mExteriorRing )
    return polygon.release();

  polygon->setExteriorRing( exteriorRing()->curveToLine() );
  QVector<QgsCurve *> interiors;
  int n = numInteriorRings();
  interiors.reserve( n );
  for ( int i = 0; i < n; ++i )
  {
    interiors.append( interiorRing( i )->curveToLine() );
  }
  polygon->setInteriorRings( interiors );
  return polygon.release();
}

QgsAbstractGeometry *QgsCurvePolygon::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  if ( mInteriorRings.isEmpty() )
  {
    return mExteriorRing->clone();
  }
  else
  {
    QgsMultiCurve *multiCurve = new QgsMultiCurve();
    int nInteriorRings = mInteriorRings.size();
    multiCurve->reserve( nInteriorRings + 1 );
    multiCurve->addGeometry( mExteriorRing->clone() );
    for ( int i = 0; i < nInteriorRings; ++i )
    {
      multiCurve->addGeometry( mInteriorRings.at( i )->clone() );
    }
    return multiCurve;
  }
}

QgsCurvePolygon *QgsCurvePolygon::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  if ( !mExteriorRing )
    return nullptr;


  std::unique_ptr< QgsCurvePolygon > polygon( createEmptyWithSameType() );

  // exterior ring
  auto exterior = std::unique_ptr<QgsCurve> { static_cast< QgsCurve *>( mExteriorRing->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing ) ) };

  if ( !exterior )
    return nullptr;

  polygon->mExteriorRing = std::move( exterior );

  //interior rings
  for ( auto interior : mInteriorRings )
  {
    if ( !interior )
      continue;

    QgsCurve *gridifiedInterior = static_cast< QgsCurve * >( interior->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing ) );

    if ( !gridifiedInterior )
      continue;

    polygon->mInteriorRings.append( gridifiedInterior );
  }

  return polygon.release();

}

bool QgsCurvePolygon::removeDuplicateNodes( double epsilon, bool useZValues )
{
  bool result = false;
  auto cleanRing = [epsilon, useZValues ]( QgsCurve * ring )->bool
  {
    if ( ring->numPoints() <= 4 )
      return false;

    if ( ring->removeDuplicateNodes( epsilon, useZValues ) )
    {
      QgsPoint startPoint;
      Qgis::VertexType type;
      ring->pointAt( 0, startPoint, type );
      // ensure ring is properly closed - if we removed the final node, it may no longer be properly closed
      ring->moveVertex( QgsVertexId( -1, -1, ring->numPoints() - 1 ), startPoint );
      return true;
    }

    return false;
  };
  if ( mExteriorRing )
  {
    result = cleanRing( mExteriorRing.get() );
  }
  for ( QgsCurve *ring : std::as_const( mInteriorRings ) )
  {
    if ( cleanRing( ring ) ) result = true;
  }
  return result;
}

bool QgsCurvePolygon::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  if ( !mExteriorRing && mInteriorRings.empty() )
    return false;

  // if we already have the bounding box calculated, then this check is trivial!
  if ( !mBoundingBox.isNull() )
  {
    return mBoundingBox.intersects( rectangle );
  }

  // loop through each ring and test the bounding box intersection.
  // This gives us a chance to use optimisations which may be present on the individual
  // ring geometry subclasses, and at worst it will cause a calculation of the bounding box
  // of each individual ring geometry which we would have to do anyway... (and these
  // bounding boxes are cached, so would be reused without additional expense)
  if ( mExteriorRing && mExteriorRing->boundingBoxIntersects( rectangle ) )
    return true;

  for ( const QgsCurve *ring : mInteriorRings )
  {
    if ( ring->boundingBoxIntersects( rectangle ) )
      return true;
  }

  // even if we don't intersect the bounding box of any rings, we may still intersect the
  // bounding box of the overall polygon (we are considering worst case scenario here and
  // the polygon is invalid, with rings outside the exterior ring!)
  // so here we fall back to the non-optimised base class check which has to first calculate
  // the overall bounding box of the polygon..
  return QgsSurface::boundingBoxIntersects( rectangle );
}

QgsPolygon *QgsCurvePolygon::toPolygon( double tolerance, SegmentationToleranceType toleranceType ) const
{
  std::unique_ptr< QgsPolygon > poly( new QgsPolygon() );
  if ( !mExteriorRing )
  {
    return poly.release();
  }

  poly->setExteriorRing( mExteriorRing->curveToLine( tolerance, toleranceType ) );

  QVector<QgsCurve *> rings;
  rings.reserve( mInteriorRings.size() );
  for ( const QgsCurve *ring : mInteriorRings )
  {
    rings.push_back( ring->curveToLine( tolerance, toleranceType ) );
  }
  poly->setInteriorRings( rings );
  return poly.release();
}

void QgsCurvePolygon::setExteriorRing( QgsCurve *ring )
{
  if ( !ring )
  {
    return;
  }
  mExteriorRing.reset( ring );

  //set proper wkb type
  if ( QgsWkbTypes::flatType( wkbType() ) == QgsWkbTypes::Polygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWkbTypes::Polygon );
  }
  else if ( QgsWkbTypes::flatType( wkbType() ) == QgsWkbTypes::CurvePolygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWkbTypes::CurvePolygon );
  }

  //match dimensionality for rings
  for ( QgsCurve *ring : std::as_const( mInteriorRings ) )
  {
    if ( is3D() )
      ring->addZValue();
    else
      ring->dropZValue();

    if ( isMeasure() )
      ring->addMValue();
    else
      ring->dropMValue();
  }
  clearCache();
}

void QgsCurvePolygon::setInteriorRings( const QVector<QgsCurve *> &rings )
{
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();

  //add rings one-by-one, so that they can each be converted to the correct type for the CurvePolygon
  for ( QgsCurve *ring : rings )
  {
    addInteriorRing( ring );
  }
  clearCache();
}

void QgsCurvePolygon::addInteriorRing( QgsCurve *ring )
{
  if ( !ring )
    return;

  //ensure dimensionality of ring matches curve polygon
  if ( !is3D() )
    ring->dropZValue();
  else if ( !ring->is3D() )
    ring->addZValue();

  if ( !isMeasure() )
    ring->dropMValue();
  else if ( !ring->isMeasure() )
    ring->addMValue();

  mInteriorRings.append( ring );
  clearCache();
}

bool QgsCurvePolygon::removeInteriorRing( int nr )
{
  if ( nr < 0 || nr >= mInteriorRings.size() )
  {
    return false;
  }
  delete mInteriorRings.takeAt( nr );
  clearCache();
  return true;
}

void QgsCurvePolygon::removeInteriorRings( double minimumAllowedArea )
{
  for ( int ringIndex = mInteriorRings.size() - 1; ringIndex >= 0; --ringIndex )
  {
    if ( minimumAllowedArea < 0 )
      delete mInteriorRings.takeAt( ringIndex );
    else
    {
      double area = 0.0;
      mInteriorRings.at( ringIndex )->sumUpArea( area );
      if ( area < minimumAllowedArea )
        delete mInteriorRings.takeAt( ringIndex );
    }
  }

  clearCache();
}

void QgsCurvePolygon::removeInvalidRings()
{
  QVector<QgsCurve *> validRings;
  validRings.reserve( mInteriorRings.size() );
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    if ( !curve->isRing() )
    {
      // remove invalid rings
      delete curve;
    }
    else
    {
      validRings << curve;
    }
  }
  mInteriorRings = validRings;
}

void QgsCurvePolygon::forceRHR()
{
  forceClockwise();
}

void QgsCurvePolygon::forceClockwise()
{
  if ( mExteriorRing && mExteriorRing->orientation() != Qgis::AngularDirection::Clockwise )
  {
    // flip exterior ring orientation
    std::unique_ptr< QgsCurve > flipped( mExteriorRing->reversed() );
    mExteriorRing = std::move( flipped );
  }

  QVector<QgsCurve *> validRings;
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    if ( curve && curve->orientation() != Qgis::AngularDirection::CounterClockwise )
    {
      // flip interior ring orientation
      QgsCurve *flipped = curve->reversed();
      validRings << flipped;
      delete curve;
    }
    else
    {
      validRings << curve;
    }
  }
  mInteriorRings = validRings;
}

void QgsCurvePolygon::forceCounterClockwise()
{
  if ( mExteriorRing && mExteriorRing->orientation() != Qgis::AngularDirection::CounterClockwise )
  {
    // flip exterior ring orientation
    mExteriorRing.reset( mExteriorRing->reversed() );
  }

  QVector<QgsCurve *> validRings;
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    if ( curve && curve->orientation() != Qgis::AngularDirection::Clockwise )
    {
      // flip interior ring orientation
      QgsCurve *flipped = curve->reversed();
      validRings << flipped;
      delete curve;
    }
    else
    {
      validRings << curve;
    }
  }
  mInteriorRings = validRings;
}

QPainterPath QgsCurvePolygon::asQPainterPath() const
{
  QPainterPath p;
  if ( mExteriorRing )
  {
    QPainterPath ring = mExteriorRing->asQPainterPath();
    ring.closeSubpath();
    p.addPath( ring );
  }

  for ( const QgsCurve *ring : mInteriorRings )
  {
    QPainterPath ringPath = ring->asQPainterPath();
    ringPath.closeSubpath();
    p.addPath( ringPath );
  }

  return p;
}

void QgsCurvePolygon::draw( QPainter &p ) const
{
  if ( !mExteriorRing )
    return;

  if ( mInteriorRings.empty() )
  {
    mExteriorRing->drawAsPolygon( p );
  }
  else
  {
    QPainterPath path;
    mExteriorRing->addToPainterPath( path );

    for ( const QgsCurve *ring : mInteriorRings )
    {
      ring->addToPainterPath( path );
    }
    p.drawPath( path );
  }
}

void QgsCurvePolygon::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( ct, d, transformZ );
  }

  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCurvePolygon::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( t, zTranslate, zScale, mTranslate, mScale );
  }

  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->transform( t, zTranslate, zScale, mTranslate, mScale );
  }
  clearCache();
}

QgsCoordinateSequence QgsCurvePolygon::coordinateSequence() const
{
  QgsCoordinateSequence sequence;
  sequence.append( QgsRingSequence() );

  if ( mExteriorRing )
  {
    sequence.back().append( QgsPointSequence() );
    mExteriorRing->points( sequence.back().back() );
  }

  for ( const QgsCurve *ring : mInteriorRings )
  {
    sequence.back().append( QgsPointSequence() );
    ring->points( sequence.back().back() );
  }

  return sequence;
}

int QgsCurvePolygon::nCoordinates() const
{
  int count = 0;

  if ( mExteriorRing )
  {
    count += mExteriorRing->nCoordinates();
  }

  for ( const QgsCurve *ring : mInteriorRings )
  {
    count += ring->nCoordinates();
  }

  return count;
}

int QgsCurvePolygon::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part != 0 )
    return -1;

  if ( id.ring < 0 || id.ring >= ringCount() || !mExteriorRing )
    return -1;

  int number = 0;
  if ( id.ring == 0 )
  {
    return mExteriorRing->vertexNumberFromVertexId( QgsVertexId( 0, 0, id.vertex ) );
  }
  else
  {
    number += mExteriorRing->numPoints();
  }

  for ( int i = 0; i < mInteriorRings.count(); ++i )
  {
    if ( id.ring == i + 1 )
    {
      int partNumber = mInteriorRings.at( i )->vertexNumberFromVertexId( QgsVertexId( 0, 0, id.vertex ) );
      if ( partNumber == -1 )
        return -1;
      return number + partNumber;
    }
    else
    {
      number += mInteriorRings.at( i )->numPoints();
    }
  }
  return -1; // should not happen
}

bool QgsCurvePolygon::isEmpty() const
{
  if ( !mExteriorRing )
    return true;

  return mExteriorRing->isEmpty();
}

double QgsCurvePolygon::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  if ( !mExteriorRing )
  {
    return -1;
  }
  QVector<QgsCurve *> segmentList;
  segmentList.append( mExteriorRing.get() );
  segmentList.append( mInteriorRings );
  return QgsGeometryUtils::closestSegmentFromComponents( segmentList, QgsGeometryUtils::Ring, pt, segmentPt,  vertexAfter, leftOf, epsilon );
}

bool QgsCurvePolygon::nextVertex( QgsVertexId &vId, QgsPoint &vertex ) const
{
  if ( !mExteriorRing || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  if ( vId.ring < 0 )
  {
    vId.ring = 0;
    vId.vertex = -1;
    if ( vId.part < 0 )
    {
      vId.part = 0;
    }
    return mExteriorRing->nextVertex( vId, vertex );
  }
  else
  {
    QgsCurve *ring = vId.ring == 0 ? mExteriorRing.get() : mInteriorRings[vId.ring - 1];

    if ( ring->nextVertex( vId, vertex ) )
    {
      return true;
    }
    ++vId.ring;
    vId.vertex = -1;
    if ( vId.ring >= 1 + mInteriorRings.size() )
    {
      return false;
    }
    ring = mInteriorRings[ vId.ring - 1 ];
    return ring->nextVertex( vId, vertex );
  }
}

void ringAdjacentVertices( const QgsCurve *curve, QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex )
{
  int n = curve->numPoints();
  if ( vertex.vertex < 0 || vertex.vertex >= n )
  {
    previousVertex = QgsVertexId();
    nextVertex = QgsVertexId();
    return;
  }

  if ( vertex.vertex == 0 && n < 3 )
  {
    previousVertex = QgsVertexId();
  }
  else if ( vertex.vertex == 0 )
  {
    previousVertex = QgsVertexId( vertex.part, vertex.ring, n - 2 );
  }
  else
  {
    previousVertex = QgsVertexId( vertex.part, vertex.ring, vertex.vertex - 1 );
  }
  if ( vertex.vertex == n - 1 && n < 3 )
  {
    nextVertex = QgsVertexId();
  }
  else if ( vertex.vertex == n - 1 )
  {
    nextVertex = QgsVertexId( vertex.part, vertex.ring, 1 );
  }
  else
  {
    nextVertex = QgsVertexId( vertex.part, vertex.ring, vertex.vertex + 1 );
  }
}

void QgsCurvePolygon::adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  if ( !mExteriorRing || vertex.ring < 0 || vertex.ring >= 1 + mInteriorRings.size() )
  {
    previousVertex = QgsVertexId();
    nextVertex = QgsVertexId();
    return;
  }

  if ( vertex.ring == 0 )
  {
    ringAdjacentVertices( mExteriorRing.get(), vertex, previousVertex, nextVertex );
  }
  else
  {
    ringAdjacentVertices( mInteriorRings.at( vertex.ring - 1 ), vertex, previousVertex, nextVertex );
  }
}

bool QgsCurvePolygon::insertVertex( QgsVertexId vId, const QgsPoint &vertex )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing.get() : mInteriorRings.at( vId.ring - 1 );
  int n = ring->numPoints();
  bool success = ring->insertVertex( QgsVertexId( 0, 0, vId.vertex ), vertex );
  if ( !success )
  {
    return false;
  }

  // If first or last vertex is inserted, re-sync the last/first vertex
  if ( vId.vertex == 0 )
    ring->moveVertex( QgsVertexId( 0, 0, n ), vertex );
  else if ( vId.vertex == n )
    ring->moveVertex( QgsVertexId( 0, 0, 0 ), vertex );

  clearCache();

  return true;
}

bool QgsCurvePolygon::moveVertex( QgsVertexId vId, const QgsPoint &newPos )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing.get() : mInteriorRings.at( vId.ring - 1 );
  int n = ring->numPoints();
  bool success = ring->moveVertex( vId, newPos );
  if ( success )
  {
    // If first or last vertex is moved, also move the last/first vertex
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, n - 1 ), newPos );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, 0 ), newPos );
    clearCache();
  }
  return success;
}

bool QgsCurvePolygon::deleteVertex( QgsVertexId vId )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing.get() : mInteriorRings.at( vId.ring - 1 );
  int n = ring->numPoints();
  if ( n <= 4 )
  {
    //no points will be left in ring, so remove whole ring
    if ( vId.ring == 0 )
    {
      mExteriorRing.reset();
      if ( !mInteriorRings.isEmpty() )
      {
        mExteriorRing.reset( mInteriorRings.takeFirst() );
      }
    }
    else
    {
      removeInteriorRing( vId.ring - 1 );
    }
    clearCache();
    return true;
  }

  bool success = ring->deleteVertex( vId );
  if ( success )
  {
    // If first or last vertex is removed, re-sync the last/first vertex
    // Do not use "n - 2", but "ring->numPoints() - 1" as more than one vertex
    // may have been deleted (e.g. with CircularString)
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( 0, 0, ring->numPoints() - 1 ), ring->vertexAt( QgsVertexId( 0, 0, 0 ) ) );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( 0, 0, 0 ), ring->vertexAt( QgsVertexId( 0, 0, ring->numPoints() - 1 ) ) );
    clearCache();
  }
  return success;
}

bool QgsCurvePolygon::hasCurvedSegments() const
{
  if ( mExteriorRing && mExteriorRing->hasCurvedSegments() )
  {
    return true;
  }

  for ( const QgsCurve *ring : mInteriorRings )
  {
    if ( ring->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometry *QgsCurvePolygon::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  return toPolygon( tolerance, toleranceType );
}

double QgsCurvePolygon::vertexAngle( QgsVertexId vertex ) const
{
  if ( !mExteriorRing || vertex.ring < 0 || vertex.ring >= 1 + mInteriorRings.size() )
  {
    //makes no sense - conversion of false to double!
    return false;
  }

  QgsCurve *ring = vertex.ring == 0 ? mExteriorRing.get() : mInteriorRings[vertex.ring - 1];
  return ring->vertexAngle( vertex );
}

int QgsCurvePolygon::vertexCount( int /*part*/, int ring ) const
{
  return ring == 0 ? mExteriorRing->vertexCount() : mInteriorRings[ring - 1]->vertexCount();
}

int QgsCurvePolygon::ringCount( int ) const
{
  return ( nullptr != mExteriorRing ) + mInteriorRings.size();
}

int QgsCurvePolygon::partCount() const
{
  return ringCount() > 0 ? 1 : 0;
}

QgsPoint QgsCurvePolygon::vertexAt( QgsVertexId id ) const
{
  return id.ring == 0 ? mExteriorRing->vertexAt( id ) : mInteriorRings[id.ring - 1]->vertexAt( id );
}

double QgsCurvePolygon::segmentLength( QgsVertexId startVertex ) const
{
  if ( !mExteriorRing || startVertex.ring < 0 || startVertex.ring >= 1 + mInteriorRings.size() )
  {
    return 0.0;
  }

  const QgsCurve *ring = startVertex.ring == 0 ? mExteriorRing.get() : mInteriorRings[startVertex.ring - 1];
  return ring->segmentLength( startVertex );
}

bool QgsCurvePolygon::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addZValue( zValue );
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addM( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addMValue( mValue );
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropZValue();
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropMValue();
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}

void QgsCurvePolygon::swapXy()
{
  if ( mExteriorRing )
    mExteriorRing->swapXy();
  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->swapXy();
  }
  clearCache();
}

QgsCurvePolygon *QgsCurvePolygon::toCurveType() const
{
  return clone();
}

bool QgsCurvePolygon::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
{
  if ( !transformer )
    return false;

  bool res = true;
  if ( mExteriorRing )
    res = mExteriorRing->transform( transformer, feedback );

  if ( !res || ( feedback && feedback->isCanceled() ) )
  {
    clearCache();
    return false;
  }

  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    res = curve->transform( transformer );

    if ( feedback && feedback->isCanceled() )
      res = false;

    if ( !res )
      break;
  }
  clearCache();
  return res;
}

void QgsCurvePolygon::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  if ( mExteriorRing )
    mExteriorRing->filterVertices( filter );

  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->filterVertices( filter );
  }
  clearCache();
}

void QgsCurvePolygon::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  if ( mExteriorRing )
    mExteriorRing->transformVertices( transform );

  for ( QgsCurve *curve : std::as_const( mInteriorRings ) )
  {
    curve->transformVertices( transform );
  }
  clearCache();
}

int QgsCurvePolygon::childCount() const
{
  return 1 + mInteriorRings.count();
}

QgsAbstractGeometry *QgsCurvePolygon::childGeometry( int index ) const
{
  if ( index == 0 )
    return mExteriorRing.get();
  else
    return mInteriorRings.at( index - 1 );
}

int QgsCurvePolygon::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsCurvePolygon *otherPolygon = qgsgeometry_cast<const QgsCurvePolygon *>( other );
  if ( !otherPolygon )
    return -1;

  if ( mExteriorRing && !otherPolygon->mExteriorRing )
    return 1;
  else if ( !mExteriorRing && otherPolygon->mExteriorRing )
    return -1;
  else if ( mExteriorRing && otherPolygon->mExteriorRing )
  {
    int shellComp = mExteriorRing->compareTo( otherPolygon->mExteriorRing.get() );
    if ( shellComp != 0 )
    {
      return shellComp;
    }
  }

  const int nHole1 = mInteriorRings.size();
  const int nHole2 = otherPolygon->mInteriorRings.size();
  if ( nHole1 < nHole2 )
  {
    return -1;
  }
  if ( nHole1 > nHole2 )
  {
    return 1;
  }

  for ( int i = 0; i < nHole1; i++ )
  {
    const int holeComp = mInteriorRings.at( i )->compareTo( otherPolygon->mInteriorRings.at( i ) );
    if ( holeComp != 0 )
    {
      return holeComp;
    }
  }

  return 0;
}
