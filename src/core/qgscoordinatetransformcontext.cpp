/***************************************************************************
                         qgscoordinatetransformcontext.cpp
                         ---------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgscoordinatetransformcontext.h"

void QgsCoordinateTransformContext::clear()
{
  mSourceDestDatumTransforms.clear();
  mSourceDatumTransforms.clear();
  mDestDatumTransforms.clear();
}

QMap<QString, int> QgsCoordinateTransformContext::sourceDatumTransforms() const
{
  return mSourceDatumTransforms;
}

bool QgsCoordinateTransformContext::addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  mSourceDatumTransforms.insert( crs.authid(), transform );
  return true;
}

QMap<QString, int> QgsCoordinateTransformContext::destinationDatumTransforms() const
{
  return mDestDatumTransforms;
}

bool QgsCoordinateTransformContext::addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  mDestDatumTransforms.insert( crs.authid(), transform );
  return true;
}

QMap<QPair<QString, QString>, QPair<int, int> > QgsCoordinateTransformContext::sourceDestinationDatumTransforms() const
{
  return mSourceDestDatumTransforms;
}

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;

  mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), qMakePair( sourceTransform, destinationTransform ) );
  return true;
}

QPair<int, int> QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{

}
