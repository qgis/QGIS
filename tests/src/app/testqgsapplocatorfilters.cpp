/***************************************************************************
     testqgsapplocatorfilters.cpp
     --------------------------
    Date                 : 2018-02-24
    Copyright            : (C) 2018 by Nyall Dawson
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
#include "qgisapp.h"
#include "qgslocatorfilter.h"
#include "qgslocator.h"
#include "qgsprintlayout.h"
#include "qgslayoutmanager.h"
//#include "qgsactionlocatorfilter.h"
#include "qgsactivelayerfeatureslocatorfilter.h"
#include "qgsalllayersfeatureslocatorfilter.h"
//#include "qgsbookmarklocatorfilter.h"
#include "qgsexpressioncalculatorlocatorfilter.h"
#include "qgsgotolocatorfilter.h"
#include "qgslayertreelocatorfilter.h"
#include "qgslayoutlocatorfilter.h"
//#include "qgsnominatimlocatorfilter.h"
//#include "qgssettingslocatorfilter.h"

#include <QSignalSpy>
#include <QClipboard>

/**
 * \ingroup UnitTests
 * This is a unit test for the field calculator
 */
class TestQgsAppLocatorFilters : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void testCalculator();
    void testLayers();
    void testLayouts();
    void testSearchActiveLayer();
    void testActiveLayerFieldRestriction();
    void testActiveLayerCompletion();
    void testSearchAllLayers();
    void testSearchAllLayersPrioritizeExactMatch();
    void testGoto();

  private:
    QgisApp *mQgisApp = nullptr;

    QList< QgsLocatorResult > gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context );
};

//runs before all tests
void TestQgsAppLocatorFilters::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsAppLocatorFilters::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAppLocatorFilters::testCalculator()
{
  QgsExpressionCalculatorLocatorFilter filter;

  // valid expression
  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "1+2" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData.toInt(), 3 );

  // trigger result
  filter.triggerResult( results.at( 0 ) );
  QCOMPARE( QApplication::clipboard()->text(), QStringLiteral( "3" ) );

  // invalid expression
  results = gatherResults( &filter, QStringLiteral( "1+" ), QgsLocatorContext() );
  QVERIFY( results.empty() );
}

