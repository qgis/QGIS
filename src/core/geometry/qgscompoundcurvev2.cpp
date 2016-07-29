/***************************************************************************
                         qgscompoundcurvev2.cpp
                         ----------------------
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

#include "qgscompoundcurvev2.h"
#include "qgsapplication.h"
#include "qgscircularstringv2.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>


QgsCompoundCurveV2::QgsCompoundCurveV2(): QgsCurveV2()
{
  mWkbType = QgsWKBTypes::CompoundCurve;
}

QgsCompoundCurveV2::~QgsCompoundCurveV2()
{
  clear();
}

bool QgsCompoundCurveV2::operator==( const QgsCurveV2& other ) const
{
  const QgsCompoundCurveV2* otherCurve = dynamic_cast< const QgsCompoundCurveV2* >( &other );
  if ( !otherCurve )
    return false;

  return *otherCurve == *this;
}

bool QgsCompoundCurveV2::operator!=( const QgsCurveV2& other ) const
{
  return !operator==( other );
}

QgsCompoundCurveV2::QgsCompoundCurveV2( const QgsCompoundCurveV2& curve ): QgsCurveV2( curve )
{
  mWkbType = QgsWKBTypes::CompoundCurve;
  Q_FOREACH ( const QgsCurveV2* c, curve.mCurves )
  {
    mCurves.append( static_cast<QgsCurveV2*>( c->clone() ) );
  }
}

QgsCompoundCurveV2& QgsCompoundCurveV2::operator=( const QgsCompoundCurveV2 & curve )
{
  if ( &curve != this )
  {
    clearCache();
    QgsCurveV2::operator=( curve );
    Q_FOREACH ( const QgsCurveV2* c, curve.mCurves )
    {
      mCurves.append( static_cast<QgsCurveV2*>( c->clone() ) );
    }
  }
  return *this;
}

QgsCompoundCurveV2 *QgsCompoundCurveV2::clone() const
{
  return new QgsCompoundCurveV2( *this );
}

void QgsCompoundCurveV2::clear()
{
  mWkbType = QgsWKBTypes::CompoundCurve;
  qDeleteAll( mCurves );
  mCurves.clear();
  clearCache();
}

QgsRectangle QgsCompoundCurveV2::calculateBoundingBox() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsRectangle();
  }

  QgsRectangle bbox = mCurves.at( 0 )->boundingBox();
  for ( int i = 1; i < mCurves.size(); ++i )
  {
    QgsRectangle curveBox = mCurves.at( i )->boundingBox();
    bbox.combineExtentWith( curveBox );
  }
  return bbox;
}

bool QgsCompoundCurveV2::fromWkb( QgsConstWkbPtr wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::CompoundCurve )
  {
    return false;
  }
  mWkbType = type;

  int nCurves;
  wkbPtr >> nCurves;
  QgsCurveV2* currentCurve = nullptr;
  int currentCurveSize = 0;
  for ( int i = 0; i < nCurves; ++i )
  {
    QgsWKBTypes::Type curveType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    if ( QgsWKBTypes::flatType( curveType ) == QgsWKBTypes::LineString )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( QgsWKBTypes::flatType( curveType ) == QgsWKBTypes::CircularString )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else
    {
      return false;
    }
    currentCurve->fromWkb( wkbPtr );
    currentCurveSize = currentCurve->wkbSize();
    mCurves.append( currentCurve );
    wkbPtr += currentCurveSize;
  }
  return true;
}

bool QgsCompoundCurveV2::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::CompoundCurve )
    return false;
  mWkbType = parts.first;

  QString defaultChildWkbType = QString( "LineString%1%2" ).arg( is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWKBTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    if ( QgsWKBTypes::flatType( childParts.first ) == QgsWKBTypes::LineString )
      mCurves.append( new QgsLineStringV2() );
    else if ( QgsWKBTypes::flatType( childParts.first ) == QgsWKBTypes::CircularString )
      mCurves.append( new QgsCircularStringV2() );
    else
    {
      clear();
      return false;
    }
    if ( !mCurves.back()->fromWkt( childWkt ) )
    {
      clear();
      return false;
    }
  }

  //scan through curves and check if dimensionality of curves is different to compound curve.
  //if so, update the type dimensionality of the compound curve to match
  bool hasZ = false;
  bool hasM = false;
  Q_FOREACH ( const QgsCurveV2* curve, mCurves )
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

int QgsCompoundCurveV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  Q_FOREACH ( const QgsCurveV2 *curve, mCurves )
  {
    size += curve->wkbSize();
  }
  return size;
}

unsigned char* QgsCompoundCurveV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr, binarySize );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mCurves.size() );
  Q_FOREACH ( const QgsCurveV2* curve, mCurves )
  {
    int curveWkbLen = 0;
    unsigned char* curveWkb = curve->asWkb( curveWkbLen );
    memcpy( wkb, curveWkb, curveWkbLen );
    wkb += curveWkbLen;
    delete[] curveWkb;
  }
  return geomPtr;
}

QString QgsCompoundCurveV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  Q_FOREACH ( const QgsCurveV2* curve, mCurves )
  {
    QString childWkt = curve->asWkt( precision );
    if ( dynamic_cast<const QgsLineStringV2*>( curve ) )
    {
      // Type names of linear geometries are omitted
      childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
    }
    wkt += childWkt + ',';
  }
  if ( wkt.endsWith( ',' ) )
  {
    wkt.chop( 1 );
  }
  wkt += ')';
  return wkt;
}

QDomElement QgsCompoundCurveV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QgsLineStringV2* line = curveToLine();
  QDomElement gml = line->asGML2( doc, precision, ns );
  delete line;
  return gml;
}

QDomElement QgsCompoundCurveV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemCurve = doc.createElementNS( ns, "Curve" );

  QDomElement elemSegments = doc.createElementNS( ns, "segments" );

  Q_FOREACH ( const QgsCurveV2* curve, mCurves )
  {
    if ( dynamic_cast<const QgsLineStringV2*>( curve ) )
    {
      QgsPointSequenceV2 pts;
      curve->points( pts );

      QDomElement elemLineStringSegment = doc.createElementNS( ns, "LineStringSegment" );
      elemLineStringSegment.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
      elemSegments.appendChild( elemLineStringSegment );
    }
    else if ( dynamic_cast<const QgsCircularStringV2*>( curve ) )
    {
      QgsPointSequenceV2 pts;
      curve->points( pts );

      QDomElement elemArcString = doc.createElementNS( ns, "ArcString" );
      elemArcString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
      elemSegments.appendChild( elemArcString );
    }
  }
  elemCurve.appendChild( elemSegments );
  return elemCurve;
}

QString QgsCompoundCurveV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QgsLineStringV2* line = curveToLine();
  QString json = line->asJSON( precision );
  delete line;
  return json;
}

double QgsCompoundCurveV2::length() const
{
  double length = 0;
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    length += ( *curveIt )->length();
  }
  return length;
}

QgsPointV2 QgsCompoundCurveV2::startPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( 0 )->startPoint();
}

QgsPointV2 QgsCompoundCurveV2::endPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( mCurves.size() - 1 )->endPoint();
}

void QgsCompoundCurveV2::points( QgsPointSequenceV2 &pts ) const
{
  pts.clear();
  if ( mCurves.size() < 1 )
  {
    return;
  }

  mCurves[0]->points( pts );
  for ( int i = 1; i < mCurves.size(); ++i )
  {
    QgsPointSequenceV2 pList;
    mCurves[i]->points( pList );
    pList.removeFirst(); //first vertex already added in previous line
    pts.append( pList );
  }
}

int QgsCompoundCurveV2::numPoints() const
{
  int nPoints = 0;
  int nCurves = mCurves.size();
  if ( nCurves < 1 )
  {
    return 0;
  }

  for ( int i = 0; i < nCurves; ++i )
  {
    nPoints += mCurves.at( i )->numPoints() - 1; //last vertex is equal to first of next section
  }
  nPoints += 1; //last vertex was removed above
  return nPoints;
}

QgsLineStringV2* QgsCompoundCurveV2::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  QgsLineStringV2* line = new QgsLineStringV2();
  QgsLineStringV2* currentLine = nullptr;
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    currentLine = ( *curveIt )->curveToLine( tolerance, toleranceType );
    line->append( currentLine );
    delete currentLine;
  }
  return line;
}

const QgsCurveV2* QgsCompoundCurveV2::curveAt( int i ) const
{
  if ( i >= mCurves.size() )
  {
    return nullptr;
  }
  return mCurves.at( i );
}

void QgsCompoundCurveV2::addCurve( QgsCurveV2* c )
{
  if ( c )
  {
    mCurves.append( c );

    if ( mWkbType == QgsWKBTypes::Unknown )
    {
      setZMTypeFromSubGeometry( c, QgsWKBTypes::CompoundCurve );
    }

    if ( QgsWKBTypes::hasZ( mWkbType ) && !QgsWKBTypes::hasZ( c->wkbType() ) )
    {
      c->addZValue();
    }
    if ( QgsWKBTypes::hasM( mWkbType ) && !QgsWKBTypes::hasM( c->wkbType() ) )
    {
      c->addMValue();
    }
    clearCache();
  }
}

void QgsCompoundCurveV2::removeCurve( int i )
{
  if ( mCurves.size() - 1  < i )
  {
    return;
  }

  delete( mCurves.at( i ) );
  mCurves.removeAt( i );
  clearCache();
}

void QgsCompoundCurveV2::addVertex( const QgsPointV2& pt )
{
  if ( mWkbType == QgsWKBTypes::Unknown )
  {
    setZMTypeFromSubGeometry( &pt, QgsWKBTypes::CompoundCurve );
  }

  //is last curve QgsLineStringV2
  QgsCurveV2* lastCurve = nullptr;
  if ( !mCurves.isEmpty() )
  {
    lastCurve = mCurves.at( mCurves.size() - 1 );
  }

  QgsLineStringV2* line = nullptr;
  if ( !lastCurve || QgsWKBTypes::flatType( lastCurve->wkbType() ) != QgsWKBTypes::LineString )
  {
    line = new QgsLineStringV2();
    mCurves.append( line );
    if ( lastCurve )
    {
      line->addVertex( lastCurve->endPoint() );
    }
    lastCurve = line;
  }
  else //create new QgsLineStringV2* with point in it
  {
    line = static_cast<QgsLineStringV2*>( lastCurve );
  }
  line->addVertex( pt );
  clearCache();
}

void QgsCompoundCurveV2::draw( QPainter& p ) const
{
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

void QgsCompoundCurveV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCompoundCurveV2::transform( const QTransform& t )
{
  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->transform( t );
  }
  clearCache();
}

void QgsCompoundCurveV2::addToPainterPath( QPainterPath& path ) const
{
  QPainterPath pp;
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  path.addPath( pp );
}

void QgsCompoundCurveV2::drawAsPolygon( QPainter& p ) const
{
  QPainterPath pp;
  QList< QgsCurveV2* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  p.drawPath( pp );
}

bool QgsCompoundCurveV2::insertVertex( QgsVertexId position, const QgsPointV2& vertex )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  if ( curveIds.size() < 1 )
  {
    return false;
  }
  int curveId = curveIds.at( 0 ).first;
  if ( curveId >= mCurves.size() )
  {
    return false;
  }

  bool success = mCurves.at( curveId )->insertVertex( curveIds.at( 0 ).second, vertex );
  if ( success )
  {
    clearCache(); //bbox changed
  }
  return success;
}

bool QgsCompoundCurveV2::moveVertex( QgsVertexId position, const QgsPointV2& newPos )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  QList< QPair<int, QgsVertexId> >::const_iterator idIt = curveIds.constBegin();
  for ( ; idIt != curveIds.constEnd(); ++idIt )
  {
    mCurves.at( idIt->first )->moveVertex( idIt->second, newPos );
  }

  bool success = !curveIds.isEmpty();
  if ( success )
  {
    clearCache(); //bbox changed
  }
  return success;
}

bool QgsCompoundCurveV2::deleteVertex( QgsVertexId position )
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  if ( curveIds.size() == 1 )
  {
    if ( !mCurves.at( curveIds.at( 0 ).first )->deleteVertex( curveIds.at( 0 ).second ) )
    {
      clearCache(); //bbox may have changed
      return false;
    }
    if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() == 0 )
    {
      removeCurve( curveIds.at( 0 ).first );
    }
  }
  else if ( curveIds.size() == 2 )
  {
    Q_ASSERT( curveIds.at( 1 ).first == curveIds.at( 0 ).first + 1 );
    Q_ASSERT( curveIds.at( 0 ).second.vertex == mCurves.at( curveIds.at( 0 ).first )->numPoints() - 1 );
    Q_ASSERT( curveIds.at( 1 ).second.vertex == 0 );
    QgsPointV2 startPoint = mCurves.at( curveIds.at( 0 ).first ) ->startPoint();
    QgsPointV2 endPoint = mCurves.at( curveIds.at( 1 ).first ) ->endPoint();
    if ( QgsWKBTypes::flatType( mCurves.at( curveIds.at( 0 ).first )->wkbType() ) == QgsWKBTypes::LineString &&
         QgsWKBTypes::flatType( mCurves.at( curveIds.at( 1 ).first )->wkbType() ) == QgsWKBTypes::CircularString &&
         mCurves.at( curveIds.at( 1 ).first )->numPoints() > 3 )
    {
      QgsPointV2 intermediatePoint;
      QgsVertexId::VertexType type;
      mCurves.at( curveIds.at( 1 ).first ) ->pointAt( 2, intermediatePoint, type );
      mCurves.at( curveIds.at( 0 ).first )->moveVertex(
        QgsVertexId( 0, 0, mCurves.at( curveIds.at( 0 ).first )->numPoints() - 1 ), intermediatePoint );
    }
    else if ( !mCurves.at( curveIds.at( 0 ).first )->deleteVertex( curveIds.at( 0 ).second ) )
    {
      clearCache(); //bbox may have changed
      return false;
    }
    if ( QgsWKBTypes::flatType( mCurves.at( curveIds.at( 0 ).first )->wkbType() ) == QgsWKBTypes::CircularString &&
         mCurves.at( curveIds.at( 0 ).first )->numPoints() > 0 &&
         QgsWKBTypes::flatType( mCurves.at( curveIds.at( 1 ).first )->wkbType() ) == QgsWKBTypes::LineString )
    {
      QgsPointV2 intermediatePoint = mCurves.at( curveIds.at( 0 ).first ) ->endPoint();
      mCurves.at( curveIds.at( 1 ).first )->moveVertex( QgsVertexId( 0, 0, 0 ), intermediatePoint );
    }
    else if ( !mCurves.at( curveIds.at( 1 ).first )->deleteVertex( curveIds.at( 1 ).second ) )
    {
      clearCache(); //bbox may have changed
      return false;
    }
    if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() == 0 &&
         mCurves.at( curveIds.at( 1 ).first )->numPoints() != 0 )
    {
      removeCurve( curveIds.at( 0 ).first );
      mCurves.at( curveIds.at( 1 ).first )->moveVertex( QgsVertexId( 0, 0, 0 ), startPoint );
    }
    else if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() != 0 &&
              mCurves.at( curveIds.at( 1 ).first )->numPoints() == 0 )
    {
      removeCurve( curveIds.at( 1 ).first );
      mCurves.at( curveIds.at( 0 ).first )->moveVertex(
        QgsVertexId( 0, 0, mCurves.at( curveIds.at( 0 ).first )->numPoints() - 1 ), endPoint );
    }
    else if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() == 0 &&
              mCurves.at( curveIds.at( 1 ).first )->numPoints() == 0 )
    {
      removeCurve( curveIds.at( 1 ).first );
      removeCurve( curveIds.at( 0 ).first );
      QgsLineStringV2* line = new QgsLineStringV2();
      line->insertVertex( QgsVertexId( 0, 0, 0 ), startPoint );
      line->insertVertex( QgsVertexId( 0, 0, 1 ), endPoint );
      mCurves.insert( curveIds.at( 0 ).first, line );
    }
    else
    {
      QgsPointV2 endPointOfFirst = mCurves.at( curveIds.at( 0 ).first ) ->endPoint();
      QgsPointV2 startPointOfSecond = mCurves.at( curveIds.at( 1 ).first ) ->startPoint();
      if ( endPointOfFirst != startPointOfSecond )
      {
        QgsLineStringV2* line = new QgsLineStringV2();
        line->insertVertex( QgsVertexId( 0, 0, 0 ), endPointOfFirst );
        line->insertVertex( QgsVertexId( 0, 0, 1 ), startPointOfSecond );
        mCurves.insert( curveIds.at( 1 ).first, line );
      }
    }
  }

  bool success = !curveIds.isEmpty();
  if ( success )
  {
    clearCache(); //bbox changed
  }
  return success;
}

QList< QPair<int, QgsVertexId> > QgsCompoundCurveV2::curveVertexId( QgsVertexId id ) const
{
  QList< QPair<int, QgsVertexId> > curveIds;

  int currentVertexIndex = 0;
  for ( int i = 0; i < mCurves.size(); ++i )
  {
    int increment = mCurves.at( i )->numPoints() - 1;
    if ( id.vertex >= currentVertexIndex && id.vertex <= currentVertexIndex + increment )
    {
      int curveVertexId = id.vertex - currentVertexIndex;
      QgsVertexId vid;
      vid.part = 0;
      vid.ring = 0;
      vid.vertex = curveVertexId;
      curveIds.append( qMakePair( i, vid ) );
      if ( curveVertexId == increment && i < ( mCurves.size() - 1 ) ) //add first vertex of next curve
      {
        vid.vertex = 0;
        curveIds.append( qMakePair( i + 1, vid ) );
      }
      break;
    }
    currentVertexIndex += increment;
  }

  return curveIds;
}

double QgsCompoundCurveV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mCurves, QgsGeometryUtils::VERTEX, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsCompoundCurveV2::pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const
{
  int currentVertexId = 0;
  for ( int j = 0; j < mCurves.size(); ++j )
  {
    int nCurvePoints = mCurves.at( j )->numPoints();
    if (( node - currentVertexId ) < nCurvePoints )
    {
      return ( mCurves.at( j )->pointAt( node - currentVertexId, point, type ) );
    }
    currentVertexId += ( nCurvePoints - 1 );
  }
  return false;
}

void QgsCompoundCurveV2::sumUpArea( double& sum ) const
{
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    ( *curveIt )->sumUpArea( sum );
  }
}

void QgsCompoundCurveV2::close()
{
  if ( numPoints() < 1 || isClosed() )
  {
    return;
  }
  addVertex( startPoint() );
}

bool QgsCompoundCurveV2::hasCurvedSegments() const
{
  QList< QgsCurveV2* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    if (( *curveIt )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

double QgsCompoundCurveV2::vertexAngle( QgsVertexId vertex ) const
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( vertex );
  if ( curveIds.size() == 1 )
  {
    QgsCurveV2* curve = mCurves[curveIds.at( 0 ).first];
    return curve->vertexAngle( curveIds.at( 0 ).second );
  }
  else if ( curveIds.size() > 1 )
  {
    QgsCurveV2* curve1 = mCurves[curveIds.at( 0 ).first];
    QgsCurveV2* curve2 = mCurves[curveIds.at( 1 ).first];
    double angle1 = curve1->vertexAngle( curveIds.at( 0 ).second );
    double angle2 = curve2->vertexAngle( curveIds.at( 1 ).second );
    return QgsGeometryUtils::averageAngle( angle1, angle2 );
  }
  else
  {
    return 0.0;
  }
}

QgsCompoundCurveV2* QgsCompoundCurveV2::reversed() const
{
  QgsCompoundCurveV2* clone = new QgsCompoundCurveV2();
  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    QgsCurveV2* reversedCurve = curve->reversed();
    clone->addCurve( reversedCurve );
  }
  return clone;
}

bool QgsCompoundCurveV2::addZValue( double zValue )
{
  if ( QgsWKBTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addZ( mWkbType );

  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsCompoundCurveV2::addMValue( double mValue )
{
  if ( QgsWKBTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addM( mWkbType );

  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsCompoundCurveV2::dropZValue()
{
  if ( !QgsWKBTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::dropZ( mWkbType );
  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsCompoundCurveV2::dropMValue()
{
  if ( !QgsWKBTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::dropM( mWkbType );
  Q_FOREACH ( QgsCurveV2* curve, mCurves )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}

