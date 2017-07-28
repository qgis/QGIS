/***************************************************************************
    qgsfieldsproperties.cpp
    ---------------------
    begin                : September 2012
    copyright            : (C) 2012 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgsaddattrdialog.h"
#include "qgsaddtaborgroup.h"
#include "qgsapplication.h"
#include "qgsattributetypedialog.h"
#include "qgsfieldcalculator.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldsproperties.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfieldexpressionwidget.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include <QTreeWidgetItem>
#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QFormLayout>

QgsFieldsProperties::QgsFieldsProperties( QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mLayer( layer )
  , mDesignerTree( nullptr )
  , mFieldsList( nullptr )
  , mRelationsList( nullptr )
{
  if ( !layer )
    return;

  setupUi( this );

  mSplitter->restoreState( QgsSettings().value( QStringLiteral( "/Windows/VectorLayerProperties/FieldsProperties/SplitState" ) ).toByteArray() );

  // Init as hidden by default, it will be enabled if project is set to
  mAttributeEditorOptionsWidget->setVisible( false );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mDeleteAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mToggleEditingButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionToggleEditing.svg" ) ) );
  mCalculateFieldButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionCalculateField.svg" ) ) );

  connect( mToggleEditingButton, &QAbstractButton::clicked, this, &QgsFieldsProperties::toggleEditing );
  connect( mLayer, &QgsVectorLayer::editingStarted, this, &QgsFieldsProperties::editingToggled );
  connect( mLayer, &QgsVectorLayer::editingStopped, this, &QgsFieldsProperties::editingToggled );
  connect( mLayer, &QgsVectorLayer::attributeAdded, this, &QgsFieldsProperties::attributeAdded );
  connect( mLayer, &QgsVectorLayer::attributeDeleted, this, &QgsFieldsProperties::attributeDeleted );

  // tab and group display
  mAddItemButton->setEnabled( false );

  mDesignerTree = new DesignerTree( mLayer, mAttributesTreeFrame );
  mDesignerListLayout->addWidget( mDesignerTree );
  mDesignerTree->setHeaderLabels( QStringList() << tr( "Label" ) );

  mFieldsList = new DragList( mAttributesListFrame );
  mAttributesListLayout->addWidget( mFieldsList );

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
  mFieldsList->setHorizontalHeaderItem( AttrEditTypeCol, new QTableWidgetItem( tr( "Edit widget" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrWMSCol, new QTableWidgetItem( QStringLiteral( "WMS" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrWFSCol, new QTableWidgetItem( QStringLiteral( "WFS" ) ) );
  mFieldsList->setHorizontalHeaderItem( AttrAliasCol, new QTableWidgetItem( tr( "Alias" ) ) );

  mFieldsList->setSortingEnabled( true );
  mFieldsList->sortByColumn( 0, Qt::AscendingOrder );
  mFieldsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mFieldsList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mFieldsList->verticalHeader()->hide();

  connect( mDesignerTree, &QTreeWidget::itemSelectionChanged, this, &QgsFieldsProperties::onAttributeSelectionChanged );
  connect( mFieldsList, &QTableWidget::itemSelectionChanged, this, &QgsFieldsProperties::onAttributeSelectionChanged );

  mRelationsList = new DragList( mRelationsFrame );
  mRelationsFrameLayout->addWidget( mRelationsList );
  mRelationsList->setColumnCount( RelColCount );
  mRelationsList->setDragDropMode( QAbstractItemView::DragOnly );
  mRelationsList->setSelectionMode( QAbstractItemView::SingleSelection );
  mRelationsList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mRelationsList->setHorizontalHeaderItem( RelIdCol, new QTableWidgetItem( tr( "Id" ) ) );
  mRelationsList->setHorizontalHeaderItem( RelNameCol, new QTableWidgetItem( tr( "Name" ) ) );
  mRelationsList->setHorizontalHeaderItem( RelLayerCol, new QTableWidgetItem( tr( "Layer" ) ) );
  mRelationsList->setHorizontalHeaderItem( RelFieldCol, new QTableWidgetItem( tr( "Field" ) ) );
  mRelationsList->setHorizontalHeaderItem( RelNmCol, new QTableWidgetItem( tr( "Cardinality" ) ) );
  mRelationsList->verticalHeader()->hide();
  mRelationsList->horizontalHeader()->setStretchLastSection( true );

  // Init function stuff
  mInitCodeSourceComboBox->addItem( tr( "" ) );
  mInitCodeSourceComboBox->addItem( tr( "Load from external file" ) );
  mInitCodeSourceComboBox->addItem( tr( "Provide code in this dialog" ) );
  mInitCodeSourceComboBox->addItem( tr( "Load from the environment" ) );

  loadRelations();

  updateButtons();
}

QgsFieldsProperties::~QgsFieldsProperties()
{
  QgsSettings().setValue( QStringLiteral( "/Windows/VectorLayerProperties/FieldsProperties/SplitState" ), mSplitter->saveState() );
}

void QgsFieldsProperties::init()
{
  loadRows();

  mEditorLayoutComboBox->setCurrentIndex( mLayer->editFormConfig().layout() );
  mFormSuppressCmbBx->setCurrentIndex( mLayer->editFormConfig().suppress() );

  loadAttributeEditorTree();
}

void QgsFieldsProperties::onAttributeSelectionChanged()
{
  bool isAddPossible = false;
  if ( mDesignerTree->selectedItems().count() == 1 && !mFieldsList->selectedItems().isEmpty() )
    if ( mDesignerTree->selectedItems()[0]->data( 0, DesignerTreeRole ).value<DesignerTreeItemData>().type() == DesignerTreeItemData::Container )
      isAddPossible = true;
  mAddItemButton->setEnabled( isAddPossible );

  updateButtons();
}

QTreeWidgetItem *QgsFieldsProperties::loadAttributeEditorTreeItem( QgsAttributeEditorElement *const widgetDef, QTreeWidgetItem *parent )
{
  QTreeWidgetItem *newWidget = nullptr;
  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      DesignerTreeItemData itemData = DesignerTreeItemData( DesignerTreeItemData::Field, widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      newWidget = mDesignerTree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation *relationEditor = static_cast<const QgsAttributeEditorRelation *>( widgetDef );
      DesignerTreeItemData itemData = DesignerTreeItemData( DesignerTreeItemData::Relation, widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );
      RelationEditorConfiguration relEdConfig;
      relEdConfig.showLinkButton = relationEditor->showLinkButton();
      relEdConfig.showUnlinkButton = relationEditor->showUnlinkButton();
      itemData.setRelationEditorConfiguration( relEdConfig );

      newWidget = mDesignerTree->addItem( parent, itemData );
      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      DesignerTreeItemData itemData( DesignerTreeItemData::Container, widgetDef->name() );
      itemData.setShowLabel( widgetDef->showLabel() );

      const QgsAttributeEditorContainer *container = static_cast<const QgsAttributeEditorContainer *>( widgetDef );
      if ( !container )
        break;

      itemData.setColumnCount( container->columnCount() );
      itemData.setShowAsGroupBox( container->isGroupBox() );
      itemData.setVisibilityExpression( container->visibilityExpression() );
      newWidget = mDesignerTree->addItem( parent, itemData );

      Q_FOREACH ( QgsAttributeEditorElement *wdg, container->children() )
      {
        loadAttributeEditorTreeItem( wdg, newWidget );
      }
    }
    break;

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }
  return newWidget;
}

void QgsFieldsProperties::setEditFormInit( const QString &editForm,
    const QString &initFunction,
    const QString &initCode,
    const QString &initFilePath,
    QgsEditFormConfig::PythonInitCodeSource codeSource )
{

  // Python init function and code
  QString code( initCode );
  if ( code.isEmpty() )
  {
    code.append( tr( "# -*- coding: utf-8 -*-\n\"\"\"\n"
                     "QGIS forms can have a Python function that is called when the form is\n"
                     "opened.\n"
                     "\n"
                     "Use this function to add extra logic to your forms.\n"
                     "\n"
                     "Enter the name of the function in the \"Python Init function\"\n"
                     "field.\n"
                     "An example follows:\n"
                     "\"\"\"\n"
                     "from qgis.PyQt.QtWidgets import QWidget\n\n"
                     "def my_form_open(dialog, layer, feature):\n"
                     "\tgeom = feature.geometry()\n"
                     "\tcontrol = dialog.findChild(QWidget, \"MyLineEdit\")\n" ) );

  }
  mEditFormLineEdit->setText( editForm );
  mInitFilePathLineEdit->setText( initFilePath );
  mInitCodeEditorPython->setText( code );
  mInitFunctionLineEdit->setText( initFunction );
  mInitCodeSourceComboBox->setCurrentIndex( codeSource );
}


void QgsFieldsProperties::loadAttributeEditorTree()
{
  // tabs and groups info
  mDesignerTree->clear();
  mDesignerTree->setSortingEnabled( false );
  mDesignerTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  mDesignerTree->setDragDropMode( QAbstractItemView::InternalMove );
  mDesignerTree->setAcceptDrops( true );
  mDesignerTree->setDragDropMode( QAbstractItemView::DragDrop );

  Q_FOREACH ( QgsAttributeEditorElement *wdg, mLayer->editFormConfig().tabs() )
  {
    loadAttributeEditorTreeItem( wdg, mDesignerTree->invisibleRootItem() );
  }
}

void QgsFieldsProperties::loadRows()
{
  disconnect( mFieldsList, &QTableWidget::cellChanged, this, &QgsFieldsProperties::attributesListCellChanged );
  const QgsFields &fields = mLayer->fields();

  mIndexedWidgets.clear();
  mFieldsList->setRowCount( 0 );

  for ( int i = 0; i < fields.count(); ++i )
    attributeAdded( i );

  mFieldsList->resizeColumnsToContents();
  connect( mFieldsList, &QTableWidget::cellChanged, this, &QgsFieldsProperties::attributesListCellChanged );
  updateFieldRenamingStatus();
}

void QgsFieldsProperties::setRow( int row, int idx, const QgsField &field )
{
  QTableWidgetItem *dataItem = new QTableWidgetItem();
  dataItem->setData( Qt::DisplayRole, idx );
  DesignerTreeItemData itemData( DesignerTreeItemData::Field, field.name() );
  dataItem->setData( DesignerTreeRole, itemData.asQVariant() );
  switch ( mLayer->fields().fieldOrigin( idx ) )
  {
    case QgsFields::OriginExpression:
      dataItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
      break;

    case QgsFields::OriginJoin:
      dataItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/join.png" ) ) );
      break;

    default:
      dataItem->setIcon( mLayer->fields().iconForField( idx ) );
      break;
  }
  mFieldsList->setItem( row, AttrIdCol, dataItem );
  mIndexedWidgets.insert( idx, mFieldsList->item( row, 0 ) );
  mFieldsList->setItem( row, AttrNameCol, new QTableWidgetItem( field.name() ) );
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
    connect( editExpressionButton, &QAbstractButton::clicked, this, &QgsFieldsProperties::updateExpression );
    expressionWidget->layout()->setContentsMargins( 0, 0, 0, 0 );
    expressionWidget->layout()->addWidget( editExpressionButton );
    expressionWidget->layout()->addWidget( new QLabel( mLayer->expressionField( idx ) ) );
    mFieldsList->setCellWidget( row, AttrCommentCol, expressionWidget );
  }
  else
  {
    mFieldsList->setItem( row, AttrCommentCol, new QTableWidgetItem( field.comment() ) );
  }

  QList<int> notEditableCols = QList<int>()
                               << AttrIdCol
                               << AttrNameCol
                               << AttrTypeCol
                               << AttrTypeNameCol
                               << AttrLengthCol
                               << AttrPrecCol;
  Q_FOREACH ( int i, notEditableCols )
    mFieldsList->item( row, i )->setFlags( mFieldsList->item( row, i )->flags() & ~Qt::ItemIsEditable );

  bool canRenameFields = mLayer->isEditable() && ( mLayer->dataProvider()->capabilities() & QgsVectorDataProvider::RenameAttributes ) && !mLayer->readOnly();
  if ( canRenameFields )
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() | Qt::ItemIsEditable );
  else
    mFieldsList->item( row, AttrNameCol )->setFlags( mFieldsList->item( row, AttrNameCol )->flags() & ~Qt::ItemIsEditable );

  FieldConfig cfg( mLayer, idx );
  QPushButton *pb = nullptr;
  pb = new QPushButton( QgsGui::editorWidgetRegistry()->name( cfg.mEditorWidgetType ) );
  cfg.mButton = pb;
  mFieldsList->setCellWidget( row, AttrEditTypeCol, pb );

  connect( pb, &QAbstractButton::pressed, this, &QgsFieldsProperties::attributeTypeDialog );

  setConfigForRow( row, cfg );

  //set the alias for the attribute
  mFieldsList->setItem( row, AttrAliasCol, new QTableWidgetItem( field.alias() ) );

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

void QgsFieldsProperties::loadRelations()
{
  mRelationsList->setRowCount( 0 );

  mRelations = QgsProject::instance()->relationManager()->referencedRelations( mLayer );

  int idx = 0;

  Q_FOREACH ( const QgsRelation &relation, mRelations )
  {
    mRelationsList->insertRow( idx );

    QTableWidgetItem *item = new QTableWidgetItem( relation.name() );
    item->setFlags( Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    DesignerTreeItemData itemData( DesignerTreeItemData::Relation, QStringLiteral( "%1" ).arg( relation.id() ) );
    item->setData( DesignerTreeRole, itemData.asQVariant() );
    mRelationsList->setItem( idx, RelNameCol, item );

    item = new QTableWidgetItem( relation.referencingLayer()->name() );
    item->setFlags( Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mRelationsList->setItem( idx, RelLayerCol, item );

    item = new QTableWidgetItem( relation.fieldPairs().at( 0 ).referencingField() );
    item->setFlags( Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mRelationsList->setItem( idx, RelFieldCol, item );

    item = new QTableWidgetItem( relation.id() );
    item->setFlags( Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    mRelationsList->setItem( idx, RelIdCol, item );

    QComboBox *nmCombo = new QComboBox( mRelationsList );
    nmCombo->addItem( tr( "Many to one relation" ) );
    Q_FOREACH ( const QgsRelation &nmrel, QgsProject::instance()->relationManager()->referencingRelations( relation.referencingLayer() ) )
    {
      if ( nmrel.fieldPairs().at( 0 ).referencingField() != relation.fieldPairs().at( 0 ).referencingField() )
        nmCombo->addItem( QStringLiteral( "%1 (%2)" ).arg( nmrel.referencedLayer()->name(), nmrel.fieldPairs().at( 0 ).referencedField() ), nmrel.id() );

      const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( mLayer, relation.id() );

      const QVariant nmrelcfg = setup.config().value( QStringLiteral( "nm-rel" ) );

      int idx =  nmCombo->findData( nmrelcfg.toString() );

      if ( idx != -1 )
        nmCombo->setCurrentIndex( idx );
    }

    if ( nmCombo->count() == 1 )
    {
      nmCombo->setEnabled( false );
    }

    mRelationsList->setCellWidget( idx, RelNmCol, nmCombo );
    ++idx;
  }
}

void QgsFieldsProperties::on_mAddItemButton_clicked()
{
  QList<QTableWidgetItem *> listItems = mFieldsList->selectedItems();
  QList<QTreeWidgetItem *> treeItems = mDesignerTree->selectedItems();

  if ( treeItems.count() != 1 && listItems.isEmpty() )
    return;

  QTreeWidgetItem *parent = treeItems[0];
  if ( parent->data( 0, DesignerTreeRole ).value<DesignerTreeItemData>().type() != DesignerTreeItemData::Container )
    return;

  Q_FOREACH ( QTableWidgetItem *item, listItems )
  {
    if ( item->column() == 0 ) // Information is in the first column
      mDesignerTree->addItem( parent, item->data( DesignerTreeRole ).value<DesignerTreeItemData>() );
  }
}

void QgsFieldsProperties::on_mAddTabOrGroupButton_clicked()
{
  QList<QgsAddTabOrGroup::TabPair> tabList;

  for ( QTreeWidgetItemIterator it( mDesignerTree ); *it; ++it )
  {
    DesignerTreeItemData itemData = ( *it )->data( 0, DesignerTreeRole ).value<DesignerTreeItemData>();
    if ( itemData.type() == DesignerTreeItemData::Container )
    {
      tabList.append( QgsAddTabOrGroup::TabPair( itemData.name(), *it ) );
    }
  }
  QgsAddTabOrGroup addTabOrGroup( mLayer, tabList, this );

  if ( !addTabOrGroup.exec() )
    return;

  QString name = addTabOrGroup.name();
  if ( addTabOrGroup.tabButtonIsChecked() )
  {
    mDesignerTree->addContainer( mDesignerTree->invisibleRootItem(), name, addTabOrGroup.columnCount() );
  }
  else
  {
    QTreeWidgetItem *tabItem = addTabOrGroup.tab();
    mDesignerTree->addContainer( tabItem, name, addTabOrGroup.columnCount() );
  }
}

void QgsFieldsProperties::on_mRemoveTabGroupItemButton_clicked()
{
  qDeleteAll( mDesignerTree->selectedItems() );
}

void QgsFieldsProperties::on_mMoveDownItem_clicked()
{
  QList<QTreeWidgetItem *> itemList = mDesignerTree->selectedItems();
  if ( itemList.count() != 1 )
    return;

  QTreeWidgetItem *itemToMoveDown = itemList.first();
  QTreeWidgetItem *parent = itemToMoveDown->parent();
  if ( !parent )
  {
    parent = mDesignerTree->invisibleRootItem();
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
  QList<QTreeWidgetItem *> itemList = mDesignerTree->selectedItems();
  if ( itemList.count() != 1 )
    return;

  QTreeWidgetItem *itemToMoveUp = itemList.first();
  QTreeWidgetItem *parent = itemToMoveUp->parent();
  if ( !parent )
  {
    parent = mDesignerTree->invisibleRootItem();
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

void QgsFieldsProperties::on_mInitCodeSourceComboBox_currentIndexChanged( int codeSource )
{
  // Show or hide ui elements as needed
  mInitFunctionContainer->setVisible( codeSource != QgsEditFormConfig::CodeSourceNone );
  mPythonInitCodeGroupBox->setVisible( codeSource == QgsEditFormConfig::CodeSourceDialog );
  mInitFilePathLineEdit->setVisible( codeSource == QgsEditFormConfig::CodeSourceFile );
  mInitFilePathLabel->setVisible( codeSource == QgsEditFormConfig::CodeSourceFile );
  pbtnSelectInitFilePath->setVisible( codeSource == QgsEditFormConfig::CodeSourceFile );
}

void QgsFieldsProperties::attributeTypeDialog()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  FieldConfig cfg;
  int index = -1;
  int row = -1;

  Q_FOREACH ( QTableWidgetItem *wdg, mIndexedWidgets )
  {
    cfg = wdg->data( FieldConfigRole ).value<FieldConfig>();
    if ( cfg.mButton == pb )
    {
      index = mIndexedWidgets.indexOf( wdg );
      row = wdg->row();
      break;
    }
  }

  if ( index == -1 )
    return;

  QgsAttributeTypeDialog attributeTypeDialog( mLayer, index );

  attributeTypeDialog.setFieldEditable( cfg.mEditable );
  attributeTypeDialog.setLabelOnTop( cfg.mLabelOnTop );
  attributeTypeDialog.setNotNull( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull );
  attributeTypeDialog.setNotNullEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
  attributeTypeDialog.setUnique( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique );
  attributeTypeDialog.setUniqueEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );

  QgsFieldConstraints constraints = mLayer->fields().at( index ).constraints();
  QgsFieldConstraints::Constraints providerConstraints = 0;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintNotNull;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintUnique;
  if ( constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) == QgsFieldConstraints::ConstraintOriginProvider )
    providerConstraints |= QgsFieldConstraints::ConstraintExpression;
  attributeTypeDialog.setProviderConstraints( providerConstraints );

  attributeTypeDialog.setConstraintExpression( cfg.mConstraint );
  attributeTypeDialog.setConstraintExpressionDescription( cfg.mConstraintDescription );
  attributeTypeDialog.setConstraintExpressionEnforced( cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard ) == QgsFieldConstraints::ConstraintStrengthHard );
  attributeTypeDialog.setDefaultValueExpression( mLayer->defaultValueExpression( index ) );

  attributeTypeDialog.setEditorWidgetConfig( cfg.mEditorWidgetConfig );
  attributeTypeDialog.setEditorWidgetType( cfg.mEditorWidgetType );

  if ( !attributeTypeDialog.exec() )
    return;

  cfg.mEditable = attributeTypeDialog.fieldEditable();
  cfg.mLabelOnTop = attributeTypeDialog.labelOnTop();

  cfg.mConstraints = 0;
  if ( attributeTypeDialog.notNull() && !( providerConstraints & QgsFieldConstraints::ConstraintNotNull ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintNotNull;
  }
  if ( attributeTypeDialog.unique() && !( providerConstraints & QgsFieldConstraints::ConstraintUnique ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintUnique;
  }
  if ( !attributeTypeDialog.constraintExpression().isEmpty() && !( providerConstraints & QgsFieldConstraints::ConstraintExpression ) )
  {
    cfg.mConstraints |= QgsFieldConstraints::ConstraintExpression;
  }

  cfg.mConstraintDescription = attributeTypeDialog.constraintExpressionDescription();
  cfg.mConstraint = attributeTypeDialog.constraintExpression();
  mLayer->setDefaultValueExpression( index, attributeTypeDialog.defaultValueExpression() );

  cfg.mEditorWidgetType = attributeTypeDialog.editorWidgetType();
  cfg.mEditorWidgetConfig = attributeTypeDialog.editorWidgetConfig();

  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, attributeTypeDialog.notNullEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, attributeTypeDialog.uniqueEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );
  cfg.mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, attributeTypeDialog.constraintExpressionEnforced() ?
                                  QgsFieldConstraints::ConstraintStrengthHard : QgsFieldConstraints::ConstraintStrengthSoft );

  pb->setText( attributeTypeDialog.editorWidgetText() );

  setConfigForRow( row, cfg );
}


void QgsFieldsProperties::attributeAdded( int idx )
{
  bool sorted = mFieldsList->isSortingEnabled();
  if ( sorted )
    mFieldsList->setSortingEnabled( false );

  const QgsFields &fields = mLayer->fields();
  int row = mFieldsList->rowCount();
  mFieldsList->insertRow( row );
  setRow( row, idx, fields.at( idx ) );
  mFieldsList->setCurrentCell( row, idx );

  for ( int i = idx + 1; i < mIndexedWidgets.count(); i++ )
    mIndexedWidgets.at( i )->setData( Qt::DisplayRole, i );

  if ( sorted )
    mFieldsList->setSortingEnabled( true );
}


void QgsFieldsProperties::attributeDeleted( int idx )
{
  mFieldsList->removeRow( mIndexedWidgets.at( idx )->row() );
  mIndexedWidgets.removeAt( idx );
  for ( int i = idx; i < mIndexedWidgets.count(); i++ )
  {
    mIndexedWidgets.at( i )->setData( Qt::DisplayRole, i );
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
    QMessageBox::critical( this, tr( "Failed to add field" ), tr( "Failed to add field '%1' of type '%2'. Is the field name unique?" ).arg( field.name(), field.typeName() ) );
    return false;
  }
}

void QgsFieldsProperties::editingToggled()
{
  updateButtons();
  updateFieldRenamingStatus();
}

QgsFieldsProperties::FieldConfig QgsFieldsProperties::configForRow( int row )
{
  Q_FOREACH ( QTableWidgetItem *wdg, mIndexedWidgets )
  {
    if ( wdg->row() == row )
    {
      return wdg->data( FieldConfigRole ).value<FieldConfig>();
    }
  }

  // Should never get here
  Q_ASSERT( false );
  return FieldConfig();
}

void QgsFieldsProperties::setConfigForRow( int row, const QgsFieldsProperties::FieldConfig &cfg )
{
  Q_FOREACH ( QTableWidgetItem *wdg, mIndexedWidgets )
  {
    if ( wdg->row() == row )
    {
      wdg->setData( FieldConfigRole, QVariant::fromValue<FieldConfig>( cfg ) );
      return;
    }
  }

  // Should never get here
  Q_ASSERT( false );
}

void QgsFieldsProperties::on_mAddAttributeButton_clicked()
{
  QgsAddAttrDialog dialog( mLayer, this );
  if ( dialog.exec() == QDialog::Accepted )
  {
    addAttribute( dialog.field() );

  }
}

void QgsFieldsProperties::on_mDeleteAttributeButton_clicked()
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

void QgsFieldsProperties::updateButtons()
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


void QgsFieldsProperties::attributesListCellChanged( int row, int column )
{
  if ( column == AttrAliasCol && mLayer )
  {
    int idx = mFieldsList->item( row, AttrIdCol )->text().toInt();

    const QgsFields &fields = mLayer->fields();

    if ( idx >= fields.count() )
    {
      return; // index must be wrong
    }

    QTableWidgetItem *aliasItem = mFieldsList->item( row, column );
    if ( aliasItem )
    {
      if ( !aliasItem->text().trimmed().isEmpty() )
      {
        mLayer->setFieldAlias( idx, aliasItem->text() );
      }
      else
      {
        mLayer->removeFieldAlias( idx );
      }
    }
  }
  else if ( column == AttrNameCol && mLayer && mLayer->isEditable() )
  {
    QTableWidgetItem *nameItem = mFieldsList->item( row, column );
    if ( !nameItem ||
         nameItem->text().isEmpty() ||
         !mLayer->fields().exists( row ) ||
         mLayer->fields().at( row ).name() == nameItem->text() )
      return;

    mLayer->beginEditCommand( tr( "Rename attribute" ) );
    if ( mLayer->renameAttribute( row,  nameItem->text() ) )
    {
      mLayer->endEditCommand();
    }
    else
    {
      mLayer->destroyEditCommand();
      QMessageBox::critical( this, tr( "Failed to rename field" ), tr( "Failed to rename field to '%1'. Is the field name unique?" ).arg( nameItem->text() ) );
    }
  }
}

void QgsFieldsProperties::updateExpression()
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

void QgsFieldsProperties::on_mCalculateFieldButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsFieldCalculator calc( mLayer, this );
  calc.exec();
}


//
// methods reimplemented from qt designer base class
//

void QgsFieldsProperties::updateFieldRenamingStatus()
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

QgsAttributeEditorElement *QgsFieldsProperties::createAttributeEditorWidget( QTreeWidgetItem *item, QgsAttributeEditorElement *parent, bool forceGroup )
{
  QgsAttributeEditorElement *widgetDef = nullptr;

  DesignerTreeItemData itemData = item->data( 0, DesignerTreeRole ).value<DesignerTreeItemData>();
  switch ( itemData.type() )
  {
    case DesignerTreeItemData::Field:
    {
      int idx = mLayer->fields().lookupField( itemData.name() );
      widgetDef = new QgsAttributeEditorField( itemData.name(), idx, parent );
      break;
    }

    case DesignerTreeItemData::Relation:
    {
      QgsRelation relation = QgsProject::instance()->relationManager()->relation( itemData.name() );
      QgsAttributeEditorRelation *relDef = new QgsAttributeEditorRelation( itemData.name(), relation, parent );
      relDef->setShowLinkButton( itemData.relationEditorConfiguration().showLinkButton );
      relDef->setShowUnlinkButton( itemData.relationEditorConfiguration().showUnlinkButton );
      widgetDef = relDef;
      break;
    }

    case DesignerTreeItemData::Container:
    {
      QgsAttributeEditorContainer *container = new QgsAttributeEditorContainer( item->text( 0 ), parent );
      container->setColumnCount( itemData.columnCount() );
      container->setIsGroupBox( forceGroup ? true : itemData.showAsGroupBox() );
      container->setVisibilityExpression( itemData.visibilityExpression() );

      for ( int t = 0; t < item->childCount(); t++ )
      {
        container->addChildElement( createAttributeEditorWidget( item->child( t ), container ) );
      }

      widgetDef = container;
      break;
    }
  }

  widgetDef->setShowLabel( itemData.showLabel() );

  return widgetDef;
}


void QgsFieldsProperties::on_pbtnSelectInitFilePath_clicked()
{
  QgsSettings myQSettings;
  QString lastUsedDir = myQSettings.value( QStringLiteral( "style/lastInitFilePathDir" ), "." ).toString();
  QString pyfilename = QFileDialog::getOpenFileName( this, tr( "Select Python file" ), lastUsedDir, tr( "Python file" )  + " (*.py)" );

  if ( pyfilename.isNull() )
    return;

  QFileInfo fi( pyfilename );
  myQSettings.setValue( QStringLiteral( "style/lastInitFilePathDir" ), fi.path() );
  mInitFilePathLineEdit->setText( pyfilename );
}


void QgsFieldsProperties::on_pbnSelectEditForm_clicked()
{
  QgsSettings myQSettings;
  QString lastUsedDir = myQSettings.value( QStringLiteral( "style/lastUIDir" ), QDir::homePath() ).toString();
  QString uifilename = QFileDialog::getOpenFileName( this, tr( "Select edit form" ), lastUsedDir, tr( "UI file" )  + " (*.ui)" );

  if ( uifilename.isNull() )
    return;

  QFileInfo fi( uifilename );
  myQSettings.setValue( QStringLiteral( "style/lastUIDir" ), fi.path() );
  mEditFormLineEdit->setText( uifilename );
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

  QgsEditFormConfig editFormConfig = mLayer->editFormConfig();

  for ( int i = 0; i < mFieldsList->rowCount(); i++ )
  {
    int idx = mFieldsList->item( i, AttrIdCol )->text().toInt();
    QString name = mLayer->fields().at( idx ).name();
    FieldConfig cfg = configForRow( i );

    editFormConfig.setReadOnly( idx, !cfg.mEditable );
    editFormConfig.setLabelOnTop( idx, cfg.mLabelOnTop );
    mLayer->setConstraintExpression( idx, cfg.mConstraint, cfg.mConstraintDescription );
    mLayer->setEditorWidgetSetup( idx, QgsEditorWidgetSetup( cfg.mEditorWidgetType, cfg.mEditorWidgetConfig ) );

    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintNotNull )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintNotNull, QgsFieldConstraints::ConstraintStrengthHard ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintNotNull );
    }
    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintUnique )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintUnique, QgsFieldConstraints::ConstraintStrengthHard ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintUnique );
    }
    if ( cfg.mConstraints & QgsFieldConstraints::ConstraintExpression )
    {
      mLayer->setFieldConstraint( idx, QgsFieldConstraints::ConstraintExpression, cfg.mConstraintStrength.value( QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard ) );
    }
    else
    {
      mLayer->removeFieldConstraint( idx, QgsFieldConstraints::ConstraintExpression );
    }

    if ( mFieldsList->item( i, AttrWMSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWMS.insert( mFieldsList->item( i, AttrNameCol )->text() );
    }
    if ( mFieldsList->item( i, AttrWFSCol )->checkState() == Qt::Unchecked )
    {
      excludeAttributesWFS.insert( mFieldsList->item( i, AttrNameCol )->text() );
    }
  }

  // tabs and groups
  editFormConfig.clearTabs();
  for ( int t = 0; t < mDesignerTree->invisibleRootItem()->childCount(); t++ )
  {
    QTreeWidgetItem *tabItem = mDesignerTree->invisibleRootItem()->child( t );

    editFormConfig.addTab( createAttributeEditorWidget( tabItem, nullptr, false ) );
  }

  editFormConfig.setUiForm( mEditFormLineEdit->text() );
  editFormConfig.setLayout( ( QgsEditFormConfig::EditorLayout ) mEditorLayoutComboBox->currentIndex() );

  // Init function configuration
  editFormConfig.setInitFunction( mInitFunctionLineEdit->text() );
  editFormConfig.setInitCode( mInitCodeEditorPython->text() );
  editFormConfig.setInitFilePath( mInitFilePathLineEdit->text() );
  editFormConfig.setInitCodeSource( ( QgsEditFormConfig::PythonInitCodeSource )mInitCodeSourceComboBox->currentIndex() );

  editFormConfig.setSuppress( ( QgsEditFormConfig::FeatureFormSuppress )mFormSuppressCmbBx->currentIndex() );

  mLayer->setExcludeAttributesWms( excludeAttributesWMS );
  mLayer->setExcludeAttributesWfs( excludeAttributesWFS );

  // relations
  for ( int i = 0; i < mRelationsList->rowCount(); ++i )
  {
    QVariantMap cfg;

    QComboBox *cb = qobject_cast<QComboBox *>( mRelationsList->cellWidget( i, RelNmCol ) );
    QVariant otherRelation = cb->currentData();

    if ( otherRelation.isValid() )
    {
      cfg[QStringLiteral( "nm-rel" )] = otherRelation.toString();
    }

    DesignerTreeItemData itemData = mRelationsList->item( i, RelNameCol )->data( DesignerTreeRole ).value<DesignerTreeItemData>();

    QString relationName = itemData.name();

    editFormConfig.setWidgetConfig( relationName, cfg );
  }

  mLayer->setEditFormConfig( editFormConfig );
}
/*
 * FieldConfig implementation
 */

