/***************************************************************************
  qgsquickscalebarkit.cpp
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSize>
#include <QPoint>

#include "qgsdistancearea.h"
#include "qgspointxy.h"

#include "qgsquickmapsettings.h"
#include "qgsquickscalebarkit.h"
#include "qgsquickutils.h"
#include "qgsunittypes.h"

QgsQuickScaleBarKit::QgsQuickScaleBarKit( QObject *parent )
  : QObject( parent )
  , mPreferredWidth( 300 )
  , mWidth( mPreferredWidth )
  , mDistance( 0 )
  , mUnits( "" )
{
  connect( this, &QgsQuickScaleBarKit::mapSettingsChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
  connect( this, &QgsQuickScaleBarKit::preferredWidthChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
}

void QgsQuickScaleBarKit::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  // If we have already something connected, disconnect it!
  if ( mMapSettings )
  {
    disconnect( mMapSettings, nullptr, this, nullptr );
  }

  mMapSettings = mapSettings;

  // Connect all signals to change scale bar when needed!
  if ( mMapSettings )
  {
    connect( mMapSettings, &QgsQuickMapSettings::extentChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
    connect( mMapSettings, &QgsQuickMapSettings::destinationCrsChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
    connect( mMapSettings, &QgsQuickMapSettings::mapUnitsPerPixelChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
    connect( mMapSettings, &QgsQuickMapSettings::visibleExtentChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
    connect( mMapSettings, &QgsQuickMapSettings::outputSizeChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
    connect( mMapSettings, &QgsQuickMapSettings::outputDpiChanged, this, &QgsQuickScaleBarKit::updateScaleBar );
  }

  emit mapSettingsChanged();
}

int QgsQuickScaleBarKit::width() const
{
  return mWidth;
}

QString QgsQuickScaleBarKit::units() const
{
  return mUnits;
}

int QgsQuickScaleBarKit::distance() const
{
  return mDistance;
}

void QgsQuickScaleBarKit::updateScaleBar()
{
  if ( !mMapSettings )
    return;

  double distInMeters = QgsQuickUtils().screenUnitsToMeters( mMapSettings, mPreferredWidth ); // meters
  double dist;
  QgsUnitTypes::DistanceUnit distUnits;
  QgsQuickUtils().humanReadableDistance( distInMeters, QgsUnitTypes::DistanceMeters,
                                         mSystemOfMeasurement,
                                         dist, distUnits );

  mUnits = QgsUnitTypes::toAbbreviatedString( distUnits );

  // we want to show nice round distances e.g. 200 km instead of e.g. 273 km
  // so we determine which "nice" number to use and also update the scale bar
  // length accordingly. First digit will be 1, 2 or 5, the rest will be zeroes.
  int digits = int( floor( log10( ( dist ) ) ) ); // number of digits after first one
  double base = pow( 10, digits ); // e.g. for 1234 this will be 1000
  double first_digit = dist / base; // get the first digit
  int round_digit;
  if ( first_digit < 2 )
    round_digit = 1;
  else if ( first_digit < 5 )
    round_digit = 2;
  else
    round_digit = 5;

  mDistance = int( round_digit * base );
  mWidth = int( mPreferredWidth * mDistance / dist );

  emit scaleBarChanged();
}
