/***************************************************************************
                         qgssfcgalengine.cpp
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
#include "qgssfcgalengine.h"

#include <SFCGAL/capi/sfcgal_c.h>
#include <nlohmann/json.hpp>

#include "qgsgeometry.h"
#include "qgsgeometryfactory.h"
#include "qgssfcgalgeometry.h"

// ===================================
// sfcgal namespace
// ===================================

thread_local sfcgal::ErrorHandler sSfcgalErrorHandler;

sfcgal::ErrorHandler *sfcgal::errorHandler()
{
  return &sSfcgalErrorHandler;
}

void sfcgal::GeometryDeleter::operator()( sfcgal::geometry *geom ) const
{
  sfcgal_geometry_delete( geom );
}

sfcgal::shared_geom sfcgal::make_shared_geom( sfcgal::geometry *geom )
{
  return sfcgal::shared_geom( geom, sfcgal::GeometryDeleter() );
}


#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
void sfcgal::PrimitiveDeleter::operator()( sfcgal::primitive *prim ) const
{
  sfcgal_primitive_delete( prim );
}

sfcgal::shared_prim sfcgal::make_shared_prim( sfcgal::primitive *prim )
{
  return sfcgal::shared_prim( prim, sfcgal::PrimitiveDeleter() );
}
#endif

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

  sfcgal::errorHandler()->addText( u"SFCGAL error occurred: %1"_s.arg( buffer ), __FILE__, __FUNCTION__, __LINE__ );

  return static_cast<int>( strlen( buffer ) );
}

int sfcgal::warningCallback( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  sfcgal::errorHandler()->addText( u"SFCGAL warning occurred: %1"_s.arg( buffer ), __FILE__, __FUNCTION__, __LINE__ );

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
  return errorMessages.isEmpty() ? QString() : QString( "Error occurred: " ) + errorMessages.last();
}

QString sfcgal::ErrorHandler::getFullText() const
{
  return errorMessages.isEmpty() ? QString() : getMainText() + "\n\t\t" + errorMessages.join( "\n\t\t" );
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
// QgsSfcgalEngine static functions
// ===================================

/**
 * For functions taking 1 geometry in parameter returning a primitive type
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geom geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
template<typename T>
static T geom_to_primtype(
  T( *func_2d )( const sfcgal_geometry_t * ),
  T( *func_3d )( const sfcgal_geometry_t * ),
  const sfcgal::geometry *geom,
  QString *errorMsg
)
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
 * For functions taking 2 geometries in parameter returning a primitive type
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geomA first geometry to apply func_2d ou func_3d
 * \param geomB second geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
template<typename T>
static T geomgeom_to_primtype(
  T( *func_2d )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
  T( *func_3d )( const sfcgal_geometry_t *, const sfcgal_geometry_t * ),
  const sfcgal::geometry *geomA,
  const sfcgal::geometry *geomB,
  QString *errorMsg
)
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
 * For functions taking 1 geometry in parameter returning a shared ptr to geometry
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geom geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
static sfcgal::shared_geom geom_to_geom(
  sfcgal::func_geom_to_geom func_2d,
  sfcgal::func_geom_to_geom func_3d,
  const sfcgal::geometry *geom,
  QString *errorMsg
)
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
 * For functions taking 2 geometries in parameter returning a shared ptr to geometry
 *
 * \param func_2d mandatory pointer to SFCGAL function
 * \param func_3d optional pointer to SFCGAL function when geom is 3D
 * \param geomA first geometry to apply func_2d ou func_3d
 * \param geomB second geometry to apply func_2d ou func_3d
 * \param errorMsg if defined, will receive errors
 */
static sfcgal::shared_geom geomgeom_to_geom(
  sfcgal::func_geomgeom_to_geom func_2d,
  sfcgal::func_geomgeom_to_geom func_3d,
  const sfcgal::geometry *geomA,
  const sfcgal::geometry *geomB,
  QString *errorMsg
)
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

  return std::make_unique<QgsSfcgalGeometry>( geom );
}

