/***************************************************************************
               qgscoordinatetransform_p.cpp
               ----------------------------
    begin                : May 2017
    copyright            : (C) 2017 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscoordinatetransform_p.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsreadwritelocker.h"
#include "qgsmessagelog.h"

#if PROJ_VERSION_MAJOR>=6
#include "qgsprojutils.h"
#include <proj.h>
#include <proj_experimental.h>
#else
#include <proj_api.h>
#endif

#include <sqlite3.h>

#include <QStringList>

/// @cond PRIVATE

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs,
                     const QgsDatumTransform::GridDetails &grid )> QgsCoordinateTransformPrivate::sMissingRequiredGridHandler = nullptr;

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs,
                     const QgsDatumTransform::TransformDetails &preferredOperation,
                     const QgsDatumTransform::TransformDetails &availableOperation )> QgsCoordinateTransformPrivate::sMissingPreferredGridHandler = nullptr;

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs,
                     const QString &error )> QgsCoordinateTransformPrivate::sCoordinateOperationCreationErrorHandler = nullptr;

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs,
                     const QgsDatumTransform::TransformDetails &desiredOperation )> QgsCoordinateTransformPrivate::sMissingGridUsedByContextHandler = nullptr;

#if PROJ_VERSION_MAJOR<6
#ifdef USE_THREAD_LOCAL
thread_local QgsProjContextStore QgsCoordinateTransformPrivate::mProjContext;
#else
QThreadStorage< QgsProjContextStore * > QgsCoordinateTransformPrivate::mProjContext;
#endif

QgsProjContextStore::QgsProjContextStore()
{
  context = pj_ctx_alloc();
}

QgsProjContextStore::~QgsProjContextStore()
{
  pj_ctx_free( context );
}

#endif

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate()
{
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source,
    const QgsCoordinateReferenceSystem &destination,
    const QgsCoordinateTransformContext &context )
  : mSourceCRS( source )
  , mDestCRS( destination )
{
  calculateTransforms( context );
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, int sourceDatumTransform, int destDatumTransform )
  : mSourceCRS( source )
  , mDestCRS( destination )
  , mSourceDatumTransform( sourceDatumTransform )
  , mDestinationDatumTransform( destDatumTransform )
{
}

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateTransformPrivate &other )
  : QSharedData( other )
  , mIsValid( other.mIsValid )
  , mShortCircuit( other.mShortCircuit )
  , mSourceCRS( other.mSourceCRS )
  , mDestCRS( other.mDestCRS )
  , mSourceDatumTransform( other.mSourceDatumTransform )
  , mDestinationDatumTransform( other.mDestinationDatumTransform )
  , mProjCoordinateOperation( other.mProjCoordinateOperation )
{
#if PROJ_VERSION_MAJOR < 6
  //must reinitialize to setup mSourceProjection and mDestinationProjection
  initialize();
#endif
}
Q_NOWARN_DEPRECATED_POP

Q_NOWARN_DEPRECATED_PUSH
QgsCoordinateTransformPrivate::~QgsCoordinateTransformPrivate()
{
  // free the proj objects
  freeProj();
}
Q_NOWARN_DEPRECATED_POP

bool QgsCoordinateTransformPrivate::checkValidity()
{
  if ( !mSourceCRS.isValid() || !mDestCRS.isValid() )
  {
    invalidate();
    return false;
  }
  return true;
}

void QgsCoordinateTransformPrivate::invalidate()
{
  mShortCircuit = true;
  mIsValid = false;
}

bool QgsCoordinateTransformPrivate::initialize()
{
  invalidate();
  if ( !mSourceCRS.isValid() )
  {
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    QgsDebugMsgLevel( QStringLiteral( "Source CRS is invalid!" ), 4 );
    return false;
  }

  if ( !mDestCRS.isValid() )
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj.
    mDestCRS = mSourceCRS;
    QgsDebugMsgLevel( QStringLiteral( "Destination CRS is invalid!" ), 4 );
    return false;
  }

  mIsValid = true;

  // init the projections (destination and source)
  freeProj();

#if PROJ_VERSION_MAJOR < 6
  Q_NOWARN_DEPRECATED_PUSH
  int sourceDatumTransform = mSourceDatumTransform;
  int destDatumTransform = mDestinationDatumTransform;
  bool useDefaultDatumTransform = ( sourceDatumTransform == - 1 && destDatumTransform == -1 );

  mSourceProjString = mSourceCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    mSourceProjString = stripDatumTransform( mSourceProjString );
  }
  if ( sourceDatumTransform != -1 )
  {
    mSourceProjString += ( ' ' + QgsDatumTransform::datumTransformToProj( sourceDatumTransform ) );
  }

  mDestProjString = mDestCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    mDestProjString = stripDatumTransform( mDestProjString );
  }
  if ( destDatumTransform != -1 )
  {
    mDestProjString += ( ' ' +  QgsDatumTransform::datumTransformToProj( destDatumTransform ) );
  }

  if ( !useDefaultDatumTransform )
  {
    addNullGridShifts( mSourceProjString, mDestProjString, sourceDatumTransform, destDatumTransform );
  }
  Q_NOWARN_DEPRECATED_POP
#endif

  // create proj projections for current thread
  ProjData res = threadLocalProjData();

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "From proj : " + mSourceCRS.toProj4() );
  QgsDebugMsg( "To proj   : " + mDestCRS.toProj4() );
#endif

#if PROJ_VERSION_MAJOR>=6
  if ( !res )
    mIsValid = false;
#else
  if ( !res.first || !res.second )
  {
    mIsValid = false;
  }
#endif

#ifdef COORDINATE_TRANSFORM_VERBOSE
  if ( mIsValid )
  {
    QgsDebugMsg( QStringLiteral( "------------------------------------------------------------" ) );
    QgsDebugMsg( QStringLiteral( "The OGR Coordinate transformation for this layer was set to" ) );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Input", mSourceCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Output", mDestCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsDebugMsg( QStringLiteral( "------------------------------------------------------------" ) );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "------------------------------------------------------------" ) );
    QgsDebugMsg( QStringLiteral( "The OGR Coordinate transformation FAILED TO INITIALIZE!" ) );
    QgsDebugMsg( QStringLiteral( "------------------------------------------------------------" ) );
  }
#else
  if ( !mIsValid )
  {
    QgsDebugMsg( QStringLiteral( "Coordinate transformation failed to initialize!" ) );
  }
#endif

  //XXX todo overload == operator for QgsCoordinateReferenceSystem
  //at the moment srs.parameters contains the whole proj def...soon it won't...
  //if (mSourceCRS->toProj4() == mDestCRS->toProj4())
  if ( mSourceCRS == mDestCRS )
  {
    // If the source and destination projection are the same, set the short
    // circuit flag (no transform takes place)
    mShortCircuit = true;
    QgsDebugMsgLevel( QStringLiteral( "Source/Dest CRS equal, shortcircuit is set." ), 3 );
  }
  else
  {
    // Transform must take place
    mShortCircuit = false;
    QgsDebugMsgLevel( QStringLiteral( "Source/Dest CRS not equal, shortcircuit is not set." ), 3 );
  }
  return mIsValid;
}

void QgsCoordinateTransformPrivate::calculateTransforms( const QgsCoordinateTransformContext &context )
{
  // recalculate datum transforms from context
#if PROJ_VERSION_MAJOR >= 6
  mProjCoordinateOperation = context.calculateCoordinateOperation( mSourceCRS, mDestCRS );
#else
  Q_NOWARN_DEPRECATED_PUSH
  QgsDatumTransform::TransformPair transforms = context.calculateDatumTransforms( mSourceCRS, mDestCRS );
  mSourceDatumTransform = transforms.sourceTransformId;
  mDestinationDatumTransform = transforms.destinationTransformId;
  Q_NOWARN_DEPRECATED_POP
#endif
}

#if PROJ_VERSION_MAJOR>=6
static void proj_collecting_logger( void *user_data, int /*level*/, const char *message )
{
  QStringList *dest = reinterpret_cast< QStringList * >( user_data );
  dest->append( QString( message ) );
}

