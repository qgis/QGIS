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

#include "qgsprojutils.h"
#include <proj.h>
#include <proj_experimental.h>

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

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs )> QgsCoordinateTransformPrivate::sDynamicCrsToDynamicCrsWarningHandler = nullptr;

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
  if ( mSourceCRS != mDestCRS )
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
  , mAvailableOpCount( other.mAvailableOpCount )
  , mIsValid( other.mIsValid )
  , mShortCircuit( other.mShortCircuit )
  , mGeographicToWebMercator( other.mGeographicToWebMercator )
  , mSourceCRS( other.mSourceCRS )
  , mDestCRS( other.mDestCRS )
  , mSourceDatumTransform( other.mSourceDatumTransform )
  , mDestinationDatumTransform( other.mDestinationDatumTransform )
  , mProjCoordinateOperation( other.mProjCoordinateOperation )
  , mShouldReverseCoordinateOperation( other.mShouldReverseCoordinateOperation )
  , mAllowFallbackTransforms( other.mAllowFallbackTransforms )
  , mSourceIsDynamic( other.mSourceIsDynamic )
  , mDestIsDynamic( other.mDestIsDynamic )
  , mSourceCoordinateEpoch( other.mSourceCoordinateEpoch )
  , mDestCoordinateEpoch( other.mDestCoordinateEpoch )
  , mDefaultTime( other.mDefaultTime )
  , mIsReversed( other.mIsReversed )
  , mProjLock()
  , mProjProjections()
  , mProjFallbackProjections()
{
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
  mAvailableOpCount = -1;
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

  if ( mSourceCRS == mDestCRS )
  {
    // If the source and destination projection are the same, set the short
    // circuit flag (no transform takes place)
    mShortCircuit = true;
    return true;
  }

  mGeographicToWebMercator =
    mSourceCRS.isGeographic() &&
    mDestCRS.authid() == QLatin1String( "EPSG:3857" );

  mSourceIsDynamic = mSourceCRS.isDynamic();
  mSourceCoordinateEpoch = mSourceCRS.coordinateEpoch();
  mDestIsDynamic = mDestCRS.isDynamic();
  mDestCoordinateEpoch = mDestCRS.coordinateEpoch();

  // Determine the default coordinate epoch.
  // For time-dependent transformations, PROJ can currently only do
  // staticCRS -> dynamicCRS or dynamicCRS -> staticCRS transformations, and
  // in either case, the coordinate epoch of the dynamicCRS must be provided
  // as the input time.
  mDefaultTime = ( mSourceIsDynamic && !std::isnan( mSourceCoordinateEpoch ) && !mDestIsDynamic )
                 ? mSourceCoordinateEpoch
                 : ( mDestIsDynamic && !std::isnan( mDestCoordinateEpoch ) && !mSourceIsDynamic )
                 ? mDestCoordinateEpoch : std::numeric_limits< double >::quiet_NaN();

  if ( mSourceIsDynamic && mDestIsDynamic && !qgsNanCompatibleEquals( mSourceCoordinateEpoch, mDestCoordinateEpoch ) )
  {
    // transforms from dynamic crs to dynamic crs with different coordinate epochs are not yet supported by PROJ
    if ( sDynamicCrsToDynamicCrsWarningHandler )
    {
      sDynamicCrsToDynamicCrsWarningHandler( mSourceCRS, mDestCRS );
    }
  }

  // init the projections (destination and source)
  freeProj();

  // create proj projections for current thread
  ProjData res = threadLocalProjData();

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "From proj : " + mSourceCRS.toProj() );
  QgsDebugMsg( "To proj   : " + mDestCRS.toProj() );
#endif

  if ( !res )
    mIsValid = false;

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

  // Transform must take place
  mShortCircuit = false;

  return mIsValid;
}

void QgsCoordinateTransformPrivate::calculateTransforms( const QgsCoordinateTransformContext &context )
{
  // recalculate datum transforms from context
  if ( mSourceCRS.isValid() && mDestCRS.isValid() )
  {
    mProjCoordinateOperation = context.calculateCoordinateOperation( mSourceCRS, mDestCRS );
    mShouldReverseCoordinateOperation = context.mustReverseCoordinateOperation( mSourceCRS, mDestCRS );
    mAllowFallbackTransforms = context.allowFallbackTransform( mSourceCRS, mDestCRS );
  }
  else
  {
    mProjCoordinateOperation.clear();
    mShouldReverseCoordinateOperation = false;
    mAllowFallbackTransforms = false;
  }
}

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
    const QString messageString( message );
    if ( messageString == QLatin1String( "push: Invalid latitude" ) )
    {
      // these messages tend to spam the console as they can be repeated 1000s of times
      QgsDebugMsgLevel( messageString, 3 );
    }
    else
    {
      QgsDebugMsg( messageString );
    }
  }
  else if ( level == PJ_LOG_DEBUG )
  {
    QgsDebugMsgLevel( QString( message ), 3 );
  }
}

