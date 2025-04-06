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


class TestQgsAttributesFormModel : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void testAvailableWidgetsModel();
    void testFormLayoutModel();

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsProject *mProject = QgsProject::instance();
};


void TestQgsAttributesFormModel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  // Set up sample project with relation
  const QString projectPath = QStringLiteral( TEST_DATA_DIR ) + "/relations.qgs";
  QVERIFY( mProject->read( projectPath ) );

  const QString layerId = QLatin1String( "points_97805748_6b30_49b8_a80b_bdbb4e8e78a3" );
  mLayer = qobject_cast< QgsVectorLayer * >( mProject->mapLayer( layerId ) );
}

void TestQgsAttributesFormModel::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributesFormModel::testAvailableWidgetsModel()
{
  QgsAttributesAvailableWidgetsModel availableWidgetsModel( mLayer, mProject );

#ifdef ENABLE_MODELTEST
  new ModelTest( &availableWidgetsModel, this ); // for model validity checking
#endif

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 0 );
  QVERIFY( !availableWidgetsModel.hasChildren() );
  QCOMPARE( availableWidgetsModel.headerData( 0, Qt::Orientation::Horizontal, Qt::DisplayRole ), tr( "Available Widgets" ) );
  QVERIFY( availableWidgetsModel.mimeTypes().size() == 1 );
  QCOMPARE( availableWidgetsModel.mimeTypes(), QStringList() << QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" ) );

  // Add data to the model
  availableWidgetsModel.populate();

  QCOMPARE( availableWidgetsModel.columnCount(), 1 );
  QCOMPARE( availableWidgetsModel.rowCount(), 4 );
  QVERIFY( availableWidgetsModel.hasChildren() );

  // Check top-level items
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Fields" ) );
  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( availableWidgetsModel.data( availableWidgetsModel.index( 0, 0, QModelIndex() ), QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Relations" ) );
  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( availableWidgetsModel.data( availableWidgetsModel.index( 1, 0, QModelIndex() ), QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Actions" ) );
  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( availableWidgetsModel.data( availableWidgetsModel.index( 2, 0, QModelIndex() ), QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::WidgetType );
  QCOMPARE( availableWidgetsModel.data( availableWidgetsModel.index( 3, 0, QModelIndex() ), Qt::DisplayRole ).toString(), tr( "Other Widgets" ) );
  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( availableWidgetsModel.data( availableWidgetsModel.index( 3, 0, QModelIndex() ), QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::WidgetType );

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

  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( availableWidgetsModel.data( fieldIndex, QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::Field );

  const QString fieldName = fieldIndex.data( QgsAttributesFormModel::NodeNameRole ).toString();

  QModelIndex foundIndex = availableWidgetsModel.fieldModelIndex( QString() );
  QVERIFY( !foundIndex.isValid() );

  foundIndex = availableWidgetsModel.fieldModelIndex( fieldName );
  QCOMPARE( foundIndex, fieldIndex );

  const QString fieldId = fieldIndex.data( QgsAttributesFormModel::NodeIdRole ).toString();
  QCOMPARE( fieldId, fieldName );

  const auto fieldData = fieldIndex.data( QgsAttributesFormModel::NodeDataRole ).value< QgsAttributesFormTreeData::DnDTreeNodeData >();
  QVERIFY( fieldData.showLabel() );

  const int fieldIdx = mLayer->fields().indexOf( fieldName );
  QCOMPARE( fieldIdx, 0 );
  QgsAttributesFormTreeData::FieldConfig config( mLayer, fieldIdx );

  const auto fieldConfig = fieldIndex.data( QgsAttributesFormModel::NodeFieldConfigRole ).value< QgsAttributesFormTreeData::FieldConfig >();
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

void TestQgsAttributesFormModel::testFormLayoutModel()
{
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
  QCOMPARE( formLayoutModel.mimeTypes(), QStringList() << QStringList() << QStringLiteral( "application/x-qgsattributesformlayoutelement" ) << QStringLiteral( "application/x-qgsattributesformavailablewidgetsrelement" ) );

  QVERIFY( !formLayoutModel.showAliases() );
  formLayoutModel.setShowAliases( true );
  QVERIFY( formLayoutModel.showAliases() );

  // Add data to the model
  formLayoutModel.populate();

  QCOMPARE( formLayoutModel.columnCount(), 1 );
  QCOMPARE( formLayoutModel.rowCount(), 1 );
  QVERIFY( formLayoutModel.hasChildren() );

  // Check top-level item
  QModelIndex containerIndex = formLayoutModel.index( 0, 0, rootIndex );
  QVERIFY( containerIndex.isValid() );
  QCOMPARE( containerIndex.row(), 0 );
  QCOMPARE( formLayoutModel.data( containerIndex, Qt::DisplayRole ).toString(), QLatin1String( "tab" ) );
  QCOMPARE( static_cast< QgsAttributesFormTreeData::AttributesFormTreeNodeType >( formLayoutModel.data( containerIndex, QgsAttributesFormModel::NodeTypeRole ).toInt() ), QgsAttributesFormTreeData::Container );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );

  QCOMPARE( containerIndex.data( QgsAttributesFormModel::NodeNameRole ).toString(), QLatin1String( "tab" ) );
  const auto containerData = containerIndex.data( QgsAttributesFormModel::NodeDataRole ).value< QgsAttributesFormTreeData::DnDTreeNodeData >();
  QCOMPARE( containerData.containerType(), Qgis::AttributeEditorContainerType::Tab );
  QCOMPARE( containerData.columnCount(), 1 );
  QVERIFY( containerData.showLabel() );

  QList< QgsAddAttributeFormContainerDialog::ContainerPair > containers = formLayoutModel.listOfContainers();
  QCOMPARE( containers.size(), 1 );
  QCOMPARE( containers.at( 0 ).first, QLatin1String( "tab" ) );
  QCOMPARE( containers.at( 0 ).second, containerIndex );

  // Start modifying model data to check other cases
  formLayoutModel.addContainer( containerIndex, QStringLiteral( "My group box" ), 2, Qgis::AttributeEditorContainerType::GroupBox );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 8 );
  const QString rowName = QStringLiteral( "My row" );
  formLayoutModel.addContainer( rootIndex, rowName, 3, Qgis::AttributeEditorContainerType::Row );
  QCOMPARE( formLayoutModel.rowCount( rootIndex ), 2 );

  containers = formLayoutModel.listOfContainers();
  QCOMPARE( containers.size(), 3 );
  QCOMPARE( containers.at( 1 ).first, QLatin1String( "My group box" ) );
  QCOMPARE( containers.at( 2 ).first, rowName );

  QModelIndex groupContainerIndex = formLayoutModel.index( formLayoutModel.rowCount( containerIndex ) - 1, 0, containerIndex );
  QCOMPARE( containers.at( 1 ).second, groupContainerIndex );

  QModelIndex rowContainerIndex = formLayoutModel.index( formLayoutModel.rowCount( rootIndex ) - 1, 0, rootIndex );
  QCOMPARE( containers.at( 2 ).second, rowContainerIndex );

  const QString newFieldName = QStringLiteral( "Staff" );
  QCOMPARE( formLayoutModel.rowCount( groupContainerIndex ), 0 );
  formLayoutModel.insertChild( groupContainerIndex, 0, newFieldName, QgsAttributesFormTreeData::Field, newFieldName );
  QCOMPARE( formLayoutModel.rowCount( groupContainerIndex ), 1 );

  // First recursive match
  const QString inexistentFieldName = QStringLiteral( "my field" );
  QModelIndex invalidIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormTreeData::Field, inexistentFieldName );
  QVERIFY( !invalidIndex.isValid() );

  QModelIndex staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormTreeData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );
  QCOMPARE( staffIndex.parent(), containerIndex );

  // Remove the first occurrence of staffIndex, and re-check
  formLayoutModel.removeRow( staffIndex.row(), containerIndex );
  QCOMPARE( formLayoutModel.rowCount( containerIndex ), 7 );
  staffIndex = formLayoutModel.firstRecursiveMatchingModelIndex( QgsAttributesFormTreeData::Field, newFieldName );
  QVERIFY( staffIndex.isValid() );

  // Before checking against the groupContainerIndex, we need to
  // get it once more since the precvious one is already outdated
  containers = formLayoutModel.listOfContainers();
  groupContainerIndex = containers.at( 1 ).second;
  QCOMPARE( staffIndex.parent(), groupContainerIndex );

  // 2 staff fields, still the one inside the group is the first that should be found
  formLayoutModel.insertChild( rootIndex, 0, newFieldName, QgsAttributesFormTreeData::Field, newFieldName );
  QCOMPARE( formLayoutModel.rowCount( rootIndex ), 3 );
  // Now that the new staff item is after the one in the group box, the first matching index should remain the same
  QCOMPARE( staffIndex.parent(), groupContainerIndex );

  // First top match
  invalidIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormTreeData::Field, inexistentFieldName );
  QVERIFY( !invalidIndex.isValid() );

  invalidIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormTreeData::Field, rowName );
  QVERIFY( !invalidIndex.isValid() );

  QModelIndex foundRowIndex = formLayoutModel.firstTopMatchingModelIndex( QgsAttributesFormTreeData::Container, rowName );
  QVERIFY( foundRowIndex.isValid() );
  QCOMPARE( foundRowIndex.parent(), rootIndex );
  QCOMPARE( foundRowIndex.data( QgsAttributesFormModel::NodeNameRole ).toString(), rowName );
}

QGSTEST_MAIN( TestQgsAttributesFormModel )
#include "testqgsattributesformmodel.moc"
