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
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsclipboard.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgsselectioncontext.h"
#include "qgssettings.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"

#include <QApplication>
#include <QObject>
#include <QRegularExpression>
#include <QSplashScreen>
#include <QString>
#include <QStringList>

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
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void copyPaste();
    void copyToText();
    void copyToTextNoFields();
    void pasteWkt();
    void pasteGeoJson();
    void retrieveFields();
    void clipboardLogic(); //test clipboard logic
    void testVectorTileLayer();
    void copyPasteUnset();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mTestDataDir;
};

TestQgisAppClipboard::TestQgisAppClipboard() = default;

//runs before all tests
void TestQgisAppClipboard::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
  filesCounts.insert( u"points.shp"_s, 17 );
  filesCounts.insert( u"lines.shp"_s, 6 );
  filesCounts.insert( u"polys.shp"_s, 10 );

  for ( auto it = filesCounts.constBegin(); it != filesCounts.constEnd(); it++ )
  {
    const QString fileName = it.key();

    // add vector layer
    const QString filePath = mTestDataDir + fileName;
    qDebug() << "add vector layer: " << filePath;
    QgsVectorLayer *inputLayer = mQgisApp->addVectorLayer( filePath, fileName, u"ogr"_s );
    QVERIFY( inputLayer->isValid() );

    // copy all features to clipboard
    inputLayer->selectAll();
    mQgisApp->copySelectionToClipboard( inputLayer );

    const QgsFeatureList features = mQgisApp->clipboard()->copyOf();
    qDebug() << features.size() << " features copied to clipboard";

    QVERIFY( features.size() == it.value() );

    QgsVectorLayer *pastedLayer = mQgisApp->pasteAsNewMemoryVector( u"pasted"_s );
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
  fields.append( QgsField( u"int_field"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"double_field"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"string_field"_s, QMetaType::Type::QString ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( u"int_field"_s, 9 );
  feat.setAttribute( u"double_field"_s, 9.9 );
  feat.setAttribute( u"string_field"_s, "val" );
  feat.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( u"int_field"_s, 19 );
  feat2.setAttribute( u"double_field"_s, 19.19 );
  feat2.setAttribute( u"string_field"_s, "val2" );
  feat2.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeature feat3( fields, 7 ); // NULL field values
  feat3.setAttribute( u"int_field"_s, QVariant( QVariant::Int ) );
  feat3.setAttribute( u"double_field"_s, QVariant( QVariant::Double ) );
  feat3.setAttribute( u"string_field"_s, QVariant( QVariant::String ) );
  feat3.setGeometry( QgsGeometry( new QgsPoint( 9, 10 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.addFeature( feat3 );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  // attributes only
  QgsSettings settings;
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesOnly );
  QString result, resultHtml;
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "int_field\tdouble_field\tstring_field\n9\t9.9\tval\n19\t19.19\tval2\n\t\t" ) );

  // attributes with WKT
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkt_geom\tint_field\tdouble_field\tstring_field\nPoint (5 6)\t9\t9.9\tval\nPoint (7 8)\t19\t19.19\tval2\nPoint (9 10)\t\t\t" ) );

  // attributes with WKB
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKB );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkb_geom\tint_field\tdouble_field\tstring_field\n010100000000000000000014400000000000001840\t9\t9.9\tval\n01010000000000000000001c400000000000002040\t19\t19.19\tval2\n010100000000000000000022400000000000002440\t\t\t" ) );

  // HTML test
  mQgisApp->clipboard()->replaceWithCopyOf( feats );
  result = mQgisApp->clipboard()->data( "text/html" );
  QCOMPARE( result, QString( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>wkb_geom</td><td>int_field</td><td>double_field</td><td>string_field</td></tr><tr><td>010100000000000000000014400000000000001840</td><td>9</td><td>9.9</td><td>val</td></tr><tr><td>01010000000000000000001c400000000000002040</td><td>19</td><td>19.19</td><td>val2</td></tr><tr><td>010100000000000000000022400000000000002440</td><td></td><td></td><td></td></tr></table></body></html>" ) );

  // GeoJSON
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::GeoJSON );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  const QString expected = "{\"features\":[{\"geometry\":{\"coordinates\":[5.0,6.0],\"type\":\"Point\"},\"id\":5,"
                           "\"properties\":{\"double_field\":9.9,\"int_field\":9,\"string_field\":\"val\"},\"type\":\"Feature\"},"
                           "{\"geometry\":{\"coordinates\":[7.0,8.0],\"type\":\"Point\"},\"id\":6,"
                           "\"properties\":{\"double_field\":19.19,\"int_field\":19,\"string_field\":\"val2\"},\"type\":\"Feature\"},"
                           "{\"geometry\":{\"coordinates\":[9.0,10.0],\"type\":\"Point\"},\"id\":7,"
                           "\"properties\":{\"double_field\":null,\"int_field\":null,\"string_field\":null},\"type\":\"Feature\"}],"
                           "\"type\":\"FeatureCollection\"}";
  QCOMPARE( result, expected );

  // test CRS is transformed correctly for GeoJSON

  const QgsCoordinateReferenceSystem crs( u"EPSG:3111"_s );
  feats = QgsFeatureStore();
  feats.setCrs( crs );
  feat.setGeometry( QgsGeometry( new QgsPoint( 2502577, 2403869 ) ) );
  feats.addFeature( feat );
  feats.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );

  // just test coordinates as integers - that's enough to verify that reprojection has occurred
  // and helps avoid rounding issues
  const thread_local QRegularExpression regex( "\\[([-\\d.]+),([-\\d.]+)\\]" );
  const QRegularExpressionMatch match = regex.match( result );
  const QStringList list = match.capturedTexts();
  QCOMPARE( list.count(), 3 );

  const int x = std::round( list.at( 1 ).toDouble() );
  const int y = std::round( list.at( 2 ).toDouble() );

  QCOMPARE( x, 145 );
  QCOMPARE( y, -38 );

  // test that multiline text fields are quoted to render correctly as csv files in WKT mode
  QgsFeature feat4( fields, 7 );
  feat4.setAttribute( u"string_field"_s, "Single line text" );
  feat4.setAttribute( u"int_field"_s, 1 );
  feat4.setAttribute( u"double_field"_s, 1.1 );
  feat4.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat5( fields, 8 );
  feat5.setAttribute( u"string_field"_s, "Unix Multiline \nText" );
  feat5.setAttribute( u"int_field"_s, 2 );
  feat5.setAttribute( u"double_field"_s, 2.2 );
  feat5.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeature feat6( fields, 9 );
  feat6.setAttribute( u"string_field"_s, "Windows Multiline \r\nText" );
  feat6.setAttribute( u"int_field"_s, 3 );
  feat6.setAttribute( u"double_field"_s, 3.3 );
  feat6.setGeometry( QgsGeometry( new QgsPoint( 9, 10 ) ) );
  QgsFeatureStore featsML;
  featsML.addFeature( feat4 );
  featsML.addFeature( feat5 );
  featsML.addFeature( feat6 );
  featsML.setFields( fields );
  mQgisApp->clipboard()->replaceWithCopyOf( featsML );

  // attributes only
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesOnly );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  qDebug() << result;
  QCOMPARE( result, QString( "int_field\tdouble_field\tstring_field\n1\t1.1\tSingle line text\n2\t2.2\t\"Unix Multiline \nText\"\n3\t3.3\t\"Windows Multiline \r\nText\"" ) );

  // attributes with WKT
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkt_geom\tint_field\tdouble_field\tstring_field\nPoint (5 6)\t1\t1.1\tSingle line text\nPoint (7 8)\t2\t2.2\t\"Unix Multiline \nText\"\nPoint (9 10)\t3\t3.3\t\"Windows Multiline \r\nText\"" ) );

  // attributes with WKB
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKB );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, QString( "wkb_geom\tint_field\tdouble_field\tstring_field\n010100000000000000000014400000000000001840\t1\t1.1\tSingle line text\n01010000000000000000001c400000000000002040\t2\t2.2\t\"Unix Multiline \nText\"\n010100000000000000000022400000000000002440\t3\t3.3\t\"Windows Multiline \r\nText\"" ) );
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
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKT );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, u"Point (5 6)\nPoint (7 8)"_s );

  // attributes with WKB
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKB );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  QCOMPARE( result, u"010100000000000000000014400000000000001840\n01010000000000000000001c400000000000002040"_s );

  // HTML test
  mQgisApp->clipboard()->replaceWithCopyOf( feats );
  result = mQgisApp->clipboard()->data( "text/html" );
  QCOMPARE( result, u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>wkb_geom</td></tr><tr><td>010100000000000000000014400000000000001840</td></tr><tr><td>01010000000000000000001c400000000000002040</td></tr></table></body></html>"_s );

  // GeoJSON
  settings.setEnumValue( u"/qgis/copyFeatureFormat"_s, QgsClipboard::GeoJSON );
  mQgisApp->clipboard()->generateClipboardText( result, resultHtml );
  const QString expected = "{\"features\":[{\"geometry\":{\"coordinates\":[5.0,6.0],\"type\":\"Point\"},\"id\":5,\"properties\":null,\"type\":\"Feature\"},{\"geometry\":{\"coordinates\":[7.0,8.0],\"type\":\"Point\"},\"id\":6,\"properties\":null,\"type\":\"Feature\"}],\"type\":\"FeatureCollection\"}";
  QCOMPARE( result, expected );
}