void TestQgsAppLocatorFilters::testLayers()
{
  QgsVectorLayer *l1 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "aaaaa" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *l2 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "abc" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *l3 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "ccccc" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayers( QList< QgsMapLayer *>() << l1 << l2 << l3 );

  QgsLayerTreeLocatorFilter filter;

  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "xxxxx" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, QStringLiteral( "aa" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData.toString(), l1->id() );

  results = gatherResults( &filter, QStringLiteral( "A" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.at( 0 ).userData.toString(), l1->id() );
  QCOMPARE( results.at( 1 ).userData.toString(), l2->id() );

  results = gatherResults( &filter, QString(), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  QgsLocatorContext context;
  context.usingPrefix = true;
  results = gatherResults( &filter, QString(), context );
  QCOMPARE( results.count(), 3 );
  QCOMPARE( results.at( 0 ).userData.toString(), l1->id() );
  QCOMPARE( results.at( 1 ).userData.toString(), l2->id() );
  QCOMPARE( results.at( 2 ).userData.toString(), l3->id() );
}

void TestQgsAppLocatorFilters::testLayouts()
{
  QgsPrintLayout *pl1 = new QgsPrintLayout( QgsProject::instance() );
  pl1->setName( QStringLiteral( "aaaaaa" ) );
  QgsProject::instance()->layoutManager()->addLayout( pl1 );
  QgsPrintLayout *pl2 = new QgsPrintLayout( QgsProject::instance() );
  pl2->setName( QStringLiteral( "abc" ) );
  QgsProject::instance()->layoutManager()->addLayout( pl2 );
  QgsPrintLayout *pl3 = new QgsPrintLayout( QgsProject::instance() );
  pl3->setName( QStringLiteral( "ccccc" ) );
  QgsProject::instance()->layoutManager()->addLayout( pl3 );

  QgsLayoutLocatorFilter filter;

  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "xxxxx" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, QStringLiteral( "aa" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData.toString(), pl1->name() );

  results = gatherResults( &filter, QStringLiteral( "A" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.at( 0 ).userData.toString(), pl1->name() );
  QCOMPARE( results.at( 1 ).userData.toString(), pl2->name() );

  results = gatherResults( &filter, QString(), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  QgsLocatorContext context;
  context.usingPrefix = true;
  results = gatherResults( &filter, QString(), context );
  QCOMPARE( results.count(), 3 );
  QCOMPARE( results.at( 0 ).userData.toString(), pl1->name() );
  QCOMPARE( results.at( 1 ).userData.toString(), pl2->name() );
  QCOMPARE( results.at( 2 ).userData.toString(), pl3->name() );
}

void TestQgsAppLocatorFilters::testSearchActiveLayer()
{
  const QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk" );
  QgsVectorLayer *vl = new QgsVectorLayer( layerDef, QStringLiteral( "Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vl );

  QgsFeature f;
  f.setAttributes( QVector<QVariant>() << 1001 << "A nice feature" << 1234567890 << 12345.6789 );
  f.setGeometry( QgsGeometry::fromWkt( "Point (-71.123 78.23)" ) );
  vl->dataProvider()->addFeature( f );
  QgsFeature f2;
  f2.setAttributes( QVector<QVariant>() << 100 << "@home" << 13579 << 13.57 );
  f2.setGeometry( QgsGeometry::fromWkt( "Point (-71.223 78.33)" ) );
  vl->dataProvider()->addFeature( f2 );

  mQgisApp->setActiveLayer( vl );

  QgsActiveLayerFeaturesLocatorFilter filter;
  const QgsLocatorContext context;

  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "12345.6789" ), context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, QStringLiteral( "12345.67" ), context );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, QStringLiteral( "1234567890" ), context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, QStringLiteral( "nice" ), context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, QStringLiteral( "@my_text nice" ), context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, QStringLiteral( "@my_integer nice" ), context );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, QStringLiteral( "@unknown_field nice" ), context );
  QCOMPARE( results.count(), 0 );

  // check with display expression, feature should not be shown twice
  vl->setDisplayExpression( QStringLiteral( "concat(\"my_text\", ' ', \"my_double\")" ) );
  results = gatherResults( &filter, QStringLiteral( "nice" ), context );
  QCOMPARE( results.count(), 1 );
  results = gatherResults( &filter, QStringLiteral( "a feature" ), context );
  QCOMPARE( results.count(), 1 );
  results = gatherResults( &filter, QStringLiteral( "nice .678" ), context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, QStringLiteral( "@my_text @home" ), context );
  QCOMPARE( results.count(), 1 );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testActiveLayerFieldRestriction()
{
  bool isRestricting = false;

  QString search = QStringLiteral( "@my_field search" );
  QString restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, QStringLiteral( "my_field" ) );
  QCOMPARE( search, QStringLiteral( "search" ) );

  search = QStringLiteral( "@home" );
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, QStringLiteral( "home" ) );
  QCOMPARE( search, QStringLiteral( "" ) );

  search = QStringLiteral( "@" );
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, QString() );
  QCOMPARE( search, QString() );

  search = QStringLiteral( "hello there" );
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( !isRestricting );
  QVERIFY( restr.isNull() );
  QCOMPARE( search, QStringLiteral( "hello there" ) );
}

