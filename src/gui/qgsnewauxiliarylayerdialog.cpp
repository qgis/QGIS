/***************************************************************************
                         qgsnewauxiliarylayerdialog.cpp  -  description
                             -------------------
    begin                : Aug 28, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  QgsGui::instance()->enableAutoGeometryRestore( this );

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
  }

  QDialog::accept();
}
