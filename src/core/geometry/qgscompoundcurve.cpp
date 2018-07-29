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
#include <memory>

QgsCompoundCurve::QgsCompoundCurve()
{
  mWkbType = QgsWkbTypes::CompoundCurve;
}

QgsCompoundCurve::~QgsCompoundCurve()
{
  clear();
}

bool QgsCompoundCurve::equals( const QgsCurve &other ) const
{
  const QgsCompoundCurve *otherCurve = qgsgeometry_cast< const QgsCompoundCurve * >( &other );
  if ( !otherCurve )
    return false;

  if ( mWkbType != otherCurve->mWkbType )
    return false;

  if ( mCurves.size() != otherCurve->mCurves.size() )
    return false;

  for ( int i = 0; i < mCurves.size(); ++i )
  {
    if ( *mCurves.at( i ) != *otherCurve->mCurves.at( i ) )
      return false;
  }

  return true;
}

QgsCompoundCurve *QgsCompoundCurve::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsCompoundCurve >();
  result->mWkbType = mWkbType;
  return result.release();
}

QString QgsCompoundCurve::geometryType() const
{
  return QStringLiteral( "CompoundCurve" );
}

int QgsCompoundCurve::dimension() const
{
  return 1;
}

QgsCompoundCurve::QgsCompoundCurve( const QgsCompoundCurve &curve ): QgsCurve( curve )
{
  mWkbType = curve.wkbType();
  for ( const QgsCurve *c : curve.mCurves )
  {
    mCurves.append( static_cast<QgsCurve *>( c->clone() ) );
  }
}

QgsCompoundCurve &QgsCompoundCurve::operator=( const QgsCompoundCurve &curve )
{
  if ( &curve != this )
  {
    clearCache();
    QgsCurve::operator=( curve );
    for ( const QgsCurve *c : curve.mCurves )
    {
      mCurves.append( static_cast<QgsCurve *>( c->clone() ) );
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
  if ( mCurves.empty() )
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

bool QgsCompoundCurve::fromWkb( QgsConstWkbPtr &wkbPtr )
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
  QgsCurve *currentCurve = nullptr;
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
    currentCurve->fromWkb( wkbPtr );  // also updates wkbPtr
    mCurves.append( currentCurve );
  }
  return true;
}

bool QgsCompoundCurve::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::CompoundCurve )
    return false;
  mWkbType = parts.first;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType );
  for ( const QString &childWkt : blocks )
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
  for ( const QgsCurve *curve : qgis::as_const( mCurves ) )
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

QByteArray QgsCompoundCurve::asWkb() const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  QVector<QByteArray> wkbForCurves;
  for ( const QgsCurve *curve : mCurves )
  {
    QByteArray wkbForCurve = curve->asWkb();
    binarySize += wkbForCurve.length();
    wkbForCurves << wkbForCurve;
  }

  QByteArray wkbArray;
  wkbArray.resize( binarySize );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mCurves.size() );
  for ( const QByteArray &wkbForCurve : qgis::as_const( wkbForCurves ) )
  {
    wkb << wkbForCurve;
  }
  return wkbArray;
}

QString QgsCompoundCurve::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  for ( const QgsCurve *curve : mCurves )
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
    wkt.chop( 1 );
  }
  wkt += ')';
  return wkt;
}

QDomElement QgsCompoundCurve::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  // GML2 does not support curves
  std::unique_ptr< QgsLineString > line( curveToLine() );
  QDomElement gml = line->asGml2( doc, precision, ns, axisOrder );
  return gml;
}

QDomElement QgsCompoundCurve::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement compoundCurveElem = doc.createElementNS( ns, QStringLiteral( "CompositeCurve" ) );

  if ( isEmpty() )
    return compoundCurveElem;

  for ( const QgsCurve *curve : mCurves )
  {
    QDomElement curveMemberElem = doc.createElementNS( ns, QStringLiteral( "curveMember" ) );
    QDomElement curveElem = curve->asGml3( doc, precision, ns, axisOrder );
    curveMemberElem.appendChild( curveElem );
    compoundCurveElem.appendChild( curveMemberElem );
  }

  return compoundCurveElem;
}