static void proj_logger( void *, int level, const char *message )
{
#ifndef QGISDEBUG
  Q_UNUSED( message )
#endif
  if ( level == PJ_LOG_ERROR )
  {
    QgsDebugMsg( QString( message ) );
  }
  else if ( level == PJ_LOG_DEBUG )
  {
    QgsDebugMsgLevel( QString( message ), 3 );
  }
}
#endif

ProjData QgsCoordinateTransformPrivate::threadLocalProjData()
{
  QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );

#if PROJ_VERSION_MAJOR>=6
  PJ_CONTEXT *context = QgsProjContext::get();
  QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constFind( reinterpret_cast< uintptr_t>( context ) );
#else
#ifdef USE_THREAD_LOCAL
  QMap < uintptr_t, QPair< projPJ, projPJ > >::const_iterator it = mProjProjections.constFind( reinterpret_cast< uintptr_t>( mProjContext.get() ) );
#else
  projCtx pContext = nullptr;
  if ( mProjContext.hasLocalData() )
  {
    pContext = mProjContext.localData()->get();
  }
  else
  {
    mProjContext.setLocalData( new QgsProjContextStore() );
    pContext = mProjContext.localData()->get();
  }
  QMap < uintptr_t, QPair< projPJ, projPJ > >::const_iterator it = mProjProjections.constFind( reinterpret_cast< uintptr_t>( pContext ) );
