/***************************************************************************
                              qgsinterpolatordialog.cpp
                              -------------------------
  begin                : March 25, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinterpolatordialog.h"

QgsInterpolatorDialog::QgsInterpolatorDialog( QWidget* parent, QgisInterface* iface ): QDialog( parent ), mInterface( iface )
{

}

QgsInterpolatorDialog::~QgsInterpolatorDialog()
{

}

void QgsInterpolatorDialog::setInputData( const QList< QgsInterpolator::LayerData >& inputData )
{
  mInputData = inputData;
}
