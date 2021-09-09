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
#include "qgsmaptoolmeasureangle.h"
#include "qgssettings.h"
#include "qgsprojectdisplaysettings.h"
#include "qgsproject.h"
#include "qgsbearingnumericformat.h"

#include <cmath>

QgsDisplayAngle::QgsDisplayAngle( QgsMapTool *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );
}

void QgsDisplayAngle::setAngleInRadians( double value )
{
  mValue = value;

  const QgsSettings settings;
  const QgsUnitTypes::AngleUnit unit = QgsUnitTypes::decodeAngleUnit( settings.value( QStringLiteral( "qgis/measure/angleunits" ), QgsUnitTypes::encodeUnit( QgsUnitTypes::AngleDegrees ) ).toString() );
  const int decimals = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), 3 ).toInt();
  mAngleLineEdit->setText( QgsUnitTypes::formatAngle( mValue * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::AngleRadians, unit ), decimals, unit ) );
}

void QgsDisplayAngle::setBearingInRadians( double value )
{
  mValue = value;

  const double degrees = value * 180.0 / M_PI;

  const QgsNumericFormatContext context;
  const QString valueAsText = QgsProject::instance()->displaySettings()->bearingFormat()->formatDouble( degrees, context );
  mAngleLineEdit->setText( valueAsText );
}
