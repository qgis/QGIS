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
#include "qgsunittypes.h"

#include <QSettings>
#include <cmath>

QgsDisplayAngle::QgsDisplayAngle( QgsMapToolMeasureAngle * tool, Qt::WindowFlags f )
    : QDialog( tool->canvas()->topLevelWidget(), f )
    , mTool( tool )
    , mValue( 0.0 )
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
  QgsUnitTypes::AngleUnit unit = QgsUnitTypes::decodeAngleUnit( settings.value( "/qgis/measure/angleunits", QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleDegrees ) ).toString() );
  int decimals = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();
  mAngleLineEdit->setText( QgsUnitTypes::formatAngle( mValue * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::Radians, unit ), decimals, unit ) );
}
