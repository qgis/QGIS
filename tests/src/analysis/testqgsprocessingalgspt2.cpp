/***************************************************************************
                         testqgsprocessingalgspt2.cpp
                         ---------------------
    begin                : November 2017
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
#include <limits>
#include <memory>

#include "qgsalgorithmgpsbabeltools.h"
#include "qgsannotationlayer.h"
#include "qgsannotationmarkeritem.h"
#include "qgsdxfexport.h"
#include "qgsfontutils.h"
#include "qgslayertree.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmanager.h"
#include "qgslinesymbol.h"
#include "qgsmeshlayer.h"
#include "qgsnativealgorithms.h"
#include "qgspallabeling.h"
#include "qgsprintlayout.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingregistry.h"
#include "qgsreferencedgeometry.h"
#include "qgssinglesymbolrenderer.h"
#include "qgstest.h"
#include "qgstextformat.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"

#include <QString>
#include <qgsrasteranalysisutils.cpp>

using namespace Qt::StringLiterals;

class TestQgsProcessingAlgsPt2 : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingAlgsPt2()
      : QgsTest( u"Processing Algorithms Pt 2"_s, u"processing_algorithm"_s )
    {
      QgsFontUtils::loadStandardTestFonts( { u"Bold"_s } );
    }

  private:
    /**
     * Helper function to get a feature based algorithm.
     */
    std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> featureBasedAlg( const QString &id );

    QgsFeature runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters = QVariantMap() );

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void exportLayoutPdf();
    void exportLayoutPng();
    void exportAtlasLayoutPdf();
    void exportAtlasLayoutPdfMultiple();
    void exportAtlasLayoutPng();

    void tinMeshCreation();
    void exportMeshVertices();
    void exportMeshFaces();
    void exportMeshEdges();
    void exportMeshOnGrid();
    void rasterizeMesh();
    void exportMeshContours();
    void exportMeshCrossSection();
    void exportMeshTimeSeries();

    void fileDownloader();

    void rasterize();

    void convertGpxFeatureType();
    void convertGpsData();
    void downloadGpsData();
    void uploadGpsData();
    void transferMainAnnotationLayer();

    void extractLabels();

    void dxfExport();

    void splitVectorLayer();
    void buffer();
    void splitWithLines();

    void randomPointsInPolygonsFromField_data();
    void randomPointsInPolygonsFromField();

    void generateElevationProfileImage();

    void copyMetadata();
    void applyMetadata();
    void exportMetadata();
    void addHistoryMetadata();
    void updateMetadata();
    void setMetadataFields();

    void mergeVectors();

    void nativeAlgsRasterSize();

    void defineProjection();
    void checkValidity();

  private:
    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
};

std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> TestQgsProcessingAlgsPt2::featureBasedAlg( const QString &id )
{
  return std::unique_ptr<QgsProcessingFeatureBasedAlgorithm>( static_cast<QgsProcessingFeatureBasedAlgorithm *>( QgsApplication::processingRegistry()->createAlgorithmById( id ) ) );
}

QgsFeature TestQgsProcessingAlgsPt2::runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters )
{
  Q_ASSERT( alg.get() );
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;
  context->setFeedback( &feedback );

  auto inputLayer = std::make_unique<QgsVectorLayer>( layerType, u"layer"_s, u"memory"_s );
  inputLayer->dataProvider()->addFeature( feature );

  parameters.insert( u"INPUT"_s, QVariant::fromValue<QgsMapLayer *>( inputLayer.get() ) );

  parameters.insert( u"OUTPUT"_s, u"memory:"_s );

  bool ok = false;
  const auto res = alg->run( parameters, *context, &feedback, &ok );
  QgsFeature result;

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast<QgsVectorLayer *>( context->getMapLayer( res.value( u"OUTPUT"_s ).toString() ) ) );
  outputLayer->getFeatures().nextFeature( result );

  return result;
}

void TestQgsProcessingAlgsPt2::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QString pointsFileName = dataDir + "/points.shp";
  const QFileInfo pointFileInfo( pointsFileName );
  mPointLayerPath = pointFileInfo.filePath();
  mPointsLayer = new QgsVectorLayer( mPointLayerPath, u"points"_s, u"ogr"_s );
  QVERIFY( mPointsLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointsLayer
  );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString polysFileName = dataDir + "/polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(), u"polygons"_s, u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer
  );
  QVERIFY( mPolygonLayer->isValid() );

  //add a mesh layer
  const QString uri( dataDir + "/mesh/quad_and_triangle.2dm" );
  const QString meshLayerName = u"mesh layer"_s;
  QgsMeshLayer *meshLayer = new QgsMeshLayer( uri, meshLayerName, u"mdal"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayer( meshLayer );
  QVERIFY( meshLayer->isValid() );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_vertex_scalar.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_vertex_vector.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_els_face_scalar.dat" );
  meshLayer->addDatasets( dataDir + "/mesh/quad_and_triangle_els_face_vector.dat" );
  QCOMPARE( meshLayer->datasetGroupCount(), 5 );

  //add a 1D mesh layer
  const QString uri1d( dataDir + "/mesh/lines.2dm" );
  const QString meshLayer1dName = u"mesh layer 1D"_s;
  QgsMeshLayer *meshLayer1d = new QgsMeshLayer( uri1d, meshLayer1dName, u"mdal"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayer( meshLayer1d );
  QVERIFY( meshLayer1d->isValid() );
  meshLayer1d->addDatasets( dataDir + "/mesh/lines_els_scalar.dat" );
  meshLayer1d->addDatasets( dataDir + "/mesh/lines_els_vector.dat" );
  QCOMPARE( meshLayer1d->datasetGroupCount(), 3 );
}

void TestQgsProcessingAlgsPt2::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QVariantMap pkgAlg( const QStringList &layers, const QString &outputGpkg, bool overwrite, bool selectedFeaturesOnly, bool saveMetadata, bool *ok )
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( u"native:package"_s ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, layers );
  parameters.insert( u"OUTPUT"_s, outputGpkg );
  parameters.insert( u"OVERWRITE"_s, overwrite );
  parameters.insert( u"SELECTED_FEATURES_ONLY"_s, selectedFeaturesOnly );
  parameters.insert( u"SAVE_METADATA"_s, saveMetadata );
  return package->run( parameters, *context, &feedback, ok );
}

void TestQgsProcessingAlgsPt2::exportLayoutPdf()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( u"my layout"_s );
  p.layoutManager()->addLayout( layout );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:printlayouttopdf"_s ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_layout.pdf";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"missing"_s );
  parameters.insert( u"OUTPUT"_s, outputPdf );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPdf ) );

  parameters.insert( u"LAYOUT"_s, u"my layout"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdf ) );
}

void TestQgsProcessingAlgsPt2::exportLayoutPng()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( u"my layout"_s );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( mPointsLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:printlayouttoimage"_s ) );
  QVERIFY( alg != nullptr );

  QString outputPng = QDir::tempPath() + "/my_layout.png";
  if ( QFile::exists( outputPng ) )
    QFile::remove( outputPng );

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"missing"_s );
  parameters.insert( u"OUTPUT"_s, outputPng );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPng ) );

  parameters.insert( u"LAYOUT"_s, u"my layout"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputPng ) );

  outputPng = QDir::tempPath() + "/my_layout_custom_layers.png";
  if ( QFile::exists( outputPng ) )
    QFile::remove( outputPng );

  parameters.insert( u"OUTPUT"_s, outputPng );
  parameters.insert( u"LAYERS"_s, QVariantList() << QVariant::fromValue( mPointsLayer ) );
  parameters.insert( u"DPI"_s, 96 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputPng ) );
  QGSVERIFYIMAGECHECK( "export_layout_custom_layers", "export_layout_custom_layers", outputPng, QString(), 500, QSize( 3, 3 ) );
}

void TestQgsProcessingAlgsPt2::exportAtlasLayoutPdf()
{
  QgsMapLayer *polygonLayer = mPolygonLayer->clone();
  QgsProject p;
  p.addMapLayers( QList<QgsMapLayer *>() << polygonLayer );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( u"my layout"_s );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:atlaslayouttopdf"_s ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_atlas_layout.pdf";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"my layout"_s );
  parameters.insert( u"COVERAGE_LAYER"_s, QVariant::fromValue( polygonLayer ) );
  parameters.insert( u"OUTPUT"_s, outputPdf );
  parameters.insert( u"DPI"_s, 96 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdf ) );
}

