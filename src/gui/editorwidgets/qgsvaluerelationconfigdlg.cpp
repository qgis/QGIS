/***************************************************************************
    qgsvaluerelationconfigdlg.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationconfigdlg.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"

QgsValueRelationConfigDlg::QgsValueRelationConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) :
    QgsEditorConfigWidget( vl, fieldIdx, parent )
{
  setupUi( this );
  mLayerName->setFilters( QgsMapLayerProxyModel::VectorLayer );
  connect( mLayerName, SIGNAL( layerChanged( QgsMapLayer* ) ), mKeyColumn, SLOT( setLayer( QgsMapLayer* ) ) );
  connect( mLayerName, SIGNAL( layerChanged( QgsMapLayer* ) ), mValueColumn, SLOT( setLayer( QgsMapLayer* ) ) );
}

QgsEditorWidgetConfig QgsValueRelationConfigDlg::config()
{
  QgsEditorWidgetConfig cfg;

  cfg.insert( "Layer", mLayerName->currentLayer()->id() );
  cfg.insert( "Key", mKeyColumn->currentField() );
  cfg.insert( "Value", mValueColumn->currentField() );
  cfg.insert( "AllowMulti", mAllowMulti->isChecked() );
  cfg.insert( "AllowNull", mAllowNull->isChecked() );
  cfg.insert( "OrderByValue", mOrderByValue->isChecked() );
  cfg.insert( "FilterExpression", mFilterExpression->toPlainText() );

  return cfg;
}

void QgsValueRelationConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  QgsVectorLayer* lyr = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( config.value( "Layer" ).toString() ) );
  mLayerName->setLayer( lyr );
  mKeyColumn->setField( config.value( "Key" ).toString() );
  mValueColumn->setField( config.value( "Value" ).toString() );
  mAllowMulti->setChecked( config.value( "AllowMulti" ).toBool() );
  mAllowNull->setChecked( config.value( "AllowNull" ).toBool() );
  mOrderByValue->setChecked( config.value( "OrderByValue" ).toBool() );
  mFilterExpression->setPlainText( config.value( "EditExpression" ).toString() );
}
