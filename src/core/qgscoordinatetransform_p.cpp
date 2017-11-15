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

QgsCoordinateTransformPrivate::QgsCoordinateTransformPrivate( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination )
  : mSourceCRS( source )
  , mDestCRS( destination )
{
  setFinder();
  initialize();
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

bool QgsCoordinateTransformPrivate::initialize()
{
  mShortCircuit = true;
  mIsValid = false;

  if ( !mSourceCRS.isValid() )
  {
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    QgsDebugMsgLevel( "Source CRS is invalid!", 4 );
    return false;
  }

  if ( !mDestCRS.isValid() )
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj.
    mDestCRS = mSourceCRS;
    QgsDebugMsgLevel( "Destination CRS is invalid!", 4 );
    return false;
  }

  mIsValid = true;

  bool useDefaultDatumTransform = ( mSourceDatumTransform == - 1 && mDestinationDatumTransform == -1 );

  // init the projections (destination and source)
  freeProj();

  mSourceProjString = mSourceCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    mSourceProjString = stripDatumTransform( mSourceProjString );
  }
  if ( mSourceDatumTransform != -1 )
  {
    mSourceProjString += ( ' ' + datumTransformString( mSourceDatumTransform ) );
  }

  mDestProjString = mDestCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    mDestProjString = stripDatumTransform( mDestProjString );
  }
  if ( mDestinationDatumTransform != -1 )
  {
    mDestProjString += ( ' ' +  datumTransformString( mDestinationDatumTransform ) );
  }

  if ( !useDefaultDatumTransform )
  {
    addNullGridShifts( mSourceProjString, mDestProjString );
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
    QgsDebugMsg( "------------------------------------------------------------" );
    QgsDebugMsg( "The OGR Coordinate transformation for this layer was set to" );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Input", mSourceCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Output", mDestCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsDebugMsg( "------------------------------------------------------------" );
  }
  else
  {
    QgsDebugMsg( "------------------------------------------------------------" );
    QgsDebugMsg( "The OGR Coordinate transformation FAILED TO INITIALIZE!" );
    QgsDebugMsg( "------------------------------------------------------------" );
  }
#else
  if ( !mIsValid )
  {
    QgsDebugMsg( "Coordinate transformation failed to initialize!" );
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
    QgsDebugMsgLevel( "Source/Dest CRS equal, shortcircuit is set.", 3 );
  }
  else
  {
    // Transform must take place
    mShortCircuit = false;
    QgsDebugMsgLevel( "Source/Dest CRS not equal, shortcircuit is not set.", 3 );
  }
  return mIsValid;
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

QString QgsCoordinateTransformPrivate::datumTransformString( int datumTransform )
{
  QString transformString;

  sqlite3_database_unique_ptr database;
  int openResult = database.open_v2( QgsApplication::srsDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( openResult != SQLITE_OK )
  {
    return transformString;
  }

  sqlite3_statement_unique_ptr statement;
  QString sql = QStringLiteral( "SELECT coord_op_method_code,p1,p2,p3,p4,p5,p6,p7 FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
  int prepareRes;
  statement = database.prepare( sql, prepareRes );
  if ( prepareRes != SQLITE_OK )
  {
    return transformString;
  }

  if ( statement.step() == SQLITE_ROW )
  {
    //coord_op_methode_code
    int methodCode = statement.columnAsInt64( 0 );
    if ( methodCode == 9615 ) //ntv2
    {
      transformString = "+nadgrids=" + statement.columnAsText( 1 );
    }
    else if ( methodCode == 9603 || methodCode == 9606 || methodCode == 9607 )
    {
      transformString += QLatin1String( "+towgs84=" );
      double p1 = statement.columnAsDouble( 1 );
      double p2 = statement.columnAsDouble( 2 );
      double p3 = statement.columnAsDouble( 3 );
      double p4 = statement.columnAsDouble( 4 );
      double p5 = statement.columnAsDouble( 5 );
      double p6 = statement.columnAsDouble( 6 );
      double p7 = statement.columnAsDouble( 7 );
      if ( methodCode == 9603 ) //3 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ) );
      }
      else //7 parameter transformation
      {
        transformString += QStringLiteral( "%1,%2,%3,%4,%5,%6,%7" ).arg( QString::number( p1 ), QString::number( p2 ), QString::number( p3 ), QString::number( p4 ), QString::number( p5 ), QString::number( p6 ), QString::number( p7 ) );
      }
    }
  }

  return transformString;
}

void QgsCoordinateTransformPrivate::addNullGridShifts( QString &srcProjString, QString &destProjString ) const
{
  //if one transformation uses ntv2, the other one needs to be null grid shift
  if ( mDestinationDatumTransform == -1 && srcProjString.contains( QLatin1String( "+nadgrids" ) ) ) //add null grid if source transformation is ntv2
  {
    destProjString += QLatin1String( " +nadgrids=@null" );
    return;
  }
  if ( mSourceDatumTransform == -1 && destProjString.contains( QLatin1String( "+nadgrids" ) ) )
  {
    srcProjString += QLatin1String( " +nadgrids=@null" );
    return;
  }

  //add null shift grid for google mercator
  //(see e.g. http://trac.osgeo.org/proj/wiki/FAQ#ChangingEllipsoidWhycantIconvertfromWGS84toGoogleEarthVirtualGlobeMercator)
  if ( mSourceCRS.authid().compare( QLatin1String( "EPSG:3857" ), Qt::CaseInsensitive ) == 0 && mSourceDatumTransform == -1 )
  {
    srcProjString += QLatin1String( " +nadgrids=@null" );
  }
  if ( mDestCRS.authid().compare( QLatin1String( "EPSG:3857" ), Qt::CaseInsensitive ) == 0 && mDestinationDatumTransform == -1 )
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
