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

extern "C"
{
#include <proj_api.h>
}
#include <sqlite3.h>

#include <QStringList>

/// @cond PRIVATE

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

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate()
{
  setFinder();
}

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source,
    const QgsCoordinateReferenceSystem &destination,
    const QgsCoordinateTransformContext &context )
  : mSourceCRS( source )
  , mDestCRS( destination )
{
  setFinder();
  calculateTransforms( context );
}

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, int sourceDatumTransform, int destDatumTransform )
  : mSourceCRS( source )
  , mDestCRS( destination )
  , mSourceDatumTransform( sourceDatumTransform )
  , mDestinationDatumTransform( destDatumTransform )
{
  setFinder();
}

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateTransformPrivate &other )
  : QSharedData( other )
  , mIsValid( other.mIsValid )
  , mShortCircuit( other.mShortCircuit )
  , mSourceCRS( other.mSourceCRS )
  , mDestCRS( other.mDestCRS )
  , mSourceDatumTransform( other.mSourceDatumTransform )
  , mDestinationDatumTransform( other.mDestinationDatumTransform )
{
  //must reinitialize to setup mSourceProjection and mDestinationProjection
  initialize();
}

QgsCoordinateTransformPrivate::~QgsCoordinateTransformPrivate()
{
  // free the proj objects
  freeProj();
}

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

  int sourceDatumTransform = mSourceDatumTransform;
  int destDatumTransform = mDestinationDatumTransform;
  bool useDefaultDatumTransform = ( sourceDatumTransform == - 1 && destDatumTransform == -1 );

  // init the projections (destination and source)
  freeProj();

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

  // create proj projections for current thread
  QPair<projPJ, projPJ> res = threadLocalProjData();

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "From proj : " + mSourceCRS.toProj4() );
  QgsDebugMsg( "To proj   : " + mDestCRS.toProj4() );
#endif

  if ( !res.first || !res.second )
  {
    mIsValid = false;
  }

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
  QgsDatumTransform::TransformPair transforms = context.calculateDatumTransforms( mSourceCRS, mDestCRS );
  mSourceDatumTransform = transforms.sourceTransformId;
  mDestinationDatumTransform = transforms.destinationTransformId;
}

QPair<projPJ, projPJ> QgsCoordinateTransformPrivate::threadLocalProjData()
{
  mProjLock.lockForRead();

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

  if ( it != mProjProjections.constEnd() )
  {
    QPair<projPJ, projPJ> res = it.value();
    mProjLock.unlock();
    return res;
  }

  // proj projections don't exist yet, so we need to create
  mProjLock.unlock();
  mProjLock.lockForWrite();

#ifdef USE_THREAD_LOCAL
  QPair<projPJ, projPJ> res = qMakePair( pj_init_plus_ctx( mProjContext.get(), mSourceProjString.toUtf8() ),
                                         pj_init_plus_ctx( mProjContext.get(), mDestProjString.toUtf8() ) );
  mProjProjections.insert( reinterpret_cast< uintptr_t>( mProjContext.get() ), res );
#else
  QPair<projPJ, projPJ> res = qMakePair( pj_init_plus_ctx( pContext, mSourceProjString.toUtf8() ),
                                         pj_init_plus_ctx( pContext, mDestProjString.toUtf8() ) );
  mProjProjections.insert( reinterpret_cast< uintptr_t>( pContext ), res );
#endif
  mProjLock.unlock();
  return res;
}

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

void QgsCoordinateTransformPrivate::setFinder()
{
#if 0
  // Attention! It should be possible to set PROJ_LIB
  // but it can happen that it was previously set by installer
  // (version 0.7) and the old installation was deleted

  // Another problem: PROJ checks if pj_finder was set before
  // PROJ_LIB environment variable. pj_finder is probably set in
  // GRASS gproj library when plugin is loaded, consequently
  // PROJ_LIB is ignored

  pj_set_finder( finder );
#endif
}

void QgsCoordinateTransformPrivate::freeProj()
{
  mProjLock.lockForWrite();
  QMap < uintptr_t, QPair< projPJ, projPJ > >::const_iterator it = mProjProjections.constBegin();
  for ( ; it != mProjProjections.constEnd(); ++it )
  {
    pj_free( it.value().first );
    pj_free( it.value().second );
  }
  mProjProjections.clear();
  mProjLock.unlock();
}

///@endcond