QString QgsCompoundCurve::asJson( int precision ) const
{
  // GeoJSON does not support curves
  std::unique_ptr< QgsLineString > line( curveToLine() );
  QString json = line->asJson( precision );
  return json;
}

double QgsCompoundCurve::length() const
{
  double length = 0;
  for ( const QgsCurve *curve : mCurves )
  {
    length += curve->length();
  }
  return length;
}

QgsPoint QgsCompoundCurve::startPoint() const
{
  if ( mCurves.empty() )
  {
    return QgsPoint();
  }
  return mCurves.at( 0 )->startPoint();
}

QgsPoint QgsCompoundCurve::endPoint() const
{
  if ( mCurves.empty() )
  {
    return QgsPoint();
  }
  return mCurves.at( mCurves.size() - 1 )->endPoint();
}

void QgsCompoundCurve::points( QgsPointSequence &pts ) const
{
  pts.clear();
  if ( mCurves.empty() )
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

bool QgsCompoundCurve::isEmpty() const
{
  if ( mCurves.isEmpty() )
    return true;

  for ( QgsCurve *curve : mCurves )
  {
    if ( !curve->isEmpty() )
      return false;
  }
  return true;
}

QgsLineString *QgsCompoundCurve::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QgsLineString *line = new QgsLineString();
  std::unique_ptr< QgsLineString > currentLine;
  for ( const QgsCurve *curve : mCurves )
  {
    currentLine.reset( curve->curveToLine( tolerance, toleranceType ) );
    line->append( currentLine.get() );
  }
  return line;
}

QgsCompoundCurve *QgsCompoundCurve::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  std::unique_ptr<QgsCompoundCurve> result( createEmptyWithSameType() );

  for ( QgsCurve *curve : mCurves )
  {
    std::unique_ptr<QgsCurve> gridified( static_cast< QgsCurve * >( curve->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing ) ) );
    if ( gridified )
    {
      result->mCurves.append( gridified.release() );
    }
  }

  if ( result->mCurves.empty() )
    return nullptr;
  else
    return result.release();
}

bool QgsCompoundCurve::removeDuplicateNodes( double epsilon, bool useZValues )
{
  bool result = false;
  const QVector< QgsCurve * > curves = mCurves;
  int i = 0;
  QgsPoint lastEnd;
  for ( QgsCurve *curve : curves )
  {
    result = result || curve->removeDuplicateNodes( epsilon, useZValues );
    if ( curve->numPoints() == 0 || qgsDoubleNear( curve->length(), 0.0, epsilon ) )
    {
      // empty curve, remove it
      delete mCurves.takeAt( i );
      result = true;
    }
    else
    {
      // ensure this line starts exactly where previous line ended
      if ( i > 0 )
      {
        curve->moveVertex( QgsVertexId( -1, -1, 0 ), lastEnd );
      }
      lastEnd = curve->vertexAt( QgsVertexId( -1, -1, curve->numPoints() - 1 ) );
    }
    i++;
  }
  return result;
}

const QgsCurve *QgsCompoundCurve::curveAt( int i ) const
{
  if ( i < 0 || i >= mCurves.size() )
  {
    return nullptr;
  }
  return mCurves.at( i );
}

void QgsCompoundCurve::addCurve( QgsCurve *c )
{
  if ( c )
  {
    if ( mCurves.empty() )
    {
      setZMTypeFromSubGeometry( c, QgsWkbTypes::CompoundCurve );
    }

    mCurves.append( c );

    if ( QgsWkbTypes::hasZ( mWkbType ) && !QgsWkbTypes::hasZ( c->wkbType() ) )
    {
      c->addZValue();
    }
    else if ( !QgsWkbTypes::hasZ( mWkbType ) && QgsWkbTypes::hasZ( c->wkbType() ) )
    {
      c->dropZValue();
    }
    if ( QgsWkbTypes::hasM( mWkbType ) && !QgsWkbTypes::hasM( c->wkbType() ) )
    {
      c->addMValue();
    }
    else if ( !QgsWkbTypes::hasM( mWkbType ) && QgsWkbTypes::hasM( c->wkbType() ) )
    {
      c->dropMValue();
    }
    clearCache();
  }
}