QgsFieldsProperties::FieldConfig::FieldConfig()
  : mEditable( true )
  , mEditableEnabled( true )
  , mLabelOnTop( false )
  , mConstraints( 0 )
  , mConstraintDescription( QString() )
  , mButton( nullptr )
{
}

QgsFieldsProperties::FieldConfig::FieldConfig( QgsVectorLayer *layer, int idx )
  : mButton( nullptr )
{
  mEditable = !layer->editFormConfig().readOnly( idx );
  mEditableEnabled = layer->fields().fieldOrigin( idx ) != QgsFields::OriginJoin
                     && layer->fields().fieldOrigin( idx ) != QgsFields::OriginExpression;
  mLabelOnTop = layer->editFormConfig().labelOnTop( idx );
  QgsFieldConstraints constraints = layer->fields().at( idx ).constraints();
  mConstraints = constraints.constraints();
  mConstraint = constraints.constraintExpression();
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintNotNull, constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintUnique, constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) );
  mConstraintStrength.insert( QgsFieldConstraints::ConstraintExpression, constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) );
  mConstraintDescription = constraints.constraintDescription();
  const QgsEditorWidgetSetup setup = QgsGui::editorWidgetRegistry()->findBest( layer, layer->fields().field( idx ).name() );
  mEditorWidgetType = setup.type();
  mEditorWidgetConfig = setup.config();
}

