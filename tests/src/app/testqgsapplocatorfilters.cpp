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
#include "qgisapp.h"
#include "qgslayoutmanager.h"
#include "qgslocator.h"
#include "qgslocatorfilter.h"
#include "qgsprintlayout.h"
#include "qgstest.h"

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


struct Result
{
    Result( QString displayString, const QgsPointXY &point, double scale = 0 )
      : displayString( displayString )
      , point( point )
      , scale( scale )
    {}

    QString displayString;
    QgsPointXY point;
    double scale = 0;
};

typedef QList<Result> Results;

Q_DECLARE_METATYPE( Results )

/**
 * \ingroup UnitTests
 * This is a unit test for the field calculator
 */
class TestQgsAppLocatorFilters : public QObject
{
    Q_OBJECT

  public:
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void testCalculator();
    void testLayers();
    void testLayouts();
    void testSearchActiveLayer();
    void testActiveLayerFieldRestriction();
    void testActiveLayerCompletion();
    void testSearchAllLayers();
    void testSearchAllLayersPrioritizeExactMatch();
    void testGoto();
    void testGoto_data();

  private:
    QgisApp *mQgisApp = nullptr;

    QList<QgsLocatorResult> gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context );
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
  QList<QgsLocatorResult> results = gatherResults( &filter, u"1+2"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData().toInt(), 3 );

  // trigger result
  filter.triggerResult( results.at( 0 ) );
  QCOMPARE( QApplication::clipboard()->text(), u"3"_s );

  // invalid expression
  results = gatherResults( &filter, u"1+"_s, QgsLocatorContext() );
  QVERIFY( results.empty() );
}

void TestQgsAppLocatorFilters::testLayers()
{
  QgsVectorLayer *l1 = new QgsVectorLayer( u"Point"_s, u"aaaaa"_s, u"memory"_s );
  QgsVectorLayer *l2 = new QgsVectorLayer( u"Point"_s, u"abc"_s, u"memory"_s );
  QgsVectorLayer *l3 = new QgsVectorLayer( u"Point"_s, u"ccccc"_s, u"memory"_s );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << l1 << l2 << l3 );

  QgsLayerTreeLocatorFilter filter;

  QList<QgsLocatorResult> results = gatherResults( &filter, u"xxxxx"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, u"aa"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData().toString(), l1->id() );

  results = gatherResults( &filter, u"A"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.at( 0 ).userData().toString(), l1->id() );
  QCOMPARE( results.at( 1 ).userData().toString(), l2->id() );

  results = gatherResults( &filter, QString(), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  QgsLocatorContext context;
  context.usingPrefix = true;
  results = gatherResults( &filter, QString(), context );
  QCOMPARE( results.count(), 3 );
  QCOMPARE( results.at( 0 ).userData().toString(), l1->id() );
  QCOMPARE( results.at( 1 ).userData().toString(), l2->id() );
  QCOMPARE( results.at( 2 ).userData().toString(), l3->id() );
}

void TestQgsAppLocatorFilters::testLayouts()
{
  QgsPrintLayout *pl1 = new QgsPrintLayout( QgsProject::instance() );
  pl1->setName( u"aaaaaa"_s );
  QgsProject::instance()->layoutManager()->addLayout( pl1 );
  QgsPrintLayout *pl2 = new QgsPrintLayout( QgsProject::instance() );
  pl2->setName( u"abc"_s );
  QgsProject::instance()->layoutManager()->addLayout( pl2 );
  QgsPrintLayout *pl3 = new QgsPrintLayout( QgsProject::instance() );
  pl3->setName( u"ccccc"_s );
  QgsProject::instance()->layoutManager()->addLayout( pl3 );

  QgsLayoutLocatorFilter filter;

  QList<QgsLocatorResult> results = gatherResults( &filter, u"xxxxx"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, u"aa"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 1 );
  QCOMPARE( results.at( 0 ).userData().toString(), pl1->name() );

  results = gatherResults( &filter, u"A"_s, QgsLocatorContext() );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.at( 0 ).userData().toString(), pl1->name() );
  QCOMPARE( results.at( 1 ).userData().toString(), pl2->name() );

  results = gatherResults( &filter, QString(), QgsLocatorContext() );
  QCOMPARE( results.count(), 0 );

  QgsLocatorContext context;
  context.usingPrefix = true;
  results = gatherResults( &filter, QString(), context );
  QCOMPARE( results.count(), 3 );
  QCOMPARE( results.at( 0 ).userData().toString(), pl1->name() );
  QCOMPARE( results.at( 1 ).userData().toString(), pl2->name() );
  QCOMPARE( results.at( 2 ).userData().toString(), pl3->name() );
}