void TestQgsProcessingAlgsPt2::exportAtlasLayoutPdfMultiple()
{
  QgsMapLayer *polygonLayer = mPolygonLayer->clone();
  QgsProject p;
  p.addMapLayers( QList<QgsMapLayer *>() << polygonLayer );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( u"my layout"_s );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:atlaslayouttomultiplepdf"_s ) );
  QVERIFY( alg != nullptr );

  const QString outputPdfDir = QDir::tempPath() + "/atlas_pdf";
  if ( QFile::exists( outputPdfDir ) )
    QDir().rmdir( outputPdfDir );

  QDir().mkdir( outputPdfDir );

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"my layout"_s );
  parameters.insert( u"COVERAGE_LAYER"_s, QVariant::fromValue( polygonLayer ) );
  parameters.insert( u"OUTPUT_FOLDER"_s, outputPdfDir );
  parameters.insert( u"DPI"_s, 96 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdfDir + "/output_1.pdf" ) );
  QVERIFY( QFile::exists( outputPdfDir + "/output_2.pdf" ) );
  QVERIFY( QFile::exists( outputPdfDir + "/output_3.pdf" ) );
}

void TestQgsProcessingAlgsPt2::exportAtlasLayoutPng()
{
  QgsMapLayer *polygonLayer = mPolygonLayer->clone();
  QgsProject p;
  p.addMapLayers( QList<QgsMapLayer *>() << polygonLayer );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( u"my layout"_s );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:atlaslayouttoimage"_s ) );
  QVERIFY( alg != nullptr );

  const QDir tempDir( QDir::tempPath() );
  if ( !tempDir.mkdir( "my_atlas" ) )
  {
    const QDir dir( QDir::tempPath() + "/my_atlas" );
    const QStringList files = dir.entryList( QStringList() << "*.*", QDir::Files );
    for ( const QString &file : files )
      QFile::remove( QDir::tempPath() + "/my_atlas/" + file );
  }

  QVariantMap parameters;
  parameters.insert( u"LAYOUT"_s, u"my layout"_s );
  parameters.insert( u"COVERAGE_LAYER"_s, QVariant::fromValue( polygonLayer ) );
  parameters.insert( u"FOLDER"_s, QString( QDir::tempPath() + "/my_atlas" ) );
  parameters.insert( u"FILENAME_EXPRESSION"_s, u"'export_'||@atlas_featurenumber"_s );
  parameters.insert( u"DPI"_s, 96 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/export_1.png" ) );
  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/export_10.png" ) );
  QGSVERIFYIMAGECHECK( "export_atlas", "export_atlas", QDir::tempPath() + "/my_atlas/export_1.png", QString(), 500, QSize( 3, 3 ) );

  parameters[u"FILENAME_EXPRESSION"_s] = u"'custom_'||@atlas_featurenumber"_s;
  parameters.insert( u"LAYERS"_s, QVariantList() << QVariant::fromValue( mPointsLayer ) );

  ok = false;
  results.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/custom_1.png" ) );
  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/custom_10.png" ) );
  QGSVERIFYIMAGECHECK( "export_atlas_custom_layers", "export_atlas_custom_layers", QDir::tempPath() + "/my_atlas/custom_1.png", QString(), 500, QSize( 3, 3 ) );
}

void TestQgsProcessingAlgsPt2::tinMeshCreation()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:tinmeshcreation"_s ) );
  QVERIFY( alg != nullptr );

  QVariantList inputLayers;

  QVariantMap pointLayer;
  pointLayer[u"source"_s] = "points";
  pointLayer[u"type"_s] = 0;
  pointLayer[u"attributeIndex"_s] = mPointsLayer->fields().indexOf( "Importance" );

  inputLayers.append( pointLayer );

  QVariantMap polyLayer;
  polyLayer[u"source"_s] = "polygons";
  polyLayer[u"type"_s] = 2;
  polyLayer[u"attributeIndex"_s] = mPolygonLayer->fields().indexOf( "Value" );

  inputLayers.append( polyLayer );

  QVariantMap parameters;
  parameters.insert( u"SOURCE_DATA"_s, inputLayers );
  parameters.insert( u"OUTPUT_MESH"_s, QString( QDir::tempPath() + "/meshLayer.2dm" ) );
  parameters.insert( u"MESH_FORMAT"_s, 0 );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsMeshLayer meshLayer( QDir::tempPath() + "/meshLayer.2dm", "mesh", "mdal" );
  QVERIFY( meshLayer.isValid() );

  QgsMeshDataProvider *provider = meshLayer.dataProvider();
  QCOMPARE( provider->vertexCount(), 627 );
  QCOMPARE( provider->faceCount(), 1218 );

  meshLayer.updateTriangularMesh();
  QVERIFY( qgsDoubleNear( meshLayer.datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( -103.0, 39.0 ) ).scalar(), 20.0, 0.001 ) );
  QVERIFY( qgsDoubleNear( meshLayer.datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( -86.0, 35.0 ) ).scalar(), 1.855, 0.001 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshVertices()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:exportmeshvertices"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"VECTOR_OPTION"_s, 2 );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == Qgis::GeometryType::Point );
  QCOMPARE( resultLayer->featureCount(), 5l );
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), u"VertexScalarDataset"_s );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), u"VertexVectorDataset_x"_s );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), u"VertexVectorDataset_y"_s );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), u"VertexVectorDataset_mag"_s );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), u"VertexVectorDataset_dir"_s );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( u"Point Z (1000 2000 20)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( u"Point Z (2000 2000 30)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 3.605, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 56.3099, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( u"Point Z (3000 2000 40)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 5.0, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 53.130, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( u"Point Z (2000 3000 50)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( u"Point Z (1000 3000 10)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), -1.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.236, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 116.565, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshFaces()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:exportmeshfaces"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 3 << 4;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"VECTOR_OPTION"_s, 2 );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == Qgis::GeometryType::Polygon );
  QCOMPARE( resultLayer->featureCount(), 2l );
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), u"FaceScalarDataset"_s );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), u"FaceVectorDataset_x"_s );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), u"FaceVectorDataset_y"_s );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), u"FaceVectorDataset_mag"_s );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), u"FaceVectorDataset_dir"_s );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( u"Polygon Z ((1000 2000 20, 2000 2000 30, 2000 3000 50, 1000 3000 10, 1000 2000 20))"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( u"Polygon Z ((2000 2000 30, 3000 2000 40, 2000 3000 50, 2000 2000 30))"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshEdges()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:exportmeshedges"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer 1D" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"VECTOR_OPTION"_s, 2 );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == Qgis::GeometryType::Line );
  QCOMPARE( resultLayer->featureCount(), 3l );
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), u"EdgeScalarDataset"_s );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), u"EdgeVectorDataset_x"_s );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), u"EdgeVectorDataset_y"_s );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), u"EdgeVectorDataset_mag"_s );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), u"EdgeVectorDataset_dir"_s );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (1000 2000 20, 2000 2000 30)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (2000 2000 30, 3000 2000 40)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (3000 2000 40, 2000 3000 50)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 4.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 5.656, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshOnGrid()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:exportmeshongrid"_s ) );
  QVERIFY( alg != nullptr );

  const QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString meshUri( dataDir + "/mesh/trap_steady_05_3D.nc" );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, meshUri );

  QVariantList datasetGroup;
  for ( int i = 0; i < 12; ++i )
    datasetGroup.append( i );
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"GRID_SPACING"_s, 25.0 );

  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"VECTOR_OPTION"_s, 2 );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == Qgis::GeometryType::Point );
  QCOMPARE( resultLayer->featureCount(), 205l );
  QCOMPARE( resultLayer->fields().count(), 21 );
  QStringList fieldsName;
  fieldsName << u"Bed Elevation"_s << u"temperature"_s << u"temperature/Maximums"_s
             << u"temperature/Minimums"_s << u"temperature/Time at Maximums"_s << u"temperature/Time at Minimums"_s
             << u"velocity_x"_s << u"velocity_y"_s << u"velocity_mag"_s << u"velocity_dir"_s
             << u"velocity/Maximums_x"_s << u"velocity/Maximums_y"_s << u"velocity/Maximums_mag"_s << u"velocity/Maximums_dir"_s
             << u"velocity/Minimums_x"_s << u"velocity/Minimums_y"_s << u"velocity/Minimums_mag"_s << u"velocity/Minimums_dir"_s
             << u"velocity/Time at Maximums"_s << u"velocity/Time at Minimums"_s << u"water depth"_s;

  for ( int i = 0; i < fieldsName.count(); ++i )
    QCOMPARE( fieldsName.at( i ), resultLayer->fields().at( i ).name() );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  for ( int i = 0; i < 8; ++i )
    featIt.nextFeature( feat );
  QCOMPARE( u"Point (25 50)"_s, feat.geometry().asWkt() );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 0 ).toDouble(), -5.025, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 1 ).toDouble(), 1.463, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 2 ).toDouble(), 5.00, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 1.4809e-36, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 0.0305, 2 ) );
}