#endif
#endif

  if ( it != mProjProjections.constEnd() )
  {
    ProjData res = it.value();
    return res;
  }

  // proj projections don't exist yet, so we need to create
  locker.changeMode( QgsReadWriteLocker::Write );

#if PROJ_VERSION_MAJOR>=6
  // use a temporary proj error collector
  QStringList projErrors;
  proj_log_func( context, &projErrors, proj_collecting_logger );

  QgsProjUtils::proj_pj_unique_ptr transform;
  if ( !mProjCoordinateOperation.isEmpty() )
  {
    transform.reset( proj_create( context, mProjCoordinateOperation.toUtf8().constData() ) );
    if ( !transform || !proj_coordoperation_is_instantiable( context, transform.get() ) )
    {
      if ( sMissingGridUsedByContextHandler )
      {
        QgsDatumTransform::TransformDetails desired;
        desired.proj = mProjCoordinateOperation;
        desired.accuracy = -1; //unknown, can't retrieve from proj as we can't instantiate the op
        desired.grids = QgsProjUtils::gridsUsed( mProjCoordinateOperation );
        sMissingGridUsedByContextHandler( mSourceCRS, mDestCRS, desired );
      }
      else
      {
        const QString err = QObject::tr( "Could not use operation specified in project between %1 and %2. (Wanted to use: %3)." ).arg( mSourceCRS.authid(),
                            mDestCRS.authid(),
                            mProjCoordinateOperation );
        QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
      }

      transform.reset();
    }
    else
    {
      // transform may have either the source or destination CRS using swapped axis order. For QGIS, we ALWAYS need regular x/y axis order
      transform.reset( proj_normalize_for_visualization( context, transform.get() ) );
      if ( !transform )
      {
        const QString err = QObject::tr( "Cannot normalize transform between %1 and %2" ).arg( mSourceCRS.authid(),
                            mDestCRS.authid() );
        QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
      }
    }
  }

  QString nonAvailableError;
  if ( !transform ) // fallback on default proj pathway
  {
    if ( !mSourceCRS.projObject() || ! mDestCRS.projObject() )
    {
      proj_log_func( context, nullptr, nullptr );
      return nullptr;
    }

    PJ_OPERATION_FACTORY_CONTEXT *operationContext = proj_create_operation_factory_context( context, nullptr );

    // We want to check ALL grids, not just those available for use
    proj_operation_factory_context_set_grid_availability_use( context, operationContext, PROJ_GRID_AVAILABILITY_IGNORED );

    // See https://lists.osgeo.org/pipermail/proj/2019-May/008604.html
    proj_operation_factory_context_set_spatial_criterion( context, operationContext, PROJ_SPATIAL_CRITERION_PARTIAL_INTERSECTION );

    if ( PJ_OBJ_LIST *ops = proj_create_operations( context, mSourceCRS.projObject(), mDestCRS.projObject(), operationContext ) )
    {
      int count = proj_list_get_count( ops );
      if ( count < 1 )
      {
        // huh?
        int errNo = proj_context_errno( context );
        if ( errNo && errNo != -61 )
        {
          nonAvailableError = QString( proj_errno_string( errNo ) );
        }
        else
        {
          nonAvailableError = QObject::tr( "No coordinate operations are available between these two reference systems" );
        }
      }
      else if ( count == 1 )
      {
        // only a single operation available. Can we use it?
        transform.reset( proj_list_get( context, ops, 0 ) );
        if ( transform )
        {
          if ( !proj_coordoperation_is_instantiable( context, transform.get() ) )
          {
            // uh oh :( something is missing! find what it is
            for ( int j = 0; j < proj_coordoperation_get_grid_used_count( context, transform.get() ); ++j )
            {
              const char *shortName = nullptr;
              const char *fullName = nullptr;
              const char *packageName = nullptr;
              const char *url = nullptr;
              int directDownload = 0;
              int openLicense = 0;
              int isAvailable = 0;
              proj_coordoperation_get_grid_used( context, transform.get(), j, &shortName, &fullName, &packageName, &url, &directDownload, &openLicense, &isAvailable );
              if ( !isAvailable )
              {
                // found it!
                if ( sMissingRequiredGridHandler )
                {
                  QgsDatumTransform::GridDetails gridDetails;
                  gridDetails.shortName = QString( shortName );
                  gridDetails.fullName = QString( fullName );
                  gridDetails.packageName = QString( packageName );
                  gridDetails.url = QString( url );
                  gridDetails.directDownload = directDownload;
                  gridDetails.openLicense = openLicense;
                  gridDetails.isAvailable = isAvailable;
                  sMissingRequiredGridHandler( mSourceCRS, mDestCRS, gridDetails );
                }
                else
                {
                  const QString err = QObject::tr( "Cannot create transform between %1 and %2, missing required grid %3" ).arg( mSourceCRS.authid(),
                                      mDestCRS.authid(),
                                      shortName );
                  QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
                }
                break;
              }
            }
          }
          else
          {

            // transform may have either the source or destination CRS using swapped axis order. For QGIS, we ALWAYS need regular x/y axis order
            transform.reset( proj_normalize_for_visualization( context, transform.get() ) );
            if ( !transform )
            {
              const QString err = QObject::tr( "Cannot normalize transform between %1 and %2" ).arg( mSourceCRS.authid(),
                                  mDestCRS.authid() );
              QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
            }
          }
        }
      }
      else
      {
        // multiple operations available. Can we use the best one?
        QgsDatumTransform::TransformDetails preferred;
        bool missingPreferred = false;
        for ( int i = 0; i < count; ++ i )
        {
          transform.reset( proj_list_get( context, ops, i ) );
          const bool isInstantiable = transform && proj_coordoperation_is_instantiable( context, transform.get() );
          if ( i == 0 && transform && !isInstantiable )
          {
            // uh oh :( something is missing blocking us from the preferred operation!
            missingPreferred = true;
            preferred = QgsDatumTransform::transformDetailsFromPj( transform.get() );
          }
          if ( transform && isInstantiable )
          {
            // found one
            break;
          }
          transform.reset();
        }

        if ( transform && missingPreferred )
        {
          // found a transform, but it's not the preferred
          QgsDatumTransform::TransformDetails available = QgsDatumTransform::transformDetailsFromPj( transform.get() );
          if ( sMissingPreferredGridHandler )
          {
            sMissingPreferredGridHandler( mSourceCRS, mDestCRS, preferred, available );
          }
          else
          {
            const QString err = QObject::tr( "Using non-preferred coordinate operation between %1 and %2. Using %3, preferred %4." ).arg( mSourceCRS.authid(),
                                mDestCRS.authid(),
                                available.proj,
                                preferred.proj );
            QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
          }
        }

        // transform may have either the source or destination CRS using swapped axis order. For QGIS, we ALWAYS need regular x/y axis order
        if ( transform )
          transform.reset( proj_normalize_for_visualization( context, transform.get() ) );
        if ( !transform )
        {
          const QString err = QObject::tr( "Cannot normalize transform between %1 and %2" ).arg( mSourceCRS.authid(),
                              mDestCRS.authid() );
          QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
        }
      }
      proj_list_destroy( ops );
    }
    proj_operation_factory_context_destroy( operationContext );
  }

  if ( !transform && nonAvailableError.isEmpty() )
  {
    int errNo = proj_context_errno( context );
    if ( errNo && errNo != -61 )
    {
      nonAvailableError = QString( proj_errno_string( errNo ) );
    }
    else if ( !projErrors.empty() )
    {
      nonAvailableError = projErrors.constLast();
    }

    if ( nonAvailableError.isEmpty() )
    {
      nonAvailableError = QObject::tr( "No coordinate operations are available between these two reference systems" );
    }
    else
    {
      // strip proj prefixes from error string, so that it's nicer for users
      nonAvailableError = nonAvailableError.remove( QStringLiteral( "internal_proj_create_operations: " ) );
    }
  }

  if ( !nonAvailableError.isEmpty() )
  {
    if ( sCoordinateOperationCreationErrorHandler )
    {
      sCoordinateOperationCreationErrorHandler( mSourceCRS, mDestCRS, nonAvailableError );
    }
    else
    {
      const QString err = QObject::tr( "Cannot create transform between %1 and %2: %3" ).arg( mSourceCRS.authid(),
                          mDestCRS.authid(),
                          nonAvailableError );
      QgsMessageLog::logMessage( err, QString(), Qgis::Critical );
    }
  }

  // reset logger to terminal output
  proj_log_func( context, nullptr, proj_logger );

  if ( !transform )
  {
    // ouch!
    return nullptr;
  }

  ProjData res = transform.release();
  mProjProjections.insert( reinterpret_cast< uintptr_t>( context ), res );
