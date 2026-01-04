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
  : mIsPrimitive( false )
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( const QgsAbstractGeometry *qgsGeom )
  : mIsPrimitive( false )
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
  : mIsPrimitive( false )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  mSfcgalGeom = QgsSfcgalEngine::fromAbstractGeometry( &qgsGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
}

QgsSfcgalGeometry::QgsSfcgalGeometry( sfcgal::shared_geom sfcgalGeom )
  : mSfcgalGeom( sfcgalGeom ), mIsPrimitive( false )
{
}

QgsSfcgalGeometry::QgsSfcgalGeometry( sfcgal::shared_prim sfcgalPrim, sfcgal::primitiveType type )
  : mIsPrimitive( true )
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  mSfcgalPrim = sfcgalPrim;
  mPrimType = type;
#else
  ( void ) sfcgalPrim;
  ( void ) type;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
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
  mIsPrimitive = otherGeom.mIsPrimitive;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  mPrimType = otherGeom.mPrimType;
  if ( mIsPrimitive )
    mSfcgalPrim = QgsSfcgalEngine::primitiveClone( otherGeom.mSfcgalPrim.get(), &errorMsg );
  else
#endif
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

sfcgal::shared_geom QgsSfcgalGeometry::workingGeom() const
{
  sfcgal::shared_geom geom;

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    QString errorMsg;
    geom = QgsSfcgalEngine::primitiveAsPolyhedral( mSfcgalPrim.get(), mPrimTransform, &errorMsg );
    THROW_ON_ERROR( &errorMsg );
  }
  else
#endif
    geom = mSfcgalGeom;

  return geom;
}


Qgis::WkbType QgsSfcgalGeometry::wkbType() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
    return Qgis::WkbType::PolyhedralSurfaceZ;
#endif

  Qgis::WkbType out = QgsSfcgalEngine::wkbType( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

QString QgsSfcgalGeometry::geometryType() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  QString out;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    switch ( mPrimType )
    {
      case sfcgal::primitiveType::SFCGAL_TYPE_CYLINDER:
        out = "cylinder";
        break;
      case sfcgal::primitiveType::SFCGAL_TYPE_SPHERE:
        out = "sphere";
        break;
      case sfcgal::primitiveType::SFCGAL_TYPE_TORUS:
        out = "torus";
        break;
      case sfcgal::primitiveType::SFCGAL_TYPE_BOX:
        out = "box";
        break;
      case sfcgal::primitiveType::SFCGAL_TYPE_CUBE:
        out = "cube";
        break;
      case sfcgal::primitiveType::SFCGAL_TYPE_CONE:
        out = "cone";
        break;
      default:
        sfcgal::errorHandler()->addText( u"Type '%1' is unknown."_s.arg( mPrimType ) );
    }
  }
  else
#endif
  {
    out = QgsSfcgalEngine::geometryType( mSfcgalGeom.get(), &errorMsg );
    THROW_ON_ERROR( &errorMsg );
  }
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

  sfcgal::shared_geom geom = workingGeom();
  QByteArray wkbArray = QgsSfcgalEngine::toWkb( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return wkbArray;
}

QString QgsSfcgalGeometry::asWkt( int precision ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  QString out = QgsSfcgalEngine::toWkt( geom.get(), precision, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return out;
}

std::unique_ptr<QgsAbstractGeometry> QgsSfcgalGeometry::asQgisGeometry() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  std::unique_ptr<QgsAbstractGeometry> out = QgsSfcgalEngine::toAbstractGeometry( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::boundary() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom boundary = QgsSfcgalEngine::boundary( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( boundary, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

bool QgsSfcgalGeometry::operator==( const QgsSfcgalGeometry &other ) const
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )other;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.1 or later" ) );
#else
  QString errorMsg;
  bool out;

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive != other.mIsPrimitive )
    return false;

  if ( mIsPrimitive )
  {
    out = QgsSfcgalEngine::primitiveIsEqual( mSfcgalPrim.get(), other.mSfcgalPrim.get(), -1.0, &errorMsg );
  }
  else
#endif
  {
    out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), -1.0, &errorMsg );
  }
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

  bool out;

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive != other.mIsPrimitive )
    return false;

  if ( mIsPrimitive )
  {
    out = QgsSfcgalEngine::primitiveIsEqual( mSfcgalPrim.get(), other.mSfcgalPrim.get(), epsilon, &errorMsg );
  }
  else