void TestQgsProcessingAlgsPt2::rasterizeMesh()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:meshrasterize"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"PIXEL_SIZE"_s, 200.0 );

  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  auto outputRaster = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "output", "gdal" );
  QVERIFY( outputRaster );
  QVERIFY( outputRaster->isValid() );
  QgsRasterDataProvider *outputProvider = outputRaster->dataProvider();

  QCOMPARE( outputProvider->bandCount(), 3 );
  QCOMPARE( outputProvider->xSize(), 10 );
  QCOMPARE( outputProvider->ySize(), 5 );

  std::unique_ptr<QgsRasterBlock> outputBlock_1( outputProvider->block( 1, outputRaster->extent(), 10, 5 ) );
  std::unique_ptr<QgsRasterBlock> outputBlock_2( outputProvider->block( 2, outputRaster->extent(), 10, 5 ) );
  std::unique_ptr<QgsRasterBlock> outputBlock_3( outputProvider->block( 3, outputRaster->extent(), 10, 5 ) );

  // load expected result
  const QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  auto expectedRaster = std::make_unique<QgsRasterLayer>( dataDir + "/mesh/rasterized_mesh.tif", "expected", "gdal" );
  QVERIFY( expectedRaster );
  QVERIFY( expectedRaster->isValid() );
  QgsRasterDataProvider *expectedProvider = outputRaster->dataProvider();
  std::unique_ptr<QgsRasterBlock> expectedBlock_1( expectedProvider->block( 1, expectedRaster->extent(), 10, 5 ) );
  std::unique_ptr<QgsRasterBlock> expectedBlock_2( expectedProvider->block( 2, expectedRaster->extent(), 10, 5 ) );
  std::unique_ptr<QgsRasterBlock> expectedBlock_3( expectedProvider->block( 3, expectedRaster->extent(), 10, 5 ) );

  for ( int ix = 0; ix < 10; ++ix )
  {
    for ( int iy = 0; iy < 5; ++iy )
    {
      if ( !( std::isnan( outputBlock_1->value( iy, ix ) ) && std::isnan( expectedBlock_1->value( iy, ix ) ) ) )
        QCOMPARE( outputBlock_1->value( iy, ix ), expectedBlock_1->value( iy, ix ) );
      if ( !( std::isnan( outputBlock_2->value( iy, ix ) ) && std::isnan( expectedBlock_2->value( iy, ix ) ) ) )
        QCOMPARE( outputBlock_2->value( iy, ix ), expectedBlock_2->value( iy, ix ) );
      if ( !( std::isnan( outputBlock_2->value( iy, ix ) ) && std::isnan( expectedBlock_2->value( iy, ix ) ) ) )
        QCOMPARE( outputBlock_3->value( iy, ix ), expectedBlock_3->value( iy, ix ) );
    }
  }
}

void TestQgsProcessingAlgsPt2::exportMeshContours()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:meshcontours"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"OUTPUT_LINES"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"OUTPUT_POLYGONS"_s, QgsProcessing::TEMPORARY_OUTPUT );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min>max
  parameters.insert( u"INCREMENT"_s, 0.5 );
  parameters.insert( u"MINIMUM"_s, 5.0 );
  parameters.insert( u"MAXIMUM"_s, 2.0 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min-max<increrment
  parameters.insert( u"INCREMENT"_s, 10 );
  parameters.insert( u"MINIMUM"_s, 5.0 );
  parameters.insert( u"MAXIMUM"_s, 2.0 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min-max<increrment
  parameters.insert( u"INCREMENT"_s, 2 );
  parameters.insert( u"MINIMUM"_s, 0.25 );
  parameters.insert( u"MAXIMUM"_s, 6.25 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLinesLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT_LINES"_s ).toString() ) );
  QVERIFY( resultLinesLayer );
  QVERIFY( resultLinesLayer->isValid() );
  QgsAttributeList attributeList = resultLinesLayer->attributeList();
  QCOMPARE( resultLinesLayer->fields().count(), 3 );
  QCOMPARE( resultLinesLayer->fields().at( 0 ).name(), u"group"_s );
  QCOMPARE( resultLinesLayer->fields().at( 1 ).name(), u"time"_s );
  QCOMPARE( resultLinesLayer->fields().at( 2 ).name(), u"value"_s );

  QCOMPARE( resultLinesLayer->featureCount(), 4l );
  QgsFeatureIterator featIt = resultLinesLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (1250 3000 20, 1250 2250 27.5, 1250 2000 22.5)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (1006.94319345290614365 3000 10.27772773811624596, 1000 2976.48044676110157525 10.23519553238898538)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexVectorDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (2009.71706923721990279 2990.28293076277986984 49.90282930762779756, 2462.15304528350043256 2000 34.62153045283500319)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexVectorDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 4.25 );
  featIt.nextFeature( feat );
  QCOMPARE( u"LineString Z (1500 3000 30, 1500 2500 35, 1500 2000 25)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"FaceScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );

  QgsVectorLayer *resultpolygonLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT_POLYGONS"_s ).toString() ) );
  QVERIFY( resultpolygonLayer );
  QVERIFY( resultpolygonLayer->isValid() );
  attributeList = resultpolygonLayer->attributeList();
  QCOMPARE( resultpolygonLayer->fields().count(), 4 );
  QCOMPARE( resultpolygonLayer->fields().at( 0 ).name(), u"group"_s );
  QCOMPARE( resultpolygonLayer->fields().at( 1 ).name(), u"time"_s );
  QCOMPARE( resultpolygonLayer->fields().at( 2 ).name(), u"min_value"_s );
  QCOMPARE( resultpolygonLayer->fields().at( 3 ).name(), u"max_value"_s );

  QCOMPARE( resultpolygonLayer->featureCount(), 6l );
  featIt = resultpolygonLayer->getFeatures();
  featIt.nextFeature( feat );
  QgsGeometry geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), u"Polygon Z ((1000 2000 20, 1000 3000 10, 1250 3000 20, 1250 2250 27.5, 1250 2000 22.5, 1000 2000 20))"_s );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), u"Polygon Z ((1250 2000 22.5, 1250 2250 27.5, 1250 3000 20, 2000 3000 50, 3000 2000 40, 2000 2000 30, 1250 2000 22.5))"_s );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 4.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt( 2 ), u"Polygon Z ((1000 2976.48 10.24, 1000 3000 10, 1006.94 3000 10.28, 1000 2976.48 10.24))"_s );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"VertexVectorDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), u"Polygon Z ((1000 2000 20, 1000 3000 10, 1500 3000 30, 1500 2500 35, 1500 2000 25, 1000 2000 20))"_s );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"FaceScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), u"Polygon Z ((1500 2000 25, 1500 2500 35, 1500 3000 30, 2000 3000 50, 3000 2000 40, 2000 2000 30, 1500 2000 25))"_s );
  QCOMPARE( feat.attributes().at( 0 ).toString(), u"FaceScalarDataset"_s );
  QCOMPARE( feat.attributes().at( 1 ).toString(), u"1950-01-01 01:00:00"_s );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 4.25 );

  parameters.insert( u"CONTOUR_LEVEL_LIST"_s, u"4,2,3"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( u"CONTOUR_LEVEL_LIST"_s, u"2,2,3"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( u"CONTOUR_LEVEL_LIST"_s, u"1,2,3"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
}

void TestQgsProcessingAlgsPt2::exportMeshCrossSection()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:meshexportcrosssection"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetTime;
  datasetTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[u"value"_s] = datasetIndex;
  parameters.insert( u"DATASET_TIME"_s, datasetTime );

  parameters.insert( u"RESOLUTION"_s, 100 );

  const QString outputPath = QDir::tempPath() + "/test_mesh_xs.csv";
  parameters.insert( u"OUTPUT"_s, outputPath );

  QgsVectorLayer *layerLine = new QgsVectorLayer( u"LineString"_s, u"lines_for_xs"_s, u"memory"_s );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QStringList wktLines;
  wktLines << u"LineString (1500 2200, 2500 2200)"_s;
  wktLines << u"LineString (1500 1500, 1500 3200)"_s;

  QgsFeatureList flist;
  for ( const QString &wkt : wktLines )
  {
    QgsFeature feat;
    feat.setGeometry( QgsGeometry::fromWkt( wkt ) );
    flist << feat;
  }
  layerLine->dataProvider()->addFeatures( flist );
  QgsProject::instance()->addMapLayer( layerLine );
  QgsProject::instance()->addMapLayer( layerLine );
  parameters.insert( u"INPUT_LINES"_s, layerLine->name() );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QFile outputFile( outputPath );
  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  QTextStream textStream( &outputFile );
  const QString header = textStream.readLine();
  QCOMPARE( header, u"fid,x,y,offset,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset"_s );

  QStringList expectedLines;
  expectedLines << u"1,1500.00,2200.00,0.00,2.50,3.33,2.00"_s
                << u"1,1600.00,2200.00,100.00,2.60,3.41,2.00"_s
                << u"1,1700.00,2200.00,200.00,2.70,3.48,2.00"_s
                << u"1,1800.00,2200.00,300.00,2.80,3.56,2.00"_s
                << u"1,1900.00,2200.00,400.00,2.90,3.64,2.00"_s
                << u"1,2000.00,2200.00,500.00,3.00,3.72,2.00"_s
                << u"1,2100.00,2200.00,600.00,3.10,3.86,3.00"_s
                << u"1,2200.00,2200.00,700.00,3.20,4.00,3.00"_s
                << u"1,2300.00,2200.00,800.00,3.30,4.14,3.00"_s
                << u"1,2400.00,2200.00,900.00,3.40,4.28,3.00"_s
                << u"1,2500.00,2200.00,1000.00,3.50,4.42,3.00"_s
                << u"2,1500.00,1500.00,0.00, , , "_s
                << u"2,1500.00,1600.00,100.00, , , "_s
                << u"2,1500.00,1700.00,200.00, , , "_s
                << u"2,1500.00,1800.00,300.00, , , "_s
                << u"2,1500.00,1900.00,400.00, , , "_s
                << u"2,1500.00,2000.00,500.00,2.50,3.20,2.00"_s
                << u"2,1500.00,2100.00,600.00,2.50,3.26,2.00"_s
                << u"2,1500.00,2200.00,700.00,2.50,3.33,2.00"_s
                << u"2,1500.00,2300.00,800.00,2.50,3.40,2.00"_s
                << u"2,1500.00,2400.00,900.00,2.50,3.47,2.00"_s
                << u"2,1500.00,2500.00,1000.00,2.50,3.54,2.00"_s
                << u"2,1500.00,2600.00,1100.00,2.50,3.33,2.00"_s
                << u"2,1500.00,2700.00,1200.00,2.50,3.14,2.00"_s
                << u"2,1500.00,2800.00,1300.00,2.50,2.97,2.00"_s
                << u"2,1500.00,2900.00,1400.00,2.50,2.82,2.00"_s
                << u"2,1500.00,3000.00,1500.00,2.50,2.69,2.00"_s
                << u"2,1500.00,3100.00,1600.00, , , "_s
                << u"2,1500.00,3200.00,1700.00, , , "_s;
  QString line = textStream.readLine();
  int i = 0;
  QVERIFY( !line.isEmpty() );
  while ( !line.isEmpty() )
  {
    QCOMPARE( line, expectedLines.at( i ) );
    ++i;
    line = textStream.readLine();
  }

  QVERIFY( i == expectedLines.count() );
}

void TestQgsProcessingAlgsPt2::fileDownloader()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:filedownloader"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"URL"_s, u"https://version.qgis.org/version.txt"_s );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  // verify that temporary outputs have the URL file extension appended
  QVERIFY( results.value( u"OUTPUT"_s ).toString().endsWith( ".txt"_L1 ) );

  const QString outputFileName = QgsProcessingUtils::generateTempFilename( u"qgis_version.txt"_s );
  parameters.insert( u"OUTPUT"_s, outputFileName );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  // compare result filename is the same as provided
  QCOMPARE( outputFileName, results.value( u"OUTPUT"_s ).toString() );
}

