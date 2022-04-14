/***************************************************************************
                         qgsabstractprofilegenerator.cpp
                         ---------------
    begin                : March 2022
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
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilesnapping.h"


QgsProfileRenderContext::QgsProfileRenderContext( QgsRenderContext &context )
  : mRenderContext( context )
{

}

const QTransform &QgsProfileRenderContext::worldTransform() const
{
  return mWorldTransform;
}

void QgsProfileRenderContext::setWorldTransform( const QTransform &transform )
{
  mWorldTransform = transform;
}

QgsDoubleRange QgsProfileRenderContext::distanceRange() const
{
  return mDistanceRange;
}

void QgsProfileRenderContext::setDistanceRange( const QgsDoubleRange &range )
{
  mDistanceRange = range;
}

QgsDoubleRange QgsProfileRenderContext::elevationRange() const
{
  return mElevationRange;
}

void QgsProfileRenderContext::setElevationRange( const QgsDoubleRange &range )
{
  mElevationRange = range;
}


QgsAbstractProfileGenerator::~QgsAbstractProfileGenerator() = default;

QgsAbstractProfileResults::~QgsAbstractProfileResults() = default;

QgsProfileSnapResult QgsAbstractProfileResults::snapPoint( const QgsProfilePoint &, double, double )
{
  return QgsProfileSnapResult();
}