#endif
  {
    out = QgsSfcgalEngine::isEqual( mSfcgalGeom.get(), other.mSfcgalGeom.get(), epsilon, &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return out;
}

int QgsSfcgalGeometry::dimension() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    return 3;

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

  if ( mIsPrimitive )
    return 1;

  int out = QgsSfcgalEngine::partCount( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return out;
}

bool QgsSfcgalGeometry::addZValue( double zValue )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "addZValue" ) );

  const bool added = QgsSfcgalEngine::addZValue( mSfcgalGeom.get(), zValue, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::addMValue( double mValue )
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "addMValue" ) );

  const bool added = QgsSfcgalEngine::addMValue( mSfcgalGeom.get(), mValue, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return added;
}

bool QgsSfcgalGeometry::dropZValue()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "dropZValue" ) );

  const bool dropped = QgsSfcgalEngine::dropZValue( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return dropped;
}

bool QgsSfcgalGeometry::dropMValue()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "dropMValue" ) );

  const bool dropped = QgsSfcgalEngine::dropMValue( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();

  return dropped;
}

void QgsSfcgalGeometry::swapXy()
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "swapXy" ) );

  QgsSfcgalEngine::swapXy( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  clearCache();
}

bool QgsSfcgalGeometry::isValid() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    return true;

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

  if ( mIsPrimitive )
    return true;

  bool result = QgsSfcgalEngine::isSimple( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return result;
}

QgsPoint QgsSfcgalGeometry::centroid() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  return QgsSfcgalEngine::centroid( geom.get(), &errorMsg );
}

