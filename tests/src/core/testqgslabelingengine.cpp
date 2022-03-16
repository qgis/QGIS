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
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsnullsymbolrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssymbol.h"
#include "pointset.h"
#include "qgslabelingresults.h"
#include "qgscallout.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbol.h"

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
    void testEngineSettings();
    void testScaledFont();
    void testBasic();
    void testDiagrams();
    void testRuleBased();
    void zOrder(); //test that labels are stacked correctly
    void testEncodeDecodePositionOrder();
    void testEncodeDecodeLinePlacement();
    void testSubstitutions();
    void testCapitalization();
    void testNumberFormat();
    void testParticipatingLayers();
    void testRegisterFeatureUnprojectible();
    void testRotateHidePartial();
    void testParallelLabelSmallFeature();
    void testAdjacentParts();
    void testTouchingParts();
    void testMergingLinesWithForks();
    void testMergingLinesWithMinimumSize();
    void testCurvedLabelsWithTinySegments();
    void testCurvedLabelCorrectLinePlacement();
    void testCurvedLabelNegativeDistance();
    void testCurvedLabelOnSmallLineNearCenter();
    void testCurvedLabelLineOrientationAbove();
    void testCurvedLabelLineOrientationBelow();
    void testCurvedLabelAllowUpsideDownAbove();
    void testCurvedLabelAllowUpsideDownBelow();
    void testRepeatDistanceWithSmallLine();
    void testParallelPlacementPreferAbove();
    void testLabelBoundary();
    void testLabelBlockingRegion();
    void testLabelRotationWithReprojection();
    void testLabelRotationUnit();
    void drawUnplaced();
    void labelingResults();
    void labelingResultsWithCallouts();
    void pointsetExtend();
    void curvedOverrun();
    void parallelOverrun();
    void testDataDefinedLabelAllParts();
    void testDataDefinedPlacementPositionPoint();
    void testVerticalOrientation();
    void testVerticalOrientationLetterLineSpacing();
    void testRotationBasedOrientationPoint();
    void testRotationBasedOrientationLine();
    void testMapUnitLetterSpacing();
    void testMapUnitWordSpacing();
    void testReferencedFields();
    void testClipping();
    void testLineAnchorParallel();
    void testLineAnchorParallelConstraints();
    void testLineAnchorDataDefinedType();
    void testLineAnchorCurved();
    void testLineAnchorCurvedConstraints();
    void testLineAnchorCurvedOverrun();
    void testLineAnchorCurvedStrictAllUpsideDown();
    void testLineAnchorHorizontal();
    void testLineAnchorHorizontalConstraints();
    void testLineAnchorClipping();
    void testShowAllLabelsWhenALabelHasNoCandidates();
    void testSymbologyScalingFactor();
    void testSymbologyScalingFactor2();

  private:
    QgsVectorLayer *vl = nullptr;

    QString mReport;

    void setDefaultLabelParams( QgsPalLayerSettings &settings );
    QgsLabelingEngineSettings createLabelEngineSettings();
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
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  vl = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsProject::instance()->addMapLayer( vl );
}

void TestQgsLabelingEngine::cleanup()
{
  QgsProject::instance()->removeMapLayer( vl->id() );
  vl = nullptr;
}

void TestQgsLabelingEngine::testEngineSettings()
{
  // test labeling engine settings

  // getters/setters
  QgsLabelingEngineSettings settings;

  // default for new projects should be placement engine v2
  QCOMPARE( settings.placementVersion(), QgsLabelingEngineSettings::PlacementEngineVersion2 );

  settings.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  QCOMPARE( settings.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
  settings.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  QCOMPARE( settings.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );

  settings.setPlacementVersion( QgsLabelingEngineSettings::PlacementEngineVersion1 );
  QCOMPARE( settings.placementVersion(), QgsLabelingEngineSettings::PlacementEngineVersion1 );

  settings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, true );
  QVERIFY( settings.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );
  settings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, false );
  QVERIFY( !settings.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );

  settings.setUnplacedLabelColor( QColor( 0, 255, 0 ) );
  QCOMPARE( settings.unplacedLabelColor().name(), QStringLiteral( "#00ff00" ) );

  // reading from project
  QgsProject p;
  settings.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  settings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, true );
  settings.setUnplacedLabelColor( QColor( 0, 255, 0 ) );
  settings.setPlacementVersion( QgsLabelingEngineSettings::PlacementEngineVersion1 );
  settings.writeSettingsToProject( &p );
  QgsLabelingEngineSettings settings2;
  settings2.readSettingsFromProject( &p );
  QCOMPARE( settings2.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
  QVERIFY( settings2.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );
  QCOMPARE( settings2.unplacedLabelColor().name(), QStringLiteral( "#00ff00" ) );

  settings.setDefaultTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  settings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, false );
  settings.writeSettingsToProject( &p );
  settings2.readSettingsFromProject( &p );
  QCOMPARE( settings2.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );
  QVERIFY( !settings2.testFlag( QgsLabelingEngineSettings::DrawUnplacedLabels ) );
  QCOMPARE( settings2.placementVersion(), QgsLabelingEngineSettings::PlacementEngineVersion1 );

  // test that older setting is still respected as a fallback
  QgsProject p2;
  QgsLabelingEngineSettings settings3;
  p2.writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), false );
  settings3.readSettingsFromProject( &p2 );
  QCOMPARE( settings3.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysText );

  p2.writeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/DrawOutlineLabels" ), true );
  settings3.readSettingsFromProject( &p2 );
  QCOMPARE( settings3.defaultTextRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );

  // when opening an older project, labeling engine version should be 1
  p2.removeEntry( QStringLiteral( "PAL" ), QStringLiteral( "/PlacementEngineVersion" ) );
  settings3.readSettingsFromProject( &p2 );
  QCOMPARE( settings3.placementVersion(), QgsLabelingEngineSettings::PlacementEngineVersion1 );
}

void TestQgsLabelingEngine::testScaledFont()
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 9.9 );
  format.setSizeUnit( QgsUnitTypes::RenderUnit::RenderPixels );

  bool isNullSize = true;

  QgsRenderContext context;
  QFont f = format.scaledFont( context, 1.0, &isNullSize );
  QCOMPARE( f.pixelSize(), 10 );
  QVERIFY( !isNullSize );

  isNullSize = true;
  f = format.scaledFont( context, 10.0, &isNullSize );
  QCOMPARE( f.pixelSize(), 100 );
  QVERIFY( !isNullSize );

  isNullSize = false;
  format.setSize( 0 );
  format.scaledFont( context, 1.0, &isNullSize );
  QVERIFY( isNullSize );

  isNullSize = false;
  format.scaledFont( context, 10.0, &isNullSize );
  QVERIFY( isNullSize );
};


void TestQgsLabelingEngine::setDefaultLabelParams( QgsPalLayerSettings &settings )
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
}

QgsLabelingEngineSettings TestQgsLabelingEngine::createLabelEngineSettings()
{
  QgsLabelingEngineSettings settings;
  settings.setPlacementVersion( QgsLabelingEngineSettings::PlacementEngineVersion2 );
  return settings;
}

void TestQgsLabelingEngine::testBasic()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  QgsDefaultLabelingEngine engine;
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
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  QgsDefaultLabelingEngine engine;
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
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );

  // set up most basic rule-based labeling for layer
  QgsRuleBasedLabeling::Rule *root = new QgsRuleBasedLabeling::Rule( nullptr );

  QgsPalLayerSettings s1;
  s1.fieldName = QStringLiteral( "Class" );
  s1.obstacleSettings().setIsObstacle( false );
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
  s2.obstacleSettings().setIsObstacle( false );
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
  const QDomElement e = vl->labeling()->save( doc, QgsReadWriteContext() );
  doc.appendChild( e );
  // read saved rules
  doc2.setContent( doc.toString() );
  const QDomElement e2 = doc2.documentElement();
  QgsRuleBasedLabeling *rl2 = QgsRuleBasedLabeling::create( e2, QgsReadWriteContext() );
  QVERIFY( rl2 );
  // check that another save will keep the data the same
  const QDomElement e3 = rl2->save( doc3, QgsReadWriteContext() );
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
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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
  QgsDefaultLabelingEngine engine;
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
  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
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
  const QString encoded = QgsLabelingUtils::encodePredefinedPositionOrder( original );
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

