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
#include <QByteArray>

QgsSfcgalGeometry::QgsSfcgalGeometry()
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( std::unique_ptr<QgsAbstractGeometry> &qgsGeom )
{
  if ( qgsGeom )
  {
    sfcgal::errorHandler()->clearText();
    mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom.get() );
    if ( !sfcgal::errorHandler()->hasSucceedOrStack() )
      QgsDebugError( sfcgal::errorHandler()->getFullText() );
  }
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry &qgsGeom )
{
  sfcgal::errorHandler()->clearText();
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( &qgsGeom );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack() )
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom )
  : mSfcgalGeom( sfcgalGeom )
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsGeometry &qgsGeom )
{
  sfcgal::errorHandler()->clearText();
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( qgsGeom.constGet() );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack() )
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsSfcgalGeometry &otherGeom )
{
  sfcgal::errorHandler()->clearText();
  mSfcgalGeom = QgsSfcgalEngine::cloneGeometry( otherGeom.mSfcgalGeom.get() );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack() )
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QString &wkt, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, errorMsg );
  if ( !sfcgal::errorHandler()->hasSucceedOrStack( errorMsg ) )
    QgsDebugError( sfcgal::errorHandler()->getFullText() );
}


Qgis::WkbType QgsSfcgalGeometry::wkbType( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  Qgis::WkbType out = QgsSfcgalEngine::wkbType( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, Qgis::WkbType::Unknown );
  return out;
}


QString QgsSfcgalGeometry::geometryType( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  QString out = QgsSfcgalEngine::geometryType( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, "" );
  return out;
}

QgsSfcgalGeometry *QgsSfcgalGeometry::clone() const
{
  return new QgsSfcgalGeometry( *this );
}

bool QgsSfcgalGeometry::fromWkb( QgsConstWkbPtr &wkbPtr, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  if ( !wkbPtr )
    return false;

  mSfcgalGeom = QgsSfcgalEngine::fromWkb( wkbPtr, errorMsg );

  return true;
}


QByteArray QgsSfcgalGeometry::asWkb( QgsAbstractGeometry::WkbFlags, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  QgsConstWkbPtr ptr = QgsSfcgalEngine::toWkb( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, QByteArray() );

  const unsigned char *wkbUnsignedPtr = ptr;
  return QByteArray( reinterpret_cast<const char *>( wkbUnsignedPtr ), ptr.remaining() );
}

QString QgsSfcgalGeometry::asWkt( int precision, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );

  QString out = QgsSfcgalEngine::toWkt( mSfcgalGeom.get(), precision, errorMsg );
  CHECK_SUCCESS( errorMsg, QString() );

  return out;
}

