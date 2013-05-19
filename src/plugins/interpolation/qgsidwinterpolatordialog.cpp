/***************************************************************************
    qgsidwinterpolatordialog.cpp
    ---------------------
    begin                : August 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsidwinterpolatordialog.h"
#include "qgsidwinterpolator.h"

QgsIDWInterpolatorDialog::QgsIDWInterpolatorDialog( QWidget* parent, QgisInterface* iface ): QgsInterpolatorDialog( parent, iface )
{
  setupUi( this );
}

QgsIDWInterpolatorDialog::~QgsIDWInterpolatorDialog()
{

}

QgsInterpolator* QgsIDWInterpolatorDialog::createInterpolator() const
{
  QgsIDWInterpolator* theInterpolator = new QgsIDWInterpolator( mInputData );
  theInterpolator->setDistanceCoefficient( mPSpinBox->value() );
  return theInterpolator;
}
