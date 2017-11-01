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

QgsValueRelationConfigDlg::QgsValueRelationConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  mLayerName->setFilters( QgsMapLayerProxyModel::VectorLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mKeyColumn, &QgsFieldComboBox::setLayer );
  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, mValueColumn, &QgsFieldComboBox::setLayer );
  connect( mEditExpression, &QAbstractButton::clicked, this, &QgsValueRelationConfigDlg::editExpression );

  connect( mLayerName, &QgsMapLayerComboBox::layerChanged, this, &QgsEditorConfigWidget::changed );
  connect( mKeyColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mValueColumn, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditorConfigWidget::changed );
  connect( mAllowMulti, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mAllowNull, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mOrderByValue, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
  connect( mFilterExpression, &QTextEdit::textChanged, this, &QgsEditorConfigWidget::changed );
  connect( mUseCompleter, &QAbstractButton::toggled, this, &QgsEditorConfigWidget::changed );
}

QVariantMap QgsValueRelationConfigDlg::config()
{
  QVariantMap cfg;

  cfg.insert( QStringLiteral( "Layer" ), mLayerName->currentLayer() ? mLayerName->currentLayer()->id() : QString() );
  cfg.insert( QStringLiteral( "Key" ), mKeyColumn->currentField() );
  cfg.insert( QStringLiteral( "Value" ), mValueColumn->currentField() );
  cfg.insert( QStringLiteral( "AllowMulti" ), mAllowMulti->isChecked() );
  cfg.insert( QStringLiteral( "AllowNull" ), mAllowNull->isChecked() );
  cfg.insert( QStringLiteral( "OrderByValue" ), mOrderByValue->isChecked() );
  cfg.insert( QStringLiteral( "FilterExpression" ), mFilterExpression->toPlainText() );
  cfg.insert( QStringLiteral( "UseCompleter" ), mUseCompleter->isChecked() );

  return cfg;
}

void QgsValueRelationConfigDlg::setConfig( const QVariantMap &config )
{
  QgsVectorLayer *lyr = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( config.value( QStringLiteral( "Layer" ) ).toString() ) );
  mLayerName->setLayer( lyr );
  mKeyColumn->setField( config.value( QStringLiteral( "Key" ) ).toString() );
  mValueColumn->setField( config.value( QStringLiteral( "Value" ) ).toString() );
  mAllowMulti->setChecked( config.value( QStringLiteral( "AllowMulti" ) ).toBool() );
  mAllowNull->setChecked( config.value( QStringLiteral( "AllowNull" ) ).toBool() );
  mOrderByValue->setChecked( config.value( QStringLiteral( "OrderByValue" ) ).toBool() );
  mFilterExpression->setPlainText( config.value( QStringLiteral( "FilterExpression" ) ).toString() );
  mUseCompleter->setChecked( config.value( QStringLiteral( "UseCompleter" ) ).toBool() );
}

void QgsValueRelationConfigDlg::editExpression()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mLayerName->currentLayer() );
  if ( !vl )
    return;

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( vl ) );

  QgsExpressionBuilderDialog dlg( vl, mFilterExpression->toPlainText(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Edit Filter Expression" ) );

  if ( dlg.exec() == QDialog::Accepted )
  {
    mFilterExpression->setText( dlg.expressionBuilder()->expressionText() );
  }
}
