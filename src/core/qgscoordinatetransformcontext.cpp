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
#include "qgscoordinatetransformcontext_p.h"
QgsCoordinateTransformContext::QgsCoordinateTransformContext()
  : d( new QgsCoordinateTransformContextPrivate() )
{}

QgsCoordinateTransformContext::QgsCoordinateTransformContext( const QgsCoordinateTransformContext &rhs ) //NOLINT
  : d( rhs.d )
{}

QgsCoordinateTransformContext &QgsCoordinateTransformContext::operator=( const QgsCoordinateTransformContext &rhs ) //NOLINT
{
  d = rhs.d;
  return *this;
}

void QgsCoordinateTransformContext::clear()
{
  d.detach();
  d->mSourceDestDatumTransforms.clear();
  d->mSourceDatumTransforms.clear();
  d->mDestDatumTransforms.clear();
}

QMap<QString, int> QgsCoordinateTransformContext::sourceDatumTransforms() const
{
  return d->mSourceDatumTransforms;
}

bool QgsCoordinateTransformContext::addSourceDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  d.detach();
  if ( transform == -1 )
  {
    d->mSourceDatumTransforms.remove( crs.authid() );
    return true;
  }

  d->mSourceDatumTransforms.insert( crs.authid(), transform );
  return true;
}

QMap<QString, int> QgsCoordinateTransformContext::destinationDatumTransforms() const
{
  return d->mDestDatumTransforms;
}

bool QgsCoordinateTransformContext::addDestinationDatumTransform( const QgsCoordinateReferenceSystem &crs, int transform )
{
  if ( !crs.isValid() )
    return false;

  d.detach();
  if ( transform == -1 )
  {
    d->mDestDatumTransforms.remove( crs.authid() );
    return true;
  }

  d->mDestDatumTransforms.insert( crs.authid(), transform );
  return true;
}

QMap<QPair<QString, QString>, QPair<int, int> > QgsCoordinateTransformContext::sourceDestinationDatumTransforms() const
{
  return d->mSourceDestDatumTransforms;
}

bool QgsCoordinateTransformContext::addSourceDestinationDatumTransform( const QgsCoordinateReferenceSystem &sourceCrs, const QgsCoordinateReferenceSystem &destinationCrs, int sourceTransform, int destinationTransform )
{
  if ( !sourceCrs.isValid() || !destinationCrs.isValid() )
    return false;

  d.detach();
  if ( sourceTransform == -1 || destinationTransform == -1 )
  {
    d->mSourceDestDatumTransforms.remove( qMakePair( sourceCrs.authid(), destinationCrs.authid() ) );
    return true;
  }

  d->mSourceDestDatumTransforms.insert( qMakePair( sourceCrs.authid(), destinationCrs.authid() ), qMakePair( sourceTransform, destinationTransform ) );
  return true;
}

QPair<int, int> QgsCoordinateTransformContext::calculateDatumTransforms( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination ) const
{

}
