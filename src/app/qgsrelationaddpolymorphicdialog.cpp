/***************************************************************************
    qgsrelationaddpolymorphicdialog.cpp
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDialogButtonBox>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>

#include "qgsrelationaddpolymorphicdialog.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayercombobox.h"
#include "qgsfieldcombobox.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsapplication.h"
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsfieldexpressionwidget.h"

QgsRelationAddPolymorphicDialog::QgsRelationAddPolymorphicDialog( bool isEditDialog, QWidget *parent )
  : QDialog( parent )
  , Ui::QgsRelationManagerAddPolymorphicDialogBase()
  , mIsEditDialog( isEditDialog )
{
  setupUi( this );

  setWindowTitle( mIsEditDialog
                  ? tr( "Edit Polymorphic Relation" )
                  : tr( "Add Polymorphic Relation" ) );

  mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRelationAddPolymorphicDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsRelationAddPolymorphicDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, [ = ]
  {
    QgsHelp::openHelp( QStringLiteral( "working_with_vector/attribute_table.html#defining-polymorphic-relations" ) );
  } );

  const QVector<QgsVectorLayer *> layers = QgsProject::instance()->layers<QgsVectorLayer *>();
  for ( const QgsMapLayer *vl : layers )
  {
    if ( !vl || !vl->isValid() )
      continue;

    mReferencedLayersComboBox->addItem( vl->name(), vl->id() );
  }

  mRelationStrengthComboBox->addItem( tr( "Association" ), QVariant::fromValue( QgsRelation::RelationStrength::Association ) );
  mRelationStrengthComboBox->addItem( tr( "Composition" ), QVariant::fromValue( QgsRelation::RelationStrength::Composition ) );
  mRelationStrengthComboBox->setToolTip( tr( "When composition is selected the child features will be duplicated too.\n"
                                         "Duplications are made by the feature duplication action.\n"
                                         "The default actions are activated in the Action section of the layer properties." ) );

  mFieldsMappingWidget->setEnabled( false );
  addFieldsRow();
  updateTypeConfigWidget();
  updateDialogButtons();
  updateReferencedLayerFieldComboBox();

  connect( mFieldsMappingTable, &QTableWidget::itemSelectionChanged, this, &QgsRelationAddPolymorphicDialog::updateFieldsMappingButtons );
  connect( mFieldsMappingAddButton, &QToolButton::clicked, this, &QgsRelationAddPolymorphicDialog::addFieldsRow );
  connect( mFieldsMappingRemoveButton, &QToolButton::clicked, this, &QgsRelationAddPolymorphicDialog::removeFieldsRow );
  connect( mReferencingLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddPolymorphicDialog::updateDialogButtons );
  connect( mRelationStrengthComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int index ) { Q_UNUSED( index ); updateDialogButtons(); } );
  connect( mReferencedLayerExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsRelationAddPolymorphicDialog::updateDialogButtons );
  connect( mReferencedLayersComboBox, &QgsCheckableComboBox::checkedItemsChanged, this, &QgsRelationAddPolymorphicDialog::referencedLayersChanged );
  connect( mReferencingLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddPolymorphicDialog::updateChildRelationsComboBox );
  connect( mReferencingLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddPolymorphicDialog::updateReferencingFieldsComboBoxes );
  connect( mReferencingLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddPolymorphicDialog::updateReferencedLayerFieldComboBox );
}

void QgsRelationAddPolymorphicDialog::setPolymorphicRelation( const QgsPolymorphicRelation polyRel )
{
  mIdLineEdit->setText( polyRel.id() );
  mReferencingLayerComboBox->setLayer( polyRel.referencingLayer() );
  mReferencedLayerFieldComboBox->setLayer( polyRel.referencingLayer() );
  mReferencedLayerFieldComboBox->setField( polyRel.referencedLayerField() );
  mReferencedLayerExpressionWidget->setExpression( polyRel.referencedLayerExpression() );
  mRelationStrengthComboBox->setCurrentIndex( mRelationStrengthComboBox->findData( polyRel.strength() ) );

  const QStringList layerIds = polyRel.referencedLayerIds();
  for ( const QString &layerId : layerIds )
    mReferencedLayersComboBox->setItemCheckState( mReferencedLayersComboBox->findData( layerId ), Qt::Checked );
  referencedLayersChanged();

  int index = 0;
  const QList<QgsRelation::FieldPair> fieldPairs = polyRel.fieldPairs();
  for ( const QgsRelation::FieldPair &fieldPair : fieldPairs )
  {
    qobject_cast<QComboBox *>( mFieldsMappingTable->cellWidget( index, 0 ) )->setCurrentText( fieldPair.referencedField() );
    qobject_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( index, 1 ) )->setCurrentText( fieldPair.referencingField() );
    index++;
  }
}

void QgsRelationAddPolymorphicDialog::updateTypeConfigWidget()
{
  updateDialogButtons();
}

void QgsRelationAddPolymorphicDialog::addFieldsRow()
{
  QgsFieldComboBox *referencingField = new QgsFieldComboBox( this );
  QComboBox *referencedPolymorphicField = new QComboBox( this );
  int index = mFieldsMappingTable->rowCount();

  referencingField->setLayer( mReferencingLayerComboBox->currentLayer() );

  connect( referencingField, &QgsFieldComboBox::fieldChanged, this, [ = ]( const QString & ) { updateDialogButtons(); } );

  mFieldsMappingTable->insertRow( index );
  mFieldsMappingTable->setCellWidget( index, 0, referencedPolymorphicField );
  mFieldsMappingTable->setCellWidget( index, 1, referencingField );

  updateFieldsMappingButtons();
  updateFieldsMappingHeaders();
  updateDialogButtons();
}

void QgsRelationAddPolymorphicDialog::removeFieldsRow()
{
  if ( mFieldsMappingTable->selectionModel()->hasSelection() )
  {
    for ( const QModelIndex &index : mFieldsMappingTable->selectionModel()->selectedRows() )
    {
      if ( index.row() == 0 )
        continue;

      if ( mFieldsMappingTable->rowCount() > 1 )
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

void QgsRelationAddPolymorphicDialog::updateFieldsMappingButtons()
{
  int rowsCount = mFieldsMappingTable->rowCount();
  int selectedRowsCount = mFieldsMappingTable->selectionModel()->selectedRows().count();
  bool isRemoveButtonEnabled = selectedRowsCount <= rowsCount - 2;

  mFieldsMappingRemoveButton->setEnabled( isRemoveButtonEnabled );
}

void QgsRelationAddPolymorphicDialog::updateFieldsMappingHeaders()
{
  int rowsCount = mFieldsMappingTable->rowCount();
  QStringList verticalHeaderLabels;

  for ( int i = 0; i < rowsCount; i++ )
    verticalHeaderLabels << tr( "Field %1" ).arg( i + 1 );

  mFieldsMappingTable->setVerticalHeaderLabels( verticalHeaderLabels );
}

QString QgsRelationAddPolymorphicDialog::referencingLayerId()
{
  return mReferencingLayerComboBox->currentLayer()->id();
}

QString QgsRelationAddPolymorphicDialog::referencedLayerField()
{
  return mReferencedLayerFieldComboBox->currentField();
}

QString QgsRelationAddPolymorphicDialog::referencedLayerExpression()
{
  return mReferencedLayerExpressionWidget->expression();
}

QStringList QgsRelationAddPolymorphicDialog::referencedLayerIds()
{
  return QVariant( mReferencedLayersComboBox->checkedItemsData() ).toStringList();
}

QList< QPair< QString, QString > > QgsRelationAddPolymorphicDialog::fieldPairs()
{
  QList< QPair< QString, QString > > references;
  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    QComboBox *referencedFieldComboBox = qobject_cast<QComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) );
    if ( referencedFieldComboBox->currentData().toInt() == -1 )
      continue;
    QString referencedField = referencedFieldComboBox->currentText();
    QString referencingField = qobject_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) )->currentField();
    references << qMakePair( referencingField, referencedField );
  }

  return references;
}

QString QgsRelationAddPolymorphicDialog::relationId()
{
  return mIdLineEdit->text();
}

QString QgsRelationAddPolymorphicDialog::relationName()
{
  QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( mReferencingLayerComboBox->currentLayer() );
  return tr( "Polymorphic relations for \"%1\"" ).arg( vl ? vl->name() : QStringLiteral( "<NO LAYER>" ) );
}

QgsRelation::RelationStrength QgsRelationAddPolymorphicDialog::relationStrength()
{
  return mRelationStrengthComboBox->currentData().value<QgsRelation::RelationStrength>();
}

void QgsRelationAddPolymorphicDialog::updateDialogButtons()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( isDefinitionValid() );
}

bool QgsRelationAddPolymorphicDialog::isDefinitionValid()
{
  bool isValid = true;
  QgsMapLayer *referencedLayer = mReferencingLayerComboBox->currentLayer();
  isValid &= referencedLayer && referencedLayer->isValid();

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    isValid &= qobject_cast<QComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) )->currentData().toInt() != -1;
    isValid &= !qobject_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) )->currentField().isNull();
  }

  return isValid;
}

void QgsRelationAddPolymorphicDialog::updateChildRelationsComboBox()
{
  QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( mReferencingLayerComboBox->currentLayer() );
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

void QgsRelationAddPolymorphicDialog::updateReferencingFieldsComboBoxes()
{
  QgsMapLayer *vl = mReferencingLayerComboBox->currentLayer();
  if ( !vl || !vl->isValid() )
    return;

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    auto fieldComboBox = qobject_cast<QgsFieldComboBox *>( mFieldsMappingTable->cellWidget( i, 1 ) );
    fieldComboBox->setLayer( vl );
  }
}

void QgsRelationAddPolymorphicDialog::updateReferencedLayerFieldComboBox()
{
  mReferencedLayerFieldComboBox->setLayer( mReferencingLayerComboBox->currentLayer() );
}

void QgsRelationAddPolymorphicDialog::referencedLayersChanged()
{
  const QStringList &layerIds = referencedLayerIds();
  mFieldsMappingWidget->setEnabled( layerIds.count() > 0 );

  bool firstLayer = true;
  QSet<QString> fields;
  for ( const QString &layerId : layerIds )
  {
    QgsVectorLayer *vl = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
    if ( vl && vl->isValid() )
    {
      const QSet layerFields = qgis::listToSet( vl->fields().names() );
      if ( firstLayer )
      {
        fields = layerFields;
        firstLayer = false;
      }
      else
      {
        fields.intersect( layerFields );
      }
    }
  }

  for ( int i = 0, l = mFieldsMappingTable->rowCount(); i < l; i++ )
  {
    QComboBox *cb = qobject_cast<QComboBox *>( mFieldsMappingTable->cellWidget( i, 0 ) );
    const QString currentField = cb->currentText();
    cb->clear();
    if ( fields.count() > 0 )
    {
      const QSet<QString> constFields = fields;
      for ( const QString &field : constFields )
      {
        cb->addItem( field );
        if ( field == currentField )
          cb->setCurrentText( field );
      }
    }
    else
    {
      cb->addItem( tr( "None" ), -1 );
      cb->addItem( tr( "the referenced layers have no common fields." ), -1 );
      QStandardItem *item = qobject_cast<QStandardItemModel *>( cb->model() )->item( 1 );
      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
    }
  }

  updateDialogButtons();
}
