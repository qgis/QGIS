/***************************************************************************
                         qgscompoundcurve.cpp
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

#include "qgscompoundcurve.h"
#include "qgsapplication.h"
#include "qgscircularstring.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>


QgsCompoundCurve::QgsCompoundCurve(): QgsCurve()
{
  mWkbType = QgsWkbTypes::CompoundCurve;
}

QgsCompoundCurve::~QgsCompoundCurve()
{
  clear();
}

bool QgsCompoundCurve::operator==( const QgsCurve& other ) const
{
  const QgsCompoundCurve* otherCurve = dynamic_cast< const QgsCompoundCurve* >( &other );
  if ( !otherCurve )
    return false;

  return *otherCurve == *this;
}

bool QgsCompoundCurve::operator!=( const QgsCurve& other ) const
{
  return !operator==( other );
}

QgsCompoundCurve::QgsCompoundCurve( const QgsCompoundCurve& curve ): QgsCurve( curve )
{
  mWkbType = QgsWkbTypes::CompoundCurve;
  Q_FOREACH ( const QgsCurve* c, curve.mCurves )
  {
    mCurves.append( static_cast<QgsCurve*>( c->clone() ) );
  }
}

QgsCompoundCurve& QgsCompoundCurve::operator=( const QgsCompoundCurve & curve )
{
  if ( &curve != this )
  {
    clearCache();
    QgsCurve::operator=( curve );
    Q_FOREACH ( const QgsCurve* c, curve.mCurves )
    {
      mCurves.append( static_cast<QgsCurve*>( c->clone() ) );
    }
  }
  return *this;
}

QgsCompoundCurve *QgsCompoundCurve::clone() const
{
  return new QgsCompoundCurve( *this );
}

void QgsCompoundCurve::clear()
{
  mWkbType = QgsWkbTypes::CompoundCurve;
  qDeleteAll( mCurves );
  mCurves.clear();
  clearCache();
}

QgsRectangle QgsCompoundCurve::calculateBoundingBox() const
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

bool QgsCompoundCurve::fromWkb( QgsConstWkbPtr wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::CompoundCurve )
  {
    return false;
  }
  mWkbType = type;

  int nCurves;
  wkbPtr >> nCurves;
  QgsCurve* currentCurve = nullptr;
  int currentCurveSize = 0;
  for ( int i = 0; i < nCurves; ++i )
  {
    QgsWkbTypes::Type curveType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    if ( QgsWkbTypes::flatType( curveType ) == QgsWkbTypes::LineString )
    {
      currentCurve = new QgsLineString();
    }
    else if ( QgsWkbTypes::flatType( curveType ) == QgsWkbTypes::CircularString )
    {
      currentCurve = new QgsCircularString();
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

bool QgsCompoundCurve::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::CompoundCurve )
    return false;
  mWkbType = parts.first;

  QString defaultChildWkbType = QString( "LineString%1%2" ).arg( is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    if ( QgsWkbTypes::flatType( childParts.first ) == QgsWkbTypes::LineString )
      mCurves.append( new QgsLineString() );
    else if ( QgsWkbTypes::flatType( childParts.first ) == QgsWkbTypes::CircularString )
      mCurves.append( new QgsCircularString() );
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
  Q_FOREACH ( const QgsCurve* curve, mCurves )
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

int QgsCompoundCurve::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  Q_FOREACH ( const QgsCurve *curve, mCurves )
  {
    size += curve->wkbSize();
  }
  return size;
}

unsigned char* QgsCompoundCurve::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr, binarySize );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mCurves.size() );
  Q_FOREACH ( const QgsCurve* curve, mCurves )
  {
    int curveWkbLen = 0;
    unsigned char* curveWkb = curve->asWkb( curveWkbLen );
    memcpy( wkb, curveWkb, curveWkbLen );
    wkb += curveWkbLen;
    delete[] curveWkb;
  }
  return geomPtr;
}

QString QgsCompoundCurve::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  Q_FOREACH ( const QgsCurve* curve, mCurves )
  {
    QString childWkt = curve->asWkt( precision );
    if ( dynamic_cast<const QgsLineString*>( curve ) )
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

QDomElement QgsCompoundCurve::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QgsLineString* line = curveToLine();
  QDomElement gml = line->asGML2( doc, precision, ns );
  delete line;
  return gml;
}

QDomElement QgsCompoundCurve::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemCurve = doc.createElementNS( ns, "Curve" );

  QDomElement elemSegments = doc.createElementNS( ns, "segments" );

  Q_FOREACH ( const QgsCurve* curve, mCurves )
  {
    if ( dynamic_cast<const QgsLineString*>( curve ) )
    {
      QgsPointSequence pts;
      curve->points( pts );

      QDomElement elemLineStringSegment = doc.createElementNS( ns, "LineStringSegment" );
      elemLineStringSegment.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
      elemSegments.appendChild( elemLineStringSegment );
    }
    else if ( dynamic_cast<const QgsCircularString*>( curve ) )
    {
      QgsPointSequence pts;
      curve->points( pts );

      QDomElement elemArcString = doc.createElementNS( ns, "ArcString" );
      elemArcString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
      elemSegments.appendChild( elemArcString );
    }
  }
  elemCurve.appendChild( elemSegments );
  return elemCurve;
}

QString QgsCompoundCurve::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QgsLineString* line = curveToLine();
  QString json = line->asJSON( precision );
  delete line;
  return json;
}

double QgsCompoundCurve::length() const
{
  double length = 0;
  QList< QgsCurve* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    length += ( *curveIt )->length();
  }
  return length;
}

QgsPointV2 QgsCompoundCurve::startPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( 0 )->startPoint();
}

QgsPointV2 QgsCompoundCurve::endPoint() const
{
  if ( mCurves.size() < 1 )
  {
    return QgsPointV2();
  }
  return mCurves.at( mCurves.size() - 1 )->endPoint();
}

void QgsCompoundCurve::points( QgsPointSequence &pts ) const
{
  pts.clear();
  if ( mCurves.size() < 1 )
  {
    return;
  }

  mCurves[0]->points( pts );
  for ( int i = 1; i < mCurves.size(); ++i )
  {
    QgsPointSequence pList;
    mCurves[i]->points( pList );
    pList.removeFirst(); //first vertex already added in previous line
    pts.append( pList );
  }
}

int QgsCompoundCurve::numPoints() const
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

QgsLineString* QgsCompoundCurve::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QList< QgsCurve* >::const_iterator curveIt = mCurves.constBegin();
  QgsLineString* line = new QgsLineString();
  QgsLineString* currentLine = nullptr;
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    currentLine = ( *curveIt )->curveToLine( tolerance, toleranceType );
    line->append( currentLine );
    delete currentLine;
  }
  return line;
}

const QgsCurve* QgsCompoundCurve::curveAt( int i ) const
{
  if ( i >= mCurves.size() )
  {
    return nullptr;
  }
  return mCurves.at( i );
}

void QgsCompoundCurve::addCurve( QgsCurve* c )
{
  if ( c )
  {
    mCurves.append( c );

    if ( mWkbType == QgsWkbTypes::Unknown )
    {
      setZMTypeFromSubGeometry( c, QgsWkbTypes::CompoundCurve );
    }

    if ( QgsWkbTypes::hasZ( mWkbType ) && !QgsWkbTypes::hasZ( c->wkbType() ) )
    {
      c->addZValue();
    }
    if ( QgsWkbTypes::hasM( mWkbType ) && !QgsWkbTypes::hasM( c->wkbType() ) )
    {
      c->addMValue();
    }
    clearCache();
  }
}

void QgsCompoundCurve::removeCurve( int i )
{
  if ( mCurves.size() - 1  < i )
  {
    return;
  }

  delete( mCurves.at( i ) );
  mCurves.removeAt( i );
  clearCache();
}

void QgsCompoundCurve::addVertex( const QgsPointV2& pt )
{
  if ( mWkbType == QgsWkbTypes::Unknown )
  {
    setZMTypeFromSubGeometry( &pt, QgsWkbTypes::CompoundCurve );
  }

  //is last curve QgsLineString
  QgsCurve* lastCurve = nullptr;
  if ( !mCurves.isEmpty() )
  {
    lastCurve = mCurves.at( mCurves.size() - 1 );
  }

  QgsLineString* line = nullptr;
  if ( !lastCurve || QgsWkbTypes::flatType( lastCurve->wkbType() ) != QgsWkbTypes::LineString )
  {
    line = new QgsLineString();
    mCurves.append( line );
    if ( lastCurve )
    {
      line->addVertex( lastCurve->endPoint() );
    }
    lastCurve = line;
  }
  else //create new QgsLineString* with point in it
  {
    line = static_cast<QgsLineString*>( lastCurve );
  }
  line->addVertex( pt );
  clearCache();
}

void QgsCompoundCurve::draw( QPainter& p ) const
{
  QList< QgsCurve* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

void QgsCompoundCurve::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCompoundCurve::transform( const QTransform& t )
{
  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->transform( t );
  }
  clearCache();
}

void QgsCompoundCurve::addToPainterPath( QPainterPath& path ) const
{
  QPainterPath pp;
  QList< QgsCurve* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  path.addPath( pp );
}

void QgsCompoundCurve::drawAsPolygon( QPainter& p ) const
{
  QPainterPath pp;
  QList< QgsCurve* >::const_iterator it = mCurves.constBegin();
  for ( ; it != mCurves.constEnd(); ++it )
  {
    ( *it )->addToPainterPath( pp );
  }
  p.drawPath( pp );
}

bool QgsCompoundCurve::insertVertex( QgsVertexId position, const QgsPointV2& vertex )
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

bool QgsCompoundCurve::moveVertex( QgsVertexId position, const QgsPointV2& newPos )
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

bool QgsCompoundCurve::deleteVertex( QgsVertexId position )
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
    if ( QgsWkbTypes::flatType( mCurves.at( curveIds.at( 0 ).first )->wkbType() ) == QgsWkbTypes::LineString &&
         QgsWkbTypes::flatType( mCurves.at( curveIds.at( 1 ).first )->wkbType() ) == QgsWkbTypes::CircularString &&
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
    if ( QgsWkbTypes::flatType( mCurves.at( curveIds.at( 0 ).first )->wkbType() ) == QgsWkbTypes::CircularString &&
         mCurves.at( curveIds.at( 0 ).first )->numPoints() > 0 &&
         QgsWkbTypes::flatType( mCurves.at( curveIds.at( 1 ).first )->wkbType() ) == QgsWkbTypes::LineString )
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
      QgsLineString* line = new QgsLineString();
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
        QgsLineString* line = new QgsLineString();
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

QList< QPair<int, QgsVertexId> > QgsCompoundCurve::curveVertexId( QgsVertexId id ) const
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

double QgsCompoundCurve::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mCurves, QgsGeometryUtils::VERTEX, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsCompoundCurve::pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const
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

double QgsCompoundCurve::xAt( int index ) const
{
  int currentVertexId = 0;
  for ( int j = 0; j < mCurves.size(); ++j )
  {
    int nCurvePoints = mCurves.at( j )->numPoints();
    if (( index - currentVertexId ) < nCurvePoints )
    {
      return mCurves.at( j )->xAt( index - currentVertexId );
    }
    currentVertexId += ( nCurvePoints - 1 );
  }
  return 0.0;
}

double QgsCompoundCurve::yAt( int index ) const
{
  int currentVertexId = 0;
  for ( int j = 0; j < mCurves.size(); ++j )
  {
    int nCurvePoints = mCurves.at( j )->numPoints();
    if (( index - currentVertexId ) < nCurvePoints )
    {
      return mCurves.at( j )->yAt( index - currentVertexId );
    }
    currentVertexId += ( nCurvePoints - 1 );
  }
  return 0.0;
}

void QgsCompoundCurve::sumUpArea( double& sum ) const
{
  QList< QgsCurve* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    ( *curveIt )->sumUpArea( sum );
  }
}

void QgsCompoundCurve::close()
{
  if ( numPoints() < 1 || isClosed() )
  {
    return;
  }
  addVertex( startPoint() );
}

bool QgsCompoundCurve::hasCurvedSegments() const
{
  QList< QgsCurve* >::const_iterator curveIt = mCurves.constBegin();
  for ( ; curveIt != mCurves.constEnd(); ++curveIt )
  {
    if (( *curveIt )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

double QgsCompoundCurve::vertexAngle( QgsVertexId vertex ) const
{
  QList< QPair<int, QgsVertexId> > curveIds = curveVertexId( vertex );
  if ( curveIds.size() == 1 )
  {
    QgsCurve* curve = mCurves[curveIds.at( 0 ).first];
    return curve->vertexAngle( curveIds.at( 0 ).second );
  }
  else if ( curveIds.size() > 1 )
  {
    QgsCurve* curve1 = mCurves[curveIds.at( 0 ).first];
    QgsCurve* curve2 = mCurves[curveIds.at( 1 ).first];
    double angle1 = curve1->vertexAngle( curveIds.at( 0 ).second );
    double angle2 = curve2->vertexAngle( curveIds.at( 1 ).second );
    return QgsGeometryUtils::averageAngle( angle1, angle2 );
  }
  else
  {
    return 0.0;
  }
}

QgsCompoundCurve* QgsCompoundCurve::reversed() const
{
  QgsCompoundCurve* clone = new QgsCompoundCurve();
  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    QgsCurve* reversedCurve = curve->reversed();
    clone->addCurve( reversedCurve );
  }
  return clone;
}

bool QgsCompoundCurve::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsCompoundCurve::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addM( mWkbType );

  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsCompoundCurve::dropZValue()
{
  if ( !QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsCompoundCurve::dropMValue()
{
  if ( !QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  Q_FOREACH ( QgsCurve* curve, mCurves )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}

