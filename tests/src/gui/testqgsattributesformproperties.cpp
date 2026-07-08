/***************************************************************************
    testqgsattributesformproperties.cpp
     --------------------------------------
    Date                 : 2025-01-21
    Copyright            : (C) 2025 Mathieu Pellerin
    Email                : mathieu at opengis dot ch
 ***************************************************************************
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
#include "qgsattributesformproperties.h"
#include "qgsattributesformview.h"
#include "qgsattributetypedialog.h"
#include "qgseditformconfig.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QItemSelectionModel>
#include <QString>

using namespace Qt::StringLiterals;

class TestQgsAttributesFormProperties : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributesFormProperties() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testConfigStored();
    void testRemoveDuplicateField();
};

void TestQgsAttributesFormProperties::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsAttributesFormProperties::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributesFormProperties::init()
{}

void TestQgsAttributesFormProperties::cleanup()
{}

void TestQgsAttributesFormProperties::testConfigStored()
{
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=col0:integer&field=col1:integer"_s, u"test"_s, u"memory"_s );
  QgsAttributesFormProperties attributeFormProperties( layer );
  attributeFormProperties.init();

  // Get the fields
  QVERIFY( attributeFormProperties.mAvailableWidgetsView );
  const QgsAttributesAvailableWidgetsModel *availableWidgetsModel = static_cast<QgsAttributesAvailableWidgetsView *>( attributeFormProperties.mAvailableWidgetsView )->availableWidgetsModel();
  QVERIFY( availableWidgetsModel );
  const QModelIndex fieldContainer = availableWidgetsModel->fieldContainer();

  QVERIFY( fieldContainer.isValid() );
  const QString name = fieldContainer.data( QgsAttributesFormModel::ItemNameRole ).toString();
  QCOMPARE( name, u"Fields"_s );
  QCOMPARE( availableWidgetsModel->rowCount( fieldContainer ), 2 );

  // Insure that the configuration was stored when switching from one available widgets tree item to another
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( u"alias0"_s );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 1, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( u"alias1"_s );

  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 0, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), u"alias0"_s );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( attributeFormProperties.mAvailableWidgetsProxyModel->mapFromSource( availableWidgetsModel->index( 1, 0, fieldContainer ) ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), u"alias1"_s );
}

void TestQgsAttributesFormProperties::testRemoveDuplicateField()
{
  // Regression test for the "removing a duplicate field deletes every copy" bug:
  // add the same field several times to a tab, select one copy and trigger the
  // real trash-button slot. Exactly that copy must be removed.

  auto layer = std::make_unique< QgsVectorLayer >( u"Point?field=a:integer&field=b:integer"_s, u"test"_s, u"memory"_s );

  // tab
  //  ├─ a   (row 0)
  //  ├─ a   (row 1)
  //  └─ a   (row 2)
  QgsAttributeEditorContainer *tab = new QgsAttributeEditorContainer( u"tab"_s, nullptr );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );
  tab->addChildElement( new QgsAttributeEditorField( u"a"_s, 0, tab ) );

  QgsEditFormConfig cfg = layer->editFormConfig();
  cfg.setLayout( Qgis::AttributeFormLayout::DragAndDrop );
  cfg.clearTabs();
  cfg.addTab( tab );
  layer->setEditFormConfig( cfg );

  QgsAttributesFormProperties attributeFormProperties( layer.get() );
  attributeFormProperties.init();

  QgsAttributesFormProxyModel *proxy = attributeFormProperties.mFormLayoutProxyModel;
  const QModelIndex tabProxyIndex = proxy->index( 0, 0, QModelIndex() );
  QCOMPARE( proxy->rowCount( tabProxyIndex ), 3 );

  // Select the middle duplicate (row 1) in the form layout view, then trigger
  // the same slot the trash button is connected to.
  const QModelIndex middle = proxy->index( 1, 0, tabProxyIndex );
  attributeFormProperties.mFormLayoutView->selectionModel()->setCurrentIndex( middle, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  QCOMPARE( attributeFormProperties.mFormLayoutView->selectionModel()->selectedRows( 0 ).count(), 1 );

  attributeFormProperties.removeTabOrGroupButton();

  // Exactly one copy must have been removed, leaving two behind.
  QgsAttributesFormLayoutModel *layoutModel = attributeFormProperties.mFormLayoutModel;
  QCOMPARE( layoutModel->rowCount( layoutModel->index( 0, 0, QModelIndex() ) ), 2 );
}

QGSTEST_MAIN( TestQgsAttributesFormProperties )
#include "testqgsattributesformproperties.moc"
