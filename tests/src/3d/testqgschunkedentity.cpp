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

#include "qgstest.h"

#include "qgs3d.h"
#include "qgs3dmapsettings.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsline3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrulebased3drenderer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgsvectorlayerchunkloader_p.h"
#include "qgsdemterrainsettings.h"
#include "qgsdemterraingenerator.h"
#include "qgs3dsymbolregistry.h"
#include "qbuffer.h"

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif


class TestQgsChunkedEntity : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsChunkedEntity()
      : QgsTest( QStringLiteral( "3D Rendering Tests" ), QStringLiteral( "3d" ) )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void vectorLayerChunkedEntityElevationDtm();
    void vectorLayerChunkedEntityElevationOffset();
    void ruleBasedChunkedEntityElevationOffset();

  private:
    void checkLowestZ( QgsFeature f, QVector<float> expectedZ, Qgs3DMapSettings *map, QgsAbstract3DSymbol *symbolTerrain, QgsVectorLayer *layerData );
    void docCheckElevationDtm( const QgsRectangle &fullExtent, const QgsCoordinateReferenceSystem &crs, const QString &dataPath, const QString &dtmPath, //
                               const QString &dtmHiResPath, const QgsRectangle &featExtent, const QVector<float> &expectedZ );

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

  mProject.reset( new QgsProject );

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

void TestQgsChunkedEntity::checkLowestZ( QgsFeature f, QVector<float> expectedZ, Qgs3DMapSettings *map, //
                                         QgsAbstract3DSymbol *symbolTerrain, QgsVectorLayer *layerData )
{
  std::unique_ptr<Qt3DCore::QEntity> entity;
  Qgs3DRenderContext renderContext = Qgs3DRenderContext::fromMapSettings( map );
  renderContext.expressionContext().setFeature( f );

  QgsFeature3DHandler *handler = QgsApplication::symbol3DRegistry()->createHandlerForSymbol( layerData, symbolTerrain );
  QSet<QString> attributeNames;
  QVERIFY( handler->prepare( renderContext, attributeNames, QgsVector3D() ) );
  handler->processFeature( f, renderContext );

  entity.reset( new Qt3DCore::QEntity() );
  entity->setObjectName( "ROOT" );

  handler->finalize( entity.get(), renderContext );

  QCOMPARE( entity->children().size(), 1 );

  for ( QObject *child : entity->children() ) // search 3d entity
  {
    Qt3DCore::QEntity *childEntity = qobject_cast<Qt3DCore::QEntity *>( child );
    if ( childEntity )
    {
      for ( QObject *comp : childEntity->children() ) // search geometry renderer
      {
        Qt3DRender::QGeometryRenderer *childComp = qobject_cast<Qt3DRender::QGeometryRenderer *>( comp );
        if ( childComp )
        {
          QVERIFY( childComp->geometry() );
          for ( Qt3DRender::QAttribute *attrib : childComp->geometry()->attributes() ) // search position attribute
          {
            if ( attrib->name() == Qt3DQAttribute::defaultPositionAttributeName() )
            {
              QCOMPARE( attrib->count(), expectedZ.size() );
              Qt3DQBuffer *buff3d = attrib->buffer();
              QByteArray buff = buff3d->data();
              const float *ptrFloat = reinterpret_cast<const float *>( buff.constData() );
              for ( uint i = 0; i < attrib->count(); ++i )
              {
                float z = ptrFloat[( i * attrib->byteStride() / sizeof( float ) ) + 2];
                if ( z != expectedZ[i] )
                {
                  qWarning() << "actual[" << i << "]=" << z << "/ expected= " << expectedZ[i];
                  for ( uint j = 0; j < attrib->count(); ++j )
                  {
                    float x = ptrFloat[( j * attrib->byteStride() / sizeof( float ) ) + 0];
                    float y = ptrFloat[( j * attrib->byteStride() / sizeof( float ) ) + 1];
                    z = ptrFloat[( j * attrib->byteStride() / sizeof( float ) ) + 2];
                    qWarning() << x << "," << y << "," << z;
                  }
                }
                QCOMPARE( z, expectedZ[i] );
              }
              break;
            }
          }
          break;
        }
      }
    }
  }
}


