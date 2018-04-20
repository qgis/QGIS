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

QgsQuickScaleBarKit::QgsQuickScaleBarKit( QObject *parent )
  : QObject( parent )
  , mMapSettings( 0 )
  , mPreferredWidth( 300 )
  , mWidth( mPreferredWidth )
  , mDistance( 0 )
  , mUnits( "" )
{
  connect( this, SIGNAL( mapSettingsChanged() ), this, SLOT( updateScaleBar() ) );
  connect( this, SIGNAL( preferredWidthChanged() ), this, SLOT( updateScaleBar() ) );
}

QgsQuickScaleBarKit::~QgsQuickScaleBarKit() {}


void QgsQuickScaleBarKit::setMapSettings( QgsQuickMapSettings *mapSettings )
{
  if ( mMapSettings == mapSettings )
    return;

  // If we have already something connected, disconnect it!
  if ( mMapSettings )
  {
    disconnect( mMapSettings, 0, this, 0 );
  }

  mMapSettings = mapSettings;

  // Connect all signals to change scale bar when needed!
  if ( mMapSettings )
  {
    connect( mMapSettings, SIGNAL( extentChanged() ), this, SLOT( updateScaleBar() ) );
    connect( mMapSettings, SIGNAL( destinationCrsChanged() ), this, SLOT( updateScaleBar() ) );
    connect( mMapSettings, SIGNAL( mapUnitsPerPixelChanged() ), this, SLOT( updateScaleBar() ) );
    connect( mMapSettings, SIGNAL( visibleExtentChanged() ), this, SLOT( updateScaleBar() ) );
    connect( mMapSettings, SIGNAL( outputSizeChanged() ), this, SLOT( updateScaleBar() ) );
    connect( mMapSettings, SIGNAL( outputDpiChanged() ), this, SLOT( updateScaleBar() ) );
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

  double dist = QgsQuickUtils::instance()->screenUnitsToMeters( mMapSettings, mPreferredWidth ); // meters
  if ( dist > 1000.0 )
  {
    dist = dist / 1000.0; // meters to kilometers
    mUnits = "km";
  }
  else
  {
    mUnits = "m";
  }

  // we want to show nice round distances e.g. 200 km instead of e.g. 273 km
  // so we determine which "nice" number to use and also update the scale bar
  // length accordingly. First digit will be 1, 2 or 5, the rest will be zeroes.
  int digits = floor( log10( ( dist ) ) ); // number of digits after first one
  double base = pow( 10, digits ); // e.g. for 1234 this will be 1000
  double first_digit = dist / base; // get the first digit
  int round_digit;
  if ( first_digit < 2 )
    round_digit = 1;
  else if ( first_digit < 5 )
    round_digit = 2;
  else
    round_digit = 5;

  mDistance = round_digit * base;
  mWidth = mPreferredWidth * mDistance / dist;

  emit scaleBarChanged();
}
