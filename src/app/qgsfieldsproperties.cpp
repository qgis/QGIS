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
#include "qgsaddtaborgroup.h"

#include <QTreeWidgetItem>
#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

QTreeWidgetItem* QgsAttributesTree::addContainer( QTreeWidgetItem* parent, QString title )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QList<QString>() << title );
  newItem->setBackground( 0 , QBrush( Qt::lightGray ) );
  newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
  newItem->setData( 0 , Qt::UserRole , "container" );
  newItem->setExpanded( true );
  parent->addChild( newItem );
  return newItem;
}

QTreeWidgetItem* QgsAttributesTree::addItem( QTreeWidgetItem* parent , QString fieldName )
{
  QTreeWidgetItem* attributeItem = new QTreeWidgetItem( QList<QString>() << fieldName );
  attributeItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
  attributeItem->setData( 0 , Qt::UserRole , "field" );
  parent->addChild( attributeItem );

  return attributeItem;
}

/*
 * Is called when mouse is moved over attributes tree before a
 * drop event. Used to inhibit dropping fields onto the root item.
 */

void QgsAttributesTree::dragMoveEvent( QDragMoveEvent *event )
{
  QTreeWidgetItem* targetItem = itemAt( event->pos() );
  const QMimeData* data = event->mimeData();

  if ( data->hasFormat( "application/x-qabstractitemmodeldatalist" ) )
  {
    QString itemType;
    if ( event->source() == this )
    {
      QByteArray itemData = data->data( "application/x-qabstractitemmodeldatalist" );
      QDataStream stream( &itemData, QIODevice::ReadOnly );
      int r, c;
      QMap<int, QVariant> roleDataMap;
      stream >> r >> c >> roleDataMap;

      itemType = roleDataMap.value( Qt::UserRole ).toString();
    }
    else
    {
      itemType = "field";
    }

    // Forbid dropping fields on root item
    if ( itemType == "field" && !targetItem )
    {
      event->ignore();
      return;
    }

    // Inner drag and drop actions are always MoveAction
    if ( event->source() == this )
    {
      event->setDropAction( Qt::MoveAction );
    }
  }

  QTreeWidget::dragMoveEvent( event );
}


bool QgsAttributesTree::dropMimeData( QTreeWidgetItem * parent, int index, const QMimeData * data, Qt::DropAction action )
{
  bool bDropSuccessful = false;

  if ( action == Qt::IgnoreAction )
  {
    bDropSuccessful = true;
  }
  else if ( data->hasFormat( "application/x-qabstractitemmodeldatalist" ) )
  {
    QByteArray itemData = data->data( "application/x-qabstractitemmodeldatalist" );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    int r, c;
    QMap<int, QVariant> roleDataMap;
    stream >> r >> c >> roleDataMap;

    QString itemType = roleDataMap.value( Qt::UserRole ).toString();
    QString itemName = roleDataMap.value( Qt::DisplayRole ).toString();

    if ( itemType == "field" ) //
    {
      if ( parent )
      {
        addItem( parent, itemName );
        bDropSuccessful = true;
      }
      else // Should never happen as we ignore drops of fields onto the root element in dragMoveEvent, but actually does happen. Qt?
      {
        // addItem( invisibleRootItem(), itemName );
        // bDropSuccessful = true;
      }
    }
    else
    {
      bDropSuccessful = QTreeWidget::dropMimeData( parent, index, data, Qt::MoveAction );
    }
  }

  return bDropSuccessful;
}

