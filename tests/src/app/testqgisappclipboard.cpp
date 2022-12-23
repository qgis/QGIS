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
#include <QRegularExpression>

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
#include "qgsvectortilelayer.h"
#include "qgsselectioncontext.h"

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
    void copyToTextNoFields();
    void pasteWkt();
    void pasteGeoJson();
    void retrieveFields();
    void clipboardLogic(); //test clipboard logic
    void testVectorTileLayer();

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

  for ( auto it = filesCounts.constBegin(); it != filesCounts.constEnd(); it++ )
  {
    const QString fileName = it.key();

    // add vector layer
    const QString filePath = mTestDataDir + fileName;
    qDebug() << "add vector layer: " << filePath;
    QgsVectorLayer *inputLayer = mQgisApp->addVectorLayer( filePath, fileName, QStringLiteral( "ogr" ) );
    QVERIFY( inputLayer->isValid() );

    // copy all features to clipboard
    inputLayer->selectAll();
    mQgisApp->copySelectionToClipboard( inputLayer );

    const QgsFeatureList features = mQgisApp->clipboard()->copyOf();
    qDebug() << features.size() << " features copied to clipboard";

    QVERIFY( features.size() == it.value() );

    QgsVectorLayer *pastedLayer = mQgisApp->pasteAsNewMemoryVector( QStringLiteral( "pasted" ) );
    QVERIFY( pastedLayer );
    QVERIFY( pastedLayer->isValid() );
    qDebug() << pastedLayer->featureCount() << " features in pasted layer";
    QVERIFY( pastedLayer->featureCount() == it.value() );
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
  QString result, resultHtml;
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "int_field\tstring_field\n9\tval\n19\tval2" ) );

  // attributes with WKT
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkt_geom\tint_field\tstring_field\nPoint (5 6)\t9\tval\nPoint (7 8)\t19\tval2" ) );

  // HTML test
  mQgisApp->clipboard()->replaceWithCopyOf( feats );
  result = mQgisApp->clipboard()->data( "text/html" );
  QCOMPARE( result, QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>wkt_geom</td><td>int_field</td><td>string_field</td></tr><tr><td>Point (5 6)</td><td>9</td><td>val</td></tr><tr><td>Point (7 8)</td><td>19</td><td>val2</td></tr></table></body></html>" ) );

  // GeoJSON
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::GeoJSON );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  const QString expected =  "{\"features\":[{\"geometry\":{\"coordinates\":[5.0,6.0],\"type\":\"Point\"},\"id\":5,"
                            "\"properties\":{\"int_field\":9,\"string_field\":\"val\"},\"type\":\"Feature\"},"
                            "{\"geometry\":{\"coordinates\":[7.0,8.0],\"type\":\"Point\"},\"id\":6,"
                            "\"properties\":{\"int_field\":19,\"string_field\":\"val2\"},\"type\":\"Feature\"}],"
                            "\"type\":\"FeatureCollection\"}";
  QCOMPARE( result, expected );

  // test CRS is transformed correctly for GeoJSON

  const QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:3111" ) );
  feats = QgsFeatureStore();
  feats.setCrs( crs );
  feat.setGeometry( QgsGeometry( new QgsPoint( 2502577, 2403869 ) ) );
  feats.addFeature( feat );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );

  // just test coordinates as integers - that's enough to verify that reprojection has occurred
  // and helps avoid rounding issues
  const QRegularExpression regex( "\\[([-\\d.]+),([-\\d.]+)\\]" );
  const QRegularExpressionMatch  match = regex.match( result );
  const QStringList list = match.capturedTexts();
  QCOMPARE( list.count(), 3 );

  const int x = std::round( list.at( 1 ).toDouble() );
  const int y = std::round( list.at( 2 ).toDouble() );

  QCOMPARE( x, 145 );
  QCOMPARE( y, -38 );

  // test that multiline text fields are quoted to render correctly as csv files in WKT mode
  QgsFeature feat3( fields, 7 );
  feat3.setAttribute( QStringLiteral( "string_field" ), "Single line text" );
  feat3.setAttribute( QStringLiteral( "int_field" ), 1 );
  feat3.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat4( fields, 8 );
  feat4.setAttribute( QStringLiteral( "string_field" ), "Unix Multiline \nText" );
  feat4.setAttribute( QStringLiteral( "int_field" ), 2 );
  feat4.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeature feat5( fields, 9 );
  feat5.setAttribute( QStringLiteral( "string_field" ), "Windows Multiline \r\nText" );
  feat5.setAttribute( QStringLiteral( "int_field" ), 3 );
  feat5.setGeometry( QgsGeometry( new QgsPoint( 9, 10 ) ) );
  QgsFeatureStore featsML;
  featsML.addFeature( feat3 );
  featsML.addFeature( feat4 );
  featsML.addFeature( feat5 );
  featsML.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( featsML );

  // attributes only
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesOnly );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  qDebug() << result;
  QCOMPARE( result, QString( "int_field\tstring_field\n1\tSingle line text\n2\t\"Unix Multiline \nText\"\n3\t\"Windows Multiline \r\nText\"" ) );

  // attributes with WKT
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkt_geom\tint_field\tstring_field\nPoint (5 6)\t1\tSingle line text\nPoint (7 8)\t2\t\"Unix Multiline \nText\"\nPoint (9 10)\t3\t\"Windows Multiline \r\nText\"" ) );

}