std::unique_ptr<QgsAbstractGeometry> QgsSfcgalEngine::toAbstractGeometry( const sfcgal::geometry *geom, QString *errorMsg )
{
  std::unique_ptr<QgsAbstractGeometry> out( nullptr );
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, out );

  QByteArray wkbArray = QgsSfcgalEngine::toWkb( geom, errorMsg );
  CHECK_SUCCESS( errorMsg, out );

  QgsConstWkbPtr wkbPtr( wkbArray );
  out = QgsGeometryFactory::geomFromWkb( wkbPtr );
  if ( !out )
  {
    Qgis::WkbType sfcgalType = QgsSfcgalEngine::wkbType( geom );
    sfcgal::errorHandler()->addText( u"WKB contains unmanaged geometry type (WKB:%1 / SFCGAL:%2"_s //
                                     .arg( static_cast<int>( wkbPtr.readHeader() ) )                          //
                                     .arg( static_cast<int>( sfcgalType ) ),
                                     __FILE__, __FUNCTION__, __LINE__ );
  }

  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::fromAbstractGeometry( const QgsAbstractGeometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, sfcgal::shared_geom( nullptr ) );

  QByteArray wkbBytes = geom->asWkb();

  sfcgal::geometry *out = sfcgal_io_read_wkb( wkbBytes.data(), wkbBytes.length() );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( out );
}

sfcgal::shared_geom QgsSfcgalEngine::cloneGeometry( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = geom_to_geom( sfcgal_geometry_clone, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

QString QgsSfcgalEngine::geometryType( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "geometryType" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );

  char *typeChar;
  size_t typeLen;
  sfcgal_geometry_type( geom, &typeChar, &typeLen );
  std::string typeStr( typeChar, typeLen );
  sfcgal_free_buffer( typeChar );

  return QString::fromStdString( typeStr );
#endif
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

QByteArray QgsSfcgalEngine::toWkb( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, QByteArray() );

  char *wkbHex;
  size_t len = 0;
  sfcgal_geometry_as_wkb( geom, &wkbHex, &len );
  QByteArray wkbArray( wkbHex, static_cast<int>( len ) );

#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
  sfcgal_free_buffer( wkbHex );
#else
  free( wkbHex );
#endif

  CHECK_SUCCESS( errorMsg, QByteArray() );
  return wkbArray;
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
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
  sfcgal_free_buffer( wkt );
#else
  free( wkt );
#endif
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

  sfcgal::errorHandler()->addText( u"WKB type '%1' is not known from QGIS"_s.arg( wkbType ), //
                                   __FILE__, __FUNCTION__, __LINE__ );
  return Qgis::WkbType::Unknown;
}

int QgsSfcgalEngine::dimension( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "dimension" ) );
#else
  int out = geom_to_primtype<int>( sfcgal_geometry_dimension, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<int>::quiet_NaN() );
  return out;
#endif
}

int QgsSfcgalEngine::partCount( const sfcgal::geometry *geom, QString *errorMsg )
{
  size_t out;
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, -1 );

  sfcgal_geometry_type_t type = sfcgal_geometry_type_id( geom );
  CHECK_SUCCESS( errorMsg, -1 );

  switch ( type )
  {
    case SFCGAL_TYPE_MULTIPOINT:
    case SFCGAL_TYPE_MULTILINESTRING:
    case SFCGAL_TYPE_MULTIPOLYGON:
    case SFCGAL_TYPE_MULTISOLID:
    case SFCGAL_TYPE_GEOMETRYCOLLECTION:
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
      out = sfcgal_geometry_num_geometries( geom );
#else
      out = sfcgal_geometry_collection_num_geometries( geom );
#endif
      break;
    case SFCGAL_TYPE_POLYGON:
      out = sfcgal_polygon_num_interior_rings( geom ) + 1;
      break;
    case SFCGAL_TYPE_SOLID:
      out = sfcgal_solid_num_shells( geom );
      break;
    case SFCGAL_TYPE_POLYHEDRALSURFACE:
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
      out = sfcgal_polyhedral_surface_num_patches( geom );
#else
      out = sfcgal_polyhedral_surface_num_polygons( geom );
#endif
      break;
    case SFCGAL_TYPE_TRIANGULATEDSURFACE:
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
      out = sfcgal_triangulated_surface_num_patches( geom );
#else
      out = sfcgal_triangulated_surface_num_triangles( geom );
#endif
      break;
    case SFCGAL_TYPE_LINESTRING:
      out = sfcgal_linestring_num_points( geom );
      break;
    case SFCGAL_TYPE_TRIANGLE:
      out = 3;
      break;
    case SFCGAL_TYPE_POINT:
      out = 1;
      break;
    default:
      out = -1;
  }

  CHECK_SUCCESS( errorMsg, -1 );

  return static_cast<int>( out );
}