#else
#ifdef USE_THREAD_LOCAL
  Q_NOWARN_DEPRECATED_PUSH
  QPair<projPJ, projPJ> res = qMakePair( pj_init_plus_ctx( mProjContext.get(), mSourceProjString.toUtf8() ),
                                         pj_init_plus_ctx( mProjContext.get(), mDestProjString.toUtf8() ) );
  Q_NOWARN_DEPRECATED_POP
  mProjProjections.insert( reinterpret_cast< uintptr_t>( mProjContext.get() ), res );
#else
  QPair<projPJ, projPJ> res = qMakePair( pj_init_plus_ctx( pContext, mSourceProjString.toUtf8() ),
                                         pj_init_plus_ctx( pContext, mDestProjString.toUtf8() ) );
  mProjProjections.insert( reinterpret_cast< uintptr_t>( pContext ), res );
#endif
#endif
  return res;
}

void QgsCoordinateTransformPrivate::setCustomMissingRequiredGridHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::GridDetails & )> &handler )
{
  sMissingRequiredGridHandler = handler;
}

void QgsCoordinateTransformPrivate::setCustomMissingPreferredGridHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::TransformDetails &, const QgsDatumTransform::TransformDetails & )> &handler )
{
  sMissingPreferredGridHandler = handler;
}