/*
 * DragList implementation
 */

QStringList DragList::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributetabledesignerelement" );
}

QMimeData *DragList::mimeData( const QList<QTableWidgetItem *> items ) const
{
  if ( items.count() <= 0 )
    return nullptr;

  QStringList types = mimeTypes();

  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  Q_FOREACH ( const QTableWidgetItem *item, items )
  {
    // Relevant information is always in the UserRole of the first column
    if ( item && item->column() == 0 )
    {
      QgsFieldsProperties::DesignerTreeItemData itemData = item->data( QgsFieldsProperties::DesignerTreeRole ).value<QgsFieldsProperties::DesignerTreeItemData>();
      stream << itemData;
    }
  }

  data->setData( format, encoded );

  return data;
}

/*
 * DesignerTree implementation
 */

QTreeWidgetItem *DesignerTree::addContainer( QTreeWidgetItem *parent, const QString &title, int columnCount )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << title );
  newItem->setBackground( 0, QBrush( Qt::lightGray ) );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
  QgsFieldsProperties::DesignerTreeItemData itemData( QgsFieldsProperties::DesignerTreeItemData::Container, title );
  itemData.setColumnCount( columnCount );
  newItem->setData( 0, QgsFieldsProperties::DesignerTreeRole, itemData.asQVariant() );
  parent->addChild( newItem );
  newItem->setExpanded( true );
  return newItem;
}