void TestQgisAppClipboard::pasteWkt()
{
  // test issue GH #44989
  QgsFeatureList features = mQgisApp->clipboard()->stringToFeatureList( u"wkt_geom\tint_field\tstring_field\nPoint (5 6)\t1\tSingle line text\nPoint (7 8)\t2\t\"Unix Multiline \nText\"\nPoint (9 10)\t3\t\"Windows Multiline \r\nText\""_s, QgsFields() );
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

  mQgisApp->clipboard()->setText( u"POINT (125 10)\nPOINT (111 30)"_s );

  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  QgsGeometry featureGeom = features.at( 0 ).geometry();
  const QgsPoint *point = dynamic_cast<const QgsPoint *>( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  point = dynamic_cast<const QgsPoint *>( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  // be sure parsing does not consider attached parameters that
  // can change geometryType as in https://github.com/qgis/QGIS/issues/24769
  mQgisApp->clipboard()->setText( u"POINT (111 30)\t GoodFieldValue\nPOINT (125 10)\t(WrongFieldValue)"_s );

  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );

  QVERIFY( features.at( 0 ).hasGeometry() );
  QVERIFY( !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast<const QgsPoint *>( featureGeom.constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  point = dynamic_cast<const QgsPoint *>( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );

  //clipboard should support features without geometry
  mQgisApp->clipboard()->setText( u"MNL\t11\t282\tkm\t\t\t\nMNL\t11\t347.80000000000001\tkm\t\t\t"_s );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).attributes().count(), 7 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), u"MNL"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), u"11"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), u"282"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 3 ).toString(), u"km"_s );
  QVERIFY( features.at( 0 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 6 ).toString().isEmpty() );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 7 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), u"MNL"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), u"11"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), u"347.80000000000001"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 3 ).toString(), u"km"_s );
  QVERIFY( features.at( 1 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 6 ).toString().isEmpty() );

  mQgisApp->clipboard()->setText( u"wkt_geom\ta\tb\tc\n\tMNL\t11\t282\tkm\t\t\t\n\tMNL\t11\t347.80000000000001\tkm\t\t\t"_s );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).fields().count(), 3 );
  QCOMPARE( features.at( 0 ).fields().at( 0 ).name(), u"a"_s );
  QCOMPARE( features.at( 0 ).fields().at( 1 ).name(), u"b"_s );
  QCOMPARE( features.at( 0 ).fields().at( 2 ).name(), u"c"_s );
  QCOMPARE( features.at( 0 ).attributes().count(), 7 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), u"MNL"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), u"11"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), u"282"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 3 ).toString(), u"km"_s );
  QVERIFY( features.at( 0 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 0 ).attributes().at( 6 ).toString().isEmpty() );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 7 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), u"MNL"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), u"11"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), u"347.80000000000001"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 3 ).toString(), u"km"_s );
  QVERIFY( features.at( 1 ).attributes().at( 4 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 5 ).toString().isEmpty() );
  QVERIFY( features.at( 1 ).attributes().at( 6 ).toString().isEmpty() );

  mQgisApp->clipboard()->setText( u"wkt_geom\ta\tb\tc\nNULL\t1\tb\t2\nNULL\t3\tc3\t4\nPoint (5 4)\t2\tb2\t3"_s );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 3 );
  QCOMPARE( features.at( 0 ).fields().count(), 3 );
  QCOMPARE( features.at( 0 ).fields().at( 0 ).name(), u"a"_s );
  QCOMPARE( features.at( 0 ).fields().at( 1 ).name(), u"b"_s );
  QCOMPARE( features.at( 0 ).fields().at( 2 ).name(), u"c"_s );
  QVERIFY( !features.at( 0 ).hasGeometry() );
  QCOMPARE( features.at( 0 ).attributes().count(), 3 );
  QCOMPARE( features.at( 0 ).attributes().at( 0 ).toString(), u"1"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 1 ).toString(), u"b"_s );
  QCOMPARE( features.at( 0 ).attributes().at( 2 ).toString(), u"2"_s );
  QVERIFY( !features.at( 1 ).hasGeometry() );
  QCOMPARE( features.at( 1 ).attributes().count(), 3 );
  QCOMPARE( features.at( 1 ).attributes().at( 0 ).toString(), u"3"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 1 ).toString(), u"c3"_s );
  QCOMPARE( features.at( 1 ).attributes().at( 2 ).toString(), u"4"_s );
  QCOMPARE( features.at( 2 ).geometry().asWkt(), u"Point (5 4)"_s );
  QCOMPARE( features.at( 2 ).attributes().count(), 3 );
  QCOMPARE( features.at( 2 ).attributes().at( 0 ).toString(), u"2"_s );
  QCOMPARE( features.at( 2 ).attributes().at( 1 ).toString(), u"b2"_s );
  QCOMPARE( features.at( 2 ).attributes().at( 2 ).toString(), u"3"_s );

  // when a set of features is built outside of QGIS, last one might be terminated by newline
  // https://github.com/qgis/QGIS/issues/33617
  mQgisApp->clipboard()->setText( u"POINT (125 10)\nPOINT (111 30)\n"_s );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast<const QgsPoint *>( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  point = dynamic_cast<const QgsPoint *>( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );

  // on MS Windows, the <EOL> marker is CRLF
  // https://github.com/qgis/QGIS/pull/33618#discussion_r363147854
  mQgisApp->clipboard()->setText( u"POINT (125 10)\r\nPOINT (111 30)\r\n"_s );
  features = mQgisApp->clipboard()->copyOf();
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast<const QgsPoint *>( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  point = dynamic_cast<const QgsPoint *>( features.at( 1 ).geometry().constGet() );
  QCOMPARE( point->x(), 111.0 );
  QCOMPARE( point->y(), 30.0 );
}

void TestQgisAppClipboard::pasteGeoJson()
{
  QgsFields fields;
  fields.append( QgsField( u"name"_s, QMetaType::Type::QString ) );
  mQgisApp->clipboard()->setText( u"{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}"_s );

  const QgsFeatureList features = mQgisApp->clipboard()->copyOf( fields );

  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), Qgis::WkbType::Point );
  const QgsGeometry featureGeom = features.at( 0 ).geometry();
  const QgsPoint *point = dynamic_cast<const QgsPoint *>( featureGeom.constGet() );
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
  mQgisApp->clipboard()->setText( u"asdasdas"_s );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 0 );

  // geojson string
  mQgisApp->clipboard()->setText( u"{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}"_s );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QCOMPARE( fields.at( 1 ).name(), QString( "height" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Double );
}

