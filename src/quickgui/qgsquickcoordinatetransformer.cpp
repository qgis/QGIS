/***************************************************************************
 qgsquickcoordinatetransformer.cpp
  --------------------------------------
  Date                 : 1.6.2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                :  matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtDebug>

#include "qgsquickcoordinatetransformer.h"

QgsQuickCoordinateTransformer::QgsQuickCoordinateTransformer( QObject *parent )
  : QObject( parent )
{
  mCoordinateTransform.setSourceCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
  mCoordinateTransform.setContext( QgsCoordinateTransformContext() );
}

QgsPoint QgsQuickCoordinateTransformer::projectedPosition() const
{
  return mProjectedPosition;
}

QgsPoint QgsQuickCoordinateTransformer::sourcePosition() const
{
  return mSourcePosition;
}

void QgsQuickCoordinateTransformer::setSourcePosition( QgsPoint sourcePosition )
{
  if ( mSourcePosition == sourcePosition )
    return;

  mSourcePosition = sourcePosition;

  emit sourcePositionChanged();
  updatePosition();
}

QgsCoordinateReferenceSystem QgsQuickCoordinateTransformer::destinationCrs() const
{
  return mCoordinateTransform.destinationCrs();
}

void QgsQuickCoordinateTransformer::setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs )
{
  if ( destinationCrs == mCoordinateTransform.destinationCrs() )
    return;

  mCoordinateTransform.setDestinationCrs( destinationCrs );
  emit destinationCrsChanged();
  updatePosition();
}

QgsCoordinateReferenceSystem QgsQuickCoordinateTransformer::sourceCrs() const
{
  return mCoordinateTransform.sourceCrs();
}

void QgsQuickCoordinateTransformer::setSourceCrs( const QgsCoordinateReferenceSystem &sourceCrs )
{
  if ( sourceCrs == mCoordinateTransform.sourceCrs() )
    return;

  mCoordinateTransform.setSourceCrs( sourceCrs );

  emit sourceCrsChanged();
  updatePosition();
}

void QgsQuickCoordinateTransformer::updatePosition()
{
  double x = mSourcePosition.x();
  double y = mSourcePosition.y();
  double z = mSourcePosition.z();

  // If Z is NaN, coordinate transformation (proj4) will
  // also set X and Y to NaN. But we also want to get projected
  // coords if we do not have any Z coordinate.
  if ( qIsNaN( z ) )
  {
    z = 0;
  }

  if ( mMapSettings )
    mCoordinateTransform.setContext( mMapSettings->transformContext() );

  mCoordinateTransform.transformInPlace( x, y, z );

  mProjectedPosition = QgsPoint( x, y );
  mProjectedPosition.addZValue( mSourcePosition.z() );

  emit projectedPositionChanged();
}
