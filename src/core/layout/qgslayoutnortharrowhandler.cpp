/***************************************************************************
                         qgslayoutnortharrowhandler.cpp
                         -------------------
begin                : April 2020
copyright            : (C) 2020 by Nyall Dawson
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

#include "qgslayoutnortharrowhandler.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgsbearingutils.h"
#include "qgslogger.h"

QgsLayoutNorthArrowHandler::QgsLayoutNorthArrowHandler( QObject *parent )
  : QObject( parent )
{

}

void QgsLayoutNorthArrowHandler::disconnectMap( QgsLayoutItemMap *map )
{
  if ( map )
  {
    disconnect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
    disconnect( map, &QgsLayoutItemMap::rotationChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
    disconnect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
  }
}

void QgsLayoutNorthArrowHandler::updateMapRotation()
{
  if ( !mRotationMap )
    return;

  // take map rotation
  double rotation = mRotationMap->mapRotation() + mRotationMap->rotation();

  // handle true north
  switch ( mNorthMode )
  {
    case GridNorth:
      break; // nothing to do

    case TrueNorth:
    {
      const QgsPointXY center = mRotationMap->extent().center();
      const QgsCoordinateReferenceSystem crs = mRotationMap->crs();
      const QgsCoordinateTransformContext transformContext = mRotationMap->layout()->project()->transformContext();

      try
      {
        const double bearing = QgsBearingUtils::bearingTrueNorth( crs, transformContext, center );
        rotation += bearing;
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e )
        QgsDebugMsg( QStringLiteral( "Caught exception %1" ).arg( e.what() ) );
      }
      break;
    }
  }

  rotation += mNorthOffset;
  const double oldRotation = mArrowRotation;
  mArrowRotation = ( rotation > 360.0 ) ? rotation - 360.0 : rotation ;
  if ( mArrowRotation != oldRotation )
    emit arrowRotationChanged( mArrowRotation );
}

void QgsLayoutNorthArrowHandler::setLinkedMap( QgsLayoutItemMap *map )
{
  if ( mRotationMap )
  {
    disconnectMap( mRotationMap );
  }

  if ( !map ) //disable rotation from map
  {
    mRotationMap = nullptr;
    if ( mArrowRotation != 0 )
    {
      mArrowRotation = 0;
      emit arrowRotationChanged( mArrowRotation );
    }
  }
  else
  {
    connect( map, &QgsLayoutItemMap::mapRotationChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
    connect( map, &QgsLayoutItemMap::rotationChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
    connect( map, &QgsLayoutItemMap::extentChanged, this, &QgsLayoutNorthArrowHandler::updateMapRotation );
    mRotationMap = map;
    updateMapRotation();
  }
}

QgsLayoutItemMap *QgsLayoutNorthArrowHandler::linkedMap() const
{
  return mRotationMap;
}

void QgsLayoutNorthArrowHandler::setNorthMode( QgsLayoutNorthArrowHandler::NorthMode mode )
{
  mNorthMode = mode;
  updateMapRotation();
}

void QgsLayoutNorthArrowHandler::setNorthOffset( double offset )
{
  mNorthOffset = offset;
  updateMapRotation();
}
