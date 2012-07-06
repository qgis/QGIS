/***************************************************************************
                         testqgscomposermap.cpp
                         ----------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgscomposermap.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsrasterlayer.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerMap: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void render(); //test if rendering of the composition with composr map is correct

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapRenderer* mMapRenderer;
    QgsRasterLayer* mRasterLayer;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init( QString() );
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  //create composition with composer map
  mMapRenderer = new QgsMapRenderer();
  mMapRenderer->setLayerSet( QStringList() << mRasterLayer->id() );
  mMapRenderer->setProjectionsEnabled( false );
  mComposition = new QgsComposition( mMapRenderer );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposition->addComposerMap( mComposerMap );
}

void TestQgsComposerMap::cleanupTestCase()
{
  delete mComposition;
  delete mMapRenderer;
}

void TestQgsComposerMap::init()
{

}

void TestQgsComposerMap::cleanup()
{

}

void TestQgsComposerMap::render()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsCompositionChecker checker( mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "composermap_landsat1.png" ) );
  QVERIFY( checker.testComposition() );
}

QTEST_MAIN( TestQgsComposerMap )
#include "moc_testqgscomposermap.cxx"
