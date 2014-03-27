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

QgsDisplayAngle::QgsDisplayAngle( QgsMapToolMeasureAngle * tool, Qt::WindowFlags f )
    : QDialog( tool->canvas()->topLevelWidget(), f ), mTool( tool )
{
  setupUi( this );
}

QgsDisplayAngle::~QgsDisplayAngle()
{
}

void QgsDisplayAngle::setValueInRadians( double value )
{
  mValue = value;
  updateUi();
}

void QgsDisplayAngle::updateUi()
{
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
