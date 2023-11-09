/***************************************************************************
  testqgsrastercontourrenderer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
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
#include <QObject>
#include <QString>

//qgis includes...
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrastercontourrenderer.h"
#include "qgslinesymbollayer.h"
#include "qgslinesymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for contour renderer
 */
class TestQgsRasterContourRenderer : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterContourRenderer() : QgsTest( QStringLiteral( "Raster Contour Renderer Tests" ) ) {}

  private:
    QString mDataDir;
    QgsRasterLayer *mLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_render();
    void testRenderOpacity();
};


void TestQgsRasterContourRenderer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/analysis";

  mLayer = new QgsRasterLayer( mDataDir + "/dem.tif", "dem", "gdal" );

  QgsProject::instance()->addMapLayer( mLayer );

  mMapSettings = new QgsMapSettings();
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLayer );
  mMapSettings->setExtent( mLayer->extent().scaled( 0.5 ) );
  mMapSettings->setDestinationCrs( mLayer->crs() );
  mMapSettings->setOutputDpi( 96 );
}

void TestQgsRasterContourRenderer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterContourRenderer::test_render()
{
  QgsSimpleLineSymbolLayer *slsl1 = new QgsSimpleLineSymbolLayer( Qt::black, 0.5 );
  QgsLineSymbol *contourSymbol = new QgsLineSymbol( QgsSymbolLayerList() << slsl1 );
  QgsSimpleLineSymbolLayer *slsl2 = new QgsSimpleLineSymbolLayer( Qt::black, 1 );
  QgsLineSymbol *contourIndexSymbol = new QgsLineSymbol( QgsSymbolLayerList() << slsl2 );

  QgsRasterContourRenderer *renderer = new QgsRasterContourRenderer( mLayer->dataProvider() );
  renderer->setContourSymbol( contourSymbol );
  renderer->setContourInterval( 100 );
  renderer->setContourIndexSymbol( contourIndexSymbol );
  renderer->setContourIndexInterval( 500 );
  renderer->setDownscale( 10 );

  mLayer->setRenderer( renderer );

  QVERIFY( renderMapSettingsCheck( QStringLiteral( "raster_contours" ), QStringLiteral( "raster_contours" ), *mMapSettings ) );
}

void TestQgsRasterContourRenderer::testRenderOpacity()
{
  // make sure renderer respects layer opacity
  QgsSimpleLineSymbolLayer *slsl1 = new QgsSimpleLineSymbolLayer( Qt::black, 0.5 );
  QgsLineSymbol *contourSymbol = new QgsLineSymbol( QgsSymbolLayerList() << slsl1 );
  QgsSimpleLineSymbolLayer *slsl2 = new QgsSimpleLineSymbolLayer( Qt::black, 1 );
  QgsLineSymbol *contourIndexSymbol = new QgsLineSymbol( QgsSymbolLayerList() << slsl2 );

  QgsRasterContourRenderer *renderer = new QgsRasterContourRenderer( mLayer->dataProvider() );
  renderer->setContourSymbol( contourSymbol );
  renderer->setContourInterval( 100 );
  renderer->setContourIndexSymbol( contourIndexSymbol );
  renderer->setContourIndexInterval( 500 );
  renderer->setDownscale( 10 );

  mLayer->setRenderer( renderer );
  mLayer->setOpacity( 0.5 );

  QVERIFY( renderMapSettingsCheck( QStringLiteral( "raster_contours_opacity" ), QStringLiteral( "raster_contours_opacity" ), *mMapSettings ) );
}


QGSTEST_MAIN( TestQgsRasterContourRenderer )
#include "testqgsrastercontourrenderer.moc"
