/***************************************************************************
                            qgsprojectproperties.cpp
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsprojectproperties.h"


#include <qbuttongroup.h>
#include <qlineedit.h>

#include "qgsproject.h"


QgsProjectProperties::QgsProjectProperties(QWidget *parent, const char *name)
    : QgsProjectPropertiesBase(parent, name)
{
    QgsProject::instance()->mapUnits( QgsScaleCalculator::METERS );
}

QgsProjectProperties::~QgsProjectProperties()
{}


QgsScaleCalculator::units QgsProjectProperties::mapUnits() const
{
  return QgsProject::instance()->mapUnits();
}


void QgsProjectProperties::mapUnitChange(int unit)
{
   QgsProject::instance()->mapUnits(
       static_cast<QgsScaleCalculator::units>(unit));
}


void QgsProjectProperties::setMapUnits(QgsScaleCalculator::units unit)
{
  // select the button
  btnGrpMapUnits->setButton(static_cast<int>(unit));
  QgsProject::instance()->mapUnits(unit);
}


QString QgsProjectProperties::title() const
{
    return titleEdit->text();
} //  QgsProjectPropertires::title() const


void QgsProjectProperties::title( QString const & title )
{
    titleEdit->setText( title );
    QgsProject::instance()->title( title );
} // QgsProjectProperties::title( QString const & title )