void TestQgsLabelingEngine::testEncodeDecodeLinePlacement()
{
  QString encoded = QgsLabelingUtils::encodeLinePlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::OnLine );
  QVERIFY( !encoded.isEmpty() );
  QCOMPARE( QgsLabelingUtils::decodeLinePlacementFlags( encoded ), QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::OnLine );
  encoded = QgsLabelingUtils::encodeLinePlacementFlags( QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  QVERIFY( !encoded.isEmpty() );
  QCOMPARE( QgsLabelingUtils::decodeLinePlacementFlags( encoded ), QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::MapOrientation );

  //test decoding with a messy string
  QCOMPARE( QgsLabelingUtils::decodeLinePlacementFlags( QStringLiteral( ",ol,," ) ), QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  QCOMPARE( QgsLabelingUtils::decodeLinePlacementFlags( QStringLiteral( ",ol,BL,  al" ) ), QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  QCOMPARE( QgsLabelingUtils::decodeLinePlacementFlags( QStringLiteral( ",ol,BL, LO,  al" ) ), QgsLabeling::LinePlacementFlag::OnLine | QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::BelowLine );
}

void TestQgsLabelingEngine::testSubstitutions()
{
  QgsPalLayerSettings settings;
  settings.useSubstitutions = false;
  const QgsStringReplacementCollection collection( QList< QgsStringReplacement >() << QgsStringReplacement( QStringLiteral( "aa" ), QStringLiteral( "bb" ) ) );
  settings.substitutions = collection;
  settings.fieldName = QStringLiteral( "'aa label'" );
  settings.isExpression = true;

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl->fields(), 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -100, 30 ) ) );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsDefaultLabelingEngine engine;
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
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -100, 30 ) ) );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsDefaultLabelingEngine engine;
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

  font.setCapitalization( QFont::MixedCase );
  format.setCapitalization( Qgis::Capitalization::AllUppercase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider2b = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test2" ), true, &settings );
  engine.addProvider( provider2b );
  provider2b->prepare( context, attributes );
  provider2b->registerFeature( f, context );
  QCOMPARE( provider2b->mLabels.at( 0 )->labelText(), QString( "A TEST LABEL" ) );

  //lowercase
  font.setCapitalization( QFont::AllLowercase );
  format.setCapitalization( Qgis::Capitalization::MixedCase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider3 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test3" ), true, &settings );
  engine.addProvider( provider3 );
  provider3->prepare( context, attributes );
  provider3->registerFeature( f, context );
  QCOMPARE( provider3->mLabels.at( 0 )->labelText(), QString( "a test label" ) );

  font.setCapitalization( QFont::MixedCase );
  format.setCapitalization( Qgis::Capitalization::AllLowercase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider3b = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test3" ), true, &settings );
  engine.addProvider( provider3b );
  provider3b->prepare( context, attributes );
  provider3b->registerFeature( f, context );
  QCOMPARE( provider3b->mLabels.at( 0 )->labelText(), QString( "a test label" ) );

  //first letter uppercase
  font.setCapitalization( QFont::Capitalize );
  format.setCapitalization( Qgis::Capitalization::MixedCase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider4 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test4" ), true, &settings );
  engine.addProvider( provider4 );
  provider4->prepare( context, attributes );
  provider4->registerFeature( f, context );
  QCOMPARE( provider4->mLabels.at( 0 )->labelText(), QString( "A TeSt LABEL" ) );

  font.setCapitalization( QFont::MixedCase );
  format.setCapitalization( Qgis::Capitalization::ForceFirstLetterToCapital );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider4b = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test4" ), true, &settings );
  engine.addProvider( provider4b );
  provider4b->prepare( context, attributes );
  provider4b->registerFeature( f, context );
  QCOMPARE( provider4b->mLabels.at( 0 )->labelText(), QString( "A TeSt LABEL" ) );

  settings.fieldName = QStringLiteral( "'A TEST LABEL'" );
  format.setCapitalization( Qgis::Capitalization::TitleCase );
  format.setFont( font );
  settings.setFormat( format );
  QgsVectorLayerLabelProvider *provider5 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test4" ), true, &settings );
  engine.addProvider( provider5 );
  provider5->prepare( context, attributes );
  provider5->registerFeature( f, context );
  QCOMPARE( provider5->mLabels.at( 0 )->labelText(), QString( "A Test Label" ) );
}

void TestQgsLabelingEngine::testNumberFormat()
{
  QgsFeature f( vl->fields(), 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -100, 30 ) ) );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );

  // no change
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "110.112" );
  settings.isExpression = true;

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider );
  provider->prepare( context, attributes );
  provider->registerFeature( f, context );
  QCOMPARE( provider->mLabels.at( 0 )->labelText(), QStringLiteral( "110.112" ) );

  settings.fieldName = QStringLiteral( "-110.112" );
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider2 );
  provider2->prepare( context, attributes );
  provider2->registerFeature( f, context );
  QCOMPARE( provider2->mLabels.at( 0 )->labelText(), QStringLiteral( "-110.112" ) );

  settings.fieldName = QStringLiteral( "110.112" );
  settings.formatNumbers = true;
  settings.decimals = 6;
  QgsVectorLayerLabelProvider *provider3 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider3 );
  provider3->prepare( context, attributes );
  provider3->registerFeature( f, context );
  QCOMPARE( provider3->mLabels.at( 0 )->labelText(), QStringLiteral( "110.112000" ) );

  settings.fieldName = QStringLiteral( "-110.112" );
  QgsVectorLayerLabelProvider *provider4 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider4 );
  provider4->prepare( context, attributes );
  provider4->registerFeature( f, context );
  QCOMPARE( provider4->mLabels.at( 0 )->labelText(), QStringLiteral( "-110.112000" ) );

  settings.fieldName = QStringLiteral( "110.112" );
  settings.formatNumbers = true;
  settings.plusSign = true;
  QgsVectorLayerLabelProvider *provider5 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider5 );
  provider5->prepare( context, attributes );
  provider5->registerFeature( f, context );
  QCOMPARE( provider5->mLabels.at( 0 )->labelText(), QStringLiteral( "+110.112000" ) );

  settings.fieldName = QStringLiteral( "-110.112" );
  QgsVectorLayerLabelProvider *provider6 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider6 );
  provider6->prepare( context, attributes );
  provider6->registerFeature( f, context );
  QCOMPARE( provider6->mLabels.at( 0 )->labelText(), QStringLiteral( "-110.112000" ) );

  settings.formatNumbers = false;
  settings.fieldName = QStringLiteral( "110.112" );
  QgsVectorLayerLabelProvider *provider7 = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings );
  engine.addProvider( provider7 );
  provider7->prepare( context, attributes );
  provider7->registerFeature( f, context );
  QCOMPARE( provider7->mLabels.at( 0 )->labelText(), QStringLiteral( "110.112" ) );
}

void TestQgsLabelingEngine::testParticipatingLayers()
{
  QgsDefaultLabelingEngine engine;
  QVERIFY( engine.participatingLayers().isEmpty() );

  const QgsPalLayerSettings settings1;
  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl, QStringLiteral( "test" ), true, &settings1 );
  engine.addProvider( provider );
  QCOMPARE( engine.participatingLayers(), QList<QgsMapLayer *>() << vl );

  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "layer2" ), QStringLiteral( "memory" ) );
  const QgsPalLayerSettings settings2;
  QgsVectorLayerLabelProvider *provider2 = new QgsVectorLayerLabelProvider( layer2, QStringLiteral( "test2" ), true, &settings2 );
  engine.addProvider( provider2 );
  QCOMPARE( qgis::listToSet( engine.participatingLayers() ), QSet< QgsMapLayer * >() << vl << layer2 );

  // add a rule-based labeling node
  QgsRuleBasedLabeling::Rule *root = new QgsRuleBasedLabeling::Rule( nullptr );
  const QgsPalLayerSettings s1;
  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s1 ) ) );
  const QgsPalLayerSettings s2;
  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s2 ) ) );

  QgsVectorLayer *layer3 = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "layer3" ), QStringLiteral( "memory" ) );
  QgsRuleBasedLabelProvider *ruleProvider = new QgsRuleBasedLabelProvider( QgsRuleBasedLabeling( root ), layer3 );
  engine.addProvider( ruleProvider );
  QCOMPARE( qgis::listToSet( engine.participatingLayers() ), QSet< QgsMapLayer * >() << vl << layer2 << layer3 );
}

bool TestQgsLabelingEngine::imageCheck( const QString &testName, QImage &image, int mismatchCount )
{
  //draw background
  QImage imageWithBackground( image.width(), image.height(), QImage::Format_RGB32 );
  QgsMultiRenderChecker::drawBackground( &imageWithBackground );
  QPainter painter( &imageWithBackground );
  painter.drawImage( 0, 0, image );
  painter.end();

  mReport += "<h2>" + testName + "</h2>\n";
  const QString tempDir = QDir::tempPath() + '/';
  const QString fileName = tempDir + testName + ".png";
  imageWithBackground.save( fileName, "PNG" );
  QgsMultiRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "labelingengine" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( fileName );
  checker.setColorTolerance( 2 );
  const bool resultFlag = checker.runTest( testName, mismatchCount );
  mReport += checker.report();
  return resultFlag;
}

