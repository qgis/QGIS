/***************************************************************************
    qgssourcefieldsproperties.cpp
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssourcefieldsproperties.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"

QgsSourceFieldsProperties::QgsSourceFieldsProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  if ( !layer )
    return;

  setupUi( this );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->setMargin( 0 );

  //button appearance
  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mDeleteAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mCalculateFieldButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) ) );

  //button signals
  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsSourceFieldsProperties::toggleEditing );
  connect( mAddAttributeButton, &QAbstractButton::clicked, this, &QgsSourceFieldsProperties::addAttributeClicked );
  connect( mDeleteAttributeButton, &QAbstractButton::clicked, this, &QgsSourceFieldsProperties::deleteAttributeClicked );
  connect( mCalculateFieldButton, &QAbstractButton::clicked, this, &QgsSourceFieldsProperties::calculateFieldClicked );

  //slots
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsSourceFieldsProperties::editingToggled );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsSourceFieldsProperties::editingToggled );
  connect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsSourceFieldsProperties::attributeAdded );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsSourceFieldsProperties::attributeDeleted );

  //field list appearance
  mFieldsList->setColumnCount( AttrColCount );
  mFieldsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFieldsList->setDragDropMode( QAbstractItemView::DragOnly );
  mFieldsList->setHorizontalHeaderItem( AttrIdCol, new QTableWidgetItem( tr( "Id" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrNameCol, new QTableWidgetItem( tr( "Name" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrTypeCol, new QTableWidgetItem( tr( "Type" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrTypeNameCol, new QTableWidgetItem( tr( "Type name" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrLengthCol, new QTableWidgetItem( tr( "Length" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrPrecCol, new QTableWidgetItem( tr( "Precision" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrCommentCol, new QTableWidgetItem( tr( "Comment" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrWMSCol, new QTableWidgetItem( QStringLiteral( "WMS" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrWFSCol, new QTableWidgetItem( QStringLiteral( "WFS" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  mFieldsList->setSortingEnabled( true );
  mFieldsList->sortByColumn( 0, Qt::AscendingOrder );
  mFieldsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFieldsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mFieldsList->verticalHeader()->hide();

  //load buttons and field list
  updateButtons();
}

void QgsSourceFieldsProperties::init()
{
  loadRows();
}

void QgsSourceFieldsProperties::loadRows()
{
  disconnect( mFieldsList, &QTableWidget::cellChanged, this, &QgsSourceFieldsProperties::attributesListCellChanged );
  const QgsFields &fields = mLayer->fields();

  mIndexedWidgets.clear();
  mFieldsList->setRowCount( 0 );

  for ( int i = 0; i < fields.count(); ++i )
    attributeAdded( i );

  mFieldsList->resizeColumnsToContents();
  connect( mFieldsList, &QTableWidget::cellChanged, this, &QgsSourceFieldsProperties::attributesListCellChanged );

  connect( mFieldsList, &QTableWidget::cellPressed, this, &QgsSourceFieldsProperties::attributesListCellPressed );

  updateButtons();
  updateFieldRenamingStatus();
}

void QgsSourceFieldsProperties::updateFieldRenamingStatus()
{
  bool canRenameFields = mLayer->isEditable() && ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && !mLayer->readOnly();

  for ( int row = 0; row < mFieldsList->rowCount(); ++row )
  {
    if ( canRenameFields )
      mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() | Qt::ItemIsEditable );
    else
      mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() & ~Qt::ItemIsEditable );
  }
}

void QgsSourceFieldsProperties::updateExpression()
{
  QToolButton *btn = qobject_cast<QToolButton *>( sender() );
  Q_ASSERT( btn );

  int index = btn->property( "Index" ).toInt();

  const QString exp = mLayer->expressionField( index );

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  QgsExpressionBuilderDialog dlg( mLayer, exp, nullptr, QStringLiteral( "generic" ), context );

  if ( dlg.exec() )
  {
    mLayer->updateExpressionField( index, dlg.expressionText() );
    loadRows();
  }
}

void QgsSourceFieldsProperties::attributeAdded( int idx )
{
  bool sorted = mFieldsList->isSortingEnabled();
  if ( sorted )
    mFieldsList->setSortingEnabled( false );

  const QgsFields &fields = mLayer->fields();
  int row = mFieldsList->rowCount();
  mFieldsList->insertRow( row );
  setRow( row, idx, fields.at( idx ) );
  mFieldsList->setCurrentCell( row, idx );

  //in case there are rows following, there is increased the id to the correct ones
  for ( int i = idx + 1; i < mIndexedWidgets.count(); i++ )
    mIndexedWidgets.at( i )->setData( Qt::DisplayRole, i );

  if ( sorted )
    mFieldsList->setSortingEnabled( true );

  for ( int i = 0; i < mFieldsList->columnCount(); i++ )
  {
    switch ( mLayer->fields().fieldOrigin( idx ) )
    {
      case QgsFields::OriginExpression:
        if ( i == 7 ) continue;
        mFieldsList->item( row, i )->setBackgroundColor( QColor( 200, 200, 255 ) );
        break;

      case QgsFields::OriginJoin:
        mFieldsList->item( row, i )->setBackgroundColor( QColor( 200, 255, 200 ) );
        break;

      default:
        mFieldsList->item( row, i )->setBackgroundColor( QColor( 255, 255, 200 ) );
        break;
    }
  }
}


void QgsSourceFieldsProperties::attributeDeleted( int idx )
{
  mFieldsList->removeRow( mIndexedWidgets.at( idx )->row() );
  mIndexedWidgets.removeAt( idx );
  for ( int i = idx; i < mIndexedWidgets.count(); i++ )
  {
    mIndexedWidgets.at( i )->setData( Qt::DisplayRole, i );
  }
}

void QgsSourceFieldsProperties::setRow( int row, int idx, const QgsField &field )
{
  QTableWidgetItem *dataItem = new QTableWidgetItem();
  dataItem->setData( Qt::DisplayRole, idx );

  switch ( mLayer->fields().fieldOrigin( idx ) )
  {
    case QgsFields::OriginExpression:
      dataItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
      break;

    case QgsFields::OriginJoin:
      dataItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/join.svg" ) ) );
      break;

    default:
      dataItem->setIcon( mLayer->fields().iconForField( idx ) );
      break;
  }
  mFieldsList->setItem( row, AttrIdCol, dataItem );

  mIndexedWidgets.insert( idx, mFieldsList->item( row, 0 ) );
  mFieldsList->setItem( row, AttrNameCol, new QTableWidgetItem( field.name() ) );
  mFieldsList->setItem( row, AttrAliasCol, new QTableWidgetItem( field.alias() ) );
  mFieldsList->setItem( row, AttrTypeCol, new QTableWidgetItem( QVariant::typeToName( field.type() ) ) );
  mFieldsList->setItem( row, AttrTypeNameCol, new QTableWidgetItem( field.typeName() ) );
  mFieldsList->setItem( row, AttrLengthCol, new QTableWidgetItem( QString::number( field.length() ) ) );
  mFieldsList->setItem( row, AttrPrecCol, new QTableWidgetItem( QString::number( field.precision() ) ) );

  if ( mLayer->fields().fieldOrigin( idx ) == QgsFields::OriginExpression )
  {
    QWidget *expressionWidget = new QWidget;
    expressionWidget->setLayout( new QHBoxLayout );
    QToolButton *editExpressionButton = new QToolButton;
    editExpressionButton->setProperty( "Index", idx );
    editExpressionButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
    connect( editExpressionButton, &QAbstractButton::clicked, this, &QgsSourceFieldsProperties::updateExpression );
    expressionWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
    expressionWidget->layout()->addWidget( editExpressionButton );
    expressionWidget->layout()->addWidget( new QLabel( mLayer->expressionField( idx ) ) );
    expressionWidget->setStyleSheet( "background-color: rgb( 200, 200, 255 )" );
    mFieldsList->setCellWidget( row, AttrCommentCol, expressionWidget );
  }
  else
  {
    mFieldsList->setItem( row, AttrCommentCol, new QTableWidgetItem( field.comment() ) );
  }

  QList<int> notEditableCols = QList<int>()
                               << AttrIdCol
                               << AttrNameCol
                               << AttrAliasCol
                               << AttrTypeCol
                               << AttrTypeNameCol
                               << AttrLengthCol
                               << AttrPrecCol
                               << AttrCommentCol;

  Q_FOREACH ( int i, notEditableCols )
  {
    if ( notEditableCols[i] != AttrCommentCol || mLayer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression )
      mFieldsList->item( row, i )->setFlags( mFieldsList->item( row, i )->flags() & ~Qt::ItemIsEditable );
    if ( notEditableCols[i] == AttrAliasCol )
      mFieldsList->item( row, i )->setToolTip( tr( "Edit alias in the Form config tab" ) );
  }
  bool canRenameFields = mLayer->isEditable() && ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && !mLayer->readOnly();
  if ( canRenameFields )
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() | Qt::ItemIsEditable );
  else
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() & ~Qt::ItemIsEditable );

  //published WMS/WFS attributes
  QTableWidgetItem *wmsAttrItem = new QTableWidgetItem();
  wmsAttrItem->setCheckState( mLayer->excludeAttributesWms().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wmsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  mFieldsList->setItem( row, AttrWMSCol, wmsAttrItem );
  QTableWidgetItem *wfsAttrItem = new QTableWidgetItem();
  wfsAttrItem->setCheckState( mLayer->excludeAttributesWfs().contains( field.name() ) ? Qt::Unchecked : Qt::Checked );
  wfsAttrItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
  mFieldsList->setItem( row, AttrWFSCol, wfsAttrItem );
}

bool QgsSourceFieldsProperties::addAttribute( const QgsField &field )
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
    QMessageBox::critical( this, tr( "Add Field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( field.name(), field.typeName() ) );
    return false;
  }
}

void QgsSourceFieldsProperties::apply()
{
  QSet<QString> excludeAttributesWMS, excludeAttributesWFS;

  for ( int i = 0; i < mFieldsList->rowCount(); i++ )
  {
    if ( mFieldsList->item( i, AttrWMSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWMS.insert( mFieldsList->item( i, AttrNameCol )->text() );
    }
    if ( mFieldsList->item( i, AttrWFSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWFS.insert( mFieldsList->item( i, AttrNameCol )->text() );
    }
  }

  mLayer->setExcludeAttributesWms( excludeAttributesWMS );
  mLayer->setExcludeAttributesWfs( excludeAttributesWFS );

}

//SLOTS

void QgsSourceFieldsProperties::editingToggled()
{
  updateButtons();
  updateFieldRenamingStatus();
}

void QgsSourceFieldsProperties::addAttributeClicked()
{
  QgsAddAttrDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    addAttribute( dialog.field() );
    loadRows();
  }
}

void QgsSourceFieldsProperties::deleteAttributeClicked()
{
  QSet<int> providerFields;
  QSet<int> expressionFields;
  Q_FOREACH ( QTableWidgetItem *item, mFieldsList->selectedItems() )
  {
    if ( item->column() == 0 )
    {
      int idx = mIndexedWidgets.indexOf( item );
      if ( idx < 0 )
        continue;

      if ( mLayer->fields().fieldOrigin( idx ) == QgsFields::OriginExpression )
        expressionFields << idx;
      else
        providerFields << idx;
    }
  }

  if ( !expressionFields.isEmpty() )
    mLayer->deleteAttributes( expressionFields.toList() );

  if ( !providerFields.isEmpty() )
  {
    mLayer->beginEditCommand( tr( "Deleted attributes" ) );
    if ( mLayer->deleteAttributes( providerFields.toList() ) )
      mLayer->endEditCommand();
    else
      mLayer->destroyEditCommand();
  }
}

void QgsSourceFieldsProperties::calculateFieldClicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsFieldCalculator calc( mLayer, this );
  if ( calc.exec() == QDialog::Accepted )
  {
    loadRows();
  }
}

void QgsSourceFieldsProperties::attributesListCellChanged( int row, int column )
{
  if ( column == AttrNameCol && mLayer && mLayer->isEditable() )
  {
    int idx = mIndexedWidgets.indexOf( mFieldsList->item( row, AttrIdCol ) );

    QTableWidgetItem *nameItem = mFieldsList->item( row, column );
    //avoiding that something will be changed, just because this is triggered by simple re-sorting
    if ( !nameItem ||
         nameItem->text().isEmpty() ||
         !mLayer->fields().exists( idx ) ||
         mLayer->fields().at( idx ).name() == nameItem->text()
       )
      return;

    mLayer->beginEditCommand( tr( "Rename attribute" ) );
    if ( mLayer->renameAttribute( idx,  nameItem->text() ) )
    {
      mLayer->endEditCommand();
    }
    else
    {
      mLayer->destroyEditCommand();
      QMessageBox::critical( this, tr( "Rename Field" ), tr( "Failed to rename field to '%1'. Is the field name unique?" ).arg( nameItem->text() ) );
    }
  }
}

void QgsSourceFieldsProperties::attributesListCellPressed( int /*row*/, int /*column*/ )
{
  updateButtons();
}

//NICE FUNCTIONS
void QgsSourceFieldsProperties::updateButtons()
{

  int cap = mLayer->dataProvider()->capabilities();

  mToggleEditingButton->setEnabled( ( cap & QgsVectorDataProvider::ChangeAttributeValues ) && !mLayer->readOnly() );

  if ( mLayer->isEditable() )
  {
    mDeleteAttributeButton->setEnabled( cap & QgsVectorDataProvider::DeleteAttributes );
    mAddAttributeButton->setEnabled( cap & QgsVectorDataProvider::AddAttributes );
    mToggleEditingButton->setChecked( true );
  }
  else
  {
    mToggleEditingButton->setChecked( false );
    mAddAttributeButton->setEnabled( false );

    // Enable delete button if items are selected
    mDeleteAttributeButton->setEnabled( !mFieldsList->selectedItems().isEmpty() );
    // and only if all selected items have their origin in an expression
    Q_FOREACH ( QTableWidgetItem *item, mFieldsList->selectedItems() )
    {
      if ( item->column() == 0 )
      {
        int idx = mIndexedWidgets.indexOf( item );
        if ( mLayer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression )
        {
          mDeleteAttributeButton->setEnabled( false );
          break;
        }
      }
    }
  }
}
