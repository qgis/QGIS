/***************************************************************************
    testqgsattributesformproperties.cpp
     --------------------------------------
    Date                 : 2025-01-21
    Copyright            : (C) 2025 Mathieu Pellerin
    Email                : paul dot blottiere at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsgui.h"

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
{
}

void TestQgsAttributesFormProperties::cleanup()
{
}

void TestQgsAttributesFormProperties::testConfigStored()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=col0:integer&field=col1:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QgsAttributesFormProperties attributeFormProperties( layer );
  attributeFormProperties.init();

  // Get the fields
  QVERIFY( attributeFormProperties.mAvailableWidgetsTree );
  QTreeWidgetItem *fieldsItem = attributeFormProperties.mAvailableWidgetsTree->topLevelItem( 0 );
  QVERIFY( fieldsItem );
  QCOMPARE( fieldsItem->text( 0 ), QStringLiteral( "Fields" ) );
  QCOMPARE( fieldsItem->childCount(), 2 );

  // Insure that the configuration was stored when switching from one available widgets tree item to another
  attributeFormProperties.mAvailableWidgetsTree->setCurrentItem( fieldsItem->child( 0 ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( QStringLiteral( "alias0" ) );
  attributeFormProperties.mAvailableWidgetsTree->setCurrentItem( fieldsItem->child( 1 ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( QStringLiteral( "alias1" ) );

  attributeFormProperties.mAvailableWidgetsTree->setCurrentItem( fieldsItem->child( 0 ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), QStringLiteral( "alias0" ) );
  attributeFormProperties.mAvailableWidgetsTree->setCurrentItem( fieldsItem->child( 1 ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), QStringLiteral( "alias1" ) );
}

QGSTEST_MAIN( TestQgsAttributesFormProperties )
#include "testqgsattributesformproperties.moc"
