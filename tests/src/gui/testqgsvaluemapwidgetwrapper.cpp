/***************************************************************************
    testqgsvaluemapwidgetwrapper.cpp
     --------------------------------------
    Date                 : January 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "qgsapplication.h"
#include "qgseditorwidgetwrapper.h"
#include "editorwidgets/qgsvaluemapwidgetwrapper.h"
#include "qgsvaluemapfieldformatter.h"
#include "editorwidgets/qgsvaluemapconfigdlg.h"
#include "qgsgui.h"

class TestQgsValueMapWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsValueMapWidgetWrapper() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void testPopulateComboBox();

};

void TestQgsValueMapWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsValueMapWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueMapWidgetWrapper::init()
{
}

void TestQgsValueMapWidgetWrapper::cleanup()
{
}

void TestQgsValueMapWidgetWrapper::testPopulateComboBox()
{
  // new style config
  QVariantMap config;
  QList<QVariant> valueList;
  QVariantMap nullValue;
  nullValue.insert( QgsApplication::nullRepresentation(), QgsValueMapFieldFormatter::NULL_VALUE );
  valueList.append( nullValue );
  QVariantMap value1;
  value1.insert( QStringLiteral( "desc 1" ), QStringLiteral( "val 1" ) );
  valueList.append( value1 );
  QVariantMap value2;
  value2.insert( QStringLiteral( "desc 2" ), QStringLiteral( "val 2" ) );
  valueList.append( value2 );

  config.insert( QStringLiteral( "map" ), valueList );


  std::unique_ptr< QComboBox > combo = qgis::make_unique< QComboBox >();

  // with nulls
  QgsValueMapConfigDlg::populateComboBox( combo.get(), config, false );

  QCOMPARE( combo->count(), 3 );
  QCOMPARE( combo->itemText( 0 ), QgsApplication::nullRepresentation() );
  QCOMPARE( combo->itemData( 0 ).toString(), QgsValueMapFieldFormatter::NULL_VALUE );
  QCOMPARE( combo->itemText( 1 ), QStringLiteral( "desc 1" ) );
  QCOMPARE( combo->itemData( 1 ).toString(), QStringLiteral( "val 1" ) );
  QCOMPARE( combo->itemText( 2 ), QStringLiteral( "desc 2" ) );
  QCOMPARE( combo->itemData( 2 ).toString(), QStringLiteral( "val 2" ) );

  // no nulls
  combo->clear();
  QgsValueMapConfigDlg::populateComboBox( combo.get(), config, true );

  QCOMPARE( combo->count(), 2 );
  QCOMPARE( combo->itemText( 0 ), QStringLiteral( "desc 1" ) );
  QCOMPARE( combo->itemData( 0 ).toString(), QStringLiteral( "val 1" ) );
  QCOMPARE( combo->itemText( 1 ), QStringLiteral( "desc 2" ) );
  QCOMPARE( combo->itemData( 1 ).toString(), QStringLiteral( "val 2" ) );

  // old style config map (2.x)
  config.clear();
  QVariantMap mapValue;
  mapValue.insert( QgsApplication::nullRepresentation(), QgsValueMapFieldFormatter::NULL_VALUE );
  mapValue.insert( QStringLiteral( "desc 1" ), QStringLiteral( "val 1" ) );
  mapValue.insert( QStringLiteral( "desc 2" ), QStringLiteral( "val 2" ) );
  config.insert( QStringLiteral( "map" ), mapValue );

  // with nulls
  combo->clear();
  QgsValueMapConfigDlg::populateComboBox( combo.get(), config, false );

  QCOMPARE( combo->count(), 3 );
  QCOMPARE( combo->itemText( 0 ), QgsApplication::nullRepresentation() );
  QCOMPARE( combo->itemData( 0 ).toString(), QgsValueMapFieldFormatter::NULL_VALUE );
  QCOMPARE( combo->itemText( 1 ), QStringLiteral( "desc 1" ) );
  QCOMPARE( combo->itemData( 1 ).toString(), QStringLiteral( "val 1" ) );
  QCOMPARE( combo->itemText( 2 ), QStringLiteral( "desc 2" ) );
  QCOMPARE( combo->itemData( 2 ).toString(), QStringLiteral( "val 2" ) );

  // no nulls
  combo->clear();
  QgsValueMapConfigDlg::populateComboBox( combo.get(), config, true );

  QCOMPARE( combo->count(), 2 );
  QCOMPARE( combo->itemText( 0 ), QStringLiteral( "desc 1" ) );
  QCOMPARE( combo->itemData( 0 ).toString(), QStringLiteral( "val 1" ) );
  QCOMPARE( combo->itemText( 1 ), QStringLiteral( "desc 2" ) );
  QCOMPARE( combo->itemData( 1 ).toString(), QStringLiteral( "val 2" ) );

}

QGSTEST_MAIN( TestQgsValueMapWidgetWrapper )
#include "testqgsvaluemapwidgetwrapper.moc"
