/***************************************************************************
                              qgstininterpolatordialog.cpp
                              ----------------------------
  begin                : March 29, 2008
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

#include "qgstininterpolatordialog.h"
#include "qgstininterpolator.h"

QgsTINInterpolatorDialog::QgsTINInterpolatorDialog( QWidget* parent, QgisInterface* iface ): QgsInterpolatorDialog( parent, iface )
{
  setupUi( this );
  //enter available interpolation methods
  mInterpolationComboBox->insertItem( 0, tr( "Linear interpolation" ) );
  //mInterpolationComboBox->insertItem(1, tr("Clough-Toucher interpolation"));
}

QgsTINInterpolatorDialog::~QgsTINInterpolatorDialog()
{

}

QgsInterpolator* QgsTINInterpolatorDialog::createInterpolator() const
{
  QList<QgsVectorLayer*> inputLayerList;

  QList< QPair <QgsVectorLayer*, QgsInterpolator::InputType> >::const_iterator data_it = mInputData.constBegin();
  for ( ; data_it != mInputData.constEnd(); ++data_it )
  {
    inputLayerList.push_back( data_it->first );
  }

  QgsTINInterpolator* theInterpolator = new QgsTINInterpolator( inputLayerList );
  return theInterpolator;
}