void TestQgsAppLocatorFilters::testActiveLayerCompletion()
{
  const QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk" );
  QgsVectorLayer *vl = new QgsVectorLayer( layerDef, QStringLiteral( "Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( vl );
  mQgisApp->setActiveLayer( vl );

  const QgsFeedback f;
  QgsActiveLayerFeaturesLocatorFilter filter;
  QgsLocatorContext context;
  context.usingPrefix = true;

  QCOMPARE( filter.prepare( QStringLiteral( "" ), context ), QStringList( { "@pk ", "@my_text ", "@my_integer ", "@my_double " } ) );
  QCOMPARE( filter.prepare( QStringLiteral( "@my_i" ), context ), QStringList( { "@my_integer " } ) );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testSearchAllLayers()
{
  const QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_number:integer&key=pk" );
  QgsVectorLayer *l1 = new QgsVectorLayer( layerDef, QStringLiteral( "Layer 1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *l2 = new QgsVectorLayer( layerDef, QStringLiteral( "Layer 2" ), QStringLiteral( "memory" ) );

  QgsProject::instance()->addMapLayers( QList< QgsMapLayer *>() << l1 << l2 );

  QgsFeature f1;
  f1.setAttributes( QVector<QVariant>() << 1001 << "A nice feature" << 6789 );
  f1.setGeometry( QgsGeometry::fromWkt( "Point (-71.123 78.23)" ) );
  QgsFeature f2;
  f2.setAttributes( QVector<QVariant>() << 1002 << "Something crazy" << 2 );
  f2.setGeometry( QgsGeometry::fromWkt( "Point (-72.123 78.23)" ) );
  QgsFeature f3;
  f3.setAttributes( QVector<QVariant>() << 2001 << "Another feature" << 6789 );
  f3.setGeometry( QgsGeometry::fromWkt( "Point (-73.123 78.23)" ) );

  l1->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );
  l2->dataProvider()->addFeatures( QgsFeatureList() << f3 );

  QgsAllLayersFeaturesLocatorFilter filter;
  const QgsLocatorContext context;

  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "100" ), context );

  l1->setDisplayExpression( QStringLiteral( "\"my_text\" || ' is ' || \"my_number\"" ) );
  l2->setDisplayExpression( QStringLiteral( "\"my_text\" || ' is ' || \"my_number\"" ) );

  results = gatherResults( &filter, QStringLiteral( "feature is 6789" ), context );
  QCOMPARE( results.count(), 2 );

  l2->setFlags( l2->flags() & ~QgsMapLayer::Searchable );

  results = gatherResults( &filter, QStringLiteral( "feature is 6789" ), context );
  QCOMPARE( results.count(), 1 );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testSearchAllLayersPrioritizeExactMatch()
{
  const QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_number:integer&key=pk" );
  QgsVectorLayer *l1 = new QgsVectorLayer( layerDef, QStringLiteral( "Layer 1" ), QStringLiteral( "memory" ) );

  QgsProject::instance()->addMapLayers( QList< QgsMapLayer *>() << l1 );

  QgsFeature f1;
  f1.setAttributes( QVector<QVariant>() << 100 << "A nice feature" << 100 );
  f1.setGeometry( QgsGeometry::fromWkt( "Point (-71.123 78.23)" ) );
  QgsFeature f2;
  f2.setAttributes( QVector<QVariant>() << 101 << "Something crazy" << 3 );
  f2.setGeometry( QgsGeometry::fromWkt( "Point (-72.123 78.23)" ) );
  QgsFeature f3;
  f3.setAttributes( QVector<QVariant>() << 102 << "Another feature" << 1 );
  f3.setGeometry( QgsGeometry::fromWkt( "Point (-73.123 78.23)" ) );

  l1->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 );

  QgsAllLayersFeaturesLocatorFilter filter;
  QgsLocatorContext context;
  context.usingPrefix = true; // Searching for short strings is only available with prefix

  l1->setDisplayExpression( QStringLiteral( "\"my_number\"" ) );

  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "1" ), context );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.first().displayString, QStringLiteral( "1" ) );
  QCOMPARE( results.last().displayString, QStringLiteral( "100" ) );

  QgsProject::instance()->removeAllMapLayers();
}

