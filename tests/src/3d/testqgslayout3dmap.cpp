/***************************************************************************
                         testqgslayout3dmap.cpp
                         -----------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgs3dmapsettings.h"
#include "qgsapplication.h"
#include "qgsflatterraingenerator.h"
#include "qgslayout.h"
#include "qgslayoutitem3dmap.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"

#include <QObject>

class TestQgsLayout3DMap : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayout3DMap()
      : QgsTest( u"Layout 3D Map Tests"_s, u"composer_3d"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testBasic();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm;
};

void TestQgsLayout3DMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mProject = std::make_unique<QgsProject>();

  const QString dataDir( TEST_DATA_DIR );
  mLayerDtm = new QgsRasterLayer( dataDir + "/3d/dtm.tif", "rgb", "gdal" );
  QVERIFY( mLayerDtm->isValid() );
  mProject->addMapLayer( mLayerDtm );

  mProject->setCrs( mLayerDtm->crs() );
}

void TestQgsLayout3DMap::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgsLayout3DMap::init()
{
}

void TestQgsLayout3DMap::cleanup()
{
}

void TestQgsLayout3DMap::testBasic()
{
  const QgsRectangle fullExtent = mLayerDtm->extent();

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( mProject->crs() );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << mLayerDtm );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), mProject->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsCameraPose cam;
  cam.setDistanceFromCenterPoint( 2500 );

  QgsLayout l( mProject.get() );
  l.initializeDefaults();

  QgsLayoutItem3DMap *map3dItem = new QgsLayoutItem3DMap( &l );
  map3dItem->setBackgroundColor( QColor( 0, 0, 0 ) );
  map3dItem->attemptSetSceneRect( QRectF( 0, 0, 297, 210 ) );
  map3dItem->setCameraPose( cam );
  map3dItem->setMapSettings( map );
  l.addLayoutItem( map3dItem );

  QGSVERIFYLAYOUTCHECK( u"composer3d_basic_qt6"_s, &l, 0, 100 );

  QVERIFY( !map->isTemporal() );

  const QDateTime begin( QDate( 2020, 01, 01 ), QTime( 10, 0, 0 ), Qt::UTC );
  const QDateTime end = begin.addSecs( 3600 );
  map3dItem->setTemporalRange( QgsDateTimeRange( begin, end ) );

  map3dItem->refresh();

  QGSVERIFYLAYOUTCHECK( u"composer3d_basic_qt6"_s, &l, 0, 100 );

  QVERIFY( map->isTemporal() );
  QCOMPARE( map->temporalRange(), QgsDateTimeRange( begin, end ) );
}


QGSTEST_MAIN( TestQgsLayout3DMap )
#include "testqgslayout3dmap.moc"