void QgsCoordinateTransformPrivate::setCustomCoordinateOperationCreationErrorHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QString & )> &handler )
{
  sCoordinateOperationCreationErrorHandler = handler;
}

void QgsCoordinateTransformPrivate::setCustomMissingGridUsedByContextHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::TransformDetails & )> &handler )
{
  sMissingGridUsedByContextHandler = handler;
}

#if PROJ_VERSION_MAJOR<6
QString QgsCoordinateTransformPrivate::stripDatumTransform( const QString &proj4 ) const
{
  QStringList parameterSplit = proj4.split( '+', QString::SkipEmptyParts );
  QString currentParameter;
  QString newProjString;

  for ( int i = 0; i < parameterSplit.size(); ++i )
  {
    currentParameter = parameterSplit.at( i );
    if ( !currentParameter.startsWith( QLatin1String( "towgs84" ), Qt::CaseInsensitive )
         && !currentParameter.startsWith( QLatin1String( "nadgrids" ), Qt::CaseInsensitive ) )
    {
      newProjString.append( '+' );
      newProjString.append( currentParameter );
      newProjString.append( ' ' );
    }
  }
  return newProjString;
}

void QgsCoordinateTransformPrivate::addNullGridShifts( QString &srcProjString, QString &destProjString,
    int sourceDatumTransform, int destinationDatumTransform ) const
{
  //if one transformation uses ntv2, the other one needs to be null grid shift
  if ( destinationDatumTransform == -1 && srcProjString.contains( QLatin1String( "+nadgrids" ) ) ) //add null grid if source transformation is ntv2
  {
    destProjString += QLatin1String( " +nadgrids=@null" );
    return;
  }
  if ( sourceDatumTransform == -1 && destProjString.contains( QLatin1String( "+nadgrids" ) ) )
  {
    srcProjString += QLatin1String( " +nadgrids=@null" );
    return;
  }

  //add null shift grid for google mercator
  //(see e.g. http://trac.osgeo.org/proj/wiki/FAQ#ChangingEllipsoidWhycantIconvertfromWGS84toGoogleEarthVirtualGlobeMercator)
  if ( mSourceCRS.authid().compare( QLatin1String( "EPSG:3857" ), Qt::CaseInsensitive ) == 0 && sourceDatumTransform == -1 )
  {
    srcProjString += QLatin1String( " +nadgrids=@null" );
  }
  if ( mDestCRS.authid().compare( QLatin1String( "EPSG:3857" ), Qt::CaseInsensitive ) == 0 && destinationDatumTransform == -1 )
  {
    destProjString += QLatin1String( " +nadgrids=@null" );
  }
}
#endif

