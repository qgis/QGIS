/***************************************************************************
    testqgsvaluemapconfigdlg.cpp
     --------------------------------------
    Date                 : 14 02 2021
    Copyright            : (C) 2019 Stephen Knox
    Email                : stephenknox73 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "editorwidgets/qgsvaluemapconfigdlg.h"
#include "qgsapplication.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgsogrproviderutils.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class TestQgsValueMapConfigDlg : public QObject
{
    Q_OBJECT
  public:
    TestQgsValueMapConfigDlg() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testLoadFromCSV();
    void testTestTrimValues();
};

void TestQgsValueMapConfigDlg::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsValueMapConfigDlg::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueMapConfigDlg::init()
{
}

void TestQgsValueMapConfigDlg::cleanup()
{
}

void TestQgsValueMapConfigDlg::testLoadFromCSV()
{
  const QString dataDir( TEST_DATA_DIR );
  QgsVectorLayer vl( u"LineString?crs=epsg:3111&field=pk:int&field=name:string"_s, u"vl1"_s, u"memory"_s );

  QList<QVariant> valueList;
  QVariantMap value;
  value.insert( u"Basic unquoted record"_s, QString( "1" ) );
  valueList << value;
  value.clear();
  value.insert( u"Forest type"_s, QString( "2" ) );
  valueList << value;
  value.clear();
  value.insert( u"So-called \"data\""_s, QString( "three" ) );
  valueList << value;
  value.clear();
  value.insert( u"444"_s, QString( "4" ) );
  valueList << value;
  value.clear();
  value.insert( u"five"_s, QString( "5" ) );
  valueList << value;

  QgsValueMapConfigDlg *valueMapConfig = static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( u"ValueMap"_s, &vl, 1, nullptr ) );
  valueMapConfig->loadMapFromCSV( dataDir + u"/valuemapsample.csv"_s );
  QCOMPARE( valueMapConfig->config().value( u"map"_s ).toList(), valueList );
  delete valueMapConfig;
}

void TestQgsValueMapConfigDlg::testTestTrimValues()
{
  // Create a GPKG layer in a temporary file using GDAL
  QTemporaryDir tempDir;
  const QString tempFile = tempDir.path() + QDir::separator() + u"test.gpkg"_s;

  QList<QPair<QString, QString>> fields;
  fields << QPair<QString, QString>( u"key"_s, u"String;1"_s );
  QString error;
  QVERIFY( QgsOgrProviderUtils::createEmptyDataSource( tempFile, u"GPKG"_s, u"UTF-8"_s, Qgis::WkbType::Point, fields, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), error ) );
  QgsVectorLayer vl { tempFile, u"vl1"_s, u"ogr"_s };
  QVERIFY( vl.isValid() );
  QCOMPARE( vl.fields().count(), 2 );

  QMap<QString, QVariant> valueList;
  valueList[u"1"_s] = u"Choice 1"_s;
  valueList[u"2"_s] = u"Choice 2"_s;

  std::unique_ptr<QgsValueMapConfigDlg> valueMapConfig;
  valueMapConfig.reset( static_cast<QgsValueMapConfigDlg *>( QgsGui::editorWidgetRegistry()->createConfigWidget( u"ValueMap"_s, &vl, 1, nullptr ) ) );

  valueMapConfig->updateMap( valueList, false );
  valueMapConfig->addNullButtonPushed();
  QVERIFY( !valueMapConfig->mValueMapErrorsLabel->text().contains( u"trimmed"_s ) );

  valueList[u"33"_s] = u"Choice 33"_s;
  valueMapConfig->updateMap( valueList, false );
  QVERIFY( valueMapConfig->mValueMapErrorsLabel->text().contains( u"trimmed"_s ) );
}

QGSTEST_MAIN( TestQgsValueMapConfigDlg )
#include "testqgsvaluemapconfigdlg.moc"
