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
#include <ogr_srs_api.h>

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH;
#else
typedef void *OGRSpatialReferenceH;
#endif

class QgsCoordinateReferenceSystemPrivate : public QSharedData
{
  public:

    explicit QgsCoordinateReferenceSystemPrivate()
      : mCRS( OSRNewSpatialReference( nullptr ) )
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
      , mCRS( nullptr )
      , mValidationHint( other.mValidationHint )
      , mWkt( other.mWkt )
      , mProj4( other.mProj4 )
      , mAxisInvertedDirty( other.mAxisInvertedDirty )
      , mAxisInverted( other.mAxisInverted )
    {
      if ( mIsValid )
      {
        mCRS = OSRClone( other.mCRS );
      }
      else
      {
        mCRS = OSRNewSpatialReference( nullptr );
      }
    }

    ~QgsCoordinateReferenceSystemPrivate()
    {
      OSRDestroySpatialReference( mCRS );
    }

    //! The internal sqlite3 srs.db primary key for this CRS
    long mSrsId = 0;

    //! A textual description of the CRS
    QString mDescription;

    //! The official proj4 acronym for the projection family
    QString mProjectionAcronym;

    //! The official proj4 acronym for the ellipoid
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
    bool mIsValid = 0;

    OGRSpatialReferenceH mCRS;

    QString mValidationHint;
    mutable QString mWkt;
    mutable QString mProj4;

    //! True if presence of an inverted axis needs to be recalculated
    mutable bool mAxisInvertedDirty = false;

    //! Whether this is a coordinate system has inverted axis
    mutable bool mAxisInverted = false;

};

/// @endcond

#endif //QGSCOORDINATEREFERENCESYSTEM_PRIVATE_H
