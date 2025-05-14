/***************************************************************************
                         qgssfcgalGeometry.cpp
                         ----------------
    begin                : September 2024
    copyright            : (C) 2024 by Benoit De Mezzo
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssfcgalgeometry.h"
#include "qgsvector3d.h"
#include "qgswkbptr.h"


QgsSfcgalGeometry::QgsSfcgalGeometry()
  : QgsAbstractGeometry()
{
  mWkbType = Qgis::WkbType::NoGeometry;
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsGeometry &qgsGeom, sfcgal::shared_geom sfcgalGeom )
  : QgsAbstractGeometry()
{
  mQgsGeom.reset( qgsGeom.constGet()->clone() );
  mWkbType = mQgsGeom->wkbType();

  if ( sfcgalGeom )
  {
    mSfcgalGeom = sfcgalGeom;
  }
  else
  {
    mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( mQgsGeom.get() );
  }

  if ( mWkbType == Qgis::WkbType::Unknown )
    mWkbType = QgsSfcgalEngine::wkbType( mSfcgalGeom.get() );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( std::unique_ptr<QgsAbstractGeometry> &qgsGeom, sfcgal::shared_geom sfcgalGeom )
  : QgsAbstractGeometry()
{
  if ( qgsGeom && qgsGeom.get() )
  {
    mQgsGeom = std::move( qgsGeom );
    mWkbType = mQgsGeom->wkbType();
  }

  if ( sfcgalGeom )
  {
    mSfcgalGeom = sfcgalGeom;
  }
  else
  {
    mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( mQgsGeom.get() );
  }

  if ( mWkbType == Qgis::WkbType::Unknown )
    mWkbType = QgsSfcgalEngine::wkbType( mSfcgalGeom.get() );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsSfcgalGeometry &geom )
  : QgsAbstractGeometry( geom )
{
  mSfcgalGeom = QgsSfcgalEngine::cloneGeometry( geom.mSfcgalGeom.get() );
  resetQgsGeometry();
}

void QgsSfcgalGeometry::resetQgsGeometry()
{
  if ( mSfcgalGeom )
  {
    mQgsGeom = QgsSfcgalEngine::toAbstractGeometry( mSfcgalGeom.get() );
    mWkbType = mQgsGeom->wkbType();
  }
}

QString QgsSfcgalGeometry::geometryType() const
{
  QString errorMsg; // used to retrieve failure messages if any

  QString out = QgsSfcgalEngine::geometryType( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, QString() );

  return out;
}

QgsAbstractGeometry *QgsSfcgalGeometry::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsSfcgalGeometry >();
  result->mWkbType = mWkbType;
  return result.release();
}

QgsAbstractGeometry *QgsSfcgalGeometry::clone() const
{
  return new QgsSfcgalGeometry( *this );
}

void QgsSfcgalGeometry::clear()
{
  mQgsGeom.reset();
  mSfcgalGeom.reset();
  mWkbType = Qgis::WkbType::NoGeometry;
  clearCache();
}

bool QgsSfcgalGeometry::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
    return false;

  clear();

  QString errorMsg; // used to retrieve failure messages if any
  mSfcgalGeom = QgsSfcgalEngine::fromWkb( wkbPtr, &errorMsg );
  resetQgsGeometry();
  CHECK_SUCCESS_LOG( &errorMsg, false );

  return true;
}

int QgsSfcgalGeometry::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  QByteArray array = asWkb( flags );
  return array.length();
}

QByteArray QgsSfcgalGeometry::asWkb( QgsAbstractGeometry::WkbFlags ) const
{
  QgsConstWkbPtr ptr = QgsSfcgalEngine::toWkb( mSfcgalGeom.get() );
  const unsigned char *wkbUnsignedPtr = ptr;
  return QByteArray( reinterpret_cast<const char *>( wkbUnsignedPtr ), ptr.remaining() );
}

QString QgsSfcgalGeometry::asWkt( int numDecim ) const
{
  QString errorMsg; // used to retrieve failure messages if any

  QString out = QgsSfcgalEngine::toWkt( mSfcgalGeom.get(), numDecim, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, QString() );

  return out;
}

QgsAbstractGeometry *QgsSfcgalGeometry::boundary() const
{
  sfcgal::shared_geom boundary = QgsSfcgalEngine::boundary( mSfcgalGeom.get() );
  return QgsSfcgalEngine::toSfcgalGeometry( boundary ).release();
}

bool QgsSfcgalGeometry::operator==( const QgsAbstractGeometry &other ) const
{
  const QgsSfcgalGeometry *otherGeom = dynamic_cast<const QgsSfcgalGeometry *>( &other );
  bool out = false;
  if ( otherGeom )
  {
    QString errorMsg; // used to retrieve failure messages if any
    out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), otherGeom->mSfcgalGeom.get(), 0.0, &errorMsg );
    CHECK_SUCCESS_LOG( &errorMsg, false );
  }
  return out;
}

bool QgsSfcgalGeometry::operator!=( const QgsAbstractGeometry &other ) const
{
  return !( *this == other );
}

bool QgsSfcgalGeometry::fuzzyEqual( const QgsAbstractGeometry &other, double epsilon ) const
{
  const QgsSfcgalGeometry *otherGeom = dynamic_cast<const QgsSfcgalGeometry *>( &other );
  bool out = false;
  if ( otherGeom )
  {
    QString errorMsg; // used to retrieve failure messages if any
    out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), otherGeom->mSfcgalGeom.get(), epsilon, &errorMsg );
    CHECK_SUCCESS_LOG( &errorMsg, false );
  }
  return out;
}

bool QgsSfcgalGeometry::fuzzyDistanceEqual( const QgsAbstractGeometry &other, double epsilon ) const
{
  return fuzzyEqual( other, epsilon );
}

QgsBox3D QgsSfcgalGeometry::boundingBox3D() const
{
  return mQgsGeom->boundingBox3D();
}

int QgsSfcgalGeometry::dimension() const
{
  QString errorMsg; // used to retrieve failure messages if any
  int result = QgsSfcgalEngine::dimension( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, -1 );

  return result;
}

void QgsSfcgalGeometry::normalize()
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
}

bool QgsSfcgalGeometry::fromWkt( const QString &wkt )
{
  clear();

  QString errorMsg; // used to retrieve failure messages if any
  mSfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );
  resetQgsGeometry();

  return true;
}

QDomElement QgsSfcgalGeometry::asGml2( QDomDocument &doc, int precision, const QString &ns, AxisOrder axisOrder ) const
{
  return mQgsGeom->asGml2( doc, precision, ns, axisOrder );
}

QDomElement QgsSfcgalGeometry::asGml3( QDomDocument &doc, int precision, const QString &ns, AxisOrder axisOrder ) const
{
  return mQgsGeom->asGml3( doc, precision, ns, axisOrder );
}

QString QgsSfcgalGeometry::asKml( int precision ) const
{
  return mQgsGeom->asKml( precision );
}

void QgsSfcgalGeometry::transform( const QgsCoordinateTransform &, Qgis::TransformDirection, bool )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
}

void QgsSfcgalGeometry::transform( const QTransform &, double, double, double, double )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
}

void QgsSfcgalGeometry::draw( QPainter &p ) const
{
  mQgsGeom->draw( p );
}

QPainterPath QgsSfcgalGeometry::asQPainterPath() const
{
  return mQgsGeom->asQPainterPath();
}

int QgsSfcgalGeometry::vertexNumberFromVertexId( QgsVertexId id ) const
{
  return mQgsGeom->vertexNumberFromVertexId( id );
}

bool QgsSfcgalGeometry::nextVertex( QgsVertexId &id, QgsPoint &vertex ) const
{
  return mQgsGeom->nextVertex( id, vertex );
}

void QgsSfcgalGeometry::adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  mQgsGeom->adjacentVertices( vertex, previousVertex, nextVertex );
}

QgsCoordinateSequence QgsSfcgalGeometry::coordinateSequence() const
{
  return mQgsGeom->coordinateSequence();
}

QgsPoint QgsSfcgalGeometry::vertexAt( QgsVertexId id ) const
{
  return mQgsGeom->vertexAt( id );
}

double QgsSfcgalGeometry::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  return mQgsGeom->closestSegment( pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsSfcgalGeometry::insertVertex( QgsVertexId, const QgsPoint & )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return false;
}

bool QgsSfcgalGeometry::moveVertex( QgsVertexId, const QgsPoint & )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return false;
}

bool QgsSfcgalGeometry::deleteVertex( QgsVertexId )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return false;
}

double QgsSfcgalGeometry::segmentLength( QgsVertexId startVertex ) const
{
  return mQgsGeom->segmentLength( startVertex );
}

QgsAbstractGeometry *QgsSfcgalGeometry::toCurveType() const
{
  return mQgsGeom->toCurveType();
}

QgsAbstractGeometry *QgsSfcgalGeometry::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing, bool removeRedundantPoints ) const
{
  return mQgsGeom->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing, removeRedundantPoints );
}

QgsAbstractGeometry *QgsSfcgalGeometry::simplifyByDistance( double tolerance ) const
{
  return mQgsGeom->simplifyByDistance( tolerance );
}

bool QgsSfcgalGeometry::removeDuplicateNodes( double, bool )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return false;
}

double QgsSfcgalGeometry::vertexAngle( QgsVertexId vertex ) const
{
  return mQgsGeom->vertexAngle( vertex );
}

int QgsSfcgalGeometry::vertexCount( int part, int ring ) const
{
  return mQgsGeom->vertexCount( part, ring );
}

int QgsSfcgalGeometry::ringCount( int ) const
{
  return mQgsGeom->partCount();
}

int QgsSfcgalGeometry::partCount() const
{
  return mQgsGeom->partCount();
}

bool QgsSfcgalGeometry::addZValue( double zValue )
{
  QString errorMsg; // used to retrieve failure messages if any
  const bool added = QgsSfcgalEngine::addZValue( mSfcgalGeom.get(), zValue, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );

  resetQgsGeometry();
  clearCache();

  return added;
}

bool QgsSfcgalGeometry::addMValue( double mValue )
{
  QString errorMsg; // used to retrieve failure messages if any
  const bool added = QgsSfcgalEngine::addMValue( mSfcgalGeom.get(), mValue, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );

  resetQgsGeometry();
  clearCache();

  return added;
}

bool QgsSfcgalGeometry::dropZValue()
{
  QString errorMsg; // used to retrieve failure messages if any
  const bool dropped = QgsSfcgalEngine::dropZValue( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );

  resetQgsGeometry();
  clearCache();

  return dropped;
}

bool QgsSfcgalGeometry::dropMValue()
{
  QString errorMsg; // used to retrieve failure messages if any
  const bool dropped = QgsSfcgalEngine::dropMValue( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );

  resetQgsGeometry();
  clearCache();

  return dropped;
}

void QgsSfcgalGeometry::swapXy()
{
  QString errorMsg; // used to retrieve failure messages if any
  QgsSfcgalEngine::swapXy( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, void() );

  resetQgsGeometry();
  clearCache();
}

bool QgsSfcgalGeometry::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  if ( flags == 0 && mHasCachedValidity )
  {
    // use cached validity results
    error = mValidityFailureReason;
    return error.isEmpty();
  }
  const bool valid = QgsSfcgalEngine::isValid( mSfcgalGeom.get(), &error, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, nullptr );
  if ( flags == 0 )
  {
    mValidityFailureReason = !valid ? error : QString();
    mHasCachedValidity = true;
  }
  return valid;
}

void QgsSfcgalGeometry::clearCache() const
{
  mHasCachedValidity = false;
  mValidityFailureReason.clear();
  QgsAbstractGeometry::clearCache();
}

bool QgsSfcgalGeometry::transform( QgsAbstractGeometryTransformer *, QgsFeedback * )
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return false;
}

int QgsSfcgalGeometry::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  QString errorMsg;
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( other, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, -1 );

  int res = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), otherShared.get(), 0.0, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, -1 );

  return res ? 0 : -1;
}

bool QgsSfcgalGeometry::isSimple( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool result = QgsSfcgalEngine::isSimple( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return result;
}

QgsPoint QgsSfcgalGeometry::centroid() const
{
  return centroid( nullptr );
}

QgsPoint QgsSfcgalGeometry::centroid( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  return QgsSfcgalEngine::centroid( mSfcgalGeom.get(), errorMsg );
}

bool QgsSfcgalGeometry::isEmpty() const
{
  QString errorMsg; // used to retrieve failure messages if any
  bool result = QgsSfcgalEngine::isEmpty( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );

  return result;
}

QgsSfcgalGeometry *QgsSfcgalGeometry::translate( const QgsPoint &translation, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::translate( mSfcgalGeom.get(), translation, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::scale( const QgsPoint &scaleFactor, const QgsPoint &center, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::scale( mSfcgalGeom.get(), scaleFactor, center, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::rotate2D( double angle, const QgsPoint &center, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate2D( mSfcgalGeom.get(), angle, center, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate3D( mSfcgalGeom.get(), angle, axisVector, center, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

double QgsSfcgalGeometry::area() const
{
  QString errorMsg; // used to retrieve failure messages if any
  double result = QgsSfcgalEngine::area( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

double QgsSfcgalGeometry::length() const
{
  QString errorMsg; // used to retrieve failure messages if any
  double result = QgsSfcgalEngine::length( mSfcgalGeom.get(), &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

bool QgsSfcgalGeometry::intersects( const QgsAbstractGeometry *other, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( other, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  return QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherShared.get(), errorMsg );
}

QgsSfcgalGeometry *QgsSfcgalGeometry::intersection( const QgsAbstractGeometry *other, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( other, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherShared.get(), errorMsg, parameters );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::combine( const QVector<const QgsAbstractGeometry *> &geomList, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  QVector<sfcgal::shared_geom> sfcgalGeomList;
  sfcgalGeomList.append( mSfcgalGeom );
  for ( QVector<const QgsAbstractGeometry *>::const_iterator ite = geomList.constBegin(); ite != geomList.constEnd(); ite++ )
  {
    sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( *ite, errorMsg );
    CHECK_SUCCESS( errorMsg, nullptr );
    sfcgalGeomList.append( otherShared );
  }

  sfcgal::shared_geom result = QgsSfcgalEngine::combine( sfcgalGeomList, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
  ;
}

QgsSfcgalGeometry *QgsSfcgalGeometry::difference( const QgsAbstractGeometry *other, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherSharedr = QgsSfcgalEngine::fromAbstractGeometry( other, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherSharedr.get(), errorMsg, parameters );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::triangulate( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::triangulate( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::convexhull( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::convexhull( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::envelope( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::envelope( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer3D( mSfcgalGeom.get(), radius, segments, joinStyle3D, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer2D( mSfcgalGeom.get(), radius, segments, joinStyle, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}
