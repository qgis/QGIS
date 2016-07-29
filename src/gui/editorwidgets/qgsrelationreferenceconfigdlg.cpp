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
#include "qgsfield.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"

static QgsExpressionContext _getExpressionContext( const void* context )
{
  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();

  const QgsVectorLayer* layer = ( const QgsVectorLayer* ) context;
  if ( layer )
    expContext << QgsExpressionContextUtils::layerScope( layer );

  return expContext;
}

QgsRelationReferenceConfigDlg::QgsRelationReferenceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
    , mReferencedLayer( nullptr )
{
  setupUi( this );

  mExpressionWidget->registerGetExpressionContextCallback( &_getExpressionContext, vl );

  connect( mComboRelation, SIGNAL( currentIndexChanged( int ) ), this, SLOT( relationChanged( int ) ) );

  Q_FOREACH ( const QgsRelation& relation, vl->referencingRelations( fieldIdx ) )
  {
    mComboRelation->addItem( QString( "%1 (%2)" ).arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    if ( relation.referencedLayer() )
    {
      mExpressionWidget->setField( relation.referencedLayer()->displayExpression() );
    }
  }
}

void QgsRelationReferenceConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  if ( config.contains( "AllowNULL" ) )
  {
    mCbxAllowNull->setChecked( config.value( "AllowNULL" ).toBool() );
  }

  if ( config.contains( "OrderByValue" ) )
  {
    mCbxOrderByValue->setChecked( config.value( "OrderByValue" ).toBool() );
  }

  if ( config.contains( "ShowForm" ) )
  {
    mCbxShowForm->setChecked( config.value( "ShowForm" ).toBool() );
  }

  if ( config.contains( "Relation" ) )
  {
    mComboRelation->setCurrentIndex( mComboRelation->findData( config.value( "Relation" ).toString() ) );
    relationChanged( mComboRelation->currentIndex() );
  }

  if ( config.contains( "MapIdentification" ) )
  {
    mCbxMapIdentification->setChecked( config.value( "MapIdentification" ).toBool() );
  }

  if ( config.contains( "AllowAddFeatures" ) )
    mCbxAllowAddFeatures->setChecked( config.value( "AllowAddFeatures" ).toBool() );

  if ( config.contains( "ReadOnly" ) )
  {
    mCbxReadOnly->setChecked( config.value( "ReadOnly" ).toBool() );
  }

  if ( config.contains( "FilterFields" ) )
  {
    mFilterGroupBox->setChecked( true );
    Q_FOREACH ( const QString& fld, config.value( "FilterFields" ).toStringList() )
    {
      addFilterField( fld );
    }

    mCbxChainFilters->setChecked( config.value( "ChainFilters" ).toBool() );
  }
}

void QgsRelationReferenceConfigDlg::relationChanged( int idx )
{
  QString relName = mComboRelation->itemData( idx ).toString();
  QgsRelation rel = QgsProject::instance()->relationManager()->relation( relName );

  mReferencedLayer = rel.referencedLayer();
  mExpressionWidget->setLayer( mReferencedLayer ); // set even if 0
  if ( mReferencedLayer )
  {
    mExpressionWidget->setField( mReferencedLayer->displayExpression() );
    mCbxMapIdentification->setEnabled( mReferencedLayer->hasGeometryType() );
  }

  loadFields();
}

void QgsRelationReferenceConfigDlg::on_mAddFilterButton_clicked()
{
  Q_FOREACH ( QListWidgetItem* item, mAvailableFieldsList->selectedItems() )
  {
    addFilterField( item );
  }
}

void QgsRelationReferenceConfigDlg::on_mRemoveFilterButton_clicked()
{
  Q_FOREACH ( QListWidgetItem* item , mFilterFieldsList->selectedItems() )
  {
    mFilterFieldsList->takeItem( indexFromListWidgetItem( item ) );
    mAvailableFieldsList->addItem( item );
  }
}

QgsEditorWidgetConfig QgsRelationReferenceConfigDlg::config()
{
  QgsEditorWidgetConfig myConfig;
  myConfig.insert( "AllowNULL", mCbxAllowNull->isChecked() );
  myConfig.insert( "OrderByValue", mCbxOrderByValue->isChecked() );
  myConfig.insert( "ShowForm", mCbxShowForm->isChecked() );
  myConfig.insert( "MapIdentification", mCbxMapIdentification->isEnabled() && mCbxMapIdentification->isChecked() );
  myConfig.insert( "ReadOnly", mCbxReadOnly->isChecked() );
  myConfig.insert( "Relation", mComboRelation->itemData( mComboRelation->currentIndex() ) );
  myConfig.insert( "AllowAddFeatures", mCbxAllowAddFeatures->isChecked() );

  if ( mFilterGroupBox->isChecked() )
  {
    QStringList filterFields;
    filterFields.reserve( mFilterFieldsList->count() );
    for ( int i = 0; i < mFilterFieldsList->count(); i++ )
    {
      filterFields << mFilterFieldsList->item( i )->data( Qt::UserRole ).toString();
    }
    myConfig.insert( "FilterFields", filterFields );

    myConfig.insert( "ChainFilters", mCbxChainFilters->isChecked() );
  }

  if ( mReferencedLayer )
  {
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
    QgsVectorLayer* l = mReferencedLayer;
    const QgsFields& flds = l->fields();
    for ( int i = 0; i < flds.count(); i++ )
    {
      mAvailableFieldsList->addItem( l->attributeAlias( i ).isEmpty() ? flds.at( i ).name() : l->attributeAlias( i ) );
      mAvailableFieldsList->item( mAvailableFieldsList->count() - 1 )->setData( Qt::UserRole, flds.at( i ).name() );
    }
  }
}

void QgsRelationReferenceConfigDlg::addFilterField( const QString& field )
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

void QgsRelationReferenceConfigDlg::addFilterField( QListWidgetItem* item )
{
  mAvailableFieldsList->takeItem( indexFromListWidgetItem( item ) );
  mFilterFieldsList->addItem( item );
}

int QgsRelationReferenceConfigDlg::indexFromListWidgetItem( QListWidgetItem* item )
{
  QListWidget* lw = item->listWidget();

  for ( int i = 0; i < lw->count(); i++ )
  {
    if ( lw->item( i ) == item )
      return i;
  }

  return -1;
}