DesignerTree::DesignerTree( QgsVectorLayer *layer, QWidget *parent )
  : QTreeWidget( parent )
  , mLayer( layer )
{
  connect( this, &QTreeWidget::itemDoubleClicked, this, &DesignerTree::onItemDoubleClicked );
}

QTreeWidgetItem *DesignerTree::addItem( QTreeWidgetItem *parent, QgsFieldsProperties::DesignerTreeItemData data )
{
  QTreeWidgetItem *newItem = new QTreeWidgetItem( QStringList() << data.name() );
  newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled );
  if ( data.type() == QgsFieldsProperties::DesignerTreeItemData::Container )
  {
    newItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
    newItem->setBackground( 0, QBrush( Qt::lightGray ) );

#if 0
    switch ( data.type() )
    {
      case DesignerTreeItemData::Field:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mFieldIcon.svg" ) );
        break;

      case DesignerTreeItemData::Relation:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mRelationIcon.svg" ) );
        break;

      case DesignerTreeItemData::Container:
        newItem->setIcon( 0, QgsApplication::getThemeIcon( "/mContainerIcon.svg" ) );
        break;
    }
#endif
  }
  newItem->setData( 0, QgsFieldsProperties::DesignerTreeRole, data.asQVariant() );
  parent->addChild( newItem );

  return newItem;
}

