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
#include <qgsvectorlayer.h>
#include <qgsvectorlayerdiagramprovider.h>
#include <qgsvectorlayerlabelprovider.h>

class TestQgsLabelingEngineV2 : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testBasic();
    void testDiagrams();

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

  QgsVectorLayerLabelProvider lp( vl );
  QgsLabelingEngineV2 engine( mapSettings, QList<QgsAbstractLabelProvider*>() << &lp );
  //engine.setFlags( QgsLabelingEngineV2::RenderOutlineLabels | QgsLabelingEngineV2::DrawLabelRectOnly );
  engine.run( context );

  p.end();

  // TODO: replace with render checker
  img.save( "/tmp/tstlabels.png" );

  vl->setCustomProperty( "labeling/enabled", false );
}

void TestQgsLabelingEngineV2::testDiagrams()
{
  QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl->extent() );
  mapSettings.setLayers( QStringList() << vl->id() );

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

  QgsVectorLayerDiagramProvider dp( vl );
  QgsLabelingEngineV2 engine( mapSettings, QList<QgsAbstractLabelProvider*>() << &dp );
  engine.run( context );

  p.end();

  // TODO: replace with render checker
  img.save( "/tmp/tstdiagrams.png" );
}

QTEST_MAIN( TestQgsLabelingEngineV2 )
#include "testqgslabelingenginev2.moc"
