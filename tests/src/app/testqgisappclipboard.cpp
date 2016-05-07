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
    void copyToText();
    void pasteWkt();
    void pasteGeoJson();
    void retrieveFields();
    void clipboardLogic(); //test clipboard logic

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
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

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

void TestQgisAppClipboard::copyToText()
{
  //set clipboard to some QgsFeatures
  QgsFields fields;
  fields.append( QgsField( "int_field", QVariant::Int ) );
  fields.append( QgsField( "string_field", QVariant::String ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( "int_field", 9 );
  feat.setAttribute( "string_field", "val" );
  feat.setGeometry( new QgsGeometry( new QgsPointV2( 5, 6 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( "int_field", 19 );
  feat2.setAttribute( "string_field", "val2" );
  feat2.setGeometry( new QgsGeometry( new QgsPointV2( 7, 8 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  // attributes only
  QSettings settings;
  settings.setValue( "/qgis/copyFeatureFormat", QgsClipboard::AttributesOnly );
  QString result = mQgisApp->clipboard()->generateClipboardText();
  QCOMPARE( result, QString( "int_field\tstring_field\n9\tval\n19\tval2" ) );

  // attributes with WKT
  settings.setValue( "/qgis/copyFeatureFormat", QgsClipboard::AttributesWithWKT );
  result = mQgisApp->clipboard()->generateClipboardText();
  QCOMPARE( result, QString( "wkt_geom\tint_field\tstring_field\nPoint (5 6)\t9\tval\nPoint (7 8)\t19\tval2" ) );

  // GeoJSON
  settings.setValue( "/qgis/copyFeatureFormat", QgsClipboard::GeoJSON );
  result = mQgisApp->clipboard()->generateClipboardText();
  QString expected = "{ \"type\": \"FeatureCollection\",\n    \"features\":[\n"
                     "{\n   \"type\":\"Feature\",\n"
                     "   \"id\":5,\n"
                     "   \"geometry\":\n"
                     "   {\"type\": \"Point\", \"coordinates\": [5, 6]},\n"
                     "   \"properties\":{\n"
                     "      \"int_field\":9,\n"
                     "      \"string_field\":\"val\"\n"
                     "   }\n"
                     "},\n"
                     "{\n   \"type\":\"Feature\",\n"
                     "   \"id\":6,\n"
                     "   \"geometry\":\n"
                     "   {\"type\": \"Point\", \"coordinates\": [7, 8]},\n"
                     "   \"properties\":{\n"
                     "      \"int_field\":19,\n"
                     "      \"string_field\":\"val2\"\n"
                     "   }\n}\n]}";
  QCOMPARE( result, expected );

  // test CRS is transformed correctly for GeoJSON

  QgsCoordinateReferenceSystem crs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  feats = QgsFeatureStore();
  feats.setCrs( crs );
  feat.setGeometry( new QgsGeometry( new QgsPointV2( 2502577, 2403869 ) ) );
  feats.addFeature( feat );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  result = mQgisApp->clipboard()->generateClipboardText();

  // just test coordinates as integers - that's enough to verify that reprojection has occurred
  // and helps avoid rounding issues
  QRegExp regex( "\\[([-\\d.]+), ([-\\d.]+)\\]" );
  ( void )regex.indexIn( result );
  QStringList list = regex.capturedTexts();
  QCOMPARE( list.count(), 3 );

  int x = qRound( list.at( 1 ).toDouble() );
  int y = qRound( list.at( 2 ).toDouble() );

  QCOMPARE( x, 145 );
  QCOMPARE( y, -38 );
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

void TestQgisAppClipboard::clipboardLogic()
{
  //start by setting clipboard contents as text
  mQgisApp->clipboard()->setText( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" );
  QgsFields fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QgsFeatureList features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 1 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );

  //set clipboard to some QgsFeatures
  fields = QgsFields();
  fields.append( QgsField( "int_field", QVariant::Int ) );
  fields.append( QgsField( "date_field", QVariant::Date ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( "int_field", 9 );
  feat.setAttribute( "date_field", QVariant( QDate( 2010, 9, 5 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( "int_field", 19 );
  feat2.setAttribute( "date_field", QVariant( QDate( 2011, 9, 5 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.setFields( fields );
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrsId( 3452 );
  feats.setCrs( crs );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  //test result
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "int_field" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::Int );
  QCOMPARE( fields.at( 1 ).name(), QString( "date_field" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Date );
  features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 2 );
  QCOMPARE( features.at( 0 ).id(), 5LL );
  QCOMPARE( features.at( 0 ).attribute( "int_field" ).toInt(), 9 );
  QCOMPARE( features.at( 0 ).attribute( "date_field" ).toDate(), QDate( 2010, 9, 5 ) );
  QCOMPARE( features.at( 1 ).id(), 6LL );
  QCOMPARE( features.at( 1 ).attribute( "int_field" ).toInt(), 19 );
  QCOMPARE( features.at( 1 ).attribute( "date_field" ).toDate(), QDate( 2011, 9, 5 ) );

  //replace with text again, make sure system clipboard is used rather than internal clipboard
  mQgisApp->clipboard()->setText( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 1 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
}

QTEST_MAIN( TestQgisAppClipboard )
#include "testqgisappclipboard.moc"
