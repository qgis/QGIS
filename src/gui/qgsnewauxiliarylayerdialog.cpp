/***************************************************************************
                         qgsnewauxiliarylayerdialog.cpp  -  description
                             -------------------
    begin                : Aug 28, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnewauxiliarylayerdialog.h"
#include "qgsproject.h"
#include "qgsauxiliarystorage.h"

#include <QMessageBox>

QgsNewAuxiliaryLayerDialog::QgsNewAuxiliaryLayerDialog( QgsVectorLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );

  for ( const QgsField &field : mLayer->fields() )
    comboBox->addItem( field.name() );
}

void QgsNewAuxiliaryLayerDialog::accept()
{
  const int idx = mLayer->fields().lookupField( comboBox->currentText() );

  if ( idx >= 0 )
  {
    const QgsField field = mLayer->fields().field( idx );
    QgsAuxiliaryLayer *alayer = QgsProject::instance()->auxiliaryStorage()->createAuxiliaryLayer( field, mLayer );

    if ( alayer )
    {
      mLayer->setAuxiliaryLayer( alayer );
    }
  }

  QDialog::accept();
}
