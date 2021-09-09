/***************************************************************************
    qgsrelationreferenceconfigdlg.cpp
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationreferenceconfigdlg.h"

#include "qgseditorwidgetfactory.h"
#include "qgsfields.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"

QgsRelationReferenceConfigDlg::QgsRelationReferenceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )

{
  setupUi( this );
  connect( mAddFilterButton, &QToolButton::clicked, this, &QgsRelationReferenceConfigDlg::mAddFilterButton_clicked );
  connect( mRemoveFilterButton, &QToolButton::clicked, this, &QgsRelationReferenceConfigDlg::mRemoveFilterButton_clicked );

  mExpressionWidget->registerExpressionContextGenerator( vl );

  connect( mComboRelation, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRelationReferenceConfigDlg::relationChanged );

  const auto constReferencingRelations = vl->referencingRelations( fieldIdx );
  for ( const QgsRelation &relation : constReferencingRelations )
  {
    if ( relation.name().isEmpty() )
      mComboRelation->addItem( QStringLiteral( "%1 (%2)" ).arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    else
      mComboRelation->addItem( QStringLiteral( "%1 (%2)" ).arg( relation.name(), relation.referencedLayerId() ), relation.id() );

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>( mComboRelation->model() );
    QStandardItem *item = model->item( model->rowCount() - 1 );
    item->setFlags( relation.type() == QgsRelation::Generated
                    ? item->flags() & ~Qt::ItemIsEnabled
                    : item->flags() | Qt::ItemIsEnabled );

    if ( auto *lReferencedLayer = relation.referencedLayer() )
    {
      mExpressionWidget->setField( lReferencedLayer->displayExpression() );
    }
  }

  connect( mCbxAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mCbxOrderByValue, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mCbxShowForm, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mCbxShowOpenFormButton, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mCbxMapIdentification, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mCbxReadOnly, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mComboRelation, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mCbxAllowAddFeatures, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFilterGroupBox, &QGroupBox::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFilterFieldsList, &QListWidget::itemChanged, this, &QgsEditorConfigWidget::changed );
  connect( mCbxChainFilters, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mEditExpression, &QAbstractButton::clicked, this, &QgsRelationReferenceConfigDlg::mEditExpression_clicked );
  connect( mFilterExpression, &QTextEdit::textChanged, this, &QgsEditorConfigWidget::changed );
}

void QgsRelationReferenceConfigDlg::mEditExpression_clicked()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer() );
  if ( !vl )
    return;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vl ) );
  context << QgsExpressionContextUtils::formScope( );
  context << QgsExpressionContextUtils::parentFormScope( );

  context.setHighlightedFunctions( QStringList() << QStringLiteral( "current_value" ) << QStringLiteral( "current_parent_value" ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "current_geometry" )
                                   << QStringLiteral( "current_feature" )
                                   << QStringLiteral( "form_mode" )
                                   << QStringLiteral( "current_parent_geometry" )
                                   << QStringLiteral( "current_parent_feature" ) );

  QgsExpressionBuilderDialog dlg( vl, mFilterExpression->toPlainText(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Edit Filter Expression" ) );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFilterExpression->setPlainText( dlg.expressionBuilder()->expressionText() );
  }
}

void QgsRelationReferenceConfigDlg::setConfig( const QVariantMap &config )
{
  mCbxAllowNull->setChecked( config.value( QStringLiteral( "AllowNULL" ), false ).toBool() );
  mCbxOrderByValue->setChecked( config.value( QStringLiteral( "OrderByValue" ), false ).toBool() );
  mCbxShowForm->setChecked( config.value( QStringLiteral( "ShowForm" ), false ).toBool() );
  mCbxShowOpenFormButton->setChecked( config.value( QStringLiteral( "ShowOpenFormButton" ), true ).toBool() );

  if ( config.contains( QStringLiteral( "Relation" ) ) )
  {
    mComboRelation->setCurrentIndex( mComboRelation->findData( config.value( QStringLiteral( "Relation" ) ).toString() ) );
    relationChanged( mComboRelation->currentIndex() );
  }

  mCbxMapIdentification->setChecked( config.value( QStringLiteral( "MapIdentification" ), false ).toBool() );
  mCbxAllowAddFeatures->setChecked( config.value( QStringLiteral( "AllowAddFeatures" ), false ).toBool() );
  mCbxReadOnly->setChecked( config.value( QStringLiteral( "ReadOnly" ), false ).toBool() );
  mFilterExpression->setPlainText( config.value( QStringLiteral( "FilterExpression" ) ).toString() );

  if ( config.contains( QStringLiteral( "FilterFields" ) ) )
  {
    mFilterGroupBox->setChecked( true );
    const auto constToStringList = config.value( "FilterFields" ).toStringList();
    for ( const QString &fld : constToStringList )
    {
      addFilterField( fld );
    }

    mCbxChainFilters->setChecked( config.value( QStringLiteral( "ChainFilters" ) ).toBool() );
  }
}

void QgsRelationReferenceConfigDlg::relationChanged( int idx )
{
  const QString relName = mComboRelation->itemData( idx ).toString();
  const QgsRelation rel = QgsProject::instance()->relationManager()->relation( relName );

  mReferencedLayer = rel.referencedLayer();
  mExpressionWidget->setLayer( mReferencedLayer ); // set even if 0
  if ( mReferencedLayer )
  {
    mExpressionWidget->setField( mReferencedLayer->displayExpression() );
    mCbxMapIdentification->setEnabled( mReferencedLayer->isSpatial() );
  }

  loadFields();
}

void QgsRelationReferenceConfigDlg::mAddFilterButton_clicked()
{
  const auto constSelectedItems = mAvailableFieldsList->selectedItems();
  for ( QListWidgetItem *item : constSelectedItems )
  {
    addFilterField( item );
  }
}

void QgsRelationReferenceConfigDlg::mRemoveFilterButton_clicked()
{
  const auto constSelectedItems = mFilterFieldsList->selectedItems();
  for ( QListWidgetItem *item : constSelectedItems )
  {
    mFilterFieldsList->takeItem( indexFromListWidgetItem( item ) );
    mAvailableFieldsList->addItem( item );
  }
}

QVariantMap QgsRelationReferenceConfigDlg::config()
{
  QVariantMap myConfig;
  myConfig.insert( QStringLiteral( "AllowNULL" ), mCbxAllowNull->isChecked() );
  myConfig.insert( QStringLiteral( "OrderByValue" ), mCbxOrderByValue->isChecked() );
  myConfig.insert( QStringLiteral( "ShowForm" ), mCbxShowForm->isChecked() );
  myConfig.insert( QStringLiteral( "ShowOpenFormButton" ), mCbxShowOpenFormButton->isChecked() );
  myConfig.insert( QStringLiteral( "MapIdentification" ), mCbxMapIdentification->isEnabled() && mCbxMapIdentification->isChecked() );
  myConfig.insert( QStringLiteral( "ReadOnly" ), mCbxReadOnly->isChecked() );
  myConfig.insert( QStringLiteral( "Relation" ), mComboRelation->currentData() );
  myConfig.insert( QStringLiteral( "AllowAddFeatures" ), mCbxAllowAddFeatures->isChecked() );

  if ( mFilterGroupBox->isChecked() )
  {
    QStringList filterFields;
    filterFields.reserve( mFilterFieldsList->count() );
    for ( int i = 0; i < mFilterFieldsList->count(); i++ )
    {
      filterFields << mFilterFieldsList->item( i )->data( Qt::UserRole ).toString();
    }
    myConfig.insert( QStringLiteral( "FilterFields" ), filterFields );

    myConfig.insert( QStringLiteral( "ChainFilters" ), mCbxChainFilters->isChecked() );
    myConfig.insert( QStringLiteral( "FilterExpression" ), mFilterExpression->toPlainText() );
  }

  if ( mReferencedLayer )
  {
    // Store referenced layer data source and provider
    myConfig.insert( QStringLiteral( "ReferencedLayerDataSource" ), mReferencedLayer->publicSource() );
    myConfig.insert( QStringLiteral( "ReferencedLayerProviderKey" ), mReferencedLayer->providerType() );
    myConfig.insert( QStringLiteral( "ReferencedLayerId" ), mReferencedLayer->id() );
    myConfig.insert( QStringLiteral( "ReferencedLayerName" ), mReferencedLayer->name() );
    mReferencedLayer->setDisplayExpression( mExpressionWidget->currentField() );
  }

  return myConfig;
}

void QgsRelationReferenceConfigDlg::loadFields()
{
  mAvailableFieldsList->clear();
  mFilterFieldsList->clear();

  if ( mReferencedLayer )
  {
    QgsVectorLayer *l = mReferencedLayer;
    const QgsFields &flds = l->fields();
    for ( int i = 0; i < flds.count(); i++ )
    {
      mAvailableFieldsList->addItem( flds.at( i ).displayName() );
      mAvailableFieldsList->item( mAvailableFieldsList->count() - 1 )->setData( Qt::UserRole, flds.at( i ).name() );
    }
  }
}

void QgsRelationReferenceConfigDlg::addFilterField( const QString &field )
{
  for ( int i = 0; i < mAvailableFieldsList->count(); i++ )
  {
    if ( mAvailableFieldsList->item( i )->data( Qt::UserRole ).toString() == field )
    {
      addFilterField( mAvailableFieldsList->item( i ) );
      break;
    }
  }
}

void QgsRelationReferenceConfigDlg::addFilterField( QListWidgetItem *item )
{
  mAvailableFieldsList->takeItem( indexFromListWidgetItem( item ) );
  mFilterFieldsList->addItem( item );
}

int QgsRelationReferenceConfigDlg::indexFromListWidgetItem( QListWidgetItem *item )
{
  QListWidget *lw = item->listWidget();

  for ( int i = 0; i < lw->count(); i++ )
  {
    if ( lw->item( i ) == item )
      return i;
  }

  return -1;
}
