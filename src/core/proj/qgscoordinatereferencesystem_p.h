/***************************************************************************
                             qgscoordinatereferencesystem_p.h

                             --------------------------------
    begin                : 2016
    copyright            : (C) 2016 by Nyall Dawson
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
#ifndef QGSCOORDINATEREFERENCESYSTEM_PRIVATE_H
#define QGSCOORDINATEREFERENCESYSTEM_PRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgscoordinatereferencesystem.h"

#include <proj.h>
#include "qgsprojutils.h"
#include "qgsreadwritelocker.h"

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH;
#else
typedef void *OGRSpatialReferenceH;
#endif

class QgsCoordinateReferenceSystemPrivate : public QSharedData
{
  public:

    explicit QgsCoordinateReferenceSystemPrivate()
    {
    }

    QgsCoordinateReferenceSystemPrivate( const QgsCoordinateReferenceSystemPrivate &other )
      : QSharedData( other )
      , mSrsId( other.mSrsId )
      , mDescription( other.mDescription )
      , mProjectionAcronym( other.mProjectionAcronym )
      , mEllipsoidAcronym( other.mEllipsoidAcronym )
      , mIsGeographic( other.mIsGeographic )
      , mMapUnits( other.mMapUnits )
      , mSRID( other.mSRID )
      , mAuthId( other.mAuthId )
      , mIsValid( other.mIsValid )
      , mCoordinateEpoch( other.mCoordinateEpoch )
      , mPj()
      , mProj4( other.mProj4 )
      , mWktPreferred( other.mWktPreferred )
      , mAxisInvertedDirty( other.mAxisInvertedDirty )
      , mAxisInverted( other.mAxisInverted )
      , mProjObjects()
    {
    }

    ~QgsCoordinateReferenceSystemPrivate()
    {
      QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );
      if ( !mProjObjects.empty() || mPj )
      {
        locker.changeMode( QgsReadWriteLocker::Write );
        cleanPjObjects();
      }
    }

    //! The internal sqlite3 srs.db primary key for this CRS
    long mSrsId = 0;

    //! A textual description of the CRS
    QString mDescription;

    //! The official proj4 acronym for the projection family
    QString mProjectionAcronym;

    //! The official proj4 acronym for the ellipsoid
    QString mEllipsoidAcronym;

    //! Whether this is a geographic or projected coordinate system
    bool mIsGeographic = false;

    //! The map units for the CRS
    QgsUnitTypes::DistanceUnit mMapUnits = QgsUnitTypes::DistanceUnknownUnit;

    //! If available, the PostGIS spatial_ref_sys identifier for this CRS (defaults to 0)
    long mSRID = 0;

    //! If available the authority identifier for this CRS
    QString mAuthId;

    //! Whether this CRS is properly defined and valid
    bool mIsValid = false;

    //! Coordinate epoch
    double mCoordinateEpoch = std::numeric_limits< double >::quiet_NaN();

    // this is the "master" proj object, to be used as a template for new proj objects created on different threads ONLY.
    // Always use threadLocalProjObject() instead of this.

  private:
    QgsProjUtils::proj_pj_unique_ptr mPj;
    PJ_CONTEXT *mPjParentContext = nullptr;

    void cleanPjObjects()
    {

      // During destruction of PJ* objects, the errno is set in the underlying
      // context. Consequently the context attached to the PJ* must still exist !
      // Which is not necessarily the case currently unfortunately. So
      // create a temporary dummy context, and attach it to the PJ* before destroying
      // it
      PJ_CONTEXT *tmpContext = proj_context_create();
      for ( auto it = mProjObjects.begin(); it != mProjObjects.end(); ++it )
      {
        proj_assign_context( it.value(), tmpContext );
        proj_destroy( it.value() );
      }
      mProjObjects.clear();
      if ( mPj )
      {
        proj_assign_context( mPj.get(), tmpContext );
        mPj.reset();
      }
      proj_context_destroy( tmpContext );
    }

  public:

    void setPj( QgsProjUtils::proj_pj_unique_ptr obj )
    {
      const QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );
      cleanPjObjects();

      mPj = std::move( obj );
      mPjParentContext = QgsProjContext::get();
    }

    bool hasPj() const
    {
      const QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );
      return static_cast< bool >( mPj );
    }

    mutable QString mProj4;

    mutable QString mWktPreferred;

    //! True if presence of an inverted axis needs to be recalculated
    mutable bool mAxisInvertedDirty = false;

    //! Whether this is a coordinate system has inverted axis
    mutable bool mAxisInverted = false;

  private:
    mutable QReadWriteLock mProjLock{};
    mutable QMap < PJ_CONTEXT *, PJ * > mProjObjects{};

  public:

    PJ *threadLocalProjObject() const
    {
      QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Read );
      if ( !mPj )
        return nullptr;

      PJ_CONTEXT *context = QgsProjContext::get();
      const QMap < PJ_CONTEXT *, PJ * >::const_iterator it = mProjObjects.constFind( context );

      if ( it != mProjObjects.constEnd() )
      {
        return it.value();
      }

      // proj object doesn't exist yet, so we need to create
      locker.changeMode( QgsReadWriteLocker::Write );

      PJ *res = proj_clone( context, mPj.get() );
      mProjObjects.insert( context, res );
      return res;
    }

    // Only meant to be called by QgsCoordinateReferenceSystem::removeFromCacheObjectsBelongingToCurrentThread()
    bool removeObjectsBelongingToCurrentThread( PJ_CONTEXT *pj_context )
    {
      const QgsReadWriteLocker locker( mProjLock, QgsReadWriteLocker::Write );

      const QMap < PJ_CONTEXT *, PJ * >::iterator it = mProjObjects.find( pj_context );
      if ( it != mProjObjects.end() )
      {
        proj_destroy( it.value() );
        mProjObjects.erase( it );
      }

      if ( mPjParentContext == pj_context )
      {
        mPj.reset();
        mPjParentContext = nullptr;
      }

      return mProjObjects.isEmpty();
    }

  private:
    QgsCoordinateReferenceSystemPrivate &operator= ( const QgsCoordinateReferenceSystemPrivate & ) = delete;

};

/// @endcond

#endif //QGSCOORDINATEREFERENCESYSTEM_PRIVATE_H