void TestQgsAppLocatorFilters::testSearchActiveLayer()
{
  const QString layerDef = u"Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk"_s;
  QgsVectorLayer *vl = new QgsVectorLayer( layerDef, u"Layer"_s, u"memory"_s );
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

  QList<QgsLocatorResult> results = gatherResults( &filter, u"12345.6789"_s, context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, u"12345.67"_s, context );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, u"1234567890"_s, context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, u"nice"_s, context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, u"@my_text nice"_s, context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, u"@my_integer nice"_s, context );
  QCOMPARE( results.count(), 0 );

  results = gatherResults( &filter, u"@unknown_field nice"_s, context );
  QCOMPARE( results.count(), 0 );

  // check with display expression, feature should not be shown twice
  vl->setDisplayExpression( u"concat(\"my_text\", ' ', \"my_double\")"_s );
  results = gatherResults( &filter, u"nice"_s, context );
  QCOMPARE( results.count(), 1 );
  results = gatherResults( &filter, u"a feature"_s, context );
  QCOMPARE( results.count(), 1 );
  results = gatherResults( &filter, u"nice .678"_s, context );
  QCOMPARE( results.count(), 1 );

  results = gatherResults( &filter, u"@my_text @home"_s, context );
  QCOMPARE( results.count(), 1 );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testActiveLayerFieldRestriction()
{
  bool isRestricting = false;

  QString search = u"@my_field search"_s;
  QString restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, u"my_field"_s );
  QCOMPARE( search, u"search"_s );

  search = u"@home"_s;
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, u"home"_s );
  QCOMPARE( search, QString() );

  search = u"@"_s;
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( isRestricting );
  QCOMPARE( restr, QString() );
  QCOMPARE( search, QString() );

  search = u"hello there"_s;
  restr = QgsActiveLayerFeaturesLocatorFilter::fieldRestriction( search, &isRestricting );
  QVERIFY( !isRestricting );
  QVERIFY( restr.isNull() );
  QCOMPARE( search, u"hello there"_s );
}

void TestQgsAppLocatorFilters::testActiveLayerCompletion()
{
  const QString layerDef = u"Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk"_s;
  QgsVectorLayer *vl = new QgsVectorLayer( layerDef, u"Layer"_s, u"memory"_s );
  QgsProject::instance()->addMapLayer( vl );
  mQgisApp->setActiveLayer( vl );

  const QgsFeedback f;
  QgsActiveLayerFeaturesLocatorFilter filter;
  QgsLocatorContext context;
  context.usingPrefix = true;

  QCOMPARE( filter.prepare( QString(), context ), QStringList( { "@pk ", "@my_text ", "@my_integer ", "@my_double " } ) );
  QCOMPARE( filter.prepare( u"@my_i"_s, context ), QStringList( { "@my_integer " } ) );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testSearchAllLayers()
{
  const QString layerDef = u"Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_number:integer&key=pk"_s;
  QgsVectorLayer *l1 = new QgsVectorLayer( layerDef, u"Layer 1"_s, u"memory"_s );
  QgsVectorLayer *l2 = new QgsVectorLayer( layerDef, u"Layer 2"_s, u"memory"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << l1 << l2 );

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

  QList<QgsLocatorResult> results = gatherResults( &filter, u"100"_s, context );

  l1->setDisplayExpression( u"\"my_text\" || ' is ' || \"my_number\""_s );
  l2->setDisplayExpression( u"\"my_text\" || ' is ' || \"my_number\""_s );

  results = gatherResults( &filter, u"feature is 6789"_s, context );
  QCOMPARE( results.count(), 2 );

  l2->setFlags( l2->flags() & ~QgsMapLayer::Searchable );

  results = gatherResults( &filter, u"feature is 6789"_s, context );
  QCOMPARE( results.count(), 1 );

  QgsProject::instance()->removeAllMapLayers();
}

void TestQgsAppLocatorFilters::testSearchAllLayersPrioritizeExactMatch()
{
  const QString layerDef = u"Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_number:integer&key=pk"_s;
  QgsVectorLayer *l1 = new QgsVectorLayer( layerDef, u"Layer 1"_s, u"memory"_s );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << l1 );

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

  l1->setDisplayExpression( u"\"my_number\""_s );

  QList<QgsLocatorResult> results = gatherResults( &filter, u"1"_s, context );
  QCOMPARE( results.count(), 2 );
  QCOMPARE( results.first().displayString, u"1"_s );
  QCOMPARE( results.last().displayString, u"100"_s );

  QgsProject::instance()->removeAllMapLayers();
}

QList<QgsLocatorResult> TestQgsAppLocatorFilters::gatherResults( QgsLocatorFilter *filter, const QString &string, const QgsLocatorContext &context )
{
  const QSignalSpy spy( filter, &QgsLocatorFilter::resultFetched );
  QgsFeedback f;
  filter->prepare( string, context );
  filter->fetchResults( string, context, &f );

  QList<QgsLocatorResult> results;
  for ( int i = 0; i < spy.count(); ++i )
  {
    const QVariant v = spy.at( i ).at( 0 );
    const QgsLocatorResult result = v.value<QgsLocatorResult>();
    results.append( result );
  }
  return results;
}