QList<QgsLocatorResult> TestQgsAppLocatorFilters::gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context )
{
  const QSignalSpy spy( filter, &QgsLocatorFilter::resultFetched );
  QgsFeedback f;
  filter->prepare( string, context );
  filter->fetchResults( string, context, &f );

  QList< QgsLocatorResult > results;
  for ( int i = 0; i < spy.count(); ++ i )
  {
    const QVariant v = spy.at( i ).at( 0 );
    const QgsLocatorResult result = v.value<QgsLocatorResult>();
    results.append( result );
  }
  return results;
}

void TestQgsAppLocatorFilters::testGoto()
{
  QgsGotoLocatorFilter filter;

  // simple goto
  QList< QgsLocatorResult > results = gatherResults( &filter, QStringLiteral( "4 5" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 4 5 (Map CRS, )" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 4, 5 ) );
  QCOMPARE( results.at( 1 ).displayString, QObject::tr( "Go to 4° 5° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 1 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 4, 5 ) );

  // locale-specific goto
  results = gatherResults( &filter, QStringLiteral( "1,234.56 789.012" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 1,234.56 789.012 (Map CRS, )" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 1234.56, 789.012 ) );

  // decimal degree with suffixes
  results = gatherResults( &filter, QStringLiteral( "12.345N, 67.890W" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to -67.89° 12.345° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( -67.890, 12.345 ) );

  results = gatherResults( &filter, QStringLiteral( "12.345 e, 67.890 s" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 12.345° -67.89° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 12.345, -67.890 ) );

  // degree/minuste/second coordinates goto
  // easting northing
  results = gatherResults( &filter, QStringLiteral( "40deg 1' 0\" E 11deg  55' 0\" S" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 40.01666667° -11.91666667° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 40.0166666667, -11.9166666667 ) );

  // northing easting
  results = gatherResults( &filter, QStringLiteral( "14°49′48″N 01°48′45″E" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 1.8125° 14.83° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 1.8125, 14.83 ) );

  // northing, esting (comma separated)
  results = gatherResults( &filter, QStringLiteral( "14°49′48″N, 01°48′45″E" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 1.8125° 14.83° (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 1.8125, 14.83 ) );

  // OSM/Leaflet/OpenLayers
  results = gatherResults( &filter, QStringLiteral( "https://www.openstreetmap.org/#map=15/44.5546/6.4936" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 6.4936° 44.5546° at scale 1:22569 (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 6.4936, 44.5546 ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "scale" )].toDouble(), 22569.0 );

  // Google Maps
  results = gatherResults( &filter, QStringLiteral( "https://www.google.com/maps/@44.5546,6.4936,15.25z" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 6.4936° 44.5546° at scale 1:22569 (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 6.4936, 44.5546 ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "scale" )].toDouble(), 22569.0 );

  results = gatherResults( &filter, QStringLiteral( "https://www.google.com/maps/@7.8750,81.0149,574195m/data=!3m1!1e3" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 81.0149, 7.8750 ) );

  results = gatherResults( &filter, QStringLiteral( "https://www.google.com/maps/@27.7132,85.3288,3a,75y,278.89h,90t/data=!3m8!1e1!3m6!1sAF1QipMrXuXozGc9x9bxx5uPl_3ys4H-rNVqMLr6EYLA!2e10!3e11!6shttps:%2F%2Flh5.googleusercontent.com%2Fp%2FAF1QipMrXuXozGc9x9bxx5uPl_3ys4H-rNVqMLr6EYLA%3Dw203-h100-k-no-pi2.869903-ya293.58762-ro-1.9255565-fo100!7i3840!8i1920" ), QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).displayString, QObject::tr( "Go to 85.3288° 27.7132° at scale 1:282 (EPSG:4326 - WGS 84)" ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "point" )].value<QgsPointXY>(), QgsPointXY( 85.3288, 27.7132 ) );
  QCOMPARE( results.at( 0 ).userData.toMap()[QStringLiteral( "scale" )].toDouble(), 282.0 );
}

QGSTEST_MAIN( TestQgsAppLocatorFilters )
#include "testqgsapplocatorfilters.moc"
