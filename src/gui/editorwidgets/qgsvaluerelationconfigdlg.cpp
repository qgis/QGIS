/***************************************************************************
    qgsvaluerelationconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationconfigdlg.h"

#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproject.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgsvectorlayer.h"

#include "moc_qgsvaluerelationconfigdlg.cpp"

QgsValueRelationConfigDlg::QgsValueRelationConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  mLayerName->setFilters( Qgis::LayerFilter::VectorLayer );
  mKeyColumn->setLayer( mLayerName->currentLayer() );
  mValueColumn->setLayer( mLayerName->currentLayer() );
  mGroupColumn->setLayer( mLayerName->currentLayer() );
  mGroupColumn->setAllowEmptyFieldName( true );
  mDescriptionExpression->setLayer( mLayerName->currentLayer() );
  mOrderByFieldName->setLayer( mLayerName->currentLayer() );
  mOrderByFieldName->setAllowEmptyFieldName( false );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mKeyColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mValueColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mGroupColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mDescriptionExpression, &QgsFieldExpressionWidget::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, this, &QgsValueRelationConfigDlg::layerChanged );
  connect( mEditExpression, &QAbstractButton::clicked, this, &QgsValueRelationConfigDlg::editExpression );
  connect( mOrderByField, &QAbstractButton::toggled, mOrderByFieldName, [this]( bool enabled ) {
    mOrderByFieldName->setEnabled( enabled );
  } );

  mOrderByGroupBox->setCollapsed( true );

  mNofColumns->setMinimum( 1 );
  mNofColumns->setMaximum( 10 );
  mNofColumns->setValue( 1 );

  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, this, &QgsEditorConfigWidget::changed );
  connect( mKeyColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mValueColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mGroupColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [this]( int index ) {
    mDisplayGroupName->setEnabled( index != 0 );
    emit changed();
  } );
  connect( mDescriptionExpression, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mAllowMulti, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mOrderByValue, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFilterExpression, &QTextEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mUseCompleter, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowMulti, &QAbstractButton::toggled, this, [this]( bool checked ) {
    label_nofColumns->setEnabled( checked );
    mNofColumns->setEnabled( checked );
  } );

  connect( mUseCompleter, &QCheckBox::stateChanged, this, [this]( int state ) {
    mCompleterMatchFromStart->setEnabled( static_cast<Qt::CheckState>( state ) == Qt::CheckState::Checked );
  } );

  mCompleterMatchFromStart->setEnabled( mUseCompleter->isChecked() );

  connect( mNofColumns, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );

  layerChanged();
}

QVariantMap QgsValueRelationConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( u"Layer"_s, mLayerName->currentLayer() ? mLayerName->currentLayer()->id() : QString() );
  cfg.insert( u"LayerName"_s, mLayerName->currentLayer() ? mLayerName->currentLayer()->name() : QString() );
  cfg.insert( u"LayerSource"_s, mLayerName->currentLayer() ? mLayerName->currentLayer()->publicSource() : QString() );
  cfg.insert( u"LayerProviderName"_s, ( mLayerName->currentLayer() && mLayerName->currentLayer()->dataProvider() ) ? mLayerName->currentLayer()->providerType() : QString() );
  cfg.insert( u"Key"_s, mKeyColumn->currentField() );
  cfg.insert( u"Value"_s, mValueColumn->currentField() );
  cfg.insert( u"Group"_s, mGroupColumn->currentField() );
  cfg.insert( u"DisplayGroupName"_s, mDisplayGroupName->isChecked() );
  cfg.insert( u"Description"_s, mDescriptionExpression->expression() );
  cfg.insert( u"AllowMulti"_s, mAllowMulti->isChecked() );
  cfg.insert( u"NofColumns"_s, mNofColumns->value() );
  cfg.insert( u"AllowNull"_s, mAllowNull->isChecked() );
  cfg.insert( u"OrderByValue"_s, mOrderByValue->isChecked() );
  cfg.insert( u"OrderByKey"_s, mOrderByKey->isChecked() );
  cfg.insert( u"OrderByField"_s, mOrderByField->isChecked() );
  cfg.insert( u"OrderByFieldName"_s, mOrderByFieldName->currentField() );
  cfg.insert( u"OrderByDescending"_s, mOrderByDescending->isChecked() );
  cfg.insert( u"FilterExpression"_s, mFilterExpression->toPlainText() );
  cfg.insert( u"UseCompleter"_s, mUseCompleter->isChecked() );
  const Qt::MatchFlags completerMatchFlags { mCompleterMatchFromStart->isChecked() ? Qt::MatchFlag::MatchStartsWith : Qt::MatchFlag::MatchContains };
  cfg.insert( u"CompleterMatchFlags"_s, static_cast<int>( completerMatchFlags ) );

  return cfg;
}

void QgsValueRelationConfigDlg::setConfig( const QVariantMap &config )
{
  QgsVectorLayer *lyr = QgsValueRelationFieldFormatter::resolveLayer( config, QgsProject::instance() );
  mLayerName->setLayer( lyr );
  mOrderByFieldName->setLayer( lyr );
  mKeyColumn->setField( config.value( u"Key"_s ).toString() );
  mValueColumn->setField( config.value( u"Value"_s ).toString() );
  mGroupColumn->setField( config.value( u"Group"_s ).toString() );
  mDisplayGroupName->setChecked( config.value( u"DisplayGroupName"_s ).toBool() );
  mDescriptionExpression->setField( config.value( u"Description"_s ).toString() );
  mAllowMulti->setChecked( config.value( u"AllowMulti"_s ).toBool() );
  mNofColumns->setValue( config.value( u"NofColumns"_s, 1 ).toInt() );
  if ( !mAllowMulti->isChecked() )
  {
    label_nofColumns->setEnabled( false );
    mNofColumns->setEnabled( false );
  }
  mAllowNull->setChecked( config.value( u"AllowNull"_s ).toBool() );
  mOrderByValue->setChecked( config.value( u"OrderByValue"_s ).toBool() );
  mOrderByField->setChecked( config.value( u"OrderByField"_s ).toBool() );
  mOrderByKey->setChecked( config.value( u"OrderByKey"_s ).toBool() );
  mOrderByFieldName->setField( config.value( u"OrderByFieldName"_s ).toString() );
  mOrderByDescending->setChecked( config.value( u"OrderByDescending"_s ).toBool() );

  if ( !mOrderByField->isChecked() && !mOrderByValue->isChecked() && !mOrderByKey->isChecked() )
  {
    mOrderByKey->setChecked( true );
  }

  // order by key is the default, if it is not checked, expand the config
  if ( !mOrderByKey->isChecked() )
  {
    mOrderByGroupBox->setCollapsed( false );
  }

  mFilterExpression->setPlainText( config.value( u"FilterExpression"_s ).toString() );
  mUseCompleter->setChecked( config.value( u"UseCompleter"_s ).toBool() );
  // Default is MatchStartsWith for backwards compatibility
  const Qt::MatchFlags completerMatchFlags { config.contains( u"CompleterMatchFlags"_s ) ? static_cast<Qt::MatchFlags>( config.value( u"CompleterMatchFlags"_s, Qt::MatchFlag::MatchStartsWith ).toInt() ) : Qt::MatchFlag::MatchStartsWith };
  mCompleterMatchFromStart->setChecked( completerMatchFlags.testFlag( Qt::MatchFlag::MatchStartsWith ) );
}

void QgsValueRelationConfigDlg::layerChanged()
{
  mFilterExpression->setEnabled( qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() ) );
  mEditExpression->setEnabled( qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() ) );
  mOrderByFieldName->setLayer( mLayerName->currentLayer() );
}

void QgsValueRelationConfigDlg::editExpression()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() );
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
    mFilterExpression->setText( dlg.expressionBuilder()->expressionText() );
  }
}
