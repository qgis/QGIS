/***************************************************************************
    qgsdisplayangle.cpp
    ------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdisplayangle.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"

#include <QSettings>
#include <cmath>

QgsDisplayAngle::QgsDisplayAngle( QgsMapToolMeasureAngle * tool, Qt::WFlags f )
    : QDialog( tool->canvas()->topLevelWidget(), f ), mTool( tool )
{
  setupUi( this );
  QSettings settings;

  // Update when the ellipsoidal button has changed state.
  connect( mcbProjectionEnabled, SIGNAL( stateChanged( int ) ),
           this, SLOT( ellipsoidalButton() ) );
  // Update whenever the canvas has refreshed. Maybe more often than needed,
  // but at least every time any canvas related settings changes
  connect( mTool->canvas(), SIGNAL( mapCanvasRefreshed() ),
           this, SLOT( updateSettings() ) );

  updateSettings();
}

QgsDisplayAngle::~QgsDisplayAngle()
{

}

bool QgsDisplayAngle::projectionEnabled()
{
  return mcbProjectionEnabled->isChecked();
}

void QgsDisplayAngle::setValueInRadians( double value )
{
  mValue = value;
  updateUi();
}

void QgsDisplayAngle::ellipsoidalButton()
{
  QSettings settings;

  // We set check state to Unchecked and button to Disabled when disabling CRS,
  // which generates a call here. Ignore that event!
  if ( mcbProjectionEnabled->isEnabled() )
  {
    if ( mcbProjectionEnabled->isChecked() )
    {
      settings.setValue( "/qgis/measure/projectionEnabled", 2 );
    }
    else
    {
      settings.setValue( "/qgis/measure/projectionEnabled", 0 );
    }
    updateSettings();
  }
}

void QgsDisplayAngle::updateSettings()
{
  QSettings settings;

  int s = settings.value( "/qgis/measure/projectionEnabled", "2" ).toInt();
  if ( s == 2 )
  {
    mEllipsoidal = true;
  }
  else
  {
    mEllipsoidal = false;
  }
  QgsDebugMsg( "****************" );
  QgsDebugMsg( QString( "Ellipsoidal: %1" ).arg( mEllipsoidal ? "true" : "false" ) );

  updateUi();
  emit changeProjectionEnabledState();

}

void QgsDisplayAngle::updateUi()
{
  mcbProjectionEnabled->setEnabled( mTool->canvas()->hasCrsTransformEnabled() );
  mcbProjectionEnabled->setCheckState( mTool->canvas()->hasCrsTransformEnabled()
                                       && mEllipsoidal ? Qt::Checked : Qt::Unchecked );

  QSettings settings;
  QString unitString = settings.value( "/qgis/measure/angleunits", "degrees" ).toString();
  int decimals = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();

  if ( unitString == "degrees" )
  {
    mAngleLineEdit->setText( tr( "%1 degrees" ).arg( QLocale::system().toString( mValue * 180 / M_PI ),
                             'f', decimals ) );
  }
  else if ( unitString == "radians" )
  {
    mAngleLineEdit->setText( tr( "%1 radians" ).arg( QLocale::system().toString( mValue ),
                             'f', decimals ) );

  }
  else if ( unitString == "gon" )
  {
    mAngleLineEdit->setText( tr( "%1 gon" ).arg( QLocale::system().toString( mValue / M_PI * 200 ),
                             'f', decimals ) );
  }
}