void TestQgisAppClipboard::clipboardLogic()
{
  //start by setting clipboard contents as text
  mQgisApp->clipboard()->setText( u"{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}"_s );
  QgsFields fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
  QgsFeatureList features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 1 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );

  //set clipboard to some QgsFeatures
  fields = QgsFields();
  fields.append( QgsField( u"int_field"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"date_field"_s, QMetaType::Type::QDate ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( u"int_field"_s, 9 );
  feat.setAttribute( u"date_field"_s, QVariant( QDate( 2010, 9, 5 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( u"int_field"_s, 19 );
  feat2.setAttribute( u"date_field"_s, QVariant( QDate( 2011, 9, 5 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.setFields( fields );
  const QgsCoordinateReferenceSystem crs( u"EPSG:4326"_s );
  feats.setCrs( crs );
  mQgisApp->clipboard()->replaceWithCopyOf( feats );

  //test result
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "int_field" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.at( 1 ).name(), QString( "date_field" ) );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::QDate );
  features = mQgisApp->clipboard()->copyOf( mQgisApp->clipboard()->fields() );
  QCOMPARE( features.length(), 2 );
  QCOMPARE( features.at( 0 ).id(), 5LL );
  QCOMPARE( features.at( 0 ).attribute( "int_field" ).toInt(), 9 );
  QCOMPARE( features.at( 0 ).attribute( "date_field" ).toDate(), QDate( 2010, 9, 5 ) );
  QCOMPARE( features.at( 1 ).id(), 6LL );
  QCOMPARE( features.at( 1 ).attribute( "int_field" ).toInt(), 19 );
  QCOMPARE( features.at( 1 ).attribute( "date_field" ).toDate(), QDate( 2011, 9, 5 ) );

  //replace with text again, make sure system clipboard is used rather than internal clipboard
  mQgisApp->clipboard()->setText( u"{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}"_s );
  fields = mQgisApp->clipboard()->fields();
  QCOMPARE( fields.count(), 1 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::QString );
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
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  QgsGeometry selectionGeometry = QgsGeometry::fromWkt( u"Polygon ((13934091.75684908032417297 -1102962.40819426625967026, 11360512.80439674854278564 -2500048.12523981928825378, 12316413.55816475301980972 -5661873.69539554417133331, 16948855.67257896065711975 -6617774.44916355609893799, 18125348.90798573195934296 -2058863.16196227818727493, 15257646.64668171107769012 -735308.27212964743375778, 13934091.75684908032417297 -1102962.40819426625967026))"_s );
  QgsSelectionContext context;
  context.setScale( 315220096 );
  layer->selectByGeometry( selectionGeometry, context, Qgis::SelectBehavior::SetSelection, Qgis::SelectGeometryRelationship::Intersect );

  QCOMPARE( layer->selectedFeatureCount(), 4 );
  const QList<QgsFeature> features = layer->selectedFeatures();

  mQgisApp->clipboard()->replaceWithCopyOf( layer.get() );

  // test that clipboard features are a "superset" of the incoming fields
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( u"disputed"_s ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( u"maritime"_s ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( u"admin_level"_s ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( u"class"_s ) > -1 );
  QVERIFY( mQgisApp->clipboard()->fields().lookupField( u"name:th"_s ) > -1 );

  QgsFeatureId maritimeId = -1;
  QgsFeatureId oceanId = -1;
  for ( const QgsFeature &feature : features )
  {
    if ( feature.fields().lookupField( u"maritime"_s ) > -1 )
      maritimeId = feature.id();
    else if ( feature.attribute( u"class"_s ).toString() == "ocean"_L1 )
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
  QCOMPARE( maritimeFeature.attribute( u"maritime"_s ).toString(), u"0"_s );
  QCOMPARE( oceanFeature.fields(), mQgisApp->clipboard()->fields() );
  QCOMPARE( oceanFeature.attribute( u"class"_s ).toString(), u"ocean"_s );
}

void TestQgisAppClipboard::copyPasteUnset()
{
  QgsClipboard clipboard;
  clipboard.clear();

  QgsFields fields;
  fields.append( QgsField( u"int_field"_s, QMetaType::Type::Int ) );
  fields.append( QgsField( u"double_field"_s, QMetaType::Type::Double ) );
  fields.append( QgsField( u"string_field"_s, QMetaType::Type::QString ) );
  QgsFeature feat( fields, 5 );
  feat.setAttribute( u"int_field"_s, 9 );
  feat.setAttribute( u"double_field"_s, 9.9 );
  feat.setAttribute( u"string_field"_s, "val" );
  feat.setGeometry( QgsGeometry( new QgsPoint( 5, 6 ) ) );
  QgsFeature feat2( fields, 6 );
  feat2.setAttribute( u"int_field"_s, 19 );
  feat2.setAttribute( u"double_field"_s, 19.19 );
  feat2.setAttribute( u"string_field"_s, "val2" );
  feat2.setGeometry( QgsGeometry( new QgsPoint( 7, 8 ) ) );
  QgsFeature feat3( fields, 7 ); // NULL field values
  feat3.setAttribute( u"int_field"_s, QVariant( QVariant::Int ) );
  feat3.setAttribute( u"double_field"_s, QVariant( QVariant::Double ) );
  feat3.setAttribute( u"string_field"_s, QVariant( QVariant::String ) );
  feat3.setGeometry( QgsGeometry( new QgsPoint( 9, 10 ) ) );
  QgsFeature feat4( fields, 8 ); // unset field values
  feat4.setAttribute( u"int_field"_s, QVariant::fromValue( QgsUnsetAttributeValue( "Autonumber" ) ) );
  feat4.setAttribute( u"double_field"_s, QVariant::fromValue( QgsUnsetAttributeValue( "Autonumber" ) ) );
  feat4.setAttribute( u"string_field"_s, QVariant::fromValue( QgsUnsetAttributeValue( "Some series" ) ) );
  feat4.setGeometry( QgsGeometry( new QgsPoint( 9, 10 ) ) );
  QgsFeatureStore feats;
  feats.addFeature( feat );
  feats.addFeature( feat2 );
  feats.addFeature( feat3 );
  feats.addFeature( feat4 );
  feats.setFields( fields );
  clipboard.replaceWithCopyOf( feats );

  std::unique_ptr< QgsVectorLayer > layer = clipboard.pasteToNewMemoryVector( nullptr );
  QCOMPARE( layer->featureCount(), 4LL );
  QgsFeatureIterator it = layer->getFeatures();
  QgsFeature f;
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toInt(), 9 );
  QCOMPARE( f.attribute( 1 ).toDouble(), 9.9 );
  QCOMPARE( f.attribute( 2 ).toString(), u"val"_s );
  QVERIFY( it.nextFeature( f ) );
  QCOMPARE( f.attribute( 0 ).toInt(), 19 );
  QCOMPARE( f.attribute( 1 ).toDouble(), 19.19 );
  QCOMPARE( f.attribute( 2 ).toString(), u"val2"_s );
  QVERIFY( it.nextFeature( f ) );
  QVERIFY( QgsVariantUtils::isNull( f.attribute( 0 ) ) );
  QVERIFY( QgsVariantUtils::isNull( f.attribute( 1 ) ) );
  QVERIFY( QgsVariantUtils::isNull( f.attribute( 2 ) ) );
  QVERIFY( it.nextFeature( f ) );
  QVERIFY( f.isUnsetValue( 0 ) );
  QVERIFY( f.isUnsetValue( 1 ) );
  QVERIFY( f.isUnsetValue( 2 ) );
}

QGSTEST_MAIN( TestQgisAppClipboard )
#include "testqgisappclipboard.moc"