void TestQgsProcessingAlgsPt2::rasterize()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:rasterize"_s ) );
  QVERIFY( alg != nullptr );

  const QString outputTif = QDir::tempPath() + "/rasterize_output.tif";
  if ( QFile::exists( outputTif ) )
    QFile::remove( outputTif );

  QVariantMap parameters;
  parameters.insert( u"EXTENT"_s, u"-120,-80,15,55"_s );
  parameters.insert( u"TILE_SIZE"_s, 320 );
  parameters.insert( u"MAP_UNITS_PER_PIXEL"_s, 0.125 );
  parameters.insert( u"OUTPUT"_s, outputTif );

  // create a temporary project with three layers, but only two are visible
  // (to test that the algorithm in the default setup without defined LAYERS or MAP_THEME uses only vsisible
  // layers that and in the correct order)
  QgsProject project;
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( dataDir + "/points.shp", u"points"_s, u"ogr"_s );
  QgsVectorLayer *linesLayer = new QgsVectorLayer( dataDir + "/lines.shp", u"lines"_s, u"ogr"_s );
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( dataDir + "/polys.shp", u"polygons"_s, u"ogr"_s );
  QVERIFY( pointsLayer->isValid() && linesLayer->isValid() && polygonLayer->isValid() );
  project.addMapLayers( QList<QgsMapLayer *>() << pointsLayer << linesLayer << polygonLayer );
  QgsLayerTreeLayer *nodePolygons = project.layerTreeRoot()->findLayer( polygonLayer );
  QVERIFY( nodePolygons );
  nodePolygons->setItemVisibilityChecked( false );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &project );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputTif ) );

  QgsRenderChecker checker;
  checker.setControlPathPrefix( u"processing_algorithm"_s );
  checker.setControlExtension( "tif" );
  checker.setControlName( "expected_rasterize" );
  checker.setRenderedImage( outputTif );
  QVERIFY( checker.compareImages( "rasterize", 500 ) );
}

void TestQgsProcessingAlgsPt2::convertGpxFeatureType()
{
  // test generation of babel argument lists
  QStringList processArgs;
  QStringList logArgs;

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( u"/home/me/my input file.gpx"_s, u"/home/me/my output file.gpx"_s, QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromRoute, processArgs, logArgs );
  QCOMPARE( processArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"/home/me/my input file.gpx"_s, u"-x"_s, u"transform,wpt=rte,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"/home/me/my output file.gpx"_s } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"\"/home/me/my input file.gpx\""_s, u"-x"_s, u"transform,wpt=rte,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"\"/home/me/my output file.gpx\""_s } ) );

  logArgs.clear();
  processArgs.clear();
  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( u"/home/me/my input file.gpx"_s, u"/home/me/my output file.gpx"_s, QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromTrack, processArgs, logArgs );
  QCOMPARE( processArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"/home/me/my input file.gpx"_s, u"-x"_s, u"transform,wpt=trk,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"/home/me/my output file.gpx"_s } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"\"/home/me/my input file.gpx\""_s, u"-x"_s, u"transform,wpt=trk,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"\"/home/me/my output file.gpx\""_s } ) );

  logArgs.clear();
  processArgs.clear();

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( u"/home/me/my input file.gpx"_s, u"/home/me/my output file.gpx"_s, QgsConvertGpxFeatureTypeAlgorithm::RouteFromWaypoints, processArgs, logArgs );
  QCOMPARE( processArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"/home/me/my input file.gpx"_s, u"-x"_s, u"transform,rte=wpt,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"/home/me/my output file.gpx"_s } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"\"/home/me/my input file.gpx\""_s, u"-x"_s, u"transform,rte=wpt,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"\"/home/me/my output file.gpx\""_s } ) );


  logArgs.clear();
  processArgs.clear();

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( u"/home/me/my input file.gpx"_s, u"/home/me/my output file.gpx"_s, QgsConvertGpxFeatureTypeAlgorithm::TrackFromWaypoints, processArgs, logArgs );
  QCOMPARE( processArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"/home/me/my input file.gpx"_s, u"-x"_s, u"transform,trk=wpt,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"/home/me/my output file.gpx"_s } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList( { u"-i"_s, u"gpx"_s, u"-f"_s, u"\"/home/me/my input file.gpx\""_s, u"-x"_s, u"transform,trk=wpt,del"_s, u"-o"_s, u"gpx"_s, u"-F"_s, u"\"/home/me/my output file.gpx\""_s } ) );
}

class TestProcessingFeedback : public QgsProcessingFeedback
{
    Q_OBJECT
  public:
    void reportError( const QString &error, bool ) override
    {
      errors << error;
    }

    QStringList errors;
};

void TestQgsProcessingAlgsPt2::convertGpsData()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:convertgpsdata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, u"%1/GARMIN_ATRK.NVM"_s.arg( TEST_DATA_DIR ) );
  parameters.insert( u"FORMAT"_s, u"garmin_xt"_s );
  parameters.insert( u"FEATURE_TYPE"_s, 0 ); // waypoints
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does not support waypoints, exception should have been raised
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << u"The GPSBabel format \u201Cgarmin_xt\u201D does not support converting waypoints."_s );
  feedback.errors.clear();

  parameters.insert( u"FEATURE_TYPE"_s, 1 ); // routes
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does not support routes, exception should have been raised
  QVERIFY( !ok );
  QCOMPARE( feedback.errors, QStringList() << u"The GPSBabel format \u201Cgarmin_xt\u201D does not support converting routes."_s );
  feedback.errors.clear();

  parameters.insert( u"FEATURE_TYPE"_s, 2 ); // tracks
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does support tracks!
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT_LAYER"_s ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->providerType(), u"gpx"_s );
  QCOMPARE( resultLayer->wkbType(), Qgis::WkbType::LineString );
  QCOMPARE( resultLayer->featureCount(), 1LL );

  // algorithm should also run when given the description for a format, not the format name
  parameters.insert( u"FORMAT"_s, u"Mobile Garmin XT Track files"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT_LAYER"_s ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->providerType(), u"gpx"_s );
  QCOMPARE( resultLayer->wkbType(), Qgis::WkbType::LineString );
  QCOMPARE( resultLayer->featureCount(), 1LL );

  // try with a format which doesn't exist
  feedback.errors.clear();
  parameters.insert( u"FORMAT"_s, u"not a format"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( u"Unknown GPSBabel format \u201Cnot a format\u201D."_s ) );
}

