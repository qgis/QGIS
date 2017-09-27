/***************************************************************************
    testqgsvaluerelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 21 07 2017
    Copyright            : (C) 2017 Paul Blottiere
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
#include <QScrollBar>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include "qgseditorwidgetwrapper.h"
#include <editorwidgets/qgsvaluerelationwidgetwrapper.h>
#include "qgsgui.h"

class TestQgsValueRelationWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsValueRelationWidgetWrapper() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testScrollBarUnlocked();
};

void TestQgsValueRelationWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsValueRelationWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueRelationWidgetWrapper::init()
{
}

void TestQgsValueRelationWidgetWrapper::cleanup()
{
}

void TestQgsValueRelationWidgetWrapper::testScrollBarUnlocked()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=fk|:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );

  // build a value relation widget wrapper
  QListWidget lw;
  QWidget editor;
  QgsValueRelationWidgetWrapper w( &vl1, 0, &editor, nullptr );
  w.setEnabled( true );
  w.initWidget( &lw );

  // add an item virtually
  QListWidgetItem item;
  item.setText( QStringLiteral( "MyText" ) );
  w.mListWidget->addItem( &item );
  QCOMPARE( w.mListWidget->item( 0 )->text(), QString( "MyText" ) );

  // when the widget wrapper is enabled, the container should be enabled
  // as well as items
  w.setEnabled( true );

  QCOMPARE( w.widget()->isEnabled(), true );

  bool itemEnabled = w.mListWidget->item( 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, true );

  // when the widget wrapper is disabled, the container should still be enabled
  // to keep the scrollbar available but items should be disabled to avoid
  // edition
  w.setEnabled( false );

  itemEnabled = w.mListWidget->item( 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, false );

  QCOMPARE( w.widget()->isEnabled(), true );

  // recheck after re-enabled
  w.setEnabled( true );

  QCOMPARE( w.widget()->isEnabled(), true );
  itemEnabled = w.mListWidget->item( 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, true );
}

QGSTEST_MAIN( TestQgsValueRelationWidgetWrapper )
#include "testqgsvaluerelationwidgetwrapper.moc"
