/***************************************************************************
                         testqgscomposerrotation.cpp
                         ----------------------
    begin                : April 2013
    copyright            : (C) 2013 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail.com
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
#include "qgsmultirenderchecker.h"
#include "qgscomposershape.h"
#include "qgscomposermap.h"
#include "qgscomposerlabel.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgsfontutils.h"
#include "qgsrasterdataprovider.h"
#include <QObject>
#include "qgstest.h"
#include <QColor>
#include <QPainter>

class TestQgsComposerRotation : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerRotation() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void shapeRotation(); //test if composer shape rotation is functioning
    void labelRotation(); //test if composer label rotation is functioning
    void mapRotation(); //test if composer map mapRotation is functioning
    void mapItemRotation(); //test if composer map item rotation is functioning

  private:
    QgsComposition *mComposition = nullptr;
    QgsComposerShape *mComposerRect = nullptr;
    QgsComposerLabel *mComposerLabel = nullptr;
    QgsComposerMap *mComposerMap = nullptr;
    QgsRasterLayer *mRasterLayer = nullptr;
    QString mReport;
};

void TestQgsComposerRotation::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  mRasterLayer->setRenderer( rasterRenderer );

  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mComposerRect = new QgsComposerShape( 70, 70, 150, 100, mComposition );
  mComposerRect->setShapeType( QgsComposerShape::Rectangle );
  mComposerRect->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );

  mComposerLabel = new QgsComposerLabel( mComposition );
  mComposerLabel->setText( QStringLiteral( "test label" ) );
  mComposerLabel->setFont( QgsFontUtils::getStandardTestFont() );
  mComposerLabel->setPos( 70, 70 );
  mComposerLabel->adjustSizeToText();
  mComposerLabel->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );
  mComposerLabel->setBackgroundEnabled( true );

  mReport = QStringLiteral( "<h1>Composer Rotation Tests</h1>\n" );
}

void TestQgsComposerRotation::cleanupTestCase()
{
  if ( mComposerMap )
  {
    mComposition->removeItem( mComposerMap );
    delete mComposerMap;
  }

  delete mComposerLabel;
  delete mComposerRect;
  delete mComposition;
  delete mRasterLayer;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsComposerRotation::init()
{

}

void TestQgsComposerRotation::cleanup()
{

}

void TestQgsComposerRotation::shapeRotation()
{
  mComposition->addComposerShape( mComposerRect );

  mComposerRect->setItemRotation( 45, true );

  QgsCompositionChecker checker( QStringLiteral( "composerrotation_shape" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testComposition( mReport ) );

  mComposition->removeItem( mComposerRect );
  mComposerRect->setItemRotation( 0, true );
}

void TestQgsComposerRotation::labelRotation()
{
  mComposition->addComposerLabel( mComposerLabel );
  mComposerLabel->setItemRotation( 135, true );

  QgsCompositionChecker checker( QStringLiteral( "composerrotation_label" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerRotation::mapRotation()
{
  // cleanup after labelRotation()
  mComposition->removeItem( mComposerLabel );
  mComposerLabel->setItemRotation( 0, true );

  //test map rotation
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 100, 50 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addItem( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( 0, -192, 256, -64 ) );
  mComposerMap->setMapRotation( 90 );
  mComposerMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );

  QgsCompositionChecker checker( QStringLiteral( "composerrotation_maprotation" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testComposition( mReport, 0, 200 ) );
}

void TestQgsComposerRotation::mapItemRotation()
{
  // cleanup after mapRotation()
  mComposition->removeItem( mComposerMap );
  delete mComposerMap;

  //test map item rotation
  mComposerMap = new QgsComposerMap( mComposition, 20, 50, 100, 50 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addItem( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( 0, -192, 256, -64 ) );
  mComposerMap->setItemRotation( 90, true );
  mComposerMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );

  QgsCompositionChecker checker( QStringLiteral( "composerrotation_mapitemrotation" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testComposition( mReport ) );
}

QGSTEST_MAIN( TestQgsComposerRotation )
#include "testqgscomposerrotation.moc"
