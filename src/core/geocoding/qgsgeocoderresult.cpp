/***************************************************************************
  qgsgeocoderresult.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocoderresult.h"

QgsGeocoderResult QgsGeocoderResult::errorResult( const QString &errorMessage )
{
  QgsGeocoderResult result;
  result.mIsValid = false;
  result.mErrorString = errorMessage;
  return result;
}

QgsGeocoderResult::QgsGeocoderResult( const QString &identifier, const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs )
  : mIsValid( true )
  , mIdentifier( identifier )
  , mGeometry( geometry )
  , mCrs( crs )
{}

QString QgsGeocoderResult::group() const
{
  return mGroup;
}

void QgsGeocoderResult::setGroup( const QString &group )
{
  mGroup = group;
}

QString QgsGeocoderResult::description() const
{
  return mDescription;
}

void QgsGeocoderResult::setDescription( const QString &description )
{
  mDescription = description;
}