void QgsCoordinateTransformPrivate::freeProj()
{
  QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );
  if ( mProjProjections.isEmpty() )
    return;
  QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constBegin();
#if PROJ_VERSION_MAJOR>=6
  // During destruction of PJ* objects, the errno is set in the underlying
  // context. Consequently the context attached to the PJ* must still exist !
  // Which is not necessarily the case currently unfortunately. So
  // create a temporary dummy context, and attach it to the PJ* before destroying
  // it
  PJ_CONTEXT *tmpContext = proj_context_create();
  for ( ; it != mProjProjections.constEnd(); ++it )
  {
    proj_assign_context( it.value(), tmpContext );
    proj_destroy( it.value() );
  }
  proj_context_destroy( tmpContext );
#else
  projCtx tmpContext = pj_ctx_alloc();
  for ( ; it != mProjProjections.constEnd(); ++it )
  {
    pj_set_ctx( it.value().first, tmpContext );
    pj_free( it.value().first );
    pj_set_ctx( it.value().second, tmpContext );
    pj_free( it.value().second );
  }
  pj_ctx_free( tmpContext );
#endif
  mProjProjections.clear();
}

#if PROJ_VERSION_MAJOR>=6
bool QgsCoordinateTransformPrivate::removeObjectsBelongingToCurrentThread( void *pj_context )
{
  QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );

  QMap < uintptr_t, ProjData >::iterator it = mProjProjections.find( reinterpret_cast< uintptr_t>( pj_context ) );
  if ( it != mProjProjections.end() )
  {
    proj_destroy( it.value() );
    mProjProjections.erase( it );
  }

  return mProjProjections.isEmpty();
}
#endif

///@endcond