// See https://github.com/qgis/QGIS/issues/23431
void TestQgsLabelingEngine::testRegisterFeatureUnprojectible()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "'aa label'" );
  settings.isExpression = true;
  settings.fitInPolygonOnly = true;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "polygon?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl2.get(), QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl2->fields(), 1 );

  const QString wkt1 = QStringLiteral( "POLYGON((0 0,8 0,8 -90,0 0))" );
  f.setGeometry( QgsGeometry::fromWkt( wkt1 ) );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  QSet<QString> attributes;
  QgsDefaultLabelingEngine engine;
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

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((0 0,8 0,8 8,0 8,0 0))" ) ) );
  vl2->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((20 20,28 20,28 28,20 28,20 20))" ) ) );
  vl2->dataProvider()->addFeature( f );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((0 20,8 20,8 28,0 28,0 20))" ) ) );
  vl2->dataProvider()->addFeature( f );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  QgsDefaultLabelingEngine engine;
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
  // want a hang/crash such as described in https://github.com/qgis/QGIS/issues/26174

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
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsVectorLayerLabelProvider *provider = new QgsVectorLayerLabelProvider( vl2.get(), QStringLiteral( "test" ), true, &settings );
  QgsFeature f( vl2->fields(), 1 );

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((491176.796876200591214 1277565.39028006233274937, 491172.03128372476203367 1277562.45040752924978733, 491167.67935446038609371 1277557.28786265244707465, 491165.36599104333436117 1277550.97473702346906066, 491165.35308923490811139 1277544.24074512091465294, 491166.8345245998352766 1277539.49665334494784474, 491169.47186020453227684 1277535.27191955596208572, 491173.11253597546601668 1277531.85408334922976792, 491179.02124191814800724 1277528.94421873707324266, 491185.57387020520400256 1277528.15719766705296934, 491192.01811734877992421 1277529.57064539520069957, 491197.62341773137450218 1277533.02997340611182153, 491201.74636711279163137 1277538.15941766835749149, 491203.92884904221864417 1277544.35095247370190918, 491203.9633954341406934 1277550.5652371181640774, 491202.02436481812037528 1277556.4815535971429199, 491198.296930403157603 1277561.48062952468171716, 491193.17346247035311535 1277565.0647635399363935, 491187.82046439842088148 1277566.747082503978163, 491182.21622701874002814 1277566.85931688314303756, 491176.796876200591214 1277565.39028006233274937))" ) ) );
  vl2->dataProvider()->addFeature( f );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( provider );

  engine.run( context );
  p.end();
  engine.removeProvider( provider );

  // no need to actually check the result here -- we were just testing that no hang/crash occurred
  //  QVERIFY( imageCheck( "label_rotate_hide_partial", img, 20 ) );
}

void TestQgsLabelingEngine::testAdjacentParts()
{
  // test polygon layer with multipart feature with adjacent parts
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  settings.labelPerPart = true;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon (((1967901.6872910603415221 5162590.11975561361759901, 1967905.31832842249423265 5162591.80023225769400597, 1967907.63076798897236586 5162586.43503414187580347, 1967903.84105980419553816 5162584.57283254805952311, 1967901.6872910603415221 5162590.11975561361759901)),((1967901.64785283687524498 5162598.3270823871716857, 1967904.82891705213114619 5162601.06552503909915686, 1967910.82140435534529388 5162587.99774718284606934, 1967907.63076798897236586 5162586.43503414187580347, 1967905.31832842249423265 5162591.80023225769400597, 1967901.6872910603415221 5162590.11975561361759901, 1967899.27472299290820956 5162596.28855143301188946, 1967901.64785283687524498 5162598.3270823871716857)),((1967904.82891705213114619 5162601.06552503909915686, 1967901.64785283687524498 5162598.3270823871716857, 1967884.28552994946949184 5162626.09785370342433453, 1967895.81538487318903208 5162633.84423183929175138, 1967901.64141261484473944 5162624.63927845563739538, 1967906.47453573765233159 5162616.87410452589392662, 1967913.7844126324634999 5162604.47178338281810284, 1967909.58057221467606723 5162602.89022256527096033, 1967904.82891705213114619 5162601.06552503909915686)))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_adjacent_parts" ), img, 20 ) );
}

void TestQgsLabelingEngine::testTouchingParts()
{
  // test line layer with multipart feature with touching (but unmerged) parts
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXXXXXXXXXXXXXXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.lineSettings().setMergeLines( true );

  // if treated individually, none of these parts are long enough for the label to fit -- but the label should be rendered if the mergeLines setting is true,
  // because the parts should be merged into a single linestring
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiLineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((190000 5000010, 190050 5000000), (190050 5000000, 190100 5000000), (190200 5000000, 190150 5000000), (190150 5000000, 190100 5000000))" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_multipart_touching_lines" ), img, 20 ) );
}

void TestQgsLabelingEngine::testMergingLinesWithForks()
{
  // test that the "merge connected features" setting works well with line networks
  // containing forks and small side branches
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXXXXXXXXXXXXXXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.dist = 1;
  settings.lineSettings().setMergeLines( true );

  // if treated individually, none of these parts are long enough for the label to fit -- but the label should be rendered if the mergeLines setting is true
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190100 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // side branch
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000000, 190100 5000010)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // side branch
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000000, 190100 4999995)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // main road continues, note that we deliberately split this up into non-consecutive sections, just for extra checks!
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190120 5000000, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190120 5000000, 190100 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_multipart_touching_branches" ), img, 20 ) );
}

void TestQgsLabelingEngine::testMergingLinesWithMinimumSize()
{
  // test that the "merge connected features" setting works well with
  // a non-zero minimum feature size value
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.lineSettings().setMergeLines( true );
  settings.thinningSettings().setMinimumFeatureSize( 90.0 );

  // if treated individually, none of these parts exceed the minimum feature size set above -- but the label should be rendered if the mergeLines setting is true
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190100 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // side branch
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000000, 190100 5000010)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // side branch
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000000, 190100 4999995)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  // main road continues, note that we deliberately split this up into non-consecutive sections, just for extra checks!
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190120 5000000, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190120 5000000, 190100 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_merged_minimum_size" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelsWithTinySegments()
{
  // test drawing curved labels when input linestring has many small segments
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXXXXXXXXXXXXXXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  // our geometry starts with many small segments, followed by long ones
  QgsGeometry g( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190100 5000000)" ) ) );
  g = g.densifyByCount( 100 );
  qgsgeometry_cast< QgsLineString * >( g.get() )->addVertex( QgsPoint( 190200, 5000000 ) );
  f.setGeometry( g );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( g.boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, false );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_label_small_segments" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelCorrectLinePlacement()
{
  // test drawing curved labels when input linestring has many small segments
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXXXXXXXXXXXXXXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  settings.maxCurvedCharAngleIn = 99;
  settings.maxCurvedCharAngleOut = 99;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  // Geometry which roughly curves around from "1 o'clock" anticlockwise to 6 o'clock.
  const QgsGeometry g( QgsGeometry::fromWkt( QStringLiteral( "LineString (0.30541596873255172 0.3835845896147404, -0.08989391401451696 0.21831379117811278, -0.33668341708542704 -0.01619207146845336, -0.156895589056393 -0.20714684533780003, 0.02735901730876611 -0.21496370742601911)" ) ) );
  f.setGeometry( g );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( g.boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_label_above_1" ), img, 20 ) );

  // and below...
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!

  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_label_below_1" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelNegativeDistance()
{
  // test line label rendering with negative distance
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXXXXXXXXXXXXXXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.dist = -5;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190100 5000000, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_negative_distance" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelOnSmallLineNearCenter()
{
  // test a small line relative to label size still gives sufficient candidates to ensure more centered placements
  // are found
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190080 5000010, 190100 5000000, 190120 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_small_feature_centered" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelLineOrientationAbove()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000007, 190094 5000012, 190096 5000019, 190103 5000024, 190111 5000023, 190114 5000018)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190080, 5000000, 190130, 5000030 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_orientation_above" ), img, 20 ) );

  // reverse line and retry, label should be flipped to other side of line
  f.setGeometry( QgsGeometry( qgsgeometry_cast< QgsLineString * >( f.geometry().constGet() )->reversed() ) );
  vl2->dataProvider()->truncate();
  QVERIFY( vl2->dataProvider()->addFeature( f ) );


  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_orientation_reversed_above" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelLineOrientationBelow()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000007, 190094 5000012, 190096 5000019, 190103 5000024, 190111 5000023, 190114 5000018)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190080, 5000000, 190130, 5000030 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_orientation_below" ), img, 20 ) );

  // reverse line and retry, label should be flipped to other side of line
  f.setGeometry( QgsGeometry( qgsgeometry_cast< QgsLineString * >( f.geometry().constGet() )->reversed() ) );
  vl2->dataProvider()->truncate();
  QVERIFY( vl2->dataProvider()->addFeature( f ) );


  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_orientation_reversed_below" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelAllowUpsideDownAbove()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.upsidedownLabels = QgsPalLayerSettings::UpsideDownLabels::ShowAll;
  settings.lineSettings().setLineAnchorPercent( 0.05 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000007, 190094 5000012, 190096 5000019, 190103 5000024, 190111 5000023, 190114 5000018)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190080, 5000000, 190130, 5000030 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_allow_upside_down_above" ), img, 20 ) );

  // reverse line and retry, label should be flipped to other side of line
  f.setGeometry( QgsGeometry( qgsgeometry_cast< QgsLineString * >( f.geometry().constGet() )->reversed() ) );
  vl2->dataProvider()->truncate();
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_allow_upside_down_reversed_above" ), img, 20 ) );
}