void TestQgsProcessingAlgsPt2::downloadGpsData()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:downloadgpsdata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"DEVICE"_s, u"xxx"_s );
  parameters.insert( u"PORT"_s, u"usb:"_s );
  parameters.insert( u"FEATURE_TYPE"_s, 0 ); // waypoints
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid device
  QVERIFY( !ok );

  QVERIFY( feedback.errors.value( 0 ).startsWith( u"Unknown GPSBabel device \u201Cxxx\u201D. Valid devices are:"_s ) );
  feedback.errors.clear();

  parameters.insert( u"DEVICE"_s, u"Garmin serial"_s );
  parameters.insert( u"PORT"_s, u"not a port"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid port
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( u"Unknown port \u201Cnot a port\u201D. Valid ports are:"_s ) );
  feedback.errors.clear();
}

void TestQgsProcessingAlgsPt2::uploadGpsData()
{
  TestProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:downloadgpsdata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"DEVICE"_s, u"xxx"_s );
  parameters.insert( u"PORT"_s, u"usb:"_s );
  parameters.insert( u"FEATURE_TYPE"_s, 0 ); // waypoints
  parameters.insert( u"INPUT"_s, u"%1/layers.gpx"_s.arg( TEST_DATA_DIR ) );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid device
  QVERIFY( !ok );

  QVERIFY( feedback.errors.value( 0 ).startsWith( u"Unknown GPSBabel device \u201Cxxx\u201D. Valid devices are:"_s ) );
  feedback.errors.clear();

  parameters.insert( u"DEVICE"_s, u"Garmin serial"_s );
  parameters.insert( u"PORT"_s, u"not a port"_s );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid port
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( u"Unknown port \u201Cnot a port\u201D. Valid ports are:"_s ) );
  feedback.errors.clear();
}

void TestQgsProcessingAlgsPt2::transferMainAnnotationLayer()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:transferannotationsfrommain"_s ) );
  QVERIFY( alg != nullptr );

  QgsProject p;
  p.mainAnnotationLayer()->addItem( new QgsAnnotationMarkerItem( QgsPoint( 1, 2 ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  QVariantMap parameters;
  parameters.insert( u"LAYER_NAME"_s, u"my annotations"_s );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.mainAnnotationLayer()->items().size(), 0 );
  QgsAnnotationLayer *newLayer = qobject_cast<QgsAnnotationLayer *>( p.mapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QCOMPARE( newLayer->name(), u"my annotations"_s );
  QCOMPARE( newLayer->items().size(), 1 );
}

void TestQgsProcessingAlgsPt2::exportMeshTimeSeries()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:meshexporttimeseries"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( u"DATASET_GROUPS"_s, datasetGroup );

  QVariantMap datasetStartTime;
  datasetStartTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndexStart;
  datasetIndexStart << 1 << 0;
  datasetStartTime[u"value"_s] = datasetIndexStart;
  parameters.insert( u"STARTING_TIME"_s, datasetStartTime );

  QVariantMap datasetEndTime;
  datasetEndTime[u"type"_s] = u"dataset-time-step"_s;
  QVariantList datasetIndexEnd;
  datasetIndexEnd << 1 << 1;
  datasetEndTime[u"value"_s] = datasetIndexEnd;
  parameters.insert( u"FINISHING_TIME"_s, datasetEndTime );

  const QString outputPath = QDir::tempPath() + "/test_mesh_ts.csv";
  parameters.insert( u"OUTPUT"_s, outputPath );

  QgsVectorLayer *layerPoints = new QgsVectorLayer( u"Point"_s, u"points_for_ts"_s, u"memory"_s );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QStringList wktPoints;
  wktPoints << u"Point (1500 2200)"_s;
  wktPoints << u"Point (1500 1500)"_s;
  wktPoints << u"Point (2500 2100)"_s;

  QgsFeatureList flist;
  for ( const QString &wkt : wktPoints )
  {
    QgsFeature feat;
    feat.setGeometry( QgsGeometry::fromWkt( wkt ) );
    flist << feat;
  }
  layerPoints->dataProvider()->addFeatures( flist );
  QgsProject::instance()->addMapLayer( layerPoints );
  QgsProject::instance()->addMapLayer( layerPoints );
  parameters.insert( u"INPUT_POINTS"_s, layerPoints->name() );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QFile outputFile( outputPath );
  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  QTextStream textStream( &outputFile );
  QString header = textStream.readLine();
  QCOMPARE( header, u"fid,x,y,time,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset"_s );

  QStringList expectedLines;
  expectedLines << u"1,1500.00,2200.00,1950-01-01 00:00:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 01:00:00,2.50,3.33,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:00:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 01:00:00,3.50,4.36,3.00"_s;

  QString line = textStream.readLine();
  int i = 0;
  QVERIFY( !line.isEmpty() );
  while ( !line.isEmpty() )
  {
    QCOMPARE( line, expectedLines.at( i ) );
    ++i;
    line = textStream.readLine();
  }
  QVERIFY( i == expectedLines.count() );
  outputFile.close();

  parameters.insert( u"TIME_STEP"_s, 0.1 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  header = textStream.readLine();
  QCOMPARE( header, u"fid,x,y,time,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset"_s );

  expectedLines.clear();
  expectedLines << u"1,1500.00,2200.00,1950-01-01 00:00:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:06:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:12:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:18:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:24:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:30:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:36:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:42:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:48:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 00:54:00,1.50,1.92,1.00"_s
                << u"1,1500.00,2200.00,1950-01-01 01:00:00,2.50,3.33,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:00:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:06:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:12:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:18:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:24:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:30:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:36:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:42:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:48:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 00:54:00,2.50,2.97,2.00"_s
                << u"3,2500.00,2100.00,1950-01-01 01:00:00,3.50,4.36,3.00"_s;

  line = textStream.readLine();
  i = 0;
  QVERIFY( !line.isEmpty() );
  while ( !line.isEmpty() )
  {
    QCOMPARE( line, expectedLines.at( i ) );
    ++i;
    line = textStream.readLine();
  }
  QVERIFY( i == expectedLines.count() );
  outputFile.close();
}

void TestQgsProcessingAlgsPt2::extractLabels()
{
  QgsProject project;
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( mPointLayerPath, u"points"_s, u"ogr"_s );
  QVERIFY( mPointsLayer->isValid() );
  project.addMapLayer( pointsLayer );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setSize( 10 );
  QgsPalLayerSettings settings;
  settings.fieldName = u"Class"_s;
  settings.setFormat( format );
  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:extractlabels"_s ) );
  QVERIFY( alg != nullptr );

  QgsReferencedRectangle extent( QgsRectangle( -120, 20, -80, 50 ), QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  QVariantMap parameters;
  parameters.insert( u"EXTENT"_s, extent );
  parameters.insert( u"SCALE"_s, 9000000.00 );
  parameters.insert( u"DPI"_s, 96.00 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &project );
  QgsProcessingFeedback feedback;

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->wkbType(), Qgis::WkbType::Point );
  QCOMPARE( resultLayer->featureCount(), 17 );

  QgsFeature feature = resultLayer->getFeature( 1 );
  QVariantMap attributes = feature.attributeMap();
  QCOMPARE( attributes[u"Layer"_s], u"points"_s );
  QCOMPARE( attributes[u"FeatureID"_s], 1 );
  QCOMPARE( attributes[u"LabelText"_s], u"Biplane"_s );
  QCOMPARE( attributes[u"LabelRotation"_s], 0.0 );
  QCOMPARE( attributes[u"Family"_s], u"QGIS Vera Sans"_s );
  QCOMPARE( attributes[u"Size"_s], 9.75 );
  QCOMPARE( attributes[u"Italic"_s], false );
  QCOMPARE( attributes[u"Bold"_s], false );
  QCOMPARE( attributes[u"FontStyle"_s], u"Roman"_s );
  QCOMPARE( attributes[u"FontLetterSpacing"_s], 0.0 );
  QCOMPARE( attributes[u"FontWordSpacing"_s], 0.0 );
  QCOMPARE( attributes[u"MultiLineAlignment"_s], u"left"_s );
  QCOMPARE( attributes[u"MultiLineHeight"_s], 1.0 );
}

void TestQgsProcessingAlgsPt2::dxfExport()
{
  QgsProject project;
  project.addMapLayer( mPolygonLayer );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:dxfexport"_s ) );
  QVERIFY( alg != nullptr );

  QgsReferencedRectangle extent( QgsRectangle( -103.9, 25.0, -98.0, 29.8 ), mPolygonLayer->crs() );

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, QStringList() << mPolygonLayer->id() );
  parameters.insert( u"EXTENT"_s, extent );
  parameters.insert( u"SCALE"_s, 1000.00 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( &project );

  TestProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  auto resultLayer = std::make_unique<QgsVectorLayer>( results.value( u"OUTPUT"_s ).toString(), "dxf" );
  QVERIFY( resultLayer->isValid() );
  QCOMPARE( resultLayer->featureCount(), 1L );
  QCOMPARE( resultLayer->wkbType(), Qgis::WkbType::LineString );
}

void TestQgsProcessingAlgsPt2::splitVectorLayer()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << QVariant() );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (0 0)"_s ) );
  layer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 2 << QString( "" ) );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (0 1)"_s ) );
  layer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << u"value"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (0 2)"_s ) );
  layer->dataProvider()->addFeature( f );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:splitvectorlayer"_s ) );
  QVERIFY( alg != nullptr );

  QDir outputDir( QDir::tempPath() + "/split_vector/" );
  if ( outputDir.exists() )
    outputDir.removeRecursively();

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"FIELD"_s, u"col1"_s );
  parameters.insert( u"FILE_TYPE"_s, u"gpkg"_s );
  parameters.insert( u"OUTPUT"_s, outputDir.absolutePath() );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT_LAYERS"_s ).toList().count(), 3 );
  QDir dataDir( outputDir );
  QStringList entries = dataDir.entryList( QStringList(), QDir::Files | QDir::NoDotAndDotDot );
  QCOMPARE( entries.count(), 3 );
}

