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

#include "qgsscalecalculator.h"

#include <qbuttongroup.h>
#include <qlineedit.h>



QgsProjectProperties::QgsProjectProperties(QWidget *parent, const char *name)
    : QgsProjectPropertiesBase(parent, name), 
      mMapUnits(QgsScaleCalculator::METERS)
{}

QgsProjectProperties::~QgsProjectProperties()
{}


int QgsProjectProperties::mapUnits()
{
  return mMapUnits;
}


void QgsProjectProperties::mapUnitChange(int unit)
{
  mMapUnits = unit;
}


void QgsProjectProperties::setMapUnits(int unit)
{
  // select the button
  btnGrpMapUnits->setButton(unit);
}


QString QgsProjectProperties::title() const
{
    return titleEdit->text();
} //  QgsProjectPropertires::title() const


void QgsProjectProperties::title( QString const & title )
{
    titleEdit->setText( title );
} // QgsProjectProperties::title( QString const & title )