void TestQgisAppClipboard::copyToTextNoFields()
{
  //set clipboard to some QgsFeatures with only geometries, no fields
  QgsFields fields;
  QgsFeature feat( fields, 5 );
  feat.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  QgsSettings settings;
  QString result, resultHtml;

  // attributes with WKT
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QStringLiteral( "Point (5 6)\nPoint (7 8)" ) );

  // HTML test
  mQgisApp->clipboard()->replaceWithCopyOf( feats );
  result = mQgisApp->clipboard()->data( "text/html" );
  QCOMPARE( result, QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>wkt_geom</td></tr><tr><td>Point (5 6)</td></tr><tr><td>Point (7 8)</td></tr></table></body></html>" ) );

  // GeoJSON
  settings.setEnumValue( QStringLiteral( "/qgis/copyFeatureFormat" ), QgsClipboard::GeoJSON );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  const QString expected =  "{\"features\":[{\"geometry\":{\"coordinates\":[5.0,6.0],\"type\":\"Point\"},\"id\":5,\"properties\":null,\"type\":\"Feature\"},{\"geometry\":{\"coordinates\":[7.0,8.0],\"type\":\"Point\"},\"id\":6,\"properties\":null,\"type\":\"Feature\"}],\"type\":\"FeatureCollection\"}";
  QCOMPARE( result, expected );
}

void TestQgisAppClipboard::pasteWkt()
{

  // test issue GH #44989
  QgsFeatureList features = mQgisApp->clipboard()->stringToFeatureList( QStringLiteral( "wkt_geom\tint_field\tstring_field\nPoint (5 6)\t1\tSingle line text\nPoint (7 8)\t2\t\"Unix Multiline \nText\"\nPoint (9 10)\t3\t\"Windows Multiline \r\nText\"" ), QgsFields() );
  QCOMPARE( features.length(), 3 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QVERIFY( features.at( 2 ).hasGeometry() && !features.at( 2 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).fields().count(), 2 );
  QCOMPARE( features.at( 0 ).attributeCount(), 2 );
  QCOMPARE( features.at( 1 ).fields().count(), 2 );
  QCOMPARE( features.at( 1 ).attributeCount(), 2 );
  QCOMPARE( features.at( 2 ).fields().count(), 2 );
  QCOMPARE( features.at( 2 ).attributeCount(), 2 );

  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (125 10)\nPOINT (111 30)" ) );

  features = mQgisApp->clipboard()->copyOf();
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
  // can change geometryType as in https://github.com/qgis/QGIS/issues/24769
  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (111 30)\t GoodFieldValue\nPOINT (125 10)\t(WrongFieldValue)" ) );

  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );

  QVERIFY( features.at( 0 ).hasGeometry() );
  QVERIFY( !features.at( 0 ).geometry().isNull() );
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
  mQgisApp->clipboard()->setText( QStringLiteral( "MNL\t11\t282\tkm\t\t\t\nMNL\t11\t347.80000000000001\tkm\t\t\t" ) );
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

  // when a set of features is built outside of QGIS, last one might be terminated by newline
  // https://github.com/qgis/QGIS/issues/33617
  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (125 10)\nPOINT (111 30)\n" ) );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  point = dynamic_cast< const QgsPoint * >( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  // on MS Windows, the <EOL> marker is CRLF
  // https://github.com/qgis/QGIS/pull/33618#discussion_r363147854
  mQgisApp->clipboard()->setText( QStringLiteral( "POINT (125 10)\r\nPOINT (111 30)\r\n" ) );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  point = dynamic_cast< const QgsPoint * >( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );
}

