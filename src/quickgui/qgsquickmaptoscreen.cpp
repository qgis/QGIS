/***************************************************************************
  qgsquickmaptoscreen.cpp
  ----------------------------------------------------
  Date                 : 22.08.2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickmaptoscreen.h"
#include "moc_qgsquickmaptoscreen.cpp"
#include "qgspoint.h"

QgsQuickMapToScreen::QgsQuickMapToScreen( QObject *parent )
  : QObject( parent )
{
}

void QgsQuickMapToScreen::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  if ( mMapSettings )
  {
    disconnect( mMapSettings, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapToScreen::transformPoint );
    disconnect( mMapSettings, &QgsQuickMapSettings::rotationChanged, this, &QgsQuickMapToScreen::transformPoint );
    disconnect( mMapSettings, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapToScreen::transformPoint );
  }

  mMapSettings = mapSettings;

  connect( mMapSettings, &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapToScreen::transformPoint );
  connect( mMapSettings, &QgsQuickMapSettings::rotationChanged, this, &QgsQuickMapToScreen::transformPoint );
  connect( mMapSettings, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickMapToScreen::transformPoint );

  transformPoint();
  transformDistance();

  emit mapSettingsChanged();
}

QgsQuickMapSettings *QgsQuickMapToScreen::mapSettings() const
{
  return mMapSettings;
}

void QgsQuickMapToScreen::setMapPoint( const QgsPoint &point )
{
  if ( mMapPoint == point )
    return;

  mMapPoint = point;
  emit mapPointChanged();
  transformPoint();
}

QgsPoint QgsQuickMapToScreen::mapPoint() const
{
  return mMapPoint;
}

QPointF QgsQuickMapToScreen::screenPoint() const
{
  return mScreenPoint;
}

void QgsQuickMapToScreen::transformPoint()
{
  if ( !mMapSettings )
  {
    mScreenPoint = QPointF();
  }
  else
  {
    mScreenPoint = mMapSettings->coordinateToScreen( mMapPoint );
  }
  emit screenPointChanged();
}

void QgsQuickMapToScreen::setMapDistance( const double distance )
{
  if ( mMapDistance == distance )
    return;

  mMapDistance = distance;
  emit mapDistanceChanged();
  transformDistance();
}

double QgsQuickMapToScreen::mapDistance() const
{
  return mMapDistance;
}

double QgsQuickMapToScreen::screenDistance() const
{
  return mScreenDistance;
}

void QgsQuickMapToScreen::transformDistance()
{
  if ( !mMapSettings || qgsDoubleNear( mMapDistance, 0.0 ) || qgsDoubleNear( mMapSettings->mapUnitsPerPoint(), 0.0 ) )
  {
    mScreenDistance = 0.0;
  }
  else
  {
    mScreenDistance = mMapDistance / mMapSettings->mapUnitsPerPoint();
  }
  emit screenDistanceChanged();
}