void QgsAttributesTree::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasFormat( "application/x-qabstractitemmodeldatalist" ) )
    return;

  if ( event->source() == this )
  {
    event->setDropAction( Qt::MoveAction );
    QTreeWidget::dropEvent( event );
  }
  else
  {
    // Qt::DropAction dropAction;
    QByteArray itemData = event->mimeData()->data( "application/x-qabstractitemmodeldatalist" );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    int r, c, rDummy, cDummy;
    QMap<int, QVariant> roleDataMap, newRoleDataMap, roleDataMapDummy;
    stream >> rDummy >> cDummy >> roleDataMapDummy >> r >> c >> roleDataMap; // fieldName is in second column

    QString fieldName = roleDataMap.value( Qt::DisplayRole ).toString();
    newRoleDataMap.insert( Qt::UserRole , "field" );
    newRoleDataMap.insert( Qt::DisplayRole , fieldName );

    QMimeData * mimeData = new QMimeData();
    QByteArray mdata;
    QDataStream newStream( &mdata, QIODevice::WriteOnly );
    newStream << r << c << newRoleDataMap;
    mimeData->setData( QString( "application/x-qabstractitemmodeldatalist" ), mdata );
    QDropEvent newEvent = QDropEvent( event->pos(), Qt::CopyAction , mimeData, event->mouseButtons(), event->keyboardModifiers() );

    QTreeWidget::dropEvent( &newEvent );
  }
}

QgsFieldsProperties::QgsFieldsProperties( QgsVectorLayer *layer, QWidget* parent )
    : QWidget( parent ), mLayer( layer )
{
  if ( !layer )
    return;

  setupUi( this );
  setupEditTypes();

  // Init as hidden by default, it will be enabled if project is set to
  mAttributeEditorOptionsWidget->setVisible( false );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.png" ) );
  mDeleteAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( "/mActionToggleEditing.svg" ) );
  mCalculateFieldButton->setIcon( QgsApplication::getThemeIcon( "/mActionCalculateField.png" ) );

  connect( mToggleEditingButton, SIGNAL( clicked() ), this, SIGNAL( toggleEditing() ) );
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
  connect( mLayer, SIGNAL( attributeAdded( int ) ), this, SLOT( attributeAdded( int ) ) );
  connect( mLayer, SIGNAL( attributeDeleted( int ) ), this, SLOT( attributeDeleted( int ) ) );

  // tab and group display
  mAddItemButton->setEnabled( false );

  QVBoxLayout *attrTreeLayout = new QVBoxLayout( mAttributesTreeFrame );
  QVBoxLayout *attrListLayout = new QVBoxLayout( mAttributesListFrame );

  attrTreeLayout->setMargin( 0 );
  attrListLayout->setMargin( 0 );
  mAttributesTree = new QgsAttributesTree( mAttributesTreeFrame );
  mAttributesList = new QgsAttributesList( mAttributesListFrame );
  attrTreeLayout->addWidget( mAttributesTree );
  attrListLayout->addWidget( mAttributesList );
  mAttributesTreeFrame->setLayout( attrTreeLayout );
  mAttributesListFrame->setLayout( attrListLayout );

  connect( mAttributesTree, SIGNAL( itemSelectionChanged() ), this, SLOT( onAttributeSelectionChanged() ) );
  connect( mAttributesList, SIGNAL( itemSelectionChanged() ), this, SLOT( onAttributeSelectionChanged() ) );

  mAttributesTree->setHeaderLabels( QStringList() << tr( "Label" ) );

  leEditForm->setText( layer->editForm() );
  leEditFormInit->setText( layer->editFormInit() );

  mEditorLayoutComboBox->setCurrentIndex( layer->editorLayout() );

  loadAttributeEditorTree();
  updateButtons();
}

void QgsFieldsProperties::init()
{
  loadRows();
}

void QgsFieldsProperties::onAttributeSelectionChanged()
{
  bool isAddPossible = false;
  if ( mAttributesTree->selectedItems().count() == 1 && mAttributesList->selectedItems().count() > 0 )
    if ( mAttributesTree->selectedItems()[0]->data( 0, Qt::UserRole ) != "field" )
      isAddPossible = true;
  mAddItemButton->setEnabled( isAddPossible );
}

QTreeWidgetItem *QgsFieldsProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement* const widgetDef, QTreeWidgetItem* parent )
{
  QTreeWidgetItem* newWidget = 0;
  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
      newWidget = mAttributesTree->addItem( parent, widgetDef->name() );
      break;

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      newWidget = mAttributesTree->addContainer( parent, widgetDef->name() );

      const QgsAttributeEditorContainer* container = dynamic_cast<const QgsAttributeEditorContainer*>( widgetDef );
      QList<QgsAttributeEditorElement*> children = container->children();
      for ( QList<QgsAttributeEditorElement*>::const_iterator it = children.begin(); it != children.end(); ++it )
      {
        loadAttributeEditorTreeItem( *it, newWidget );
      }
    }
    break;

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }
  return newWidget;
}

