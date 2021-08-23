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
#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsfieldexpressionwidget.h"


QgsRelationAddDlg::QgsRelationAddDlg( QWidget *parent )
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

  mRelationStrengthComboBox->addItem( tr( "Association" ), QVariant::fromValue( QgsRelation::RelationStrength::Association ) );
  mRelationStrengthComboBox->addItem( tr( "Composition" ), QVariant::fromValue( QgsRelation::RelationStrength::Composition ) );
  mRelationStrengthComboBox->setToolTip( tr( "When composition is selected the child features will be duplicated too.\n"
                                         "Duplications are made by the feature duplication action.\n"
                                         "The default actions are activated in the Action section of the layer properties." ) );

  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRelationAddDlg::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsRelationAddDlg::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/attribute_table.html#defining-1-n-relations" ) );
  } );

  addFieldsRow();
  updateDialogButtons();

  connect( mFieldsMappingTable, &QTableWidget::itemSelectionChanged, this, &QgsRelationAddDlg::updateFieldsMappingButtons );
  connect( mFieldsMappingAddButton, &QToolButton::clicked, this, &QgsRelationAddDlg::addFieldsRow );
  connect( mFieldsMappingRemoveButton, &QToolButton::clicked, this, &QgsRelationAddDlg::removeFieldsRow );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::updateDialogButtons );
  connect( mReferencedLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::updateReferencedFieldsComboBoxes );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::updateDialogButtons );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::updateChildRelationsComboBox );
  connect( mReferencingLayerCombobox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::updateReferencingFieldsComboBoxes );
}

void QgsRelationAddDlg::addFieldsRow()
{
  QgsFieldComboBox *referencedField = new QgsFieldComboBox( this );
  QgsFieldComboBox *referencingField = new QgsFieldComboBox( this );
  const int index = mFieldsMappingTable->rowCount();

  referencedField->setLayer( mReferencedLayerCombobox->currentLayer() );
  referencingField->setLayer( mReferencingLayerCombobox->currentLayer() );

  mFieldsMappingTable->insertRow( index );
  mFieldsMappingTable->setCellWidget( index, 0, referencedField );
  mFieldsMappingTable->setCellWidget( index, 1, referencingField );

  connect( referencedField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::updateDialogButtons );
  connect( referencingField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::updateDialogButtons );

  updateFieldsMappingButtons();
  updateFieldsMappingHeaders();
  updateDialogButtons();
}

void QgsRelationAddDlg::removeFieldsRow()
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

void QgsRelationAddDlg::updateFieldsMappingButtons()
{
  const int rowsCount = mFieldsMappingTable->rowCount();
  const int selectedRowsCount = mFieldsMappingTable->selectionModel()->selectedRows().count();
  const bool isLayersRowSelected = mFieldsMappingTable->selectionModel()->isRowSelected( 0, QModelIndex() );
  const bool isRemoveButtonEnabled = !isLayersRowSelected && selectedRowsCount <= rowsCount - 2;

  mFieldsMappingRemoveButton->setEnabled( isRemoveButtonEnabled );
}

void QgsRelationAddDlg::updateFieldsMappingHeaders()
{
  const int rowsCount = mFieldsMappingTable->rowCount();
  QStringList verticalHeaderLabels( {tr( "Layer" )} );

  for ( int i = 0; i < rowsCount; i++ )
    verticalHeaderLabels << tr( "Field %1" ).arg( i + 1 );

  mFieldsMappingTable->setVerticalHeaderLabels( verticalHeaderLabels );
}

QString QgsRelationAddDlg::referencingLayerId()
{
  return mReferencingLayerCombobox->currentLayer()->id();
}

QString QgsRelationAddDlg::referencedLayerId()
{
  return mReferencedLayerCombobox->currentLayer()->id();
}

QList< QPair< QString, QString > > QgsRelationAddDlg::references()
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

QString QgsRelationAddDlg::relationId()
{
  return mIdLineEdit->text();
}

QString QgsRelationAddDlg::relationName()
{
  return mNameLineEdit->text();
}

QgsRelation::RelationStrength QgsRelationAddDlg::relationStrength()
{
  return mRelationStrengthComboBox->currentData().value<QgsRelation::RelationStrength>();
}

void QgsRelationAddDlg::updateDialogButtons()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isDefinitionValid() );
}

bool QgsRelationAddDlg::isDefinitionValid()
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

void QgsRelationAddDlg::updateChildRelationsComboBox()
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

void QgsRelationAddDlg::updateReferencedFieldsComboBoxes()
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

void QgsRelationAddDlg::updateReferencingFieldsComboBoxes()
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