void QgsCompoundCurve::removeCurve( int i )
{
  if ( i < 0 || i >= mCurves.size() )
  {
    return;
  }

  delete mCurves.takeAt( i );
  clearCache();
}

void QgsCompoundCurve::addVertex( const QgsPoint &pt )
{
  if ( mCurves.isEmpty() || mWkbType == QgsWkbTypes::Unknown )
  {
    setZMTypeFromSubGeometry( &pt, QgsWkbTypes::CompoundCurve );
  }

  //is last curve QgsLineString
  QgsCurve *lastCurve = nullptr;
  if ( !mCurves.isEmpty() )
  {
    lastCurve = mCurves.at( mCurves.size() - 1 );
  }

  QgsLineString *line = nullptr;
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
    line = static_cast<QgsLineString *>( lastCurve );
  }
  line->addVertex( pt );
  clearCache();
}

void QgsCompoundCurve::draw( QPainter &p ) const
{
  for ( const QgsCurve *curve : mCurves )
  {
    curve->draw( p );
  }
}

void QgsCompoundCurve::transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCompoundCurve::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->transform( t, zTranslate, zScale, mTranslate, mScale );
  }
  clearCache();
}

void QgsCompoundCurve::addToPainterPath( QPainterPath &path ) const
{
  QPainterPath pp;
  for ( const QgsCurve *curve : mCurves )
  {
    curve->addToPainterPath( pp );
  }
  path.addPath( pp );
}

void QgsCompoundCurve::drawAsPolygon( QPainter &p ) const
{
  QPainterPath pp;
  for ( const QgsCurve *curve : mCurves )
  {
    curve->addToPainterPath( pp );
  }
  p.drawPath( pp );
}

bool QgsCompoundCurve::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  QVector< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  if ( curveIds.empty() )
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

bool QgsCompoundCurve::moveVertex( QgsVertexId position, const QgsPoint &newPos )
{
  QVector< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
  QVector< QPair<int, QgsVertexId> >::const_iterator idIt = curveIds.constBegin();
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
  QVector< QPair<int, QgsVertexId> > curveIds = curveVertexId( position );
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
    QgsPoint startPoint = mCurves.at( curveIds.at( 0 ).first ) ->startPoint();
    QgsPoint endPoint = mCurves.at( curveIds.at( 1 ).first ) ->endPoint();
    if ( QgsWkbTypes::flatType( mCurves.at( curveIds.at( 0 ).first )->wkbType() ) == QgsWkbTypes::LineString &&
         QgsWkbTypes::flatType( mCurves.at( curveIds.at( 1 ).first )->wkbType() ) == QgsWkbTypes::CircularString &&
         mCurves.at( curveIds.at( 1 ).first )->numPoints() > 3 )
    {
      QgsPoint intermediatePoint;
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
      QgsPoint intermediatePoint = mCurves.at( curveIds.at( 0 ).first ) ->endPoint();
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
      mCurves.at( curveIds.at( 1 ).first )->moveVertex( QgsVertexId( 0, 0, 0 ), startPoint );
      removeCurve( curveIds.at( 0 ).first );
    }
    else if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() != 0 &&
              mCurves.at( curveIds.at( 1 ).first )->numPoints() == 0 )
    {
      mCurves.at( curveIds.at( 0 ).first )->moveVertex(
        QgsVertexId( 0, 0, mCurves.at( curveIds.at( 0 ).first )->numPoints() - 1 ), endPoint );
      removeCurve( curveIds.at( 1 ).first );
    }
    else if ( mCurves.at( curveIds.at( 0 ).first )->numPoints() == 0 &&
              mCurves.at( curveIds.at( 1 ).first )->numPoints() == 0 )
    {
      removeCurve( curveIds.at( 1 ).first );
      removeCurve( curveIds.at( 0 ).first );
      QgsLineString *line = new QgsLineString();
      line->insertVertex( QgsVertexId( 0, 0, 0 ), startPoint );
      line->insertVertex( QgsVertexId( 0, 0, 1 ), endPoint );
      mCurves.insert( curveIds.at( 0 ).first, line );
    }
    else
    {
      QgsPoint endPointOfFirst = mCurves.at( curveIds.at( 0 ).first ) ->endPoint();
      QgsPoint startPointOfSecond = mCurves.at( curveIds.at( 1 ).first ) ->startPoint();
      if ( endPointOfFirst != startPointOfSecond )
      {
        QgsLineString *line = new QgsLineString();
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

QVector< QPair<int, QgsVertexId> > QgsCompoundCurve::curveVertexId( QgsVertexId id ) const
{
  QVector< QPair<int, QgsVertexId> > curveIds;

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

double QgsCompoundCurve::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mCurves, QgsGeometryUtils::Vertex, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsCompoundCurve::pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const
{
  int currentVertexId = 0;
  for ( int j = 0; j < mCurves.size(); ++j )
  {
    int nCurvePoints = mCurves.at( j )->numPoints();
    if ( ( node - currentVertexId ) < nCurvePoints )
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
    if ( ( index - currentVertexId ) < nCurvePoints )
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
    if ( ( index - currentVertexId ) < nCurvePoints )
    {
      return mCurves.at( j )->yAt( index - currentVertexId );
    }
    currentVertexId += ( nCurvePoints - 1 );
  }
  return 0.0;
}

void QgsCompoundCurve::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->filterVertices( filter );
  }
  clearCache();
}

void QgsCompoundCurve::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->transformVertices( transform );
  }
  clearCache();
}