/**
 * Is called when mouse is moved over attributes tree before a
 * drop event. Used to inhibit dropping fields onto the root item.
 */

void DesignerTree::dragMoveEvent( QDragMoveEvent *event )
{
  const QMimeData *data = event->mimeData();

  if ( data->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
  {
    QgsFieldsProperties::DesignerTreeItemData itemElement;

    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    stream >> itemElement;

    // Inner drag and drop actions are always MoveAction
    if ( event->source() == this )
    {
      event->setDropAction( Qt::MoveAction );
    }
  }
  else
  {
    event->ignore();
  }

  QTreeWidget::dragMoveEvent( event );
}


bool DesignerTree::dropMimeData( QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action )
{
  Q_UNUSED( index )
  bool bDropSuccessful = false;

  if ( action == Qt::IgnoreAction )
  {
    bDropSuccessful = true;
  }
  else if ( data->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
  {
    QByteArray itemData = data->data( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) );
    QDataStream stream( &itemData, QIODevice::ReadOnly );
    QgsFieldsProperties::DesignerTreeItemData itemElement;

    while ( !stream.atEnd() )
    {
      stream >> itemElement;

      if ( parent )
      {
        addItem( parent, itemElement );
        bDropSuccessful = true;
      }
      else
      {
        addItem( invisibleRootItem(), itemElement );
        bDropSuccessful = true;
      }
    }
  }

  return bDropSuccessful;
}

void DesignerTree::dropEvent( QDropEvent *event )
{
  if ( !event->mimeData()->hasFormat( QStringLiteral( "application/x-qgsattributetabledesignerelement" ) ) )
    return;

  if ( event->source() == this )
  {
    event->setDropAction( Qt::MoveAction );
  }

  QTreeWidget::dropEvent( event );
}

QStringList DesignerTree::mimeTypes() const
{
  return QStringList() << QStringLiteral( "application/x-qgsattributetabledesignerelement" );
}

QMimeData *DesignerTree::mimeData( const QList<QTreeWidgetItem *> items ) const
{
  if ( items.count() <= 0 )
    return nullptr;

  QStringList types = mimeTypes();

  if ( types.isEmpty() )
    return nullptr;

  QMimeData *data = new QMimeData();
  QString format = types.at( 0 );
  QByteArray encoded;
  QDataStream stream( &encoded, QIODevice::WriteOnly );

  Q_FOREACH ( const QTreeWidgetItem *item, items )
  {
    if ( item )
    {
      // Relevant information is always in the DesignerTreeRole of the first column
      QgsFieldsProperties::DesignerTreeItemData itemData = item->data( 0, QgsFieldsProperties::DesignerTreeRole ).value<QgsFieldsProperties::DesignerTreeItemData>();
      stream << itemData;
    }
  }

  data->setData( format, encoded );

  return data;
}

void DesignerTree::onItemDoubleClicked( QTreeWidgetItem *item, int column )
{
  Q_UNUSED( column )
  QgsFieldsProperties::DesignerTreeItemData itemData = item->data( 0, QgsFieldsProperties::DesignerTreeRole ).value<QgsFieldsProperties::DesignerTreeItemData>();

  QGroupBox *baseData = new QGroupBox( tr( "Base configuration" ) );

  QFormLayout *baseLayout = new QFormLayout();
  baseData->setLayout( baseLayout );
  QCheckBox *showLabelCheckbox = new QCheckBox( QStringLiteral( "Show label" ) );
  showLabelCheckbox->setChecked( itemData.showLabel() );
  baseLayout->addRow( showLabelCheckbox );
  QWidget *baseWidget = new QWidget();
  baseWidget->setLayout( baseLayout );

  if ( itemData.type() == QgsFieldsProperties::DesignerTreeItemData::Container )
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Configure Container" ) );
    QFormLayout *layout = new QFormLayout() ;
    dlg.setLayout( layout );
    layout->addRow( baseWidget );

    QCheckBox *showAsGroupBox = nullptr;
    QLineEdit *title = new QLineEdit( itemData.name() );
    QSpinBox *columnCount = new QSpinBox();
    QGroupBox *visibilityExpressionGroupBox = new QGroupBox( tr( "Control visibility by expression" ) );
    visibilityExpressionGroupBox->setCheckable( true );
    visibilityExpressionGroupBox->setChecked( itemData.visibilityExpression().enabled() );
    visibilityExpressionGroupBox->setLayout( new QGridLayout );
    QgsFieldExpressionWidget *visibilityExpressionWidget = new QgsFieldExpressionWidget;
    visibilityExpressionWidget->setLayer( mLayer );
    visibilityExpressionWidget->setExpressionDialogTitle( tr( "Visibility Expression" ) );
    visibilityExpressionWidget->setExpression( itemData.visibilityExpression()->expression() );
    visibilityExpressionGroupBox->layout()->addWidget( visibilityExpressionWidget );

    columnCount->setRange( 1, 5 );
    columnCount->setValue( itemData.columnCount() );

    layout->addRow( tr( "Title" ), title );
    layout->addRow( tr( "Column count" ), columnCount );
    layout->addRow( visibilityExpressionGroupBox );

    if ( !item->parent() )
    {
      showAsGroupBox = new QCheckBox( tr( "Show as group box" ) );
      showAsGroupBox->setChecked( itemData.showAsGroupBox() );
      layout->addRow( showAsGroupBox );
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok
        | QDialogButtonBox::Cancel );

    connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

    layout->addWidget( buttonBox );

    if ( dlg.exec() )
    {
      itemData.setColumnCount( columnCount->value() );
      itemData.setShowAsGroupBox( showAsGroupBox ? showAsGroupBox->isChecked() : true );
      itemData.setName( title->text() );
      itemData.setShowLabel( showLabelCheckbox->isChecked() );

      QgsOptionalExpression visibilityExpression;
      visibilityExpression.setData( QgsExpression( visibilityExpressionWidget->expression() ) );
      visibilityExpression.setEnabled( visibilityExpressionGroupBox->isChecked() );
      itemData.setVisibilityExpression( visibilityExpression );

      item->setData( 0, QgsFieldsProperties::DesignerTreeRole, itemData.asQVariant() );
      item->setText( 0, title->text() );
    }
  }
  else if ( itemData.type() == QgsFieldsProperties::DesignerTreeItemData::Relation )
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Configure Relation Editor" ) );
    QFormLayout *layout = new QFormLayout() ;
    dlg.setLayout( layout );
    layout->addWidget( baseWidget );

    QCheckBox *showLinkButton = new QCheckBox( tr( "Show link button" ) );
    showLinkButton->setChecked( itemData.relationEditorConfiguration().showLinkButton );
    QCheckBox *showUnlinkButton = new QCheckBox( tr( "Show unlink button" ) );
    showUnlinkButton->setChecked( itemData.relationEditorConfiguration().showUnlinkButton );
    layout->addRow( showLinkButton );
    layout->addRow( showUnlinkButton );

    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

    dlg.layout()->addWidget( buttonBox );

    if ( dlg.exec() )
    {
      QgsFieldsProperties::RelationEditorConfiguration relEdCfg;
      relEdCfg.showLinkButton = showLinkButton->isChecked();
      relEdCfg.showUnlinkButton = showUnlinkButton->isChecked();
      itemData.setShowLabel( showLabelCheckbox->isChecked() );
      itemData.setRelationEditorConfiguration( relEdCfg );

      item->setData( 0, QgsFieldsProperties::DesignerTreeRole, itemData.asQVariant() );
    }
  }
  else
  {
    QDialog dlg;
    dlg.setWindowTitle( tr( "Configure Field" ) );
    dlg.setLayout( new QGridLayout() );
    dlg.layout()->addWidget( baseWidget );

    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok
        | QDialogButtonBox::Cancel );

    connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );

    dlg.layout()->addWidget( buttonBox );

    if ( dlg.exec() )
    {
      itemData.setShowLabel( showLabelCheckbox->isChecked() );

      item->setData( 0, QgsFieldsProperties::DesignerTreeRole, itemData.asQVariant() );
    }
  }
}

