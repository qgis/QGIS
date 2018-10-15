/***************************************************************************
    testqgsvectorlayerproperties.cpp
     --------------------------------------
    Date                 : 15 10 2018
    Copyright            : (C) 2018 Julien Cabieces
    Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <qgsvectorlayerproperties.h>
#include <qgsattributesformproperties.h>
#include <qgsattributetypedialog.h>
#include <qgshiddenwidgetfactory.h>
#include <qgstexteditwidgetfactory.h>
#include <editorwidgets/core/qgseditorwidgetwrapper.h>
#include <qgsapplication.h>
#include "qgisapp.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the qgs vector layer properties dialog
 */
class TestQgsVectorLayerProperties : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayerProperties();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testHiddenWidgetApply();
    void testHiddenWidgetLoad();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsVectorLayerProperties::TestQgsVectorLayerProperties() = default;

void TestQgsVectorLayerProperties::initTestCase()
{
  qDebug() << "TestQgsVectorLayerProperties::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  // setup the test QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

void TestQgsVectorLayerProperties::cleanupTestCase() // will be called after the last testfunction was executed.
{
  QgsApplication::exitQgis();
}

void TestQgsVectorLayerProperties::testHiddenWidgetApply()
{
  QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer" );
  QgsVectorLayer layer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  QgsVectorLayerProperties propertiesDialog( &layer );

  // we hide first column
  layer.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( "Hidden", QVariantMap() ) );
  propertiesDialog.apply();

  QVERIFY( layer.attributeTableConfig().columns()[0].hidden );
  QVERIFY( !layer.attributeTableConfig().columns()[1].hidden );
}

void TestQgsVectorLayerProperties::testHiddenWidgetLoad()
{
  QString def = QStringLiteral( "Point?field=col0:integer&field=col1:integer" );
  QgsVectorLayer layer( def, QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  QgsGui::editorWidgetRegistry()->registerWidget( QStringLiteral( "Hidden" ), new QgsHiddenWidgetFactory( tr( "Hidden" ) ) );
  QgsGui::editorWidgetRegistry()->registerWidget( QStringLiteral( "TextEdit" ), new QgsTextEditWidgetFactory( tr( "Text Edit" ) ) );

  // we hide first column
  layer.setEditorWidgetSetup( 0, QgsEditorWidgetSetup( "Hidden", QVariantMap() ) );

  QgsAttributesFormProperties propertiesWidget( &layer );
  propertiesWidget.initAvailableWidgetsTree();

  // get widgets
  QTreeWidget *widgetsTree = propertiesWidget.findChild<QTreeWidget *>( "mAvailableWidgetsTree" );
  QgsAttributeTypeDialog *widgetTypeCmb = propertiesWidget.findChild<QgsAttributeTypeDialog *>();
  QVERIFY( widgetTypeCmb );

  QList<QTreeWidgetItem *> items = widgetsTree->findItems( "col0", Qt::MatchExactly | Qt::MatchRecursive );
  QVERIFY( items.count() > 0 );
  widgetsTree->setCurrentItem( items.at( 0 ) );
  QVERIFY( widgetTypeCmb->editorWidgetType() == "Hidden" );

  items = widgetsTree->findItems( "col1", Qt::MatchExactly | Qt::MatchRecursive );
  QVERIFY( items.count() > 0 );
  widgetsTree->setCurrentItem( items.at( 0 ) );
  QVERIFY( widgetTypeCmb->editorWidgetType() == "TextEdit" );
}

QGSTEST_MAIN( TestQgsVectorLayerProperties )
#include "testqgsvectorlayerproperties.moc"