void TestQgsLabelingEngine::testCurvedLabelAllowUpsideDownBelow()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.upsidedownLabels = QgsPalLayerSettings::UpsideDownLabels::ShowAll;
  settings.lineSettings().setLineAnchorPercent( 0.05 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190100 5000007, 190094 5000012, 190096 5000019, 190103 5000024, 190111 5000023, 190114 5000018)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190080, 5000000, 190130, 5000030 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_allow_upside_down_below" ), img, 20 ) );

  // reverse line and retry, label should be flipped to other side of line
  f.setGeometry( QgsGeometry( qgsgeometry_cast< QgsLineString * >( f.geometry().constGet() )->reversed() ) );
  vl2->dataProvider()->truncate();
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_line_allow_upside_down_reversed_below" ), img, 20 ) );
}

void TestQgsLabelingEngine::testRepeatDistanceWithSmallLine()
{
  // test a small line relative to label size still gives sufficient candidates to ensure more centered placements
  // are found
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.repeatDistance = 55;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190050 5000000, 190150 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_repeat_distance_with_small_line" ), img, 20 ) );
}

void TestQgsLabelingEngine::testParallelPlacementPreferAbove()
{
  // given the choice of above or below placement, labels should always be placed above
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::BelowLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  settings.labelPerPart = false;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_prefer_above" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLabelBoundary()
{
  // test that no labels are drawn outside of the specified label boundary
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f( vl2->fields(), 1 );

  for ( int x = 0; x < 15; x++ )
  {
    for ( int y = 0; y < 12; y++ )
    {
      f.setGeometry( std::make_unique< QgsPoint >( x, y ) );
      vl2->dataProvider()->addFeature( f );
    }
  }

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:4326" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  mapSettings.setLabelBoundaryGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((3 1, 12 1, 12 9, 3 9, 3 1),(8 4, 10 4, 10 7, 8 7, 8 4))" ) ) );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_boundary_geometry" ), img, 20 ) );

  // with rotation
  mapSettings.setRotation( 45 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "rotated_label_boundary_geometry" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLabelBlockingRegion()
{
  // test that no labels are drawn inside blocking regions
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f( vl2->fields(), 1 );

  for ( int x = 0; x < 15; x++ )
  {
    for ( int y = 0; y < 12; y++ )
    {
      f.setGeometry( std::make_unique< QgsPoint >( x, y ) );
      vl2->dataProvider()->addFeature( f );
    }
  }

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( QStringLiteral( "EPSG:4326" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QList< QgsLabelBlockingRegion > regions;
  regions << QgsLabelBlockingRegion( QgsGeometry::fromWkt( QStringLiteral( "Polygon((6 1, 12 1, 12 9, 6 9, 6 1),(8 4, 10 4, 10 7, 8 7, 8 4))" ) ) );
  regions << QgsLabelBlockingRegion( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 3 0, 3 3, 0 3, 0 0))" ) ) );
  mapSettings.setLabelBlockingRegions( regions );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_blocking_geometry" ), img, 20 ) );

  // with rotation
  mapSettings.setRotation( 45 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "rotated_label_blocking_geometry" ), img, 20 ) );

  // blocking regions WITH label margin
  mapSettings.setRotation( 0 );
  mapSettings.setLabelBoundaryGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((1 1, 14 1, 14 9, 1 9, 1 1))" ) ) );

  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_blocking_boundary_geometry" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLabelRotationWithReprojection()
{
  // test combination of map rotation with reprojected layer
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -6.250851540391068, 53.335006994584944 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 2 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -21.950014487179544, 64.150023619739216 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 3 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -0.118667702475932, 51.5019405883275 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  const QgsCoordinateReferenceSystem tgtCrs( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -4348530.5, 5618594.3, 2516176.1, 12412237.9 ) );
  mapSettings.setRotation( 60 );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_rotate_with_reproject" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLabelRotationUnit()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromExpression( QString::number( 3.14 / 2.0 ) ) );
  settings.setRotationUnit( QgsUnitTypes::AngleRadians );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "label_rotate_unit", img, 20 ) );

  vl->setLabeling( nullptr );
}

void TestQgsLabelingEngine::drawUnplaced()
{
  // test drawing unplaced labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  // first create two overlapping point labels
  QgsTextFormat format = settings.format();
  format.setSize( 50 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  settings.priority = 3;
  settings.obstacleSettings().setFactor( 0 );

  std::unique_ptr< QgsVectorLayer> vl1( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl1->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -6.250851540391068, 53.335006994584944 ) ) );
  QVERIFY( vl1->dataProvider()->addFeature( f ) );

  vl1->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl1->setLabelsEnabled( true );

  // second layer
  settings.fieldName = QStringLiteral( "'YY'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  settings.priority = 5; // higher priority - YY should be placed, not XX
  settings.obstacleSettings().setFactor( 0 );
  format.setSize( 90 );
  settings.setFormat( format );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // test a label with 0 candidates (line is too short for label)
  std::unique_ptr< QgsVectorLayer> vl3( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl3->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString(-6.250851540391068 60.6, -6.250851640391068 60.6 )" ) ) );
  QVERIFY( vl3->dataProvider()->addFeature( f ) );

  settings.placement = QgsPalLayerSettings::Curved;
  vl3->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl3->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  const QgsCoordinateReferenceSystem tgtCrs( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -4348530.5, 5618594.3, 2516176.1, 12412237.9 ) );
  mapSettings.setRotation( 60 );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl1.get() << vl2.get() << vl3.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, true );
  engineSettings.setUnplacedLabelColor( QColor( 255, 0, 255 ) );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "unplaced_labels" ), img, 20 ) );
}

void TestQgsLabelingEngine::labelingResults()
{
  // test retrieval of labeling results
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "\"id\"" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  settings.priority = 10;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -6.250851540391068, 53.335006994584944 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8888 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -21.950014487179544, 64.150023619739216 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 33333 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -0.118667702475932, 51.5019405883275 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  vl2->updateExtents();

  std::unique_ptr< QgsVectorLayer> vl3( vl2->clone() );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  const QgsCoordinateReferenceSystem tgtCrs( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -4137976.6, 6557092.6, 1585557.4, 9656515.0 ) );