void TestQgsProcessingAlgsPt2::buffer()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"vl"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:buffer"_s ) );
  QVERIFY( alg != nullptr );

  // buffering empty layer should produce an empty layer
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"DISTANCE"_s, 2.0 );
  parameters.insert( u"SEGMENTS"_s, 5 );
  parameters.insert( u"END_CAP_STYLE"_s, 0 );
  parameters.insert( u"JOIN_STYLE"_s, 0 );
  parameters.insert( u"MITER_LIMIT"_s, 0 );
  parameters.insert( u"DISSOLVE"_s, false );
  parameters.insert( u"OUTPUT"_s, u"memory:"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  QgsVectorLayer *bufferedLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( bufferedLayer->isValid() );
  QCOMPARE( bufferedLayer->wkbType(), Qgis::WkbType::MultiPolygon );
  QCOMPARE( bufferedLayer->featureCount(), layer->featureCount() );

  // buffering empty layer with dissolve should produce an empty layer
  parameters.insert( u"DISSOLVE"_s, true );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );
  bufferedLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( bufferedLayer->isValid() );
  QCOMPARE( bufferedLayer->wkbType(), Qgis::WkbType::MultiPolygon );
  QCOMPARE( bufferedLayer->featureCount(), layer->featureCount() );
}

void TestQgsProcessingAlgsPt2::splitWithLines()
{
  QgsFeature l1_1, l1_2, l2_1, l2_2, p1_1, p1_2, p2_1, p2_2;
  auto lineLayer1 = std::make_unique<QgsVectorLayer>( u"MultiLineString?crs=epsg:4326"_s, u"l1"_s, u"memory"_s );
  QVERIFY( lineLayer1->isValid() );
  l1_1.setGeometry( QgsGeometry::fromWkt( "MultiLineString ((19 40, 26 40),(20 39, 25 39))" ) );
  l1_2.setGeometry( QgsGeometry::fromWkt( "MultiLineString ((19 35, 26 35))" ) );
  lineLayer1->dataProvider()->addFeature( l1_1 );
  lineLayer1->dataProvider()->addFeature( l1_2 );
  auto lineLayer2 = std::make_unique<QgsVectorLayer>( u"MultiLineString?crs=epsg:4326"_s, u"l2"_s, u"memory"_s );
  QVERIFY( lineLayer2->isValid() );
  l2_1.setGeometry( QgsGeometry::fromWkt( "MultiLineString ((20 42, 20 34, 23 42))" ) );
  l2_2.setGeometry( QgsGeometry::fromWkt( "MultiLineString ((21 42, 21 34),(23 42, 23 34, 25 42))" ) );
  lineLayer2->dataProvider()->addFeature( l2_1 );
  lineLayer2->dataProvider()->addFeature( l2_2 );
  auto polygonLayer1 = std::make_unique<QgsVectorLayer>( u"MultiPolygon?crs=epsg:4326"_s, u"p1"_s, u"memory"_s );
  QVERIFY( polygonLayer1->isValid() );
  p1_1.setGeometry( QgsGeometry::fromWkt( "MultiPolygon (((25 41, 25 38, 18 38, 18 41, 25 41),(19 39, 24 39, 24 40, 19 40, 19 39)))" ) );
  p1_2.setGeometry( QgsGeometry::fromWkt( "MultiPolygon (((18 37, 21 37, 21 35, 18 35, 18 37),(19.5 36.5, 19.5 35.5, 20.5 35.5, 20.5 36.5, 19.5 36.5)),((22 37, 25 37, 25 35, 22 35, 22 37),(24 36, 24 35.5, 24.5 35.5, 24.5 36, 24 36),(23.5 35.5, 23.5 36.5, 22.5 36.5, 22.5 35.5, 23.5 35.5)))" ) );
  polygonLayer1->dataProvider()->addFeature( p1_1 );
  polygonLayer1->dataProvider()->addFeature( p1_2 );
  auto polygonLayer2 = std::make_unique<QgsVectorLayer>( u"MultiPolygon?crs=epsg:4326"_s, u"p2"_s, u"memory"_s );
  QVERIFY( polygonLayer2->isValid() );
  p2_1.setGeometry( QgsGeometry::fromWkt( "MultiPolygon (((23 42, 20 34, 20 42, 23 42),(20.5 38.5, 21 38.5, 21.5 40.5, 20.5 40.5, 20.5 38.5)))" ) );
  p2_2.setGeometry( QgsGeometry::fromWkt( "MultiPolygon (((23 34, 23 42, 25 42, 23 34),(24 40.5, 23.5 40.5, 23.5 39.5, 24 40.5)),((19.5 34.5, 17.5 34.5, 17.5 42, 18.5 42, 19.5 34.5),(18.5 37.5, 18 37.5, 18.5 36.5, 18.5 37.5)))" ) );
  polygonLayer2->dataProvider()->addFeature( p2_1 );
  polygonLayer2->dataProvider()->addFeature( p2_2 );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:splitwithlines"_s ) );
  QVERIFY( alg != nullptr );

  // Split lineLayer1 with lineLayer2
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( lineLayer1.get() ) );
  parameters.insert( u"LINES"_s, QVariant::fromValue( lineLayer2.get() ) );
  parameters.insert( u"OUTPUT"_s, u"memory:"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *splitLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( splitLayer->isValid() );
  QCOMPARE( splitLayer->wkbType(), Qgis::WkbType::MultiLineString );
  QCOMPARE( splitLayer->featureCount(), 17 );

  // Split polygonLayer1 with lineLayer2
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer1.get() ) );
  parameters.insert( u"LINES"_s, QVariant::fromValue( lineLayer2.get() ) );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  splitLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( splitLayer->isValid() );
  QCOMPARE( splitLayer->wkbType(), Qgis::WkbType::MultiPolygon );
  QCOMPARE( splitLayer->featureCount(), 16 );

  // Split lineLayer1 with polygonLayer2
  parameters.insert( u"INPUT"_s, QVariant::fromValue( lineLayer1.get() ) );
  parameters.insert( u"LINES"_s, QVariant::fromValue( polygonLayer2.get() ) );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  splitLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( splitLayer->isValid() );
  QCOMPARE( splitLayer->wkbType(), Qgis::WkbType::MultiLineString );
  QCOMPARE( splitLayer->featureCount(), 21 );

  // Split polygonLayer1 with polygonLayer2
  parameters.insert( u"INPUT"_s, QVariant::fromValue( polygonLayer1.get() ) );
  parameters.insert( u"LINES"_s, QVariant::fromValue( polygonLayer2.get() ) );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  splitLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( splitLayer->isValid() );
  QCOMPARE( splitLayer->wkbType(), Qgis::WkbType::MultiPolygon );
  QCOMPARE( splitLayer->featureCount(), 20 );
}

void TestQgsProcessingAlgsPt2::randomPointsInPolygonsFromField_data()
{
  QTest::addColumn<QVariant>( "num_points" );
  QTest::addColumn<int>( "expected" );

  QTest::newRow( "5" ) << QVariant::fromValue<int>( 5 ) << 5;
  QTest::newRow( "NULL" ) << QVariant() << 0;
}

void TestQgsProcessingAlgsPt2::randomPointsInPolygonsFromField()
{
  QFETCH( QVariant, num_points );
  QFETCH( int, expected );

  // Create a polygon memory layer
  QgsVectorLayer polygonLayer { u"Polygon?crs=epsg:4326"_s, u"polygons_points"_s, u"memory"_s };
  // Add an integer field num_points
  QList<QgsField> fields;
  fields.append( QgsField( u"num_points"_s, QMetaType::Type::Int ) );
  polygonLayer.dataProvider()->addAttributes( fields );
  polygonLayer.updateFields();
  // Create a feature
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( u"Polygon ((0 0, 0 10, 10 10, 10 0, 0 0))"_s ) );
  f.setAttributes( QgsAttributes() << num_points );

  // Add the feature to the layer
  polygonLayer.startEditing();
  polygonLayer.addFeature( f );
  polygonLayer.commitChanges();

  // Run algorithm to generate random points in polygons from field num_points
  QVariantMap parameters;

  parameters.insert( u"INPUT"_s, QVariant::fromValue( &polygonLayer ) );
  parameters.insert( u"OUTPUT"_s, u"memory:"_s );
  parameters.insert( u"POINTS_NUMBER"_s, QgsProperty::fromExpression( u"num_points"_s ) );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:randompointsinpolygons"_s ) );
  QVERIFY( alg != nullptr );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->wkbType(), Qgis::WkbType::Point );
  QCOMPARE( resultLayer->featureCount(), expected );
}