QgsAbstractGeometry *QgsSfcgalGeometry::asQgisGeometry( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  std::unique_ptr<QgsAbstractGeometry> out = QgsSfcgalEngine::toAbstractGeometry( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out.release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::boundary( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom boundary = QgsSfcgalEngine::boundary( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( boundary ).release();
}

bool QgsSfcgalGeometry::operator==( const QgsSfcgalGeometry &other ) const
{
  bool out = false;
  QString errorMsg; // used to retrieve failure messages if any
  out = QgsSfcgalEngine::isEquals( mSfcgalGeom.get(), other.mSfcgalGeom.get(), 0.0, &errorMsg );
  CHECK_SUCCESS_LOG( &errorMsg, false );
  return out;
}

bool QgsSfcgalGeometry::operator!=( const QgsSfcgalGeometry &other ) const
{
  return !( *this == other );
}

bool QgsSfcgalGeometry::fuzzyEqual( const QgsSfcgalGeometry &other, double epsilon, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool out = false;
  out = QgsSfcgalEngine::isEquals( mSfcgalGeom.get(), other.mSfcgalGeom.get(), epsilon, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return out;
}

QgsBox3D QgsSfcgalGeometry::boundingBox3D( QString * ) const
{
  QgsDebugError( QStringLiteral( "Not implemented" ) );
  return QgsBox3D();
}

int QgsSfcgalGeometry::dimension( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  int result = QgsSfcgalEngine::dimension( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, -1 );

  return result;
}

bool QgsSfcgalGeometry::fromWkt( const QString &wkt, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromWkt( wkt, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  clearCache();

  return true;
}

int QgsSfcgalGeometry::partCount( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );

  int out = QgsSfcgalEngine::partCount( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, -1 );

  return out;
}

bool QgsSfcgalGeometry::addZValue( double zValue, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  const bool added = QgsSfcgalEngine::addZValue( mSfcgalGeom.get(), zValue, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::addMValue( double mValue, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  const bool added = QgsSfcgalEngine::addMValue( mSfcgalGeom.get(), mValue, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::dropZValue( QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  const bool dropped = QgsSfcgalEngine::dropZValue( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  clearCache();

  return dropped;
}

bool QgsSfcgalGeometry::dropMValue( QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  const bool dropped = QgsSfcgalEngine::dropMValue( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  clearCache();

  return dropped;
}

void QgsSfcgalGeometry::swapXy( QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  QgsSfcgalEngine::swapXy( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, void() );

  clearCache();
}

bool QgsSfcgalGeometry::isValid( Qgis::GeometryValidityFlags flags, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  const bool valid = QgsSfcgalEngine::isValid( mSfcgalGeom.get(), errorMsg, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, nullptr );
  CHECK_SUCCESS( errorMsg, false );
  return valid;
}


void QgsSfcgalGeometry::clearCache() const
{
}

bool QgsSfcgalGeometry::isSimple( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool result = QgsSfcgalEngine::isSimple( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return result;
}

QgsPoint QgsSfcgalGeometry::centroid( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::errorHandler()->clearText( errorMsg );
  return QgsSfcgalEngine::centroid( mSfcgalGeom.get(), errorMsg );
}

bool QgsSfcgalGeometry::isEmpty( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool result = QgsSfcgalEngine::isEmpty( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );

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

double QgsSfcgalGeometry::area( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  double result = QgsSfcgalEngine::area( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

double QgsSfcgalGeometry::length( QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  double result = QgsSfcgalEngine::length( mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );

  return result;
}

bool QgsSfcgalGeometry::intersects( const QgsAbstractGeometry *otherGeom, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherShared.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return out;
}

bool QgsSfcgalGeometry::intersects( const QgsSfcgalGeometry &otherGeom, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool out = QgsSfcgalEngine::intersects( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return out;
}

QgsSfcgalGeometry *QgsSfcgalGeometry::intersection( const QgsAbstractGeometry *otherGeom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherShared.get(), errorMsg, parameters );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::intersection( const QgsSfcgalGeometry &otherGeom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), errorMsg, parameters );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::combine( const QVector<const QgsAbstractGeometry *> &geomList, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  QVector<sfcgal::shared_geom> sfcgalGeomList;
  sfcgalGeomList.append( mSfcgalGeom );
  for ( QVector<const QgsAbstractGeometry *>::const_iterator ite = geomList.constBegin(); ite != geomList.constEnd(); ++ite )
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

QgsSfcgalGeometry *QgsSfcgalGeometry::difference( const QgsAbstractGeometry *otherGeom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom otherSharedr = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherSharedr.get(), errorMsg, parameters );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::difference( const QgsSfcgalGeometry &otherGeom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );

  sfcgal::shared_geom result = QgsSfcgalEngine::difference( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), errorMsg, parameters );
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

bool QgsSfcgalGeometry::covers( const QgsSfcgalGeometry &otherGeom, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  bool out = QgsSfcgalEngine::covers( mSfcgalGeom.get(), otherGeom.mSfcgalGeom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return out;
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

QgsSfcgalGeometry *QgsSfcgalGeometry::simplify( double tolerance, bool preserveTopology, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::simplify( mSfcgalGeom.get(), tolerance, preserveTopology, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}

QgsSfcgalGeometry *QgsSfcgalGeometry::extrude( const QgsPoint &extrusion, QString *errorMsg ) const
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::shared_geom result = QgsSfcgalEngine::extrude( mSfcgalGeom.get(), extrusion, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return QgsSfcgalEngine::toSfcgalGeometry( result, errorMsg ).release();
}
