/***************************************************************************
                         testqgslayoutmapoverview.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
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
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapoverview.h"
#include "qgsproject.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsfontutils.h"
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutMapOverview : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutMapOverview() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void overviewMap(); //test if overview map frame works
    void overviewMapRotated(); //test if overview map frame works with rotated overview
    void overviewMapRotated2(); //test if overview map frame works with rotated map
    void overviewMapBlending(); //test if blend modes with overview map frame works
    void overviewMapInvert(); //test if invert of overview map frame works
    void overviewMapCenter(); //test if centering of overview map frame works
    void overviewReprojected(); //test that overview frame is reprojected

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
    QString mReport;
};

void TestQgsLayoutMapOverview::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  const QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  mRasterLayer->setRenderer( rasterRenderer );
  mReport = QStringLiteral( "<h1>Composer Map Overview Tests</h1>\n" );
}

void TestQgsLayoutMapOverview::cleanupTestCase()
{
  delete mRasterLayer;

  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
}

void TestQgsLayoutMapOverview::init()
{
}

void TestQgsLayoutMapOverview::cleanup()
{

}

void TestQgsLayoutMapOverview::overviewMap()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMap =  new QgsLayoutItemMap( &l );
  overviewMap->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMap->setFrameEnabled( true );
  overviewMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMap );
  map->setExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMap->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setLinkedMap( map );

  QCOMPARE( overviewMap->overview()->linkedMap(), map );
  overviewMap->overview()->setLinkedMap( nullptr );
  QVERIFY( !overviewMap->overview()->linkedMap() );
  overviewMap->overview()->setLinkedMap( map );
  QCOMPARE( overviewMap->overview()->linkedMap(), map );
  overviewMap->overview()->setLinkedMap( nullptr );
  QVERIFY( !overviewMap->overview()->linkedMap() );

  //render
  overviewMap->overview()->setLinkedMap( map );
  QgsLayoutChecker checker( QStringLiteral( "composermap_overview" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewMapRotated()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMap = new QgsLayoutItemMap( &l );
  overviewMap->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMap->setFrameEnabled( true );
  overviewMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMap );
  map->setExtent( QgsRectangle( 96, -144, 160, -112 ) ); //zoom in
  map->setMapRotation( 30 );
  overviewMap->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setLinkedMap( map );
  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_rotated" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 600 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewMapRotated2()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMap = new QgsLayoutItemMap( &l );
  overviewMap->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMap->setFrameEnabled( true );
  overviewMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMap );
  map->setExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMap->setMapRotation( 30 );
  overviewMap->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setLinkedMap( map );
  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_rotated2" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 600 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewMapBlending()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMapBlend = new QgsLayoutItemMap( &l );
  overviewMapBlend->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMapBlend->setFrameEnabled( true );
  overviewMapBlend->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMapBlend );
  map->setExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMapBlend->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapBlend->overview()->setLinkedMap( map );
  overviewMapBlend->overview()->setBlendMode( QPainter::CompositionMode_Multiply );

  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_blending" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewMapInvert()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMapInvert = new QgsLayoutItemMap( &l );
  overviewMapInvert->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMapInvert->setFrameEnabled( true );
  overviewMapInvert->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMapInvert );
  map->setExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMapInvert->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapInvert->overview()->setLinkedMap( map );
  overviewMapInvert->overview()->setInverted( true );

  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_invert" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewMapCenter()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMapCenter = new QgsLayoutItemMap( &l );
  overviewMapCenter->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMapCenter->setFrameEnabled( true );
  overviewMapCenter->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMapCenter );
  map->setExtent( QgsRectangle( 192, -288, 320, -224 ) );
  overviewMapCenter->setExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapCenter->overview()->setLinkedMap( map );
  overviewMapCenter->overview()->setCentered( true );

  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_center" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapOverview::overviewReprojected()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  QgsLayoutItemMap *overviewMap = new QgsLayoutItemMap( &l );
  overviewMap->attemptSetSceneRect( QRectF( 20, 130, 70, 70 ) );
  overviewMap->setFrameEnabled( true );
  //overviewMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( overviewMap );

  map->setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) );
  overviewMap->setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 54030 ) );

  map->setExtent( QgsRectangle( 93, -64.245, 120.6, -45 ) );
  overviewMap->setExtent( QgsRectangle( 4712502, -7620278, 10872777, -2531356 ) );
  overviewMap->overview()->setLinkedMap( map );

  QgsLayoutChecker checker( QStringLiteral( "composermap_overview_reprojected" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapoverview" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

QGSTEST_MAIN( TestQgsLayoutMapOverview )
#include "testqgslayoutmapoverview.moc"