bool QgsSfcgalGeometry::isEmpty() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    return false;

  bool result = QgsSfcgalEngine::isEmpty( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::translate( const QgsVector3D &translation ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  std::unique_ptr<QgsSfcgalGeometry> resultGeom;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    sfcgal::shared_prim prim = QgsSfcgalEngine::primitiveClone( mSfcgalPrim.get(), &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( prim, mPrimType, &errorMsg );
    resultGeom->setPrimitiveTranslate( translation );
  }
  else
#endif
  {
    sfcgal::shared_geom result = QgsSfcgalEngine::translate( mSfcgalGeom.get(), translation, &errorMsg );
    THROW_ON_ERROR( &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::scale( const QgsVector3D &scaleFactor, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  std::unique_ptr<QgsSfcgalGeometry> resultGeom;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    sfcgal::shared_prim prim = QgsSfcgalEngine::primitiveClone( mSfcgalPrim.get(), &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( prim, mPrimType, &errorMsg );
    resultGeom->setPrimitiveScale( scaleFactor, center );
  }
  else
#endif
  {
    sfcgal::shared_geom result = QgsSfcgalEngine::scale( mSfcgalGeom.get(), scaleFactor, center, &errorMsg );
    THROW_ON_ERROR( &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate2D( double angle, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  std::unique_ptr<QgsSfcgalGeometry> resultGeom;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    sfcgal::shared_prim prim = QgsSfcgalEngine::primitiveClone( mSfcgalPrim.get(), &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( prim, mPrimType, &errorMsg );
    resultGeom->setPrimitiveRotation( angle, { 0.0, 0.0, 1.0 }, center );
  }
  else
#endif
  {
    sfcgal::shared_geom result = QgsSfcgalEngine::rotate2D( mSfcgalGeom.get(), angle, center, &errorMsg );
    THROW_ON_ERROR( &errorMsg );

    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::transform( const QMatrix4x4 &mat ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  sfcgal::shared_geom geom = workingGeom();

  sfcgal::shared_geom result = QgsSfcgalEngine::transform( geom.get(), mat );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
#else
  ( void ) mat;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::rotate3D( double angle, const QgsVector3D &axisVector, const QgsPoint &center ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  std::unique_ptr<QgsSfcgalGeometry> resultGeom;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    sfcgal::shared_prim prim = QgsSfcgalEngine::primitiveClone( mSfcgalPrim.get(), &errorMsg );
    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( prim, mPrimType, &errorMsg );
    resultGeom->setPrimitiveRotation( angle, axisVector, center );
  }
  else
#endif
  {
    sfcgal::shared_geom result = QgsSfcgalEngine::rotate3D( mSfcgalGeom.get(), angle, axisVector, center, &errorMsg );
    THROW_ON_ERROR( &errorMsg );

    resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

double QgsSfcgalGeometry::area( bool withDiscretization ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  double result;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( mIsPrimitive )
  {
    result = QgsSfcgalEngine::primitiveArea( mSfcgalPrim.get(), withDiscretization, &errorMsg );
  }
  else
#endif
  {
    ( void ) withDiscretization;
    result = QgsSfcgalEngine::area( mSfcgalGeom.get(), &errorMsg );
  }

  THROW_ON_ERROR( &errorMsg );
  return result;
}

double QgsSfcgalGeometry::length() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to primitive." ).arg( "length" ) );

  double result = QgsSfcgalEngine::length( mSfcgalGeom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
}

double QgsSfcgalGeometry::volume( bool withDiscretization ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  if ( !mIsPrimitive )
    throw QgsNotSupportedException( QObject::tr( "Operation '%1' does not apply to geometry." ).arg( "volume" ) );

  double result;
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  result = QgsSfcgalEngine::primitiveVolume( mSfcgalPrim.get(), withDiscretization, &errorMsg );
#else
  ( void ) withDiscretization;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif

  THROW_ON_ERROR( &errorMsg );
  return result;
}

bool QgsSfcgalGeometry::intersects( const QgsAbstractGeometry *otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  bool out = QgsSfcgalEngine::intersects( geom.get(), otherShared.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

bool QgsSfcgalGeometry::intersects( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  bool out = QgsSfcgalEngine::intersects( geom.get(), otherGeom.workingGeom().get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsAbstractGeometry *otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_geom otherShared = QgsSfcgalEngine::fromAbstractGeometry( otherGeom, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( geom.get(), otherShared.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::intersection( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::intersection( geom.get(), otherGeom.workingGeom().get(), &errorMsg );
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

  sfcgal::shared_geom geom = workingGeom();
  sfcgalGeomList.append( geom );
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

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::difference( geom.get(), otherSharedr.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::difference( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::difference( geom.get(), otherGeom.workingGeom().get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::triangulate() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::triangulate( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::convexHull() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::convexHull( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::envelope() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::envelope( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

bool QgsSfcgalGeometry::covers( const QgsSfcgalGeometry &otherGeom ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  bool out = QgsSfcgalEngine::covers( geom.get(), otherGeom.workingGeom().get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return out;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer3D( double radius, int segments, Qgis::JoinStyle3D joinStyle3D ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::buffer3D( geom.get(), radius, segments, joinStyle3D, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::buffer2D( double radius, int segments, Qgis::JoinStyle joinStyle ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::buffer2D( geom.get(), radius, segments, joinStyle, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::simplify( double tolerance, bool preserveTopology ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::simplify( geom.get(), tolerance, preserveTopology, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::extrude( const QgsVector3D &extrusion ) const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::extrude( geom.get(), extrusion, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::approximateMedialAxis() const
{
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );

  sfcgal::shared_geom geom = workingGeom();
  sfcgal::shared_geom result = QgsSfcgalEngine::approximateMedialAxis( geom.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::createCube( double size )
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_prim result = QgsSfcgalEngine::createCube( size, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, sfcgal::primitiveType::SFCGAL_TYPE_CUBE, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
#else
  ( void ) size;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalGeometry::primitiveAsPolyhedralSurface() const
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( !mIsPrimitive )
    throw QgsSfcgalException( "Need primitive geometry to operate." );

  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  sfcgal::shared_prim result = QgsSfcgalEngine::primitiveAsPolyhedral( mSfcgalPrim.get(), mPrimTransform, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  auto resultGeom = QgsSfcgalEngine::toSfcgalGeometry( result, &errorMsg );
  THROW_ON_ERROR( &errorMsg );
  return resultGeom;
#else
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

QMatrix4x4 QgsSfcgalGeometry::primitiveTransform() const
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( !mIsPrimitive )
    throw QgsSfcgalException( "Need primitive geometry to operate." );

  return mPrimTransform;
#else
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

QList<std::pair<QString, QString>> QgsSfcgalGeometry::primitiveParameters() const
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( !mIsPrimitive )
    throw QgsSfcgalException( "Need primitive geometry to operate." );

  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QVector<sfcgal::PrimitiveParameterDesc> result = QgsSfcgalEngine::primitiveParameters( mSfcgalPrim.get(), &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  QList<QPair<QString, QString>> out;
  for ( const auto &param : result )
  {
    out.append( std::pair<QString, QString>( QString::fromStdString( param.name ), QString::fromStdString( param.type ) ) );
  }

  return out;
#else
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

QVariant QgsSfcgalGeometry::primitiveParameter( const QString &name ) const
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( !mIsPrimitive )
    throw QgsSfcgalException( "Need primitive geometry to operate." );

  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QVariant result = QgsSfcgalEngine::primitiveParameter( mSfcgalPrim.get(), name, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

  return result;
#else
  ( void ) name;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}

void QgsSfcgalGeometry::primitiveSetParameter( const QString &name, const QVariant &value )
{
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
  if ( !mIsPrimitive )
    throw QgsSfcgalException( "Need primitive geometry to operate." );

  QString errorMsg;
  sfcgal::errorHandler()->clearText( &errorMsg );
  QgsSfcgalEngine::primitiveSetParameter( mSfcgalPrim.get(), name, value, &errorMsg );
  THROW_ON_ERROR( &errorMsg );

#else
  ( void ) name;
  ( void ) value;
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based on SFCGAL 2.3 or later" ) );
#endif
}


#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
void QgsSfcgalGeometry::setPrimitiveTranslate( const QgsVector3D &translation )
{
  mPrimTransform.translate( mPrimTransform.column( 3 ).toVector3D() + translation.toVector3D() );
}

void QgsSfcgalGeometry::setPrimitiveScale( const QgsVector3D &scaleFactor, const QgsPoint &center )
{
  QVector3D qCenter( center.x(), center.y(), center.z() );
  QVector3D prevTrans = mPrimTransform.column( 3 ).toVector3D();
  mPrimTransform.translate( prevTrans - qCenter );
  mPrimTransform.scale( scaleFactor.toVector3D() );
  mPrimTransform.translate( prevTrans + qCenter );
}

void QgsSfcgalGeometry::setPrimitiveRotation( double angle, const QgsVector3D &axisVector, const QgsPoint &center )
{
  QVector3D qCenter( center.x(), center.y(), center.z() );
  QVector3D prevTrans = mPrimTransform.column( 3 ).toVector3D();
  mPrimTransform.translate( prevTrans - qCenter );
  // TODO: need to merge previous rotation values with the new ones
  mPrimTransform.rotate( QQuaternion::fromAxisAndAngle( axisVector.toVector3D(), angle ) );
  mPrimTransform.translate( prevTrans + qCenter );
}

#endif

#endif