// mapSettings.setRotation( 60 );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() << vl3.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  std::unique_ptr< QgsLabelingResults > results( job.takeLabelingResults() );
  QVERIFY( results );

  // retrieve some labels
  QList<QgsLabelPosition> labels = results->allLabels();
  QCOMPARE( labels.count(), 3 );
  std::sort( labels.begin(), labels.end(), []( const QgsLabelPosition & a, const QgsLabelPosition & b )
  {
    return a.labelText.compare( b.labelText ) < 0;
  } );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QCOMPARE( labels.at( 1 ).labelText, QStringLiteral( "33333" ) );
  QCOMPARE( labels.at( 2 ).labelText, QStringLiteral( "8888" ) );

  labels = results->labelsAtPosition( QgsPointXY( -654732, 7003282 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 1 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 167961, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -779822, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), -611861, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 6897647, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 7192767, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 0.0 );

  labels = results->labelsAtPosition( QgsPointXY( -769822, 6927647 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 1 );
  labels = results->labelsAtPosition( QgsPointXY( -615861, 7132767 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 1 );

  labels = results->labelsAtPosition( QgsPointXY( -2463392, 9361711 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 2 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "8888" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 671844, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -2779386, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), -2107542, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 9240403, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 9535523, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 0.0 );
  labels = results->labelsAtPosition( QgsPointXY( -1383, 6708478 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 3 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "33333" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 839805, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -433112, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), 406692, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 6563006, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 6858125, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 0.0 );
  labels = results->labelsAtPosition( QgsPointXY( -2463392, 6708478 ) );
  QCOMPARE( labels.count(), 0 );

  // with unplaced labels -- all vl3 labels will be unplaced, because they are conflicting with those in vl2
  settings.priority = 1;
  vl3->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl3->setLabelsEnabled( true );
  engineSettings.setFlag( QgsLabelingEngineSettings::CollectUnplacedLabels, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob jobB( mapSettings );
  jobB.start();
  jobB.waitForFinished();

  results.reset( jobB.takeLabelingResults() );
  QVERIFY( results );

  labels = results->allLabels();
  QCOMPARE( labels.count(), 6 );
  std::sort( labels.begin(), labels.end(), []( const QgsLabelPosition & a, const QgsLabelPosition & b )
  {
    return a.isUnplaced == b.isUnplaced ? a.labelText.compare( b.labelText ) < 0 : a.isUnplaced < b.isUnplaced;
  } );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QVERIFY( !labels.at( 0 ).isUnplaced );
  QCOMPARE( labels.at( 1 ).labelText, QStringLiteral( "33333" ) );
  QVERIFY( !labels.at( 1 ).isUnplaced );
  QCOMPARE( labels.at( 2 ).labelText, QStringLiteral( "8888" ) );
  QVERIFY( !labels.at( 2 ).isUnplaced );
  QCOMPARE( labels.at( 3 ).labelText, QStringLiteral( "1" ) );
  QVERIFY( labels.at( 3 ).isUnplaced );
  QCOMPARE( labels.at( 4 ).labelText, QStringLiteral( "33333" ) );
  QVERIFY( labels.at( 4 ).isUnplaced );
  QCOMPARE( labels.at( 5 ).labelText, QStringLiteral( "8888" ) );
  QVERIFY( labels.at( 5 ).isUnplaced );

  mapSettings.setLayers( {vl2.get() } );

  // with rotation
  mapSettings.setRotation( 60 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();
  results.reset( job2.takeLabelingResults() );
  QVERIFY( results );
  labels = results->labelsAtPosition( QgsPointXY( -654732, 7003282 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 1 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 167961, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -865622, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), -526060, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 6898697, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 7191716, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 60.0 );

  // should fall outside of rotated bounding box!
  labels = results->labelsAtPosition( QgsPointXY( -769822, 6927647 ) );
  QCOMPARE( labels.count(), 0 );
  labels = results->labelsAtPosition( QgsPointXY( -615861, 7132767 ) );
  QCOMPARE( labels.count(), 0 );
  // just on corner, should only work if rotation of label's bounding box is handled correctly
  labels = results->labelsAtPosition( QgsPointXY( -610000, 6898800 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 1 );

  labels = results->labelsAtPosition( QgsPointXY( -2463392, 9361711 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 2 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "8888" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 671844, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -2739216, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), -2147712, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 9023266, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 9752660, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 60.0 );
  labels = results->labelsAtPosition( QgsPointXY( -1383, 6708478 ) );
  QCOMPARE( labels.count(), 1 );
  QCOMPARE( labels.at( 0 ).featureId, 3 );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "33333" ) );
  QGSCOMPARENEAR( labels.at( 0 ).width, 839805, 500 ); // tolerance will probably need tweaking, to account for cross-platform font diffs
  QGSCOMPARENEAR( labels.at( 0 ).height, 295119, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMinimum(), -350952, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.xMaximum(), 324531, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMinimum(), 6273139, 500 );
  QGSCOMPARENEAR( labels.at( 0 ).labelRect.yMaximum(), 7147992, 500 );
  QCOMPARE( labels.at( 0 ).rotation, 60.0 );
  labels = results->labelsAtPosition( QgsPointXY( -2463392, 6708478 ) );
  QCOMPARE( labels.count(), 0 );
}

void TestQgsLabelingEngine::labelingResultsWithCallouts()
{
  // test retrieval of rendered callout properties from labeling results
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "\"id\"" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  QgsPropertyCollection labelProps;
  labelProps.setProperty( QgsPalLayerSettings::PositionX, QgsProperty::fromField( QStringLiteral( "labelx" ) ) );
  labelProps.setProperty( QgsPalLayerSettings::PositionY, QgsProperty::fromField( QStringLiteral( "labely" ) ) );
  settings.setDataDefinedProperties( labelProps );

  settings.setCallout( new QgsSimpleLineCallout() );
  settings.callout()->setEnabled( true );
  QgsPropertyCollection calloutProps;
  calloutProps.setProperty( QgsCallout::OriginX, QgsProperty::fromField( QStringLiteral( "calloutoriginx" ) ) );
  calloutProps.setProperty( QgsCallout::OriginY, QgsProperty::fromField( QStringLiteral( "calloutoriginy" ) ) );
  calloutProps.setProperty( QgsCallout::DestinationX, QgsProperty::fromField( QStringLiteral( "calloutdestx" ) ) );
  calloutProps.setProperty( QgsCallout::DestinationY, QgsProperty::fromField( QStringLiteral( "calloutdesty" ) ) );

  settings.callout()->setDataDefinedProperties( calloutProps );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer&field=labelx:double&field=labely:double&field=calloutoriginx:double&field=calloutoriginy:double&field=calloutdestx:double&field=calloutdesty:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << -20.173 << 58.624 << -11.160 << 58.001 << -3.814 << 56.046 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -6.250851540391068, 53.335006994584944 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  vl2->updateExtents();

  settings.setCallout( new QgsManhattanLineCallout() );
  settings.callout()->setEnabled( true );
  settings.callout()->setDataDefinedProperties( calloutProps );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // another layer
  std::unique_ptr< QgsVectorLayer> vl3( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3857&field=id:integer&field=labelx:double&field=labely:double&field=calloutoriginx:double&field=calloutoriginy:double&field=calloutdestx:double&field=calloutdesty:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );

  f.setAttributes( QgsAttributes() << 2 << -3424024 << 7849709 << -2713442 << 7628322 << -2567040 << 6974872 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -2995532, 7242679 ) ) );
  QVERIFY( vl3->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 3 << -4024024 << 7849709 << QVariant() << QVariant() << QVariant() << QVariant() );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -2995532, 7242679 ) ) );
  QVERIFY( vl3->dataProvider()->addFeature( f ) );

  vl3->updateExtents();

  vl3->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl3->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  const QgsCoordinateReferenceSystem tgtCrs( QStringLiteral( "EPSG:3857" ) );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -4137976.6, 6557092.6, 1585557.4, 9656515.0 ) );
  // mapSettings.setRotation( 60 );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() << vl3.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  job.renderedImage().save( QStringLiteral( "/tmp/renderer.png" ) );

  std::unique_ptr< QgsLabelingResults > results( job.takeLabelingResults() );
  QVERIFY( results );

  // retrieve some callouts
  QList<QgsCalloutPosition> callouts = results->calloutsWithinRectangle( QgsRectangle( -2713542, 7628122, -2713142, 7628822 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 6974872.0, 10 );
  QVERIFY( callouts.at( 0 ).originIsPinned() );
  QVERIFY( callouts.at( 0 ).destinationIsPinned() );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -2567340, 6974572, -2566740, 6975172 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 6974872.0, 10 );
  QVERIFY( callouts.at( 0 ).originIsPinned() );
  QVERIFY( callouts.at( 0 ).destinationIsPinned() );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -1242625, 7967227, -1242025, 7967827 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 7567578.0, 10 );
  QVERIFY( callouts.at( 0 ).originIsPinned() );
  QVERIFY( callouts.at( 0 ).destinationIsPinned() );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -424872, 7567278, -424272, 7567878 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 7567578.0, 10 );
  QVERIFY( callouts.at( 0 ).originIsPinned() );
  QVERIFY( callouts.at( 0 ).destinationIsPinned() );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -4104024, 7609709, -3804024, 8249709 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 2 );
  QCOMPARE( callouts.at( 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -3856062.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7849709.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -2995532.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 7242679.0, 10 );
  QVERIFY( !callouts.at( 0 ).originIsPinned() );
  QVERIFY( !callouts.at( 0 ).destinationIsPinned() );

  callouts = results->calloutsWithinRectangle( mapSettings.visibleExtent() );
  QCOMPARE( callouts.count(), 3 );
  const int callout1Index = callouts.at( 0 ).layerID == vl2->id() ? 0 : callouts.at( 1 ).layerID == vl2->id() ? 1 : 2;
  const int callout2Index = callouts.at( 0 ).layerID == vl3->id() && callouts.at( 0 ).featureId == 1 ? 0 :  callouts.at( 1 ).layerID == vl3->id() && callouts.at( 1 ).featureId == 1 ? 1 : 2;
  const int callout3Index = callouts.at( 0 ).layerID == vl3->id() && callouts.at( 0 ).featureId == 2 ? 0 :  callouts.at( 1 ).layerID == vl3->id() && callouts.at( 1 ).featureId == 2 ? 1 : 2;
  QCOMPARE( callouts.at( callout1Index ).featureId, 1 );
  QCOMPARE( callouts.at( callout1Index ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( callout1Index ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1Index ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1Index ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1Index ).destination().y(), 7567578.0, 10 );
  QVERIFY( callouts.at( callout1Index ).originIsPinned() );
  QVERIFY( callouts.at( callout1Index ).destinationIsPinned() );
  QCOMPARE( callouts.at( callout2Index ).featureId, 1 );
  QCOMPARE( callouts.at( callout2Index ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( callout2Index ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout2Index ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout2Index ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout2Index ).destination().y(), 6974872.0, 10 );
  QVERIFY( callouts.at( callout2Index ).originIsPinned() );
  QVERIFY( callouts.at( callout2Index ).destinationIsPinned() );
  QCOMPARE( callouts.at( callout3Index ).featureId, 2 );
  QCOMPARE( callouts.at( callout3Index ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( callout3Index ).origin().x(), -3856062.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout3Index ).origin().y(), 7849709.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout3Index ).destination().x(), -2995532.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout3Index ).destination().y(), 7242679.0, 10 );
  QVERIFY( !callouts.at( callout3Index ).originIsPinned() );
  QVERIFY( !callouts.at( callout3Index ).destinationIsPinned() );

  // with rotation
  mapSettings.setRotation( 60 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  results.reset( job2.takeLabelingResults() );
  QVERIFY( results );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -2713542, 7628122, -2713142, 7628822 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 6974872.0, 10 );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -2567340, 6974572, -2566740, 6975172 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 6974872.0, 10 );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -1242625, 7967227, -1242025, 7967827 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 7567578.0, 10 );

  callouts = results->calloutsWithinRectangle( QgsRectangle( -424872, 7567278, -424272, 7567878 ) );
  QCOMPARE( callouts.count(), 1 );
  QCOMPARE( callouts.at( 0 ).featureId, 1 );
  QCOMPARE( callouts.at( 0 ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( 0 ).destination().y(), 7567578.0, 10 );

  callouts = results->calloutsWithinRectangle( mapSettings.visibleExtent() );
  QCOMPARE( callouts.count(), 2 );
  const bool callout1IsFirstLayer = callouts.at( 0 ).layerID == vl2->id();
  QCOMPARE( callouts.at( callout1IsFirstLayer ? 0 : 1 ).featureId, 1 );
  QCOMPARE( callouts.at( callout1IsFirstLayer ? 0 : 1 ).layerID, vl2->id() );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 0 : 1 ).origin().x(), -1242325.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 0 : 1 ).origin().y(), 7967527.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 0 : 1 ).destination().x(), -424572.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 0 : 1 ).destination().y(), 7567578.0, 10 );
  QCOMPARE( callouts.at( callout1IsFirstLayer ? 1 : 0 ).featureId, 1 );
  QCOMPARE( callouts.at( callout1IsFirstLayer ? 1 : 0 ).layerID, vl3->id() );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 1 : 0 ).origin().x(), -2713442.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 1 : 0 ).origin().y(), 7628322.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 1 : 0 ).destination().x(), -2567040.0, 10 );
  QGSCOMPARENEAR( callouts.at( callout1IsFirstLayer ? 1 : 0 ).destination().y(), 6974872.0, 10 );
}

