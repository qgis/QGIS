/***************************************************************************
                         testqgsattributesformmodel.cpp
                         --------------------------
    begin                : April 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributesformmodel.h"
#include "qgseditformconfig.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

#include <QMimeData>


class TestQgsAttributesFormModel : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testAttributesFormItem();
    void testAvailableWidgetsModel();
    void testAvailableWidgetsModelIndexOderInDragAndDrop();
    void testFormLayoutModel();
    void testFormLayoutModelOrphanFields();
    void testFormLayoutModelDragAndDrop();
    void testInvalidRelationInAvailableWidgets();
    void testInvalidRelationInFormLayout();

  private:
    void setUpProjectWithRelation();
    void setUpProjectWithInvalidRelations();

    QgsVectorLayer *mLayer = nullptr;
    QgsProject *mProject = QgsProject::instance();
};


void TestQgsAttributesFormModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsAttributesFormModel::setUpProjectWithRelation()
{
  const QString projectPath = QStringLiteral( TEST_DATA_DIR ) + "/relations.qgs";
  QVERIFY( mProject->read( projectPath ) );

  const QString layerId = "points_97805748_6b30_49b8_a80b_bdbb4e8e78a3"_L1;
  mLayer = qobject_cast< QgsVectorLayer * >( mProject->mapLayer( layerId ) );
}

void TestQgsAttributesFormModel::setUpProjectWithInvalidRelations()
{
  const QString projectPath = QStringLiteral( TEST_DATA_DIR ) + "/broken_relations2.qgz";
  QVERIFY( mProject->read( projectPath ) );

  const QString layerId = "household_0c432204_12d4_47a6_8d90_d759b02560dd"_L1;
  mLayer = qobject_cast< QgsVectorLayer * >( mProject->mapLayer( layerId ) );
}

void TestQgsAttributesFormModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributesFormModel::cleanup()
{
  mProject->clear();
}

void TestQgsAttributesFormModel::testAttributesFormItem()
{
  // Default constructor (used for the root item)
  auto rootItem = std::make_unique< QgsAttributesFormItem >();
  QVERIFY( rootItem->data( QgsAttributesFormModel::ItemIdRole ).toString().isEmpty() );
  QVERIFY( rootItem->data( QgsAttributesFormModel::ItemNameRole ).toString().isEmpty() );
  QVERIFY( rootItem->data( QgsAttributesFormModel::ItemDisplayRole ).toString().isEmpty() );
  QVERIFY( !rootItem->parent() );
  QCOMPARE( rootItem->childCount(), 0 );
  QVERIFY( !rootItem->child( 0 ) );

  // Second constructor
  const QString item1Name = u"child item1 name"_s;
  const QString item1DisplayName = u"child item1 display name"_s;
  auto item = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::Field, item1Name, item1DisplayName );
  QVERIFY( !item->parent() );
  QCOMPARE( item->data( QgsAttributesFormModel::ItemNameRole ).toString(), item1Name );
  QCOMPARE( item->data( QgsAttributesFormModel::ItemDisplayRole ).toString(), item1DisplayName );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( item->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );
  QCOMPARE( item->name(), item1Name );
  QCOMPARE( item->displayName(), item1DisplayName );
  QCOMPARE( item->type(), QgsAttributesFormData::Field );
  QCOMPARE( item->id(), QString() );

  const QString item1Id = u"itemId"_s;
  item->setData( QgsAttributesFormModel::ItemIdRole, item1Id );
  QCOMPARE( item->id(), item1Id );

  rootItem->addChild( std::move( item ) );
  QCOMPARE( rootItem->childCount(), 1 );
  QVERIFY( rootItem->child( 0 ) );

  QgsAttributesFormItem *itemPointer = rootItem->child( 0 );

  QVERIFY( itemPointer->parent() );
  QCOMPARE( itemPointer->parent(), rootItem.get() );
  QCOMPARE( itemPointer->row(), 0 );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemNameRole ).toString(), item1Name );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemDisplayRole ).toString(), item1DisplayName );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( itemPointer->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );

  // Third constructor
  const QString item2Name = u"child item2 name"_s;
  const QString item2DisplayName = u"child item2 display name"_s;
  QgsAttributesFormData::AttributeFormItemData itemData;
  itemData.setShowLabel( false );

  auto item2 = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::Field, itemData, item2Name, item2DisplayName, rootItem.get() );
  QVERIFY( item2->parent() );
  QCOMPARE( item2->data( QgsAttributesFormModel::ItemNameRole ).toString(), item2Name );
  QCOMPARE( item2->data( QgsAttributesFormModel::ItemDisplayRole ).toString(), item2DisplayName );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( item2->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );
  QCOMPARE( item2->data( QgsAttributesFormModel::ItemDataRole ).value< QgsAttributesFormData::AttributeFormItemData >().showLabel(), false );

  const QString item2Id = u"item2Id"_s;
  item2->setData( QgsAttributesFormModel::ItemIdRole, item2Id );

  rootItem->insertChild( 1, std::move( item2 ) );
  QCOMPARE( rootItem->childCount(), 2 );
  QVERIFY( rootItem->child( 1 ) );

  itemPointer = rootItem->child( 1 );
  QVERIFY( itemPointer->parent() );
  QCOMPARE( itemPointer->parent(), rootItem.get() );
  QCOMPARE( itemPointer->row(), 1 );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemNameRole ).toString(), item2Name );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemDisplayRole ).toString(), item2DisplayName );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( itemPointer->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );

  // Add container and grandchild
  const QString containerName = u"Tab"_s;
  auto containerItem = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::Container, containerName );
  QCOMPARE( containerItem->childCount(), 0 );
  containerItem->setData( QgsAttributesFormModel::ItemIdRole, containerName );

  const QString relationName = u"Relation item"_s;
  auto relationItem = std::make_unique< QgsAttributesFormItem >( QgsAttributesFormData::Relation, relationName );

  const QString relationId = u"relationId"_s;
  relationItem->setData( QgsAttributesFormModel::ItemIdRole, relationId );

  containerItem->addChild( std::move( relationItem ) );
  QCOMPARE( containerItem->childCount(), 1 );
  QVERIFY( containerItem->child( 0 ) );
  QCOMPARE( containerItem->child( 0 )->name(), relationName );
  QCOMPARE( containerItem->child( 0 )->parent(), containerItem.get() );

  rootItem->insertChild( 1, std::move( containerItem ) );
  QCOMPARE( rootItem->childCount(), 3 );
  QVERIFY( rootItem->child( 1 ) );
  itemPointer = rootItem->child( 1 );
  QVERIFY( itemPointer->parent() );
  QCOMPARE( itemPointer->parent(), rootItem.get() );
  QCOMPARE( itemPointer->row(), 1 );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( itemPointer->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Container );

  // Search items
  itemPointer = rootItem->firstTopChild( QgsAttributesFormData::Field, u"Inexistent"_s );
  QVERIFY( !itemPointer );

  itemPointer = rootItem->firstTopChild( QgsAttributesFormData::Relation, relationId );
  QVERIFY( !itemPointer );

  itemPointer = rootItem->firstTopChild( QgsAttributesFormData::Container, containerName );
  QVERIFY( itemPointer );
  QCOMPARE( itemPointer->name(), containerName );
  QCOMPARE( itemPointer->id(), containerName );

  itemPointer = rootItem->firstTopChild( QgsAttributesFormData::Field, item2Id );
  QVERIFY( itemPointer );
  QCOMPARE( itemPointer->name(), item2Name );

  itemPointer = rootItem->firstChildRecursive( QgsAttributesFormData::Field, u"Inexistent"_s );
  QVERIFY( !itemPointer );

  itemPointer = rootItem->firstChildRecursive( QgsAttributesFormData::Relation, relationId );
  QVERIFY( itemPointer );
  QCOMPARE( itemPointer->name(), relationName );
  QCOMPARE( itemPointer->id(), relationId );

  itemPointer = rootItem->firstChildRecursive( QgsAttributesFormData::Field, item2Id );
  QVERIFY( itemPointer );
  QCOMPARE( itemPointer->name(), item2Name );
  QCOMPARE( itemPointer->id(), item2Id );

  // Delete items
  rootItem->deleteChildAtIndex( 1 );
  QCOMPARE( rootItem->childCount(), 2 );
  itemPointer = rootItem->child( 1 );
  QVERIFY( itemPointer->parent() );
  QCOMPARE( itemPointer->parent(), rootItem.get() );
  QCOMPARE( itemPointer->row(), 1 );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemNameRole ).toString(), item2Name );
  QCOMPARE( itemPointer->data( QgsAttributesFormModel::ItemDisplayRole ).toString(), item2DisplayName );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( itemPointer->data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );

  rootItem->deleteChildren();
  QCOMPARE( rootItem->childCount(), 0 );
}

void TestQgsAttributesFormModel::testAvailableWidgetsModel()
{
  setUpProjectWithRelation();

  QgsAttributesAvailableWidgetsModel availableWidgetsModel( mLayer, mProject );

#ifdef ENABLE_MODELTEST
  new ModelTest( &availableWidgetsModel, this ); // for model validity checking
#endif

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 0 );
  QVERIFY( !availableWidgetsModel.hasChildren() );
  QCOMPARE( availableWidgetsModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Available Widgets" ) );
  QVERIFY( availableWidgetsModel.mimeTypes().size() == 1 );
  QCOMPARE( availableWidgetsModel.mimeTypes(), QStringList() << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  // Add data to the model
  availableWidgetsModel.populate();

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 4 );
  QVERIFY( availableWidgetsModel.hasChildren() );

  // Check top-level items
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Fields" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Relations" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Actions" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 2, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 3, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Other Widgets" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 3, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );

  // Check fields
  const QModelIndex fieldsIndex = availableWidgetsModel.index( 0, 0, QModelIndex() );
  QVERIFY( fieldsIndex.isValid() );
  QCOMPARE( availableWidgetsModel.rowCount( fieldsIndex ), 6 );
  QCOMPARE( fieldsIndex.row(), 0 );

  QCOMPARE( fieldsIndex, availableWidgetsModel.fieldContainer() );

  const QModelIndex fieldIndex = availableWidgetsModel.index( 0, 0, fieldsIndex );
  QVERIFY( fieldIndex.isValid() );
  QCOMPARE( fieldIndex.parent(), fieldsIndex );
  QCOMPARE( fieldIndex.row(), 0 );

  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( fieldIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );

  const QString fieldName = fieldIndex.data( QgsAttributesFormModel::ItemNameRole ).toString();

  QModelIndex foundIndex = availableWidgetsModel.fieldModelIndex( QString() );
  QVERIFY( !foundIndex.isValid() );

  foundIndex = availableWidgetsModel.fieldModelIndex( fieldName );
  QCOMPARE( foundIndex, fieldIndex );

  const QString fieldId = fieldIndex.data( QgsAttributesFormModel::ItemIdRole ).toString();
  QCOMPARE( fieldId, fieldName );

  const auto fieldData = fieldIndex.data( QgsAttributesFormModel::ItemDataRole ).value< QgsAttributesFormData::AttributeFormItemData >();
  QVERIFY( fieldData.showLabel() );

  const int fieldIdx = mLayer->fields().indexOf( fieldName );
  QCOMPARE( fieldIdx, 0 );
  QgsAttributesFormData::FieldConfig config( mLayer, fieldIdx );

  const auto fieldConfig = fieldIndex.data( QgsAttributesFormModel::ItemFieldConfigRole ).value< QgsAttributesFormData::FieldConfig >();
  QCOMPARE( fieldConfig.mFieldConstraints, config.mFieldConstraints );
  QCOMPARE( fieldConfig.mEditable, config.mEditable );
  QCOMPARE( fieldConfig.mEditorWidgetType, config.mEditorWidgetType );
  QCOMPARE( fieldConfig.mEditorWidgetConfig, config.mEditorWidgetConfig );
  QCOMPARE( fieldConfig.mAlias, config.mAlias );
  QCOMPARE( fieldConfig.mComment, config.mComment );

  // Check relations
  const QModelIndex relationsIndex = availableWidgetsModel.index( 1, 0, QModelIndex() );
  QVERIFY( relationsIndex.isValid() );
  QCOMPARE( availableWidgetsModel.rowCount( relationsIndex ), 1 );
  QCOMPARE( relationsIndex.row(), 1 );

  QCOMPARE( relationsIndex, availableWidgetsModel.relationContainer() );

  const QModelIndex relationIndex = availableWidgetsModel.index( 0, 0, relationsIndex );
  QVERIFY( relationIndex.isValid() );
  QCOMPARE( relationIndex.parent(), relationsIndex );
  QCOMPARE( relationIndex.row(), 0 );

  // Check actions
  const QModelIndex actionsIndex = availableWidgetsModel.index( 2, 0, QModelIndex() );
  QVERIFY( actionsIndex.isValid() );
  QCOMPARE( availableWidgetsModel.rowCount( actionsIndex ), 0 );
  QCOMPARE( actionsIndex.row(), 2 );

  // Check other widgets
  const QModelIndex otherWidgetsIndex = availableWidgetsModel.index( 3, 0, QModelIndex() );
  QVERIFY( otherWidgetsIndex.isValid() );
  QCOMPARE( availableWidgetsModel.rowCount( otherWidgetsIndex ), 4 );
  QCOMPARE( otherWidgetsIndex.row(), 3 );
}

void TestQgsAttributesFormModel::testAvailableWidgetsModelIndexOderInDragAndDrop()
{
  // We test the QMimeData object to check that indexes are returned
  // in the expected order for GUI actions like Drag and Drop

  setUpProjectWithRelation();

  QgsAttributesAvailableWidgetsModel availableWidgetsModel( mLayer, mProject );

#ifdef ENABLE_MODELTEST
  new ModelTest( &availableWidgetsModel, this ); // for model validity checking
#endif

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 0 );
  QVERIFY( !availableWidgetsModel.hasChildren() );
  QCOMPARE( availableWidgetsModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Available Widgets" ) );
  QVERIFY( availableWidgetsModel.mimeTypes().size() == 1 );
  QCOMPARE( availableWidgetsModel.mimeTypes(), QStringList() << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  // Add data to the model
  availableWidgetsModel.populate();

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 4 );
  QVERIFY( availableWidgetsModel.hasChildren() );

  // Select all fields and see they are stored in a proper order in the mimeData
  QModelIndexList indexes;
  QStringList expectedItemIds;
  QModelIndex fields = availableWidgetsModel.fieldContainer();

  for ( int i = 0; i < availableWidgetsModel.rowCount( fields ); i++ )
  {
    QModelIndex index = availableWidgetsModel.index( i, 0, fields );
    indexes << index;

    expectedItemIds << index.data( QgsAttributesFormModel::ItemIdRole ).toString();
  }

  QMimeData *data = availableWidgetsModel.mimeData( indexes );
  QByteArray itemData = data->data( u"application/x-qgsattributesformavailablewidgetsrelement"_s );
  QDataStream stream( &itemData, QIODevice::ReadOnly );
  QStringList obtainedItemIds;

  while ( !stream.atEnd() )
  {
    QString itemId;
    int itemTypeInt;
    QString itemName;
    stream >> itemId >> itemTypeInt >> itemName;

    obtainedItemIds << itemId;
  }
  QCOMPARE( obtainedItemIds, expectedItemIds );

  // Select a field and a relation and see they are stored in a proper order in the mimeData
  indexes.clear();
  expectedItemIds.clear();
  obtainedItemIds.clear();

  QModelIndex relations = availableWidgetsModel.relationContainer();
  QCOMPARE( availableWidgetsModel.rowCount( relations ), 1 );

  // Add indexes backwards to add some complexity :)
  QModelIndex tmpIndex = availableWidgetsModel.index( 0, 0, relations );
  indexes << tmpIndex;
  expectedItemIds << tmpIndex.data( QgsAttributesFormModel::ItemIdRole ).toString();

  tmpIndex = availableWidgetsModel.index( availableWidgetsModel.rowCount( fields ) - 1, 0, fields );
  indexes << tmpIndex;
  // Fields item is above the relations one, so it is expected first
  expectedItemIds.insert( 0, tmpIndex.data( QgsAttributesFormModel::ItemIdRole ).toString() );

  data = availableWidgetsModel.mimeData( indexes );
  itemData = data->data( u"application/x-qgsattributesformavailablewidgetsrelement"_s );
  QDataStream stream2 = QDataStream( &itemData, QIODevice::ReadOnly );

  while ( !stream2.atEnd() )
  {
    QString itemId;
    int itemTypeInt;
    QString itemName;
    stream2 >> itemId >> itemTypeInt >> itemName;

    obtainedItemIds << itemId;
  }
  // Note: this one would fail with the default std::sort
  // (i.e., without our auxiliary function indexLessThan)
  QCOMPARE( obtainedItemIds, expectedItemIds );
}

void TestQgsAttributesFormModel::testFormLayoutModel()
{
  setUpProjectWithRelation();

  QgsAttributesFormLayoutModel formLayoutModel( mLayer, mProject );
  QModelIndex rootIndex = QModelIndex();

#ifdef ENABLE_MODELTEST
  new ModelTest( &formLayoutModel, this ); // for model validity checking
#endif

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 0 );
  QVERIFY( !formLayoutModel.hasChildren() );
  QCOMPARE( formLayoutModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Form Layout" ) );
  QVERIFY( formLayoutModel.mimeTypes().size() == 2 );
  QCOMPARE( formLayoutModel.mimeTypes(), QStringList() << QStringList() << u"application/x-qgsattributesformlayoutelement"_s << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  QVERIFY( !formLayoutModel.showAliases() );
  formLayoutModel.setShowAliases( true );
  QVERIFY( formLayoutModel.showAliases() );
  formLayoutModel.setShowAliases( false );
  QVERIFY( !formLayoutModel.showAliases() );

  // Add data to the model
  formLayoutModel.populate();

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 1 );
  QVERIFY( formLayoutModel.hasChildren() );

  // Check top-level item
  QModelIndex containerIndex = formLayoutModel.index( 0, 0, rootIndex );
  QVERIFY( containerIndex.isValid() );
  QCOMPARE( containerIndex.row(), 0 );
  QCOMPARE( formLayoutModel.data( containerIndex, Qt::DisplayRole ).toString(), "tab"_L1 );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( containerIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Container );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );

  QCOMPARE( containerIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), "tab"_L1 );
  const auto containerData = containerIndex.data( QgsAttributesFormModel::ItemDataRole ).value< QgsAttributesFormData::AttributeFormItemData >();
  QCOMPARE( containerData.containerType(), Qgis::AttributeEditorContainerType::Tab );
  QCOMPARE( containerData.columnCount(), 1 );
  QVERIFY( containerData.showLabel() );

  QList< QgsAddAttributeFormContainerDialog::ContainerPair > containers = formLayoutModel.listOfContainers();
  QCOMPARE( containers.size(), 1 );
  QCOMPARE( containers.at( 0 ).first, "tab"_L1 );
  QCOMPARE( containers.at( 0 ).second, containerIndex );

  // Start modifying model data to check other cases
  formLayoutModel.addContainer( containerIndex, u"My group box"_s, 2, Qgis::AttributeEditorContainerType::GroupBox );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 8 );
  QCOMPARE( formLayoutModel.rowCount(), 1 );

  const QString rowName = u"My row"_s;
  formLayoutModel.addContainer( rootIndex, rowName, 4, Qgis::AttributeEditorContainerType::Row );
  QCOMPARE( formLayoutModel.rowCount(), 2 );

  containers = formLayoutModel.listOfContainers();
  QCOMPARE( containers.size(), 3 );
  QCOMPARE( containers.at( 1 ).first, "My group box"_L1 );
  QCOMPARE( containers.at( 2 ).first, rowName );

  QModelIndex groupContainerIndex = formLayoutModel.index( formLayoutModel.rowCount( containerIndex ) - 1, 0, containerIndex );
  QCOMPARE( containers.at( 1 ).second, groupContainerIndex );

  QModelIndex rowContainerIndex = formLayoutModel.index( formLayoutModel.rowCount( rootIndex ) - 1, 0, rootIndex );
  QCOMPARE( containers.at( 2 ).second, rowContainerIndex );
  QCOMPARE( rowContainerIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), rowName );
  QCOMPARE( rowContainerIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), rowName );

  const QString newFieldName = u"Staff"_s;
  QCOMPARE( formLayoutModel.rowCount( groupContainerIndex ), 0 );
  formLayoutModel.insertChild( groupContainerIndex, 0, newFieldName, QgsAttributesFormData::Field, newFieldName );
  QCOMPARE( formLayoutModel.rowCount( groupContainerIndex ), 1 );

  // First recursive match
  const QString inexistentFieldName = u"my field"_s;
  QModelIndex invalidIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormData::Field, inexistentFieldName );
  QVERIFY( !invalidIndex.isValid() );

  QModelIndex staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );
  QCOMPARE( staffIndex.parent(), containerIndex );

  // Remove the first occurrence of staffIndex, and re-check
  formLayoutModel.removeRow( staffIndex.row(), containerIndex );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );
  staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );

  // Before checking against the groupContainerIndex, we need to
  // get it once more since the precvious one is already outdated
  containers = formLayoutModel.listOfContainers();
  groupContainerIndex = containers.at( 1 ).second;
  QCOMPARE( staffIndex.parent(), groupContainerIndex );

  // 2 staff fields, still the one inside the group is the first that should be found
  formLayoutModel.insertChild( rootIndex, formLayoutModel.rowCount(), newFieldName, QgsAttributesFormData::Field, newFieldName );
  QCOMPARE( formLayoutModel.rowCount(), 3 );
  // Now that the new staff item is after the one in the group box, the first matching index should remain the same
  staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );
  QCOMPARE( staffIndex.parent(), groupContainerIndex );

  // First top match
  invalidIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormData::Field, inexistentFieldName );
  QVERIFY( !invalidIndex.isValid() );

  invalidIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormData::Field, rowName );
  QVERIFY( !invalidIndex.isValid() );

  QModelIndex foundRowIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormData::Container, rowName );
  QVERIFY( foundRowIndex.isValid() );
  QCOMPARE( foundRowIndex.parent(), rootIndex );
  QCOMPARE( foundRowIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), rowName );
  QCOMPARE( foundRowIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), rowName );

  staffIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), newFieldName );

  // Check showAliases mode
  QCOMPARE( staffIndex.data( Qt::DisplayRole ).toString(), newFieldName );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString(), QString() );

  formLayoutModel.setShowAliases( true );

  // Empty alias will be replaced by field name, even if showAliases is true
  QVERIFY( !staffIndex.data( Qt::DisplayRole ).toString().isEmpty() );
  QCOMPARE( staffIndex.data( Qt::DisplayRole ).toString(), newFieldName );

  // Check that non-empty aliases are actually shown
  const QString staffAlias = u"Staff alias!"_s;
  formLayoutModel.updateAliasForFieldItems( newFieldName, staffAlias );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString(), staffAlias );
  QCOMPARE( staffIndex.data( Qt::DisplayRole ).toString(), staffAlias );

  formLayoutModel.setShowAliases( false );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString(), staffAlias );
  QCOMPARE( staffIndex.data( Qt::DisplayRole ).toString(), newFieldName );

  // Lastly check that also the Staff item inside the group box has been updated
  staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormData::Field, newFieldName );
  QCOMPARE( staffIndex.parent(), groupContainerIndex );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString(), staffAlias );

  // Check setData
  QString newStaffAlias = QString( "New Staff alias!" );
  formLayoutModel.setData( staffIndex, newStaffAlias, QgsAttributesFormModel::ItemDisplayRole );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemDisplayRole ).toString(), newStaffAlias );

  formLayoutModel.setData( staffIndex, newStaffAlias, QgsAttributesFormModel::ItemNameRole );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), newStaffAlias );

  formLayoutModel.setData( staffIndex, newStaffAlias, QgsAttributesFormModel::ItemIdRole );
  QCOMPARE( staffIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), newStaffAlias );
}

void TestQgsAttributesFormModel::testFormLayoutModelOrphanFields()
{
  setUpProjectWithRelation();

  QgsAttributesFormLayoutModel formLayoutModel( mLayer, mProject );
  QModelIndex rootIndex = QModelIndex();

#ifdef ENABLE_MODELTEST
  new ModelTest( &formLayoutModel, this ); // for model validity checking
#endif

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 0 );
  QVERIFY( !formLayoutModel.hasChildren() );
  QCOMPARE( formLayoutModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Form Layout" ) );
  QVERIFY( formLayoutModel.mimeTypes().size() == 2 );
  QCOMPARE( formLayoutModel.mimeTypes(), QStringList() << QStringList() << u"application/x-qgsattributesformlayoutelement"_s << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  // Add data to the model
  formLayoutModel.populate();

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 1 );
  QVERIFY( formLayoutModel.hasChildren() );

  // Check top-level item
  const QModelIndex containerIndex = formLayoutModel.index( 0, 0, rootIndex );
  QVERIFY( containerIndex.isValid() );
  QCOMPARE( containerIndex.row(), 0 );
  QCOMPARE( formLayoutModel.data( containerIndex, Qt::DisplayRole ).toString(), "tab"_L1 );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( containerIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Container );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );

  // Check last field (Pilots)
  const int fieldPosition = 5;
  QModelIndex fieldIndex = formLayoutModel.index( fieldPosition, 0, containerIndex );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( fieldIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );
  QCOMPARE( formLayoutModel.data( fieldIndex, Qt::DisplayRole ).toString(), "Pilots"_L1 );
  QCOMPARE( formLayoutModel.data( fieldIndex, Qt::ToolTipRole ).toString(), "Pilots"_L1 );

  // Remove field Pilots (even without committing, which reproduces
  // the scenario of removing a field in Layer Properties without saving,
  // and then going back to Attributes Form page)
  mLayer->startEditing();
  const bool deleted = mLayer->deleteAttribute( mLayer->fields().indexOf( "Pilots"_L1 ) );
  QVERIFY( deleted );

  // Check field Pilots'data changes after layer field removal
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );
  fieldIndex = formLayoutModel.index( fieldPosition, 0, containerIndex );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( fieldIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Field );
  QCOMPARE( formLayoutModel.data( fieldIndex, Qt::DisplayRole ).toString(), "Pilots"_L1 );
  QCOMPARE( formLayoutModel.data( fieldIndex, Qt::ToolTipRole ).toString(), "Invalid field"_L1 );

  const bool discarded = mLayer->rollBack();
  QVERIFY( discarded );
}

void TestQgsAttributesFormModel::testFormLayoutModelDragAndDrop()
{
  // Covers drag-and-drop behavior of the form layout model:
  // - internal moves of a single item (container or field) must relocate it,
  //   not copy it or lose it;
  // - internal moves of multiple items at once must preserve every item and
  //   produce the expected order, including when moving downward (the drop
  //   anchor shifts up each time a source above it is taken out);
  // - items dropped from the available widgets tree must show the same icon
  //   they have in that tree.

  // Internal moves of a single item
  {
    auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer&field=c:integer&field=d:integer"_s, u"test"_s, u"memory"_s );

    // tab
    //  ├─ a
    //  ├─ group
    //  │   ├─ b
    //  │   └─ c
    //  └─ d
    QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
    tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
    QgsAttributeEditorContainer *group = new QgsAttributeEditorContainer( u"group"_s, tab );
    group->addChildElement( new QgsAttributeEditorField( u"b"_s, 1, group ) );
    group->addChildElement( new QgsAttributeEditorField( u"c"_s, 2, group ) );
    tab->addChildElement( group );
    tab->addChildElement( new QgsAttributeEditorField( u"d"_s, 3, tab ) );

    QgsEditFormConfig cfg = layer->editFormConfig();
    cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
    cfg.clearTabs();
    cfg.addTab( tab );
    layer->setEditFormConfig( cfg );

    // Move the "group" container (initially at row 1 under "tab")
    auto moveGroupAndVerify = [&layer, this]( int targetRow, int expectedFinalRow ) {
      QgsAttributesFormLayoutModel model( layer.get(), mProject );
      model.populate();

      const QModelIndex tabIndex = model.index( 0, 0, QModelIndex() );
      QCOMPARE( tabIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"tab"_s );
      QCOMPARE( model.rowCount( tabIndex ), 3 );

      const QModelIndex groupIndex = model.index( 1, 0, tabIndex );
      QCOMPARE( groupIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"group"_s );
      QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( groupIndex.data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Container );
      QCOMPARE( model.rowCount( groupIndex ), 2 );

      // mimeData() records the dragged source item, so dropMimeData() performs a
      // true in-place move: the source is relocated, not copied and left behind.
      QMimeData *mimeData = model.mimeData( QModelIndexList() << groupIndex );
      QVERIFY( mimeData );

      const bool dropped = model.dropMimeData( mimeData, Qt::MoveAction, targetRow, 0, tabIndex );
      QVERIFY( dropped );
      delete mimeData;

      // No duplicate was inserted: the container count is unchanged and the group
      // ended up where we asked, still holding both children.
      QCOMPARE( model.rowCount( tabIndex ), 3 );
      const QModelIndex movedGroup = model.index( expectedFinalRow, 0, tabIndex );
      QCOMPARE( movedGroup.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"group"_s );
      QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( movedGroup.data( QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Container );
      QCOMPARE( movedGroup.parent().data( QgsAttributesFormModel::ItemIdRole ).toString(), u"tab"_s );
      QCOMPARE( model.rowCount( movedGroup ), 2 );
      QCOMPARE( model.index( 0, 0, movedGroup ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"b"_s );
      QCOMPARE( model.index( 1, 0, movedGroup ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"c"_s );

      // The "group" identity must appear exactly once under "tab".
      int groupCount = 0;
      for ( int r = 0; r < model.rowCount( tabIndex ); ++r )
      {
        if ( model.index( r, 0, tabIndex ).data( QgsAttributesFormModel::ItemIdRole ).toString() == "group"_L1 )
          groupCount++;
      }
      QCOMPARE( groupCount, 1 );
    };

    // Move up to the top of the container.
    moveGroupAndVerify( 0, 0 );

    // Move down to the bottom of the container (dropping past the last row appends).
    moveGroupAndVerify( 3, 2 );

    // Reordering a field within its own group must move it, not drop it. Moving
    // "b" (row 0) past "c" (row 1) inside "group" should leave the group holding
    // [c, b] with both fields still present.
    {
      QgsAttributesFormLayoutModel model( layer.get(), mProject );
      model.populate();

      const QModelIndex tabIndex = model.index( 0, 0, QModelIndex() );
      const QModelIndex groupIndex = model.index( 1, 0, tabIndex );
      QCOMPARE( model.rowCount( groupIndex ), 2 );

      const QModelIndex fieldB = model.index( 0, 0, groupIndex );
      QCOMPARE( fieldB.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"b"_s );

      QMimeData *mimeData = model.mimeData( QModelIndexList() << fieldB );
      QVERIFY( mimeData );
      // Drop below "c" (targetRow == 2, i.e. past the last row of the group).
      const bool dropped = model.dropMimeData( mimeData, Qt::MoveAction, 2, 0, groupIndex );
      QVERIFY( dropped );
      delete mimeData;

      QCOMPARE( model.rowCount( groupIndex ), 2 );
      QCOMPARE( model.index( 0, 0, groupIndex ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"c"_s );
      QCOMPARE( model.index( 1, 0, groupIndex ).data( QgsAttributesFormModel::ItemIdRole ).toString(), u"b"_s );
    }

    // we must use Qt::CopyAction
    // otherwise QAbstractItemView::startDrag() will delete the dragged item after a successful drop
    {
      QgsAttributesFormLayoutModel model( layer.get(), mProject );
      QVERIFY( model.supportedDragActions() & Qt::CopyAction );
    }
  }

  // Internal moves of multiple items at once
  {
    auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer&field=c:integer&field=d:integer"_s, u"test"_s, u"memory"_s );

    // tab
    //  ├─ a
    //  ├─ b
    //  ├─ c
    //  └─ d
    QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
    tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
    tab->addChildElement( new QgsAttributeEditorField( u"b"_s, 1, tab ) );
    tab->addChildElement( new QgsAttributeEditorField( u"c"_s, 2, tab ) );
    tab->addChildElement( new QgsAttributeEditorField( u"d"_s, 3, tab ) );

    QgsEditFormConfig cfg = layer->editFormConfig();
    cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
    cfg.clearTabs();
    cfg.addTab( tab );
    layer->setEditFormConfig( cfg );

    // Drag the fields at the given rows onto targetRow and compare the resulting order
    auto moveFieldsAndVerify = [&layer, this]( const QList< int > &sourceRows, int targetRow, const QStringList &expectedOrder ) {
      QgsAttributesFormLayoutModel model( layer.get(), mProject );
      model.populate();

      const QModelIndex tabIndex = model.index( 0, 0, QModelIndex() );
      QCOMPARE( model.rowCount( tabIndex ), 4 );

      QModelIndexList draggedIndexes;
      for ( const int sourceRow : sourceRows )
        draggedIndexes << model.index( sourceRow, 0, tabIndex );

      QMimeData *mimeData = model.mimeData( draggedIndexes );
      QVERIFY( mimeData );

      const bool dropped = model.dropMimeData( mimeData, Qt::MoveAction, targetRow, 0, tabIndex );
      QVERIFY( dropped );
      delete mimeData;

      // No item may get lost or duplicated by the move
      QCOMPARE( model.rowCount( tabIndex ), 4 );

      QStringList order;
      for ( int r = 0; r < model.rowCount( tabIndex ); ++r )
        order << model.index( r, 0, tabIndex ).data( QgsAttributesFormModel::ItemIdRole ).toString();

      QCOMPARE( order, expectedOrder );
    };

    // Move "a" and "b" downward past the end (dropping below "d" appends).
    // The drop anchor shifts up each time a source above it is taken out;
    // a buggy implementation loses "b" entirely here.
    moveFieldsAndVerify( { 0, 1 }, 4, { u"c"_s, u"d"_s, u"a"_s, u"b"_s } );

    // Move "a" and "b" downward in between "c" and "d".
    moveFieldsAndVerify( { 0, 1 }, 3, { u"c"_s, u"a"_s, u"b"_s, u"d"_s } );

    // Move "c" and "d" upward to the top.
    moveFieldsAndVerify( { 2, 3 }, 0, { u"c"_s, u"d"_s, u"a"_s, u"b"_s } );

    // Move "b" and "d" (non-adjacent) upward to the top.
    moveFieldsAndVerify( { 1, 3 }, 0, { u"b"_s, u"d"_s, u"a"_s, u"c"_s } );

    // Move "a" and "c" (non-adjacent) downward past the end.
    moveFieldsAndVerify( { 0, 2 }, 4, { u"b"_s, u"d"_s, u"a"_s, u"c"_s } );
  }

  // External drops from the available widgets tree: dropped items must show
  // the same icon they have in that tree
  {
    setUpProjectWithRelation();

    QgsAttributesAvailableWidgetsModel availableWidgetsModel( mLayer, mProject );
    availableWidgetsModel.populate();

    QgsAttributesFormLayoutModel formLayoutModel( mLayer, mProject );
    formLayoutModel.populate();

    // Collect one index per item type: a field, a relation and all "other" widgets
    QModelIndexList draggedIndexes;
    draggedIndexes << availableWidgetsModel.index( 0, 0, availableWidgetsModel.fieldContainer() );
    draggedIndexes << availableWidgetsModel.index( 0, 0, availableWidgetsModel.relationContainer() );
    const QModelIndex otherWidgetsIndex = availableWidgetsModel.index( 3, 0, QModelIndex() );
    for ( int i = 0; i < availableWidgetsModel.rowCount( otherWidgetsIndex ); i++ )
      draggedIndexes << availableWidgetsModel.index( i, 0, otherWidgetsIndex );

    QMimeData *mimeData = availableWidgetsModel.mimeData( draggedIndexes );
    QVERIFY( mimeData );

    const int rowCountBeforeDrop = formLayoutModel.rowCount();
    QVERIFY( formLayoutModel.dropMimeData( mimeData, Qt::CopyAction, -1, -1, QModelIndex() ) );
    delete mimeData;

    QCOMPARE( formLayoutModel.rowCount(), rowCountBeforeDrop + draggedIndexes.count() );

    for ( int i = 0; i < draggedIndexes.count(); i++ )
    {
      const QModelIndex droppedIndex = formLayoutModel.index( rowCountBeforeDrop + i, 0, QModelIndex() );
      const QIcon icon = droppedIndex.data( Qt::DecorationRole ).value< QIcon >();
      QVERIFY2( !icon.isNull(), u"Dropped item '%1' has no icon"_s.arg( droppedIndex.data( QgsAttributesFormModel::ItemNameRole ).toString() ).toUtf8().constData() );
    }
  }
}

void TestQgsAttributesFormModel::testInvalidRelationInAvailableWidgets()
{
  setUpProjectWithInvalidRelations();

  QgsAttributesAvailableWidgetsModel availableWidgetsModel( mLayer, mProject );

#ifdef ENABLE_MODELTEST
  new ModelTest( &availableWidgetsModel, this ); // for model validity checking
#endif

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 0 );
  QVERIFY( !availableWidgetsModel.hasChildren() );
  QCOMPARE( availableWidgetsModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Available Widgets" ) );
  QVERIFY( availableWidgetsModel.mimeTypes().size() == 1 );
  QCOMPARE( availableWidgetsModel.mimeTypes(), QStringList() << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  // Add data to the model
  availableWidgetsModel.populate();

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 4 );
  QVERIFY( availableWidgetsModel.hasChildren() );

  // Check top-level items
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Fields" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Relations" ) );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::WidgetType );

  // Check broken relation
  const QModelIndex relationsIndex = availableWidgetsModel.index( 1, 0, QModelIndex() );
  QCOMPARE( availableWidgetsModel.rowCount( relationsIndex ), 1 );

  // Broken relation in Available widgets must have an id and name, because it can be
  // find in relation manager. We add a custom tooltip to it and color it red.
  const QModelIndex invalidRelationIndex = availableWidgetsModel.index( 0, 0, relationsIndex );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( availableWidgetsModel.data( invalidRelationIndex, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Relation );
  QCOMPARE( invalidRelationIndex.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"side_buildings_cbbdebec_0396_4617_9416_a4c348d1d529_household_household_0c432204_12d4_47a6_8d90_d759b02560dd_id"_s );
  QCOMPARE( invalidRelationIndex.data( QgsAttributesFormModel::ItemNameRole ).toString(), u"side_buildings_household_fkey"_s );
  QCOMPARE( invalidRelationIndex.data( Qt::DisplayRole ), u"side_buildings_household_fkey"_s );
  QCOMPARE( invalidRelationIndex.data( Qt::ToolTipRole ), tr( "Invalid relation" ) );
}

void TestQgsAttributesFormModel::testInvalidRelationInFormLayout()
{
  setUpProjectWithInvalidRelations();

  QgsAttributesFormLayoutModel formLayoutModel( mLayer, mProject );
  QModelIndex rootIndex = QModelIndex();

#ifdef ENABLE_MODELTEST
  new ModelTest( &formLayoutModel, this ); // for model validity checking
#endif

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 0 );
  QVERIFY( !formLayoutModel.hasChildren() );
  QCOMPARE( formLayoutModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Form Layout" ) );
  QVERIFY( formLayoutModel.mimeTypes().size() == 2 );
  QCOMPARE( formLayoutModel.mimeTypes(), QStringList() << QStringList() << u"application/x-qgsattributesformlayoutelement"_s << u"application/x-qgsattributesformavailablewidgetsrelement"_s );

  // Add data to the model
  formLayoutModel.populate();

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 1 );
  QVERIFY( formLayoutModel.hasChildren() );

  // Check top-level item
  QModelIndex containerIndex = formLayoutModel.index( 0, 0, rootIndex );
  QVERIFY( containerIndex.isValid() );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 4 );

  // Check broken relation 1 (exists in relation manager)
  const QModelIndex relation1Index = formLayoutModel.index( 1, 0, containerIndex );

  // Since it can be found in relation manager, it has an id and name.
  // We add a custom tooltip to it and color it red.
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( relation1Index, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Relation );
  QCOMPARE( relation1Index.data( QgsAttributesFormModel::ItemIdRole ).toString(), u"side_buildings_cbbdebec_0396_4617_9416_a4c348d1d529_household_household_0c432204_12d4_47a6_8d90_d759b02560dd_id"_s );
  QCOMPARE( relation1Index.data( QgsAttributesFormModel::ItemNameRole ).toString(), u"side_buildings_household_fkey"_s );
  QCOMPARE( relation1Index.data( Qt::DisplayRole ), u"side_buildings_household_fkey"_s );
  QCOMPARE( relation1Index.data( Qt::ToolTipRole ), tr( "Invalid relation" ) );

  // Check broken relation 2 (does not exist in relation manager)
  // It was added to the form layout and then it get broken since
  // its relation is missing in the project.
  const QModelIndex relation2Index = formLayoutModel.index( 2, 0, containerIndex );
  QCOMPARE( static_cast< QgsAttributesFormData::AttributesFormItemType >( formLayoutModel.data( relation2Index, QgsAttributesFormModel::ItemTypeRole ).toInt() ), QgsAttributesFormData::Relation );
  QCOMPARE( relation2Index.data( QgsAttributesFormModel::ItemIdRole ).toString(), QString() );   // No id
  QCOMPARE( relation2Index.data( QgsAttributesFormModel::ItemNameRole ).toString(), QString() ); // No name
  QCOMPARE( relation2Index.data( Qt::DisplayRole ), tr( "Invalid relation" ) );                  // Custom display text
  QVERIFY( relation2Index.data( Qt::ToolTipRole ).toString().isEmpty() );                        // No tooltip
}

QGSTEST_MAIN( TestQgsAttributesFormModel )
#include "testqgsattributesformmodel.moc"
