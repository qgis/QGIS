/***************************************************************************
  testqgs3dannotationlayerrendering.cpp
  --------------------------------------
  Date                 : August 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall.dawson@gmail.com
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
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsannotationlayer.h"
#include "qgsannotationlayer3drenderer.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpictureitem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgsflatterraingenerator.h"
#include "qgsfontutils.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsoffscreen3dengine.h"
#include "qgssettingsentryenumflag.h"
#include "qgstest.h"

#include <QString>
#include <QSurfaceFormat>

using namespace Qt::StringLiterals;

class TestQgs3DAnnotationLayerRendering : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3DAnnotationLayerRendering()
      : QgsTest( u"3D Annotation Layer Rendering Tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testAnnotationLayerBillboards();
    void testAnnotationLayerText();
    void testAnnotationLayerPictureFixedSize();
    void testAnnotationLayerPicturePerspective();
    void testAnnotationLayerPictureSVG();

  private:
};

// runs before all tests
void TestQgs3DAnnotationLayerRendering::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QSurfaceFormat format;
  format.setRenderableType( QSurfaceFormat::OpenGL );
#ifdef Q_OS_MACOS
  format.setVersion( 4, 1 ); //OpenGL is deprecated on MacOS, use last supported version
  format.setProfile( QSurfaceFormat::CoreProfile );
#else
  format.setVersion( 4, 3 );
  format.setProfile( QSurfaceFormat::CompatibilityProfile );
#endif
  format.setDepthBufferSize( 24 );
  format.setSamples( 4 );
  format.setStencilBufferSize( 8 );
  QSurfaceFormat::setDefaultFormat( format );

  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();
  Qgs3D::settingTextureFilterQuality->setValue( Qgis::TextureFilterQuality::Trilinear );
}

//runs after all tests
void TestQgs3DAnnotationLayerRendering::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DAnnotationLayerRendering::testAnnotationLayerBillboards()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto annotationLayer = std::make_unique<QgsAnnotationLayer>( "test", QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );

  auto marker1 = std::make_unique< QgsAnnotationMarkerItem >( QgsPoint( 1000, 1000 ) );
  QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  markerSymbol->setColor( QColor( 255, 0, 0 ) );
  markerSymbol->setSize( 4 );
  QgsSimpleMarkerSymbolLayer *sl = static_cast<QgsSimpleMarkerSymbolLayer *>( markerSymbol->symbolLayer( 0 ) );
  sl->setStrokeColor( QColor( 0, 0, 255 ) );
  sl->setStrokeWidth( 2 );
  marker1->setSymbol( markerSymbol );
  annotationLayer->addItem( marker1.release() );

  auto marker2 = std::make_unique< QgsAnnotationMarkerItem >( QgsPoint( 1000, 2000 ) );
  markerSymbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  markerSymbol->setColor( QColor( 0, 255, 0 ) );
  markerSymbol->setSize( 20 );
  sl = static_cast<QgsSimpleMarkerSymbolLayer *>( markerSymbol->symbolLayer( 0 ) );
  sl->setStrokeColor( QColor( 255, 0, 255 ) );
  sl->setStrokeWidth( 2 );
  marker2->setSymbol( markerSymbol );
  annotationLayer->addItem( marker2.release() );

  auto marker3 = std::make_unique< QgsAnnotationMarkerItem >( QgsPoint( 2000, 2000 ) );
  markerSymbol = static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) );
  markerSymbol->setColor( QColor( 0, 0, 255 ) );
  markerSymbol->setSize( 30 );
  sl = static_cast<QgsSimpleMarkerSymbolLayer *>( markerSymbol->symbolLayer( 0 ) );
  sl->setStrokeColor( QColor( 0, 255, 255 ) );
  sl->setStrokeWidth( 2 );
  marker3->setSymbol( markerSymbol );
  annotationLayer->addItem( marker3.release() );

  auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();

  annotationLayer->setRenderer3D( renderer->clone() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << annotationLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "annotation_billboard_rendering_1", "annotation_billboard_rendering_1", img, QString(), 40, QSize( 0, 0 ), 2 );

  // more perspective look, with z offset
  renderer->setZOffset( 200 );
  renderer->setShowCalloutLines( true );
  renderer->setCalloutLineColor( QColor( 255, 255, 255 ) );
  renderer->setCalloutLineWidth( 8 );
  annotationLayer->setRenderer3D( renderer->clone() );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "annotation_billboard_rendering_2", "annotation_billboard_rendering_2", img2, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DAnnotationLayerRendering::testAnnotationLayerText()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto annotationLayer = std::make_unique<QgsAnnotationLayer>( "test", QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );

  auto text1 = std::make_unique< QgsAnnotationPointTextItem >( u"POINT"_s, QgsPoint( 1000, 1000 ) );
  annotationLayer->addItem( text1.release() );

  const QgsGeometry curve = QgsGeometry::fromWkt( u"Linestring( 1000 2000, 1500 2000 )"_s );
  auto text2 = std::make_unique< QgsAnnotationLineTextItem >( u"LINE"_s, qgsgeometry_cast< const QgsLineString * >( curve.constGet() )->clone() );
  annotationLayer->addItem( text2.release() );

  auto text3 = std::make_unique< QgsAnnotationRectangleTextItem >( u"RECT"_s, QgsRectangle::fromCenterAndSize( QgsPointXY( 2000, 2000 ), 400, 200 ) );
  annotationLayer->addItem( text3.release() );

  auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );
  format.setSize( 48 );
  format.setSizeUnit( Qgis::RenderUnit::Points );
  format.setColor( QColor( 0, 0, 255 ) );
  renderer->setTextFormat( format );

  annotationLayer->setRenderer3D( renderer->clone() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << annotationLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "annotation_text_rendering_1", "annotation_text_rendering_1", img, QString(), 40, QSize( 0, 0 ), 2 );

  // more perspective look, with z offset
  renderer->setZOffset( 300 );
  renderer->setShowCalloutLines( true );
  renderer->setCalloutLineColor( QColor( 255, 255, 255 ) );
  renderer->setCalloutLineWidth( 8 );
  annotationLayer->setRenderer3D( renderer->clone() );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "annotation_text_rendering_2", "annotation_text_rendering_2", img2, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DAnnotationLayerRendering::testAnnotationLayerPictureFixedSize()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto annotationLayer = std::make_unique<QgsAnnotationLayer>( "test", QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );

  const QString sourceImage1 = testDataPath( u"raster_brush.png"_s );
  auto picture1 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, sourceImage1, QgsRectangle::fromCenterAndSize( QgsPointXY( 1000, 1000 ), 400, 200 ) );
  picture1->setBillboard3DScaleMode( Qgis::BillboardScaleMode::ViewIndependent );
  picture1->setBillboard3DSize( QSizeF( 100, 100 ) );
  annotationLayer->addItem( picture1.release() );

  auto picture2 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, sourceImage1, QgsRectangle::fromCenterAndSize( QgsPointXY( 1000, 2000 ), 400, 200 ) );
  picture2->setBillboard3DScaleMode( Qgis::BillboardScaleMode::ViewIndependent );
  picture2->setBillboard3DSize( QSizeF( 170, 170 ) );
  annotationLayer->addItem( picture2.release() );

  auto picture3 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, testDataPath( u"sample_image.png"_s ), QgsRectangle::fromCenterAndSize( QgsPointXY( 2000, 2000 ), 400, 200 ) );
  picture3->setBillboard3DScaleMode( Qgis::BillboardScaleMode::ViewIndependent );
  picture3->setBillboard3DSize( QSizeF( 130, 130 ) );
  annotationLayer->addItem( picture3.release() );

  auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();

  annotationLayer->setRenderer3D( renderer->clone() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << annotationLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "annotation_billboard_picture_rendering_1", "annotation_billboard_picture_rendering_1", img, QString(), 40, QSize( 0, 0 ), 2 );

  // more perspective look, with z offset
  renderer->setZOffset( 400 );
  renderer->setShowCalloutLines( true );
  renderer->setCalloutLineColor( QColor( 255, 255, 255 ) );
  renderer->setCalloutLineWidth( 8 );
  annotationLayer->setRenderer3D( renderer->clone() );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "annotation_billboard_picture_rendering_2", "annotation_billboard_picture_rendering_2", img2, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DAnnotationLayerRendering::testAnnotationLayerPicturePerspective()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto annotationLayer = std::make_unique<QgsAnnotationLayer>( "test", QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );

  const QString sourceImage1 = testDataPath( u"raster_brush.png"_s );
  auto picture1 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, sourceImage1, QgsRectangle::fromCenterAndSize( QgsPointXY( 1000, 1000 ), 400, 200 ) );
  picture1->setBillboard3DScaleMode( Qgis::BillboardScaleMode::Perspective );
  picture1->setBillboard3DSize( QSizeF( 300, 300 ) );
  annotationLayer->addItem( picture1.release() );

  auto picture2 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, sourceImage1, QgsRectangle::fromCenterAndSize( QgsPointXY( 1000, 2000 ), 400, 200 ) );
  picture2->setBillboard3DScaleMode( Qgis::BillboardScaleMode::Perspective );
  picture2->setBillboard3DSize( QSizeF( 270, 270 ) );
  annotationLayer->addItem( picture2.release() );

  auto picture3 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, testDataPath( u"sample_image.png"_s ), QgsRectangle::fromCenterAndSize( QgsPointXY( 2000, 2000 ), 400, 200 ) );
  picture3->setBillboard3DScaleMode( Qgis::BillboardScaleMode::Perspective );
  picture3->setBillboard3DSize( QSizeF( 430, 430 ) );
  annotationLayer->addItem( picture3.release() );

  auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();

  annotationLayer->setRenderer3D( renderer->clone() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << annotationLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "annotation_billboard_picture_perspective_rendering_1", "annotation_billboard_picture_perspective_rendering_1", img, QString(), 40, QSize( 0, 0 ), 2 );

  // more perspective look, with z offset
  renderer->setZOffset( 400 );
  renderer->setShowCalloutLines( true );
  renderer->setCalloutLineColor( QColor( 255, 255, 255 ) );
  renderer->setCalloutLineWidth( 8 );
  annotationLayer->setRenderer3D( renderer->clone() );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img2 = Qgs3DUtils::captureSceneImage( engine, scene );
  delete scene;
  delete map;

  QGSVERIFYIMAGECHECK( "annotation_billboard_picture_perspective_rendering_2", "annotation_billboard_picture_perspective_rendering_2", img2, QString(), 40, QSize( 0, 0 ), 2 );
}

void TestQgs3DAnnotationLayerRendering::testAnnotationLayerPictureSVG()
{
  const QgsRectangle fullExtent( 1000, 1000, 2000, 2000 );

  auto annotationLayer = std::make_unique<QgsAnnotationLayer>( "test", QgsAnnotationLayer::LayerOptions( QgsCoordinateTransformContext() ) );

  const QString sourceImage1 = testDataPath( u"sample_svg.svg"_s );
  auto picture1 = std::make_unique< QgsAnnotationPictureItem >( Qgis::PictureFormat::Raster, sourceImage1, QgsRectangle::fromCenterAndSize( QgsPointXY( 1000, 1000 ), 400, 200 ) );
  picture1->setBillboard3DScaleMode( Qgis::BillboardScaleMode::ViewIndependent );
  picture1->setBillboard3DSize( QSizeF( 100, 100 ) );
  annotationLayer->addItem( picture1.release() );

  auto renderer = std::make_unique< QgsAnnotationLayer3DRenderer >();

  annotationLayer->setRenderer3D( renderer->clone() );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  map->setExtent( fullExtent );
  map->setLayers( QList<QgsMapLayer *>() << annotationLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs(), map->transformContext() );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  // look from the top
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 0, 0 );

  // When running the test on Travis, it would initially return empty rendered image.
  // Capturing the initial image and throwing it away fixes that. Hopefully we will
  // find a better fix in the future.
  Qgs3DUtils::captureSceneImage( engine, scene );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );
  QGSVERIFYIMAGECHECK( "annotation_billboard_picture_svg_rendering", "annotation_billboard_picture_svg_rendering", img, QString(), 40, QSize( 0, 0 ), 2 );
}

QGSTEST_MAIN( TestQgs3DAnnotationLayerRendering )
#include "testqgs3dannotationlayerrendering.moc"
