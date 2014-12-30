/***************************************************************************
                         testqgscomposerscalebar.cpp
                         ---------------------------
    begin                : August 2012
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
#include "qgscomposerscalebar.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerScaleBar : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void singleBox();
    void singleBoxAlpha();
    void doubleBox();
    void numeric();
    void tick();

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsComposerScaleBar* mComposerScaleBar;
    QgsRasterLayer* mRasterLayer;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerScaleBar::initTestCase()
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
  mMapSettings.setLayers( QStringList() << mRasterLayer->id() );

  //reproject to WGS84
  QgsCoordinateReferenceSystem destCRS;
  destCRS.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  mMapSettings.setDestinationCrs( destCRS );
  mMapSettings.setCrsTransformEnabled( true );

  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 150, 150 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  mComposerScaleBar = new QgsComposerScaleBar( mComposition );
  mComposerScaleBar->setSceneRect( QRectF( 20, 180, 50, 20 ) );
  mComposition->addComposerScaleBar( mComposerScaleBar );
  mComposerScaleBar->setComposerMap( mComposerMap );
  mComposerScaleBar->setFont( QgsFontUtils::getStandardTestFont() );
  mComposerScaleBar->setUnits( QgsComposerScaleBar::Meters );
  mComposerScaleBar->setNumUnitsPerSegment( 2000 );
  mComposerScaleBar->setNumSegmentsLeft( 0 );
  mComposerScaleBar->setNumSegments( 2 );
  mComposerScaleBar->setHeight( 5 );


  qWarning() << "scalebar font: " << mComposerScaleBar->font().toString() << " exactMatch:" << mComposerScaleBar->font().exactMatch();

  mReport = "<h1>Composer Scalebar Tests</h1>\n";
}

void TestQgsComposerScaleBar::cleanupTestCase()
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
};

void TestQgsComposerScaleBar::init()
{

};

void TestQgsComposerScaleBar::cleanup()
{

};

void TestQgsComposerScaleBar::singleBox()
{
  mComposerScaleBar->setStyle( "Single Box" );
  QgsCompositionChecker checker( "composerscalebar_singlebox", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
};

void TestQgsComposerScaleBar::singleBoxAlpha()
{
  mComposerScaleBar->setStyle( "Single Box" );
  mComposerScaleBar->setBrush( QBrush( QColor( 255, 0, 0, 100 ) ) );
  mComposerScaleBar->setBrush2( QBrush( QColor( 0, 255, 0, 50 ) ) );
  QPen prevPen = mComposerScaleBar->pen();
  QPen newPen = prevPen;
  newPen.setColor( QColor( 0, 0, 255, 150 ) );
  mComposerScaleBar->setPen( newPen );
  mComposerScaleBar->setFontColor( QColor( 255, 0, 255, 100 ) );
  QgsCompositionChecker checker( "composerscalebar_singlebox_alpha", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
  mComposerScaleBar->setBrush( QBrush( Qt::black ) );
  mComposerScaleBar->setBrush2( QBrush( Qt::white ) );
  mComposerScaleBar->setPen( prevPen );
  mComposerScaleBar->setFontColor( Qt::black );
};

void TestQgsComposerScaleBar::doubleBox()
{
  mComposerScaleBar->setStyle( "Double Box" );
  QgsCompositionChecker checker( "composerscalebar_doublebox", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
};

void TestQgsComposerScaleBar::numeric()
{
  mComposerScaleBar->setStyle( "Numeric" );
  mComposerScaleBar->setSceneRect( QRectF( 20, 180, 50, 20 ) );
  QgsCompositionChecker checker( "composerscalebar_numeric", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
};

void TestQgsComposerScaleBar::tick()
{
  mComposerScaleBar->setStyle( "Line Ticks Up" );
  QgsCompositionChecker checker( "composerscalebar_tick", mComposition );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
};

QTEST_MAIN( TestQgsComposerScaleBar )
#include "testqgscomposerscalebar.moc"
