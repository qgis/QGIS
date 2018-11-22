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
#include "qgstest.h"

#include <QApplication>
#include <QObject>
#include <QSplashScreen>
#include <QString>
#include <QStringList>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfield.h"
#include "qgsclipboard.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgssettings.h"

/**
 * \ingroup UnitTests
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
    QgisApp *mQgisApp = nullptr;
    QString mTestDataDir;
};

TestQgisAppClipboard::TestQgisAppClipboard() = default;

//runs before all tests
void TestQgisAppClipboard::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
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
  filesCounts.insert( QStringLiteral( "points.shp" ), 17 );
  filesCounts.insert( QStringLiteral( "lines.shp" ), 6 );
  filesCounts.insert( QStringLiteral( "polys.shp" ), 10 );

  Q_FOREACH ( const QString &fileName, filesCounts.keys() )
  {
    // add vector layer
    QString filePath = mTestDataDir + fileName;
    qDebug() << "add vector layer: " << filePath;
    QgsVectorLayer *inputLayer = mQgisApp->addVectorLayer( filePath, fileName, QStringLiteral( "ogr" ) );
    QVERIFY( inputLayer->isValid() );

    // copy all features to clipboard
    inputLayer->selectAll();
    mQgisApp->copySelectionToClipboard( inputLayer );

    QgsFeatureList features = mQgisApp->clipboard()->copyOf();
    qDebug() << features.size() << " features copied to clipboard";

    QVERIFY( features.size() == filesCounts.value( fileName ) );

    QgsVectorLayer *pastedLayer = mQgisApp->pasteAsNewMemoryVector( QStringLiteral( "pasted" ) );
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
  fields.append( QgsField( QStringLiteral( "int_field" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "string_field" ), QVariant::String ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( QStringLiteral( "int_field" ), 9 );
  feat.setAttribute( QStringLiteral( "string_field" ), "val" );
  feat.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( QStringLiteral( "int_field" ), 19 );
  feat2.setAttribute( QStringLiteral( "string_field" ), "val2" );
  feat2.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  // attributes only
  QgsSettings settings;
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesOnly );
  QString result = mQgisApp->clipboard()->generateClipboardText();
  QCOMPARE( result, QString( "int_field\tstring_field\n9\tval\n19\tval2" ) );

  // attributes with WKT
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesWithWKT );
  result = mQgisApp->clipboard()->generateClipboardText();
  QCOMPARE( result, QString( "wkt_geom\tint_field\tstring_field\nPoint (5 6)\t9\tval\nPoint (7 8)\t19\tval2" ) );

  // HTML test
  mQgisApp->clipboard()->replaceWithCopyOf( feats );
  result = mQgisApp->clipboard()->data( "text/html" );
  QCOMPARE( result, QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>wkt_geom</td><td>int_field</td><td>string_field</td></tr><tr><td>Point (5 6)</td><td>9</td><td>val</td></tr><tr><td>Point (7 8)</td><td>19</td><td>val2</td></tr></table></body></html>" ) );

  // GeoJSON
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::GeoJSON );
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
  feat.setGeometry( QgsGeometry( new QgsPoint( 2502577, 2403869 ) ) );
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

  int x = std::round( list.at( 1 ).toDouble() );
  int y = std::round( list.at( 2 ).toDouble() );

  QCOMPARE( x, 145 );
  QCOMPARE( y, -38 );
}

void TestQgisAppClipboard::pasteWkt()
{
  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (125 10)\nPOINT (111 30)" ) );

  QgsFeatureList features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  QgsGeometry featureGeom = features.at( 0 ).geometry();
  const QgsPoint *point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  point = dynamic_cast< const QgsPoint * >( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  // be sure parsing does not consider attached parameters that
  // can change geometryType as in https://issues.qgis.org/issues/16870
  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (111 30)\t GoodFieldValue\nPOINT (125 10)\t(WrongFieldValue)" ) );

  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );

  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  point = dynamic_cast< const QgsPoint * >( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );

  //clipboard should support features without geometry
  mQgisApp->clipboard()->setText( QStringLiteral( "\tMNL\t11\t282\tkm\t\t\t\n\tMNL\t11\t347.80000000000001\tkm\t\t\t" ) );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).attributes().count(), 7 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), QStringLiteral( "MNL" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), QStringLiteral( "11" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), QStringLiteral( "282" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 3 ).toString(), QStringLiteral( "km" ) );
  QVERIFY( features.at( 0 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 6 ).toString().isEmpty() );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 7 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), QStringLiteral( "MNL" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "11" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "347.80000000000001" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 3 ).toString(), QStringLiteral( "km" ) );
  QVERIFY( features.at( 1 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 6 ).toString().isEmpty() );

  mQgisApp->clipboard()->setText( QStringLiteral( "wkt_geom\ta\tb\tc\n\tMNL\t11\t282\tkm\t\t\t\n\tMNL\t11\t347.80000000000001\tkm\t\t\t" ) );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).fields().count(), 3 );
  QCOMPARE( features.at( 0 ).fields().at( 0 ).name(), QStringLiteral( "a" ) );
  QCOMPARE( features.at( 0 ).fields().at( 1 ).name(), QStringLiteral( "b" ) );
  QCOMPARE( features.at( 0 ).fields().at( 2 ).name(), QStringLiteral( "c" ) );
  QCOMPARE( features.at( 0 ).attributes().count(), 7 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), QStringLiteral( "MNL" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), QStringLiteral( "11" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), QStringLiteral( "282" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 3 ).toString(), QStringLiteral( "km" ) );
  QVERIFY( features.at( 0 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 6 ).toString().isEmpty() );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 7 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), QStringLiteral( "MNL" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "11" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "347.80000000000001" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 3 ).toString(), QStringLiteral( "km" ) );
  QVERIFY( features.at( 1 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 6 ).toString().isEmpty() );

  mQgisApp->clipboard()->setText( QStringLiteral( "wkt_geom\ta\tb\tc\nNULL\t1\tb\t2\nNULL\t3\tc3\t4\nPoint (5 4)\t2\tb2\t3" ) );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 3 );
  QCOMPARE( features.at( 0 ).fields().count(), 3 );
  QCOMPARE( features.at( 0 ).fields().at( 0 ).name(), QStringLiteral( "a" ) );
  QCOMPARE( features.at( 0 ).fields().at( 1 ).name(), QStringLiteral( "b" ) );
  QCOMPARE( features.at( 0 ).fields().at( 2 ).name(), QStringLiteral( "c" ) );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).attributes().count(), 3 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), QStringLiteral( "b" ) );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), QStringLiteral( "2" ) );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 3 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "c3" ) );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( features.at( 2 ).geometry().asWkt(), QStringLiteral( "Point (5 4)" ) );
  QCOMPARE( features.at( 2 ).attributes().count(), 3 );
  QCOMPARE( features.at( 2 ).attributes().at( 0 ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( features.at( 2 ).attributes().at( 1 ).toString(), QStringLiteral( "b2" ) );
  QCOMPARE( features.at( 2 ).attributes().at( 2 ).toString(), QStringLiteral( "3" ) );
}

void TestQgisAppClipboard::pasteGeoJson()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
  mQgisApp->clipboard()->setText( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" ) );

  QgsFeatureList features = mQgisApp->clipboard()->copyOf( fields );
  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  QgsGeometry featureGeom = features.at( 0 ).geometry();
  const QgsPoint *point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
}

void TestQgisAppClipboard::retrieveFields()
{
  //empty string
  mQgisApp->clipboard()->setText( QString() );

  QgsFields fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 0 );

  // bad string
  mQgisApp->clipboard()->setText( QStringLiteral( "asdasdas" ) );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 0 );

  // geojson string
  mQgisApp->clipboard()->setText( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}" ) );
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
  mQgisApp->clipboard()->setText( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" ) );
  QgsFields fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QgsFeatureList features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 1 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );

  //set clipboard to some QgsFeatures
  fields = QgsFields();
  fields.append( QgsField( QStringLiteral( "int_field" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "date_field" ), QVariant::Date ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( QStringLiteral( "int_field" ), 9 );
  feat.setAttribute( QStringLiteral( "date_field" ), QVariant( QDate( 2010, 9, 5 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( QStringLiteral( "int_field" ), 19 );
  feat2.setAttribute( QStringLiteral( "date_field" ), QVariant( QDate( 2011, 9, 5 ) ) );
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
  mQgisApp->clipboard()->setText( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" ) );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 1 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
}

QGSTEST_MAIN( TestQgisAppClipboard )
#include "testqgisappclipboard.moc"