bool QgsSfcgalEngine::addZValue( sfcgal::geometry *geom, double zValue, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )zValue;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "addZValue" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_force_z( geom, zValue );
#endif
}

bool QgsSfcgalEngine::addMValue( sfcgal::geometry *geom, double mValue, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )mValue;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "addMValue" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_force_m( geom, mValue );
#endif
}

bool QgsSfcgalEngine::dropZValue( sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "dropZValue" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_drop_z( geom );
#endif
}

bool QgsSfcgalEngine::dropMValue( sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "dropMValue" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  return sfcgal_geometry_drop_m( geom );
#endif
}

void QgsSfcgalEngine::swapXy( sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "swapXy" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, void() );

  sfcgal_geometry_swap_xy( geom );
#endif
}

bool QgsSfcgalEngine::isEqual( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, double tolerance, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geomA;
  ( void )geomB;
  ( void )tolerance;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "isEqual" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geomA, false );
  CHECK_NOT_NULL( geomB, false );

  bool result = sfcgal_geometry_is_almost_equals( geomA, geomB, tolerance );
  CHECK_SUCCESS( errorMsg, false );

  return result;
#endif
}

bool QgsSfcgalEngine::isEmpty( const sfcgal::geometry *geom, QString *errorMsg )
{
  int res = geom_to_primtype<int>( sfcgal_geometry_is_empty, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

bool QgsSfcgalEngine::isValid( const sfcgal::geometry *geom, QString *errorMsg, QgsGeometry *errorLoc )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, false );

  bool result = false;
  char *reason;
  sfcgal::geometry *location;
  result = sfcgal_geometry_is_valid_detail( geom, &reason, &location );

  CHECK_SUCCESS( errorMsg, false );

  if ( reason && strlen( reason ) )
  {
    sfcgal::errorHandler()->addText( QString( reason ) );
    free( reason );
  }

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
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Using %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "isSimple" ) );
#else
  int res = geom_to_primtype<int>( sfcgal_geometry_is_simple, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
#endif
}

sfcgal::shared_geom QgsSfcgalEngine::boundary( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "boundary" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *boundary = sfcgal_geometry_boundary( geom );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( boundary );
#endif
}

QgsPoint QgsSfcgalEngine::centroid( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "centroid" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, QgsPoint() );

  const sfcgal::geometry *result = nullptr;
  if ( sfcgal_geometry_is_3d( geom ) )
    result = sfcgal_geometry_centroid_3d( geom );
  else
    result = sfcgal_geometry_centroid( geom );

  CHECK_SUCCESS( errorMsg, QgsPoint() );
  CHECK_NOT_NULL( result, QgsPoint() );

  QByteArray wkbArray = QgsSfcgalEngine::toWkb( result, errorMsg );
  QgsConstWkbPtr wkbPtr( wkbArray );
  QgsPoint out;
  out.fromWkb( wkbPtr );

  return out;
#endif
}