ProjData QgsCoordinateTransformPrivate::threadLocalProjData()
{
  QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );

  PJ_CONTEXT *context = QgsProjContext::get();
  const QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constFind( reinterpret_cast< uintptr_t>( context ) );

  if ( it != mProjProjections.constEnd() )
  {
    ProjData res = it.value();
    return res;
  }

  // proj projections don't exist yet, so we need to create
  locker.changeMode( QgsReadWriteLocker::Write );

  // use a temporary proj error collector
  QStringList projErrors;
  proj_log_func( context, &projErrors, proj_collecting_logger );

  mIsReversed = false;

  QgsProjUtils::proj_pj_unique_ptr transform;
  if ( !mProjCoordinateOperation.isEmpty() )
  {
    transform.reset( proj_create( context, mProjCoordinateOperation.toUtf8().constData() ) );
    // Only use proj_coordoperation_is_instantiable() if PROJ networking is enabled.
    // The reason is that proj_coordoperation_is_instantiable() in PROJ < 9.0
    // does not work properly when a coordinate operation refers to a PROJ < 7 grid name (gtx/gsb)
    // but the user has installed PROJ >= 7 GeoTIFF grids.
    // Cf https://github.com/OSGeo/PROJ/pull/3025.
    // When networking is not enabled, proj_create() will check that all grids are
    // present, so proj_coordoperation_is_instantiable() is not necessary.
    if ( !transform
         || (
           proj_context_is_network_enabled( context ) &&
           !proj_coordoperation_is_instantiable( context, transform.get() ) )
       )
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
        QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
      }

      transform.reset();
    }
    else
    {
      mIsReversed = mShouldReverseCoordinateOperation;
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
      mAvailableOpCount = proj_list_get_count( ops );
      if ( mAvailableOpCount < 1 )
      {
        // huh?
        const int errNo = proj_context_errno( context );
        if ( errNo && errNo != -61 )
        {
          nonAvailableError = QString( proj_errno_string( errNo ) );
        }
        else
        {
          nonAvailableError = QObject::tr( "No coordinate operations are available between these two reference systems" );
        }
      }
      else if ( mAvailableOpCount == 1 )
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
                  QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
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
              QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
            }
          }
        }
      }
      else
      {
        // multiple operations available. Can we use the best one?
        QgsDatumTransform::TransformDetails preferred;
        bool missingPreferred = false;
        bool stillLookingForPreferred = true;
        for ( int i = 0; i < mAvailableOpCount; ++ i )
        {
          transform.reset( proj_list_get( context, ops, i ) );
          const bool isInstantiable = transform && proj_coordoperation_is_instantiable( context, transform.get() );
          if ( stillLookingForPreferred && transform && !isInstantiable )
          {
            // uh oh :( something is missing blocking us from the preferred operation!
            const QgsDatumTransform::TransformDetails candidate = QgsDatumTransform::transformDetailsFromPj( transform.get() );
            if ( !candidate.proj.isEmpty() )
            {
              preferred = candidate;
              missingPreferred = true;
              stillLookingForPreferred = false;
            }
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
          const QgsDatumTransform::TransformDetails available = QgsDatumTransform::transformDetailsFromPj( transform.get() );
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
            QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
          }
        }

        // transform may have either the source or destination CRS using swapped axis order. For QGIS, we ALWAYS need regular x/y axis order
        if ( transform )
          transform.reset( proj_normalize_for_visualization( context, transform.get() ) );
        if ( !transform )
        {
          const QString err = QObject::tr( "Cannot normalize transform between %1 and %2" ).arg( mSourceCRS.authid(),
                              mDestCRS.authid() );
          QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
        }
      }
      proj_list_destroy( ops );
    }
    proj_operation_factory_context_destroy( operationContext );
  }

  if ( !transform && nonAvailableError.isEmpty() )
  {
    const int errNo = proj_context_errno( context );
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
      QgsMessageLog::logMessage( err, QString(), Qgis::MessageLevel::Critical );
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
  return res;
}

ProjData QgsCoordinateTransformPrivate::threadLocalFallbackProjData()
{
  QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );

  PJ_CONTEXT *context = QgsProjContext::get();
  const QMap < uintptr_t, ProjData >::const_iterator it = mProjFallbackProjections.constFind( reinterpret_cast< uintptr_t>( context ) );

  if ( it != mProjFallbackProjections.constEnd() )
  {
    ProjData res = it.value();
    return res;
  }

  // proj projections don't exist yet, so we need to create
  locker.changeMode( QgsReadWriteLocker::Write );

  QgsProjUtils::proj_pj_unique_ptr transform( proj_create_crs_to_crs_from_pj( context, mSourceCRS.projObject(), mDestCRS.projObject(), nullptr, nullptr ) );
  if ( transform )
    transform.reset( proj_normalize_for_visualization( QgsProjContext::get(), transform.get() ) );

  ProjData res = transform.release();
  mProjFallbackProjections.insert( reinterpret_cast< uintptr_t>( context ), res );
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

void QgsCoordinateTransformPrivate::setDynamicCrsToDynamicCrsWarningHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem & )> &handler )
{
  sDynamicCrsToDynamicCrsWarningHandler = handler;
}

void QgsCoordinateTransformPrivate::freeProj()
{
  const QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );
  if ( mProjProjections.isEmpty() && mProjFallbackProjections.isEmpty() )
    return;
  QMap < uintptr_t, ProjData >::const_iterator it = mProjProjections.constBegin();

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

  it = mProjFallbackProjections.constBegin();
  for ( ; it != mProjFallbackProjections.constEnd(); ++it )
  {
    proj_assign_context( it.value(), tmpContext );
    proj_destroy( it.value() );
  }

  proj_context_destroy( tmpContext );
  mProjProjections.clear();
  mProjFallbackProjections.clear();
}

bool QgsCoordinateTransformPrivate::removeObjectsBelongingToCurrentThread( void *pj_context )
{
  const QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );

  QMap < uintptr_t, ProjData >::iterator it = mProjProjections.find( reinterpret_cast< uintptr_t>( pj_context ) );
  if ( it != mProjProjections.end() )
  {
    proj_destroy( it.value() );
    mProjProjections.erase( it );
  }

  it = mProjFallbackProjections.find( reinterpret_cast< uintptr_t>( pj_context ) );
  if ( it != mProjFallbackProjections.end() )
  {
    proj_destroy( it.value() );
    mProjFallbackProjections.erase( it );
  }

  return mProjProjections.isEmpty();
}

///@endcond
