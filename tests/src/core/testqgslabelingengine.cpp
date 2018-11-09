/***************************************************************************
  testqgslabelingengine.cpp
  --------------------------------------
  Date                 : September 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <qgsapplication.h>
#include <qgslabelingengine.h>
#include <qgsproject.h>
#include <qgsmaprenderersequentialjob.h>
#include <qgsreadwritecontext.h>
#include <qgsrulebasedlabeling.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabeling.h>
#include <qgsvectorlayerlabelprovider.h>
#include "qgsrenderchecker.h"
#include "qgsfontutils.h"
#include "qgsnullsymbolrenderer.h"

class TestQgsLabelingEngine : public QObject
{
    Q_OBJECT
  public:
    TestQgsLabelingEngine() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testBasic();
    void testDiagrams();
    void testRuleBased();
    void zOrder(); //test that labels are stacked correctly
    void testEncodeDecodePositionOrder();
    void testSubstitutions();
    void testCapitalization();
    void testParticipatingLayers();
    void testRegisterFeatureUnprojectible();
    void testRotateHidePartial();
    void testParallelLabelSmallFeature();

  private:
    QgsVectorLayer *vl = nullptr;

    QString mReport;

    void setDefaultLabelParams( QgsPalLayerSettings &settings );
    bool imageCheck( const QString &testName, QImage &image, int mismatchCount );

};

void TestQgsLabelingEngine::initTestCase()
{
  mReport += QLatin1String( "<h1>Labeling Engine Tests</h1>\n" );

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

void TestQgsLabelingEngine::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLabelingEngine::init()
{
  QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  vl = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
}

void TestQgsLabelingEngine::cleanup()
{
  QgsProject::instance()->removeMapLayer( vl->id() );
  vl = nullptr;
}

void TestQgsLabelingEngine::setDefaultLabelParams( QgsPalLayerSettings &settings )
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
}

void TestQgsLabelingEngine::testBasic()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  setDefaultLabelParams( settings );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_basic", img, 20 ) );

  // now let's test the variant when integrated into rendering loop
  //note the reference images are slightly different due to use of renderer for this test

  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  vl->setLabeling( nullptr );

  QVERIFY( imageCheck( "labeling_basic", img2, 20 ) );
}


void TestQgsLabelingEngine::testDiagrams()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // first render the map and diagrams separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  bool res;
  vl->loadNamedStyle( QStringLiteral( TEST_DATA_DIR ) + "/points_diagrams.qml", res );
  QVERIFY( res );

  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerDiagramProvider( vl ) );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_point_diagrams", img, 20 ) );

  // now let's test the variant when integrated into rendering loop
  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  vl->loadDefaultStyle( res );
  QVERIFY( imageCheck( "labeling_point_diagrams", img2, 20 ) );
}


void TestQgsLabelingEngine::testRuleBased()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // set up most basic rule-based labeling for layer
  QgsRuleBasedLabeling::Rule *root = new QgsRuleBasedLabeling::Rule( nullptr );

  QgsPalLayerSettings s1;
  s1.fieldName = QStringLiteral( "Class" );
  s1.obstacle = false;
  s1.dist = 2;
  QgsTextFormat format = s1.format();
  format.setColor( QColor( 200, 0, 200 ) );
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 12 );
  s1.setFormat( format );
  s1.placement = QgsPalLayerSettings::OverPoint;
  s1.quadOffset = QgsPalLayerSettings::QuadrantAboveLeft;
  s1.displayAll = true;

  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s1 ) ) );

  QgsPalLayerSettings s2;
  s2.fieldName = QStringLiteral( "Class" );
  s2.obstacle = false;
  s2.dist = 2;
  format = s2.format();
  format.setColor( Qt::red );
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  s2.setFormat( format );
  s2.placement = QgsPalLayerSettings::OverPoint;
  s2.quadOffset = QgsPalLayerSettings::QuadrantBelowRight;
  s2.displayAll = true;

  s2.dataDefinedProperties().setProperty( QgsPalLayerSettings::Size, QgsProperty::fromValue( QStringLiteral( "18" ) ) );

  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s2 ), 0, 0, QStringLiteral( "Class = 'Jet'" ) ) );

  vl->setLabeling( new QgsRuleBasedLabeling( root ) );
  vl->setLabelsEnabled( true );
  //setDefaultLabelParams( vl );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();
  QImage img = job.renderedImage();
  QVERIFY( imageCheck( "labeling_rulebased", img, 20 ) );

  // test read/write rules
  QDomDocument doc, doc2, doc3;
  QDomElement e = vl->labeling()->save( doc, QgsReadWriteContext() );
  doc.appendChild( e );
  // read saved rules
  doc2.setContent( doc.toString() );
  QDomElement e2 = doc2.documentElement();
  QgsRuleBasedLabeling *rl2 = QgsRuleBasedLabeling::create( e2, QgsReadWriteContext() );
  QVERIFY( rl2 );
  // check that another save will keep the data the same
  QDomElement e3 = rl2->save( doc3, QgsReadWriteContext() );
  doc3.appendChild( e3 );
  QCOMPARE( doc.toString(), doc3.toString() );

  vl->setLabeling( nullptr );

  delete rl2;

#if 0
  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsRuleBasedLabelProvider(, vl ) );
  engine.run( context );
#endif
}

void TestQgsLabelingEngine::zOrder()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings pls1;
  pls1.fieldName = QStringLiteral( "Class" );
  pls1.placement = QgsPalLayerSettings::OverPoint;
  pls1.quadOffset = QgsPalLayerSettings::QuadrantAboveRight;
  pls1.displayAll = true;
  QgsTextFormat format = pls1.format();
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 70 );
  pls1.setFormat( format );

  //use data defined coloring and font size so that stacking order of labels can be determined
  pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Color, QgsProperty::fromExpression( QStringLiteral( "case when \"Class\"='Jet' then '#ff5500' when \"Class\"='B52' then '#00ffff' else '#ff00ff' end" ) ) );
  pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::Size, QgsProperty::fromExpression( QStringLiteral( "case when \"Class\"='Jet' then 100 when \"Class\"='B52' then 30 else 50 end" ) ) );

  QgsVectorLayerLabelProvider *provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider1 );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );
  p.end();
  engine.removeProvider( provider1 );

  // since labels are all from same layer and have same z-index then smaller labels should be stacked on top of larger
  // labels. For example: B52 > Biplane > Jet
  QVERIFY( imageCheck( "label_order_size", img, 20 ) );
  img = job.renderedImage();

  //test data defined z-index
  pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::ZIndex, QgsProperty::fromExpression( QStringLiteral( "case when \"Class\"='Jet' then 3 when \"Class\"='B52' then 1 else 2 end" ) ) );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );
  p.begin( &img );
  engine.run( context );
  p.end();
  engine.removeProvider( provider1 );

  // z-index will take preference over label size, so labels should be stacked Jet > Biplane > B52
  QVERIFY( imageCheck( "label_order_zindex", img, 20 ) );
  img = job.renderedImage();

  pls1.dataDefinedProperties().clear();
  format = pls1.format();
  format.setColor( QColor( 255, 50, 100 ) );
  format.setSize( 30 );
  pls1.setFormat( format );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );

  //add a second layer
  QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  QgsVectorLayer *vl2 = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl2->isValid() );
  QgsProject::instance()->addMapLayer( vl2 );

  QgsPalLayerSettings pls2( pls1 );
  format = pls2.format();
  format.setColor( QColor( 0, 0, 0 ) );
  pls2.setFormat( format );
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( vl2, QString(), true, &pls2 );
  engine.addProvider( provider2 );

  mapSettings.setLayers( QList<QgsMapLayer *>() << vl << vl2 );
  engine.setMapSettings( mapSettings );

  p.begin( &img );
  engine.run( context );
  p.end();

  // labels have same z-index, so layer order will be used
  QVERIFY( imageCheck( "label_order_layer1", img, 20 ) );
  img = job.renderedImage();

  //flip layer order and re-test
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2 << vl );
  engine.setMapSettings( mapSettings );
  p.begin( &img );
  engine.run( context );
  p.end();

  // label order should be reversed
  QVERIFY( imageCheck( "label_order_layer2", img, 20 ) );
  img = job.renderedImage();

  //try mixing layer order and z-index
  engine.removeProvider( provider1 );
  pls1.dataDefinedProperties().setProperty( QgsPalLayerSettings::ZIndex, QgsProperty::fromExpression( QStringLiteral( "if(\"Class\"='Jet',3,0)" ) ) );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );

  p.begin( &img );
  engine.run( context );
  p.end();

  // label order should be most labels from layer 1, then labels from layer 2, then "Jet"s from layer 1
  QVERIFY( imageCheck( "label_order_mixed", img, 20 ) );
  img = job.renderedImage();

  //cleanup
  QgsProject::instance()->removeMapLayer( vl2 );
}

void TestQgsLabelingEngine::testEncodeDecodePositionOrder()
{
  //create an ordered position list
  QVector< QgsPalLayerSettings::PredefinedPointPosition > original;
  //make sure all placements are added here
  original << QgsPalLayerSettings::BottomLeft << QgsPalLayerSettings::BottomSlightlyLeft
           << QgsPalLayerSettings::BottomMiddle << QgsPalLayerSettings::BottomSlightlyRight
           << QgsPalLayerSettings::BottomRight << QgsPalLayerSettings::MiddleRight
           << QgsPalLayerSettings::MiddleLeft << QgsPalLayerSettings::TopLeft
           << QgsPalLayerSettings::TopSlightlyLeft << QgsPalLayerSettings::TopMiddle
           << QgsPalLayerSettings::TopSlightlyRight << QgsPalLayerSettings::TopRight;
  //encode list
  QString encoded = QgsLabelingUtils::encodePredefinedPositionOrder( original );
  QVERIFY( !encoded.isEmpty() );

  //decode
  QVector< QgsPalLayerSettings::PredefinedPointPosition > decoded = QgsLabelingUtils::decodePredefinedPositionOrder( encoded );
  QCOMPARE( decoded, original );

  //test decoding with a messy string
  decoded = QgsLabelingUtils::decodePredefinedPositionOrder( QStringLiteral( ",tr,x,BSR, L, t,," ) );
  QVector< QgsPalLayerSettings::PredefinedPointPosition > expected;
  expected << QgsPalLayerSettings::TopRight << QgsPalLayerSettings::BottomSlightlyRight
           << QgsPalLayerSettings::MiddleLeft << QgsPalLayerSettings::TopMiddle;
  QCOMPARE( decoded, expected );
}

void TestQgsLabelingEngine::testSubstitutions()
{
  QgsPalLayerSettings settings;
  settings.useSubstitutions = false;
  QgsStringReplacementCollection collection( QList< QgsStringReplacement >() << QgsStringReplacement( QStringLiteral( "aa" ), QStringLiteral( "bb" ) ) );
  settings.substitutions = collection;
  settings.fieldName = QStringLiteral( "'aa label'" );
  settings.isExpression = true;

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl->fields(), 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );

  // make a fake render context
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider );
  provider->prepare( context, attributes );

  provider->registerFeature( f, context );
  QCOMPARE( provider->mLabels.at( 0 )->labelText(), QString( "aa label" ) );

  //with substitution
  settings.useSubstitutions = true;
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test2" ), true, &settings );
  engine.addProvider( provider2 );
  provider2->prepare( context, attributes );

  provider2->registerFeature( f, context );
  QCOMPARE( provider2->mLabels.at( 0 )->labelText(), QString( "bb label" ) );
}

void TestQgsLabelingEngine::testCapitalization()
{
  QgsFeature f( vl->fields(), 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 1, 2 ) ) );

  // make a fake render context
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );

  // no change
  QgsPalLayerSettings settings;
  QgsTextFormat format = settings.format();
  QFont font = format.font();
  font.setCapitalization( QFont::MixedCase );
  format.setFont( font );
  settings.setFormat( format );
  settings.fieldName = QStringLiteral( "'a teSt LABEL'" );
  settings.isExpression = true;

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider );
  provider->prepare( context, attributes );
  provider->registerFeature( f, context );
  QCOMPARE( provider->mLabels.at( 0 )->labelText(), QString( "a teSt LABEL" ) );

  //uppercase
  font.setCapitalization( QFont::AllUppercase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test2" ), true, &settings );
  engine.addProvider( provider2 );
  provider2->prepare( context, attributes );
  provider2->registerFeature( f, context );
  QCOMPARE( provider2->mLabels.at( 0 )->labelText(), QString( "A TEST LABEL" ) );

  //lowercase
  font.setCapitalization( QFont::AllLowercase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider3 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test3" ), true, &settings );
  engine.addProvider( provider3 );
  provider3->prepare( context, attributes );
  provider3->registerFeature( f, context );
  QCOMPARE( provider3->mLabels.at( 0 )->labelText(), QString( "a test label" ) );

  //first letter uppercase
  font.setCapitalization( QFont::Capitalize );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider4 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test4" ), true, &settings );
  engine.addProvider( provider4 );
  provider4->prepare( context, attributes );
  provider4->registerFeature( f, context );
  QCOMPARE( provider4->mLabels.at( 0 )->labelText(), QString( "A TeSt LABEL" ) );
}

void TestQgsLabelingEngine::testParticipatingLayers()
{
  QgsLabelingEngine engine;
  QVERIFY( engine.participatingLayers().isEmpty() );

  QgsPalLayerSettings settings1;
  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings1 );
  engine.addProvider( provider );
  QCOMPARE( engine.participatingLayers(), QList<QgsMapLayer *>() << vl );

  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "layer2" ), QStringLiteral( "memory" ) );
  QgsPalLayerSettings settings2;
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( layer2, QStringLiteral( "test2" ), true, &settings2 );
  engine.addProvider( provider2 );
  QCOMPARE( engine.participatingLayers().toSet(), QSet< QgsMapLayer * >() << vl << layer2 );

  // add a rule-based labeling node
  QgsRuleBasedLabeling::Rule *root = new QgsRuleBasedLabeling::Rule( nullptr );
  QgsPalLayerSettings s1;
  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s1 ) ) );
  QgsPalLayerSettings s2;
  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s2 ) ) );

  QgsVectorLayer *layer3 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "layer3" ), QStringLiteral( "memory" ) );
  QgsRuleBasedLabelProvider *ruleProvider = new QgsRuleBasedLabelProvider( QgsRuleBasedLabeling( root ), layer3 );
  engine.addProvider( ruleProvider );
  QCOMPARE( engine.participatingLayers().toSet(), QSet< QgsMapLayer * >() << vl << layer2 << layer3 );
}

bool TestQgsLabelingEngine::imageCheck( const QString &testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  QString tempDir = QDir::tempPath() + '/';
  QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "labelingengine" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

// See https://issues.qgis.org/issues/15507
void TestQgsLabelingEngine::testRegisterFeatureUnprojectible()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'aa label'" );
  settings.isExpression = true;
  settings.fitInPolygonOnly = true;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "polygon?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl2.get(), QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl2->fields(), 1 );

  QString wkt1 = QStringLiteral( "POLYGON((0 0,8 0,8 -90,0 0))" );
  f.setGeometry( QgsGeometry().fromWkt( wkt1 ) );

  // make a fake render context
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider );
  provider->prepare( context, attributes );

  provider->registerFeature( f, context );
  QCOMPARE( provider->mLabels.size(), 0 );
}

void TestQgsLabelingEngine::testRotateHidePartial()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'label'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "polygon?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl2.get(), QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl2->fields(), 1 );

  f.setGeometry( QgsGeometry().fromWkt( QStringLiteral( "POLYGON((0 0,8 0,8 8,0 8,0 0))" ) ) );
  vl2->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry().fromWkt( QStringLiteral( "POLYGON((20 20,28 20,28 28,20 28,20 20))" ) ) );
  vl2->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry().fromWkt( QStringLiteral( "POLYGON((0 20,8 20,8 28,0 28,0 20))" ) ) );
  vl2->dataProvider()->addFeature( f );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:4326" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setRotation( 45 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider );

  engine.run( context );
  p.end();
  engine.removeProvider( provider );

  QVERIFY( imageCheck( "label_rotate_hide_partial", img, 20 ) );
}

void TestQgsLabelingEngine::testParallelLabelSmallFeature()
{
  // Test rendering a small, closed linestring using parallel labeling
  // This test assumes that NO label is drawn in this situation. In future we may want
  // to revisit this and e.g. draw a centered horizontal label over the feature -- in which
  // case the reference image here should be freely revised. But for now, we just don't
  // want a hang/crash such as described in https://issues.qgis.org/issues/18283

  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'long label which doesn\\'t fit'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "linestring?crs=epsg:3148&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl2.get(), QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl2->fields(), 1 );

  f.setGeometry( QgsGeometry().fromWkt( QStringLiteral( "MultiLineString ((491176.796876200591214 1277565.39028006233274937, 491172.03128372476203367 1277562.45040752924978733, 491167.67935446038609371 1277557.28786265244707465, 491165.36599104333436117 1277550.97473702346906066, 491165.35308923490811139 1277544.24074512091465294, 491166.8345245998352766 1277539.49665334494784474, 491169.47186020453227684 1277535.27191955596208572, 491173.11253597546601668 1277531.85408334922976792, 491179.02124191814800724 1277528.94421873707324266, 491185.57387020520400256 1277528.15719766705296934, 491192.01811734877992421 1277529.57064539520069957, 491197.62341773137450218 1277533.02997340611182153, 491201.74636711279163137 1277538.15941766835749149, 491203.92884904221864417 1277544.35095247370190918, 491203.9633954341406934 1277550.5652371181640774, 491202.02436481812037528 1277556.4815535971429199, 491198.296930403157603 1277561.48062952468171716, 491193.17346247035311535 1277565.0647635399363935, 491187.82046439842088148 1277566.747082503978163, 491182.21622701874002814 1277566.85931688314303756, 491176.796876200591214 1277565.39028006233274937))" ) ) );
  vl2->dataProvider()->addFeature( f );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:3148" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 490359.7, 1276862.1, 492587.8, 1278500.0 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider );

  engine.run( context );
  p.end();
  engine.removeProvider( provider );

  // no need to actually check the result here -- we were just testing that no hang/crash occurred
  //  QVERIFY( imageCheck( "label_rotate_hide_partial", img, 20 ) );
}

QGSTEST_MAIN( TestQgsLabelingEngine )
#include "testqgslabelingengine.moc"
