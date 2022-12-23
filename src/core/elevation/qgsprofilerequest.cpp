/***************************************************************************
                         qgsprofilerequest.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#include "qgsprofilerequest.h"
#include "qgscurve.h"
#include "qgsterrainprovider.h"

QgsProfileRequest::QgsProfileRequest( QgsCurve *curve )
  : mCurve( curve )
{

}

QgsProfileRequest::~QgsProfileRequest() = default;

QgsProfileRequest::QgsProfileRequest( const QgsProfileRequest &other )
  : mCurve( other.mCurve ? other.mCurve->clone() : nullptr )
  , mCrs( other.mCrs )
  , mTransformContext( other.mTransformContext )
  , mTolerance( other.mTolerance )
  , mStepDistance( other.mStepDistance )
  , mTerrainProvider( other.mTerrainProvider ? other.mTerrainProvider->clone() : nullptr )
  , mExpressionContext( other.mExpressionContext )
{

}

QgsProfileRequest &QgsProfileRequest::operator=( const QgsProfileRequest &other )
{
  mCurve.reset( other.mCurve ? other.mCurve->clone() : nullptr );
  mCrs = other.mCrs;
  mTransformContext = other.mTransformContext;
  mTolerance = other.mTolerance;
  mStepDistance = other.mStepDistance;
  mTerrainProvider.reset( other.mTerrainProvider ? other.mTerrainProvider->clone() : nullptr );
  mExpressionContext = other.mExpressionContext;
  return *this;
}

bool QgsProfileRequest::operator==( const QgsProfileRequest &other ) const
{
  if ( !qgsDoubleNear( mTolerance, other.mTolerance )
       || !qgsDoubleNear( mStepDistance, other.mStepDistance )
       || mCrs != other.mCrs
       || !( mTransformContext == other.mTransformContext ) )
    return false;

  if ( ( !mCurve && other.mCurve )
       || ( mCurve && !other.mCurve ) )
  {
    return false;
  }
  else if ( mCurve && other.mCurve )
  {
    if ( !mCurve->equals( *other.mCurve ) )
      return false;
  }

  if ( ( mTerrainProvider && !other.mTerrainProvider )
       || ( !mTerrainProvider && other.mTerrainProvider ) )
  {
    return false;
  }
  else if ( mTerrainProvider && other.mTerrainProvider )
  {
    if ( !mTerrainProvider->equals( other.mTerrainProvider.get() ) )
      return false;
  }

  return true;
}

bool QgsProfileRequest::operator!=( const QgsProfileRequest &other ) const
{
  return !( *this == other );
}

QgsProfileRequest &QgsProfileRequest::setProfileCurve( QgsCurve *curve )
{
  mCurve.reset( curve );
  return *this;
}

QgsCurve *QgsProfileRequest::profileCurve() const
{
  return mCurve.get();
}

QgsProfileRequest &QgsProfileRequest::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
  return *this;
}

QgsCoordinateReferenceSystem QgsProfileRequest::crs() const
{
  return mCrs;
}

QgsCoordinateTransformContext QgsProfileRequest::transformContext() const
{
  return mTransformContext;
}

QgsProfileRequest &QgsProfileRequest::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
  return *this;
}

QgsProfileRequest &QgsProfileRequest::setTolerance( double tolerance )
{
  mTolerance = tolerance;
  return *this;
}

QgsProfileRequest &QgsProfileRequest::setTerrainProvider( QgsAbstractTerrainProvider *provider )
{
  mTerrainProvider.reset( provider );
  return *this;
}

QgsAbstractTerrainProvider *QgsProfileRequest::terrainProvider() const
{
  return mTerrainProvider.get();
}

QgsProfileRequest &QgsProfileRequest::setStepDistance( double distance )
{
  mStepDistance = distance;
  return *this;
}

QgsProfileRequest &QgsProfileRequest::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
  return *this;
}

