/***************************************************************************
    testqgsdiagramproperties.cpp
    ---------------------
    begin                : 2025/10/08
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdiagramproperties.h"
#include "qgspiediagram.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class TestQgsDiagramProperties : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsDiagramProperties()
      : QgsTest( u"Labeling Widget Tests"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testDataDefinedButtonsPreserved();

  private:
    QString mTestDataDir;
    QgsVectorLayer *mPointsLayer = nullptr;
};

// will be called before the first testfunction is executed.
void TestQgsDiagramProperties::initTestCase()
{
  //create some objects that will be used in all tests...

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //create a point layer that will be used in all tests...
  const QString myPointsFileName = mTestDataDir + "stacked_diagrams.gpkg|layername=centroids";
  mPointsLayer = new QgsVectorLayer( myPointsFileName, u"population"_s, u"ogr"_s );

  QgsProject::instance()->addMapLayer( mPointsLayer );
}

// will be called after the last testfunction was executed.
void TestQgsDiagramProperties::cleanupTestCase()
{
  QgsProject::instance()->removeAllMapLayers();

  QgsApplication::exitQgis();
}

void TestQgsDiagramProperties::init()
{
}

void TestQgsDiagramProperties::cleanup()
{
}

void TestQgsDiagramProperties::testDataDefinedButtonsPreserved()
{
  // test that data defined properties are preserved and not reset when reopening diagram properties

  auto widget = new QgsDiagramProperties( mPointsLayer, nullptr, nullptr );
  widget->syncToLayer();
  widget->setDiagramType( QgsPieDiagram::DIAGRAM_NAME_PIE );

  QVERIFY( widget );

  // Check that no data defined property is there
  QCOMPARE( widget->mDataDefinedProperties.count(), 0 );
  QCOMPARE( widget->mShowDiagramDDBtn->toProperty(), QgsProperty() );

  // Modify and check that we have it now
  widget->mShowDiagramDDBtn->setToProperty( QgsProperty::fromField( u"show_diagrams"_s, true ) );
  emit widget->mShowDiagramDDBtn->changed();
  QCOMPARE( widget->mDataDefinedProperties.count(), 1 );
  QVERIFY( widget->mDataDefinedProperties.isActive( QgsDiagramLayerSettings::Property::Show ) );
  widget->apply(); // Transfer diagram layer settings to layer

  // Open a new widget
  delete widget;
  widget = new QgsDiagramProperties( mPointsLayer, nullptr, nullptr );
  widget->syncToLayer();
  widget->setDiagramType( QgsPieDiagram::DIAGRAM_NAME_PIE );

  // Check that the data defined property is preserved and set to the button
  QCOMPARE( widget->mDataDefinedProperties.count(), 1 );
  QVERIFY( widget->mDataDefinedProperties.isActive( QgsDiagramLayerSettings::Property::Show ) );
  QgsProperty property = widget->mShowDiagramDDBtn->toProperty();
  QVERIFY( property.isActive() );
  QCOMPARE( property.propertyType(), Qgis::PropertyType::Field );
  QCOMPARE( property.field(), u"show_diagrams"_s );

  // Open a new widget and sync to settings (used for instance by stacked diagrams)
  delete widget;
  widget = new QgsDiagramProperties( mPointsLayer, nullptr, nullptr );
  widget->syncToSettings( mPointsLayer->diagramLayerSettings() );
  widget->setDiagramType( QgsPieDiagram::DIAGRAM_NAME_PIE );

  // Check that the data defined property is preserved and set to the button
  QCOMPARE( widget->mDataDefinedProperties.count(), 1 );
  QVERIFY( widget->mDataDefinedProperties.isActive( QgsDiagramLayerSettings::Property::Show ) );
  property = widget->mShowDiagramDDBtn->toProperty();
  QVERIFY( property.isActive() );
  QCOMPARE( property.propertyType(), Qgis::PropertyType::Field );
  QCOMPARE( property.field(), u"show_diagrams"_s );

  delete widget;
}

QGSTEST_MAIN( TestQgsDiagramProperties )
#include "testqgsdiagramproperties.moc"
