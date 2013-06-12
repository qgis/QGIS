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
#include "qgsmultibandcolorrenderer.h"
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
    void grid(); //test if grid and grid annotation works
    void overviewMap(); //test if overview map frame works
    void overviewMapBlending(); //test if blend modes with overview map frame works
    void overviewMapInvert(); //test if invert of overview map frame works
    void uniqueId(); //test if map id is adapted when doing copy paste
    void zebraStyle(); //test zebra map border style
    void overviewMapCenter(); //test if centering of overview map frame works
    void worldFileGeneration(); // test world file generation

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapRenderer* mMapRenderer;
    QgsRasterLayer* mRasterLayer;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  //create composition with composer map
  mMapRenderer = new QgsMapRenderer();
  mMapRenderer->setLayerSet( QStringList() << mRasterLayer->id() );
  mMapRenderer->setProjectionsEnabled( false );
  mComposition = new QgsComposition( mMapRenderer );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );
}

void TestQgsComposerMap::cleanupTestCase()
{
  delete mComposition;
  delete mMapRenderer;
  delete mRasterLayer;
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
  QgsCompositionChecker checker( "Composer map render", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_render.png" ) );
  QVERIFY( checker.testComposition() );
}

void TestQgsComposerMap::grid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridIntervalX( 2000 );
  mComposerMap->setGridIntervalY( 2000 );
  mComposerMap->setShowGridAnnotation( true );
  mComposerMap->setGridPenWidth( 0.5 );
  mComposerMap->setGridPenColor( QColor( 0, 255, 0 ) );
  mComposerMap->setGridAnnotationPrecision( 0 );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Bottom );
  mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, QgsComposerMap::Bottom );
  mComposerMap->setAnnotationFontColor( QColor( 255, 0, 0, 150 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_Overlay );
  qWarning() << "grid annotation font: " << mComposerMap->gridAnnotationFont().toString() << " exactMatch:" << mComposerMap->gridAnnotationFont().exactMatch();
  QgsCompositionChecker checker( "Composer map grid", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_grid.png" ) );
  bool testResult = checker.testComposition();
  mComposerMap->setGridEnabled( false );
  mComposerMap->setShowGridAnnotation( false );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMap()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMap->setOverviewFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "Composer map overview", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_overview.png" ) );
  bool testResult = checker.testComposition();
  mComposition->removeComposerItem( overviewMap );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapBlending()
{
  QgsComposerMap* overviewMapBlend = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapBlend->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapBlend );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMapBlend->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapBlend->setOverviewFrameMap( mComposerMap->id() );
  overviewMapBlend->setOverviewBlendMode( QPainter::CompositionMode_Multiply );

  QgsCompositionChecker checker( "Composer map overview blending", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_overview_blend.png" ) );
  bool testResult = checker.testComposition();
  mComposition->removeComposerItem( overviewMapBlend );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapInvert()
{
  QgsComposerMap* overviewMapInvert = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapInvert->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapInvert );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMapInvert->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapInvert->setOverviewFrameMap( mComposerMap->id() );
  overviewMapInvert->setOverviewInverted( true );

  QgsCompositionChecker checker( "Composer map overview invert", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_overview_invert.png" ) );
  bool testResult = checker.testComposition();
  mComposition->removeComposerItem( overviewMapInvert );
  QVERIFY( testResult );
}

void TestQgsComposerMap::uniqueId()
{
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( "ComposerItemClipboard" );
  mComposerMap->writeXML( documentElement, doc );
  mComposition->addItemsFromXML( documentElement, doc, 0, false );

  //test if both composer maps have different ids
  const QgsComposerMap* newMap = 0;
  QList<const QgsComposerMap*> mapList = mComposition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapIt = mapList.constBegin();
  for ( ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if ( *mapIt != mComposerMap )
    {
      newMap = *mapIt;
      break;
    }
  }
  int oldId = mComposerMap->id();
  int newId = newMap->id();

  mComposition->removeComposerItem( const_cast<QgsComposerMap*>( newMap ) );

  QVERIFY( oldId != newId );
}

void TestQgsComposerMap::zebraStyle()
{
  mComposerMap->setGridPenColor( QColor( 0, 0, 0 ) );
  mComposerMap->setAnnotationFontColor( QColor( 0, 0, 0, 0 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );

  mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  mComposerMap->setGridEnabled( true );

  QgsCompositionChecker checker( "Composer map zebra", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_zebra_style.png" ) );
  bool testResult = checker.testComposition();
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapCenter()
{
  QgsComposerMap* overviewMapCenter = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapCenter->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapCenter );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375+5000, 3341423.125, 789262.375+5000, 3343323.125 ) ); //zoom in
  mComposerMap->setGridEnabled( false );
  overviewMapCenter->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapCenter->setOverviewFrameMap( mComposerMap->id() );
  overviewMapCenter->setOverviewCentered( true );

  QgsCompositionChecker checker( "Composer map overview center", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composermap" + QDir::separator() + "composermap_landsat_overview_center.png" ) );
  bool testResult = checker.testComposition();
  mComposition->removeComposerItem( overviewMapCenter );
  QVERIFY( testResult );
}

void TestQgsComposerMap::worldFileGeneration()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setRotation( 30.0 );

  mComposition->setGenerateWorldFile( true );
  mComposition->setWorldFileMap( mComposerMap );

  double params[6];
  mComposition->computeWorldFileParameters( params );

  QVERIFY( fabs(params[0] - 4.18048) < 0.001 );
  QVERIFY( fabs(params[1] - 2.41331) < 0.001 );
  QVERIFY( fabs(params[2] - 779444) < 1 );
  QVERIFY( fabs(params[3] - 2.4136) < 0.001 );
  QVERIFY( fabs(params[4] + 4.17997) < 0.001 );
  QVERIFY( fabs(params[5] - 3.34241e+06) < 1e+03 );
}

QTEST_MAIN( TestQgsComposerMap )
#include "moc_testqgscomposermap.cxx"
