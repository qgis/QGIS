/***************************************************************************
    qgsfieldsproperties.cpp
    ---------------------
    begin                : September 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldsproperties.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsattributetypedialog.h"
#include "qgsaddattrdialog.h"
#include "qgsfieldcalculator.h"
#include "qgsvectordataprovider.h"

#include <QTreeWidgetItem>
#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>

QgsFieldsProperties::QgsFieldsProperties( QgsVectorLayer *layer, QWidget* parent )
  : QWidget( parent ), mLayer ( layer )
{
  if ( !layer )
    return;

  setupUi( this );
  setupEditTypes();

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.png" ) );
  mDeleteAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( "/mActionToggleEditing.png" ) );
  mCalculateFieldButton->setIcon( QgsApplication::getThemeIcon( "/mActionCalculateField.png" ) );

  connect( mToggleEditingButton, SIGNAL( clicked() ), this, SLOT( toggleEditing() ) );
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  connect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );

  updateButtons();
}

void QgsFieldsProperties::toggleEditing()
{
  emit toggleEditing( mLayer );
}


void QgsFieldsProperties::loadRows()
{
  disconnect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );
  const QgsFieldMap &fields = mLayer->pendingFields();

  tblAttributes->clear();

  tblAttributes->setColumnCount( attrColCount );
  tblAttributes->setRowCount( fields.size() );
  tblAttributes->setHorizontalHeaderItem( attrIdCol, new QTableWidgetItem( tr( "Id" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrNameCol, new QTableWidgetItem( tr( "Name" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrTypeCol, new QTableWidgetItem( tr( "Type" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrLengthCol, new QTableWidgetItem( tr( "Length" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrPrecCol, new QTableWidgetItem( tr( "Precision" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrCommentCol, new QTableWidgetItem( tr( "Comment" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrEditTypeCol, new QTableWidgetItem( tr( "Edit widget" ) ) );
  tblAttributes->setHorizontalHeaderItem( attrWMSCol, new QTableWidgetItem( "WMS" ) );
  tblAttributes->setHorizontalHeaderItem( attrWFSCol, new QTableWidgetItem( "WFS" ) );
  tblAttributes->setHorizontalHeaderItem( attrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  tblAttributes->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
  tblAttributes->horizontalHeader()->setResizeMode( 7, QHeaderView::Stretch );
  tblAttributes->setSelectionBehavior( QAbstractItemView::SelectRows );
  tblAttributes->setSelectionMode( QAbstractItemView::ExtendedSelection );
  tblAttributes->verticalHeader()->hide();

  int row = 0;
  for ( QgsFieldMap::const_iterator it = fields.begin(); it != fields.end(); it++, row++ )
    setRow( row, it.key(), it.value() );

  tblAttributes->resizeColumnsToContents();
  connect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );
}

void QgsFieldsProperties::setRow( int row, int idx, const QgsField &field )
{
  tblAttributes->setItem( row, attrIdCol, new QTableWidgetItem( QString::number( idx ) ) );
  tblAttributes->setItem( row, attrNameCol, new QTableWidgetItem( field.name() ) );
  tblAttributes->setItem( row, attrTypeCol, new QTableWidgetItem( field.typeName() ) );
  tblAttributes->setItem( row, attrLengthCol, new QTableWidgetItem( QString::number( field.length() ) ) );
  tblAttributes->setItem( row, attrPrecCol, new QTableWidgetItem( QString::number( field.precision() ) ) );
  tblAttributes->setItem( row, attrCommentCol, new QTableWidgetItem( field.comment() ) );

  for ( int i = 0; i < attrEditTypeCol; i++ )
    tblAttributes->item( row, i )->setFlags( tblAttributes->item( row, i )->flags() & ~Qt::ItemIsEditable );

  QPushButton *pb = new QPushButton( editTypeButtonText( mLayer->editType( idx ) ) );
  tblAttributes->setCellWidget( row, attrEditTypeCol, pb );
  connect( pb, SIGNAL( pressed() ), this, SLOT( attributeTypeDialog( ) ) );
  mButtonMap.insert( idx, pb );

  //set the alias for the attribute
  tblAttributes->setItem( row, attrAliasCol, new QTableWidgetItem( mLayer->attributeAlias( idx ) ) );

  //published WMS/WFS attributes
  QTableWidgetItem* wmsAttrItem = new QTableWidgetItem();
  wmsAttrItem->setCheckState( mLayer->excludeAttributesWMS().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wmsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  tblAttributes->setItem( row, attrWMSCol, wmsAttrItem );
  QTableWidgetItem* wfsAttrItem = new QTableWidgetItem();
  wfsAttrItem->setCheckState( mLayer->excludeAttributesWFS().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wfsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  tblAttributes->setItem( row, attrWFSCol, wfsAttrItem );

}

void QgsFieldsProperties::attributeTypeDialog( )
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  int index = mButtonMap.key( pb, -1 );
  if ( index == -1 )
    return;

  QgsAttributeTypeDialog attributeTypeDialog( mLayer );

  attributeTypeDialog.setValueMap( mValueMaps.value( index, mLayer->valueMap( index ) ) );
  attributeTypeDialog.setRange( mRanges.value( index, mLayer->range( index ) ) );
  attributeTypeDialog.setValueRelation( mValueRelationData.value( index, mLayer->valueRelation( index ) ) );

  QPair<QString, QString> checkStates = mCheckedStates.value( index, mLayer->checkedState( index ) );
  attributeTypeDialog.setCheckedState( checkStates.first, checkStates.second );

  attributeTypeDialog.setIndex( index, mEditTypeMap.value( index, mLayer->editType( index ) ) );

  if ( !attributeTypeDialog.exec() )
    return;

  QgsVectorLayer::EditType editType = attributeTypeDialog.editType();

  mEditTypeMap.insert( index, editType );

  QString buttonText;
  switch ( editType )
  {
    case QgsVectorLayer::ValueMap:
      mValueMaps.insert( index, attributeTypeDialog.valueMap() );
      break;
    case QgsVectorLayer::EditRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::DialRange:
      mRanges.insert( index, attributeTypeDialog.rangeData() );
      break;
    case QgsVectorLayer::CheckBox:
      mCheckedStates.insert( index, attributeTypeDialog.checkedState() );
      break;
    case QgsVectorLayer::ValueRelation:
      mValueRelationData.insert( index, attributeTypeDialog.valueRelationData() );
      break;
    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::TextEdit:
    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Classification:
    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Enumeration:
    case QgsVectorLayer::Immutable:
    case QgsVectorLayer::Hidden:
    case QgsVectorLayer::Calendar:
    case QgsVectorLayer::UuidGenerator:
      break;
  }

  pb->setText( editTypeButtonText( editType ) );
}


void QgsFieldsProperties::attributeAdded( int idx )
{
  const QgsFieldMap &fields = mLayer->pendingFields();
  int row = tblAttributes->rowCount();
  tblAttributes->insertRow( row );
  setRow( row, idx, fields[idx] );
  tblAttributes->setCurrentCell( row, idx );
}


void QgsFieldsProperties::attributeDeleted( int idx )
{
  for ( int i = 0; i < tblAttributes->rowCount(); i++ )
  {
    if ( tblAttributes->item( i, 0 )->text().toInt() == idx )
    {
      tblAttributes->removeRow( i );
      break;
    }
  }
}

void QgsFieldsProperties::addAttribute()
{
  QgsAddAttrDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    mLayer->beginEditCommand( "Attribute added" );
    if ( !addAttribute( dialog.field() ) )
    {
      mLayer->destroyEditCommand();
      QMessageBox::information( this, tr( "Name conflict" ), tr( "The attribute could not be inserted. The name already exists in the table." ) );
    }
    else
    {
      mLayer->endEditCommand();
    }
  }
}

bool QgsFieldsProperties::addAttribute( const QgsField &field )
{
  QgsDebugMsg( "inserting attribute " + field.name() + " of type " + field.typeName() );
  mLayer->beginEditCommand( tr( "Added attribute" ) );
  if ( mLayer->addAttribute( field ) )
  {
    mLayer->endEditCommand();
    return true;
  }
  else
  {
    mLayer->destroyEditCommand();
    return false;
  }
}

void QgsFieldsProperties::deleteAttribute()
{
  QList<QTableWidgetItem*> items = tblAttributes->selectedItems();
  QList<int> idxs;

  for ( QList<QTableWidgetItem*>::const_iterator it = items.begin(); it != items.end(); it++ )
  {
    if (( *it )->column() == 0 )
      idxs << ( *it )->text().toInt();
  }
  for ( QList<int>::const_iterator it = idxs.begin(); it != idxs.end(); it++ )
  {
    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    mLayer->deleteAttribute( *it );
    mLayer->endEditCommand();
  }
}

void QgsFieldsProperties::editingToggled()
{
  if ( !mLayer->isEditable() )
    loadRows();

  updateButtons();
}

void QgsFieldsProperties::on_mAddAttributeButton_clicked()
{
  QgsAddAttrDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    mLayer->beginEditCommand( "Attribute added" );
    if ( !addAttribute( dialog.field() ) )
    {
      mLayer->destroyEditCommand();
      QMessageBox::information( this, tr( "Name conflict" ), tr( "The attribute could not be inserted. The name already exists in the table." ) );
    }
    else
    {
      mLayer->endEditCommand();
    }
  }
}

void QgsFieldsProperties::on_mDeleteAttributeButton_clicked()
{
  QList<QTableWidgetItem*> items = tblAttributes->selectedItems();
  QList<int> idxs;

  for ( QList<QTableWidgetItem*>::const_iterator it = items.begin(); it != items.end(); it++ )
  {
    if (( *it )->column() == 0 )
      idxs << ( *it )->text().toInt();
  }
  for ( QList<int>::const_iterator it = idxs.begin(); it != idxs.end(); it++ )
  {
    mLayer->beginEditCommand( tr( "Deleted attribute" ) );
    mLayer->deleteAttribute( *it );
    mLayer->endEditCommand();
  }
}

void QgsFieldsProperties::updateButtons()
{
  if ( mLayer->isEditable() )
  {
    int cap = mLayer->dataProvider()->capabilities();
    mAddAttributeButton->setEnabled( cap & QgsVectorDataProvider::AddAttributes );
    mDeleteAttributeButton->setEnabled( cap & QgsVectorDataProvider::DeleteAttributes );
    mCalculateFieldButton->setEnabled( cap & ( QgsVectorDataProvider::ChangeAttributeValues | QgsVectorDataProvider::AddAttributes ) );
    mToggleEditingButton->setChecked( true );
  }
  else
  {
    mAddAttributeButton->setEnabled( false );
    mDeleteAttributeButton->setEnabled( false );
    mToggleEditingButton->setChecked( false );
    mToggleEditingButton->setEnabled( false );
    mCalculateFieldButton->setEnabled( false );
  }
}


void QgsFieldsProperties::on_tblAttributes_cellChanged( int row, int column )
{
  if ( column == attrAliasCol && mLayer ) //only consider attribute aliases in this function
  {
    int idx = tblAttributes->item( row, attrIdCol )->text().toInt();

    const QgsFieldMap &fields = mLayer->pendingFields();

    if ( !fields.contains( idx ) )
    {
      return; // index must be wrong
    }

    QTableWidgetItem *aliasItem = tblAttributes->item( row, column );
    if ( aliasItem )
    {
      mLayer->addAttributeAlias( idx, aliasItem->text() );
    }
  }
}

void QgsFieldsProperties::on_mCalculateFieldButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsFieldCalculator calc( mLayer );
  calc.exec();
}


//
// methods reimplemented from qt designer base class
//

QMap< QgsVectorLayer::EditType, QString > QgsFieldsProperties::editTypeMap;

void QgsFieldsProperties::setupEditTypes()
{
  if ( !editTypeMap.isEmpty() )
    return;

  editTypeMap.insert( QgsVectorLayer::LineEdit, tr( "Line edit" ) );
  editTypeMap.insert( QgsVectorLayer::UniqueValues, tr( "Unique values" ) );
  editTypeMap.insert( QgsVectorLayer::UniqueValuesEditable, tr( "Unique values editable" ) );
  editTypeMap.insert( QgsVectorLayer::Classification, tr( "Classification" ) );
  editTypeMap.insert( QgsVectorLayer::ValueMap, tr( "Value map" ) );
  editTypeMap.insert( QgsVectorLayer::EditRange, tr( "Edit range" ) );
  editTypeMap.insert( QgsVectorLayer::SliderRange, tr( "Slider range" ) );
  editTypeMap.insert( QgsVectorLayer::DialRange, tr( "Dial range" ) );
  editTypeMap.insert( QgsVectorLayer::FileName, tr( "File name" ) );
  editTypeMap.insert( QgsVectorLayer::Enumeration, tr( "Enumeration" ) );
  editTypeMap.insert( QgsVectorLayer::Immutable, tr( "Immutable" ) );
  editTypeMap.insert( QgsVectorLayer::Hidden, tr( "Hidden" ) );
  editTypeMap.insert( QgsVectorLayer::CheckBox, tr( "Checkbox" ) );
  editTypeMap.insert( QgsVectorLayer::TextEdit, tr( "Text edit" ) );
  editTypeMap.insert( QgsVectorLayer::Calendar, tr( "Calendar" ) );
  editTypeMap.insert( QgsVectorLayer::ValueRelation, tr( "Value relation" ) );
  editTypeMap.insert( QgsVectorLayer::UuidGenerator, tr( "UUID generator" ) );
}

QString QgsFieldsProperties::editTypeButtonText( QgsVectorLayer::EditType type )
{
  return editTypeMap[ type ];
}

QgsVectorLayer::EditType QgsFieldsProperties::editTypeFromButtonText( QString text )
{
  return editTypeMap.key( text );
}

void QgsFieldsProperties::reset()
{
/*  QObject::disconnect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );

  QObject::connect( tblAttributes, SIGNAL( cellChanged( int, int ) ), this, SLOT( on_tblAttributes_cellChanged( int, int ) ) );*/
}


