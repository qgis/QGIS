/***************************************************************************
                         qgssfcgalengine.cpp
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

#include <SFCGAL/capi/sfcgal_c.h>

#include "qgssfcgalengine.h"
#include "qgsgeometry.h"
#include "qgspolygon.h"
#include "qgsgeometryfactory.h"
#include "qgssfcgalgeometry.h"

// ===================================
// sfcgal namespace
// ===================================

Q_GLOBAL_STATIC( sfcgal::ErrorHandler, _sfcgalErrorHandler )

sfcgal::ErrorHandler *sfcgal::errorHandler()
{
  return _sfcgalErrorHandler;
}


void sfcgal::Deleter::operator()( sfcgal::geometry *geom ) const
{
  sfcgal_geometry_delete( geom );
}

sfcgal::shared_geom sfcgal::make_shared_geom( sfcgal::geometry *geom )
{
  return sfcgal::shared_geom( geom, sfcgal::Deleter() );
}

bool sfcgal::ErrorHandler::hasSucceedOrStack( QString *errorMsg, const char *fromFile, const char *fromFunc, int fromLine )
{
  bool succeed = isTextEmpty();
  if ( !succeed )
  {
    addText( "relaying error from: ", fromFile, fromFunc, fromLine );
    if ( errorMsg )
    {
      errorMsg->append( errorMessages.first() );
    }
  }
  return succeed;
}

int sfcgal::errorCallback( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  sfcgal::errorHandler()->addText( QStringLiteral( "SFCGAL error occurred: %1" ).arg( buffer ), __FILE__, __FUNCTION__, __LINE__ );

  return static_cast<int>( strlen( buffer ) );
}

int sfcgal::warningCallback( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  sfcgal::errorHandler()->addText( QStringLiteral( "SFCGAL warning occurred: %1" ).arg( buffer ), __FILE__, __FUNCTION__, __LINE__ );

  return static_cast<int>( strlen( buffer ) );
}


sfcgal::ErrorHandler::ErrorHandler()
{
  sfcgal_init(); // empty but called
  sfcgal_set_error_handlers( sfcgal::warningCallback, sfcgal::errorCallback );
}

void sfcgal::ErrorHandler::clearText( QString *errorMsg )
{
  errorMessages.clear();
  if ( errorMsg )
  {
    errorMsg->clear();
  }
}

QString sfcgal::ErrorHandler::getMainText() const
{
  return errorMessages.isEmpty() ? QString( "No error!" ) : QString( "Error occurred: " ) + errorMessages.last();
}

QString sfcgal::ErrorHandler::getFullText() const
{
  return errorMessages.isEmpty() ? QString( "No error!" ) : getMainText() + "\n\t\t" + errorMessages.join( "\n\t\t" );
}

bool sfcgal::ErrorHandler::isTextEmpty() const
{
  return errorMessages.isEmpty();
}

void sfcgal::ErrorHandler::addText( const QString &msg, const char *fromFile, const char *fromFunc, int fromLine )
{
  QString txt = QString( "%2 (%3:%4) %1" ).arg( msg ).arg( fromFunc ).arg( fromFile ).arg( fromLine );

  errorMessages.push_front( txt );
}

// ===================================
// QgsSfcgalEngine lambda functions
// ===================================

/**
 * Lambda for functions taking 1 geometry in parameter returning a primitive type
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geom geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
template<typename T>
std::function<T(
  T( * )( const sfcgal_geometry_t * ), T( * )( const sfcgal_geometry_t * ),
  const sfcgal::geometry *,
  QString *
)>
lambda_geom_to_prim = [](
                        T( *func_2d )( const sfcgal_geometry_t * ),
                        T( *func_3d )( const sfcgal_geometry_t * ),
                        const sfcgal::geometry *geom,
                        QString *errorMsg
                      )
                      -> T
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, std::numeric_limits<T>::quiet_NaN() );

  T result;
  if ( func_3d && sfcgal_geometry_is_3d( geom ) )
    result = func_3d( geom );
  else
    result = func_2d( geom );

  CHECK_SUCCESS( errorMsg, std::numeric_limits<T>::quiet_NaN() );

  return result;
};

/**
 * Lambda for functions taking 2 geometries in parameter returning a primitive type
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geomA first geometry to apply func_2d ou func_3d
 * \param geomB second geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
template<typename T>
std::function<T(
  T( * )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
  T( * )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
  const sfcgal::geometry *,
  const sfcgal::geometry *,
  QString *
)>
lambda_geomgeom_to_prim = [](
                            T( *func_2d )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
                            T( *func_3d )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
                            const sfcgal::geometry *geomA,
                            const sfcgal::geometry *geomB,
                            QString *errorMsg
                          )
                          -> T
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geomA, false );
  CHECK_NOT_NULL( geomB, false );

  T result;
  if ( func_3d && ( sfcgal_geometry_is_3d( geomA ) || sfcgal_geometry_is_3d( geomB ) ) )
    result = func_3d( geomA, geomB );
  else
    result = func_2d( geomA, geomB );

  CHECK_SUCCESS( errorMsg, std::numeric_limits<T>::quiet_NaN() );

  return result;
};

/**
 * Lambda for functions taking 1 geometry in parameter returning a shared ptr to geometry
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geom geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
std::function<sfcgal::shared_geom(
  sfcgal::func_geom_to_geom, sfcgal::func_geom_to_geom,
  const sfcgal::geometry *,
  QString *
)>
lambda_geom_to_geom = [](
                        sfcgal::func_geom_to_geom func_2d,
                        sfcgal::func_geom_to_geom func_3d,
                        const sfcgal::geometry *geom,
                        QString *errorMsg
                      )
                      -> sfcgal::shared_geom
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = nullptr;
  if ( func_3d && sfcgal_geometry_is_3d( geom ) )
    result = func_3d( geom );
  else
    result = func_2d( geom );

  CHECK_SUCCESS( errorMsg, nullptr );
  CHECK_NOT_NULL( result, nullptr );

  return sfcgal::make_shared_geom( result );
};


/**
 * Lambda for functions taking 2 geometries in parameter returning a shared ptr to geometry
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geomA first geometry to apply func_2d ou func_3d
 * \param geomB second geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
std::function<sfcgal::shared_geom(
  sfcgal::func_geomgeom_to_geom, sfcgal::func_geomgeom_to_geom,
  const sfcgal::geometry *,
  const sfcgal::geometry *,
  QString *
)>
lambda_geomgeom_to_geom = [](
                            sfcgal::func_geomgeom_to_geom func_2d,
                            sfcgal::func_geomgeom_to_geom func_3d,
                            const sfcgal::geometry *geomA,
                            const sfcgal::geometry *geomB,
                            QString *errorMsg
                          )
                          -> sfcgal::shared_geom
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geomA, nullptr );
  CHECK_NOT_NULL( geomB, nullptr );

  sfcgal::geometry *result = nullptr;
  if ( func_3d && ( sfcgal_geometry_is_3d( geomA ) || sfcgal_geometry_is_3d( geomB ) ) )
    result = func_3d( geomA, geomB );
  else
    result = func_2d( geomA, geomB );

  CHECK_SUCCESS( errorMsg, nullptr );
  CHECK_NOT_NULL( result, nullptr );

  return sfcgal::make_shared_geom( result );
};


// ===================================
// QgsSfcgalEngine class
// ===================================


std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalEngine::toSfcgalGeometry( sfcgal::shared_geom &geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom.get(), nullptr );

  std::unique_ptr<QgsAbstractGeometry> qgsGeom = QgsSfcgalEngine::toAbstractGeometry( geom.get(), errorMsg );
  CHECK_SUCCESS( errorMsg, std::unique_ptr<QgsSfcgalGeometry>( nullptr ) );

  return std::make_unique<QgsSfcgalGeometry>( qgsGeom, geom );
}

std::unique_ptr<QgsAbstractGeometry> QgsSfcgalEngine::toAbstractGeometry( const sfcgal::geometry *geom, QString *errorMsg )
{
  std::unique_ptr<QgsAbstractGeometry> out( nullptr );
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, out );

  QgsConstWkbPtr ptr = QgsSfcgalEngine::toWkb( geom, errorMsg );
  CHECK_SUCCESS( errorMsg, out );

  out = QgsGeometryFactory::geomFromWkb( ptr );
  if ( !out )
  {
    Qgis::WkbType sfcgalType = QgsSfcgalEngine::wkbType( geom );
    QgsConstWkbPtr ptrError = QgsSfcgalEngine::toWkb( geom );
    sfcgal::errorHandler()->addText( QStringLiteral( "WKB contains unmanaged geometry type (WKB:%1 / SFCGAL:%2" ) //
                                     .arg( static_cast<int>( ptrError.readHeader() ) )                          //
                                     .arg( static_cast<int>( sfcgalType ) ),
                                     __FILE__, __FUNCTION__, __LINE__ );
  }

  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::fromAbstractGeometry( const QgsAbstractGeometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, sfcgal::shared_geom( nullptr ) );

  const QgsSfcgalGeometry *sfcgalGeometry = dynamic_cast<const QgsSfcgalGeometry *>( geom );
  if ( sfcgalGeometry )
    return sfcgalGeometry->sfcgalGeometry();
  else
  {
    QByteArray wkbBytes = geom->asWkb();

    sfcgal::geometry *out = sfcgal_io_read_wkb( wkbBytes.data(), wkbBytes.length() );
    CHECK_SUCCESS( errorMsg, nullptr );

    return sfcgal::make_shared_geom( out );
  }
}

sfcgal::shared_geom QgsSfcgalEngine::cloneGeometry( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = lambda_geom_to_geom( sfcgal_geometry_clone, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

QString QgsSfcgalEngine::geometryType( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );

  char *typeChar;
  size_t typeLen;
  sfcgal_geometry_type( geom, &typeChar, &typeLen );
  std::string typeStr( typeChar, typeLen );
  sfcgal_free_buffer( typeChar );

  return QString::fromStdString( typeStr );
}

sfcgal::shared_geom QgsSfcgalEngine::fromWkb( const QgsConstWkbPtr &wkbPtr, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );

  const unsigned char *wkbUnsignedPtr = wkbPtr;
  sfcgal::geometry *out = sfcgal_io_read_wkb( reinterpret_cast<const char *>( wkbUnsignedPtr ), wkbPtr.remaining() );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( out );
}

sfcgal::shared_geom QgsSfcgalEngine::fromWkt( const QString &wkt, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );

  sfcgal::geometry *out = sfcgal_io_read_wkt( wkt.toStdString().c_str(), wkt.length() );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::unique_geom( out );
}

QgsConstWkbPtr QgsSfcgalEngine::toWkb( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, QgsConstWkbPtr( nullptr, 0 ) );

  char *wkbHex;
  size_t len = 0;
  sfcgal_geometry_as_wkb( geom, &wkbHex, &len );
  CHECK_SUCCESS( errorMsg, QgsConstWkbPtr( nullptr, 0 ) );

  return QgsConstWkbPtr( reinterpret_cast<unsigned char *>( wkbHex ), static_cast<int>( len ) );
}

QString QgsSfcgalEngine::toWkt( const sfcgal::geometry *geom, int numDecimals, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, QString() );

  char *wkt;
  size_t len = 0;
  sfcgal_geometry_as_text_decim( geom, numDecimals, &wkt, &len );
  CHECK_SUCCESS( errorMsg, QString() );

  std::string wktString( wkt, len );
  sfcgal_free_buffer( wkt );
  return QString::fromStdString( wktString );
}

Qgis::WkbType QgsSfcgalEngine::wkbType( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, Qgis::WkbType::Unknown );

  sfcgal_geometry_type_t type = sfcgal_geometry_type_id( geom );
  CHECK_SUCCESS( errorMsg, Qgis::WkbType::Unknown );

  int wkbType = type;
  if ( sfcgal_geometry_is_3d( geom ) )
    wkbType += 1000;

  if ( sfcgal_geometry_is_measured( geom ) )
    wkbType += 2000;

  Qgis::WkbType qgisType = static_cast<Qgis::WkbType>( wkbType );
  if ( qgisType >= Qgis::WkbType::Unknown && qgisType <= Qgis::WkbType::TriangleZM )
    return qgisType;

  sfcgal::errorHandler()->addText( QStringLiteral( "WKB type '%1' is not known from QGIS" ).arg( wkbType ), //
                                   __FILE__, __FUNCTION__, __LINE__ );
  return Qgis::WkbType::Unknown;
}

int QgsSfcgalEngine::dimension( const sfcgal::geometry *geom, QString *errorMsg )
{
  int out = lambda_geom_to_prim<int>( sfcgal_geometry_dimension, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<int>::quiet_NaN() );
  return out;
}

bool QgsSfcgalEngine::addZValue( sfcgal::geometry *geom, double zValue, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_force_z( geom, zValue );
}

bool QgsSfcgalEngine::addMValue( sfcgal::geometry *geom, double mValue, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_force_m( geom, mValue );
}

bool QgsSfcgalEngine::dropZValue( sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_drop_z( geom );
}

bool QgsSfcgalEngine::dropMValue( sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_drop_m( geom );
}


void QgsSfcgalEngine::swapXy( sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, void() );

  sfcgal_geometry_swap_xy( geom );
}

bool QgsSfcgalEngine::isEquals( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, double tolerance, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geomA, false );
  CHECK_NOT_NULL( geomB, false );

  bool result = sfcgal_geometry_is_almost_equals( geomA, geomB, tolerance );
  CHECK_SUCCESS( errorMsg, false );

  return result;
}

bool QgsSfcgalEngine::isEmpty( const sfcgal::geometry *geom, QString *errorMsg )
{
  int res = lambda_geom_to_prim<int>( sfcgal_geometry_is_empty, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

bool QgsSfcgalEngine::isValid( const sfcgal::geometry *geom, QString *errorMsg, bool, QgsGeometry *errorLoc )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  bool result = false;
  char *reason;
  sfcgal::geometry *location;
  result = sfcgal_geometry_is_valid_detail( geom, &reason, &location );

  CHECK_SUCCESS( errorMsg, false );

  if ( reason && strlen( reason ) )
    sfcgal::errorHandler()->addText( QString( reason ) );

  if ( location && errorLoc )
  {
    std::unique_ptr<QgsAbstractGeometry> locationGeom = toAbstractGeometry( location, errorMsg );
    CHECK_SUCCESS( errorMsg, false );
    errorLoc->addPartV2( locationGeom.release() );
  }

  return result;
}

bool QgsSfcgalEngine::isSimple( const sfcgal::geometry *geom, QString *errorMsg )
{
  int res = lambda_geom_to_prim<int>( sfcgal_geometry_is_simple, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

sfcgal::shared_geom QgsSfcgalEngine::boundary( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *boundary = sfcgal_geometry_boundary( geom );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( boundary );
}

QgsPoint QgsSfcgalEngine::centroid( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, QgsPoint() );

  const sfcgal::geometry *result = nullptr;
  if ( sfcgal_geometry_is_3d( geom ) )
    result = sfcgal_geometry_centroid_3d( geom );
  else
    result = sfcgal_geometry_centroid( geom );

  CHECK_SUCCESS( errorMsg, QgsPoint() );
  CHECK_NOT_NULL( result, QgsPoint() );

  QgsConstWkbPtr wkbPtr = QgsSfcgalEngine::toWkb( result, errorMsg );
  QgsPoint out;
  out.fromWkb( wkbPtr );

  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::translate( const sfcgal::geometry *geom, const QgsPoint &translation, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result;
  if ( translation.is3D() )
    result = sfcgal_geometry_translate_3d( geom, translation.x(), translation.y(), translation.z() );
  else
    result = sfcgal_geometry_translate_2d( geom, translation.x(), translation.y() );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::scale( const sfcgal::geometry *geom, const QgsPoint &scaleFactor, const QgsPoint &center, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  const double scaleZFactor = scaleFactor.is3D() ? scaleFactor.z() : 0;
  sfcgal::geometry *result;
  if ( center.isEmpty() )
  {
    result = sfcgal_geometry_scale_3d( geom, scaleFactor.x(), scaleFactor.y(), scaleZFactor );
  }
  else
  {
    const double centerZ = center.is3D() ? center.z() : 0;
    result = sfcgal_geometry_scale_3d_around_center( geom, scaleFactor.x(), scaleFactor.y(), scaleZFactor, center.x(), center.y(), centerZ );
  }

  CHECK_SUCCESS( errorMsg, nullptr );
  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::rotate2D( const sfcgal::geometry *geom, double angle, const QgsPoint &center, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = sfcgal_geometry_rotate_2d( geom, angle, center.x(), center.y() );

  CHECK_SUCCESS( errorMsg, nullptr );
  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::rotate3D( const sfcgal::geometry *geom, double angle, const QgsVector3D &axisVector, const QgsPoint &center, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result;
  if ( center.isEmpty() )
  {
    result = sfcgal_geometry_rotate_3d( geom, angle, axisVector.x(), axisVector.y(), axisVector.z() );
  }
  else
  {
    result = sfcgal_geometry_rotate_3d_around_center( geom, angle, axisVector.x(), axisVector.y(), axisVector.z(), center.x(), center.y(), center.z() );
  }

  CHECK_SUCCESS( errorMsg, nullptr );
  return sfcgal::make_shared_geom( result );
}

double QgsSfcgalEngine::distance( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  double out = lambda_geomgeom_to_prim<double>( sfcgal_geometry_distance, sfcgal_geometry_distance_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

bool QgsSfcgalEngine::distanceWithin( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, double maxdistance, QString *errorMsg )
{
  double dist = QgsSfcgalEngine::distance( geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, false );

  return dist <= maxdistance;
}

double QgsSfcgalEngine::area( const sfcgal::geometry *geom, QString *errorMsg )
{
  double out = lambda_geom_to_prim<double>( sfcgal_geometry_area, sfcgal_geometry_area_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

double QgsSfcgalEngine::length( const sfcgal::geometry *geom, QString *errorMsg )
{
  double out = lambda_geom_to_prim<double>( sfcgal_geometry_length, sfcgal_geometry_length_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

bool QgsSfcgalEngine::intersects( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  int res = lambda_geomgeom_to_prim<int>( sfcgal_geometry_intersects, sfcgal_geometry_intersects_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

sfcgal::shared_geom QgsSfcgalEngine::intersection( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg, const QgsGeometryParameters & )
{
  sfcgal::shared_geom out = lambda_geomgeom_to_geom( sfcgal_geometry_intersection, sfcgal_geometry_intersection_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::difference( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg, const QgsGeometryParameters & )
{
  sfcgal::shared_geom out = lambda_geomgeom_to_geom( sfcgal_geometry_difference, sfcgal_geometry_difference_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::combine( const QVector<sfcgal::shared_geom> &geomList, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  sfcgal::geometry *combined = nullptr;
  for ( sfcgal::shared_geom other : geomList )
  {
    if ( !combined )
    {
      combined = other.get();
      continue;
    }

    if ( sfcgal_geometry_is_3d( other.get() ) || sfcgal_geometry_is_3d( combined ) )
      combined = sfcgal_geometry_union_3d( combined, other.get() );
    else
      combined = sfcgal_geometry_union( combined, other.get() );

    if ( !combined )
      sfcgal::errorHandler()->addText( "SFCGAL produced null result." );

    CHECK_SUCCESS( errorMsg, nullptr );
  }

  return sfcgal::make_shared_geom( combined );
}

sfcgal::shared_geom QgsSfcgalEngine::triangulate( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = lambda_geom_to_geom( sfcgal_geometry_triangulate_2dz, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

bool QgsSfcgalEngine::covers( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  int res = lambda_geomgeom_to_prim<int>( sfcgal_geometry_covers, sfcgal_geometry_covers_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

sfcgal::shared_geom QgsSfcgalEngine::envelope( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = lambda_geom_to_geom( sfcgal_geometry_envelope, sfcgal_geometry_envelope_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::convexhull( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = lambda_geom_to_geom( sfcgal_geometry_convexhull, sfcgal_geometry_convexhull_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::offsetCurve( const sfcgal::geometry *geom, double distance, int, Qgis::JoinStyle, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = nullptr;
  result = sfcgal_geometry_offset_polygon( geom, distance );

  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::buffer2D( const sfcgal::geometry *geom, double radius, int segments, Qgis::JoinStyle joinStyle, QString *errorMsg )
{
  if ( joinStyle != Qgis::JoinStyle::Round )
    qWarning() << ( QStringLiteral( "Buffer not implemented for %1! Defaulting to round join." ) );

  return offsetCurve( geom, radius, segments, joinStyle, errorMsg );
}

sfcgal::shared_geom QgsSfcgalEngine::buffer3D( const sfcgal::geometry *geom, double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal_buffer3d_type_t buffer_type;
  switch ( joinStyle3D )
  {
    case Qgis::JoinStyle3D::Flat:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_FLAT;
      break;
    case Qgis::JoinStyle3D::Round:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_ROUND;
      break;
    case Qgis::JoinStyle3D::CylSphere:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_CYLSPHERE;
      break;
  }

  sfcgal::geometry *result = sfcgal_geometry_buffer3d( geom, radius, segments, buffer_type );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::extrude( const sfcgal::geometry *geom, const QgsPoint &extrusion, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  const double zFactor = extrusion.is3D() ? extrusion.z() : 0;
  sfcgal_geometry_t *solid = sfcgal_geometry_extrude( geom, extrusion.x(), extrusion.y(), zFactor );

  CHECK_SUCCESS( errorMsg, nullptr );

  // sfcgal_geometry_extrude returns a SOLID
  // This is not handled by QGIS
  // convert it to a PolyhedralSurface
  sfcgal_geometry_t *polySurface = sfcgal_polyhedral_surface_create();
  for ( unsigned int shellIdx = 0; shellIdx < sfcgal_solid_num_shells( solid ); ++shellIdx )
  {
    const sfcgal_geometry_t *shell = sfcgal_solid_shell_n( solid, shellIdx );
    for ( unsigned int polyIdx = 0; polyIdx < sfcgal_polyhedral_surface_num_patches( shell ); ++polyIdx )
    {
      const sfcgal_geometry_t *patch = sfcgal_polyhedral_surface_patch_n( shell, polyIdx );
      sfcgal_polyhedral_surface_add_patch( polySurface, sfcgal_geometry_clone( patch ) );
    }
  }

  sfcgal_geometry_delete( solid );

  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( polySurface );
}

sfcgal::shared_geom QgsSfcgalEngine::simplify( const sfcgal::geometry *geom, double tolerance, bool preserveTopology, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = sfcgal_geometry_simplify( geom, tolerance, preserveTopology );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}
