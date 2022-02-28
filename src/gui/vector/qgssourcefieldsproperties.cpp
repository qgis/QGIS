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

#include "qgsaddattrdialog.h"
#include "qgscheckablecombobox.h"
#include "qgssourcefieldsproperties.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsnative.h"


QgsSourceFieldsProperties::QgsSourceFieldsProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
{
  if ( !layer )
    return;

  setupUi( this );
  layout()->setContentsMargins( 0, 0, 0, 0 );

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
  const auto configurationFlagsWi = new QTableWidgetItem( tr( "Configuration" ) );
  configurationFlagsWi->setToolTip( tr( "Configures the field" ) );
  mFieldsList->setHorizontalHeaderItem( AttrConfigurationFlagsCol, configurationFlagsWi );
  mFieldsList->setHorizontalHeaderItem( AttrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  mFieldsList->setSortingEnabled( true );
  mFieldsList->sortByColumn( 0, Qt::AscendingOrder );
  mFieldsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFieldsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mFieldsList->horizontalHeader()->setStretchLastSection( true );
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
  const bool canRenameFields = mLayer->isEditable() && ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && !mLayer->readOnly();

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

  const int index = btn->property( "Index" ).toInt();

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
  const bool sorted = mFieldsList->isSortingEnabled();
  if ( sorted )
    mFieldsList->setSortingEnabled( false );

  const QgsFields &fields = mLayer->fields();
  const int row = mFieldsList->rowCount();
  mFieldsList->insertRow( row );
  setRow( row, idx, fields.at( idx ) );
  mFieldsList->setCurrentCell( row, idx );

  const QColor expressionColor = QColor( 103, 0, 243, 44 );
  const QColor joinColor = QColor( 0, 243, 79, 44 );
  const QColor defaultColor = QColor( 252, 255, 79, 44 );

  for ( int i = 0; i < mFieldsList->columnCount(); i++ )
  {
    if ( i == AttrConfigurationFlagsCol )
      continue;

    switch ( mLayer->fields().fieldOrigin( idx ) )
    {
      case QgsFields::OriginExpression:
        if ( i == 7 ) continue;
        mFieldsList->item( row, i )->setBackground( expressionColor );
        break;

      case QgsFields::OriginJoin:
        mFieldsList->item( row, i )->setBackground( joinColor );
        break;

      default:
        if ( defaultColor.isValid() )
          mFieldsList->item( row, i )->setBackground( defaultColor );
        break;
    }
  }

  if ( sorted )
    mFieldsList->setSortingEnabled( true );
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
  dataItem->setIcon( mLayer->fields().iconForField( idx, true ) );
  mFieldsList->setItem( row, AttrIdCol, dataItem );

  // in case we insert and not append reindex remaining widgets by 1
  for ( int i = idx + 1; i < mIndexedWidgets.count(); i++ )
    mIndexedWidgets.at( i )->setData( Qt::DisplayRole, i );
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

  const auto constNotEditableCols = notEditableCols;
  for ( const int i : constNotEditableCols )
  {
    if ( notEditableCols[i] != AttrCommentCol || mLayer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression )
      mFieldsList->item( row, i )->setFlags( mFieldsList->item( row, i )->flags() & ~Qt::ItemIsEditable );
    if ( notEditableCols[i] == AttrAliasCol )
      mFieldsList->item( row, i )->setToolTip( tr( "Edit alias in the Form config tab" ) );
  }
  const bool canRenameFields = mLayer->isEditable() && ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && !mLayer->readOnly();
  if ( canRenameFields )
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() | Qt::ItemIsEditable );
  else
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() & ~Qt::ItemIsEditable );

  // Flags
  QgsCheckableComboBox *cb = new QgsCheckableComboBox( mFieldsList );
  const QList<QgsField::ConfigurationFlag> flagList = qgsEnumList<QgsField::ConfigurationFlag>();
  for ( const QgsField::ConfigurationFlag flag : flagList )
  {
    if ( flag == QgsField::ConfigurationFlag::None )
      continue;

    cb->addItemWithCheckState( QgsField::readableConfigurationFlag( flag ),
                               mLayer->fieldConfigurationFlags( idx ).testFlag( flag ) ? Qt::Checked : Qt::Unchecked,
                               QVariant::fromValue( flag ) );
  }
  mFieldsList->setCellWidget( row, AttrConfigurationFlagsCol, cb );
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
  for ( int i = 0; i < mFieldsList->rowCount(); i++ )
  {
    const int idx = mFieldsList->item( i, AttrIdCol )->data( Qt::DisplayRole ).toInt();
    QgsField::ConfigurationFlags flags = mLayer->fieldConfigurationFlags( idx );

    QgsCheckableComboBox *cb = qobject_cast<QgsCheckableComboBox *>( mFieldsList->cellWidget( i, AttrConfigurationFlagsCol ) );
    if ( cb )
    {
      QgsCheckableItemModel *model = cb->model();
      for ( int r = 0; r < model->rowCount(); ++r )
      {
        const QModelIndex index = model->index( r, 0 );
        const QgsField::ConfigurationFlag flag = model->data( index, Qt::UserRole ).value<QgsField::ConfigurationFlag>();
        const bool active = model->data( index, Qt::CheckStateRole ).value<Qt::CheckState>() == Qt::Checked ? true : false;
        flags.setFlag( flag, active );
      }
      mLayer->setFieldConfigurationFlags( idx, flags );
    }
  }
}

//SLOTS

void QgsSourceFieldsProperties::editingToggled()
{
  updateButtons();
  updateFieldRenamingStatus();
}

void QgsSourceFieldsProperties::addAttributeClicked()
{
  if ( !mLayer || !mLayer->dataProvider() )
  {
    return;
  }

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
  const auto constSelectedItems = mFieldsList->selectedItems();
  for ( QTableWidgetItem *item : constSelectedItems )
  {
    if ( item->column() == 0 )
    {
      const int idx = mIndexedWidgets.indexOf( item );
      if ( idx < 0 )
        continue;

      if ( mLayer->fields().fieldOrigin( idx ) == QgsFields::OriginExpression )
        expressionFields << idx;
      else
        providerFields << idx;
    }
  }

  if ( !expressionFields.isEmpty() )
    mLayer->deleteAttributes( expressionFields.values() );

  if ( !providerFields.isEmpty() )
  {
    mLayer->beginEditCommand( tr( "Deleted attributes" ) );
    if ( mLayer->deleteAttributes( providerFields.values() ) )
      mLayer->endEditCommand();
    else
      mLayer->destroyEditCommand();
  }
}

void QgsSourceFieldsProperties::calculateFieldClicked()
{
  if ( !mLayer || !mLayer->dataProvider() )
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
    const int idx = mIndexedWidgets.indexOf( mFieldsList->item( row, AttrIdCol ) );

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
  QgsVectorDataProvider *provider = mLayer->dataProvider();
  if ( !provider )
    return;
  const QgsVectorDataProvider::Capabilities cap = provider->capabilities();

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
    const auto constSelectedItems = mFieldsList->selectedItems();
    for ( QTableWidgetItem *item : constSelectedItems )
    {
      if ( item->column() == 0 )
      {
        const int idx = mIndexedWidgets.indexOf( item );
        if ( mLayer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression )
        {
          mDeleteAttributeButton->setEnabled( false );
          break;
        }
      }
    }
  }
}
