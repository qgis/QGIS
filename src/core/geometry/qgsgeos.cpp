/***************************************************************************
                        qgsgeos.cpp
  -------------------------------------------------------------------
Date                 : 22 Sept 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeos.h"
#include "qgsabstractgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgslinestring.h"
#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgslogger.h"
#include "qgspolygon.h"
#include "qgsgeometryeditutils.h"
#include "qgspolyhedralsurface.h"
#include "qgsgeometryutils_base.h"
#include <limits>
#include <cstdio>

#define DEFAULT_QUADRANT_SEGMENTS 8

#define CATCH_GEOS(r) \
  catch (GEOSException &) \
  { \
    return r; \
  }

#define CATCH_GEOS_WITH_ERRMSG(r) \
  catch (GEOSException &e) \
  { \
    if ( errorMsg ) \
    { \
      *errorMsg = e.what(); \
    } \
    return r; \
  }

/// @cond PRIVATE

static void throwGEOSException( const char *fmt, ... )
{
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );

  QString message = QString::fromUtf8( buffer );

#ifdef _MSC_VER
  // stupid stupid MSVC, *SOMETIMES* raises it's own exception if we throw GEOSException, resulting in a crash!
  // see https://github.com/qgis/QGIS/issues/22709
  // if you want to test alternative fixes for this, run the testqgsexpression.cpp test suite - that will crash
  // and burn on the "line_interpolate_point point" test if a GEOSException is thrown.
  // TODO - find a real fix for the underlying issue
  try
  {
    throw GEOSException( message );
  }
  catch ( ... )
  {
    // oops, msvc threw an exception when we tried to throw the exception!
    // just throw nothing instead (except your mouse at your monitor)
  }
#else
  throw GEOSException( message );
#endif
}


static void printGEOSNotice( const char *fmt, ... )
{
#if defined(QGISDEBUG)
  va_list ap;
  char buffer[1024];

  va_start( ap, fmt );
  vsnprintf( buffer, sizeof buffer, fmt, ap );
  va_end( ap );
#else
  Q_UNUSED( fmt )
#endif
}

//
// QgsGeosContext
//

#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
thread_local QgsGeosContext QgsGeosContext::sGeosContext;
#else
QThreadStorage< QgsGeosContext * > QgsGeosContext::sGeosContext;
#endif


QgsGeosContext::QgsGeosContext()
{
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=5 )
  mContext = GEOS_init_r();
  GEOSContext_setNoticeHandler_r( mContext, printGEOSNotice );
  GEOSContext_setErrorHandler_r( mContext, throwGEOSException );
#else
  mContext = initGEOS_r( printGEOSNotice, throwGEOSException );
#endif
}

QgsGeosContext::~QgsGeosContext()
{
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=5 )
  GEOS_finish_r( mContext );
#else
  finishGEOS_r( mContext );
#endif
}

GEOSContextHandle_t QgsGeosContext::get()
{
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
  return sGeosContext.mContext;
#else
  GEOSContextHandle_t gContext = nullptr;
  if ( sGeosContext.hasLocalData() )
  {
    gContext = sGeosContext.localData()->mContext;
  }
  else
  {
    sGeosContext.setLocalData( new QgsGeosContext() );
    gContext = sGeosContext.localData()->mContext;
  }
  return gContext;
#endif
}

//
// geos
//

void geos::GeosDeleter::operator()( GEOSGeometry *geom ) const
{
  GEOSGeom_destroy_r( QgsGeosContext::get(), geom );
}

void geos::GeosDeleter::operator()( const GEOSPreparedGeometry *geom ) const
{
  GEOSPreparedGeom_destroy_r( QgsGeosContext::get(), geom );
}

void geos::GeosDeleter::operator()( GEOSBufferParams *params ) const
{
  GEOSBufferParams_destroy_r( QgsGeosContext::get(), params );
}

void geos::GeosDeleter::operator()( GEOSCoordSequence *sequence ) const
{
  GEOSCoordSeq_destroy_r( QgsGeosContext::get(), sequence );
}


///@endcond


QgsGeos::QgsGeos( const QgsAbstractGeometry *geometry, double precision, Qgis::GeosCreationFlags flags )
  : QgsGeometryEngine( geometry )
  , mGeos( nullptr )
  , mPrecision( precision )
{
  cacheGeos( flags );
}

QgsGeometry QgsGeos::geometryFromGeos( GEOSGeometry *geos )
{
  QgsGeometry g( QgsGeos::fromGeos( geos ) );
  GEOSGeom_destroy_r( QgsGeosContext::get(), geos );
  return g;
}

QgsGeometry QgsGeos::geometryFromGeos( const geos::unique_ptr &geos )
{
  QgsGeometry g( QgsGeos::fromGeos( geos.get() ) );
  return g;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::makeValid( Qgis::MakeValidMethod method, bool keepCollapsed, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();

#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
  if ( method != Qgis::MakeValidMethod::Linework )
    throw QgsNotSupportedException( QObject::tr( "The structured method to make geometries valid requires a QGIS build based on GEOS 3.10 or later" ) );

  if ( keepCollapsed )
    throw QgsNotSupportedException( QObject::tr( "The keep collapsed option for making geometries valid requires a QGIS build based on GEOS 3.10 or later" ) );
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSMakeValid_r( context, mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
#else

  GEOSMakeValidParams *params = GEOSMakeValidParams_create_r( context );
  switch ( method )
  {
    case Qgis::MakeValidMethod::Linework:
      GEOSMakeValidParams_setMethod_r( context, params, GEOS_MAKE_VALID_LINEWORK );
      break;

    case Qgis::MakeValidMethod::Structure:
      GEOSMakeValidParams_setMethod_r( context, params, GEOS_MAKE_VALID_STRUCTURE );
      break;
  }

  GEOSMakeValidParams_setKeepCollapsed_r( context,
                                          params,
                                          keepCollapsed ? 1 : 0 );

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSMakeValidWithParams_r( context, mGeos.get(), params ) );
    GEOSMakeValidParams_destroy_r( context, params );
  }
  catch ( GEOSException &e )
  {
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    GEOSMakeValidParams_destroy_r( context, params );
    return nullptr;
  }
#endif

  return fromGeos( geos.get() );
}

geos::unique_ptr QgsGeos::asGeos( const QgsGeometry &geometry, double precision, Qgis::GeosCreationFlags flags )
{
  if ( geometry.isNull() )
  {
    return nullptr;
  }

  return asGeos( geometry.constGet(), precision, flags );
}

Qgis::GeometryOperationResult QgsGeos::addPart( QgsGeometry &geometry, GEOSGeometry *newPart )
{
  if ( geometry.isNull() )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }
  if ( !newPart )
  {
    return Qgis::GeometryOperationResult::AddPartNotMultiGeometry;
  }

  std::unique_ptr< QgsAbstractGeometry > geom = fromGeos( newPart );
  return QgsGeometryEditUtils::addPart( geometry.get(), std::move( geom ) );
}

void QgsGeos::geometryChanged()
{
  mGeos.reset();
  mGeosPrepared.reset();
  cacheGeos( Qgis::GeosCreationFlag::RejectOnInvalidSubGeometry );
}

void QgsGeos::prepareGeometry()
{
  if ( mGeosPrepared )
  {
    // Already prepared
    return;
  }
  if ( mGeos )
  {
    mGeosPrepared.reset( GEOSPrepare_r( QgsGeosContext::get(), mGeos.get() ) );
  }
}

void QgsGeos::cacheGeos( Qgis::GeosCreationFlags flags ) const
{
  if ( mGeos )
  {
    // Already cached
    return;
  }
  if ( !mGeometry )
  {
    return;
  }

  mGeos = asGeos( mGeometry, mPrecision, flags );
}

QgsAbstractGeometry *QgsGeos::intersection( const QgsAbstractGeometry *geom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  return overlay( geom, OverlayIntersection, errorMsg, parameters ).release();
}

QgsAbstractGeometry *QgsGeos::difference( const QgsAbstractGeometry *geom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  return overlay( geom, OverlayDifference, errorMsg, parameters ).release();
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::clip( const QgsRectangle &rect, QString *errorMsg ) const
{
  if ( !mGeos || rect.isNull() || rect.isEmpty() )
  {
    return nullptr;
  }

  try
  {
    geos::unique_ptr opGeom( GEOSClipByRect_r( QgsGeosContext::get(), mGeos.get(), rect.xMinimum(), rect.yMinimum(), rect.xMaximum(), rect.yMaximum() ) );
    return fromGeos( opGeom.get() );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return nullptr;
  }
}

void QgsGeos::subdivideRecursive( const GEOSGeometry *currentPart, int maxNodes, int depth, QgsGeometryCollection *parts, const QgsRectangle &clipRect, double gridSize ) const
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  int partType = GEOSGeomTypeId_r( context, currentPart );
  if ( qgsDoubleNear( clipRect.width(), 0.0 ) && qgsDoubleNear( clipRect.height(), 0.0 ) )
  {
    if ( partType == GEOS_POINT )
    {
      parts->addGeometry( fromGeos( currentPart ).release() );
      return;
    }
    else
    {
      return;
    }
  }

  if ( partType == GEOS_MULTILINESTRING || partType == GEOS_MULTIPOLYGON || partType == GEOS_GEOMETRYCOLLECTION )
  {
    int partCount = GEOSGetNumGeometries_r( context, currentPart );
    for ( int i = 0; i < partCount; ++i )
    {
      subdivideRecursive( GEOSGetGeometryN_r( context, currentPart, i ), maxNodes, depth, parts, clipRect, gridSize );
    }
    return;
  }

  if ( depth > 50 )
  {
    parts->addGeometry( fromGeos( currentPart ).release() );
    return;
  }

  int vertexCount = GEOSGetNumCoordinates_r( context, currentPart );
  if ( vertexCount == 0 )
  {
    return;
  }
  else if ( vertexCount < maxNodes )
  {
    parts->addGeometry( fromGeos( currentPart ).release() );
    return;
  }

  // chop clipping rect in half by longest side
  double width = clipRect.width();
  double height = clipRect.height();
  QgsRectangle halfClipRect1 = clipRect;
  QgsRectangle halfClipRect2 = clipRect;
  if ( width > height )
  {
    halfClipRect1.setXMaximum( clipRect.xMinimum() + width / 2.0 );
    halfClipRect2.setXMinimum( halfClipRect1.xMaximum() );
  }
  else
  {
    halfClipRect1.setYMaximum( clipRect.yMinimum() + height / 2.0 );
    halfClipRect2.setYMinimum( halfClipRect1.yMaximum() );
  }

  if ( height <= 0 )
  {
    halfClipRect1.setYMinimum( halfClipRect1.yMinimum() - std::numeric_limits<double>::epsilon() );
    halfClipRect2.setYMinimum( halfClipRect2.yMinimum() - std::numeric_limits<double>::epsilon() );
    halfClipRect1.setYMaximum( halfClipRect1.yMaximum() + std::numeric_limits<double>::epsilon() );
    halfClipRect2.setYMaximum( halfClipRect2.yMaximum() + std::numeric_limits<double>::epsilon() );
  }
  if ( width <= 0 )
  {
    halfClipRect1.setXMinimum( halfClipRect1.xMinimum() - std::numeric_limits<double>::epsilon() );
    halfClipRect2.setXMinimum( halfClipRect2.xMinimum() - std::numeric_limits<double>::epsilon() );
    halfClipRect1.setXMaximum( halfClipRect1.xMaximum() + std::numeric_limits<double>::epsilon() );
    halfClipRect2.setXMaximum( halfClipRect2.xMaximum() + std::numeric_limits<double>::epsilon() );
  }

  geos::unique_ptr clipPart1( GEOSClipByRect_r( context, currentPart, halfClipRect1.xMinimum(), halfClipRect1.yMinimum(), halfClipRect1.xMaximum(), halfClipRect1.yMaximum() ) );
  geos::unique_ptr clipPart2( GEOSClipByRect_r( context, currentPart, halfClipRect2.xMinimum(), halfClipRect2.yMinimum(), halfClipRect2.xMaximum(), halfClipRect2.yMaximum() ) );

  ++depth;

  if ( clipPart1 )
  {
    if ( gridSize > 0 )
    {
      clipPart1.reset( GEOSIntersectionPrec_r( context, mGeos.get(), clipPart1.get(), gridSize ) );
    }
    subdivideRecursive( clipPart1.get(), maxNodes, depth, parts, halfClipRect1, gridSize );
  }
  if ( clipPart2 )
  {
    if ( gridSize > 0 )
    {
      clipPart2.reset( GEOSIntersectionPrec_r( context, mGeos.get(), clipPart2.get(), gridSize ) );
    }
    subdivideRecursive( clipPart2.get(), maxNodes, depth, parts, halfClipRect2, gridSize );
  }
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::subdivide( int maxNodes, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  // minimum allowed max is 8
  maxNodes = std::max( maxNodes, 8 );

  std::unique_ptr< QgsGeometryCollection > parts = QgsGeometryFactory::createCollectionOfType( mGeometry->wkbType() );
  try
  {
    subdivideRecursive( mGeos.get(), maxNodes, 0, parts.get(), mGeometry->boundingBox(), parameters.gridSize() );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )

  return std::move( parts );
}

QgsAbstractGeometry *QgsGeos::combine( const QgsAbstractGeometry *geom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  return overlay( geom, OverlayUnion, errorMsg, parameters ).release();
}

QgsAbstractGeometry *QgsGeos::combine( const QVector<QgsAbstractGeometry *> &geomList, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  std::vector<geos::unique_ptr> geosGeometries;
  geosGeometries.reserve( geomList.size() );
  for ( const QgsAbstractGeometry *g : geomList )
  {
    if ( !g )
      continue;

    geosGeometries.emplace_back( asGeos( g, mPrecision ) );
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr geomUnion;
  try
  {
    geos::unique_ptr geomCollection = createGeosCollection( GEOS_GEOMETRYCOLLECTION, geosGeometries );
    if ( parameters.gridSize() > 0 )
    {
      geomUnion.reset( GEOSUnaryUnionPrec_r( context, geomCollection.get(), parameters.gridSize() ) );
    }
    else
    {
      geomUnion.reset( GEOSUnaryUnion_r( context, geomCollection.get() ) );
    }
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )

  std::unique_ptr< QgsAbstractGeometry > result = fromGeos( geomUnion.get() );
  return result.release();
}

QgsAbstractGeometry *QgsGeos::combine( const QVector<QgsGeometry> &geomList, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  std::vector<geos::unique_ptr> geosGeometries;
  geosGeometries.reserve( geomList.size() );
  for ( const QgsGeometry &g : geomList )
  {
    if ( g.isNull() )
      continue;

    geosGeometries.emplace_back( asGeos( g.constGet(), mPrecision ) );
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr geomUnion;
  try
  {
    geos::unique_ptr geomCollection = createGeosCollection( GEOS_GEOMETRYCOLLECTION, geosGeometries );

    if ( parameters.gridSize() > 0 )
    {
      geomUnion.reset( GEOSUnaryUnionPrec_r( context, geomCollection.get(), parameters.gridSize() ) );
    }
    else
    {
      geomUnion.reset( GEOSUnaryUnion_r( context, geomCollection.get() ) );
    }

  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )

  std::unique_ptr< QgsAbstractGeometry > result = fromGeos( geomUnion.get() );
  return result.release();
}

QgsAbstractGeometry *QgsGeos::symDifference( const QgsAbstractGeometry *geom, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  return overlay( geom, OverlaySymDifference, errorMsg, parameters ).release();
}

static bool isZVerticalLine( const QgsAbstractGeometry *geom, double tolerance = 4 * std::numeric_limits<double>::epsilon() )
{
  // checks if the Geometry if a purely vertical 3D line LineString Z((X Y Z1, X Y Z2, ..., X Y Zn))
  // This is needed because QgsGeos is not able to handle this type of geometry on distance computation.

  if ( geom->wkbType() != Qgis::WkbType::LineStringZ && geom->wkbType() != Qgis::WkbType::LineStringZM )
  {
    return false;
  }

  bool isVertical = true;
  if ( const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( geom ) )
  {
    const int nrPoints = line->numPoints();
    if ( nrPoints == 1 )
    {
      return true;
    }

    // if the 2D part of two points of the line are different, this means
    // that the line is not purely vertical
    const double sqrTolerance = tolerance * tolerance;
    const double *lineX = line->xData();
    const double *lineY = line->yData();
    for ( int iVert = nrPoints - 1, jVert = 0; jVert < nrPoints; iVert = jVert++ )
    {
      if ( QgsGeometryUtilsBase::sqrDistance2D( lineX[iVert], lineY[iVert], lineX[jVert], lineY[jVert] ) > sqrTolerance )
      {
        isVertical = false;
        break;
      }
    }
  }

  return isVertical;
}

double QgsGeos::distance( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr otherGeosGeom;

  // GEOSPreparedDistance_r is not able to properly compute the distance if one
  // of the geometries if a vertical line (LineString Z((X Y Z1, X Y Z2, ..., X Y Zn))).
  // In that case, replace `geom` by a single point.
  // However, GEOSDistance_r works.
  if ( mGeosPrepared && isZVerticalLine( geom->simplifiedTypeRef() ) )
  {
    QgsPoint firstPoint = geom->vertexAt( QgsVertexId( 0, 0, 0 ) );
    otherGeosGeom = asGeos( &firstPoint, mPrecision );
  }
  else
  {
    otherGeosGeom = asGeos( geom, mPrecision );
  }

  if ( !otherGeosGeom )
  {
    return distance;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    if ( mGeosPrepared && !isZVerticalLine( mGeometry->simplifiedTypeRef() ) )
    {
      GEOSPreparedDistance_r( context, mGeosPrepared.get(), otherGeosGeom.get(), &distance );
    }
    else
    {
      GEOSDistance_r( context, mGeos.get(), otherGeosGeom.get(), &distance );
    }
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

double QgsGeos::distance( double x, double y, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr point = createGeosPointXY( x, y, false, 0, false, 0, 2, 0 );
  if ( !point )
    return distance;

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    if ( mGeosPrepared )
    {
      GEOSPreparedDistance_r( context, mGeosPrepared.get(), point.get(), &distance );
    }
    else
    {
      GEOSDistance_r( context, mGeos.get(), point.get(), &distance );
    }
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

bool QgsGeos::distanceWithin( const QgsAbstractGeometry *geom, double maxdist, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return false;
  }

  geos::unique_ptr otherGeosGeom;

  // GEOSPreparedDistanceWithin_r GEOSPreparedDistance_r are not able to properly compute the distance if one
  // of the geometries if a vertical line (LineString Z((X Y Z1, X Y Z2, ..., X Y Zn))).
  // In that case, replace `geom` by a single point.
  // However, GEOSDistanceWithin_r and GEOSDistance_r work.
  if ( mGeosPrepared && isZVerticalLine( geom->simplifiedTypeRef() ) )
  {
    QgsPoint firstPoint = geom->vertexAt( QgsVertexId( 0, 0, 0 ) );
    otherGeosGeom = asGeos( &firstPoint );
  }
  else
  {
    otherGeosGeom = asGeos( geom, mPrecision );
  }

  if ( !otherGeosGeom )
  {
    return false;
  }

  // TODO: optimize implementation of this function to early-exit if
  // any part of othergeosGeom is found to be within the given
  // distance
  double distance;

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    if ( mGeosPrepared && !isZVerticalLine( mGeometry->simplifiedTypeRef() ) )
    {
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
      return GEOSPreparedDistanceWithin_r( context, mGeosPrepared.get(), otherGeosGeom.get(), maxdist );
#else
      GEOSPreparedDistance_r( context, mGeosPrepared.get(), otherGeosGeom.get(), &distance );
#endif
    }
    else
    {
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
      return GEOSDistanceWithin_r( context, mGeos.get(), otherGeosGeom.get(), maxdist );
#else
      GEOSDistance_r( context, mGeos.get(), otherGeosGeom.get(), &distance );
#endif
    }
  }
  CATCH_GEOS_WITH_ERRMSG( false )

  return distance <= maxdist;
}

bool QgsGeos::contains( double x, double y, QString *errorMsg ) const
{
  bool result = false;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=12 )
    // defer point creation until after prepared geometry check, we may not need it
#else
    geos::unique_ptr point = createGeosPointXY( x, y, false, 0, false, 0, 2, 0 );
    if ( !point )
      return false;
#endif
    if ( mGeosPrepared ) //use faster version with prepared geometry
    {
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=12 )
      return GEOSPreparedContainsXY_r( context, mGeosPrepared.get(), x, y ) == 1;
#else
      return GEOSPreparedContains_r( context, mGeosPrepared.get(), point.get() ) == 1;
#endif
    }

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=12 )
    geos::unique_ptr point = createGeosPointXY( x, y, false, 0, false, 0, 2, 0 );
    if ( !point )
      return false;
#endif

    result = ( GEOSContains_r( context, mGeos.get(), point.get() ) == 1 );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return false;
  }

  return result;
}

double QgsGeos::hausdorffDistance( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr otherGeosGeom( asGeos( geom, mPrecision ) );
  if ( !otherGeosGeom )
  {
    return distance;
  }

  try
  {
    GEOSHausdorffDistance_r( QgsGeosContext::get(), mGeos.get(), otherGeosGeom.get(), &distance );
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

double QgsGeos::hausdorffDistanceDensify( const QgsAbstractGeometry *geom, double densifyFraction, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr otherGeosGeom( asGeos( geom, mPrecision ) );
  if ( !otherGeosGeom )
  {
    return distance;
  }

  try
  {
    GEOSHausdorffDistanceDensify_r( QgsGeosContext::get(), mGeos.get(), otherGeosGeom.get(), densifyFraction, &distance );
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

double QgsGeos::frechetDistance( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr otherGeosGeom( asGeos( geom, mPrecision ) );
  if ( !otherGeosGeom )
  {
    return distance;
  }

  try
  {
    GEOSFrechetDistance_r( QgsGeosContext::get(), mGeos.get(), otherGeosGeom.get(), &distance );
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

double QgsGeos::frechetDistanceDensify( const QgsAbstractGeometry *geom, double densifyFraction, QString *errorMsg ) const
{
  double distance = -1.0;
  if ( !mGeos )
  {
    return distance;
  }

  geos::unique_ptr otherGeosGeom( asGeos( geom, mPrecision ) );
  if ( !otherGeosGeom )
  {
    return distance;
  }

  try
  {
    GEOSFrechetDistanceDensify_r( QgsGeosContext::get(), mGeos.get(), otherGeosGeom.get(), densifyFraction, &distance );
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )

  return distance;
}

bool QgsGeos::intersects( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  if ( !mGeos || !geom )
  {
    return false;
  }

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=12 )
  // special optimised case for point intersects
  if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom->simplifiedTypeRef() ) )
  {
    if ( mGeosPrepared )
    {
      try
      {
        return GEOSPreparedIntersectsXY_r( QgsGeosContext::get(), mGeosPrepared.get(), point->x(), point->y() ) == 1;
      }
      catch ( GEOSException &e )
      {
        logError( QStringLiteral( "GEOS" ), e.what() );
        if ( errorMsg )
        {
          *errorMsg = e.what();
        }
        return false;
      }
    }
  }
#endif

  return relation( geom, RelationIntersects, errorMsg );
}

bool QgsGeos::touches( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  return relation( geom, RelationTouches, errorMsg );
}

bool QgsGeos::crosses( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  return relation( geom, RelationCrosses, errorMsg );
}

bool QgsGeos::within( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  return relation( geom, RelationWithin, errorMsg );
}

bool QgsGeos::overlaps( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  return relation( geom, RelationOverlaps, errorMsg );
}

bool QgsGeos::contains( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  if ( !mGeos || !geom )
  {
    return false;
  }

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=12 )
  // special optimised case for point containment
  if ( const QgsPoint *point = qgsgeometry_cast< const QgsPoint * >( geom->simplifiedTypeRef() ) )
  {
    if ( mGeosPrepared )
    {
      try
      {
        return GEOSPreparedContainsXY_r( QgsGeosContext::get(), mGeosPrepared.get(), point->x(), point->y() ) == 1;
      }
      catch ( GEOSException &e )
      {
        logError( QStringLiteral( "GEOS" ), e.what() );
        if ( errorMsg )
        {
          *errorMsg = e.what();
        }
        return false;
      }
    }
  }
#endif

  return relation( geom, RelationContains, errorMsg );
}

bool QgsGeos::disjoint( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  return relation( geom, RelationDisjoint, errorMsg );
}

QString QgsGeos::relate( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return QString();
  }

  geos::unique_ptr geosGeom( asGeos( geom, mPrecision ) );
  if ( !geosGeom )
  {
    return QString();
  }

  QString result;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    char *r = GEOSRelate_r( context, mGeos.get(), geosGeom.get() );
    if ( r )
    {
      result = QString( r );
      GEOSFree_r( context, r );
    }
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
  }

  return result;
}

bool QgsGeos::relatePattern( const QgsAbstractGeometry *geom, const QString &pattern, QString *errorMsg ) const
{
  if ( !mGeos || !geom )
  {
    return false;
  }

  geos::unique_ptr geosGeom( asGeos( geom, mPrecision ) );
  if ( !geosGeom )
  {
    return false;
  }

  bool result = false;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    result = ( GEOSRelatePattern_r( context, mGeos.get(), geosGeom.get(), pattern.toLocal8Bit().constData() ) == 1 );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
  }

  return result;
}

double QgsGeos::area( QString *errorMsg ) const
{
  double area = -1.0;
  if ( !mGeos )
  {
    return area;
  }

  try
  {
    if ( GEOSArea_r( QgsGeosContext::get(), mGeos.get(), &area ) != 1 )
      return -1.0;
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )
  return area;
}

double QgsGeos::length( QString *errorMsg ) const
{
  double length = -1.0;
  if ( !mGeos )
  {
    return length;
  }
  try
  {
    if ( GEOSLength_r( QgsGeosContext::get(), mGeos.get(), &length ) != 1 )
      return -1.0;
  }
  CATCH_GEOS_WITH_ERRMSG( -1.0 )
  return length;
}

QgsGeometryEngine::EngineOperationResult QgsGeos::splitGeometry( const QgsLineString &splitLine,
    QVector<QgsGeometry> &newGeometries,
    bool topological,
    QgsPointSequence &topologyTestPoints,
    QString *errorMsg, bool skipIntersectionCheck ) const
{

  EngineOperationResult returnCode = Success;
  if ( !mGeos || !mGeometry )
  {
    return InvalidBaseGeometry;
  }

  //return if this type is point/multipoint
  if ( mGeometry->dimension() == 0 )
  {
    return SplitCannotSplitPoint; //cannot split points
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  if ( !GEOSisValid_r( context, mGeos.get() ) )
    return InvalidBaseGeometry;

  //make sure splitLine is valid
  if ( ( mGeometry->dimension() == 1 && splitLine.numPoints() < 1 ) ||
       ( mGeometry->dimension() == 2 && splitLine.numPoints() < 2 ) )
    return InvalidInput;

  newGeometries.clear();
  geos::unique_ptr splitLineGeos;

  try
  {
    if ( splitLine.numPoints() > 1 )
    {
      splitLineGeos = createGeosLinestring( &splitLine, mPrecision );
    }
    else if ( splitLine.numPoints() == 1 )
    {
      splitLineGeos = createGeosPointXY( splitLine.xAt( 0 ), splitLine.yAt( 0 ), false, 0, false, 0, 2, mPrecision );
    }
    else
    {
      return InvalidInput;
    }

    if ( !GEOSisValid_r( context, splitLineGeos.get() ) || !GEOSisSimple_r( context, splitLineGeos.get() ) )
    {
      return InvalidInput;
    }

    if ( topological )
    {
      //find out candidate points for topological corrections
      if ( !topologicalTestPointsSplit( splitLineGeos.get(), topologyTestPoints ) )
      {
        return InvalidInput; // TODO: is it really an invalid input?
      }
    }

    //call split function depending on geometry type
    if ( mGeometry->dimension() == 1 )
    {
      returnCode = splitLinearGeometry( splitLineGeos.get(), newGeometries, skipIntersectionCheck );
    }
    else if ( mGeometry->dimension() == 2 )
    {
      returnCode = splitPolygonGeometry( splitLineGeos.get(), newGeometries, skipIntersectionCheck );
    }
    else
    {
      return InvalidInput;
    }
  }
  CATCH_GEOS_WITH_ERRMSG( EngineError )

  return returnCode;
}



bool QgsGeos::topologicalTestPointsSplit( const GEOSGeometry *splitLine, QgsPointSequence &testPoints, QString *errorMsg ) const
{
  //Find out the intersection points between splitLineGeos and this geometry.
  //These points need to be tested for topological correctness by the calling function
  //if topological editing is enabled

  if ( !mGeos )
  {
    return false;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    testPoints.clear();
    geos::unique_ptr intersectionGeom( GEOSIntersection_r( context, mGeos.get(), splitLine ) );
    if ( !intersectionGeom )
      return false;

    bool simple = false;
    int nIntersectGeoms = 1;
    if ( GEOSGeomTypeId_r( context, intersectionGeom.get() ) == GEOS_LINESTRING
         || GEOSGeomTypeId_r( context, intersectionGeom.get() ) == GEOS_POINT )
      simple = true;

    if ( !simple )
      nIntersectGeoms = GEOSGetNumGeometries_r( context, intersectionGeom.get() );

    for ( int i = 0; i < nIntersectGeoms; ++i )
    {
      const GEOSGeometry *currentIntersectGeom = nullptr;
      if ( simple )
        currentIntersectGeom = intersectionGeom.get();
      else
        currentIntersectGeom = GEOSGetGeometryN_r( context, intersectionGeom.get(), i );

      const GEOSCoordSequence *lineSequence = GEOSGeom_getCoordSeq_r( context, currentIntersectGeom );
      unsigned int sequenceSize = 0;
      double x, y, z;
      if ( GEOSCoordSeq_getSize_r( context, lineSequence, &sequenceSize ) != 0 )
      {
        for ( unsigned int i = 0; i < sequenceSize; ++i )
        {
          if ( GEOSCoordSeq_getXYZ_r( context, lineSequence, i, &x, &y, &z ) )
          {
            testPoints.push_back( QgsPoint( x, y, z ) );
          }
        }
      }
    }
  }
  CATCH_GEOS_WITH_ERRMSG( true )

  return true;
}

geos::unique_ptr QgsGeos::linePointDifference( GEOSGeometry *GEOSsplitPoint ) const
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  int type = GEOSGeomTypeId_r( context, mGeos.get() );

  std::unique_ptr< QgsMultiCurve > multiCurve;
  if ( type == GEOS_MULTILINESTRING )
  {
    multiCurve.reset( qgsgeometry_cast<QgsMultiCurve *>( mGeometry->clone() ) );
  }
  else if ( type == GEOS_LINESTRING )
  {
    multiCurve.reset( new QgsMultiCurve() );
    multiCurve->addGeometry( mGeometry->clone() );
  }
  else
  {
    return nullptr;
  }

  if ( !multiCurve )
  {
    return nullptr;
  }


  // we might have a point or a multipoint, depending on number of
  // intersections between the geometry and the split geometry
  std::unique_ptr< QgsMultiPoint > splitPoints;
  {
    std::unique_ptr< QgsAbstractGeometry > splitGeom( fromGeos( GEOSsplitPoint ) );

    if ( qgsgeometry_cast<QgsMultiPoint *>( splitGeom.get() ) )
    {
      splitPoints.reset( qgsgeometry_cast<QgsMultiPoint *>( splitGeom.release() ) );
    }
    else if ( qgsgeometry_cast<QgsPoint *>( splitGeom.get() ) )
    {
      splitPoints = std::make_unique< QgsMultiPoint >();
      if ( qgsgeometry_cast<QgsPoint *>( splitGeom.get() ) )
      {
        splitPoints->addGeometry( qgsgeometry_cast<QgsPoint *>( splitGeom.release() ) );
      }
    }
  }

  QgsMultiCurve lines;

  //For each part
  for ( int geometryIndex = 0; geometryIndex < multiCurve->numGeometries(); ++geometryIndex )
  {
    const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( multiCurve->geometryN( geometryIndex ) );
    if ( !line )
    {
      const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( multiCurve->geometryN( geometryIndex ) );
      line = curve->curveToLine();
    }
    if ( !line )
    {
      return nullptr;
    }
    // we gather the intersection points and their distance from previous node grouped by segment
    QMap< int, QVector< QPair< double, QgsPoint > > >pointMap;
    for ( int splitPointIndex = 0; splitPointIndex < splitPoints->numGeometries(); ++splitPointIndex )
    {
      const QgsPoint *intersectionPoint = splitPoints->pointN( splitPointIndex );

      QgsPoint segmentPoint2D;
      QgsVertexId nextVertex;
      // With closestSegment we only get a 2D point so we need to interpolate if we
      // don't want to lose Z data
      line->closestSegment( *intersectionPoint, segmentPoint2D, nextVertex );

      // The intersection might belong to another part, skip it
      // Note: cannot test for equality because of Z
      if ( !qgsDoubleNear( intersectionPoint->x(), segmentPoint2D.x() ) || !qgsDoubleNear( intersectionPoint->y(), segmentPoint2D.y() ) )
      {
        continue;
      }

      const QgsLineString segment = QgsLineString( line->pointN( nextVertex.vertex - 1 ), line->pointN( nextVertex.vertex ) );
      const double distance = segmentPoint2D.distance( line->pointN( nextVertex.vertex - 1 ) );

      // Due to precision issues, distance can be a tad larger than the actual segment length, making interpolatePoint() return nullptr
      // In that case we'll use the segment's endpoint instead of interpolating
      std::unique_ptr< QgsPoint > correctSegmentPoint( distance > segment.length() ? segment.endPoint().clone() : segment.interpolatePoint( distance ) );

      const QPair< double, QgsPoint > pair = qMakePair( distance, *correctSegmentPoint.get() );
      if ( pointMap.contains( nextVertex.vertex - 1 ) )
        pointMap[ nextVertex.vertex - 1 ].append( pair );
      else
        pointMap[ nextVertex.vertex - 1 ] = QVector< QPair< double, QgsPoint > >() << pair;
    }

    // When we have more than one intersection point on a segment we need those points
    // to be sorted by their distance from the previous geometry vertex
    for ( auto &p : pointMap )
    {
      std::sort( p.begin(), p.end(), []( const QPair< double, QgsPoint > &a, const QPair< double, QgsPoint > &b ) { return a.first < b.first; } );
    }

    //For each segment
    QgsLineString newLine;
    int nVertices = line->numPoints();
    QgsPoint splitPoint;
    for ( int vertexIndex = 0; vertexIndex < nVertices; ++vertexIndex )
    {
      QgsPoint currentPoint = line->pointN( vertexIndex );
      newLine.addVertex( currentPoint );
      if ( pointMap.contains( vertexIndex ) )
      {
        // For each intersecting point
        for ( int k = 0; k < pointMap[ vertexIndex ].size(); ++k )
        {
          splitPoint = pointMap[ vertexIndex ][k].second;
          if ( splitPoint == currentPoint )
          {
            lines.addGeometry( newLine.clone() );
            newLine = QgsLineString();
            newLine.addVertex( currentPoint );
          }
          else if ( splitPoint == line->pointN( vertexIndex + 1 ) )
          {
            newLine.addVertex( line->pointN( vertexIndex + 1 ) );
            lines.addGeometry( newLine.clone() );
            newLine = QgsLineString();
          }
          else
          {
            newLine.addVertex( splitPoint );
            lines.addGeometry( newLine.clone() );
            newLine = QgsLineString();
            newLine.addVertex( splitPoint );
          }
        }
      }
    }
    lines.addGeometry( newLine.clone() );
  }

  return asGeos( &lines, mPrecision );
}

QgsGeometryEngine::EngineOperationResult QgsGeos::splitLinearGeometry( const GEOSGeometry *splitLine, QVector<QgsGeometry> &newGeometries, bool skipIntersectionCheck ) const
{
  Q_UNUSED( skipIntersectionCheck )
  if ( !splitLine )
    return InvalidInput;

  if ( !mGeos )
    return InvalidBaseGeometry;

  GEOSContextHandle_t context = QgsGeosContext::get();

  geos::unique_ptr intersectGeom( GEOSIntersection_r( context, splitLine, mGeos.get() ) );
  if ( !intersectGeom || GEOSisEmpty_r( context, intersectGeom.get() ) )
    return NothingHappened;

  //check that split line has no linear intersection
  const int linearIntersect = GEOSRelatePattern_r( context, mGeos.get(), splitLine, "1********" );
  if ( linearIntersect > 0 )
    return InvalidInput;

  geos::unique_ptr splitGeom = linePointDifference( intersectGeom.get() );

  if ( !splitGeom )
    return InvalidBaseGeometry;

  std::vector<geos::unique_ptr> lineGeoms;

  const int splitType = GEOSGeomTypeId_r( context, splitGeom.get() );
  if ( splitType == GEOS_MULTILINESTRING )
  {
    const int nGeoms = GEOSGetNumGeometries_r( context, splitGeom.get() );
    lineGeoms.reserve( nGeoms );
    for ( int i = 0; i < nGeoms; ++i )
      lineGeoms.emplace_back( GEOSGeom_clone_r( context, GEOSGetGeometryN_r( context, splitGeom.get(), i ) ) );

  }
  else
  {
    lineGeoms.emplace_back( GEOSGeom_clone_r( context, splitGeom.get() ) );
  }

  mergeGeometriesMultiTypeSplit( lineGeoms );

  for ( geos::unique_ptr &lineGeom : lineGeoms )
  {
    newGeometries << QgsGeometry( fromGeos( lineGeom.get() ) );
  }

  return Success;
}

QgsGeometryEngine::EngineOperationResult QgsGeos::splitPolygonGeometry( const GEOSGeometry *splitLine, QVector<QgsGeometry> &newGeometries, bool skipIntersectionCheck ) const
{
  if ( !splitLine )
    return InvalidInput;

  if ( !mGeos )
    return InvalidBaseGeometry;

  // we will need prepared geometry for intersection tests
  const_cast<QgsGeos *>( this )->prepareGeometry();
  if ( !mGeosPrepared )
    return EngineError;

  GEOSContextHandle_t context = QgsGeosContext::get();

  //first test if linestring intersects geometry. If not, return straight away
  if ( !skipIntersectionCheck && !GEOSPreparedIntersects_r( context, mGeosPrepared.get(), splitLine ) )
    return NothingHappened;

  //first union all the polygon rings together (to get them noded, see JTS developer guide)
  geos::unique_ptr nodedGeometry = nodeGeometries( splitLine, mGeos.get() );
  if ( !nodedGeometry )
    return NodedGeometryError; //an error occurred during noding

  const GEOSGeometry *noded = nodedGeometry.get();
  geos::unique_ptr polygons( GEOSPolygonize_r( context, &noded, 1 ) );
  if ( !polygons )
  {
    return InvalidBaseGeometry;
  }
  const int numberOfGeometriesPolygon =  numberOfGeometries( polygons.get() );
  if ( numberOfGeometriesPolygon == 0 )
  {
    return InvalidBaseGeometry;
  }

  //test every polygon is contained in original geometry
  //include in result if yes
  std::vector<geos::unique_ptr> testedGeometries;

  // test whether the polygon parts returned from polygonize algorithm actually
  // belong to the source polygon geometry (if the source polygon contains some holes,
  // those would be also returned by polygonize and we need to skip them)
  for ( int i = 0; i < numberOfGeometriesPolygon; i++ )
  {
    const GEOSGeometry *polygon = GEOSGetGeometryN_r( context, polygons.get(), i );

    geos::unique_ptr pointOnSurface( GEOSPointOnSurface_r( context, polygon ) );
    if ( pointOnSurface && GEOSPreparedIntersects_r( context, mGeosPrepared.get(), pointOnSurface.get() ) )
      testedGeometries.emplace_back( GEOSGeom_clone_r( context, polygon ) );
  }

  const size_t nGeometriesThis = numberOfGeometries( mGeos.get() ); //original number of geometries
  if ( testedGeometries.empty() || testedGeometries.size() == nGeometriesThis )
  {
    //no split done, preserve original geometry
    return NothingHappened;
  }

  // For multi-part geometries, try to identify parts that have been unchanged and try to merge them back
  // to a single multi-part geometry. For example, if there's a multi-polygon with three parts, but only
  // one part is being split, this function makes sure that the other two parts will be kept in a multi-part
  // geometry rather than being separated into two single-part geometries.
  mergeGeometriesMultiTypeSplit( testedGeometries );

  size_t i;
  for ( i = 0; i < testedGeometries.size() && GEOSisValid_r( context, testedGeometries[i].get() ); ++i )
    ;

  if ( i < testedGeometries.size() )
  {
    return InvalidBaseGeometry;
  }

  for ( geos::unique_ptr &testedGeometry : testedGeometries )
  {
    newGeometries << QgsGeometry( fromGeos( testedGeometry.get() ) );
  }

  return Success;
}

geos::unique_ptr QgsGeos::nodeGeometries( const GEOSGeometry *splitLine, const GEOSGeometry *geom )
{
  if ( !splitLine || !geom )
    return nullptr;

  geos::unique_ptr geometryBoundary;
  GEOSContextHandle_t context = QgsGeosContext::get();
  if ( GEOSGeomTypeId_r( context, geom ) == GEOS_POLYGON || GEOSGeomTypeId_r( context, geom ) == GEOS_MULTIPOLYGON )
    geometryBoundary.reset( GEOSBoundary_r( context, geom ) );
  else
    geometryBoundary.reset( GEOSGeom_clone_r( context, geom ) );

  geos::unique_ptr splitLineClone( GEOSGeom_clone_r( context, splitLine ) );
  geos::unique_ptr unionGeometry( GEOSUnion_r( context, splitLineClone.get(), geometryBoundary.get() ) );

  return unionGeometry;
}

int QgsGeos::mergeGeometriesMultiTypeSplit( std::vector<geos::unique_ptr> &splitResult ) const
{
  if ( !mGeos )
    return 1;

  //convert mGeos to geometry collection
  GEOSContextHandle_t context = QgsGeosContext::get();
  int type = GEOSGeomTypeId_r( context, mGeos.get() );
  if ( type != GEOS_GEOMETRYCOLLECTION &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_MULTIPOINT )
    return 0;

  //collect all the geometries that belong to the initial multifeature
  std::vector<geos::unique_ptr> unionGeom;

  std::vector<geos::unique_ptr> newSplitResult;

  for ( size_t i = 0; i < splitResult.size(); ++i )
  {
    //is this geometry a part of the original multitype?
    bool isPart = false;
    for ( int j = 0; j < GEOSGetNumGeometries_r( context, mGeos.get() ); j++ )
    {
      if ( GEOSEquals_r( context, splitResult[i].get(), GEOSGetGeometryN_r( context, mGeos.get(), j ) ) )
      {
        isPart = true;
        break;
      }
    }

    if ( isPart )
    {
      unionGeom.emplace_back( std::move( splitResult[i] ) );
    }
    else
    {
      std::vector<geos::unique_ptr> geomVector;
      geomVector.emplace_back( std::move( splitResult[i] ) );

      if ( type == GEOS_MULTILINESTRING )
        newSplitResult.emplace_back( createGeosCollection( GEOS_MULTILINESTRING, geomVector ) );
      else if ( type == GEOS_MULTIPOLYGON )
        newSplitResult.emplace_back( createGeosCollection( GEOS_MULTIPOLYGON, geomVector ) );
    }
  }

  splitResult = std::move( newSplitResult );

  //make multifeature out of unionGeom
  if ( !unionGeom.empty() )
  {
    if ( type == GEOS_MULTILINESTRING )
      splitResult.emplace_back( createGeosCollection( GEOS_MULTILINESTRING, unionGeom ) );
    else if ( type == GEOS_MULTIPOLYGON )
      splitResult.emplace_back( createGeosCollection( GEOS_MULTIPOLYGON, unionGeom ) );
  }

  return 0;
}

geos::unique_ptr QgsGeos::createGeosCollection( int typeId, std::vector<geos::unique_ptr> &geoms )
{
  std::vector<GEOSGeometry *> geomarr;
  geomarr.reserve( geoms.size() );

  GEOSContextHandle_t context = QgsGeosContext::get();
  for ( geos::unique_ptr &geomUniquePtr : geoms )
  {
    if ( geomUniquePtr )
    {
      if ( !GEOSisEmpty_r( context, geomUniquePtr.get() ) )
      {
        // don't add empty parts to a geos collection, it can cause crashes in GEOS
        // transfer ownership of the geometries to GEOSGeom_createCollection_r()
        geomarr.emplace_back( geomUniquePtr.release() );
      }
    }
  }
  geos::unique_ptr geomRes;

  try
  {
    geomRes.reset( GEOSGeom_createCollection_r( context, typeId, geomarr.data(), geomarr.size() ) );
  }
  catch ( GEOSException & )
  {
    for ( GEOSGeometry *geom : geomarr )
    {
      GEOSGeom_destroy_r( context, geom );
    }
  }

  return geomRes;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::fromGeos( const GEOSGeometry *geos )
{
  if ( !geos )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  int nCoordDims = GEOSGeom_getCoordinateDimension_r( context, geos );
  int nDims = GEOSGeom_getDimensions_r( context, geos );
  bool hasZ = ( nCoordDims == 3 );
  bool hasM = ( ( nDims - nCoordDims ) == 1 );

  switch ( GEOSGeomTypeId_r( context, geos ) )
  {
    case GEOS_POINT:                 // a point
    {
      if ( GEOSisEmpty_r( context, geos ) )
        return nullptr;

      const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( context, geos );
      unsigned int nPoints = 0;
      GEOSCoordSeq_getSize_r( context, cs, &nPoints );
      return nPoints > 0 ? std::unique_ptr<QgsAbstractGeometry>( coordSeqPoint( cs, 0, hasZ, hasM ).clone() ) : nullptr;
    }
    case GEOS_LINESTRING:
    {
      return sequenceToLinestring( geos, hasZ, hasM );
    }
    case GEOS_POLYGON:
    {
      return fromGeosPolygon( geos );
    }
    case GEOS_MULTIPOINT:
    {
      std::unique_ptr< QgsMultiPoint > multiPoint( new QgsMultiPoint() );
      int nParts = GEOSGetNumGeometries_r( context, geos );
      multiPoint->reserve( nParts );
      for ( int i = 0; i < nParts; ++i )
      {
        const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( context, GEOSGetGeometryN_r( context, geos, i ) );
        if ( cs )
        {
          unsigned int nPoints = 0;
          GEOSCoordSeq_getSize_r( context, cs, &nPoints );
          if ( nPoints > 0 )
            multiPoint->addGeometry( coordSeqPoint( cs, 0, hasZ, hasM ).clone() );
        }
      }
      return std::move( multiPoint );
    }
    case GEOS_MULTILINESTRING:
    {
      std::unique_ptr< QgsMultiLineString > multiLineString( new QgsMultiLineString() );
      int nParts = GEOSGetNumGeometries_r( context, geos );
      multiLineString->reserve( nParts );
      for ( int i = 0; i < nParts; ++i )
      {
        std::unique_ptr< QgsLineString >line( sequenceToLinestring( GEOSGetGeometryN_r( context, geos, i ), hasZ, hasM ) );
        if ( line )
        {
          multiLineString->addGeometry( line.release() );
        }
      }
      return std::move( multiLineString );
    }
    case GEOS_MULTIPOLYGON:
    {
      std::unique_ptr< QgsMultiPolygon > multiPolygon( new QgsMultiPolygon() );

      int nParts = GEOSGetNumGeometries_r( context, geos );
      multiPolygon->reserve( nParts );
      for ( int i = 0; i < nParts; ++i )
      {
        std::unique_ptr< QgsPolygon > poly = fromGeosPolygon( GEOSGetGeometryN_r( context, geos, i ) );
        if ( poly )
        {
          multiPolygon->addGeometry( poly.release() );
        }
      }
      return std::move( multiPolygon );
    }
    case GEOS_GEOMETRYCOLLECTION:
    {
      std::unique_ptr< QgsGeometryCollection > geomCollection( new QgsGeometryCollection() );
      int nParts = GEOSGetNumGeometries_r( context, geos );
      geomCollection->reserve( nParts );
      for ( int i = 0; i < nParts; ++i )
      {
        std::unique_ptr< QgsAbstractGeometry > geom( fromGeos( GEOSGetGeometryN_r( context, geos, i ) ) );
        if ( geom )
        {
          geomCollection->addGeometry( geom.release() );
        }
      }
      return std::move( geomCollection );
    }
  }
  return nullptr;
}

std::unique_ptr<QgsPolygon> QgsGeos::fromGeosPolygon( const GEOSGeometry *geos )
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  if ( GEOSGeomTypeId_r( context, geos ) != GEOS_POLYGON )
  {
    return nullptr;
  }

  int nCoordDims = GEOSGeom_getCoordinateDimension_r( context, geos );
  int nDims = GEOSGeom_getDimensions_r( context, geos );
  bool hasZ = ( nCoordDims == 3 );
  bool hasM = ( ( nDims - nCoordDims ) == 1 );

  std::unique_ptr< QgsPolygon > polygon( new QgsPolygon() );

  const GEOSGeometry *ring = GEOSGetExteriorRing_r( context, geos );
  if ( ring )
  {
    polygon->setExteriorRing( sequenceToLinestring( ring, hasZ, hasM ).release() );
  }

  QVector<QgsCurve *> interiorRings;
  const int ringCount = GEOSGetNumInteriorRings_r( context, geos );
  interiorRings.reserve( ringCount );
  for ( int i = 0; i < ringCount; ++i )
  {
    ring = GEOSGetInteriorRingN_r( context, geos, i );
    if ( ring )
    {
      interiorRings.push_back( sequenceToLinestring( ring, hasZ, hasM ).release() );
    }
  }
  polygon->setInteriorRings( interiorRings );

  return polygon;
}

std::unique_ptr<QgsLineString> QgsGeos::sequenceToLinestring( const GEOSGeometry *geos, bool hasZ, bool hasM )
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( context, geos );

  unsigned int nPoints;
  GEOSCoordSeq_getSize_r( context, cs, &nPoints );

  QVector< double > xOut( nPoints );
  QVector< double > yOut( nPoints );
  QVector< double > zOut;
  if ( hasZ )
    zOut.resize( nPoints );
  QVector< double > mOut;
  if ( hasM )
    mOut.resize( nPoints );

  double *x = xOut.data();
  double *y = yOut.data();
  double *z = zOut.data();
  double *m = mOut.data();

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
  GEOSCoordSeq_copyToArrays_r( context, cs, x, y, hasZ ? z : nullptr, hasM ? m : nullptr );
#else
  for ( unsigned int i = 0; i < nPoints; ++i )
  {
    if ( hasZ )
      GEOSCoordSeq_getXYZ_r( context, cs, i, x++, y++, z++ );
    else
      GEOSCoordSeq_getXY_r( context, cs, i, x++, y++ );
    if ( hasM )
    {
      GEOSCoordSeq_getOrdinate_r( context, cs, i, 3, m++ );
    }
  }
#endif
  std::unique_ptr< QgsLineString > line( new QgsLineString( xOut, yOut, zOut, mOut ) );
  return line;
}

int QgsGeos::numberOfGeometries( GEOSGeometry *g )
{
  if ( !g )
    return 0;

  GEOSContextHandle_t context = QgsGeosContext::get();
  int geometryType = GEOSGeomTypeId_r( context, g );
  if ( geometryType == GEOS_POINT || geometryType == GEOS_LINESTRING || geometryType == GEOS_LINEARRING
       || geometryType == GEOS_POLYGON )
    return 1;

  //calling GEOSGetNumGeometries is save for multi types and collections also in geos2
  return GEOSGetNumGeometries_r( context, g );
}

QgsPoint QgsGeos::coordSeqPoint( const GEOSCoordSequence *cs, int i, bool hasZ, bool hasM )
{
  if ( !cs )
  {
    return QgsPoint();
  }

  GEOSContextHandle_t context = QgsGeosContext::get();

  double x, y;
  double z = 0;
  double m = 0;
  if ( hasZ )
    GEOSCoordSeq_getXYZ_r( context, cs, i, &x, &y, &z );
  else
    GEOSCoordSeq_getXY_r( context, cs, i, &x, &y );
  if ( hasM )
  {
    GEOSCoordSeq_getOrdinate_r( context, cs, i, 3, &m );
  }

  Qgis::WkbType t = Qgis::WkbType::Point;
  if ( hasZ && hasM )
  {
    t = Qgis::WkbType::PointZM;
  }
  else if ( hasZ )
  {
    t = Qgis::WkbType::PointZ;
  }
  else if ( hasM )
  {
    t = Qgis::WkbType::PointM;
  }
  return QgsPoint( t, x, y, z, m );
}

geos::unique_ptr QgsGeos::asGeos( const QgsAbstractGeometry *geom, double precision, Qgis::GeosCreationFlags flags )
{
  if ( !geom )
    return nullptr;

  int coordDims = 2;
  if ( geom->is3D() )
  {
    ++coordDims;
  }
  if ( geom->isMeasure() )
  {
    ++coordDims;
  }

  if ( QgsWkbTypes::isMultiType( geom->wkbType() )  || QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::GeometryCollection )
  {
    int geosType = GEOS_GEOMETRYCOLLECTION;

    if ( QgsWkbTypes::flatType( geom->wkbType() ) != Qgis::WkbType::GeometryCollection )
    {
      switch ( QgsWkbTypes::geometryType( geom->wkbType() ) )
      {
        case Qgis::GeometryType::Point:
          geosType = GEOS_MULTIPOINT;
          break;

        case Qgis::GeometryType::Line:
          geosType = GEOS_MULTILINESTRING;
          break;

        case Qgis::GeometryType::Polygon:
          geosType = GEOS_MULTIPOLYGON;
          break;

        case Qgis::GeometryType::Unknown:
        case Qgis::GeometryType::Null:
          return nullptr;
      }
    }


    const QgsGeometryCollection *c = qgsgeometry_cast<const QgsGeometryCollection *>( geom );

    if ( !c )
      return nullptr;

    std::vector<geos::unique_ptr> geomVector;
    geomVector.reserve( c->numGeometries() );
    for ( int i = 0; i < c->numGeometries(); ++i )
    {
      geos::unique_ptr geosGeom = asGeos( c->geometryN( i ), precision, flags );
      if ( flags & Qgis::GeosCreationFlag::RejectOnInvalidSubGeometry && !geosGeom )
      {
        return nullptr;
      }
      geomVector.emplace_back( std::move( geosGeom ) );
    }
    return createGeosCollection( geosType, geomVector );
  }
  else if ( QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::PolyhedralSurface  ||
            QgsWkbTypes::flatType( geom->wkbType() ) == Qgis::WkbType::TIN )
  {
    // PolyhedralSurface and TIN support
    // convert it to a geos MultiPolygon
    const QgsPolyhedralSurface *polyhedralSurface = qgsgeometry_cast<const QgsPolyhedralSurface *>( geom );
    if ( !polyhedralSurface )
      return nullptr;

    std::vector<geos::unique_ptr> geomVector;
    geomVector.reserve( polyhedralSurface->numPatches() );
    for ( int i = 0; i < polyhedralSurface->numPatches(); ++i )
    {
      geos::unique_ptr geosPolygon = createGeosPolygon( polyhedralSurface->patchN( i ), precision );
      if ( flags & Qgis::GeosCreationFlag::RejectOnInvalidSubGeometry && !geosPolygon )
      {
        return nullptr;
      }
      geomVector.emplace_back( std::move( geosPolygon ) );
    }

    return createGeosCollection( GEOS_MULTIPOLYGON, geomVector );
  }
  else
  {
    switch ( QgsWkbTypes::geometryType( geom->wkbType() ) )
    {
      case Qgis::GeometryType::Point:
        return createGeosPoint( static_cast<const QgsPoint *>( geom ), coordDims, precision, flags );

      case Qgis::GeometryType::Line:
        return createGeosLinestring( static_cast<const QgsLineString *>( geom ), precision, flags );

      case Qgis::GeometryType::Polygon:
        return createGeosPolygon( static_cast<const QgsPolygon *>( geom ), precision, flags );

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        return nullptr;
    }
  }
  return nullptr;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::overlay( const QgsAbstractGeometry *geom, Overlay op, QString *errorMsg, const QgsGeometryParameters &parameters ) const
{
  if ( !mGeos || !geom )
  {
    return nullptr;
  }

  geos::unique_ptr geosGeom( asGeos( geom, mPrecision ) );
  if ( !geosGeom )
  {
    return nullptr;
  }

  const double gridSize = parameters.gridSize();

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos::unique_ptr opGeom;
    switch ( op )
    {
      case OverlayIntersection:
        if ( gridSize > 0 )
        {
          opGeom.reset( GEOSIntersectionPrec_r( context, mGeos.get(), geosGeom.get(), gridSize ) );
        }
        else
        {
          opGeom.reset( GEOSIntersection_r( context, mGeos.get(), geosGeom.get() ) );
        }
        break;

      case OverlayDifference:
        if ( gridSize > 0 )
        {
          opGeom.reset( GEOSDifferencePrec_r( context, mGeos.get(), geosGeom.get(), gridSize ) );
        }
        else
        {
          opGeom.reset( GEOSDifference_r( context, mGeos.get(), geosGeom.get() ) );
        }
        break;

      case OverlayUnion:
      {
        geos::unique_ptr unionGeometry;
        if ( gridSize > 0 )
        {
          unionGeometry.reset( GEOSUnionPrec_r( context, mGeos.get(), geosGeom.get(), gridSize ) );
        }
        else
        {
          unionGeometry.reset( GEOSUnion_r( context, mGeos.get(), geosGeom.get() ) );
        }

        if ( unionGeometry && GEOSGeomTypeId_r( context, unionGeometry.get() ) == GEOS_MULTILINESTRING )
        {
          geos::unique_ptr mergedLines( GEOSLineMerge_r( context, unionGeometry.get() ) );
          if ( mergedLines )
          {
            unionGeometry = std::move( mergedLines );
          }
        }

        opGeom = std::move( unionGeometry );
      }
      break;

      case OverlaySymDifference:
        if ( gridSize > 0 )
        {
          opGeom.reset( GEOSSymDifferencePrec_r( context, mGeos.get(), geosGeom.get(), gridSize ) );
        }
        else
        {
          opGeom.reset( GEOSSymDifference_r( context, mGeos.get(), geosGeom.get() ) );
        }
        break;
    }
    return fromGeos( opGeom.get() );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return nullptr;
  }
}

bool QgsGeos::relation( const QgsAbstractGeometry *geom, Relation r, QString *errorMsg ) const
{
  if ( !mGeos || !geom )
  {
    return false;
  }

  geos::unique_ptr geosGeom( asGeos( geom, mPrecision ) );
  if ( !geosGeom )
  {
    return false;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  bool result = false;
  try
  {
    if ( mGeosPrepared ) //use faster version with prepared geometry
    {
      switch ( r )
      {
        case RelationIntersects:
          result = ( GEOSPreparedIntersects_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationTouches:
          result = ( GEOSPreparedTouches_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationCrosses:
          result = ( GEOSPreparedCrosses_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationWithin:
          result = ( GEOSPreparedWithin_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationContains:
          result = ( GEOSPreparedContains_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationDisjoint:
          result = ( GEOSPreparedDisjoint_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
        case RelationOverlaps:
          result = ( GEOSPreparedOverlaps_r( context, mGeosPrepared.get(), geosGeom.get() ) == 1 );
          break;
      }
      return result;
    }

    switch ( r )
    {
      case RelationIntersects:
        result = ( GEOSIntersects_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationTouches:
        result = ( GEOSTouches_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationCrosses:
        result = ( GEOSCrosses_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationWithin:
        result = ( GEOSWithin_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationContains:
        result = ( GEOSContains_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationDisjoint:
        result = ( GEOSDisjoint_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
      case RelationOverlaps:
        result = ( GEOSOverlaps_r( context, mGeos.get(), geosGeom.get() ) == 1 );
        break;
    }
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return false;
  }

  return result;
}

QgsAbstractGeometry *QgsGeos::buffer( double distance, int segments, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSBuffer_r( QgsGeosContext::get(), mGeos.get(), distance, segments ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() ).release();
}

QgsAbstractGeometry *QgsGeos::buffer( double distance, int segments, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSBufferWithStyle_r( QgsGeosContext::get(), mGeos.get(), distance, segments, static_cast< int >( endCapStyle ), static_cast< int >( joinStyle ), miterLimit ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() ).release();
}

QgsAbstractGeometry *QgsGeos::simplify( double tolerance, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSTopologyPreserveSimplify_r( QgsGeosContext::get(), mGeos.get(), tolerance ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() ).release();
}

QgsAbstractGeometry *QgsGeos::interpolate( double distance, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSInterpolate_r( QgsGeosContext::get(), mGeos.get(), distance ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() ).release();
}

QgsPoint *QgsGeos::centroid( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  double x;
  double y;

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos.reset( GEOSGetCentroid_r( context,  mGeos.get() ) );

    if ( !geos )
      return nullptr;

    GEOSGeomGetX_r( context, geos.get(), &x );
    GEOSGeomGetY_r( context, geos.get(), &y );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )

  return new QgsPoint( x, y );
}

QgsAbstractGeometry *QgsGeos::envelope( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSEnvelope_r( QgsGeosContext::get(), mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() ).release();
}

QgsPoint *QgsGeos::pointOnSurface( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  double x;
  double y;

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSPointOnSurface_r( context, mGeos.get() ) );

    if ( !geos || GEOSisEmpty_r( context, geos.get() ) != 0 )
    {
      return nullptr;
    }

    GEOSGeomGetX_r( context, geos.get(), &x );
    GEOSGeomGetY_r( context, geos.get(), &y );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )

  return new QgsPoint( x, y );
}

QgsAbstractGeometry *QgsGeos::convexHull( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  try
  {
    geos::unique_ptr cHull( GEOSConvexHull_r( QgsGeosContext::get(), mGeos.get() ) );
    std::unique_ptr< QgsAbstractGeometry > cHullGeom = fromGeos( cHull.get() );
    return cHullGeom.release();
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
}

std::unique_ptr< QgsAbstractGeometry > QgsGeos::concaveHull( double targetPercent, bool allowHoles, QString *errorMsg ) const
{
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<11
  ( void )allowHoles;
  ( void )targetPercent;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating concaveHull requires a QGIS build based on GEOS 3.11 or later" ) );
#else
  if ( !mGeos )
  {
    return nullptr;
  }

  try
  {
    geos::unique_ptr concaveHull( GEOSConcaveHull_r( QgsGeosContext::get(), mGeos.get(), targetPercent, allowHoles ) );
    std::unique_ptr< QgsAbstractGeometry > concaveHullGeom = fromGeos( concaveHull.get() );
    return concaveHullGeom;
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
#endif
}

Qgis::CoverageValidityResult QgsGeos::validateCoverage( double gapWidth, std::unique_ptr<QgsAbstractGeometry> *invalidEdges, QString *errorMsg ) const
{
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<12
  ( void )gapWidth;
  ( void )invalidEdges;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Validating coverages requires a QGIS build based on GEOS 3.12 or later" ) );
#else
  if ( !mGeos )
  {
    if ( errorMsg )
      *errorMsg = QStringLiteral( "Input geometry was not set" );
    return Qgis::CoverageValidityResult::Error;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    GEOSGeometry *invalidEdgesGeos = nullptr;
    const int result = GEOSCoverageIsValid_r( context, mGeos.get(), gapWidth, invalidEdges ? &invalidEdgesGeos : nullptr );
    if ( invalidEdges && invalidEdgesGeos )
    {
      *invalidEdges = fromGeos( invalidEdgesGeos );
    }
    if ( invalidEdgesGeos )
    {
      GEOSGeom_destroy_r( context, invalidEdgesGeos );
      invalidEdgesGeos = nullptr;
    }

    switch ( result )
    {
      case 0:
        return Qgis::CoverageValidityResult::Invalid;
      case 1:
        return Qgis::CoverageValidityResult::Valid;
      case 2:
        break;
    }
    return Qgis::CoverageValidityResult::Error;
  }
  CATCH_GEOS_WITH_ERRMSG( Qgis::CoverageValidityResult::Error )
#endif
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::simplifyCoverageVW( double tolerance, bool preserveBoundary, QString *errorMsg ) const
{
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<12
  ( void )tolerance;
  ( void )preserveBoundary;
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Simplifying coverages requires a QGIS build based on GEOS 3.12 or later" ) );
#else
  if ( !mGeos )
  {
    if ( errorMsg )
      *errorMsg = QStringLiteral( "Input geometry was not set" );
    return nullptr;
  }

  try
  {
    geos::unique_ptr simplified( GEOSCoverageSimplifyVW_r( QgsGeosContext::get(), mGeos.get(), tolerance, preserveBoundary ? 1 : 0 ) );
    std::unique_ptr< QgsAbstractGeometry > simplifiedGeom = fromGeos( simplified.get() );
    return simplifiedGeom;
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
#endif
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::unionCoverage( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    if ( errorMsg )
      *errorMsg = QStringLiteral( "Input geometry was not set" );
    return nullptr;
  }

  try
  {
    geos::unique_ptr unioned( GEOSCoverageUnion_r( QgsGeosContext::get(), mGeos.get() ) );
    std::unique_ptr< QgsAbstractGeometry > result = fromGeos( unioned.get() );
    return result;
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
}

bool QgsGeos::isValid( QString *errorMsg, const bool allowSelfTouchingHoles, QgsGeometry *errorLoc ) const
{
  if ( !mGeos )
  {
    if ( errorMsg )
      *errorMsg = QObject::tr( "QGIS geometry cannot be converted to a GEOS geometry", "GEOS Error" );
    return false;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    GEOSGeometry *g1 = nullptr;
    char *r = nullptr;
    char res = GEOSisValidDetail_r( context, mGeos.get(), allowSelfTouchingHoles ? GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE : 0, &r, &g1 );
    const bool invalid = res != 1;

    QString error;
    if ( r )
    {
      error = QString( r );
      GEOSFree_r( context, r );
    }

    if ( invalid && errorMsg )
    {
      // Copied from https://github.com/libgeos/geos/blob/main/src/operation/valid/TopologyValidationError.cpp
      static const std::map< QString, QString > sTranslatedErrors
      {
        { QStringLiteral( "topology validation error" ), QObject::tr( "Topology validation error", "GEOS Error" ) },
        { QStringLiteral( "repeated point" ), QObject::tr( "Repeated point", "GEOS Error" ) },
        { QStringLiteral( "hole lies outside shell" ), QObject::tr( "Hole lies outside shell", "GEOS Error" ) },
        { QStringLiteral( "holes are nested" ), QObject::tr( "Holes are nested", "GEOS Error" ) },
        { QStringLiteral( "interior is disconnected" ), QObject::tr( "Interior is disconnected", "GEOS Error" ) },
        { QStringLiteral( "self-intersection" ), QObject::tr( "Self-intersection", "GEOS Error" ) },
        { QStringLiteral( "ring self-intersection" ), QObject::tr( "Ring self-intersection", "GEOS Error" ) },
        { QStringLiteral( "nested shells" ), QObject::tr( "Nested shells", "GEOS Error" ) },
        { QStringLiteral( "duplicate rings" ), QObject::tr( "Duplicate rings", "GEOS Error" ) },
        { QStringLiteral( "too few points in geometry component" ), QObject::tr( "Too few points in geometry component", "GEOS Error" ) },
        { QStringLiteral( "invalid coordinate" ), QObject::tr( "Invalid coordinate", "GEOS Error" ) },
        { QStringLiteral( "ring is not closed" ), QObject::tr( "Ring is not closed", "GEOS Error" ) },
      };

      const auto translatedError = sTranslatedErrors.find( error.toLower() );
      if ( translatedError != sTranslatedErrors.end() )
        *errorMsg = translatedError->second;
      else
        *errorMsg = error;

      if ( g1 && errorLoc )
      {
        *errorLoc = geometryFromGeos( g1 );
      }
      else if ( g1 )
      {
        GEOSGeom_destroy_r( context, g1 );
      }
    }
    return !invalid;
  }
  CATCH_GEOS_WITH_ERRMSG( false )
}

bool QgsGeos::isEqual( const QgsAbstractGeometry *geom, QString *errorMsg ) const
{
  if ( !mGeos || !geom )
  {
    return false;
  }

  try
  {
    geos::unique_ptr geosGeom( asGeos( geom, mPrecision ) );
    if ( !geosGeom )
    {
      return false;
    }
    bool equal = GEOSEquals_r( QgsGeosContext::get(), mGeos.get(), geosGeom.get() );
    return equal;
  }
  CATCH_GEOS_WITH_ERRMSG( false )
}

bool QgsGeos::isEmpty( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return false;
  }

  try
  {
    return GEOSisEmpty_r( QgsGeosContext::get(), mGeos.get() );
  }
  CATCH_GEOS_WITH_ERRMSG( false )
}

bool QgsGeos::isSimple( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return false;
  }

  try
  {
    return GEOSisSimple_r( QgsGeosContext::get(), mGeos.get() );
  }
  CATCH_GEOS_WITH_ERRMSG( false )
}

GEOSCoordSequence *QgsGeos::createCoordinateSequence( const QgsCurve *curve, double precision, bool forceClose )
{
  GEOSContextHandle_t context = QgsGeosContext::get();

  std::unique_ptr< QgsLineString > segmentized;
  const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( curve );

  if ( !line )
  {
    segmentized.reset( curve->curveToLine() );
    line = segmentized.get();
  }

  if ( !line )
  {
    return nullptr;
  }
  GEOSCoordSequence *coordSeq = nullptr;

  const int numPoints = line->numPoints();

  const bool hasZ = line->is3D();

#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=10 )
  if ( qgsDoubleNear( precision, 0 ) )
  {
    if ( !forceClose || ( line->pointN( 0 ) == line->pointN( numPoints - 1 ) ) )
    {
      // use optimised method if we don't have to force close an open ring
      try
      {
        coordSeq = GEOSCoordSeq_copyFromArrays_r( context, line->xData(), line->yData(), line->zData(), nullptr, numPoints );
        if ( !coordSeq )
        {
          QgsDebugError( QStringLiteral( "GEOS Exception: Could not create coordinate sequence for %1 points" ).arg( numPoints ) );
          return nullptr;
        }
      }
      CATCH_GEOS( nullptr )
    }
    else
    {
      QVector< double > x = line->xVector();
      if ( numPoints > 0 )
        x.append( x.at( 0 ) );
      QVector< double > y = line->yVector();
      if ( numPoints > 0 )
        y.append( y.at( 0 ) );
      QVector< double > z = line->zVector();
      if ( hasZ && numPoints > 0 )
        z.append( z.at( 0 ) );
      try
      {
        coordSeq = GEOSCoordSeq_copyFromArrays_r( context, x.constData(), y.constData(), !hasZ ? nullptr : z.constData(), nullptr, numPoints + 1 );
        if ( !coordSeq )
        {
          QgsDebugError( QStringLiteral( "GEOS Exception: Could not create closed coordinate sequence for %1 points" ).arg( numPoints + 1 ) );
          return nullptr;
        }
      }
      CATCH_GEOS( nullptr )
    }
    return coordSeq;
  }
#endif

  int coordDims = 2;
  const bool hasM = false; //line->isMeasure(); //disabled until geos supports m-coordinates

  if ( hasZ )
  {
    ++coordDims;
  }
  if ( hasM )
  {
    ++coordDims;
  }

  int numOutPoints = numPoints;
  if ( forceClose && ( line->pointN( 0 ) != line->pointN( numPoints - 1 ) ) )
  {
    ++numOutPoints;
  }

  try
  {
    coordSeq = GEOSCoordSeq_create_r( context, numOutPoints, coordDims );
    if ( !coordSeq )
    {
      QgsDebugError( QStringLiteral( "GEOS Exception: Could not create coordinate sequence for %1 points in %2 dimensions" ).arg( numPoints ).arg( coordDims ) );
      return nullptr;
    }

    const double *xData = line->xData();
    const double *yData = line->yData();
    const double *zData = hasZ ? line->zData() : nullptr;
    const double *mData = hasM ? line->mData() : nullptr;

    if ( precision > 0. )
    {
      for ( int i = 0; i < numOutPoints; ++i )
      {
        if ( i >= numPoints )
        {
          // start reading back from start of line
          xData = line->xData();
          yData = line->yData();
          zData = hasZ ? line->zData() : nullptr;
          mData = hasM ? line->mData() : nullptr;
        }
        if ( hasZ )
        {
          GEOSCoordSeq_setXYZ_r( context, coordSeq, i, std::round( *xData++ / precision ) * precision, std::round( *yData++ / precision ) * precision, std::round( *zData++ / precision ) * precision );
        }
        else
        {
          GEOSCoordSeq_setXY_r( context, coordSeq, i, std::round( *xData++ / precision ) * precision, std::round( *yData++ / precision ) * precision );
        }
        if ( hasM )
        {
          GEOSCoordSeq_setOrdinate_r( context, coordSeq, i, 3, *mData++ );
        }
      }
    }
    else
    {
      for ( int i = 0; i < numOutPoints; ++i )
      {
        if ( i >= numPoints )
        {
          // start reading back from start of line
          xData = line->xData();
          yData = line->yData();
          zData = hasZ ? line->zData() : nullptr;
          mData = hasM ? line->mData() : nullptr;
        }
        if ( hasZ )
        {
          GEOSCoordSeq_setXYZ_r( context, coordSeq, i, *xData++, *yData++, *zData++ );
        }
        else
        {
          GEOSCoordSeq_setXY_r( context, coordSeq, i, *xData++, *yData++ );
        }
        if ( hasM )
        {
          GEOSCoordSeq_setOrdinate_r( context, coordSeq, i, 3, *mData++ );
        }
      }
    }
  }
  CATCH_GEOS( nullptr )

  return coordSeq;
}

geos::unique_ptr QgsGeos::createGeosPoint( const QgsAbstractGeometry *point, int coordDims, double precision, Qgis::GeosCreationFlags )
{
  const QgsPoint *pt = qgsgeometry_cast<const QgsPoint *>( point );
  if ( !pt )
    return nullptr;

  return createGeosPointXY( pt->x(), pt->y(), pt->is3D(), pt->z(), pt->isMeasure(), pt->m(), coordDims, precision );
}

geos::unique_ptr QgsGeos::createGeosPointXY( double x, double y, bool hasZ, double z, bool hasM, double m, int coordDims, double precision, Qgis::GeosCreationFlags )
{
  Q_UNUSED( hasM )
  Q_UNUSED( m )

  geos::unique_ptr geosPoint;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    if ( coordDims == 2 )
    {
      // optimised constructor
      if ( precision > 0. )
        geosPoint.reset( GEOSGeom_createPointFromXY_r( context, std::round( x / precision ) * precision, std::round( y / precision ) * precision ) );
      else
        geosPoint.reset( GEOSGeom_createPointFromXY_r( context, x, y ) );
      return geosPoint;
    }

    GEOSCoordSequence *coordSeq = GEOSCoordSeq_create_r( context, 1, coordDims );
    if ( !coordSeq )
    {
      QgsDebugError( QStringLiteral( "GEOS Exception: Could not create coordinate sequence for point with %1 dimensions" ).arg( coordDims ) );
      return nullptr;
    }
    if ( precision > 0. )
    {
      GEOSCoordSeq_setX_r( context, coordSeq, 0, std::round( x / precision ) * precision );
      GEOSCoordSeq_setY_r( context, coordSeq, 0, std::round( y / precision ) * precision );
      if ( hasZ )
      {
        GEOSCoordSeq_setOrdinate_r( context, coordSeq, 0, 2, std::round( z / precision ) * precision );
      }
    }
    else
    {
      GEOSCoordSeq_setX_r( context, coordSeq, 0, x );
      GEOSCoordSeq_setY_r( context, coordSeq, 0, y );
      if ( hasZ )
      {
        GEOSCoordSeq_setOrdinate_r( context, coordSeq, 0, 2, z );
      }
    }
#if 0 //disabled until geos supports m-coordinates
    if ( hasM )
    {
      GEOSCoordSeq_setOrdinate_r( context, coordSeq, 0, 3, m );
    }
#endif
    geosPoint.reset( GEOSGeom_createPoint_r( context, coordSeq ) );
  }
  CATCH_GEOS( nullptr )
  return geosPoint;
}

geos::unique_ptr QgsGeos::createGeosLinestring( const QgsAbstractGeometry *curve, double precision, Qgis::GeosCreationFlags )
{
  const QgsCurve *c = qgsgeometry_cast<const QgsCurve *>( curve );
  if ( !c )
    return nullptr;

  GEOSCoordSequence *coordSeq = createCoordinateSequence( c, precision );
  if ( !coordSeq )
    return nullptr;

  geos::unique_ptr geosGeom;
  try
  {
    geosGeom.reset( GEOSGeom_createLineString_r( QgsGeosContext::get(), coordSeq ) );
  }
  CATCH_GEOS( nullptr )
  return geosGeom;
}

geos::unique_ptr QgsGeos::createGeosPolygon( const QgsAbstractGeometry *poly, double precision, Qgis::GeosCreationFlags flags )
{
  const QgsCurvePolygon *polygon = qgsgeometry_cast<const QgsCurvePolygon *>( poly );
  if ( !polygon )
    return nullptr;

  const QgsCurve *exteriorRing = polygon->exteriorRing();
  if ( !exteriorRing )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr geosPolygon;
  try
  {
    geos::unique_ptr exteriorRingGeos( GEOSGeom_createLinearRing_r( context, createCoordinateSequence( exteriorRing, precision, true ) ) );

    int nHoles = 0;
    int nInteriorRings = polygon->numInteriorRings();
    if ( flags & Qgis::GeosCreationFlag::SkipEmptyInteriorRings )
    {
      for ( int i = 0; i < nInteriorRings; ++i )
      {
        const QgsCurve *interiorRing = polygon->interiorRing( i );
        if ( !interiorRing->isEmpty() )
        {
          nHoles++;
        }
      }
    }
    else
    {
      nHoles = nInteriorRings;
    }
    GEOSGeometry **holes = nullptr;
    if ( nHoles > 0 )
    {
      holes = new GEOSGeometry*[ nHoles ];
    }

    for ( int i = 0; i < nInteriorRings; ++i )
    {
      const QgsCurve *interiorRing = polygon->interiorRing( i );
      if ( !( flags & Qgis::GeosCreationFlag::SkipEmptyInteriorRings ) || !interiorRing->isEmpty() )
      {
        holes[i] = GEOSGeom_createLinearRing_r( context, createCoordinateSequence( interiorRing, precision, true ) );
      }
    }
    geosPolygon.reset( GEOSGeom_createPolygon_r( context, exteriorRingGeos.release(), holes, nHoles ) );
    delete[] holes;
  }
  CATCH_GEOS( nullptr )

  return geosPolygon;
}

QgsAbstractGeometry *QgsGeos::offsetCurve( double distance, int segments, Qgis::JoinStyle joinStyle, double miterLimit, QString *errorMsg ) const
{
  if ( !mGeos )
    return nullptr;

  geos::unique_ptr offset;
  try
  {
    // Force quadrant segments to be at least 8, see
    // https://github.com/qgis/QGIS/issues/53165#issuecomment-1563470832
    if ( segments < 8 )
      segments = 8;
    offset.reset( GEOSOffsetCurve_r( QgsGeosContext::get(), mGeos.get(), distance, segments, static_cast< int >( joinStyle ), miterLimit ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  std::unique_ptr< QgsAbstractGeometry > offsetGeom = fromGeos( offset.get() );
  return offsetGeom.release();
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::singleSidedBuffer( double distance, int segments, Qgis::BufferSide side, Qgis::JoinStyle joinStyle, double miterLimit, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos::buffer_params_unique_ptr bp( GEOSBufferParams_create_r( context ) );
    GEOSBufferParams_setSingleSided_r( context, bp.get(), 1 );
    GEOSBufferParams_setQuadrantSegments_r( context, bp.get(), segments );
    GEOSBufferParams_setJoinStyle_r( context, bp.get(), static_cast< int >( joinStyle ) );
    GEOSBufferParams_setMitreLimit_r( context, bp.get(), miterLimit );  //#spellok

    if ( side == Qgis::BufferSide::Right )
    {
      distance = -distance;
    }
    geos.reset( GEOSBufferWithParams_r( context, mGeos.get(), bp.get(), distance ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::maximumInscribedCircle( double tolerance, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSMaximumInscribedCircle_r( QgsGeosContext::get(), mGeos.get(), tolerance ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::largestEmptyCircle( double tolerance, const QgsAbstractGeometry *boundary, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos::unique_ptr boundaryGeos;
    if ( boundary )
      boundaryGeos = asGeos( boundary );

    geos.reset( GEOSLargestEmptyCircle_r( QgsGeosContext::get(), mGeos.get(), boundaryGeos.get(), tolerance ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::minimumWidth( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSMinimumWidth_r( QgsGeosContext::get(), mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

double QgsGeos::minimumClearance( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return std::numeric_limits< double >::quiet_NaN();
  }

  geos::unique_ptr geos;
  double res = 0;
  try
  {
    if ( GEOSMinimumClearance_r( QgsGeosContext::get(), mGeos.get(), &res ) != 0 )
      return std::numeric_limits< double >::quiet_NaN();
  }
  CATCH_GEOS_WITH_ERRMSG( std::numeric_limits< double >::quiet_NaN() )
  return res;
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::minimumClearanceLine( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSMinimumClearanceLine_r( QgsGeosContext::get(), mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::node( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSNode_r( QgsGeosContext::get(), mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::sharedPaths( const QgsAbstractGeometry *other, QString *errorMsg ) const
{
  if ( !mGeos || !other )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  try
  {
    geos::unique_ptr otherGeos = asGeos( other );
    if ( !otherGeos )
      return nullptr;

    geos.reset( GEOSSharedPaths_r( QgsGeosContext::get(), mGeos.get(), otherGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::reshapeGeometry( const QgsLineString &reshapeWithLine, EngineOperationResult *errorCode, QString *errorMsg ) const
{
  if ( !mGeos || mGeometry->dimension() == 0 )
  {
    if ( errorCode ) { *errorCode = InvalidBaseGeometry; }
    return nullptr;
  }

  if ( reshapeWithLine.numPoints() < 2 )
  {
    if ( errorCode ) { *errorCode = InvalidInput; }
    return nullptr;
  }

  geos::unique_ptr reshapeLineGeos = createGeosLinestring( &reshapeWithLine, mPrecision );

  GEOSContextHandle_t context = QgsGeosContext::get();
  //single or multi?
  int numGeoms = GEOSGetNumGeometries_r( context, mGeos.get() );
  if ( numGeoms == -1 )
  {
    if ( errorCode )
    {
      *errorCode = InvalidBaseGeometry;
    }
    return nullptr;
  }

  bool isMultiGeom = false;
  int geosTypeId = GEOSGeomTypeId_r( context, mGeos.get() );
  if ( geosTypeId == GEOS_MULTILINESTRING || geosTypeId == GEOS_MULTIPOLYGON )
    isMultiGeom = true;

  bool isLine = ( mGeometry->dimension() == 1 );

  if ( !isMultiGeom )
  {
    geos::unique_ptr reshapedGeometry;
    if ( isLine )
    {
      reshapedGeometry = reshapeLine( mGeos.get(), reshapeLineGeos.get(), mPrecision );
    }
    else
    {
      reshapedGeometry = reshapePolygon( mGeos.get(), reshapeLineGeos.get(), mPrecision );
    }

    if ( errorCode )
      *errorCode = Success;
    std::unique_ptr< QgsAbstractGeometry > reshapeResult = fromGeos( reshapedGeometry.get() );
    return reshapeResult;
  }
  else
  {
    try
    {
      //call reshape for each geometry part and replace mGeos with new geometry if reshape took place
      bool reshapeTookPlace = false;

      geos::unique_ptr currentReshapeGeometry;
      GEOSGeometry **newGeoms = new GEOSGeometry*[numGeoms];

      for ( int i = 0; i < numGeoms; ++i )
      {
        if ( isLine )
          currentReshapeGeometry = reshapeLine( GEOSGetGeometryN_r( context, mGeos.get(), i ), reshapeLineGeos.get(), mPrecision );
        else
          currentReshapeGeometry = reshapePolygon( GEOSGetGeometryN_r( context, mGeos.get(), i ), reshapeLineGeos.get(), mPrecision );

        if ( currentReshapeGeometry )
        {
          newGeoms[i] = currentReshapeGeometry.release();
          reshapeTookPlace = true;
        }
        else
        {
          newGeoms[i] = GEOSGeom_clone_r( context, GEOSGetGeometryN_r( context, mGeos.get(), i ) );
        }
      }

      geos::unique_ptr newMultiGeom;
      if ( isLine )
      {
        newMultiGeom.reset( GEOSGeom_createCollection_r( context, GEOS_MULTILINESTRING, newGeoms, numGeoms ) );
      }
      else //multipolygon
      {
        newMultiGeom.reset( GEOSGeom_createCollection_r( context, GEOS_MULTIPOLYGON, newGeoms, numGeoms ) );
      }

      delete[] newGeoms;
      if ( !newMultiGeom )
      {
        if ( errorCode ) { *errorCode = EngineError; }
        return nullptr;
      }

      if ( reshapeTookPlace )
      {
        if ( errorCode )
          *errorCode = Success;
        std::unique_ptr< QgsAbstractGeometry > reshapedMultiGeom = fromGeos( newMultiGeom.get() );
        return reshapedMultiGeom;
      }
      else
      {
        if ( errorCode )
        {
          *errorCode = NothingHappened;
        }
        return nullptr;
      }
    }
    CATCH_GEOS_WITH_ERRMSG( nullptr )
  }
}

std::unique_ptr< QgsAbstractGeometry > QgsGeos::mergeLines( QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  if ( GEOSGeomTypeId_r( context, mGeos.get() ) != GEOS_MULTILINESTRING )
    return nullptr;

  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSLineMerge_r( context, mGeos.get() ) );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
  return fromGeos( geos.get() );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::closestPoint( const QgsGeometry &other, QString *errorMsg ) const
{
  if ( !mGeos || isEmpty() || other.isEmpty() )
  {
    return nullptr;
  }

  geos::unique_ptr otherGeom( asGeos( other.constGet(), mPrecision ) );
  if ( !otherGeom )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  double nx = 0.0;
  double ny = 0.0;
  try
  {
    geos::coord_sequence_unique_ptr nearestCoord;
    if ( mGeosPrepared ) // use faster version with prepared geometry
    {
      nearestCoord.reset( GEOSPreparedNearestPoints_r( context, mGeosPrepared.get(), otherGeom.get() ) );
    }
    else
    {
      nearestCoord.reset( GEOSNearestPoints_r( context, mGeos.get(), otherGeom.get() ) );
    }

    ( void )GEOSCoordSeq_getX_r( context, nearestCoord.get(), 0, &nx );
    ( void )GEOSCoordSeq_getY_r( context, nearestCoord.get(), 0, &ny );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return nullptr;
  }

  return std::make_unique< QgsPoint >( nx, ny );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::shortestLine( const QgsGeometry &other, QString *errorMsg ) const
{
  if ( !mGeos || other.isEmpty() )
  {
    return nullptr;
  }

  return shortestLine( other.constGet(), errorMsg );
}

std::unique_ptr< QgsAbstractGeometry > QgsGeos::shortestLine( const QgsAbstractGeometry *other, QString *errorMsg ) const
{
  if ( !other || other->isEmpty() )
    return nullptr;

  geos::unique_ptr otherGeom( asGeos( other, mPrecision ) );
  if ( !otherGeom )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  double nx1 = 0.0;
  double ny1 = 0.0;
  double nx2 = 0.0;
  double ny2 = 0.0;
  try
  {
    geos::coord_sequence_unique_ptr nearestCoord( GEOSNearestPoints_r( context, mGeos.get(), otherGeom.get() ) );

    if ( !nearestCoord )
    {
      if ( errorMsg )
        *errorMsg = QStringLiteral( "GEOS returned no nearest points" );
      return nullptr;
    }

    ( void )GEOSCoordSeq_getX_r( context, nearestCoord.get(), 0, &nx1 );
    ( void )GEOSCoordSeq_getY_r( context, nearestCoord.get(), 0, &ny1 );
    ( void )GEOSCoordSeq_getX_r( context, nearestCoord.get(), 1, &nx2 );
    ( void )GEOSCoordSeq_getY_r( context, nearestCoord.get(), 1, &ny2 );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return nullptr;
  }

  std::unique_ptr< QgsLineString > line = std::make_unique< QgsLineString >();
  line->addVertex( QgsPoint( nx1, ny1 ) );
  line->addVertex( QgsPoint( nx2, ny2 ) );
  return line;
}

double QgsGeos::lineLocatePoint( const QgsPoint &point, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return -1;
  }

  geos::unique_ptr otherGeom( asGeos( &point, mPrecision ) );
  if ( !otherGeom )
  {
    return -1;
  }

  double distance = -1;
  try
  {
    distance = GEOSProject_r( QgsGeosContext::get(), mGeos.get(), otherGeom.get() );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return -1;
  }

  return distance;
}

double QgsGeos::lineLocatePoint( double x, double y, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return -1;
  }

  geos::unique_ptr point = createGeosPointXY( x, y, false, 0, false, 0, 2, 0 );
  if ( !point )
    return false;

  double distance = -1;
  try
  {
    distance = GEOSProject_r( QgsGeosContext::get(), mGeos.get(), point.get() );
  }
  catch ( GEOSException &e )
  {
    logError( QStringLiteral( "GEOS" ), e.what() );
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    return -1;
  }

  return distance;
}

QgsGeometry QgsGeos::polygonize( const QVector<const QgsAbstractGeometry *> &geometries, QString *errorMsg )
{
  GEOSGeometry **const lineGeosGeometries = new GEOSGeometry*[ geometries.size()];
  int validLines = 0;
  for ( const QgsAbstractGeometry *g : geometries )
  {
    geos::unique_ptr l = asGeos( g );
    if ( l )
    {
      lineGeosGeometries[validLines] = l.release();
      validLines++;
    }
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos::unique_ptr result( GEOSPolygonize_r( context, lineGeosGeometries, validLines ) );
    for ( int i = 0; i < validLines; ++i )
    {
      GEOSGeom_destroy_r( context, lineGeosGeometries[i] );
    }
    delete[] lineGeosGeometries;
    return QgsGeometry( fromGeos( result.get() ) );
  }
  catch ( GEOSException &e )
  {
    if ( errorMsg )
    {
      *errorMsg = e.what();
    }
    for ( int i = 0; i < validLines; ++i )
    {
      GEOSGeom_destroy_r( context, lineGeosGeometries[i] );
    }
    delete[] lineGeosGeometries;
    return QgsGeometry();
  }
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::voronoiDiagram( const QgsAbstractGeometry *extent, double tolerance, bool edgesOnly, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr extentGeosGeom;
  if ( extent )
  {
    extentGeosGeom = asGeos( extent, mPrecision );
    if ( !extentGeosGeom )
    {
      return nullptr;
    }
  }

  geos::unique_ptr geos;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos.reset( GEOSVoronoiDiagram_r( context, mGeos.get(), extentGeosGeom.get(), tolerance, edgesOnly ) );

    if ( !geos || GEOSisEmpty_r( context, geos.get() ) != 0 )
    {
      return nullptr;
    }

    return fromGeos( geos.get() );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::delaunayTriangulation( double tolerance, bool edgesOnly, QString *errorMsg ) const
{
  if ( !mGeos )
  {
    return nullptr;
  }

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr geos;
  try
  {
    geos.reset( GEOSDelaunayTriangulation_r( context, mGeos.get(), tolerance, edgesOnly ) );

    if ( !geos || GEOSisEmpty_r( context, geos.get() ) != 0 )
    {
      return nullptr;
    }

    return fromGeos( geos.get() );
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
}

std::unique_ptr<QgsAbstractGeometry> QgsGeos::constrainedDelaunayTriangulation( QString *errorMsg ) const
{
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<11
  ( void )errorMsg;
  throw QgsNotSupportedException( QObject::tr( "Calculating constrainedDelaunayTriangulation requires a QGIS build based on GEOS 3.11 or later" ) );
#else
  if ( !mGeos )
  {
    return nullptr;
  }

  geos::unique_ptr geos;
  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    geos.reset( GEOSConstrainedDelaunayTriangulation_r( context, mGeos.get() ) );

    if ( !geos || GEOSisEmpty_r( context, geos.get() ) != 0 )
    {
      return nullptr;
    }

    std::unique_ptr< QgsAbstractGeometry > res = fromGeos( geos.get() );
    if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( res.get() ) )
    {
      return std::unique_ptr< QgsAbstractGeometry >( collection->extractPartsByType( Qgis::WkbType::Polygon, true ) );
    }
    else
    {
      return res;
    }
  }
  CATCH_GEOS_WITH_ERRMSG( nullptr )
#endif
}

//! Extract coordinates of linestring's endpoints. Returns false on error.
static bool _linestringEndpoints( const GEOSGeometry *linestring, double &x1, double &y1, double &x2, double &y2 )
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( context, linestring );
  if ( !coordSeq )
    return false;

  unsigned int coordSeqSize;
  if ( GEOSCoordSeq_getSize_r( context, coordSeq, &coordSeqSize ) == 0 )
    return false;

  if ( coordSeqSize < 2 )
    return false;

  GEOSCoordSeq_getX_r( context, coordSeq, 0, &x1 );
  GEOSCoordSeq_getY_r( context, coordSeq, 0, &y1 );
  GEOSCoordSeq_getX_r( context, coordSeq, coordSeqSize - 1, &x2 );
  GEOSCoordSeq_getY_r( context, coordSeq, coordSeqSize - 1, &y2 );
  return true;
}


//! Merge two linestrings if they meet at the given intersection point, return new geometry or null on error.
static geos::unique_ptr _mergeLinestrings( const GEOSGeometry *line1, const GEOSGeometry *line2, const QgsPointXY &intersectionPoint )
{
  double x1, y1, x2, y2;
  if ( !_linestringEndpoints( line1, x1, y1, x2, y2 ) )
    return nullptr;

  double rx1, ry1, rx2, ry2;
  if ( !_linestringEndpoints( line2, rx1, ry1, rx2, ry2 ) )
    return nullptr;

  bool intersectionAtOrigLineEndpoint =
    ( intersectionPoint.x() == x1 && intersectionPoint.y() == y1 ) !=
    ( intersectionPoint.x() == x2 && intersectionPoint.y() == y2 );
  bool intersectionAtReshapeLineEndpoint =
    ( intersectionPoint.x() == rx1 && intersectionPoint.y() == ry1 ) ||
    ( intersectionPoint.x() == rx2 && intersectionPoint.y() == ry2 );

  GEOSContextHandle_t context = QgsGeosContext::get();
  // the intersection must be at the begin/end of both lines
  if ( intersectionAtOrigLineEndpoint && intersectionAtReshapeLineEndpoint )
  {
    geos::unique_ptr g1( GEOSGeom_clone_r( context, line1 ) );
    geos::unique_ptr g2( GEOSGeom_clone_r( context, line2 ) );
    GEOSGeometry *geoms[2] = { g1.release(), g2.release() };
    geos::unique_ptr multiGeom( GEOSGeom_createCollection_r( context, GEOS_MULTILINESTRING, geoms, 2 ) );
    geos::unique_ptr res( GEOSLineMerge_r( context, multiGeom.get() ) );
    return res;
  }
  else
    return nullptr;
}


geos::unique_ptr QgsGeos::reshapeLine( const GEOSGeometry *line, const GEOSGeometry *reshapeLineGeos, double precision )
{
  if ( !line || !reshapeLineGeos )
    return nullptr;

  bool atLeastTwoIntersections = false;
  bool oneIntersection = false;
  QgsPointXY oneIntersectionPoint;

  GEOSContextHandle_t context = QgsGeosContext::get();
  try
  {
    //make sure there are at least two intersection between line and reshape geometry
    geos::unique_ptr intersectGeom( GEOSIntersection_r( context, line, reshapeLineGeos ) );
    if ( intersectGeom )
    {
      const int geomType = GEOSGeomTypeId_r( context, intersectGeom.get() );
      atLeastTwoIntersections = ( geomType == GEOS_MULTIPOINT && GEOSGetNumGeometries_r( context, intersectGeom.get() ) > 1 )
                                || ( geomType == GEOS_GEOMETRYCOLLECTION && GEOSGetNumGeometries_r( context, intersectGeom.get() ) > 0 ) // a collection implies at least two points!
                                || ( geomType == GEOS_MULTILINESTRING && GEOSGetNumGeometries_r( context, intersectGeom.get() ) > 0 );
      // one point is enough when extending line at its endpoint
      if ( GEOSGeomTypeId_r( context, intersectGeom.get() ) == GEOS_POINT )
      {
        const GEOSCoordSequence *intersectionCoordSeq = GEOSGeom_getCoordSeq_r( context, intersectGeom.get() );
        double xi, yi;
        GEOSCoordSeq_getX_r( context, intersectionCoordSeq, 0, &xi );
        GEOSCoordSeq_getY_r( context, intersectionCoordSeq, 0, &yi );
        oneIntersection = true;
        oneIntersectionPoint = QgsPointXY( xi, yi );
      }
    }
  }
  catch ( GEOSException & )
  {
    atLeastTwoIntersections = false;
  }

  // special case when extending line at its endpoint
  if ( oneIntersection )
    return _mergeLinestrings( line, reshapeLineGeos, oneIntersectionPoint );

  if ( !atLeastTwoIntersections )
    return nullptr;

  //begin and end point of original line
  double x1, y1, x2, y2;
  if ( !_linestringEndpoints( line, x1, y1, x2, y2 ) )
    return nullptr;

  geos::unique_ptr beginLineVertex = createGeosPointXY( x1, y1, false, 0, false, 0, 2, precision );
  geos::unique_ptr endLineVertex = createGeosPointXY( x2, y2, false, 0, false, 0, 2, precision );

  bool isRing = false;
  if ( GEOSGeomTypeId_r( context, line ) == GEOS_LINEARRING
       || GEOSEquals_r( context, beginLineVertex.get(), endLineVertex.get() ) == 1 )
    isRing = true;

  //node line and reshape line
  geos::unique_ptr nodedGeometry = nodeGeometries( reshapeLineGeos, line );
  if ( !nodedGeometry )
  {
    return nullptr;
  }

  //and merge them together
  geos::unique_ptr mergedLines( GEOSLineMerge_r( context, nodedGeometry.get() ) );
  if ( !mergedLines )
  {
    return nullptr;
  }

  int numMergedLines = GEOSGetNumGeometries_r( context, mergedLines.get() );
  if ( numMergedLines < 2 ) //some special cases. Normally it is >2
  {
    if ( numMergedLines == 1 ) //reshape line is from begin to endpoint. So we keep the reshapeline
    {
      geos::unique_ptr result( GEOSGeom_clone_r( context, reshapeLineGeos ) );
      return result;
    }
    else
      return nullptr;
  }

  QVector<GEOSGeometry *> resultLineParts; //collection with the line segments that will be contained in result
  QVector<GEOSGeometry *> probableParts; //parts where we can decide on inclusion only after going through all the candidates

  for ( int i = 0; i < numMergedLines; ++i )
  {
    const GEOSGeometry *currentGeom = GEOSGetGeometryN_r( context, mergedLines.get(), i );

    // have we already added this part?
    bool alreadyAdded = false;
    double distance = 0;
    double bufferDistance = std::pow( 10.0L, geomDigits( currentGeom ) - 11 );
    for ( const GEOSGeometry *other : std::as_const( resultLineParts ) )
    {
      GEOSHausdorffDistance_r( context, currentGeom, other, &distance );
      if ( distance < bufferDistance )
      {
        alreadyAdded = true;
        break;
      }
    }
    if ( alreadyAdded )
      continue;

    const GEOSCoordSequence *currentCoordSeq = GEOSGeom_getCoordSeq_r( context, currentGeom );
    unsigned int currentCoordSeqSize;
    GEOSCoordSeq_getSize_r( context, currentCoordSeq, &currentCoordSeqSize );
    if ( currentCoordSeqSize < 2 )
      continue;

    //get the two endpoints of the current line merge result
    double xBegin, xEnd, yBegin, yEnd;
    GEOSCoordSeq_getX_r( context, currentCoordSeq, 0, &xBegin );
    GEOSCoordSeq_getY_r( context, currentCoordSeq, 0, &yBegin );
    GEOSCoordSeq_getX_r( context, currentCoordSeq, currentCoordSeqSize - 1, &xEnd );
    GEOSCoordSeq_getY_r( context, currentCoordSeq, currentCoordSeqSize - 1, &yEnd );
    geos::unique_ptr beginCurrentGeomVertex = createGeosPointXY( xBegin, yBegin, false, 0, false, 0, 2, precision );
    geos::unique_ptr endCurrentGeomVertex = createGeosPointXY( xEnd, yEnd, false, 0, false, 0, 2, precision );

    //check how many endpoints of the line merge result are on the (original) line
    int nEndpointsOnOriginalLine = 0;
    if ( pointContainedInLine( beginCurrentGeomVertex.get(), line ) == 1 )
      nEndpointsOnOriginalLine += 1;

    if ( pointContainedInLine( endCurrentGeomVertex.get(), line ) == 1 )
      nEndpointsOnOriginalLine += 1;

    //check how many endpoints equal the endpoints of the original line
    int nEndpointsSameAsOriginalLine = 0;
    if ( GEOSEquals_r( context, beginCurrentGeomVertex.get(), beginLineVertex.get() ) == 1
         || GEOSEquals_r( context, beginCurrentGeomVertex.get(), endLineVertex.get() ) == 1 )
      nEndpointsSameAsOriginalLine += 1;

    if ( GEOSEquals_r( context, endCurrentGeomVertex.get(), beginLineVertex.get() ) == 1
         || GEOSEquals_r( context, endCurrentGeomVertex.get(), endLineVertex.get() ) == 1 )
      nEndpointsSameAsOriginalLine += 1;

    //check if the current geometry overlaps the original geometry (GEOSOverlap does not seem to work with linestrings)
    bool currentGeomOverlapsOriginalGeom = false;
    bool currentGeomOverlapsReshapeLine = false;
    if ( lineContainedInLine( currentGeom, line ) == 1 )
      currentGeomOverlapsOriginalGeom = true;

    if ( lineContainedInLine( currentGeom, reshapeLineGeos ) == 1 )
      currentGeomOverlapsReshapeLine = true;

    //logic to decide if this part belongs to the result
    if ( !isRing && nEndpointsSameAsOriginalLine == 1 && nEndpointsOnOriginalLine == 2 && currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone_r( context, currentGeom ) );
    }
    //for closed rings, we take one segment from the candidate list
    else if ( isRing && nEndpointsOnOriginalLine == 2 && currentGeomOverlapsOriginalGeom )
    {
      probableParts.push_back( GEOSGeom_clone_r( context, currentGeom ) );
    }
    else if ( nEndpointsOnOriginalLine == 2 && !currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone_r( context, currentGeom ) );
    }
    else if ( nEndpointsSameAsOriginalLine == 2 && !currentGeomOverlapsOriginalGeom )
    {
      resultLineParts.push_back( GEOSGeom_clone_r( context, currentGeom ) );
    }
    else if ( currentGeomOverlapsOriginalGeom && currentGeomOverlapsReshapeLine )
    {
      resultLineParts.push_back( GEOSGeom_clone_r( context, currentGeom ) );
    }
  }

  //add the longest segment from the probable list for rings (only used for polygon rings)
  if ( isRing && !probableParts.isEmpty() )
  {
    geos::unique_ptr maxGeom; //the longest geometry in the probabla list
    GEOSGeometry *currentGeom = nullptr;
    double maxLength = -std::numeric_limits<double>::max();
    double currentLength = 0;
    for ( int i = 0; i < probableParts.size(); ++i )
    {
      currentGeom = probableParts.at( i );
      GEOSLength_r( context, currentGeom, &currentLength );
      if ( currentLength > maxLength )
      {
        maxLength = currentLength;
        maxGeom.reset( currentGeom );
      }
      else
      {
        GEOSGeom_destroy_r( context, currentGeom );
      }
    }
    resultLineParts.push_back( maxGeom.release() );
  }

  geos::unique_ptr result;
  if ( resultLineParts.empty() )
    return nullptr;

  if ( resultLineParts.size() == 1 ) //the whole result was reshaped
  {
    result.reset( resultLineParts[0] );
  }
  else //>1
  {
    GEOSGeometry **lineArray = new GEOSGeometry*[resultLineParts.size()];
    for ( int i = 0; i < resultLineParts.size(); ++i )
    {
      lineArray[i] = resultLineParts[i];
    }

    //create multiline from resultLineParts
    geos::unique_ptr multiLineGeom( GEOSGeom_createCollection_r( context, GEOS_MULTILINESTRING, lineArray, resultLineParts.size() ) );
    delete [] lineArray;

    //then do a linemerge with the newly combined partstrings
    result.reset( GEOSLineMerge_r( context, multiLineGeom.get() ) );
  }

  //now test if the result is a linestring. Otherwise something went wrong
  if ( GEOSGeomTypeId_r( context, result.get() ) != GEOS_LINESTRING )
  {
    return nullptr;
  }

  return result;
}

geos::unique_ptr QgsGeos::reshapePolygon( const GEOSGeometry *polygon, const GEOSGeometry *reshapeLineGeos, double precision )
{
  //go through outer shell and all inner rings and check if there is exactly one intersection of a ring and the reshape line
  int nIntersections = 0;
  int lastIntersectingRing = -2;
  const GEOSGeometry *lastIntersectingGeom = nullptr;

  GEOSContextHandle_t context = QgsGeosContext::get();
  int nRings = GEOSGetNumInteriorRings_r( context, polygon );
  if ( nRings < 0 )
    return nullptr;

  //does outer ring intersect?
  const GEOSGeometry *outerRing = GEOSGetExteriorRing_r( context, polygon );
  if ( GEOSIntersects_r( context, outerRing, reshapeLineGeos ) == 1 )
  {
    ++nIntersections;
    lastIntersectingRing = -1;
    lastIntersectingGeom = outerRing;
  }

  //do inner rings intersect?
  const GEOSGeometry **innerRings = new const GEOSGeometry*[nRings];

  try
  {
    for ( int i = 0; i < nRings; ++i )
    {
      innerRings[i] = GEOSGetInteriorRingN_r( context, polygon, i );
      if ( GEOSIntersects_r( context, innerRings[i], reshapeLineGeos ) == 1 )
      {
        ++nIntersections;
        lastIntersectingRing = i;
        lastIntersectingGeom = innerRings[i];
      }
    }
  }
  catch ( GEOSException & )
  {
    nIntersections = 0;
  }

  if ( nIntersections != 1 ) //reshape line is only allowed to intersect one ring
  {
    delete [] innerRings;
    return nullptr;
  }

  //we have one intersecting ring, let's try to reshape it
  geos::unique_ptr reshapeResult = reshapeLine( lastIntersectingGeom, reshapeLineGeos, precision );
  if ( !reshapeResult )
  {
    delete [] innerRings;
    return nullptr;
  }

  //if reshaping took place, we need to reassemble the polygon and its rings
  GEOSGeometry *newRing = nullptr;
  const GEOSCoordSequence *reshapeSequence = GEOSGeom_getCoordSeq_r( context, reshapeResult.get() );
  GEOSCoordSequence *newCoordSequence = GEOSCoordSeq_clone_r( context, reshapeSequence );

  reshapeResult.reset();

  newRing = GEOSGeom_createLinearRing_r( context, newCoordSequence );
  if ( !newRing )
  {
    delete [] innerRings;
    return nullptr;
  }

  GEOSGeometry *newOuterRing = nullptr;
  if ( lastIntersectingRing == -1 )
    newOuterRing = newRing;
  else
    newOuterRing = GEOSGeom_clone_r( context, outerRing );

  //check if all the rings are still inside the outer boundary
  QVector<GEOSGeometry *> ringList;
  if ( nRings > 0 )
  {
    GEOSGeometry *outerRingPoly = GEOSGeom_createPolygon_r( context, GEOSGeom_clone_r( context, newOuterRing ), nullptr, 0 );
    if ( outerRingPoly )
    {
      ringList.reserve( nRings );
      GEOSGeometry *currentRing = nullptr;
      for ( int i = 0; i < nRings; ++i )
      {
        if ( lastIntersectingRing == i )
          currentRing = newRing;
        else
          currentRing = GEOSGeom_clone_r( context, innerRings[i] );

        //possibly a ring is no longer contained in the result polygon after reshape
        if ( GEOSContains_r( context, outerRingPoly, currentRing ) == 1 )
          ringList.push_back( currentRing );
        else
          GEOSGeom_destroy_r( context, currentRing );
      }
    }
    GEOSGeom_destroy_r( context, outerRingPoly );
  }

  GEOSGeometry **newInnerRings = new GEOSGeometry*[ringList.size()];
  for ( int i = 0; i < ringList.size(); ++i )
    newInnerRings[i] = ringList.at( i );

  delete [] innerRings;

  geos::unique_ptr reshapedPolygon( GEOSGeom_createPolygon_r( context, newOuterRing, newInnerRings, ringList.size() ) );
  delete[] newInnerRings;

  return reshapedPolygon;
}

int QgsGeos::lineContainedInLine( const GEOSGeometry *line1, const GEOSGeometry *line2 )
{
  if ( !line1 || !line2 )
  {
    return -1;
  }

  double bufferDistance = std::pow( 10.0L, geomDigits( line2 ) - 11 );

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr bufferGeom( GEOSBuffer_r( context, line2, bufferDistance, DEFAULT_QUADRANT_SEGMENTS ) );
  if ( !bufferGeom )
    return -2;

  geos::unique_ptr intersectionGeom( GEOSIntersection_r( context, bufferGeom.get(), line1 ) );

  //compare ratio between line1Length and intersectGeomLength (usually close to 1 if line1 is contained in line2)
  double intersectGeomLength;
  double line1Length;

  GEOSLength_r( context, intersectionGeom.get(), &intersectGeomLength );
  GEOSLength_r( context, line1, &line1Length );

  double intersectRatio = line1Length / intersectGeomLength;
  if ( intersectRatio > 0.9 && intersectRatio < 1.1 )
    return 1;

  return 0;
}

int QgsGeos::pointContainedInLine( const GEOSGeometry *point, const GEOSGeometry *line )
{
  if ( !point || !line )
    return -1;

  double bufferDistance = std::pow( 10.0L, geomDigits( line ) - 11 );

  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr lineBuffer( GEOSBuffer_r( context, line, bufferDistance, 8 ) );
  if ( !lineBuffer )
    return -2;

  bool contained = false;
  if ( GEOSContains_r( context, lineBuffer.get(), point ) == 1 )
    contained = true;

  return contained;
}

int QgsGeos::geomDigits( const GEOSGeometry *geom )
{
  GEOSContextHandle_t context = QgsGeosContext::get();
  geos::unique_ptr bbox( GEOSEnvelope_r( context, geom ) );
  if ( !bbox.get() )
    return -1;

  const GEOSGeometry *bBoxRing = GEOSGetExteriorRing_r( context, bbox.get() );
  if ( !bBoxRing )
    return -1;

  const GEOSCoordSequence *bBoxCoordSeq = GEOSGeom_getCoordSeq_r( context, bBoxRing );

  if ( !bBoxCoordSeq )
    return -1;

  unsigned int nCoords = 0;
  if ( !GEOSCoordSeq_getSize_r( context, bBoxCoordSeq, &nCoords ) )
    return -1;

  int maxDigits = -1;
  for ( unsigned int i = 0; i < nCoords - 1; ++i )
  {
    double t;
    GEOSCoordSeq_getX_r( context, bBoxCoordSeq, i, &t );

    int digits;
    digits = std::ceil( std::log10( std::fabs( t ) ) );
    if ( digits > maxDigits )
      maxDigits = digits;

    GEOSCoordSeq_getY_r( context, bBoxCoordSeq, i, &t );
    digits = std::ceil( std::log10( std::fabs( t ) ) );
    if ( digits > maxDigits )
      maxDigits = digits;
  }

  return maxDigits;
}