/*
 * Serialization helpers for DesigerTreeItemData so we can stuff this easily into QMimeData
 */

QDataStream &operator<<( QDataStream &stream, const QgsFieldsProperties::DesignerTreeItemData &data )
{
  stream << ( quint32 )data.type() << data.name();
  return stream;
}

QDataStream &operator>>( QDataStream &stream, QgsFieldsProperties::DesignerTreeItemData &data )
{
  QString name;
  quint32 type;

  stream >> type >> name;

  data.setType( ( QgsFieldsProperties::DesignerTreeItemData::Type )type );
  data.setName( name );

  return stream;
}

bool QgsFieldsProperties::DesignerTreeItemData::showAsGroupBox() const
{
  return mShowAsGroupBox;
}

void QgsFieldsProperties::DesignerTreeItemData::setShowAsGroupBox( bool showAsGroupBox )
{
  mShowAsGroupBox = showAsGroupBox;
}

bool QgsFieldsProperties::DesignerTreeItemData::showLabel() const
{
  return mShowLabel;
}

void QgsFieldsProperties::DesignerTreeItemData::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
}

QgsOptionalExpression QgsFieldsProperties::DesignerTreeItemData::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsFieldsProperties::DesignerTreeItemData::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  mVisibilityExpression = visibilityExpression;
}

QgsFieldsProperties::RelationEditorConfiguration QgsFieldsProperties::DesignerTreeItemData::relationEditorConfiguration() const
{
  return mRelationEditorConfiguration;
}

void QgsFieldsProperties::DesignerTreeItemData::setRelationEditorConfiguration( QgsFieldsProperties::RelationEditorConfiguration relationEditorConfiguration )
{
  mRelationEditorConfiguration = relationEditorConfiguration;
}
