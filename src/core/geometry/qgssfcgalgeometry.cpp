/***************************************************************************
                         qgssfcgalGeometry.cpp
                         ----------------
    begin                : May 2025
    copyright            : (C) 2025 by Oslandia
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

#ifdef WITH_SFCGAL

#include "qgssfcgalgeometry.h"
#include "qgsvector3d.h"
#include "qgswkbptr.h"
#include "qgslogger.h"

#include <QByteArray>

QgsSfcgalGeometry::QgsSfcgalGeometry()
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom )
{
  if ( qgsGeom )
  {
    sfcgal::errorHandler()->clearText( &mLastError );
    mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom, &mLastError );
    if ( !sfcgal::errorHandler()->hasSucceedOrStack( &mLastError ) )
    {
      QgsDebugError( sfcgal::errorHandler()->getFullText() );
    }
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( &qgsGeom, &mLastError );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( &mLastError ) )
  {
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom )
  : mSfcgalGeom( sfcgalGeom )
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsGeometry &qgsGeom )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom.constGet(), &mLastError );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( &mLastError ) )
  {
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  mSfcgalGeom = QgsSfcgalEngine::cloneGeometry( otherGeom.mSfcgalGeom.get(), &mLastError );
  otherGeom.mLastError = mLastError;
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( &mLastError ) )
  {
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QString &wkt )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  mSfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, &mLastError );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( &mLastError ) )
  {
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
  }
}


Qgis::WkbType QgsSfcgalGeometry::wkbType() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  Qgis::WkbType out = QgsSfcgalEngine::wkbType( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, Qgis::WkbType::Unknown );
  return out;
}

QString QgsSfcgalGeometry::geometryType() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  QString out = QgsSfcgalEngine::geometryType( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, "" );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::clone() const
{
  return std::make_unique<QgsSfcgalGeometry>( *this );
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::fromWkb( const QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
  {
    return nullptr;
  }

  QString errorMsg;
  sfcgal::shared_geom sfcgalGeom = QgsSfcgalEngine::fromWkb( wkbPtr, &errorMsg );
  CHECK_SUCCESS( &errorMsg, nullptr );

  return std::make_unique<QgsSfcgalGeometry>( sfcgalGeom );
}


QByteArray QgsSfcgalGeometry::asWkb( QgsAbstractGeometry::WkbFlags ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  QgsConstWkbPtr ptr = QgsSfcgalEngine::toWkb( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, QByteArray() );

  const unsigned char *wkbUnsignedPtr = ptr;
  return QByteArray( reinterpret_cast<const char *>( wkbUnsignedPtr ), ptr.remaining() );
}

QString QgsSfcgalGeometry::asWkt( int precision ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );

  QString out = QgsSfcgalEngine::toWkt( mSfcgalGeom.get(), precision, &mLastError );
  CHECK_SUCCESS( &mLastError, QString() );

  return out;
}

std::unique_ptr<QgsAbstractGeometry> QgsSfcgalGeometry::asQgisGeometry() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  std::unique_ptr<QgsAbstractGeometry> out = QgsSfcgalEngine::toAbstractGeometry( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::boundary() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom boundary = QgsSfcgalEngine::boundary( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( boundary, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

bool QgsSfcgalGeometry::operator==( const QgsSfcgalGeometry &other ) const
{
#if SFCGAL_VERSION_MAJOR_INT == 2 && SFCGAL_VERSION_MINOR_INT < 1
  ( void )other;
  throw QgsNotSupportedException( QObject::tr( "This operator requires a QGIS build based on SFCGAL 2.1 or later" ) );
#else
  bool out = false;
  out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), 0.0, &mLastError );
  CHECK_SUCCESS_LOG( &mLastError, false );
  return out;
#endif
}

bool QgsSfcgalGeometry::operator!=( const QgsSfcgalGeometry &other ) const
{
  return !( *this == other );
}

bool QgsSfcgalGeometry::fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  bool out = false;
  out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), epsilon, &mLastError );
  CHECK_SUCCESS( &mLastError, false );
  other.mLastError = mLastError;
  return out;
}

int QgsSfcgalGeometry::dimension() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  int result = QgsSfcgalEngine::dimension( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, -1 );

  return result;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::fromWkt( const QString &wkt )
{
  QString errorMsg;

  sfcgal::shared_geom sfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, &errorMsg );
  CHECK_SUCCESS( &errorMsg, nullptr );

  return std::make_unique<QgsSfcgalGeometry>( sfcgalGeom );
}

int QgsSfcgalGeometry::partCount() const
{
  sfcgal::errorHandler()->clearText( &mLastError );

  int out = QgsSfcgalEngine::partCount( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, -1 );

  return out;
}

bool QgsSfcgalGeometry::addZValue( double zValue )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  const bool added = QgsSfcgalEngine::addZValue( mSfcgalGeom.get(), zValue, &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::addMValue( double mValue )
{
  sfcgal::errorHandler()->clearText( &mLastError );
  const bool added = QgsSfcgalEngine::addMValue( mSfcgalGeom.get(), mValue, &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::dropZValue()
{
  sfcgal::errorHandler()->clearText( &mLastError );
  const bool dropped = QgsSfcgalEngine::dropZValue( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  clearCache();

  return dropped;
}

bool QgsSfcgalGeometry::dropMValue()
{
  sfcgal::errorHandler()->clearText( &mLastError );
  const bool dropped = QgsSfcgalEngine::dropMValue( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  clearCache();

  return dropped;
}

void QgsSfcgalGeometry::swapXy()
{
  sfcgal::errorHandler()->clearText( &mLastError );
  QgsSfcgalEngine::swapXy( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, void() );

  clearCache();
}

bool QgsSfcgalGeometry::isValid() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  const bool valid = QgsSfcgalEngine::isValid( mSfcgalGeom.get(), &mLastError, nullptr );
  CHECK_SUCCESS( &mLastError, false );
  return valid;
}

void QgsSfcgalGeometry::clearCache() const
{
}

bool QgsSfcgalGeometry::isSimple() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  bool result = QgsSfcgalEngine::isSimple( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, false );
  return result;
}

QgsPoint QgsSfcgalGeometry::centroid() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  return QgsSfcgalEngine::centroid( mSfcgalGeom.get(), &mLastError );
}

bool QgsSfcgalGeometry::isEmpty() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  bool result = QgsSfcgalEngine::isEmpty( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  return result;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::translate( const QgsVector3D &translation ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::translate( mSfcgalGeom.get(), translation, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::scale( const QgsVector3D &scaleFactor, const QgsPoint &center ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::scale( mSfcgalGeom.get(), scaleFactor, center, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate2D( double angle, const QgsPoint &center ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate2D( mSfcgalGeom.get(), angle, center, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate3D( mSfcgalGeom.get(), angle, axisVector, center, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

double QgsSfcgalGeometry::area() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  double result = QgsSfcgalEngine::area( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

double QgsSfcgalGeometry::length() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  double result = QgsSfcgalEngine::length( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

bool QgsSfcgalGeometry::intersects( const QgsAbstractGeometry *otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &mLastError );
  CHECK_SUCCESS( &mLastError, false );

  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherShared.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, false );
  return out;
}

bool QgsSfcgalGeometry::intersects( const QgsSfcgalGeometry &otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &mLastError );
  otherGeom.mLastError = mLastError;
  CHECK_SUCCESS( &mLastError, false );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsAbstractGeometry *otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherShared.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsSfcgalGeometry &otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &mLastError );
  otherGeom.mLastError = mLastError;
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::combine( const QVector<QgsAbstractGeometry *> &geomList ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  QVector<sfcgal::shared_geom> sfcgalGeomList;
  sfcgalGeomList.append( mSfcgalGeom );
  for ( QVector<QgsAbstractGeometry *>::const_iterator ite = geomList.constBegin(); ite != geomList.constEnd(); ++ite )
  {
    sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( *ite, &mLastError );
    CHECK_SUCCESS( &mLastError, nullptr );
    sfcgalGeomList.append( otherShared );
  }

  sfcgal::shared_geom result = QgsSfcgalEngine::combine( sfcgalGeomList, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::difference( const QgsAbstractGeometry *otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom otherSharedr = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherSharedr.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::difference( const QgsSfcgalGeometry &otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &mLastError );
  otherGeom.mLastError = mLastError;
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::triangulate() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::triangulate( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::convexHull() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::convexHull( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::envelope() const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::envelope( mSfcgalGeom.get(), &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

bool QgsSfcgalGeometry::covers( const QgsSfcgalGeometry &otherGeom ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  bool out = QgsSfcgalEngine::covers( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &mLastError );
  otherGeom.mLastError = mLastError;
  CHECK_SUCCESS( &mLastError, false );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer3D( mSfcgalGeom.get(), radius, segments, joinStyle3D, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer2D( mSfcgalGeom.get(), radius, segments, joinStyle, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::simplify( double tolerance, bool preserveTopology ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::simplify( mSfcgalGeom.get(), tolerance, preserveTopology, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::extrude( const QgsVector3D &extrusion ) const
{
  sfcgal::errorHandler()->clearText( &mLastError );
  sfcgal::shared_geom result = QgsSfcgalEngine::extrude( mSfcgalGeom.get(), extrusion, &mLastError );
  CHECK_SUCCESS( &mLastError, nullptr );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &mLastError );
  resultGeom->mLastError = mLastError;
  return resultGeom;
}
#endif
