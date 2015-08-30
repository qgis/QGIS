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
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsvisibilitypresetcollection.h"
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
        , mPointsLayer( 0 )
        , mPolysLayer( 0 )
        , mLinesLayer( 0 )
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
    void dataDefinedLayers(); //test data defined layer string
    void dataDefinedStyles(); //test data defined styles

  private:
    QgsComposition *mComposition;
    QgsComposerMap *mComposerMap;
    QgsMapSettings *mMapSettings;
    QgsRasterLayer* mRasterLayer;
    QgsVectorLayer* mPointsLayer;
    QgsVectorLayer* mPolysLayer;
    QgsVectorLayer* mLinesLayer;
    QString mReport;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );
  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  QFileInfo pointFileInfo( QString( TEST_DATA_DIR ) + "/points.shp" );
  mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                     pointFileInfo.completeBaseName(), "ogr" );
  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer *>() << mPointsLayer );

  QFileInfo polyFileInfo( QString( TEST_DATA_DIR ) + "/polys.shp" );
  mPolysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                    polyFileInfo.completeBaseName(), "ogr" );
  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer *>() << mPolysLayer );

  QFileInfo lineFileInfo( QString( TEST_DATA_DIR ) + "/lines.shp" );
  mLinesLayer = new QgsVectorLayer( lineFileInfo.filePath(),
                                    lineFileInfo.completeBaseName(), "ogr" );
  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer *>() << mLinesLayer );
}

void TestQgsComposerMap::cleanupTestCase()
{
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
  mMapSettings = new QgsMapSettings();

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

void TestQgsComposerMap::cleanup()
{
  delete mComposition;
  delete mMapSettings;
}

void TestQgsComposerMap::render()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsCompositionChecker checker( "composermap_render", mComposition );
  checker.setControlPathPrefix( "composer_map" );

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

void TestQgsComposerMap::dataDefinedLayers()
{
  delete mComposition;
  QgsMapSettings ms;
  ms.setLayers( QStringList() << mRasterLayer->id() << mPolysLayer->id() << mPointsLayer->id() << mLinesLayer->id() );
  ms.setCrsTransformEnabled( true );

  mComposition = new QgsComposition( ms );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  //test malformed layer set string
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true, "'x'", QString() );
  QStringList result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );

  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true, "'x|'", QString() );
  result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );

  //test subset of valid layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QString( "'%1|%2'" ).arg( mPolysLayer->name() ).arg( mRasterLayer->name() ), QString() );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mPolysLayer->id() ) );
  QVERIFY( result.contains( mRasterLayer->id() ) );

  //test non-existant layer
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QString( "'x|%1|%2'" ).arg( mLinesLayer->name() ).arg( mPointsLayer->name() ), QString() );
  result = mComposerMap->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer->id() ) );
  QVERIFY( result.contains( mPointsLayer->id() ) );

  //test no layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QString( "''" ), QString() );
  result = mComposerMap->layersToRender();
  QVERIFY( result.isEmpty() );

  //render test
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QString( "'%1|%2'" ).arg( mPolysLayer->name() ).arg( mPointsLayer->name() ), QString() );
  mComposerMap->setNewExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsCompositionChecker checker( "composermap_ddlayers", mComposition );
  checker.setControlPathPrefix( "composer_map" );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerMap::dataDefinedStyles()
{
  delete mComposition;
  QgsMapSettings ms;
  ms.setLayers( QStringList() << mRasterLayer->id() << mPolysLayer->id() << mPointsLayer->id() << mLinesLayer->id() );
  ms.setCrsTransformEnabled( true );

  mComposition = new QgsComposition( ms );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  QgsVisibilityPresetCollection::PresetRecord rec;
  rec.mVisibleLayerIDs.insert( mPointsLayer->id() );
  rec.mVisibleLayerIDs.insert( mLinesLayer->id() );

  QgsProject::instance()->visibilityPresetCollection()->insert( "test preset", rec );

  //test malformed style string
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, "5", QString() );
  QSet<QString> result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result, ms.layers().toSet() );

  //test valid preset
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, QString( "'test preset'" ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer->id() ) );
  QVERIFY( result.contains( mPointsLayer->id() ) );

  //test non-existant preset
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true,
                                        QString( "'bad preset'" ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result, ms.layers().toSet() );

  //test that dd layer set overrides style layers
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true, QString( "'test preset'" ), QString() );
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, true, true,
                                        QString( "'%1'" ).arg( mPolysLayer->name() ), QString() );
  result = mComposerMap->layersToRender().toSet();
  QCOMPARE( result.count(), 1 );
  QVERIFY( result.contains( mPolysLayer->id() ) );
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapLayers, false, true, QString(), QString() );

  //render test
  mComposerMap->setDataDefinedProperty( QgsComposerObject::MapStylePreset, true, true,
                                        QString( "'test preset'" ), QString() );
  mComposerMap->setNewExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsCompositionChecker checker( "composermap_ddstyles", mComposition );
  checker.setControlPathPrefix( "composer_map" );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

QTEST_MAIN( TestQgsComposerMap )
#include "testqgscomposermap.moc"