void TestQgsLabelingEngine::pointsetExtend()
{
  // test extending pointsets by distance
  {
    QVector< double > x;
    x << 1 << 9;
    QVector< double > y;
    y << 2 << 2;
    pal::PointSet set( 2, x.data(), y.data() );

    set.extendLineByDistance( 1, 3, 0 );
    QCOMPARE( set.getNumPoints(), 4 );
    QCOMPARE( set.x.at( 0 ), 0.0 );
    QCOMPARE( set.y.at( 0 ), 2.0 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 9.0 );
    QCOMPARE( set.y.at( 2 ), 2.0 );
    QCOMPARE( set.x.at( 3 ), 12.0 );
    QCOMPARE( set.y.at( 3 ), 2.0 );
  }

  {
    QVector< double > x;
    x << 1 << 9;
    QVector< double > y;
    y << 2 << 2;
    pal::PointSet set( 2, x.data(), y.data() );
    set.extendLineByDistance( 0, 0, 0 );
    QCOMPARE( set.getNumPoints(), 2 );
    QCOMPARE( set.x.at( 0 ), 1.0 );
    QCOMPARE( set.y.at( 0 ), 2.0 );
    QCOMPARE( set.x.at( 1 ), 9.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
  }

  {
    pal::PointSet set( 0, nullptr, nullptr );
    set.extendLineByDistance( 1, 3, 0 );
    QCOMPARE( set.getNumPoints(), 0 );
  }

  {
    QVector< double > x;
    x << 1;
    QVector< double > y;
    y << 2;
    pal::PointSet set( 1, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 0 );
    QCOMPARE( set.getNumPoints(), 1 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 0 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.292893, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.29289, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.121320, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), -0.121320, 0.00001 );
  }

  {
    QVector< double > x;
    x << 9 << 8 << 2 << 1;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 0 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 9.707107, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.29289, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 9.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 8.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 2.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 1.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), -1.121320, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), -0.121320, 0.00001 );
  }

  {
    // with averaging
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 0.5 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.292893, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.29289, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.573264, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), 0.457821, 0.00001 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 1 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.292893, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.29289, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.788722, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), 0.894094, 0.00001 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 2 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.011936, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.845957, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.917393, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), 1.300845, 0.00001 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 4 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.024713, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.779058, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.990524, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), 1.761739, 0.00001 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 5 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.040317, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.718915, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.998204, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ),  1.896217, 0.00001 );
  }

  {
    QVector< double > x;
    x << 1 << 2 << 8 << 9;
    QVector< double > y;
    y << 2 << 3 << 3 << 2;
    pal::PointSet set( 4, x.data(), y.data() );
    set.extendLineByDistance( 1, 3, 15 );
    QCOMPARE( set.getNumPoints(), 6 );
    QGSCOMPARENEAR( set.x.at( 0 ), 0.292893, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 0 ), 1.292893, 0.00001 );
    QCOMPARE( set.x.at( 1 ), 1.0 );
    QCOMPARE( set.y.at( 1 ), 2.0 );
    QCOMPARE( set.x.at( 2 ), 2.0 );
    QCOMPARE( set.y.at( 2 ), 3.0 );
    QCOMPARE( set.x.at( 3 ), 8.0 );
    QCOMPARE( set.y.at( 3 ), 3.0 );
    QCOMPARE( set.x.at( 4 ), 9.0 );
    QCOMPARE( set.y.at( 4 ), 2.0 );
    QGSCOMPARENEAR( set.x.at( 5 ), 11.982541, 0.00001 );
    QGSCOMPARENEAR( set.y.at( 5 ), 1.676812, 0.00001 );
  }
}

void TestQgsLabelingEngine::curvedOverrun()
{
  // test a small line with curved labels allows overruns when specified
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.labelPerPart = false;
  settings.lineSettings().setOverrunDistance( 0 );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190079.9 5000000.3, 190080 5000000, 190085 5000005, 190110 5000005)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_no_overrun" ), img, 20 ) );

  settings.lineSettings().setOverrunDistance( 11 );
  settings.maxCurvedCharAngleIn = 99;
  settings.maxCurvedCharAngleOut = 99;
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_overrun" ), img, 20 ) );

  // too short for what's required...
  settings.lineSettings().setOverrunDistance( 3 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_no_overrun" ), img, 20 ) );
}

void TestQgsLabelingEngine::parallelOverrun()
{
  // test a small line with curved labels allows overruns when specified
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.labelPerPart = false;
  settings.lineSettings().setOverrunDistance( 0 );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190079.9 5000000.3, 190080 5000000, 190085 5000005, 190110 5000005)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_no_overrun" ), img, 20 ) );

  settings.lineSettings().setOverrunDistance( 10 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_parallel_overrun" ), img, 20 ) );

  // too short for what's required...
  settings.lineSettings().setOverrunDistance( 3 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_curved_no_overrun" ), img, 20 ) );
}

void TestQgsLabelingEngine::testDataDefinedLabelAllParts()
{
  // test a small line with curved labels allows overruns when specified
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::OverPoint;
  settings.labelPerPart = false;

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "MultiPolygon?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsNullSymbolRenderer() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000000, 190080 5000000, 190084 5000000 )" ) ).buffer( 10, 5 ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << 2 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPoint (190030 5000060, 190080 5000060, 190084 5000060 )" ) ).buffer( 10, 5 ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelAllParts, QgsProperty::fromExpression( QStringLiteral( "\"id\" = 2" ) ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_datadefined_label_all_parts" ), img, 20 ) );
}

