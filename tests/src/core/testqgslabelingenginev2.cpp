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

class TestQgsLabelingEngineV2 : public QObject
{
    Q_OBJECT
  public:
    TestQgsLabelingEngineV2() : vl( 0 ) {}
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testBasic();
    void testDiagrams();
    void testRuleBased();

  private:
    QgsVectorLayer* vl;
};

void TestQgsLabelingEngineV2::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QString filename = QString( TEST_DATA_DIR ) + "/points.shp";

  vl = new QgsVectorLayer( filename, "points", "ogr" );
  Q_ASSERT( vl->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( vl );
}

void TestQgsLabelingEngineV2::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsLabelingEngineV2::testBasic()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );

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

  QgsLabelingEngineV2 engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsVectorLayerLabelProvider( vl ) );
  //engine.setFlags( QgsLabelingEngineV2::RenderOutlineLabels | QgsLabelingEngineV2::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  // now let's test the variant when integrated into rendering loop

  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  vl->setCustomProperty( "labeling/enabled", false );

  QCOMPARE( img, img2 );
}

void TestQgsLabelingEngineV2::testDiagrams()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );

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

  // now let's test the variant when integrated into rendering loop
  job.start();
  job.waitForFinished();
  QImage img2 = job.renderedImage();

  QCOMPARE( img, img2 );

  vl->loadDefaultStyle( res );
}


void TestQgsLabelingEngineV2::testRuleBased()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );

  // set up most basic rule-based labeling for layer
  QgsRuleBasedLabeling::Rule* root = new QgsRuleBasedLabeling::Rule( 0 );

  QgsPalLayerSettings s1;
  s1.enabled = true;
  s1.fieldName = "Class";
  s1.obstacle = false;
  s1.dist = 2;
  s1.distInMapUnits = false;
  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s1 ) ) );

  QgsPalLayerSettings s2;
  s2.enabled = true;
  s2.fieldName = "Class";
  s2.obstacle = false;
  s2.dist = 2;
  s2.textColor = Qt::red;
  s2.setDataDefinedProperty( QgsPalLayerSettings::Size, true, true, "18", QString() );

  root->appendChild( new QgsRuleBasedLabeling::Rule( new QgsPalLayerSettings( s2 ), 0, 0, "Class = 'Jet'" ) );

  vl->setLabeling( new QgsRuleBasedLabeling( root ) );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();
  QImage img = job.renderedImage();

  img.save( "/tmp/rules.png" );

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

  /*
  QPainter p( &img );
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  context.setPainter( &p );

  QgsLabelingEngineV2 engine;
  engine.setMapSettings( mapSettings );
  engine.addProvider( new QgsRuleBasedLabelProvider( , vl ) );
  engine.run( context );*/

}

QTEST_MAIN( TestQgsLabelingEngineV2 )
#include "testqgslabelingenginev2.moc"