sfcgal::shared_geom QgsSfcgalEngine::translate( const sfcgal::geometry *geom, const QgsVector3D &translation, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )translation;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "translate" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result;
  if ( sfcgal_geometry_is_3d( geom ) )
    result = sfcgal_geometry_translate_3d( geom, translation.x(), translation.y(), translation.z() );
  else
    result = sfcgal_geometry_translate_2d( geom, translation.x(), translation.y() );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
#endif
}

sfcgal::shared_geom QgsSfcgalEngine::scale( const sfcgal::geometry *geom, const QgsVector3D &scaleFactor, const QgsPoint &center, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result;
  if ( center.isEmpty() )
  {
    result = sfcgal_geometry_scale_3d( geom, scaleFactor.x(), scaleFactor.y(), scaleFactor.z() );
  }
  else
  {
    const double centerZ = center.is3D() ? center.z() : 0;
    result = sfcgal_geometry_scale_3d_around_center( geom, scaleFactor.x(), scaleFactor.y(), scaleFactor.z(), center.x(), center.y(), centerZ );
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
  double out = geomgeom_to_primtype<double>( sfcgal_geometry_distance, sfcgal_geometry_distance_3d, geomA, geomB, errorMsg );
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
  double out = geom_to_primtype<double>( sfcgal_geometry_area, sfcgal_geometry_area_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

double QgsSfcgalEngine::length( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "length" ) );
#else
  double out = geom_to_primtype<double>( sfcgal_geometry_length, sfcgal_geometry_length_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
#endif
}

bool QgsSfcgalEngine::intersects( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  int res = geomgeom_to_primtype<int>( sfcgal_geometry_intersects, sfcgal_geometry_intersects_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

sfcgal::shared_geom QgsSfcgalEngine::intersection( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  sfcgal::shared_geom out = geomgeom_to_geom( sfcgal_geometry_intersection, sfcgal_geometry_intersection_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

sfcgal::shared_geom QgsSfcgalEngine::difference( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  sfcgal::shared_geom out = geomgeom_to_geom( sfcgal_geometry_difference, sfcgal_geometry_difference_3d, geomA, geomB, errorMsg );
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
  sfcgal::shared_geom out = geom_to_geom( sfcgal_geometry_triangulate_2dz, nullptr, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
}

bool QgsSfcgalEngine::covers( const sfcgal::geometry *geomA, const sfcgal::geometry *geomB, QString *errorMsg )
{
  int res = geomgeom_to_primtype<int>( sfcgal_geometry_covers, sfcgal_geometry_covers_3d, geomA, geomB, errorMsg );
  CHECK_SUCCESS( errorMsg, false );
  return static_cast<bool>( res );
}

sfcgal::shared_geom QgsSfcgalEngine::envelope( const sfcgal::geometry *geom, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "envelope" ) );
#else
  sfcgal::shared_geom out = geom_to_geom( sfcgal_geometry_envelope, sfcgal_geometry_envelope_3d, geom, errorMsg );
  CHECK_SUCCESS( errorMsg, nullptr );
  return out;
#endif
}

sfcgal::shared_geom QgsSfcgalEngine::convexHull( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::shared_geom out = geom_to_geom( sfcgal_geometry_convexhull, sfcgal_geometry_convexhull_3d, geom, errorMsg );
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
    qWarning() << ( u"Buffer not implemented for %1! Defaulting to round join."_s );

  return offsetCurve( geom, radius, segments, joinStyle, errorMsg );
}

sfcgal::shared_geom QgsSfcgalEngine::buffer3D( const sfcgal::geometry *geom, double radius, int segments, Qgis::JoinStyle3D joinStyle3D, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal_buffer3d_type_t buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_FLAT;
  switch ( joinStyle3D )
  {
    case Qgis::JoinStyle3D::Flat:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_FLAT;
      break;
    case Qgis::JoinStyle3D::Round:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_ROUND;
      break;
    case Qgis::JoinStyle3D::CylindersAndSpheres:
      buffer_type = sfcgal_buffer3d_type_t::SFCGAL_BUFFER3D_CYLSPHERE;
      break;
  }

  sfcgal::geometry *result = sfcgal_geometry_buffer3d( geom, radius, segments, buffer_type );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}

sfcgal::shared_geom QgsSfcgalEngine::extrude( const sfcgal::geometry *geom, const QgsVector3D &extrusion, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal_geometry_t *solid = sfcgal_geometry_extrude( geom, extrusion.x(), extrusion.y(), extrusion.z() );

  CHECK_SUCCESS( errorMsg, nullptr );

  // sfcgal_geometry_extrude returns a SOLID
  // This is not handled by QGIS
  // convert it to a PolyhedralSurface
  sfcgal_geometry_t *polySurface = sfcgal_polyhedral_surface_create();
  for ( unsigned int shellIdx = 0; shellIdx < sfcgal_solid_num_shells( solid ); ++shellIdx )
  {
    const sfcgal_geometry_t *shell = sfcgal_solid_shell_n( solid, shellIdx );
#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 1, 0 )
    for ( unsigned int polyIdx = 0; polyIdx < sfcgal_polyhedral_surface_num_patches( shell ); ++polyIdx )
    {
      const sfcgal_geometry_t *patch = sfcgal_polyhedral_surface_patch_n( shell, polyIdx );
      sfcgal_polyhedral_surface_add_patch( polySurface, sfcgal_geometry_clone( patch ) );
    }
#else
    for ( unsigned int polyIdx = 0; polyIdx < sfcgal_polyhedral_surface_num_polygons( shell ); ++polyIdx )
    {
      const sfcgal_geometry_t *patch = sfcgal_polyhedral_surface_polygon_n( shell, polyIdx );
      sfcgal_polyhedral_surface_add_polygon( polySurface, sfcgal_geometry_clone( patch ) );
    }
#endif
  }

  sfcgal_geometry_delete( solid );

  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( polySurface );
}

sfcgal::shared_geom QgsSfcgalEngine::simplify( const sfcgal::geometry *geom, double tolerance, bool preserveTopology, QString *errorMsg )
{
#if SFCGAL_VERSION < SFCGAL_MAKE_VERSION( 2, 1, 0 )
  ( void )geom;
  ( void )tolerance;
  ( void )preserveTopology;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating %1 requires a QGIS build based on SFCGAL 2.1 or later" ).arg( "boundary" ) );
#else
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = sfcgal_geometry_simplify( geom, tolerance, preserveTopology );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
#endif
}

sfcgal::shared_geom QgsSfcgalEngine::approximateMedialAxis( const sfcgal::geometry *geom, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result = sfcgal_geometry_approximate_medial_axis( geom );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_geom( result );
}


#if SFCGAL_VERSION >= SFCGAL_MAKE_VERSION( 2, 3, 0 )
sfcgal::shared_geom QgsSfcgalEngine::transform( const sfcgal::geometry *geom, const QMatrix4x4 &mat, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( geom, nullptr );

  sfcgal::geometry *result;
  result = sfcgal_geometry_transform( geom, mat.constData() );

  CHECK_SUCCESS( errorMsg, nullptr );
  return sfcgal::make_shared_geom( result );
}

std::unique_ptr<QgsSfcgalGeometry> QgsSfcgalEngine::toSfcgalGeometry( sfcgal::shared_prim &prim, sfcgal::primitiveType type, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim.get(), nullptr );

  return std::make_unique<QgsSfcgalGeometry>( prim, type );
}

sfcgal::shared_prim QgsSfcgalEngine::createCube( double size, QString *errorMsg )
{
  sfcgal::primitive *result = sfcgal_primitive_create( SFCGAL_TYPE_CUBE );
  CHECK_SUCCESS( errorMsg, nullptr );

  sfcgal_primitive_set_parameter_double( result, "size", size );
  CHECK_SUCCESS( errorMsg, nullptr );

  return sfcgal::make_shared_prim( result );
}

sfcgal::shared_geom QgsSfcgalEngine::primitiveAsPolyhedral( const sfcgal::primitive *prim, const QMatrix4x4 &mat, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, nullptr );

  sfcgal::geometry *result = sfcgal_primitive_as_polyhedral_surface( prim );
  CHECK_SUCCESS( errorMsg, nullptr );

  if ( !mat.isIdentity() )
  {
    sfcgal::geometry *result2 = sfcgal_geometry_transform( result, mat.constData() );
    sfcgal_geometry_delete( result );
    result = result2;
    CHECK_SUCCESS( errorMsg, nullptr );
  }

  return sfcgal::make_shared_geom( result );
}

bool QgsSfcgalEngine::primitiveIsEqual( const sfcgal::primitive *primA, const sfcgal::primitive *primB, double tolerance, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( primA, false );
  CHECK_NOT_NULL( primB, false );

  bool result = sfcgal_primitive_is_almost_equals( primA, primB, tolerance );
  CHECK_SUCCESS( errorMsg, false );

  return result;
}

sfcgal::shared_prim QgsSfcgalEngine::primitiveClone( const sfcgal::primitive *prim, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, nullptr );

  sfcgal::primitive *result = sfcgal_primitive_clone( prim );

  CHECK_SUCCESS( errorMsg, nullptr );
  CHECK_NOT_NULL( result, nullptr );

  return sfcgal::make_shared_prim( result );
}

double QgsSfcgalEngine::primitiveArea( const sfcgal::primitive *prim, bool withDiscretization, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, std::numeric_limits<double>::quiet_NaN() );

  double out = sfcgal_primitive_area( prim, withDiscretization );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

double QgsSfcgalEngine::primitiveVolume( const sfcgal::primitive *prim, bool withDiscretization, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, std::numeric_limits<double>::quiet_NaN() );

  double out = sfcgal_primitive_volume( prim, withDiscretization );
  CHECK_SUCCESS( errorMsg, std::numeric_limits<double>::quiet_NaN() );
  return out;
}

void sfcgal::to_json( json &j, const sfcgal::PrimitiveParameterDesc &p )
{
  j["name"] = p.name;
  j["type"] = p.type;

  if ( std::holds_alternative<int>( p.value ) )
  {
    j["value"] = std::get<int>( p.value );
  }
  else if ( std::holds_alternative<double>( p.value ) )
  {
    j["value"] = std::get<double>( p.value );
  }
  else if ( std::holds_alternative<QgsPoint>( p.value ) )
  {
    QgsPoint point = std::get<QgsPoint>( p.value );
    double z = std::numeric_limits<double>::quiet_NaN();
    double m = std::numeric_limits<double>::quiet_NaN();
    if ( point.is3D() )
      z = point.z();
    if ( point.isMeasure() )
      m = point.m();
    j["value"] = std::vector<double> { point.x(), point.y(), z, m };
  }
  else if ( std::holds_alternative<QgsVector3D>( p.value ) )
  {
    QgsVector3D vect = std::get<QgsVector3D>( p.value );
    j["value"] = std::vector<double> { vect.x(), vect.y(), vect.z() };
  }
  else
    throw json::type_error::create( 306, u"Unknown type '%1'."_s.arg( p.type.c_str() ).toStdString(), nullptr );
}

void sfcgal::from_json( const json &j, sfcgal::PrimitiveParameterDesc &p )
{
  j.at( "name" ).get_to( p.name );
  j.at( "type" ).get_to( p.type );
  if ( j.contains( "value" ) )
  {
    json value = j.at( "value" );
    if ( p.type == "int" )
    {
      p.value = value.get<int>();
    }
    else if ( p.type == "double" )
    {
      p.value = value.get<double>();
    }
    else if ( p.type == "point3" )
    {
      std::vector<double> vect;
      vect = value.get<std::vector<double>>();
      QgsPoint point( vect[0], vect[1],                                                         //
                      ( vect.size() > 2 ? vect[2] : std::numeric_limits<double>::quiet_NaN() ), //
                      ( vect.size() > 3 ? vect[3] : std::numeric_limits<double>::quiet_NaN() ) );
      p.value = point;
    }
    else if ( p.type == "vector3" )
    {
      std::vector<double> vect;
      vect = value.get<std::vector<double>>();
      QgsPoint point( vect[0], vect[1],                                                         //
                      ( vect.size() > 2 ? vect[2] : std::numeric_limits<double>::quiet_NaN() ), //
                      ( vect.size() > 3 ? vect[3] : std::numeric_limits<double>::quiet_NaN() ) );
      p.value = point;
    }
    else
      throw json::type_error::create( 306, u"Unknown type '%1'."_s.arg( p.type.c_str() ).toStdString(), nullptr );
  }
}

QVector<sfcgal::PrimitiveParameterDesc> QgsSfcgalEngine::primitiveParameters( const sfcgal::primitive *prim, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, QVector<sfcgal::PrimitiveParameterDesc>() );

  char *jsonChars = nullptr;
  size_t len = 0;
  sfcgal_primitive_parameters( prim, &jsonChars, &len );
  CHECK_SUCCESS( errorMsg, QVector<sfcgal::PrimitiveParameterDesc>() );

  std::string jsonString( jsonChars, len );
  sfcgal_free_buffer( jsonChars );

  QVector<sfcgal::PrimitiveParameterDesc> result;
  try
  {
    const auto jParams = json::parse( jsonString );
    for ( const auto &jParam : jParams )
    {
      result.append( jParam.get<sfcgal::PrimitiveParameterDesc>() );
    }
  }
  catch ( json::exception &e )
  {
    sfcgal::errorHandler()->addText( u"Caught json exception for json: %1. Error: %2"_s.arg( jsonString.c_str() ).arg( e.what() ) );
  }

  return result;
}

