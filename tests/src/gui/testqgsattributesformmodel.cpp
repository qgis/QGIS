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

#include "qgsattributesformmodel.h"
#include "qgsproject.h"
#include "qgstest.h"

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

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
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
