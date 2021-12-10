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
#include "qgsauxiliarystorage.h"
#include "qgsproject.h"
#include "qgsgui.h"

#include <QMessageBox>
#include <QPushButton>

QgsNewAuxiliaryLayerDialog::QgsNewAuxiliaryLayerDialog( QgsVectorLayer *layer, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  const QgsFields fields = mLayer->fields();
  for ( const QgsField &field : fields )
    comboBox->addItem( field.name() );

  if ( fields.isEmpty() )
  {
    buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  }
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
    else
    {
      QDialog::close();
      const QString errMsg = QgsProject::instance()->auxiliaryStorage()->errorString();
      QMessageBox::critical( this, tr( "New Auxiliary Layer" ), errMsg );
      return;
    }
  }

  QDialog::accept();
}
