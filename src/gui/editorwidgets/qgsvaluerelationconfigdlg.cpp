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
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvaluerelationfieldformatter.h"

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
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mKeyColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mValueColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mGroupColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mDescriptionExpression, &QgsFieldExpressionWidget::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, this, &QgsValueRelationConfigDlg::layerChanged );
  connect( mEditExpression, &QAbstractButton::clicked, this, &QgsValueRelationConfigDlg::editExpression );

  mNofColumns->setMinimum( 1 );
  mNofColumns->setMaximum( 10 );
  mNofColumns->setValue( 1 );

  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, this, &QgsEditorConfigWidget::changed );
  connect( mKeyColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mValueColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mGroupColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int index )
  {
    mDisplayGroupName->setEnabled( index != 0 );
    emit changed();
  } );
  connect( mDescriptionExpression, static_cast<void ( QgsFieldExpressionWidget::* )( const QString & )>( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mAllowMulti, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mOrderByValue, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFilterExpression, &QTextEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mUseCompleter, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowMulti, &QAbstractButton::toggled, this, [ = ]( bool checked )
  {
    label_nofColumns->setEnabled( checked );
    mNofColumns->setEnabled( checked );
  } );

  connect( mUseCompleter, &QCheckBox::stateChanged, this,  [ = ]( int state )
  {
    mCompleterMatchFromStart->setEnabled( static_cast<Qt::CheckState>( state ) == Qt::CheckState::Checked );
  } );

  mCompleterMatchFromStart->setEnabled( mUseCompleter->isChecked() );

  connect( mNofColumns, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsEditorConfigWidget::changed );

  layerChanged();
}

QVariantMap QgsValueRelationConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "Layer" ), mLayerName->currentLayer() ? mLayerName->currentLayer()->id() : QString() );
  cfg.insert( QStringLiteral( "LayerName" ), mLayerName->currentLayer() ? mLayerName->currentLayer()->name() : QString() );
  cfg.insert( QStringLiteral( "LayerSource" ), mLayerName->currentLayer() ? mLayerName->currentLayer()->publicSource() : QString() );
  cfg.insert( QStringLiteral( "LayerProviderName" ), ( mLayerName->currentLayer() && mLayerName->currentLayer()->dataProvider() ) ?
              mLayerName->currentLayer()->providerType() :
              QString() );
  cfg.insert( QStringLiteral( "Key" ), mKeyColumn->currentField() );
  cfg.insert( QStringLiteral( "Value" ), mValueColumn->currentField() );
  cfg.insert( QStringLiteral( "Group" ), mGroupColumn->currentField() );
  cfg.insert( QStringLiteral( "DisplayGroupName" ), mDisplayGroupName->isChecked() );
  cfg.insert( QStringLiteral( "Description" ), mDescriptionExpression->expression() );
  cfg.insert( QStringLiteral( "AllowMulti" ), mAllowMulti->isChecked() );
  cfg.insert( QStringLiteral( "NofColumns" ), mNofColumns->value() );
  cfg.insert( QStringLiteral( "AllowNull" ), mAllowNull->isChecked() );
  cfg.insert( QStringLiteral( "OrderByValue" ), mOrderByValue->isChecked() );
  cfg.insert( QStringLiteral( "FilterExpression" ), mFilterExpression->toPlainText() );
  cfg.insert( QStringLiteral( "UseCompleter" ), mUseCompleter->isChecked() );
  const Qt::MatchFlags completerMatchFlags { mCompleterMatchFromStart->isChecked() ? Qt::MatchFlag::MatchStartsWith : Qt::MatchFlag::MatchContains };
  cfg.insert( QStringLiteral( "CompleterMatchFlags" ), static_cast<int>( completerMatchFlags ) );

  return cfg;
}

void QgsValueRelationConfigDlg::setConfig( const QVariantMap &config )
{
  QgsVectorLayer *lyr = QgsValueRelationFieldFormatter::resolveLayer( config, QgsProject::instance() );
  mLayerName->setLayer( lyr );
  mKeyColumn->setField( config.value( QStringLiteral( "Key" ) ).toString() );
  mValueColumn->setField( config.value( QStringLiteral( "Value" ) ).toString() );
  mGroupColumn->setField( config.value( QStringLiteral( "Group" ) ).toString() );
  mDisplayGroupName->setChecked( config.value( QStringLiteral( "DisplayGroupName" ) ).toBool() );
  mDescriptionExpression->setField( config.value( QStringLiteral( "Description" ) ).toString() );
  mAllowMulti->setChecked( config.value( QStringLiteral( "AllowMulti" ) ).toBool() );
  mNofColumns->setValue( config.value( QStringLiteral( "NofColumns" ), 1 ).toInt() );
  if ( !mAllowMulti->isChecked() )
  {
    label_nofColumns->setEnabled( false );
    mNofColumns->setEnabled( false );
  }
  mAllowNull->setChecked( config.value( QStringLiteral( "AllowNull" ) ).toBool() );
  mOrderByValue->setChecked( config.value( QStringLiteral( "OrderByValue" ) ).toBool() );
  mFilterExpression->setPlainText( config.value( QStringLiteral( "FilterExpression" ) ).toString() );
  mUseCompleter->setChecked( config.value( QStringLiteral( "UseCompleter" ) ).toBool() );
  // Default is MatchStartsWith for backwards compatibility
  const Qt::MatchFlags completerMatchFlags { config.contains( QStringLiteral( "CompleterMatchFlags" ) ) ? static_cast<Qt::MatchFlags>( config.value( QStringLiteral( "CompleterMatchFlags" ), Qt::MatchFlag::MatchStartsWith ).toInt( ) ) :  Qt::MatchFlag::MatchStartsWith };
  mCompleterMatchFromStart->setChecked( completerMatchFlags.testFlag( Qt::MatchFlag::MatchStartsWith ) );
}

void QgsValueRelationConfigDlg::layerChanged()
{
  mFilterExpression->setEnabled( qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() ) );
  mEditExpression->setEnabled( qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() ) );
}

void QgsValueRelationConfigDlg::editExpression()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() );
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
    mFilterExpression->setText( dlg.expressionBuilder()->expressionText() );
  }
}