void QgsFieldsProperties::apply()
{
  for ( int i = 0; i < tblAttributes->rowCount(); i++ )
  {
    int idx = tblAttributes->item( i, attrIdCol )->text().toInt();

    QPushButton *pb = qobject_cast<QPushButton *>( tblAttributes->cellWidget( i, attrEditTypeCol ) );
    if ( !pb )
      continue;

    QgsVectorLayer::EditType editType = editTypeFromButtonText( pb->text() );
    mLayer->setEditType( idx, editType );

    switch ( editType )
    {
      case QgsVectorLayer::ValueMap:
        if ( mValueMaps.contains( idx ) )
        {
          QMap<QString, QVariant> &map = mLayer->valueMap( idx );
          map.clear();
          map = mValueMaps[idx];
        }
        break;

      case QgsVectorLayer::EditRange:
      case QgsVectorLayer::SliderRange:
      case QgsVectorLayer::DialRange:
        if ( mRanges.contains( idx ) )
        {
          mLayer->range( idx ) = mRanges[idx];
        }
        break;

      case QgsVectorLayer::CheckBox:
        if ( mCheckedStates.contains( idx ) )
        {
          mLayer->setCheckedState( idx, mCheckedStates[idx].first, mCheckedStates[idx].second );
        }
        break;

      case QgsVectorLayer::ValueRelation:
        if ( mValueRelationData.contains( idx ) )
        {
          mLayer->valueRelation( idx ) = mValueRelationData[idx];
        }
        break;

      case QgsVectorLayer::LineEdit:
      case QgsVectorLayer::UniqueValues:
      case QgsVectorLayer::UniqueValuesEditable:
      case QgsVectorLayer::Classification:
      case QgsVectorLayer::FileName:
      case QgsVectorLayer::Enumeration:
      case QgsVectorLayer::Immutable:
      case QgsVectorLayer::Hidden:
      case QgsVectorLayer::TextEdit:
      case QgsVectorLayer::Calendar:
      case QgsVectorLayer::UuidGenerator:
        break;
    }
  }
}