void TestQgsAppLocatorFilters::testGoto_data()
{
  qRegisterMetaType<Results>( "Results" );

  QTest::addColumn<QString>( "string" );
  QTest::addColumn<Results>( "expected" );

  QTest::newRow( "simple" ) << u"4 5"_s << Results( { { QObject::tr( "Go to 4 5 (Map CRS, )" ), QgsPointXY( 4, 5 ) }, { QObject::tr( "Go to 4°N 5°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 5, 4 ) } } );

  QTest::newRow( "locale" ) << u"1,234.56 789.012"_s << Results( { { QObject::tr( "Go to 1,234.56 789.012 (Map CRS, )" ), QgsPointXY( 1234.56, 789.012 ) } } );

  QTest::newRow( "nort-west" ) << u"12.345N, 67.890W"_s << Results( { { QObject::tr( "Go to 12.345°N -67.89°E (EPSG:4326 - WGS 84)" ), QgsPointXY( -67.890, 12.345 ) } } );
  QTest::newRow( "east-south" ) << u"12.345 e, 67.890 s"_s << Results( { { QObject::tr( "Go to -67.89°N 12.345°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 12.345, -67.890 ) } } );
  QTest::newRow( "degree-suffix" ) << u"40deg 1' 0\" E 11deg  55' 0\" S"_s << Results( { { QObject::tr( "Go to -11.91666667°N 40.01666667°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 40.0166666667, -11.9166666667 ) } } );
  QTest::newRow( "north-east------------" ) << u"14°49′48″N 01°48′45″E"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "north-east-space------" ) << u"14°49′48″ N 01°48′45″ E"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "north-east-comma------" ) << u"14°49′48″N, 01°48′45″E"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "north-east-comma-space" ) << u"14°49′48″ N, 01°48′45″ E"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "north-east-front------" ) << u"N 14°49′48″ E 01°48′45″"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "north-east-front-comma" ) << u"N 14°49′48″, E 01°48′45″"_s << Results( { { QObject::tr( "Go to 14.83°N 1.8125°E (EPSG:4326 - WGS 84)" ), QgsPointXY( 1.8125, 14.83 ) } } );
  QTest::newRow( "osm.leaflet.OL" ) << u"https://www.openstreetmap.org/#map=15/44.5546/6.4936"_s << Results( { { QObject::tr( "Go to 44.5546°N 6.4936°E at scale 1:22569 (EPSG:4326 - WGS 84)" ), QgsPointXY( 6.4936, 44.5546 ), 22569.0 } } );
  QTest::newRow( "gmaps1" ) << u"https://www.google.com/maps/@44.5546,6.4936,15.25z"_s << Results( { { QObject::tr( "Go to 44.5546°N 6.4936°E at scale 1:22569 (EPSG:4326 - WGS 84)" ), QgsPointXY( 6.4936, 44.5546 ), 22569.0 } } );
  QTest::newRow( "gmaps2" ) << u"https://www.google.com/maps/@7.8750,81.0149,574195m/data=!3m1!1e3"_s << Results( { { QObject::tr( "Go to 7.875°N 81.0149°E at scale 1:6.49572e+07 (EPSG:4326 - WGS 84)" ), QgsPointXY( 81.0149, 7.8750 ) } } );
  QTest::newRow( "gmaps3" ) << u"https://www.google.com/maps/@27.7132,85.3288,3a,75y,278.89h,90t/data=!3m8!1e1!3m6!1sAF1QipMrXuXozGc9x9bxx5uPl_3ys4H-rNVqMLr6EYLA!2e10!3e11!6shttps:%2F%2Flh5.googleusercontent.com%2Fp%2FAF1QipMrXuXozGc9x9bxx5uPl_3ys4H-rNVqMLr6EYLA%3Dw203-h100-k-no-pi2.869903-ya293.58762-ro-1.9255565-fo100!7i3840!8i1920"_s << Results( { { QObject::tr( "Go to 27.7132°N 85.3288°E at scale 1:282 (EPSG:4326 - WGS 84)" ), QgsPointXY( 85.3288, 27.7132 ), 282.0 } } );
}

void TestQgsAppLocatorFilters::testGoto()
{
  QFETCH( QString, string );
  QFETCH( Results, expected );

  QgsGotoLocatorFilter filter;

  QList<QgsLocatorResult> results = gatherResults( &filter, string, QgsLocatorContext() );
  QCOMPARE( results.count(), expected.count() );

  for ( int i = 0; i < results.count(); i++ )
  {
    QCOMPARE( results.at( i ).displayString, expected.at( i ).displayString );
    QCOMPARE( results.at( i ).userData().toMap()[u"point"_s].value<QgsPointXY>(), expected.at( i ).point );
    if ( expected.at( i ).scale > 0 )
      QCOMPARE( results.at( 0 ).userData().toMap()[u"scale"_s].toDouble(), expected.at( i ).scale );
  }
}

QGSTEST_MAIN( TestQgsAppLocatorFilters )
#include "testqgsapplocatorfilters.moc"