void QgsFieldsProperties::loadAttributeEditorTree()
{
  // tabs and groups info
  mAttributesTree->clear();
  mAttributesTree->setSortingEnabled( false );
  mAttributesTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAttributesTree->setDragDropMode( QAbstractItemView::InternalMove );
  mAttributesTree->setAcceptDrops( true );
  mAttributesTree->setDragDropMode( QAbstractItemView::DragDrop );
  // tabs and groups info
  mAttributesTree->clear();
  mAttributesTree->setSortingEnabled( false );
  mAttributesTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAttributesTree->setDragDropMode( QAbstractItemView::InternalMove );
  mAttributesTree->setAcceptDrops( true );
  mAttributesTree->setDragDropMode( QAbstractItemView::DragDrop );

  QList<QgsAttributeEditorElement*> widgets = mLayer->attributeEditorElements();

  for ( QList<QgsAttributeEditorElement*>::const_iterator it = widgets.begin(); it != widgets.end(); ++it )
  {
    loadAttributeEditorTreeItem( *it, mAttributesTree->invisibleRootItem() );
  }
}

void QgsFieldsProperties::loadRows()
{
  disconnect( mAttributesList, SIGNAL( cellChanged( int, int ) ), this, SLOT( attributesListCellChanged( int, int ) ) );
  const QgsFields &fields = mLayer->pendingFields();

  mAttributesList->clear();

  mAttributesList->setColumnCount( attrColCount );
  mAttributesList->setRowCount( fields.size() );
  mAttributesList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAttributesList->setDragDropMode( QAbstractItemView::DragOnly );
  mAttributesList->setHorizontalHeaderItem( attrIdCol, new QTableWidgetItem( tr( "Id" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrNameCol, new QTableWidgetItem( tr( "Name" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrTypeCol, new QTableWidgetItem( tr( "Type" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrLengthCol, new QTableWidgetItem( tr( "Length" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrPrecCol, new QTableWidgetItem( tr( "Precision" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrCommentCol, new QTableWidgetItem( tr( "Comment" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrEditTypeCol, new QTableWidgetItem( tr( "Edit widget" ) ) );
  mAttributesList->setHorizontalHeaderItem( attrWMSCol, new QTableWidgetItem( "WMS" ) );
  mAttributesList->setHorizontalHeaderItem( attrWFSCol, new QTableWidgetItem( "WFS" ) );
  mAttributesList->setHorizontalHeaderItem( attrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  mAttributesList->setSortingEnabled( true );
  mAttributesList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mAttributesList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mAttributesList->verticalHeader()->hide();

  for ( int i = 0; i < fields.count(); ++i )
    setRow( i, i, fields[i] );

  mAttributesList->resizeColumnsToContents();
  connect( mAttributesList, SIGNAL( cellChanged( int, int ) ), this, SLOT( attributesListCellChanged( int, int ) ) );
}

void QgsFieldsProperties::setRow( int row, int idx, const QgsField &field )
{
  mAttributesList->setItem( row, attrIdCol, new QTableWidgetItem( QString::number( idx ) ) );
  mAttributesList->setItem( row, attrNameCol, new QTableWidgetItem( field.name() ) );
  mAttributesList->setItem( row, attrTypeCol, new QTableWidgetItem( field.typeName() ) );
  mAttributesList->setItem( row, attrLengthCol, new QTableWidgetItem( QString::number( field.length() ) ) );
  mAttributesList->setItem( row, attrPrecCol, new QTableWidgetItem( QString::number( field.precision() ) ) );
  mAttributesList->setItem( row, attrCommentCol, new QTableWidgetItem( field.comment() ) );

  for ( int i = 0; i < attrEditTypeCol; i++ )
    mAttributesList->item( row, i )->setFlags( mAttributesList->item( row, i )->flags() & ~Qt::ItemIsEditable );

  QPushButton *pb = new QPushButton( editTypeButtonText( mLayer->editType( idx ) ) );
  mAttributesList->setCellWidget( row, attrEditTypeCol, pb );
  connect( pb, SIGNAL( pressed() ), this, SLOT( attributeTypeDialog( ) ) );
  mButtonMap.insert( idx, pb );

  //set the alias for the attribute
  mAttributesList->setItem( row, attrAliasCol, new QTableWidgetItem( mLayer->attributeAlias( idx ) ) );

  //published WMS/WFS attributes
  QTableWidgetItem* wmsAttrItem = new QTableWidgetItem();
  wmsAttrItem->setCheckState( mLayer->excludeAttributesWMS().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wmsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  mAttributesList->setItem( row, attrWMSCol, wmsAttrItem );
  QTableWidgetItem* wfsAttrItem = new QTableWidgetItem();
  wfsAttrItem->setCheckState( mLayer->excludeAttributesWFS().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wfsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  mAttributesList->setItem( row, attrWFSCol, wfsAttrItem );
}

void QgsFieldsProperties::on_mAddItemButton_clicked()
{
  QList<QTableWidgetItem*> listItems = mAttributesList->selectedItems();
  QList<QTreeWidgetItem*> treeItems = mAttributesTree->selectedItems();

  if ( treeItems.count() != 1 && listItems.count() == 0 )
    return;

  QTreeWidgetItem *parent = treeItems[0];
  if ( parent->data( 0, Qt::UserRole ) == "field" )
    return;

  for ( QList<QTableWidgetItem*>::const_iterator it = listItems.begin(); it != listItems.end(); it++ )
  {
    if (( *it )->column() == attrNameCol )
      mAttributesTree->addItem( parent , ( *it )->text() );
  }
}

void QgsFieldsProperties::on_mAddTabOrGroupButton_clicked()
{
  QList<QString> tabList;
  QList<QTreeWidgetItem*> tabWidgetList;
  QTreeWidgetItemIterator it( mAttributesTree );
  while ( *it )
  {
    if (( *it )->data( 0 , Qt::UserRole ) == "container" )
      tabList.append(( *it )->text( 0 ) );
    tabWidgetList.append( *it );
    ++it;
  }
  QgsAddTabOrGroup addTabOrGroup( mLayer , this, tabList );

  if ( !addTabOrGroup.exec() )
    return;

  QString name = addTabOrGroup.name();
  if ( addTabOrGroup.tabButtonIsChecked() )
  {
    mAttributesTree->addContainer( mAttributesTree->invisibleRootItem(), name );
  }
  else
  {
    int tabId = addTabOrGroup.tabId();
    QTreeWidgetItem* tabItem = tabWidgetList[tabId];
    mAttributesTree->addContainer( tabItem , name );
  }
}

void QgsFieldsProperties::on_mRemoveTabGroupItemButton_clicked()
{
  QList<QTreeWidgetItem*> items = mAttributesTree->selectedItems();
  for ( QList<QTreeWidgetItem*>::const_iterator it = items.begin(); it != items.end(); it++ )
  {
    delete *it;
  }
}

void QgsFieldsProperties::on_mMoveDownItem_clicked()
{
  QList<QTreeWidgetItem*> itemList = mAttributesTree->selectedItems();
  if ( itemList.count() != 1 )
    return;

  QTreeWidgetItem* itemToMoveDown = itemList.first();
  QTreeWidgetItem* parent = itemToMoveDown->parent();
  if ( !parent )
  {
    parent = mAttributesTree->invisibleRootItem();
  }
  int itemIndex = parent->indexOfChild( itemToMoveDown );

  if ( itemIndex < parent->childCount() - 1 )
  {
    parent->takeChild( itemIndex );
    parent->insertChild( itemIndex + 1, itemToMoveDown );

    itemToMoveDown->setSelected( true );
    parent->child( itemIndex )->setSelected( false );
  }
}

void QgsFieldsProperties::on_mMoveUpItem_clicked()
{
  QList<QTreeWidgetItem*> itemList = mAttributesTree->selectedItems();
  if ( itemList.count() != 1 )
    return;

  QTreeWidgetItem* itemToMoveUp = itemList.first();
  QTreeWidgetItem* parent = itemToMoveUp->parent();
  if ( !parent )
  {
    parent = mAttributesTree->invisibleRootItem();
  }
  int itemIndex = parent->indexOfChild( itemToMoveUp );

  if ( itemIndex > 0 )
  {
    parent->takeChild( itemIndex );
    parent->insertChild( itemIndex - 1, itemToMoveUp );

    itemToMoveUp->setSelected( true );
    parent->child( itemIndex )->setSelected( false );
  }
}

void QgsFieldsProperties::attributeTypeDialog()
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

  attributeTypeDialog.setDateFormat( mDateFormat.value( index, mLayer->dateFormat( index ) ) );
  attributeTypeDialog.setWidgetSize( mWidgetSize.value( index, mLayer->widgetSize( index ) ) );
  attributeTypeDialog.setFieldEditable( mFieldEditables.value( index, mLayer->fieldEditable( index ) ) );

  attributeTypeDialog.setIndex( index, mEditTypeMap.value( index, mLayer->editType( index ) ) );

  if ( !attributeTypeDialog.exec() )
    return;

  QgsVectorLayer::EditType editType = attributeTypeDialog.editType();
  mEditTypeMap.insert( index, editType );

  bool isFieldEditable = attributeTypeDialog.fieldEditable();
  mFieldEditables.insert( index, isFieldEditable );

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
    case QgsVectorLayer::Calendar:
      mDateFormat.insert( index, attributeTypeDialog.dateFormat() );
      break;
    case QgsVectorLayer::Photo:
      mWidgetSize.insert( index, attributeTypeDialog.widgetSize() );
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
    case QgsVectorLayer::UuidGenerator:
    case QgsVectorLayer::WebView:
    case QgsVectorLayer::Color:
      break;
  }

  pb->setText( editTypeButtonText( editType ) );
}


void QgsFieldsProperties::attributeAdded( int idx )
{
  const QgsFields &fields = mLayer->pendingFields();
  int row = mAttributesList->rowCount();
  mAttributesList->insertRow( row );
  setRow( row, idx, fields[idx] );
  mAttributesList->setCurrentCell( row, idx );
}


void QgsFieldsProperties::attributeDeleted( int idx )
{
  for ( int i = 0; i < mAttributesList->rowCount(); i++ )
  {
    if ( mAttributesList->item( i, 0 )->text().toInt() == idx )
    {
      mAttributesList->removeRow( i );
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
  QList<QTableWidgetItem*> items = mAttributesList->selectedItems();
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
  QList<QTableWidgetItem*> items = mAttributesList->selectedItems();
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
  int cap = mLayer->dataProvider()->capabilities();

  mToggleEditingButton->setEnabled(( cap & QgsVectorDataProvider::ChangeAttributeValues ) && !mLayer->isReadOnly() );

  if ( mLayer->isEditable() )
  {
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
    mCalculateFieldButton->setEnabled( false );
  }
}


void QgsFieldsProperties::attributesListCellChanged( int row, int column )
{
  if ( column == attrAliasCol && mLayer ) //only consider attribute aliases in this function
  {
    int idx = mAttributesList->item( row, attrIdCol )->text().toInt();

    const QgsFields &fields = mLayer->pendingFields();

    if ( idx >= fields.count() )
    {
      return; // index must be wrong
    }

    QTableWidgetItem *aliasItem = mAttributesList->item( row, column );
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
  editTypeMap.insert( QgsVectorLayer::Photo, tr( "Photo" ) );
  editTypeMap.insert( QgsVectorLayer::WebView, tr( "Web view" ) );
  editTypeMap.insert( QgsVectorLayer::Color, tr( "Color" ) );
}

QString QgsFieldsProperties::editTypeButtonText( QgsVectorLayer::EditType type )
{
  return editTypeMap[ type ];
}

QgsVectorLayer::EditType QgsFieldsProperties::editTypeFromButtonText( QString text )
{
  return editTypeMap.key( text );
}

QgsAttributeEditorElement* QgsFieldsProperties::createAttributeEditorWidget( QTreeWidgetItem* item, QObject *parent )
{
  QgsAttributeEditorElement* widgetDef;

  if ( item->data( 0, Qt::UserRole ) == "field" )
  {
    int idx = *( mLayer->dataProvider()->fieldNameMap() ).find( item->text( 0 ) );
    widgetDef = new QgsAttributeEditorField( item->text( 0 ), idx, parent );
  }
  else
  {
    QgsAttributeEditorContainer* container = new QgsAttributeEditorContainer( item->text( 0 ), parent );

    for ( int t = 0; t < item->childCount(); t++ )
    {
      container->addChildElement( createAttributeEditorWidget( item->child( t ), container ) );
    }

    widgetDef = container;
  }

  return widgetDef;
}

void QgsFieldsProperties::on_pbnSelectEditForm_clicked()
{
  QSettings myQSettings;
  QString lastUsedDir = myQSettings.value( "style/lastUIDir", "." ).toString();
  QString uifilename = QFileDialog::getOpenFileName( this, tr( "Select edit form" ), lastUsedDir, tr( "UI file" )  + " (*.ui)" );

  if ( uifilename.isNull() )
    return;

  leEditForm->setText( uifilename );
}

void QgsFieldsProperties::on_mEditorLayoutComboBox_currentIndexChanged( int index )
{
  switch ( index )
  {
    case 0:
      mAttributeEditorOptionsWidget->setVisible( false );
      break;

    case 1:
      mAttributeEditorOptionsWidget->setVisible( true );
      mAttributeEditorOptionsWidget->setCurrentIndex( 1 );
      break;

    case 2:
      mAttributeEditorOptionsWidget->setVisible( true );
      mAttributeEditorOptionsWidget->setCurrentIndex( 0 );
      break;
  }
}

void QgsFieldsProperties::apply()
{
  QSet<QString> excludeAttributesWMS, excludeAttributesWFS;

  for ( int i = 0; i < mAttributesList->rowCount(); i++ )
  {
    int idx = mAttributesList->item( i, attrIdCol )->text().toInt();

    QPushButton *pb = qobject_cast<QPushButton *>( mAttributesList->cellWidget( i, attrEditTypeCol ) );
    if ( !pb )
      continue;

    QgsVectorLayer::EditType editType = editTypeFromButtonText( pb->text() );
    mLayer->setEditType( idx, editType );

    if ( mFieldEditables.contains( idx ) )
      mLayer->setFieldEditable( idx, mFieldEditables[idx] );

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

      case QgsVectorLayer::Calendar:
        if ( mDateFormat.contains( idx ) )
        {
          mLayer->dateFormat( idx ) = mDateFormat[idx];
        }
        break;

      case QgsVectorLayer::Photo:
        if ( mWidgetSize.contains( idx ) )
        {
          mLayer->widgetSize( idx ) = mWidgetSize[idx];
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
      case QgsVectorLayer::UuidGenerator:
      case QgsVectorLayer::WebView:
      case QgsVectorLayer::Color:
        break;
    }

    if ( mAttributesList->item( i, attrWMSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWMS.insert( mAttributesList->item( i, attrNameCol )->text() );
    }
    if ( mAttributesList->item( i, attrWFSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWFS.insert( mAttributesList->item( i, attrNameCol )->text() );
    }
  }

  //tabs and groups
  mLayer->clearAttributeEditorWidgets();
  for ( int t = 0; t < mAttributesTree->invisibleRootItem()->childCount(); t++ )
  {
    QTreeWidgetItem* tabItem = mAttributesTree->invisibleRootItem()->child( t );

    mLayer->addAttributeEditorWidget( createAttributeEditorWidget( tabItem, mLayer ) );
  }

  mLayer->setEditorLayout(( QgsVectorLayer::EditorLayout )mEditorLayoutComboBox->currentIndex() );
  mLayer->setEditForm( leEditForm->text() );
  mLayer->setEditFormInit( leEditFormInit->text() );

  mLayer->setExcludeAttributesWMS( excludeAttributesWMS );
  mLayer->setExcludeAttributesWFS( excludeAttributesWFS );
}
