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
#include <QtTest/QtTest>

class TestQgsComposerMap : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerMap()
        : mComposition( 0 )
        , mComposerMap( 0 )
        , mMapSettings( 0 )
        , mRasterLayer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void render(); //test if rendering of the composition with composr map is correct
    void uniqueId(); //test if map id is adapted when doing copy paste
    void worldFileGeneration(); // test world file generation
    void mapPolygonVertices(); // test mapPolygon function with no map rotation

  private:
    QgsComposition *mComposition;
    QgsComposerMap *mComposerMap;
    QgsMapSettings *mMapSettings;
    QgsRasterLayer* mRasterLayer;
    QString mReport;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  //create composition with composer map
  mMapSettings->setLayers( QStringList() << mRasterLayer->id() );
  mMapSettings->setCrsTransformEnabled( false );
  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  mReport = "<h1>Composer Map Tests</h1>\n";
}

void TestQgsComposerMap::cleanupTestCase()
{
  delete mComposition;
  delete mMapSettings;

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

void TestQgsComposerMap::init()
{
}

void TestQgsComposerMap::cleanup()
{

}

void TestQgsComposerMap::render()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsCompositionChecker checker( "composermap_render", mComposition );

  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
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

  QVERIFY( newMap );

  int oldId = mComposerMap->id();
  int newId = newMap->id();

  mComposition->removeComposerItem( const_cast<QgsComposerMap*>( newMap ) );

  QVERIFY( oldId != newId );
}

void TestQgsComposerMap::worldFileGeneration()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setMapRotation( 30.0 );

  mComposition->setGenerateWorldFile( true );
  mComposition->setWorldFileMap( mComposerMap );

  double a, b, c, d, e, f;
  mComposition->computeWorldFileParameters( a, b, c, d, e, f );

  QVERIFY( fabs( a - 4.18048 ) < 0.001 );
  QVERIFY( fabs( b - 2.41331 ) < 0.001 );
  QVERIFY( fabs( c - 779444 ) < 1 );
  QVERIFY( fabs( d - 2.4136 ) < 0.001 );
  QVERIFY( fabs( e + 4.17997 ) < 0.001 );
  QVERIFY( fabs( f - 3.34241e+06 ) < 1e+03 );

  mComposition->setGenerateWorldFile( false );
  mComposerMap->setMapRotation( 0.0 );

}

void TestQgsComposerMap::mapPolygonVertices()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QPolygonF visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3339523.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3339523.125 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  //now test with rotated map
  mComposerMap->setMapRotation( 10 );
  visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781254.0735015 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3344190.0324834 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 792480.881886 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3346169.62171 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793470.676499 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3340556.21752 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 782243.868114 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3338576.62829 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  mComposerMap->setMapRotation( 0 );

}

QTEST_MAIN( TestQgsComposerMap )
#include "testqgscomposermap.moc"
