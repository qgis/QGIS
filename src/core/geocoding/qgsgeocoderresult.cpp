/***************************************************************************
  qgsgeocoderresult.cpp
  ---------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