void TestQgsLabelingEngine::testDataDefinedPlacementPositionPoint()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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

  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::PositionPoint, QgsProperty::fromExpression( QStringLiteral( "translate($geometry, 1, 0.5)" ) ) );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "label_datadefined_placement_position_point", img, 20 ) );

  vl->setLabeling( nullptr );
}

void TestQgsLabelingEngine::testVerticalOrientation()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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
  QgsTextFormat format = settings.format();
  format.setOrientation( QgsTextFormat::VerticalOrientation );
  settings.setFormat( format );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_vertical", img, 20 ) );

  vl->setLabeling( nullptr );
}

void TestQgsLabelingEngine::testVerticalOrientationLetterLineSpacing()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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
  settings.fieldName = QStringLiteral( "\"Class\" || '\n' || \"Heading\"" );
  settings.isExpression = true;
  setDefaultLabelParams( settings );
  QgsTextFormat format = settings.format();
  format.setOrientation( QgsTextFormat::VerticalOrientation );
  format.setLineHeight( 1.5 );
  QFont font = format.font();
  font.setLetterSpacing( QFont::AbsoluteSpacing, 3.75 );
  format.setFont( font );
  settings.setFormat( format );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_vertical_letter_line_spacing", img, 20 ) );

  vl->setLabeling( nullptr );
}

void TestQgsLabelingEngine::testRotationBasedOrientationPoint()
{
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
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
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LabelRotation, QgsProperty::fromExpression( QStringLiteral( "\"Heading\"" ) ) );
  QgsTextFormat format = settings.format();
  format.setOrientation( QgsTextFormat::RotationBasedOrientation );
  settings.setFormat( format );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_rotation_based_orientation_point", img, 20 ) );

  vl->setLabeling( nullptr );
}

void TestQgsLabelingEngine::testRotationBasedOrientationLine()
{
  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
  QgsVectorLayer *vl2 = new QgsVectorLayer( filename, QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl2->isValid() );
  QgsProject::instance()->addMapLayer( vl2 );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2 );
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
  settings.fieldName = QStringLiteral( "'1234'" );
  settings.isExpression = true;
  setDefaultLabelParams( settings );
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  QgsTextFormat format = settings.format();
  format.setOrientation( QgsTextFormat::RotationBasedOrientation );
  settings.setFormat( format );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  QgsDefaultLabelingEngine engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl2, QString(), true, &settings ) );
  //engine.setFlags( QgsLabelingEngine::RenderOutlineLabels | QgsLabelingEngine::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  QVERIFY( imageCheck( "labeling_rotation_based_orientation_line", img, 20 ) );

  vl2->setLabeling( nullptr );
  QgsProject::instance()->removeMapLayer( vl2 );
}

void TestQgsLabelingEngine::testMapUnitLetterSpacing()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 50 );
  format.setSizeUnit( QgsUnitTypes::RenderMapUnits );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  QFont font = format.font();
  font.setLetterSpacing( QFont::AbsoluteSpacing, 30 );
  format.setFont( font );
  settings.setFormat( format );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190020 5000000, 190180 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_letter_spacing_map_units" ), img, 20 ) );
}

void TestQgsLabelingEngine::testMapUnitWordSpacing()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 50 );
  format.setSizeUnit( QgsUnitTypes::RenderMapUnits );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'X X'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  QFont font = format.font();
  font.setWordSpacing( 30 );
  format.setFont( font );
  settings.setFormat( format );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190020 5000000, 190180 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 190000, 5000000, 190200, 5000010 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_word_spacing_map_units" ), img, 20 ) );
}

void TestQgsLabelingEngine::testReferencedFields()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "hello+world" );
  settings.isExpression = false;

  QCOMPARE( settings.referencedFields( QgsRenderContext() ), QSet<QString>() << QStringLiteral( "hello+world" ) );

  settings.isExpression = true;

  QCOMPARE( settings.referencedFields( QgsRenderContext() ), QSet<QString>() << QStringLiteral( "hello" ) << QStringLiteral( "world" ) );

  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::Size, QgsProperty::fromField( QStringLiteral( "my_dd_size" ) ) );

  QCOMPARE( settings.referencedFields( QgsRenderContext() ), QSet<QString>() << QStringLiteral( "hello" ) << QStringLiteral( "world" ) << QStringLiteral( "my_dd_size" ) );
}

