/***************************************************************************
    testqgschhunkedentity.cpp
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgs3d.h"
#include "qgs3dmapsettings.h"
#include "qgspolygon3dsymbol.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrulebased3drenderer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsvectorlayerchunkloader_p.h"

class TestQgsChunkedEntity : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsChunkedEntity()
      : QgsTest( u"3D Rendering Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void vectorLayerChunkedEntityElevationOffset();
    void ruleBasedChunkedEntityElevationOffset();

  private:
    std::unique_ptr<QgsProject> mProject;
    QgsRasterLayer *mLayerDtm = nullptr;
    QgsRasterLayer *mLayerRgb = nullptr;
    QgsVectorLayer *mLayerBuildings = nullptr;
};


// runs before all tests
void TestQgsChunkedEntity::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();

  mProject = std::make_unique<QgsProject>();

  mLayerDtm = new QgsRasterLayer( testDataPath( "/3d/dtm.tif" ), "dtm", "gdal" );
  QVERIFY( mLayerDtm->isValid() );
  mProject->addMapLayer( mLayerDtm );

  mLayerRgb = new QgsRasterLayer( testDataPath( "/3d/rgb.tif" ), "rgb", "gdal" );
  QVERIFY( mLayerRgb->isValid() );
  mProject->addMapLayer( mLayerRgb );

  mLayerBuildings = new QgsVectorLayer( testDataPath( "/3d/buildings.shp" ), "buildings", "ogr" );
  QVERIFY( mLayerBuildings->isValid() );
  mProject->addMapLayer( mLayerBuildings );

  // best to keep buildings without 2D renderer so it is not painted on the terrain
  // so we do not get some possible artifacts
  mLayerBuildings->setRenderer( nullptr );

  QgsPhongMaterialSettings materialSettings;
  materialSettings.setAmbient( Qt::lightGray );
  QgsPolygon3DSymbol *symbol3d = new QgsPolygon3DSymbol;
  symbol3d->setMaterialSettings( materialSettings.clone() );
  symbol3d->setExtrusionHeight( 10.f );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbol3d );
  mLayerBuildings->setRenderer3D( renderer3d );

  mProject->setCrs( mLayerDtm->crs() );
}

//runs after all tests
void TestQgsChunkedEntity::cleanupTestCase()
{
  mProject.reset();
  QgsApplication::exitQgis();
}

void TestQgsChunkedEntity::vectorLayerChunkedEntityElevationOffset()
{
  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );

  QgsPolygon3DSymbol *symbolAbsolute = new QgsPolygon3DSymbol;
  symbolAbsolute->setAltitudeClamping( Qgis::AltitudeClamping::Absolute );
  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbolAbsolute );
  renderer3d->setLayer( mLayerBuildings );
  mLayerBuildings->setRenderer3D( renderer3d );

  std::unique_ptr<Qt3DCore::QEntity> entity( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  QVector<Qt3DCore::QTransform *> trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, 0.0f ) );

  // set an elevation offset, no change is clamping is absolute
  const float offset = 42.f;
  QgsAbstractTerrainSettings *terrainSettings = map->terrainSettings()->clone();
  terrainSettings->setElevationOffset( offset );
  map->setTerrainSettings( terrainSettings );

  entity.reset( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, 0.0f ) );

  // if clamping is terrain, offset is applied
  QgsPolygon3DSymbol *symbolTerrain = static_cast<QgsPolygon3DSymbol *>( symbolAbsolute->clone() );
  symbolTerrain->setAltitudeClamping( Qgis::AltitudeClamping::Terrain );
  ;
  renderer3d->setSymbol( symbolTerrain );

  entity.reset( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, offset ) );
}

void TestQgsChunkedEntity::ruleBasedChunkedEntityElevationOffset()
{
  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setLayers( QList<QgsMapLayer *>() << mLayerBuildings );

  auto symbolAbsolute = std::make_unique<QgsPolygon3DSymbol>();
  symbolAbsolute->setAltitudeClamping( Qgis::AltitudeClamping::Absolute );
  QgsRuleBased3DRenderer::Rule *root = new QgsRuleBased3DRenderer::Rule( nullptr );
  QgsRuleBased3DRenderer::Rule *rule1 = new QgsRuleBased3DRenderer::Rule( symbolAbsolute->clone(), "ogc_fid < 29069", "rule 1" );
  QgsRuleBased3DRenderer::Rule *rule2 = new QgsRuleBased3DRenderer::Rule( symbolAbsolute->clone(), "ogc_fid > 29069", "rule 2" );
  root->appendChild( rule1 );
  root->appendChild( rule2 );
  QgsRuleBased3DRenderer *renderer3d = new QgsRuleBased3DRenderer( root );
  renderer3d->setLayer( mLayerBuildings );
  mLayerBuildings->setRenderer3D( renderer3d );

  std::unique_ptr<Qt3DCore::QEntity> entity( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  QVector<Qt3DCore::QTransform *> trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, 0.0f ) );

  // set an elevation offset, no change if clamping is absolute for all rules
  const float offset = 42.f;
  QgsAbstractTerrainSettings *terrainSettings = map->terrainSettings()->clone();
  terrainSettings->setElevationOffset( offset );
  map->setTerrainSettings( terrainSettings );

  entity.reset( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, 0.0f ) );

  // if clamping is terrain for all rules, offset is applied
  std::unique_ptr<QgsPolygon3DSymbol> symbolTerrain( static_cast<QgsPolygon3DSymbol *>( symbolAbsolute->clone() ) );
  symbolTerrain->setAltitudeClamping( Qgis::AltitudeClamping::Terrain );
  rule1->setSymbol( symbolTerrain->clone() );
  rule2->setSymbol( symbolTerrain->clone() );

  entity.reset( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, offset ) );

  // this is not ideal, but if clamping is absolute in at least one rule, offset is zero
  rule1->setSymbol( symbolAbsolute->clone() );
  rule2->setSymbol( symbolTerrain->clone() );

  entity.reset( renderer3d->createEntity( map ) );
  QVERIFY( entity );
  trs = entity->componentsOfType<Qt3DCore::QTransform>();
  QVERIFY( trs.size() == 1 );
  QVERIFY( trs.constFirst()->translation() == QVector3D( 0.0f, 0.0f, 0.0f ) );
}

QGSTEST_MAIN( TestQgsChunkedEntity )
#include "testqgschunkedentity.moc"