void TestQgsProcessingAlgsPt2::generateElevationProfileImage()
{
  auto lineLayer = std::make_unique<QgsVectorLayer>( u"LineStringZ?crs=epsg:3857"_s, u"lines"_s, u"memory"_s );
  QVERIFY( lineLayer->isValid() );
  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( "LineStringZ (0 0 0, 10 10 10)" ) );
  lineLayer->dataProvider()->addFeature( feature );

  QVariantMap properties;
  properties.insert( u"color"_s, u"255,0,0,255"_s );
  properties.insert( u"width"_s, u"1"_s );
  properties.insert( u"capstyle"_s, u"flat"_s );
  dynamic_cast<QgsSingleSymbolRenderer *>( lineLayer->renderer() )->setSymbol( QgsLineSymbol::createSimple( properties ).release() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:generateelevationprofileimage"_s ) );
  QVERIFY( alg != nullptr );

  QgsReferencedGeometry curve( QgsGeometry::fromWkt( "LineString(0 0, 10 10)" ), QgsCoordinateReferenceSystem( "EPSG:3857" ) );

  const QString outputImage = QDir::tempPath() + "/my_elevation_profile.png";
  if ( QFile::exists( outputImage ) )
    QFile::remove( outputImage );

  QVariantMap parameters;
  parameters.insert( u"CURVE"_s, QVariant::fromValue( curve ) );
  parameters.insert( u"MAP_LAYERS"_s, QVariantList() << QVariant::fromValue( lineLayer.get() ) );
  parameters.insert( u"WIDTH"_s, 500 );
  parameters.insert( u"HEIGHT"_s, 350 );
  parameters.insert( u"OUTPUT"_s, outputImage );
  parameters.insert( u"TEXT_FONT_FAMILY"_s, QgsFontUtils::standardTestFontFamily() );
  parameters.insert( u"TEXT_FONT_STYLE"_s, u"Bold"_s );
  parameters.insert( u"TEXT_FONT_SIZE"_s, 12 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputImage ) );
  QGSVERIFYIMAGECHECK( "generate_elevation_profile", "generate_elevation_profile", outputImage, QString(), 500, QSize( 3, 3 ) );
}

void TestQgsProcessingAlgsPt2::copyMetadata()
{
  auto sourceLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( sourceLayer->isValid() );

  auto targetLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"target"_s, u"memory"_s );
  QVERIFY( targetLayer->isValid() );

  QgsLayerMetadata md;
  md.setTitle( u"Title"_s );
  md.setAbstract( u"Abstract"_s );
  sourceLayer->setMetadata( md );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:copylayermetadata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"SOURCE"_s, QVariant::fromValue( sourceLayer.get() ) );
  parameters.insert( u"TARGET"_s, QVariant::fromValue( targetLayer.get() ) );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), targetLayer->id() );

  QgsLayerMetadata targetMetadata = targetLayer->metadata();
  QCOMPARE( targetMetadata.title(), u"Title"_s );
  QCOMPARE( targetMetadata.abstract(), u"Abstract"_s );
}

void TestQgsProcessingAlgsPt2::applyMetadata()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:setlayermetadata"_s ) );
  QVERIFY( alg != nullptr );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString metadataFileName = dataDir + "/simple_metadata.qmd";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"METADATA"_s, metadataFileName );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );

  QgsLayerMetadata targetMetadata = layer->metadata();
  QCOMPARE( targetMetadata.title(), u"Title"_s );
  QCOMPARE( targetMetadata.abstract(), u"Abstract"_s );
}

void TestQgsProcessingAlgsPt2::exportMetadata()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsLayerMetadata md;
  md.setTitle( u"Title"_s );
  md.setAbstract( u"Abstract"_s );
  layer->setMetadata( md );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:exportlayermetadata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  int line, column;
  QString errorMessage;

  QDomDocument doc( u"qgis"_s );
  QFile metadataFile( results.value( u"OUTPUT"_s ).toString() );
  if ( metadataFile.open( QFile::ReadOnly ) )
  {
    ok = doc.setContent( &metadataFile, &errorMessage, &line, &column );
    QVERIFY( ok );
    metadataFile.close();
  }

  const QDomElement root = doc.firstChildElement( u"qgis"_s );
  QVERIFY( !root.isNull() );

  QgsLayerMetadata exportedMetadata;
  exportedMetadata.readMetadataXml( root );
  QCOMPARE( md.title(), exportedMetadata.title() );
  QCOMPARE( md.abstract(), exportedMetadata.abstract() );
}

void TestQgsProcessingAlgsPt2::addHistoryMetadata()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsLayerMetadata md;
  md.setTitle( u"Title"_s );
  md.setAbstract( u"Abstract"_s );
  layer->setMetadata( md );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:addhistorymetadata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"HISTORY"_s, u"do something"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );

  QStringList history = layer->metadata().history();
  QCOMPARE( history.count(), 1 );
  QCOMPARE( history.at( 0 ), u"do something"_s );

  parameters[u"HISTORY"_s] = u"do something else"_s;

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );

  history = layer->metadata().history();
  QCOMPARE( history.count(), 2 );
  QCOMPARE( history.at( 0 ), u"do something"_s );
  QCOMPARE( history.at( 1 ), u"do something else"_s );
}

void TestQgsProcessingAlgsPt2::updateMetadata()
{
  auto sourceLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( sourceLayer->isValid() );

  auto targetLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"target"_s, u"memory"_s );
  QVERIFY( targetLayer->isValid() );

  QgsLayerMetadata mdInput;
  mdInput.setTitle( u"New title"_s );
  mdInput.setAbstract( u"New abstract"_s );
  mdInput.setLanguage( u"Language"_s );
  sourceLayer->setMetadata( mdInput );

  QgsLayerMetadata mdTarget;
  mdTarget.setTitle( u"Title"_s );
  mdTarget.setAbstract( u"Abstract"_s );
  mdTarget.setType( u"Type"_s );
  targetLayer->setMetadata( mdTarget );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:updatelayermetadata"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"SOURCE"_s, QVariant::fromValue( sourceLayer.get() ) );
  parameters.insert( u"TARGET"_s, QVariant::fromValue( targetLayer.get() ) );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), targetLayer->id() );

  QgsLayerMetadata targetMetadata = targetLayer->metadata();
  QCOMPARE( targetMetadata.title(), u"New title"_s );
  QCOMPARE( targetMetadata.abstract(), u"New abstract"_s );
  QCOMPARE( targetMetadata.language(), u"Language"_s );
  QCOMPARE( targetMetadata.type(), u"Type"_s );
}

void TestQgsProcessingAlgsPt2::setMetadataFields()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=pk:int&field=col1:string"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:setmetadatafields"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"TITLE"_s, u"Title"_s );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );
  QCOMPARE( layer->metadata().title(), u"Title"_s );

  // update existing field and set new
  parameters[u"TITLE"_s] = u"New title"_s;
  parameters.insert( u"ABSTRACT"_s, u"Abstract"_s );
  parameters.insert( u"FEES"_s, u"Enormous fee"_s );
  parameters.insert( u"CRS"_s, QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );
  QCOMPARE( layer->metadata().title(), u"New title"_s );
  QCOMPARE( layer->metadata().abstract(), u"Abstract"_s );
  QCOMPARE( layer->metadata().fees(), u"Enormous fee"_s );
  QVERIFY( layer->metadata().crs().isValid() );
  QCOMPARE( layer->metadata().crs().authid(), u"EPSG:4326"_s );

  // ignore empty field
  parameters[u"TITLE"_s] = QString();
  parameters.insert( u"IGNORE_EMPTY"_s, true );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );
  QCOMPARE( layer->metadata().title(), u"New title"_s );
  QCOMPARE( layer->metadata().abstract(), u"Abstract"_s );
  QCOMPARE( layer->metadata().fees(), u"Enormous fee"_s );
  QVERIFY( layer->metadata().crs().isValid() );
  QCOMPARE( layer->metadata().crs().authid(), u"EPSG:4326"_s );

  parameters[u"IGNORE_EMPTY"_s] = false;

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );
  QCOMPARE( layer->metadata().title(), QString() );
  QCOMPARE( layer->metadata().abstract(), u"Abstract"_s );
  QCOMPARE( layer->metadata().fees(), u"Enormous fee"_s );
  QVERIFY( layer->metadata().crs().isValid() );
  QCOMPARE( layer->metadata().crs().authid(), u"EPSG:4326"_s );
}

