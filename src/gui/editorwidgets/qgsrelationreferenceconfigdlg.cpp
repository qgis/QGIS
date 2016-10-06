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

QgsRelationReferenceConfigDlg::QgsRelationReferenceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QgsEditorConfigWidget( vl, fieldIdx, parent )
    , mReferencedLayer( nullptr )
{
  setupUi( this );

  mExpressionWidget->registerExpressionContextGenerator( vl );

  connect( mComboRelation, SIGNAL( currentIndexChanged( int ) ), this, SLOT( relationChanged( int ) ) );

  Q_FOREACH ( const QgsRelation& relation, vl->referencingRelations( fieldIdx ) )
  {
    mComboRelation->addItem( QString( "%1 (%2)" ).arg( relation.id(), relation.referencedLayerId() ), relation.id() );
    if ( relation.referencedLayer() )
    {
      mExpressionWidget->setField( relation.referencedLayer()->displayExpression() );
    }
  }

  connect( mCbxAllowNull, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mCbxOrderByValue, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mCbxShowForm, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mCbxMapIdentification, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mCbxReadOnly, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mComboRelation, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( changed() ) );
  connect( mCbxAllowAddFeatures, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mFilterGroupBox, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mFilterFieldsList, SIGNAL( itemChanged( QListWidgetItem* ) ), this, SIGNAL( changed() ) );
  connect( mCbxChainFilters, SIGNAL( toggled( bool ) ), this, SIGNAL( changed() ) );
  connect( mExpressionWidget, SIGNAL( fieldChanged( QString ) ), this, SIGNAL( changed() ) );
}

void QgsRelationReferenceConfigDlg::setConfig( const QgsEditorWidgetConfig& config )
{
  mCbxAllowNull->setChecked( config.value( "AllowNULL", false ).toBool() );
  mCbxOrderByValue->setChecked( config.value( "OrderByValue", false ).toBool() );
  mCbxShowForm->setChecked( config.value( "ShowForm", true ).toBool() );

  if ( config.contains( "Relation" ) )
  {
    mComboRelation->setCurrentIndex( mComboRelation->findData( config.value( "Relation" ).toString() ) );
    relationChanged( mComboRelation->currentIndex() );
  }

  mCbxMapIdentification->setChecked( config.value( "MapIdentification", false ).toBool() );
  mCbxAllowAddFeatures->setChecked( config.value( "AllowAddFeatures", false ).toBool() );
  mCbxReadOnly->setChecked( config.value( "ReadOnly", false ).toBool() );

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
  myConfig.insert( "Relation", mComboRelation->currentData() );
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
      mAvailableFieldsList->addItem( flds.at( i ).displayName() );
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
