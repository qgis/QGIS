/***************************************************************************
                         testqgscomposermapoverview.cpp
                         ----------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
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
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgscomposermap.h"
#include "qgscomposermapoverview.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerMapOverview : public QObject
{
    Q_OBJECT
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

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapSettings mMapSettings;
    QgsRasterLayer* mRasterLayer;
    QString mReport;
};

void TestQgsComposerMapOverview::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "rgb256x256.png" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  mRasterLayer->setRenderer( rasterRenderer );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mRasterLayer->id() );
  mMapSettings.setCrsTransformEnabled( false );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  mReport = "<h1>Composer Map Overview Tests</h1>\n";
}

void TestQgsComposerMapOverview::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
  QgsApplication::exitQgis();
}

void TestQgsComposerMapOverview::init()
{
}

void TestQgsComposerMapOverview::cleanup()
{

}

void TestQgsComposerMapOverview::overviewMap()
{
  QgsComposerMap* overviewMap =  new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMap->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMap );
  QVERIFY( testResult );
}

void TestQgsComposerMapOverview::overviewMapRotated()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 96, -144, 160, -112 ) ); //zoom in
  mComposerMap->setMapRotation( 30 );
  overviewMap->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview_rotated", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMap );
  mComposerMap->setMapRotation( 0 );
  QVERIFY( testResult );
}

void TestQgsComposerMapOverview::overviewMapRotated2()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMap->setMapRotation( 30 );
  overviewMap->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMap->overview()->setFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview_rotated2", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMap );
  QVERIFY( testResult );
}

void TestQgsComposerMapOverview::overviewMapBlending()
{
  QgsComposerMap* overviewMapBlend = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapBlend->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapBlend );
  mComposerMap->setNewExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMapBlend->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapBlend->overview()->setFrameMap( mComposerMap->id() );
  overviewMapBlend->overview()->setBlendMode( QPainter::CompositionMode_Multiply );

  QgsCompositionChecker checker( "composermap_overview_blending", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMapBlend );
  QVERIFY( testResult );
}

void TestQgsComposerMapOverview::overviewMapInvert()
{
  QgsComposerMap* overviewMapInvert = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapInvert->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapInvert );
  mComposerMap->setNewExtent( QgsRectangle( 96, -152, 160, -120 ) ); //zoom in
  overviewMapInvert->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapInvert->overview()->setFrameMap( mComposerMap->id() );
  overviewMapInvert->overview()->setInverted( true );

  QgsCompositionChecker checker( "composermap_overview_invert", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMapInvert );
  QVERIFY( testResult );
}

void TestQgsComposerMapOverview::overviewMapCenter()
{
  QgsComposerMap* overviewMapCenter = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapCenter->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapCenter );
  mComposerMap->setNewExtent( QgsRectangle( 192, -288, 320, -224 ) );
  overviewMapCenter->setNewExtent( QgsRectangle( 0, -256, 256, 0 ) );
  overviewMapCenter->overview()->setFrameMap( mComposerMap->id() );
  overviewMapCenter->overview()->setCentered( true );

  QgsCompositionChecker checker( "composermap_overview_center", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposition->removeComposerItem( overviewMapCenter );
  QVERIFY( testResult );
}

QTEST_MAIN( TestQgsComposerMapOverview )
#include "testqgscomposermapoverview.moc"