void TestQgsProcessingAlgsPt2::mergeVectors()
{
  auto baseLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=id:int(10)&field=test_precision:double(10,2)&field=test_length:string(8)"_s, u"base"_s, u"memory"_s );
  QVERIFY( baseLayer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << 1.25 << u"01234567"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (0 0)"_s ) );
  baseLayer->dataProvider()->addFeature( f );

  auto mergeLayer = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=id:int(10)&field=test_precision:double(10,4)&field=test_length:string(10)"_s, u"to_merge"_s, u"memory"_s );
  QVERIFY( mergeLayer->isValid() );

  f.setAttributes( QgsAttributes() << 1 << 1.2515 << u"0123456789"_s );
  f.setGeometry( QgsGeometry::fromWkt( u"Point (1 1)"_s ) );
  mergeLayer->dataProvider()->addFeature( f );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:mergevectorlayers"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, QVariantList() << QVariant::fromValue( baseLayer.get() ) << QVariant::fromValue( mergeLayer.get() ) );
  parameters.insert( u"ADD_SOURCE_FIELDS"_s, false );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( u"OUTPUT"_s ).toString().isEmpty() );

  QgsVectorLayer *resultLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == Qgis::GeometryType::Point );
  QCOMPARE( resultLayer->featureCount(), 2 );
  QCOMPARE( resultLayer->fields().count(), 3 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), u"id"_s );
  QCOMPARE( resultLayer->fields().at( 0 ).length(), 10 );
  QCOMPARE( resultLayer->fields().at( 0 ).precision(), 0 );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), u"test_precision"_s );
  QCOMPARE( resultLayer->fields().at( 1 ).length(), 10 );
  QCOMPARE( resultLayer->fields().at( 1 ).precision(), 4 );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), u"test_length"_s );
  QCOMPARE( resultLayer->fields().at( 2 ).length(), 10 );
  QCOMPARE( resultLayer->fields().at( 2 ).precision(), 0 );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( u"Point (0 0)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 1.2500 );
  QCOMPARE( feat.attributes().at( 2 ).toString(), u"01234567"_s );

  featIt.nextFeature( feat );
  QCOMPARE( u"Point (1 1)"_s, feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 1.2515 );
  QCOMPARE( feat.attributes().at( 2 ).toString(), u"0123456789"_s );
}

void TestQgsProcessingAlgsPt2::nativeAlgsRasterSize()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"LineString?crs=epsg:32633"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( u"LineString(656000 4551184, 656184 4551000)"_s ) );
  layer->dataProvider()->addFeature( f );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  // reference raster was generated by gdal_rasterize algorithm using following parameters
  // UNITS: Georeferenced units
  // WIDTH: 10
  // HEIGHT: 10
  // EXTENT: 656000,656184,4551000,4551184 [EPSG:32633]
  auto gdalRasterLayer = std::make_unique<QgsRasterLayer>( dataDir + u"/raster/gdal_rasterize.tif"_s, "gdal_rasterize", "gdal" );

  // create constant raster
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:createconstantrasterlayer"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.clear();
  parameters.insert( u"EXTENT"_s, u"656000,656184,4551000,4551184 [EPSG:32633]"_s );
  parameters.insert( u"NUMBER"_s, 1 );
  parameters.insert( u"OUTPUT_TYPE"_s, 0 );
  parameters.insert( u"PIXEL_SIZE"_s, 10 );
  parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( "EPSG:32633" ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  auto constantRasterLayer = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "constant", "gdal" );

  QCOMPARE( constantRasterLayer->extent(), gdalRasterLayer->extent() );
  QCOMPARE( constantRasterLayer->height(), gdalRasterLayer->height() );
  QCOMPARE( constantRasterLayer->width(), gdalRasterLayer->width() );

  // create random raster (it is enough to check only one alg here as they have same base class)
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:createrandomuniformrasterlayer"_s ) );
  QVERIFY( alg != nullptr );

  parameters.clear();
  parameters.insert( u"EXTENT"_s, u"656000,656184,4551000,4551184 [EPSG:32633]"_s );
  parameters.insert( u"OUTPUT_TYPE"_s, 0 );
  parameters.insert( u"PIXEL_SIZE"_s, 10 );
  parameters.insert( u"TARGET_CRS"_s, QgsCoordinateReferenceSystem( "EPSG:32633" ) );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  auto randomRasterLayer = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "random", "gdal" );

  QCOMPARE( randomRasterLayer->extent(), gdalRasterLayer->extent() );
  QCOMPARE( randomRasterLayer->height(), gdalRasterLayer->height() );
  QCOMPARE( randomRasterLayer->width(), gdalRasterLayer->width() );

  // line density
  alg.reset( QgsApplication::processingRegistry()->createAlgorithmById( u"native:linedensity"_s ) );
  QVERIFY( alg != nullptr );

  parameters.clear();
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"PIXEL_SIZE"_s, 10 );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  auto densityRasterLayer = std::make_unique<QgsRasterLayer>( results.value( u"OUTPUT"_s ).toString(), "density", "gdal" );

  QCOMPARE( densityRasterLayer->extent(), gdalRasterLayer->extent() );
  QCOMPARE( densityRasterLayer->height(), gdalRasterLayer->height() );
  QCOMPARE( densityRasterLayer->width(), gdalRasterLayer->width() );
}

void TestQgsProcessingAlgsPt2::defineProjection()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Point"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:definecurrentprojection"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"CRS"_s, QgsCoordinateReferenceSystem( u"EPSG:3857"_s ) );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( u"OUTPUT"_s ), layer->id() );
  QVERIFY( layer->crs().isValid() );
  QCOMPARE( layer->crs().authid(), u"EPSG:3857"_s );

  // check that .prj file is create and .qpj file is deleted
  const QTemporaryDir tmpPath;
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QFile::copy( dataDir + "/points.shp", tmpPath.filePath( u"points.shp"_s ) );
  QFile::copy( dataDir + "/points.shx", tmpPath.filePath( u"points.shx"_s ) );
  QFile::copy( dataDir + "/points.dbf", tmpPath.filePath( u"points.dbf"_s ) );
  QFile::copy( dataDir + "/points.qpj", tmpPath.filePath( u"points.qpj"_s ) );

  layer = std::make_unique<QgsVectorLayer>( tmpPath.filePath( u"points.shp"_s ), u"input"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QVERIFY( !layer->crs().isValid() );

  parameters.insert( u"INPUT"_s, QVariant::fromValue( layer.get() ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QFile prjFile( tmpPath.filePath( u"points.prj"_s ) );
  QVERIFY( prjFile.exists() );
  QFile qpjFile( tmpPath.filePath( u"points.qpj"_s ) );
  QVERIFY( !qpjFile.exists() );
}

void TestQgsProcessingAlgsPt2::checkValidity()
{
  auto layer = std::make_unique<QgsVectorLayer>( u"Polygon?crs=epsg:4326&field=int_f:int"_s, u"input"_s, u"memory"_s );
  QVERIFY( layer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromWkt( u"POLYGON ((0 0, 2 2, 0 2, 2 0, 0 0))"_s ) );
  QVERIFY( f.isValid() );
  layer->dataProvider()->addFeature( f );

  f.setAttributes( QgsAttributes() << 2 );
  f.setGeometry( QgsGeometry::fromWkt( u"POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))"_s ) );
  QVERIFY( f.isValid() );
  layer->dataProvider()->addFeature( f );
  QCOMPARE( layer->featureCount(), 2 );

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"native:checkvalidity"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT_LAYER"_s, QVariant::fromValue( layer.get() ) );
  parameters.insert( u"VALID_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"INVALID_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( u"ERROR_OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  // QGIS method
  parameters.insert( u"METHOD"_s, 1 );

  bool ok = false;
  auto context = std::make_unique<QgsProcessingContext>();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *invalidLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"INVALID_OUTPUT"_s ).toString() ) );
  QCOMPARE( invalidLayer->fields().at( invalidLayer->fields().size() - 1 ).name(), u"_errors"_s );
  QCOMPARE( invalidLayer->featureCount(), 1 );
  QgsFeatureIterator it = invalidLayer->getFeatures();
  it.nextFeature( f );
  QCOMPARE( f.attributes(), QgsAttributes() << 1 << u"segments 0 and 2 of line 0 intersect at 1, 1"_s );

  // GEOS method
  parameters.insert( u"METHOD"_s, 2 );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  invalidLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( u"INVALID_OUTPUT"_s ).toString() ) );
  QCOMPARE( invalidLayer->fields().at( invalidLayer->fields().size() - 1 ).name(), u"_errors"_s );
  QCOMPARE( invalidLayer->featureCount(), 1 );
  it = invalidLayer->getFeatures();
  it.nextFeature( f );
  QCOMPARE( f.attributes(), QgsAttributes() << 1 << u"Self-intersection"_s );
}

QGSTEST_MAIN( TestQgsProcessingAlgsPt2 )
#include "testqgsprocessingalgspt2.moc"