void TestQgisAppClipboard::pasteGeoJson()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );
  mQgisApp->clipboard()->setText( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" ) );

  const QgsFeatureList features = mQgisApp->clipboard()->copyOf( fields );

  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  const QgsGeometry featureGeom = features.at( 0 ).geometry();
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
  const QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:4326" ) );
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

void TestQgisAppClipboard::testVectorTileLayer()
{
  QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  dataDir += "/vector_tile";

  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( dataDir ) );
  ds.setParam( "zmax", "1" );
  std::unique_ptr< QgsVectorTileLayer > layer = std::make_unique< QgsVectorTileLayer >( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  QgsGeometry selectionGeometry = QgsGeometry::fromWkt( QStringLiteral( "Polygon ((13934091.75684908032417297 -1102962.40819426625967026, 11360512.80439674854278564 -2500048.12523981928825378, 12316413.55816475301980972 -5661873.69539554417133331, 16948855.67257896065711975 -6617774.44916355609893799, 18125348.90798573195934296 -2058863.16196227818727493, 15257646.64668171107769012 -735308.27212964743375778, 13934091.75684908032417297 -1102962.40819426625967026))" ) );
  QgsSelectionContext context;
  context.setScale( 315220096 );
  layer->selectByGeometry( selectionGeometry, context, Qgis::SelectBehavior::SetSelection, Qgis::SelectGeometryRelationship::Intersect );

  QCOMPARE( layer->selectedFeatureCount(), 4 );
  const QList< QgsFeature > features = layer->selectedFeatures();

  mQgisApp->clipboard()->replaceWithCopyOf( layer.get() );

  // test that clipboard features are a "superset" of the incoming fields
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( QStringLiteral( "disputed" ) ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( QStringLiteral( "maritime" ) ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( QStringLiteral( "admin_level" ) ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( QStringLiteral( "class" ) ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( QStringLiteral( "name:th" ) ) > -1 );

  QgsFeatureId maritimeId = -1;
  QgsFeatureId oceanId = -1;
  for ( const QgsFeature &feature :  features )
  {
    if ( feature.fields().lookupField( QStringLiteral( "maritime" ) ) > -1 )
      maritimeId = feature.id();
    else if ( feature.attribute( QStringLiteral( "class" ) ).toString() == QLatin1String( "ocean" ) )
      oceanId = feature.id();
  }

  const QgsFeatureList clipboardFeatures = mQgisApp->clipboard()->copyOf();
  QCOMPARE( clipboardFeatures.size(), 4 );

  QgsFeature maritimeFeature;
  QgsFeature oceanFeature;
  for ( const QgsFeature &feature : clipboardFeatures )
  {
    if ( feature.id() == maritimeId )
      maritimeFeature = feature;
    else if ( feature.id() == oceanId )
      oceanFeature = feature;
  }
  QVERIFY( maritimeFeature.isValid() );
  QVERIFY( oceanFeature.isValid() );

  // ensure that clipboard features are the superset of incoming fields, and that features have consistent fields with this superset
  QCOMPARE( maritimeFeature.fields(), mQgisApp->clipboard()->fields() );
  QCOMPARE( maritimeFeature.attribute( QStringLiteral( "maritime" ) ).toString(), QStringLiteral( "0" ) );
  QCOMPARE( oceanFeature.fields(), mQgisApp->clipboard()->fields() );
  QCOMPARE( oceanFeature.attribute( QStringLiteral( "class" ) ).toString(), QStringLiteral( "ocean" ) );
}

QGSTEST_MAIN( TestQgisAppClipboard )
#include "testqgisappclipboard.moc"
