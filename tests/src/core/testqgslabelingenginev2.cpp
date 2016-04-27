/***************************************************************************
  testqgslabelingenginev2.cpp
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

#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgslabelingenginev2.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaprenderersequentialjob.h>
#include <qgsrulebasedlabeling.h>
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabeling.h>
#include <qgsvectorlayerlabelprovider.h>
#include "qgsrenderchecker.h"
#include "qgsfontutils.h"

class TestQgsLabelingEngineV2 : public QObject
{
    Q_OBJECT
  public:
    TestQgsLabelingEngineV2() : vl( 0 ) {}

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

  private:
    QgsVectorLayer* vl;

    QString mReport;

    void setDefaultLabelParams( QgsVectorLayer* layer );
    bool imageCheck( const QString& testName, QImage &image, int mismatchCount );
};

void TestQgsLabelingEngineV2::initTestCase()
{
  mReport += "<h1>Labeling Engine V2 Tests</h1>\n";

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsFontUtils::loadStandardTestFonts( QStringList() << "Bold" );
}

void TestQgsLabelingEngineV2::cleanupTestCase()
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

void TestQgsLabelingEngineV2::init()
{
  QString filename = QString( TEST_DATA_DIR ) + "/points.shp";
  vl = new QgsVectorLayer( filename, "points", "ogr" );
  Q_ASSERT( vl->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( vl );
}

void TestQgsLabelingEngineV2::cleanup()
{
  QgsMapLayerRegistry::instance()->removeMapLayer( vl->id() );
  vl = 0;
}

void TestQgsLabelingEngineV2::setDefaultLabelParams( QgsVectorLayer* layer )
{
  layer->setCustomProperty( "labeling/fontFamily", QgsFontUtils::getStandardTestFont( "Bold" ).family() );
  layer->setCustomProperty( "labeling/namedStyle", "Bold" );
  layer->setCustomProperty( "labeling/textColorR", "200" );
  layer->setCustomProperty( "labeling/textColorG", "0" );
  layer->setCustomProperty( "labeling/textColorB", "200" );
}

void TestQgsLabelingEngineV2::testBasic()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );
  mapSettings.setOutputDpi( 96 );

  // first render the map and labeling separately

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  vl->setCustomProperty( "labeling", "pal" );
  vl->setCustomProperty( "labeling/enabled", true );
  vl->setCustomProperty( "labeling/fieldName", "Class" );
  setDefaultLabelParams( vl );

  QgsLabelingEngineV2 engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString() ) );
  //engine.setFlags( QgsLabelingEngineV2::RenderOutlineLabels | QgsLabelingEngineV2::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_basic", img, 20 ) );

  // now let's test the variant when integrated into rendering loop
  //note the reference images are slightly different due to use of renderer for this test

  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  vl->setCustomProperty( "labeling/enabled", false );

  QVERIFY( imageCheck( "labeling_basic", img2, 20 ) );
}


void TestQgsLabelingEngineV2::testDiagrams()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );
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
  vl->loadNamedStyle( QString( TEST_DATA_DIR ) + "/points_diagrams.qml", res );
  Q_ASSERT( res );

  QgsLabelingEngineV2 engine;
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


void TestQgsLabelingEngineV2::testRuleBased()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );
  mapSettings.setOutputDpi( 96 );

  // set up most basic rule-based labeling for layer
  QgsRuleBasedLabeling::Rule* root = new QgsRuleBasedLabeling::Rule( 0 );

  QgsPalLayerSettings s1;
  s1.enabled = true;
  s1.fieldName = "Class";
  s1.obstacle = false;
  s1.dist = 2;
  s1.distInMapUnits = false;
  s1.textColor = QColor( 200, 0, 200 );
  s1.textFont = QgsFontUtils::getStandardTestFont( "Bold" );
  s1.placement = QgsPalLayerSettings::OverPoint;
  s1.quadOffset = QgsPalLayerSettings::QuadrantAboveLeft;
  s1.displayAll = true;

  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s1 ) ) );

  QgsPalLayerSettings s2;
  s2.enabled = true;
  s2.fieldName = "Class";
  s2.obstacle = false;
  s2.dist = 2;
  s2.textColor = Qt::red;
  s2.textFont = QgsFontUtils::getStandardTestFont( "Bold" );
  s2.placement = QgsPalLayerSettings::OverPoint;
  s2.quadOffset = QgsPalLayerSettings::QuadrantBelowRight;
  s2.displayAll = true;
  s2.setDataDefinedProperty( QgsPalLayerSettings::Size, true, true, "18", QString() );

  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s2 ), 0, 0, "Class = 'Jet'" ) );

  vl->setLabeling( new QgsRuleBasedLabeling( root ) );
  setDefaultLabelParams( vl );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();
  QImage img = job.renderedImage();
  QVERIFY( imageCheck( "labeling_rulebased", img, 20 ) );

  // test read/write rules
  QDomDocument doc, doc2, doc3;
  QDomElement e = vl->labeling()->save( doc );
  doc.appendChild( e );
  // read saved rules
  doc2.setContent( doc.toString() );
  QDomElement e2 = doc2.documentElement();
  QgsRuleBasedLabeling* rl2 = QgsRuleBasedLabeling::create( e2 );
  QVERIFY( rl2 );
  // check that another save will keep the data the same
  QDomElement e3 = rl2->save( doc3 );
  doc3.appendChild( e3 );
  QCOMPARE( doc.toString(), doc3.toString() );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling );

  delete rl2;

#if 0
  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsLabelingEngineV2 engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsRuleBasedLabelProvider( , vl ) );
  engine.run( context );
#endif
}

void TestQgsLabelingEngineV2::zOrder()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();

  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsPalLayerSettings pls1;
  pls1.enabled = true;
  pls1.fieldName = "Class";
  pls1.placement = QgsPalLayerSettings::OverPoint;
  pls1.quadOffset = QgsPalLayerSettings::QuadrantAboveRight;
  pls1.displayAll = true;
  pls1.textFont = QgsFontUtils::getStandardTestFont( "Bold" );
  pls1.textFont.setPointSizeF( 70 );
  //use data defined coloring and font size so that stacking order of labels can be determined
  pls1.setDataDefinedProperty( QgsPalLayerSettings::Color, true, true, "case when \"Class\"='Jet' then '#ff5500' when \"Class\"='B52' then '#00ffff' else '#ff00ff' end", QString() );
  pls1.setDataDefinedProperty( QgsPalLayerSettings::Size, true, true, "case when \"Class\"='Jet' then 100 when \"Class\"='B52' then 30 else 50 end", QString() );

  QgsVectorLayerLabelProvider* provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  QgsLabelingEngineV2 engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider1 );
  //engine.setFlags( QgsLabelingEngineV2::RenderOutlineLabels | QgsLabelingEngineV2::DrawLabelRectOnly );
  engine.run( context );
  p.end();
  engine.removeProvider( provider1 );

  // since labels are all from same layer and have same z-index then smaller labels should be stacked on top of larger
  // labels. Eg: B52 > Biplane > Jet
  QVERIFY( imageCheck( "label_order_size", img, 20 ) );
  img = job.renderedImage();

  //test data defined z-index
  pls1.setDataDefinedProperty( QgsPalLayerSettings::ZIndex, true, true, "case when \"Class\"='Jet' then 3 when \"Class\"='B52' then 1 else 2 end", QString() );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );
  p.begin( &img );
  engine.run( context );
  p.end();
  engine.removeProvider( provider1 );

  // z-index will take preference over label size, so labels should be stacked Jet > Biplane > B52
  QVERIFY( imageCheck( "label_order_zindex", img, 20 ) );
  img = job.renderedImage();

  pls1.removeAllDataDefinedProperties();
  pls1.textColor = QColor( 255, 50, 100 );
  pls1.textFont.setPointSizeF( 30 );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );

  //add a second layer
  QString filename = QString( TEST_DATA_DIR ) + "/points.shp";
  QgsVectorLayer* vl2 = new QgsVectorLayer( filename, "points", "ogr" );
  Q_ASSERT( vl2->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( vl2 );

  QgsPalLayerSettings pls2( pls1 );
  pls2.textColor = QColor( 0, 0, 0 );
  QgsVectorLayerLabelProvider* provider2 = new QgsVectorLayerLabelProvider( vl2, QString(), true, &pls2 );
  engine.addProvider( provider2 );

  mapSettings.setLayers( QStringList() << vl->id() << vl2->id() );
  engine.setMapSettings( mapSettings );

  p.begin( &img );
  engine.run( context );
  p.end();

  // labels have same z-index, so layer order will be used
  QVERIFY( imageCheck( "label_order_layer1", img, 20 ) );
  img = job.renderedImage();

  //flip layer order and re-test
  mapSettings.setLayers( QStringList() << vl2->id() << vl->id() );
  engine.setMapSettings( mapSettings );
  p.begin( &img );
  engine.run( context );
  p.end();

  // label order should be reversed
  QVERIFY( imageCheck( "label_order_layer2", img, 20 ) );
  img = job.renderedImage();

  //try mixing layer order and z-index
  engine.removeProvider( provider1 );
  pls1.setDataDefinedProperty( QgsPalLayerSettings::ZIndex, true, true, "if(\"Class\"='Jet',3,0)", QString() );
  provider1 = new QgsVectorLayerLabelProvider( vl, QString(), true, &pls1 );
  engine.addProvider( provider1 );

  p.begin( &img );
  engine.run( context );
  p.end();

  // label order should be most labels from layer 1, then labels from layer 2, then "Jet"s from layer 1
  QVERIFY( imageCheck( "label_order_mixed", img, 20 ) );
  img = job.renderedImage();

  //cleanup
  QgsMapLayerRegistry::instance()->removeMapLayer( vl2 );
}

void TestQgsLabelingEngineV2::testEncodeDecodePositionOrder()
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
  decoded = QgsLabelingUtils::decodePredefinedPositionOrder( ",tr,x,BSR, L, t,," );
  QVector< QgsPalLayerSettings::PredefinedPointPosition > expected;
  expected << QgsPalLayerSettings::TopRight << QgsPalLayerSettings::BottomSlightlyRight
  << QgsPalLayerSettings::MiddleLeft << QgsPalLayerSettings::TopMiddle;
  QCOMPARE( decoded, expected );
}

bool TestQgsLabelingEngineV2::imageCheck( const QString& testName, QImage &image, int mismatchCount )
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
  checker.setControlPathPrefix( "labelingenginev2" );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  bool resultFlag = checker.compareImages( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

QTEST_MAIN( TestQgsLabelingEngineV2 )
#include "testqgslabelingenginev2.moc"
