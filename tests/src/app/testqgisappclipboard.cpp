/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Frida  Nov 23  2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include <qgisapp.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsclipboard.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include "qgsgeometry.h"
#include "qgspointv2.h"

/** \ingroup UnitTests
 * This is a unit test for the QgisApp clipboard.
 */
class TestQgisAppClipboard : public QObject
{
    Q_OBJECT

  public:
    TestQgisAppClipboard();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void copyPaste();
    void pasteWkt();
    void pasteGeoJson();
    void retrieveFields();

  private:
    QgisApp * mQgisApp;
    QString mTestDataDir;
};

TestQgisAppClipboard::TestQgisAppClipboard()
    : mQgisApp( nullptr )
{

}

//runs before all tests
void TestQgisAppClipboard::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgisAppClipboard::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgisAppClipboard::copyPaste()
{
  qDebug() << "TestQgisAppClipboard::copyPaste()";

  QMap<QString, int> filesCounts;
  filesCounts.insert( "points.shp", 17 );
  filesCounts.insert( "lines.shp", 6 );
  filesCounts.insert( "polys.shp", 10 );

  Q_FOREACH ( const QString& fileName, filesCounts.keys() )
  {
    // add vector layer
    QString filePath = mTestDataDir + fileName;
    qDebug() << "add vector layer: " << filePath;
    QgsVectorLayer *inputLayer = mQgisApp->addVectorLayer( filePath, fileName, "ogr" );
    QVERIFY( inputLayer->isValid() );

    // copy all features to clipboard
    inputLayer->selectAll();
    mQgisApp->editCopy( inputLayer );

    QgsFeatureList features = mQgisApp->clipboard()->copyOf();
    qDebug() << features.size() << " features copied to clipboard";

    QVERIFY( features.size() == filesCounts.value( fileName ) );

    QgsVectorLayer *pastedLayer = mQgisApp->pasteAsNewMemoryVector( "pasted" );
    QVERIFY( pastedLayer );
    QVERIFY( pastedLayer->isValid() );
    qDebug() << pastedLayer->featureCount() << " features in pasted layer";
    QVERIFY( pastedLayer->featureCount() == filesCounts.value( fileName ) );
  }
}

void TestQgisAppClipboard::pasteWkt()
{
  mQgisApp->clipboard()->setText( "POINT (125 10)\nPOINT (111 30)" );

  QgsFeatureList features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).constGeometry() && !features.at( 0 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 0 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  const QgsPointV2* point = dynamic_cast< QgsPointV2* >( features.at( 0 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).constGeometry() && !features.at( 1 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 1 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  point = dynamic_cast< QgsPointV2* >( features.at( 1 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );
}

void TestQgisAppClipboard::pasteGeoJson()
{
  QgsFields fields;
  fields.append( QgsField( "name", QVariant::String ) );
  mQgisApp->clipboard()->setText( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" );

  QgsFeatureList features = mQgisApp->clipboard()->copyOf( fields );
  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).constGeometry() && !features.at( 0 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 0 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  const QgsPointV2* point = dynamic_cast< QgsPointV2* >( features.at( 0 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
}

void TestQgisAppClipboard::retrieveFields()
{
  //empty string
  mQgisApp->clipboard()->setText( "" );

  QgsFields fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 0 );

  // bad string
  mQgisApp->clipboard()->setText( "asdasdas" );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 0 );

  // geojson string
  mQgisApp->clipboard()->setText( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}" );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QCOMPARE( fields.at( 1 ).name(), QString( "height" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Double );
}

QTEST_MAIN( TestQgisAppClipboard )
#include "testqgisappclipboard.moc"
