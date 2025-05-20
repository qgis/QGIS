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


#include "qgstest.h"

#include "qgsattributesformmodel.h"
#include "qgsattributesformproperties.h"
#include "qgsattributetypedialog.h"
#include "qgsvectorlayer.h"

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
  QVERIFY( attributeFormProperties.mAvailableWidgetsView );
  const QgsAttributesAvailableWidgetsModel *availableWidgetsModel = static_cast<QgsAttributesAvailableWidgetsView *>( attributeFormProperties.mAvailableWidgetsView )->availableWidgetsModel();
  QVERIFY( availableWidgetsModel );
  const QModelIndex fieldContainer = availableWidgetsModel->fieldContainer();

  QVERIFY( fieldContainer.isValid() );
  const QString name = fieldContainer.data( QgsAttributesFormModel::ItemNameRole ).toString();
  QCOMPARE( name, QStringLiteral( "Fields" ) );
  QCOMPARE( availableWidgetsModel->rowCount( fieldContainer ), 2 );

  // Insure that the configuration was stored when switching from one available widgets tree item to another
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( availableWidgetsModel->index( 0, 0, fieldContainer ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( QStringLiteral( "alias0" ) );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( availableWidgetsModel->index( 1, 0, fieldContainer ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  attributeFormProperties.mAttributeTypeDialog->setAlias( QStringLiteral( "alias1" ) );

  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( availableWidgetsModel->index( 0, 0, fieldContainer ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), QStringLiteral( "alias0" ) );
  attributeFormProperties.mAvailableWidgetsView->setCurrentIndex( availableWidgetsModel->index( 1, 0, fieldContainer ) );
  QVERIFY( attributeFormProperties.mAttributeTypeDialog );
  QCOMPARE( attributeFormProperties.mAttributeTypeDialog->alias(), QStringLiteral( "alias1" ) );
}

QGSTEST_MAIN( TestQgsAttributesFormProperties )
#include "testqgsattributesformproperties.moc"
