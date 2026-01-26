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

#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfieldconstraints.h"
#include "qgsfields.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

#include "moc_qgsrelationreferenceconfigdlg.cpp"

QgsRelationReferenceConfigDlg::QgsRelationReferenceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )

{
  setupUi( this );

  mFetchLimit->setMaximum( std::numeric_limits<int>::max() );

  connect( mAddFilterButton, &QToolButton::clicked, this, &QgsRelationReferenceConfigDlg::mAddFilterButton_clicked );
  connect( mRemoveFilterButton, &QToolButton::clicked, this, &QgsRelationReferenceConfigDlg::mRemoveFilterButton_clicked );

  mExpressionWidget->registerExpressionContextGenerator( vl );

  connect( mComboRelation, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsRelationReferenceConfigDlg::relationChanged );

  const auto constReferencingRelations = vl->referencingRelations( fieldIdx );
  for ( const QgsRelation &relation : constReferencingRelations )
  {
    if ( relation.name().isEmpty() )
      mComboRelation->addItem( u"%1 (%2)"_s.arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    else
      mComboRelation->addItem( u"%1 (%2)"_s.arg( relation.name(), relation.referencedLayerId() ), relation.id() );

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>( mComboRelation->model() );
    QStandardItem *item = model->item( model->rowCount() - 1 );
    item->setFlags( relation.type() == Qgis::RelationshipType::Generated ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled );

    if ( auto *lReferencedLayer = relation.referencedLayer() )
    {
      mExpressionWidget->setField( lReferencedLayer->displayExpression() );
    }
  }

  connect( mCbxAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
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
  connect( mFetchLimitCheckBox, &QCheckBox::toggled, mFetchLimit, &QSpinBox::setEnabled );
  connect( mCbxChainFilters, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mOrderByDescending, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mOrderByExpressionWidget, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsEditorConfigWidget::changed );
}

void QgsRelationReferenceConfigDlg::mEditExpression_clicked()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer() );
  if ( !vl )
    return;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vl ) );
  context << QgsExpressionContextUtils::formScope();
  context << QgsExpressionContextUtils::parentFormScope();

  context.setHighlightedFunctions( QStringList() << u"current_value"_s << u"current_parent_value"_s );
  context.setHighlightedVariables( QStringList() << u"current_geometry"_s << u"current_feature"_s << u"form_mode"_s << u"current_parent_geometry"_s << u"current_parent_feature"_s );

  QgsExpressionBuilderDialog dlg( vl, mFilterExpression->toPlainText(), this, u"generic"_s, context );
  dlg.setWindowTitle( tr( "Edit Filter Expression" ) );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFilterExpression->setPlainText( dlg.expressionBuilder()->expressionText() );
  }
}

void QgsRelationReferenceConfigDlg::setConfig( const QVariantMap &config )
{
  // Only unset allowNull if it was in the config or the default value that was
  // calculated from the field constraints when the widget was created will be overridden
  mAllowNullWasSetByConfig = config.contains( u"AllowNULL"_s );
  if ( mAllowNullWasSetByConfig )
  {
    mCbxAllowNull->setChecked( config.value( u"AllowNULL"_s, false ).toBool() );
  }
  mCbxShowForm->setChecked( config.value( u"ShowForm"_s, false ).toBool() );
  mCbxShowOpenFormButton->setChecked( config.value( u"ShowOpenFormButton"_s, true ).toBool() );

  if ( config.contains( u"Relation"_s ) )
  {
    mComboRelation->setCurrentIndex( mComboRelation->findData( config.value( u"Relation"_s ).toString() ) );
    relationChanged( mComboRelation->currentIndex() );
  }

  mCbxMapIdentification->setChecked( config.value( u"MapIdentification"_s, false ).toBool() );
  mCbxAllowAddFeatures->setChecked( config.value( u"AllowAddFeatures"_s, false ).toBool() );
  mCbxReadOnly->setChecked( config.value( u"ReadOnly"_s, false ).toBool() );
  mFetchLimitCheckBox->setChecked( config.value( u"FetchLimitActive"_s, QgsSettings().value( u"maxEntriesRelationWidget"_s, 100, QgsSettings::Gui ).toInt() > 0 ).toBool() );
  mFetchLimit->setValue( config.value( u"FetchLimitNumber"_s, QgsSettings().value( u"maxEntriesRelationWidget"_s, 100, QgsSettings::Gui ) ).toInt() );

  mOrderByExpressionWidget->setExpression( config.value( u"OrderExpression"_s ).toString() );
  mOrderByDescending->setChecked( config.value( u"OrderDescending"_s, false ).toBool() );

  mFilterExpression->setPlainText( config.value( u"FilterExpression"_s ).toString() );
  if ( config.contains( u"FilterFields"_s ) )
  {
    mFilterGroupBox->setChecked( true );
    const auto constToStringList = config.value( "FilterFields" ).toStringList();
    for ( const QString &fld : constToStringList )
    {
      addFilterField( fld );
    }

    mCbxChainFilters->setChecked( config.value( u"ChainFilters"_s ).toBool() );
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
  mOrderByExpressionWidget->setLayer( mReferencedLayer );
  // If AllowNULL is not set in the config, provide a default value based on the
  // constraints of the referencing fields
  if ( !mAllowNullWasSetByConfig )
  {
    mCbxAllowNull->setChecked( rel.referencingFieldsAllowNull() );
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
  myConfig.insert( u"AllowNULL"_s, mCbxAllowNull->isChecked() );
  myConfig.insert( u"ShowForm"_s, mCbxShowForm->isChecked() );
  myConfig.insert( u"ShowOpenFormButton"_s, mCbxShowOpenFormButton->isChecked() );
  myConfig.insert( u"MapIdentification"_s, mCbxMapIdentification->isEnabled() && mCbxMapIdentification->isChecked() );
  myConfig.insert( u"ReadOnly"_s, mCbxReadOnly->isChecked() );
  myConfig.insert( u"Relation"_s, mComboRelation->currentData() );
  myConfig.insert( u"AllowAddFeatures"_s, mCbxAllowAddFeatures->isChecked() );
  myConfig.insert( u"FetchLimitActive"_s, mFetchLimitCheckBox->isChecked() );
  myConfig.insert( u"FetchLimitNumber"_s, mFetchLimit->value() );

  myConfig.insert( u"OrderExpression"_s, mOrderByExpressionWidget->currentField() );
  myConfig.insert( u"OrderDescending"_s, mOrderByDescending->isChecked() );

  if ( mFilterGroupBox->isChecked() )
  {
    QStringList filterFields;
    filterFields.reserve( mFilterFieldsList->count() );
    for ( int i = 0; i < mFilterFieldsList->count(); i++ )
    {
      filterFields << mFilterFieldsList->item( i )->data( Qt::UserRole ).toString();
    }
    myConfig.insert( u"FilterFields"_s, filterFields );

    myConfig.insert( u"ChainFilters"_s, mCbxChainFilters->isChecked() );
    myConfig.insert( u"FilterExpression"_s, mFilterExpression->toPlainText() );
  }

  if ( mReferencedLayer )
  {
    // Store referenced layer data source and provider
    myConfig.insert( u"ReferencedLayerDataSource"_s, mReferencedLayer->publicSource() );
    myConfig.insert( u"ReferencedLayerProviderKey"_s, mReferencedLayer->providerType() );
    myConfig.insert( u"ReferencedLayerId"_s, mReferencedLayer->id() );
    myConfig.insert( u"ReferencedLayerName"_s, mReferencedLayer->name() );
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