QVariant QgsSfcgalEngine::primitiveParameter( const sfcgal::primitive *prim, const QString &name, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, QVariant() );

  char *jsonChars = nullptr;
  size_t len = 0;
  sfcgal_primitive_parameter( prim, name.toStdString().c_str(), &jsonChars, &len );
  CHECK_SUCCESS( errorMsg, QVariant() );

  std::string jsonString( jsonChars, len );
  sfcgal_free_buffer( jsonChars );

  QVariant result;
  try
  {
    const auto jParam = json::parse( jsonString );
    sfcgal::PrimitiveParameterDesc param = jParam.get<sfcgal::PrimitiveParameterDesc>();
    result = QVariant::fromStdVariant( param.value );
  }
  catch ( json::exception &e )
  {
    sfcgal::errorHandler()->addText( u"Caught json exception for json: %1. Error: %2"_s.arg( jsonString.c_str() ).arg( e.what() ) );
  }

  return result;
}

void QgsSfcgalEngine::primitiveSetParameter( sfcgal::primitive *prim, const QString &name, const QVariant &value, QString *errorMsg )
{
  sfcgal::errorHandler()->clearText( errorMsg );
  CHECK_NOT_NULL( prim, void() );

  try
  {
    json jParam;
    sfcgal::PrimitiveParameterDesc paramDesc;
    paramDesc.name = name.toStdString();
    paramDesc.type = value.typeName();
    if ( paramDesc.type == "int" )
      paramDesc.value = value.toInt();
    else if ( paramDesc.type == "double" )
      paramDesc.value = value.toDouble();
    else if ( value.canConvert<QgsPoint>() )
      paramDesc.value = value.value<QgsPoint>();
    else if ( value.canConvert<QgsVector3D>() )
      paramDesc.value = value.value<QgsVector3D>();

    sfcgal::to_json( jParam, paramDesc );
    std::string jsonStr = jParam.dump();
    sfcgal_primitive_set_parameter( prim, name.toStdString().c_str(), jsonStr.c_str() );
    CHECK_SUCCESS( errorMsg, void() );
  }
  catch ( ... )
  {
    sfcgal::errorHandler()->addText( u"Caught json exception"_s );
  }
}

#endif


#endif // #ifdef WITH_SFCGAL