void TestQgsLabelingEngine::testClipping()
{
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 12 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "Name" );
  settings.placement = QgsPalLayerSettings::Line;

  const QString filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( filename, QStringLiteral( "lines" ), QStringLiteral( "ogr" ) ) );

  QVariantMap props;
  props.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#487bb6" ) );
  props.insert( QStringLiteral( "outline_width" ), QStringLiteral( "1" ) );
  std::unique_ptr< QgsLineSymbol > symbol( QgsLineSymbol::createSimple( props ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( symbol.release() ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -117.543, 49.438, -82.323, 21.839 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapClippingRegion region1( QgsGeometry::fromWkt( "Polygon ((-92 45, -99 36, -94 29, -82 29, -81 45, -92 45))" ) );
  region1.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection );
  mapSettings.addClippingRegion( region1 );

  QgsMapClippingRegion region2( QgsGeometry::fromWkt( "Polygon ((-85 36, -85 46, -107 47, -108 28, -85 28, -85 36))" ) );
  region2.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly );
  mapSettings.addClippingRegion( region2 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  //engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_feature_clipping" ), img, 20 ) );

  // also check with symbol levels
  vl2->renderer()->setUsingSymbolLevels( true );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "label_feature_clipping" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorParallel()
{
  // test line label anchor with parallel labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_anchor_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorParallelConstraints()
{
  // test line label anchor with parallel labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::HintOnly );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000000, 190010 5000010, 190190 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_hint_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_hint_anchor_end" ), img, 20 ) );

  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job4( mapSettings );
  job4.start();
  job4.waitForFinished();

  img = job4.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_end" ), img, 20 ) );

  settings.fieldName = QStringLiteral( "'XXXXX'" );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setLineAnchorPercent( 0.20 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job5( mapSettings );
  job5.start();
  job5.waitForFinished();

  img = job5.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_20_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job6( mapSettings );
  job6.start();
  job6.waitForFinished();

  img = job6.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_20_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job7( mapSettings );
  job7.start();
  job7.waitForFinished();

  img = job7.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_20_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job8( mapSettings );
  job8.start();
  job8.waitForFinished();

  img = job8.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_20_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 0.40 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job9( mapSettings );
  job9.start();
  job9.waitForFinished();

  img = job9.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_40_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job10( mapSettings );
  job10.start();
  job10.waitForFinished();

  img = job10.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_40_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job11( mapSettings );
  job11.start();
  job11.waitForFinished();

  img = job11.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_40_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job12( mapSettings );
  job12.start();
  job12.waitForFinished();

  img = job12.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_40_center" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 0.80 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job13( mapSettings );
  job13.start();
  job13.waitForFinished();

  img = job13.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_80_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job14( mapSettings );
  job14.start();
  job14.waitForFinished();

  img = job14.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_80_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job15( mapSettings );
  job15.start();
  job15.waitForFinished();

  img = job15.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_80_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job16( mapSettings );
  job16.start();
  job16.waitForFinished();

  img = job16.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_80_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorDataDefinedType()
{
  // test data defined line anchor types
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  // override hint by strict!
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::HintOnly );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LineAnchorType, QgsProperty::fromExpression( QStringLiteral( "'strict'" ) ) );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000000, 190010 5000010, 190190 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_strict_anchor_start" ), img, 20 ) );

  // override strict by hint
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.dataDefinedProperties().setProperty( QgsPalLayerSettings::LineAnchorType, QgsProperty::fromExpression( QStringLiteral( "'hi' || 'nt'" ) ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "parallel_hint_anchor_start" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorCurved()
{
  // test line label anchor with curved labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_anchor_end" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 0.3 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_anchor_30_above" ), img, 20 ) );

  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::BelowLine );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job4( mapSettings );
  job4.start();
  job4.waitForFinished();

  img = job4.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_anchor_30_below" ), img, 20 ) );

}

void TestQgsLabelingEngine::testLineAnchorCurvedConstraints()
{
  // test line label anchor with parallel labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::HintOnly );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000000, 190010 5000010, 190190 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_hint_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_hint_anchor_end" ), img, 20 ) );

  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job4( mapSettings );
  job4.start();
  job4.waitForFinished();

  img = job4.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_end" ), img, 20 ) );

  settings.fieldName = QStringLiteral( "'XXXXX'" );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setLineAnchorPercent( 0.20 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job5( mapSettings );
  job5.start();
  job5.waitForFinished();

  img = job5.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_20_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job6( mapSettings );
  job6.start();
  job6.waitForFinished();

  img = job6.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_20_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job7( mapSettings );
  job7.start();
  job7.waitForFinished();

  img = job7.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_20_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job8( mapSettings );
  job8.start();
  job8.waitForFinished();

  img = job8.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_20_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 0.40 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job9( mapSettings );
  job9.start();
  job9.waitForFinished();

  img = job9.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_40_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job10( mapSettings );
  job10.start();
  job10.waitForFinished();

  img = job10.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_40_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job11( mapSettings );
  job11.start();
  job11.waitForFinished();

  img = job11.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_40_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job12( mapSettings );
  job12.start();
  job12.waitForFinished();

  img = job12.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_40_center" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 0.80 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job13( mapSettings );
  job13.start();
  job13.waitForFinished();

  img = job13.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_80_center" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job14( mapSettings );
  job14.start();
  job14.waitForFinished();

  img = job14.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_80_start" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job15( mapSettings );
  job15.start();
  job15.waitForFinished();

  img = job15.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_80_end" ), img, 20 ) );

  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::FollowPlacement );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job16( mapSettings );
  job16.start();
  job16.waitForFinished();

  img = job16.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_80_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorCurvedOverrun()
{
  // strict anchor type should imply sufficient overrun to fit label in, regardless of where it's placed
  // test line label anchor with parallel labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;

  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::EndOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000000, 190010 5000010, 190190 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 100 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_overrun_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_overrun_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorCurvedStrictAllUpsideDown()
{
  // test curved labels with strict placement, when no upright candidates can be generated
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Curved;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine | QgsLabeling::LinePlacementFlag::MapOrientation );
  settings.labelPerPart = false;
  settings.upsidedownLabels = QgsPalLayerSettings::Upright;

  settings.lineSettings().setLineAnchorPercent( 0.5 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::StartOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190088.74697824331815355 4999955.67284447979182005, 190121.74456083800760098 5000011.17647058796137571, 190090.26994359385571443 5000072.94117647036910057)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 100 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "curved_strict_anchor_all_upside_down" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorHorizontal()
{
  // test line label anchor with horizontal labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'XXXXXXXX'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Horizontal;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190000 5000010, 190200 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 10 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_anchor_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorHorizontalConstraints()
{
  // test line label anchor with parallel labels
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "l" );
  settings.isExpression = false;
  settings.placement = QgsPalLayerSettings::Horizontal;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::HintOnly );
  settings.lineSettings().setAnchorTextPoint( QgsLabelLineSettings::AnchorTextPoint::CenterOfText );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=l:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << QVariant() );

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190030 5000000, 190030 5000010)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190170 5000000, 190170 5000010)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setAttributes( QgsAttributes() << QStringLiteral( "XXXXXXXX" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (190020 5000000, 190030 5000010, 190170 5000010, 190190 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( f.geometry().boundingBox().buffered( 40 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_hint_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_hint_anchor_end" ), img, 20 ) );

  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setLineAnchorPercent( 0.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job3( mapSettings );
  job3.start();
  job3.waitForFinished();

  img = job3.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_strict_anchor_start" ), img, 20 ) );

  settings.lineSettings().setLineAnchorPercent( 1.0 );
  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  QgsMapRendererSequentialJob job4( mapSettings );
  job4.start();
  job4.waitForFinished();

  img = job4.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "horizontal_strict_anchor_end" ), img, 20 ) );
}

void TestQgsLabelingEngine::testLineAnchorClipping()
{
  // test line label anchor with no clipping
  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'x'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Horizontal;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::AboveLine );
  settings.labelPerPart = false;
  settings.lineSettings().setLineAnchorPercent( 0.5 );
  settings.lineSettings().setAnchorType( QgsLabelLineSettings::AnchorType::Strict );
  settings.lineSettings().setAnchorClipping( QgsLabelLineSettings::AnchorClipping::UseEntireLine );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3946&field=l:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#ff0000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (189950 5000000, 190100 5000000)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( 189999, 4999999, 190101, 5000001 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::UsePartialCandidates, false );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "line_anchor_no_clipping" ), img, 20 ) );
}

void TestQgsLabelingEngine::testShowAllLabelsWhenALabelHasNoCandidates()
{
  // test that showing all labels when a label has no candidate placements doesn't
  // result in a crash
  // refs https://github.com/qgis/QGIS/issues/38093

  QgsPalLayerSettings settings;
  setDefaultLabelParams( settings );

  QgsTextFormat format = settings.format();
  format.setSize( 20 );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = QStringLiteral( "'xxxxxxxxxxxxxx'" );
  settings.isExpression = true;
  settings.placement = QgsPalLayerSettings::Line;
  settings.lineSettings().setPlacementFlags( QgsLabeling::LinePlacementFlag::OnLine );
  settings.obstacleSettings().setFactor( 10 );
  settings.lineSettings().setOverrunDistance( 50 );

  std::unique_ptr< QgsVectorLayer> vl2( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:23700&field=l:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  vl2->setRenderer( new QgsSingleSymbolRenderer( QgsLineSymbol::createSimple( { {QStringLiteral( "color" ), QStringLiteral( "#000000" )}, {QStringLiteral( "outline_width" ), 0.6} } ) ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << QVariant() );

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (-2446233 -5204828, -2342845 -5203825)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString (-2439207 -5198806, -2331302 -5197802)" ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );  // TODO: this should not be necessary!
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setLabelingEngineSettings( createLabelEngineSettings() );
  mapSettings.setDestinationCrs( vl2->crs() );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( QgsRectangle( -3328044.9, -5963176., -1127782.7, -4276844.3 ) );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsLabelingEngineSettings engineSettings = mapSettings.labelingEngineSettings();
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawLabelRectOnly, true );
  engineSettings.setFlag( QgsLabelingEngineSettings::UseAllLabels, true );
  engineSettings.setFlag( QgsLabelingEngineSettings::DrawUnplacedLabels, true );
  // engineSettings.setFlag( QgsLabelingEngineSettings::DrawCandidates, true );
  mapSettings.setLabelingEngineSettings( engineSettings );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "show_all_labels_when_no_candidates" ), img, 20 ) );
}

void TestQgsLabelingEngine::testSymbologyScalingFactor()
{
  // test rendering labels with a layer with a reference scale set (with callout)
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( TEST_DATA_DIR ) + "/points.shp", QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );

  vl->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = QgsPalLayerSettings::AroundPoint;
  settings.dist = 7;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );

  format.buffer().setEnabled( true );
  format.buffer().setSize( 3 );
  format.buffer().setColor( QColor( 200, 255, 200 ) );

  settings.setFormat( format );

  QgsBalloonCallout *callout = new QgsBalloonCallout();
  callout->setEnabled( true );
  callout->setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  callout->setMargins( QgsMargins( 2, 2, 2, 2 ) );

  settings.setCallout( callout );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "labeling_reference_scale_not_set" ), img, 20 ) );

  // with reference scale set
  vl->renderer()->setReferenceScale( mapSettings.scale() * 2 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "labeling_reference_scale_set" ), img, 20 ) );
}

void TestQgsLabelingEngine::testSymbologyScalingFactor2()
{
  // test rendering labels with a layer with a reference scale set (with label background)
  std::unique_ptr< QgsVectorLayer > vl = std::make_unique< QgsVectorLayer >( QStringLiteral( TEST_DATA_DIR ) + "/points.shp", QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( vl->isValid() );
  QgsMarkerSymbol *marker = static_cast< QgsMarkerSymbol * >( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  marker->setColor( QColor( 255, 0, 0 ) );
  marker->setSize( 3 );
  static_cast< QgsSimpleMarkerSymbolLayer * >( marker->symbolLayer( 0 ) )->setStrokeStyle( Qt::NoPen );

  vl->setRenderer( new QgsSingleSymbolRenderer( marker ) );

  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl.get() );
  mapSettings.setOutputDpi( 96 );

  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.placement = QgsPalLayerSettings::AroundPoint;
  settings.dist = 7;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );

  format.buffer().setEnabled( true );
  format.buffer().setSize( 3 );
  format.buffer().setColor( QColor( 200, 255, 200 ) );

  format.background().setEnabled( true );
  format.background().setFillSymbol( QgsFillSymbol::createSimple( QVariantMap( { { "color", "#ffcccc"},
    { "outline-width", "1"}
  } ) ) );
  format.background().setSize( QSizeF( 2, 3 ) );

  settings.setFormat( format );

  vl->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl->setLabelsEnabled( true );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "labeling_reference_scale2_not_set" ), img, 20 ) );

  // with reference scale set
  vl->renderer()->setReferenceScale( mapSettings.scale() * 2 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QVERIFY( imageCheck( QStringLiteral( "labeling_reference_scale2_set" ), img, 20 ) );
}

QGSTEST_MAIN( TestQgsLabelingEngine )
#include "testqgslabelingengine.moc"