void QgsCompoundCurve::sumUpArea( double &sum ) const
{
  for ( const QgsCurve *curve : mCurves )
  {
    curve->sumUpArea( sum );
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
  for ( const QgsCurve *curve : mCurves )
  {
    if ( curve->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

double QgsCompoundCurve::vertexAngle( QgsVertexId vertex ) const
{
  QVector< QPair<int, QgsVertexId> > curveIds = curveVertexId( vertex );
  if ( curveIds.size() == 1 )
  {
    QgsCurve *curve = mCurves[curveIds.at( 0 ).first];
    return curve->vertexAngle( curveIds.at( 0 ).second );
  }
  else if ( curveIds.size() > 1 )
  {
    QgsCurve *curve1 = mCurves[curveIds.at( 0 ).first];
    QgsCurve *curve2 = mCurves[curveIds.at( 1 ).first];
    double angle1 = curve1->vertexAngle( curveIds.at( 0 ).second );
    double angle2 = curve2->vertexAngle( curveIds.at( 1 ).second );
    return QgsGeometryUtils::averageAngle( angle1, angle2 );
  }
  else
  {
    return 0.0;
  }
}

double QgsCompoundCurve::segmentLength( QgsVertexId startVertex ) const
{
  QVector< QPair<int, QgsVertexId> > curveIds = curveVertexId( startVertex );
  double length = 0.0;
  for ( auto it = curveIds.constBegin(); it != curveIds.constEnd(); ++it )
  {
    length += mCurves.at( it->first )->segmentLength( it->second );
  }
  return length;
}

QgsCompoundCurve *QgsCompoundCurve::reversed() const
{
  QgsCompoundCurve *clone = new QgsCompoundCurve();
  for ( int i = mCurves.count() - 1; i >= 0; --i )
  {
    QgsCurve *reversedCurve = mCurves.at( i )->reversed();
    clone->addCurve( reversedCurve );
  }
  return clone;
}

bool QgsCompoundCurve::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
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

  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
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
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
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
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}

void QgsCompoundCurve::swapXy()
{
  for ( QgsCurve *curve : qgis::as_const( mCurves ) )
  {
    curve->swapXy();
  }
  clearCache();
}