void TestQgsChunkedEntity::docCheckElevationDtm( const QgsRectangle &fullExtent, const QgsCoordinateReferenceSystem &crs,      //
                                                 const QString &dataPath, const QString &dtmPath, const QString &dtmHiResPath, //
                                                 const QgsRectangle &featExtent, const QVector<float> &expectedZ )
{
  QgsVectorLayer *layerData = new QgsVectorLayer( dataPath, "data", "ogr" );
  QgsRasterLayer *layerDtm = new QgsRasterLayer( dtmPath, "dtm", "gdal" );

  Qgs3DMapSettings *mapSettings = new Qgs3DMapSettings;
  mapSettings->setLayers( QList<QgsMapLayer *>() << layerData << layerDtm );
  mapSettings->setCrs( crs );
  mapSettings->setExtent( fullExtent );

  QgsDemTerrainSettings *demTerrainSettings = new QgsDemTerrainSettings;
  demTerrainSettings->setLayer( layerDtm );
  demTerrainSettings->setVerticalScale( 3 );

  mapSettings->setTerrainSettings( demTerrainSettings );

  // if clamping is terrain, offset is applied
  QgsAbstract3DSymbol *symbolTerrain;
  if ( layerData->geometryType() == Qgis::GeometryType::Line )
  {
    QgsLine3DSymbol *symbol = new QgsLine3DSymbol;
    symbol->setRenderAsSimpleLines( false );
    symbol->setAltitudeClamping( Qgis::AltitudeClamping::Terrain );
    symbol->setAltitudeBinding( Qgis::AltitudeBinding::Vertex );
    symbol->setWidth( 2.0 );
    symbol->setExtrusionHeight( 0.0 );
    symbolTerrain = symbol;
  }
  else
  {
    QgsPolygon3DSymbol *symbol = new QgsPolygon3DSymbol;
    symbol->setAltitudeClamping( Qgis::AltitudeClamping::Terrain );
    symbol->setAltitudeBinding( Qgis::AltitudeBinding::Vertex );
    symbolTerrain = symbol;
  }

  QgsVectorLayer3DRenderer *renderer3d = new QgsVectorLayer3DRenderer( symbolTerrain );
  renderer3d->setLayer( layerData );
  layerData->setRenderer3D( renderer3d );

  // build full res terrain generator
  QgsRasterLayer *layerDtmHiRes = new QgsRasterLayer( dtmHiResPath, "dtm", "gdal" );
  QgsDemTerrainGenerator fullResDem;
  fullResDem.setLayer( layerDtmHiRes );
  fullResDem.setCrs( mapSettings->crs(), QgsCoordinateTransformContext() );

  // lowest Z is computed above terrain
  QgsFeatureIterator fi = layerData->getFeatures( featExtent );
  QgsFeature f;
  QVERIFY( fi.nextFeature( f ) );

  QgsPointXY centroid = f.geometry().centroid().asPoint();
  float fullResZ;
  qDebug() << "Check elevation around x/y:" << centroid.x() << "/" << centroid.y();
  for ( int x = -10; x <= 10; x += 2 )
    for ( int y = -10; y <= 10; y += 2 )
    {
      double tx = centroid.x() + x;
      double ty = centroid.y() + y;
      fullResZ = fullResDem.heightAt( tx, ty, Qgs3DRenderContext() );
      qDebug() << tx << "," << ty << "," << fullResZ * demTerrainSettings->verticalScale();
      float z = mapSettings->terrainGenerator()->heightAt( tx, ty, Qgs3DRenderContext() );
      QCOMPARE( fullResZ, z );
    }

  checkLowestZ( f, expectedZ, mapSettings, symbolTerrain, layerData );
}


void TestQgsChunkedEntity::vectorLayerChunkedEntityElevationDtm()
{
  docCheckElevationDtm( QgsRectangle( 321875, 130109, 321930, 130390 ),                                                           //
                        QgsCoordinateReferenceSystem( "EPSG:27700" ),                                                             //
                        testDataPath( "/3d/buildings.shp" ), testDataPath( "/3d/dtm.tif" ), testDataPath( "/3d/dtm_hi_res.tif" ), //
                        QgsRectangle( 321900, 130360, 321930, 130390 ),                                                           //
                        QVector<float> { 306.0f, 309.0f, 309.0f, 309.0f, 309.0f, 309.0f } );

  docCheckElevationDtm( QgsRectangle( 321875, 130109, 321930, 130390 ),                                                           //
                        QgsCoordinateReferenceSystem( "EPSG:27700" ),                                                             //
                        testDataPath( "/3d/buildings.shp" ), testDataPath( "/3d/dtm.tif" ), testDataPath( "/3d/dtm_hi_res.tif" ), //
                        QgsRectangle( 321875, 130109, 321883, 130120 ),                                                           //
                        QVector<float> { 228.0f, 234.0f, 231.0f, 228.0f, 231.0f, 228.0f } );

  docCheckElevationDtm( QgsRectangle( 60000, 60000, 70000, 70000 ),  //
                        QgsCoordinateReferenceSystem( "EPSG:2169" ), //
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/routes.gpkg.gz",
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/terrain.tif",                             //
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/terrain.tif",                             //
                        QgsRectangle( 62780, 64545, 62784, 64555 ),                                                                         //
                        QVector<float> { 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, //
                                         1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, //
                                         1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, //
                                         1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, //
                                         1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, //
                                         1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f, 1128.0f } );

  docCheckElevationDtm( QgsRectangle( 60000, 60000, 70000, 70000 ),  //
                        QgsCoordinateReferenceSystem( "EPSG:2169" ), //
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/routes.gpkg.gz",
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/terrain.tif",                             //
                        "/home/bde/prog/2010_25_lille_3d/QGIS-3D-projects-public/luxembourg-sanem/terrain.tif",                             //
                        QgsRectangle( 63265, 65796, 63260, 65802 ),                                                                         // id: 3118
                        QVector<float> { 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, //
                                         1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, //
                                         1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, //
                                         1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, 1170.0f, //
                                         1170.0f, 1170.0f, 1170.0f, 1170.0f } );
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
