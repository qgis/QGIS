/***************************************************************************
  qgsrelationadddlg.cpp - QgsRelationAddDlg
  ---------------------------------

 begin                : 4.10.2013
 copyright            : (C) 2013 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "qgsrelationadddlg.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayercombobox.h"
#include "qgsfieldcombobox.h"
#include "qgsmaplayerproxymodel.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"


QgsCreateRelationDialog::QgsCreateRelationDialog( QWidget *parent )
  : QDialog( parent )
  , Ui::QgsRelationManagerAddDialogBase()
{
  setupUi( this );

  mReferencedLayerCombobox = new QgsMapLayerComboBox( this );
  mReferencedLayerCombobox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mFieldsMappingTable->setCellWidget( 0, 0, mReferencedLayerCombobox );

  mReferencingLayerCombobox = new QgsMapLayerComboBox( this );
  mReferencingLayerCombobox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mFieldsMappingTable->setCellWidget( 0, 1, mReferencingLayerCombobox );

  mRelationStrengthComboBox->addItem( tr( "Association" ), static_cast< int >( Qgis::RelationshipStrength::Association ) );
  mRelationStrengthComboBox->addItem( tr( "Composition" ), static_cast< int >( Qgis::RelationshipStrength::Composition ) );
  mRelationStrengthComboBox->setToolTip( tr( "When composition is selected the child features will be duplicated too.\n"
                                         "Duplications are made by the feature duplication action.\n"
                                         "The default actions are activated in the Action section of the layer properties." ) );

  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsCreateRelationDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsCreateRelationDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/attribute_table.html#defining-1-n-relations" ) );
  } );

  addFieldsRow();
  updateDialogButtons();

  connect( mFieldsMappingTable, &QTableWidget::itemSelectionChanged, this, &QgsCreateRelationDialog::updateFieldsMappingButtons );
  connect( mFieldsMappingAddButton, &QToolButton::clicked, this, &QgsCreateRelationDialog::addFieldsRow );
  connect( mFieldsMappingRemoveButton, &QToolButton::clicked, this, &QgsCreateRelationDialog::removeFieldsRow );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsCreateRelationDialog::updateDialogButtons );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsCreateRelationDialog::updateReferencedFieldsComboBoxes );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsCreateRelationDialog::updateDialogButtons );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsCreateRelationDialog::updateChildRelationsComboBox );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsCreateRelationDialog::updateReferencingFieldsComboBoxes );
}

void QgsCreateRelationDialog::addFieldsRow()
{
  QgsFieldComboBox *referencedField = new QgsFieldComboBox( this );
  QgsFieldComboBox *referencingField = new QgsFieldComboBox( this );
  const int index = mFieldsMappingTable->rowCount();

  referencedField->setLayer( mReferencedLayerCombobox->currentLayer() );
  referencingField->setLayer( mReferencingLayerCombobox->currentLayer() );

  mFieldsMappingTable->insertRow( index );
  mFieldsMappingTable->setCellWidget( index, 0, referencedField );
  mFieldsMappingTable->setCellWidget( index, 1, referencingField );

  connect( referencedField, &QgsFieldComboBox::fieldChanged, this, &QgsCreateRelationDialog::updateDialogButtons );
  connect( referencingField, &QgsFieldComboBox::fieldChanged, this, &QgsCreateRelationDialog::updateDialogButtons );

  updateFieldsMappingButtons();
  updateFieldsMappingHeaders();
  updateDialogButtons();
}

void QgsCreateRelationDialog::removeFieldsRow()
{
  if ( mFieldsMappingTable->selectionModel()->hasSelection() )
  {
    for ( const QModelIndex &index : mFieldsMappingTable->selectionModel()->selectedRows() )
    {
      if ( index.row() == 0 )
        continue;

      if ( mFieldsMappingTable->rowCount() > 2 )
        mFieldsMappingTable->removeRow( index.row() );
    }
  }
  else
  {
    mFieldsMappingTable->removeRow( mFieldsMappingTable->rowCount() - 1 );
  }

  updateFieldsMappingButtons();
  updateFieldsMappingHeaders();
  updateDialogButtons();
}

void QgsCreateRelationDialog::updateFieldsMappingButtons()
{
  const int rowsCount = mFieldsMappingTable->rowCount();
  const int selectedRowsCount = mFieldsMappingTable->selectionModel()->selectedRows().count();
  const bool isLayersRowSelected = mFieldsMappingTable->selectionModel()->isRowSelected( 0, QModelIndex() );
  const bool isRemoveButtonEnabled = !isLayersRowSelected && selectedRowsCount <= rowsCount - 2;

  mFieldsMappingRemoveButton->setEnabled( isRemoveButtonEnabled );
}

void QgsCreateRelationDialog::updateFieldsMappingHeaders()
{
  const int rowsCount = mFieldsMappingTable->rowCount();
  QStringList verticalHeaderLabels( {tr( "Layer" )} );

  for ( int i = 0; i < rowsCount; i++ )
    verticalHeaderLabels << tr( "Field %1" ).arg( i + 1 );

  mFieldsMappingTable->setVerticalHeaderLabels( verticalHeaderLabels );
}

QString QgsCreateRelationDialog::referencingLayerId() const
{
  return mReferencingLayerCombobox->currentLayer()->id();
}

QString QgsCreateRelationDialog::referencedLayerId() const
{
  return mReferencedLayerCombobox->currentLayer()->id();
}

QList< QPair< QString, QString > > QgsCreateRelationDialog::references() const
{
  QList< QPair< QString, QString > > references;
  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    // ignore the layers row
    if ( i == 0 )
      continue;

    const QString referencedField = static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) )->currentField();
    const QString referencingField = static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) )->currentField();
    references << qMakePair( referencingField, referencedField );
  }

  return references;
}

QString QgsCreateRelationDialog::relationId() const
{
  return mIdLineEdit->text();
}

QString QgsCreateRelationDialog::relationName() const
{
  return mNameLineEdit->text();
}

Qgis::RelationshipStrength QgsCreateRelationDialog::relationStrength() const
{
  return static_cast< Qgis::RelationshipStrength >( mRelationStrengthComboBox->currentData().toInt() );
}

void QgsCreateRelationDialog::updateDialogButtons()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isDefinitionValid() );
}

bool QgsCreateRelationDialog::isDefinitionValid()
{
  bool isValid = true;
  QgsMapLayer *referencedLayer = mReferencedLayerCombobox->currentLayer();
  isValid &= referencedLayer && referencedLayer->isValid();
  QgsMapLayer *referencingLayer = mReferencingLayerCombobox->currentLayer();
  isValid &= referencingLayer && referencingLayer->isValid();

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    // ignore the layers row
    if ( i == 0 )
      continue;

    isValid &= !static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) )->currentField().isNull();
    isValid &= !static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) )->currentField().isNull();
  }

  return isValid && mFieldsMappingTable->rowCount() > 1;
}

void QgsCreateRelationDialog::updateChildRelationsComboBox()
{
  QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( mReferencedLayerCombobox->currentLayer() );
  if ( !vl || !vl->isValid() )
    return;

  QStringList relationIdsList;

  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( vl );
  for ( const QgsRelation &relation : relations )
  {
    if ( !relation.isValid() )
      continue;

    if ( relation.referencingLayer() != vl )
      continue;

    relationIdsList << relationName();
  }
}

void QgsCreateRelationDialog::updateReferencedFieldsComboBoxes()
{
  QgsMapLayer *vl = mReferencedLayerCombobox->currentLayer();
  if ( !vl || !vl->isValid() )
    return;

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    // ignore the layers row
    if ( i == 0 )
      continue;

    auto fieldComboBox = static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) );
    fieldComboBox->setLayer( vl );
  }
}

void QgsCreateRelationDialog::updateReferencingFieldsComboBoxes()
{
  QgsMapLayer *vl = mReferencingLayerCombobox->currentLayer();
  if ( !vl || !vl->isValid() )
    return;

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    // ignore the layers row
    if ( i == 0 )
      continue;

    auto fieldComboBox = static_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) );
    fieldComboBox->setLayer( vl );
  }
}
