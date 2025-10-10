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

#include <QByteArray>

QgsSfcgalGeometry::QgsSfcgalGeometry()
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom )
{
  if ( qgsGeom )
  {
    QString errorMsg;
    sfcgal::errorHandler()->clearText( &errorMsg );
    mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom, &errorMsg );
    THROW_ON_ERROR( &errorMsg );
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( &qgsGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom )
  : mSfcgalGeom( sfcgalGeom )
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsGeometry &qgsGeom )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom.constGet(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::cloneGeometry( otherGeom.mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QString &wkt )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
}


Qgis::WkbType QgsSfcgalGeometry::wkbType() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  Qgis::WkbType out = QgsSfcgalEngine::wkbType( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

QString QgsSfcgalGeometry::geometryType() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QString out = QgsSfcgalEngine::geometryType( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
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
  THROW_ON_ERROR( &errorMsg );

  return std::make_unique<QgsSfcgalGeometry>( sfcgalGeom );
}


QByteArray QgsSfcgalGeometry::asWkb( QgsAbstractGeometry::WkbFlags ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QByteArray wkbArray = QgsSfcgalEngine::toWkb( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return wkbArray;
}

QString QgsSfcgalGeometry::asWkt( int precision ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  QString out = QgsSfcgalEngine::toWkt( mSfcgalGeom.get(), precision, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return out;
}

std::unique_ptr<QgsAbstractGeometry> QgsSfcgalGeometry::asQgisGeometry() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  std::unique_ptr<QgsAbstractGeometry> out = QgsSfcgalEngine::toAbstractGeometry( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::boundary() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom boundary = QgsSfcgalEngine::boundary( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( boundary, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

bool QgsSfcgalGeometry::operator==( const QgsSfcgalGeometry &other ) const
{
#if SFCGAL_VERSION_MAJOR_INT == 2 && SFCGAL_VERSION_MINOR_INT < 1
  ( void )other;
  throw QgsNotSupportedException( QObject::tr( "This operator requires a QGIS build based on SFCGAL 2.1 or later" ) );
#else
  QString errorMsg;
  bool out = false;
  out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), 0.0, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
#endif
}

bool QgsSfcgalGeometry::operator!=( const QgsSfcgalGeometry &other ) const
{
  return !( *this == other );
}

bool QgsSfcgalGeometry::fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  bool out = false;
  out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), epsilon, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return out;
}

int QgsSfcgalGeometry::dimension() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  int result = QgsSfcgalEngine::dimension( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::fromWkt( const QString &wkt )
{
  QString errorMsg;

  sfcgal::shared_geom sfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return std::make_unique<QgsSfcgalGeometry>( sfcgalGeom );
}

int QgsSfcgalGeometry::partCount() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  int out = QgsSfcgalEngine::partCount( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return out;
}

bool QgsSfcgalGeometry::addZValue( double zValue )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  const bool added = QgsSfcgalEngine::addZValue( mSfcgalGeom.get(), zValue, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::addMValue( double mValue )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  const bool added = QgsSfcgalEngine::addMValue( mSfcgalGeom.get(), mValue, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::dropZValue()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  const bool dropped = QgsSfcgalEngine::dropZValue( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return dropped;
}

bool QgsSfcgalGeometry::dropMValue()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  const bool dropped = QgsSfcgalEngine::dropMValue( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return dropped;
}

void QgsSfcgalGeometry::swapXy()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QgsSfcgalEngine::swapXy( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();
}

bool QgsSfcgalGeometry::isValid() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  const bool valid = QgsSfcgalEngine::isValid( mSfcgalGeom.get(), &errorMsg, nullptr );
  THROW_ON_ERROR( &errorMsg );
  return valid;
}

void QgsSfcgalGeometry::clearCache() const
{
}

bool QgsSfcgalGeometry::isSimple() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  bool result = QgsSfcgalEngine::isSimple( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return result;
}

QgsPoint QgsSfcgalGeometry::centroid() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  return QgsSfcgalEngine::centroid( mSfcgalGeom.get(), &errorMsg );
}

bool QgsSfcgalGeometry::isEmpty() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  bool result = QgsSfcgalEngine::isEmpty( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::translate( const QgsVector3D &translation ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::translate( mSfcgalGeom.get(), translation, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::scale( const QgsVector3D &scaleFactor, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::scale( mSfcgalGeom.get(), scaleFactor, center, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate2D( double angle, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate2D( mSfcgalGeom.get(), angle, center, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::rotate3D( mSfcgalGeom.get(), angle, axisVector, center, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

double QgsSfcgalGeometry::area() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  double result = QgsSfcgalEngine::area( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

double QgsSfcgalGeometry::length() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  double result = QgsSfcgalEngine::length( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

bool QgsSfcgalGeometry::intersects( const QgsAbstractGeometry *otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherShared.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

bool QgsSfcgalGeometry::intersects( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsAbstractGeometry *otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherShared.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::combine( const QVector<QgsAbstractGeometry *> &geomList ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QVector<sfcgal::shared_geom> sfcgalGeomList;
  sfcgalGeomList.append( mSfcgalGeom );
  for ( QVector<QgsAbstractGeometry *>::const_iterator ite = geomList.constBegin(); ite != geomList.constEnd(); ++ite )
  {
    sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( *ite, &errorMsg );
    THROW_ON_ERROR( &errorMsg );
    sfcgalGeomList.append( otherShared );
  }

  sfcgal::shared_geom result = QgsSfcgalEngine::combine( sfcgalGeomList, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::difference( const QgsAbstractGeometry *otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom otherSharedr = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherSharedr.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::difference( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::triangulate() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::triangulate( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::convexHull() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::convexHull( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::envelope() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::envelope( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

bool QgsSfcgalGeometry::covers( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  bool out = QgsSfcgalEngine::covers( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer3D( mSfcgalGeom.get(), radius, segments, joinStyle3D, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::buffer2D( mSfcgalGeom.get(), radius, segments, joinStyle, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::simplify( double tolerance, bool preserveTopology ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::simplify( mSfcgalGeom.get(), tolerance, preserveTopology, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::extrude( const QgsVector3D &extrusion ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::extrude( mSfcgalGeom.get(), extrusion, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::approximateMedialAxis() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::approximateMedialAxis( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

#endif
