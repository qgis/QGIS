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
#include "limits"

#include "qgstest.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"
#include "qgsalgorithmfillnodata.h"
#include "qgsalgorithmlinedensity.h"
#include "qgsalgorithmimportphotos.h"
#include "qgsalgorithmtransform.h"
#include "qgsalgorithmkmeansclustering.h"
#include "qgsvectorlayer.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmultipolygon.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasteranalysisutils.cpp"
#include "qgsrasterfilewriter.h"
#include "qgsreclassifyutils.h"
#include "qgsalgorithmrasterlogicalop.h"
#include "qgsprintlayout.h"
#include "qgslayertree.h"
#include "qgslayoutmanager.h"
#include "qgslayoutitemmap.h"
#include "qgsmarkersymbollayer.h"
#include "qgsrulebasedrenderer.h"
#include "qgspallabeling.h"
#include "qgsrastershader.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgslayoutitemscalebar.h"
#include "annotations/qgstextannotation.h"
#include "qgsfontutils.h"
#include "annotations/qgsannotationmanager.h"
#include "qgsvectorlayerlabeling.h"
#include "qgsstyle.h"
#include "qgsbookmarkmanager.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrenderchecker.h"
#include "qgsrelationmanager.h"
#include "qgsmeshlayer.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgsalgorithmgpsbabeltools.h"
#include "qgsannotationlayer.h"
#include "qgsannotationmarkeritem.h"
#include "qgscolorrampimpl.h"
#include "qgstextformat.h"

class TestQgsProcessingAlgsPt2: public QObject
{
    Q_OBJECT

  private:

    /**
     * Helper function to get a feature based algorithm.
     */
    std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> featureBasedAlg( const QString &id );

    QgsFeature runForFeature( const std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters = QVariantMap() );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

#ifndef QT_NO_PRINTER
    void exportLayoutPdf();
    void exportLayoutPng();
    void exportAtlasLayoutPdf();
    void exportAtlasLayoutPdfMultiple();
    void exportAtlasLayoutPng();
#endif

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

    void splitVectorLayer();
    void buffer();

  private:

    bool imageCheck( const QString &testName, const QString &renderedImage );

    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

    void exportToSpreadsheet( const QString &outputPath );
};

std::unique_ptr<QgsProcessingFeatureBasedAlgorithm> TestQgsProcessingAlgsPt2::featureBasedAlg( const QString &id )
{
  return std::unique_ptr<QgsProcessingFeatureBasedAlgorithm>( static_cast<QgsProcessingFeatureBasedAlgorithm *>( QgsApplication::processingRegistry()->createAlgorithmById( id ) ) );
}

QgsFeature TestQgsProcessingAlgsPt2::runForFeature( const std::unique_ptr< QgsProcessingFeatureBasedAlgorithm > &alg, QgsFeature feature, const QString &layerType, QVariantMap parameters )
{
  Q_ASSERT( alg.get() );
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;
  context->setFeedback( &feedback );

  std::unique_ptr<QgsVectorLayer> inputLayer( std::make_unique<QgsVectorLayer>( layerType, QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
  inputLayer->dataProvider()->addFeature( feature );

  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue<QgsMapLayer *>( inputLayer.get() ) );

  parameters.insert( QStringLiteral( "OUTPUT" ), QStringLiteral( "memory:" ) );

  bool ok = false;
  const auto res = alg->run( parameters, *context, &feedback, &ok );
  QgsFeature result;

  std::unique_ptr<QgsVectorLayer> outputLayer( qobject_cast< QgsVectorLayer * >( context->getMapLayer( res.value( QStringLiteral( "OUTPUT" ) ).toString() ) ) );
  outputLayer->getFeatures().nextFeature( result );

  return result;
}

void TestQgsProcessingAlgsPt2::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QString pointsFileName = dataDir + "/points.shp";
  const QFileInfo pointFileInfo( pointsFileName );
  mPointLayerPath = pointFileInfo.filePath();
  mPointsLayer = new QgsVectorLayer( mPointLayerPath,
                                     QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointsLayer->isValid() );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString polysFileName = dataDir + "/polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                      QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );

  //add a mesh layer
  const QString uri( dataDir + "/mesh/quad_and_triangle.2dm" );
  const QString meshLayerName = QStringLiteral( "mesh layer" );
  QgsMeshLayer *meshLayer = new QgsMeshLayer( uri, meshLayerName, QStringLiteral( "mdal" ) );
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
  const QString meshLayer1dName = QStringLiteral( "mesh layer 1D" );
  QgsMeshLayer *meshLayer1d = new QgsMeshLayer( uri1d, meshLayer1dName, QStringLiteral( "mdal" ) );
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
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), layers );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );
  parameters.insert( QStringLiteral( "OVERWRITE" ), overwrite );
  parameters.insert( QStringLiteral( "SELECTED_FEATURES_ONLY" ), selectedFeaturesOnly );
  parameters.insert( QStringLiteral( "SAVE_METADATA" ), saveMetadata );
  return package->run( parameters, *context, &feedback, ok );
}

void TestQgsProcessingAlgsPt2::exportToSpreadsheet( const QString &outputPath )
{
  if ( QFile::exists( outputPath ) )
    QFile::remove( outputPath );

  QVariantMap parameters;
  const QStringList layers = QStringList() << mPointsLayer->id() << mPolygonLayer->id();
  bool ok = false;

  const QgsProcessingAlgorithm *alg( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:exporttospreadsheet" ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  parameters.insert( QStringLiteral( "LAYERS" ), layers );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPath );
  parameters.insert( QStringLiteral( "OVERWRITE" ), false );
  const QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > pointLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  pointLayer.reset();
  std::unique_ptr< QgsVectorLayer > polygonLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
  polygonLayer.reset();

  std::unique_ptr<QgsVectorLayer> rectangles = std::make_unique<QgsVectorLayer>( QStringLiteral( TEST_DATA_DIR ) + "/rectangles.shp",
      QStringLiteral( "rectangles" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << rectangles.get() );

  // Test adding an additional layer (overwrite disabled)
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << rectangles->id() );
  const QVariantMap results2 = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results2.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > rectanglesPackagedLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );
  rectanglesPackagedLayer.reset();

  pointLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  pointLayer.reset();

  // And finally, test with overwrite enabled
  parameters.insert( QStringLiteral( "OVERWRITE" ), true );
  const QVariantMap results3 = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results3.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  rectanglesPackagedLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=rectangles", "points", "ogr" );
  QVERIFY( rectanglesPackagedLayer->isValid() );
  QCOMPARE( rectanglesPackagedLayer->featureCount(), rectangles->featureCount() );

  pointLayer = std::make_unique< QgsVectorLayer >( outputPath + "|layername=points", "points", "ogr" );
  QVERIFY( !pointLayer->isValid() ); // It's gone -- the xlsx was recreated with a single layer
}

#ifndef QT_NO_PRINTER
void TestQgsProcessingAlgsPt2::exportLayoutPdf()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( QStringLiteral( "my layout" ) );
  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:printlayouttopdf" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_layout.pdf";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "missing" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPdf );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPdf ) );

  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( outputPdf ) );
}

void TestQgsProcessingAlgsPt2::exportLayoutPng()
{
  QgsProject p;
  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( QStringLiteral( "my layout" ) );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( mPointsLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:printlayouttoimage" ) ) );
  QVERIFY( alg != nullptr );

  QString outputPng = QDir::tempPath() + "/my_layout.png";
  if ( QFile::exists( outputPng ) )
    QFile::remove( outputPng );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "missing" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPng );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid layout name
  QVERIFY( !ok );
  QVERIFY( !QFile::exists( outputPng ) );

  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputPng ) );

  outputPng = QDir::tempPath() + "/my_layout_custom_layers.png";
  if ( QFile::exists( outputPng ) )
    QFile::remove( outputPng );

  parameters.insert( QStringLiteral( "OUTPUT" ), outputPng );
  parameters.insert( QStringLiteral( "LAYERS" ), QVariantList() << QVariant::fromValue( mPointsLayer ) );
  parameters.insert( QStringLiteral( "DPI" ), 96 );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputPng ) );
  QVERIFY( imageCheck( "export_layout_custom_layers", outputPng ) );
}

void TestQgsProcessingAlgsPt2::exportAtlasLayoutPdf()
{
  QgsMapLayer *polygonLayer = mPolygonLayer->clone();
  QgsProject p;
  p.addMapLayers( QList<QgsMapLayer *>() << polygonLayer );

  QgsPrintLayout *layout = new QgsPrintLayout( &p );
  layout->initializeDefaults();
  layout->setName( QStringLiteral( "my layout" ) );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:atlaslayouttopdf" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputPdf = QDir::tempPath() + "/my_atlas_layout.pdf";
  if ( QFile::exists( outputPdf ) )
    QFile::remove( outputPdf );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  parameters.insert( QStringLiteral( "COVERAGE_LAYER" ), QVariant::fromValue( polygonLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPdf );
  parameters.insert( QStringLiteral( "DPI" ), 96 );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
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
  layout->setName( QStringLiteral( "my layout" ) );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:atlaslayouttomultiplepdf" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputPdfDir = QDir::tempPath() + "/atlas_pdf";
  if ( QFile::exists( outputPdfDir ) )
    QDir().rmdir( outputPdfDir );

  QDir().mkdir( outputPdfDir );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  parameters.insert( QStringLiteral( "COVERAGE_LAYER" ), QVariant::fromValue( polygonLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT_FOLDER" ), outputPdfDir );
  parameters.insert( QStringLiteral( "DPI" ), 96 );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
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
  layout->setName( QStringLiteral( "my layout" ) );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( layout );
  map->setBackgroundEnabled( false );
  map->setFrameEnabled( false );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  layout->addLayoutItem( map );
  map->setExtent( polygonLayer->extent() );

  p.layoutManager()->addLayout( layout );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:atlaslayouttoimage" ) ) );
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
  parameters.insert( QStringLiteral( "LAYOUT" ), QStringLiteral( "my layout" ) );
  parameters.insert( QStringLiteral( "COVERAGE_LAYER" ), QVariant::fromValue( polygonLayer ) );
  parameters.insert( QStringLiteral( "FOLDER" ), QString( QDir::tempPath() + "/my_atlas" ) );
  parameters.insert( QStringLiteral( "FILENAME_EXPRESSION" ), QStringLiteral( "'export_'||@atlas_featurenumber" ) );
  parameters.insert( QStringLiteral( "DPI" ), 96 );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/export_1.png" ) );
  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/export_10.png" ) );
  QVERIFY( imageCheck( "export_atlas", QDir::tempPath() + "/my_atlas/export_1.png" ) );

  parameters[QStringLiteral( "FILENAME_EXPRESSION" )] = QStringLiteral( "'custom_'||@atlas_featurenumber" );
  parameters.insert( QStringLiteral( "LAYERS" ), QVariantList() << QVariant::fromValue( mPointsLayer ) );

  ok = false;
  results.clear();
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/custom_1.png" ) );
  QVERIFY( QFile::exists( QDir::tempPath() + "/my_atlas/custom_10.png" ) );
  QVERIFY( imageCheck( "export_atlas_custom_layers", QDir::tempPath() + "/my_atlas/custom_1.png" ) );
}
#endif

void TestQgsProcessingAlgsPt2::tinMeshCreation()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:tinmeshcreation" ) ) );
  QVERIFY( alg != nullptr );

  QVariantList inputLayers;

  QVariantMap pointLayer;
  pointLayer[QStringLiteral( "source" )] = "points";
  pointLayer[QStringLiteral( "type" )] = 0;
  pointLayer[QStringLiteral( "attributeIndex" )] = mPointsLayer->fields().indexOf( "Importance" );

  inputLayers.append( pointLayer );

  QVariantMap polyLayer;
  polyLayer[QStringLiteral( "source" )] = "polygons";
  polyLayer[QStringLiteral( "type" )] = 2;
  polyLayer[QStringLiteral( "attributeIndex" )] = mPolygonLayer->fields().indexOf( "Value" );

  inputLayers.append( polyLayer );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "SOURCE_DATA" ), inputLayers );
  parameters.insert( QStringLiteral( "OUTPUT_MESH" ), QString( QDir::tempPath() + "/meshLayer.2dm" ) );
  parameters.insert( QStringLiteral( "MESH_FORMAT" ), 0 );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
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
  QVERIFY( qgsDoubleNear( meshLayer.datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( -86.0, 35.0 ) ).scalar(), 1.855, 0.001 ) ) ;
}

void TestQgsProcessingAlgsPt2::exportMeshVertices()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:exportmeshvertices" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "VECTOR_OPTION" ), 2 );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == QgsWkbTypes::PointGeometry );
  QCOMPARE( resultLayer->featureCount(), 5l );
  const QgsAttributeList attributeList = resultLayer->attributeList();
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), QStringLiteral( "VertexScalarDataset" ) );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), QStringLiteral( "VertexVectorDataset_x" ) );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), QStringLiteral( "VertexVectorDataset_y" ) );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), QStringLiteral( "VertexVectorDataset_mag" ) );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), QStringLiteral( "VertexVectorDataset_dir" ) );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PointZ (1000 2000 20)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PointZ (2000 2000 30)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 3.605, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 56.3099, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PointZ (3000 2000 40)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 5.0, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 53.130, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PointZ (2000 3000 50)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45, 2 ) );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PointZ (1000 3000 10)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), -1.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.236, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 116.565, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshFaces()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:exportmeshfaces" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 3 << 4;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "VECTOR_OPTION" ), 2 );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == QgsWkbTypes::PolygonGeometry );
  QCOMPARE( resultLayer->featureCount(), 2l );
  const QgsAttributeList attributeList = resultLayer->attributeList();
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), QStringLiteral( "FaceScalarDataset" ) );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), QStringLiteral( "FaceVectorDataset_x" ) );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), QStringLiteral( "FaceVectorDataset_y" ) );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), QStringLiteral( "FaceVectorDataset_mag" ) );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), QStringLiteral( "FaceVectorDataset_dir" ) );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PolygonZ ((1000 2000 20, 2000 2000 30, 2000 3000 50, 1000 3000 10, 1000 2000 20))" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "PolygonZ ((2000 2000 30, 3000 2000 40, 2000 3000 50, 2000 2000 30))" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshEdges()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:exportmeshedges" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer 1D" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "VECTOR_OPTION" ), 2 );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == QgsWkbTypes::LineGeometry );
  QCOMPARE( resultLayer->featureCount(), 3l );
  const QgsAttributeList attributeList = resultLayer->attributeList();
  QCOMPARE( resultLayer->fields().count(), 5 );
  QCOMPARE( resultLayer->fields().at( 0 ).name(), QStringLiteral( "EdgeScalarDataset" ) );
  QCOMPARE( resultLayer->fields().at( 1 ).name(), QStringLiteral( "EdgeVectorDataset_x" ) );
  QCOMPARE( resultLayer->fields().at( 2 ).name(), QStringLiteral( "EdgeVectorDataset_y" ) );
  QCOMPARE( resultLayer->fields().at( 3 ).name(), QStringLiteral( "EdgeVectorDataset_mag" ) );
  QCOMPARE( resultLayer->fields().at( 4 ).name(), QStringLiteral( "EdgeVectorDataset_dir" ) );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (1000 2000 20, 2000 2000 30)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 2.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 2.828, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (2000 2000 30, 3000 2000 40)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 3.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 3.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 4.242, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );

  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (3000 2000 40, 2000 3000 50)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 1 ).toDouble(), 4.0 );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 4.0 );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 5.656, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 45.0, 2 ) );
}

void TestQgsProcessingAlgsPt2::exportMeshOnGrid()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:exportmeshongrid" ) ) );
  QVERIFY( alg != nullptr );

  const QString dataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString meshUri( dataDir + "/mesh/trap_steady_05_3D.nc" );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), meshUri );

  QVariantList datasetGroup;
  for ( int i = 0; i < 12; ++i )
    datasetGroup.append( i );
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "GRID_SPACING" ), 25.0 );

  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "VECTOR_OPTION" ), 2 );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( resultLayer );
  QVERIFY( resultLayer->isValid() );
  QVERIFY( resultLayer->geometryType() == QgsWkbTypes::PointGeometry );
  QCOMPARE( resultLayer->featureCount(), 205l );
  const QgsAttributeList attributeList = resultLayer->attributeList();
  QCOMPARE( resultLayer->fields().count(), 21 );
  QStringList fieldsName;
  fieldsName << QStringLiteral( "Bed Elevation" ) << QStringLiteral( "temperature" ) << QStringLiteral( "temperature/Maximums" )
             << QStringLiteral( "temperature/Minimums" ) << QStringLiteral( "temperature/Time at Maximums" ) << QStringLiteral( "temperature/Time at Minimums" )
             << QStringLiteral( "velocity_x" ) << QStringLiteral( "velocity_y" ) << QStringLiteral( "velocity_mag" ) << QStringLiteral( "velocity_dir" )
             << QStringLiteral( "velocity/Maximums_x" ) << QStringLiteral( "velocity/Maximums_y" ) << QStringLiteral( "velocity/Maximums_mag" ) << QStringLiteral( "velocity/Maximums_dir" )
             << QStringLiteral( "velocity/Minimums_x" ) << QStringLiteral( "velocity/Minimums_y" ) << QStringLiteral( "velocity/Minimums_mag" ) << QStringLiteral( "velocity/Minimums_dir" )
             << QStringLiteral( "velocity/Time at Maximums" ) << QStringLiteral( "velocity/Time at Minimums" ) << QStringLiteral( "water depth" );

  for ( int i = 0; i < fieldsName.count(); ++i )
    QCOMPARE( fieldsName.at( i ), resultLayer->fields().at( i ).name() );

  QgsFeatureIterator featIt = resultLayer->getFeatures();
  QgsFeature feat;
  for ( int i = 0; i < 8; ++i )
    featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "Point (25 50)" ), feat.geometry().asWkt() );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 0 ).toDouble(), -5.025, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 1 ).toDouble(), 1.424, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 2 ).toDouble(), 5.00, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 3 ).toDouble(), 1.32e-36, 2 ) );
  QVERIFY( qgsDoubleNearSig( feat.attributes().at( 4 ).toDouble(), 0.02776, 2 ) );

}

void TestQgsProcessingAlgsPt2::rasterizeMesh()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:meshrasterize" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "PIXEL_SIZE" ), 200.0 );

  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  std::unique_ptr<QgsRasterLayer> outputRaster = std::make_unique< QgsRasterLayer >( results.value( QStringLiteral( "OUTPUT" ) ).toString(), "output", "gdal" );
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
  std::unique_ptr<QgsRasterLayer> expectedRaster = std::make_unique< QgsRasterLayer >( dataDir + "/mesh/rasterized_mesh.tif", "expected", "gdal" );
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
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:meshcontours" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "OUTPUT_LINES" ), QgsProcessing::TEMPORARY_OUTPUT );
  parameters.insert( QStringLiteral( "OUTPUT_POLYGONS" ), QgsProcessing::TEMPORARY_OUTPUT );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min>max
  parameters.insert( QStringLiteral( "INCREMENT" ), 0.5 );
  parameters.insert( QStringLiteral( "MINIMUM" ), 5.0 );
  parameters.insert( QStringLiteral( "MAXIMUM" ), 2.0 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min-max<increrment
  parameters.insert( QStringLiteral( "INCREMENT" ), 10 );
  parameters.insert( QStringLiteral( "MINIMUM" ), 5.0 );
  parameters.insert( QStringLiteral( "MAXIMUM" ), 2.0 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  // min-max<increrment
  parameters.insert( QStringLiteral( "INCREMENT" ), 2 );
  parameters.insert( QStringLiteral( "MINIMUM" ), 0.25 );
  parameters.insert( QStringLiteral( "MAXIMUM" ), 6.25 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLinesLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT_LINES" ) ).toString() ) );
  QVERIFY( resultLinesLayer );
  QVERIFY( resultLinesLayer->isValid() );
  QgsAttributeList attributeList = resultLinesLayer->attributeList();
  QCOMPARE( resultLinesLayer->fields().count(), 3 );
  QCOMPARE( resultLinesLayer->fields().at( 0 ).name(), QStringLiteral( "group" ) );
  QCOMPARE( resultLinesLayer->fields().at( 1 ).name(), QStringLiteral( "time" ) );
  QCOMPARE( resultLinesLayer->fields().at( 2 ).name(), QStringLiteral( "value" ) );

  QCOMPARE( resultLinesLayer->featureCount(), 4l );
  QgsFeatureIterator featIt = resultLinesLayer->getFeatures();
  QgsFeature feat;
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (1250 3000 20, 1250 2250 27.5, 1250 2000 22.5)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (1006.94319345290614365 3000 10.27772773811624596, 1000 2976.48044676110157525 10.23519553238898538)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexVectorDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (2009.71706923721990279 2990.28293076277986984 49.90282930762779756, 2462.15304528350043256 2000 34.62153045283500319)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexVectorDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 4.25 );
  featIt.nextFeature( feat );
  QCOMPARE( QStringLiteral( "LineStringZ (1500 3000 30, 1500 2500 35, 1500 2000 25)" ), feat.geometry().asWkt() );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "FaceScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );

  QgsVectorLayer *resultpolygonLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT_POLYGONS" ) ).toString() ) );
  QVERIFY( resultpolygonLayer );
  QVERIFY( resultpolygonLayer->isValid() );
  attributeList = resultpolygonLayer->attributeList();
  QCOMPARE( resultpolygonLayer->fields().count(), 4 );
  QCOMPARE( resultpolygonLayer->fields().at( 0 ).name(), QStringLiteral( "group" ) );
  QCOMPARE( resultpolygonLayer->fields().at( 1 ).name(), QStringLiteral( "time" ) );
  QCOMPARE( resultpolygonLayer->fields().at( 2 ).name(), QStringLiteral( "min_value" ) );
  QCOMPARE( resultpolygonLayer->fields().at( 3 ).name(), QStringLiteral( "max_value" ) );

  QCOMPARE( resultpolygonLayer->featureCount(), 6l );
  featIt = resultpolygonLayer->getFeatures();
  featIt.nextFeature( feat );
  QgsGeometry geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), QStringLiteral( "PolygonZ ((1000 2000 20, 1000 3000 10, 1250 3000 20, 1250 2250 27.5, 1250 2000 22.5, 1000 2000 20))" ) );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), QStringLiteral( "PolygonZ ((1250 2000 22.5, 1250 2250 27.5, 1250 3000 20, 2000 3000 50, 3000 2000 40, 2000 2000 30, 1250 2000 22.5))" ) );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 4.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt( 2 ), QStringLiteral( "PolygonZ ((1000 2976.48 10.24, 1000 3000 10, 1006.94 3000 10.28, 1000 2976.48 10.24))" ) );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "VertexVectorDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), QStringLiteral( "PolygonZ ((1000 2000 20, 1000 3000 10, 1500 3000 30, 1500 2500 35, 1500 2000 25, 1000 2000 20))" ) );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "FaceScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 0.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 2.25 );
  featIt.nextFeature( feat );
  geom = feat.geometry();
  geom.normalize();
  QCOMPARE( geom.asWkt(), QStringLiteral( "PolygonZ ((1500 2000 25, 1500 2500 35, 1500 3000 30, 2000 3000 50, 3000 2000 40, 2000 2000 30, 1500 2000 25))" ) );
  QCOMPARE( feat.attributes().at( 0 ).toString(), QStringLiteral( "FaceScalarDataset" ) );
  QCOMPARE( feat.attributes().at( 1 ).toString(), QStringLiteral( "1950-01-01 01:00:00" ) );
  QCOMPARE( feat.attributes().at( 2 ).toDouble(), 2.25 );
  QCOMPARE( feat.attributes().at( 3 ).toDouble(), 4.25 );

  parameters.insert( QStringLiteral( "CONTOUR_LEVEL_LIST" ), QStringLiteral( "4,2,3" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "CONTOUR_LEVEL_LIST" ), QStringLiteral( "2,2,3" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  parameters.insert( QStringLiteral( "CONTOUR_LEVEL_LIST" ), QStringLiteral( "1,2,3" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
}

void TestQgsProcessingAlgsPt2::exportMeshCrossSection()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:meshexportcrosssection" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetTime;
  datasetTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndex;
  datasetIndex << 1 << 1;
  datasetTime[QStringLiteral( "value" )] = datasetIndex;
  parameters.insert( QStringLiteral( "DATASET_TIME" ), datasetTime );

  parameters.insert( QStringLiteral( "RESOLUTION" ), 100 );

  const QString outputPath = QDir::tempPath() + "/test_mesh_xs.csv";
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPath );

  QgsVectorLayer *layerLine = new QgsVectorLayer( QStringLiteral( "LineString" ),
      QStringLiteral( "lines_for_xs" ),
      QStringLiteral( "memory" ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QStringList wktLines;
  wktLines << QStringLiteral( "LineString (1500 2200, 2500 2200)" );
  wktLines << QStringLiteral( "LineString (1500 1500, 1500 3200)" );

  QgsFeatureList flist;
  for ( const QString &wkt : wktLines )
  {
    QgsFeature feat;
    feat.setGeometry( QgsGeometry::fromWkt( wkt ) );
    flist << feat;
  }
  layerLine->dataProvider()->addFeatures( flist );
  QgsProject::instance()->addMapLayer( layerLine );  QgsProject::instance()->addMapLayer( layerLine );
  parameters.insert( QStringLiteral( "INPUT_LINES" ), layerLine->name() );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QFile outputFile( outputPath );
  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  QTextStream textStream( &outputFile );
  const QString header = textStream.readLine();
  QCOMPARE( header, QStringLiteral( "fid,x,y,offset,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset" ) );

  QStringList expectedLines;
  expectedLines << QStringLiteral( "1,1500.00,2200.00,0.00,2.50,3.33,2.00" )
                << QStringLiteral( "1,1600.00,2200.00,100.00,2.60,3.41,2.00" )
                << QStringLiteral( "1,1700.00,2200.00,200.00,2.70,3.48,2.00" )
                << QStringLiteral( "1,1800.00,2200.00,300.00,2.80,3.56,2.00" )
                << QStringLiteral( "1,1900.00,2200.00,400.00,2.90,3.64,2.00" )
                << QStringLiteral( "1,2000.00,2200.00,500.00,3.00,3.72,2.00" )
                << QStringLiteral( "1,2100.00,2200.00,600.00,3.10,3.86,3.00" )
                << QStringLiteral( "1,2200.00,2200.00,700.00,3.20,4.00,3.00" )
                << QStringLiteral( "1,2300.00,2200.00,800.00,3.30,4.14,3.00" )
                << QStringLiteral( "1,2400.00,2200.00,900.00,3.40,4.28,3.00" )
                << QStringLiteral( "1,2500.00,2200.00,1000.00,3.50,4.42,3.00" )
                << QStringLiteral( "2,1500.00,1500.00,0.00, , , " )
                << QStringLiteral( "2,1500.00,1600.00,100.00, , , " )
                << QStringLiteral( "2,1500.00,1700.00,200.00, , , " )
                << QStringLiteral( "2,1500.00,1800.00,300.00, , , " )
                << QStringLiteral( "2,1500.00,1900.00,400.00, , , " )
                << QStringLiteral( "2,1500.00,2000.00,500.00,2.50,3.20,2.00" )
                << QStringLiteral( "2,1500.00,2100.00,600.00,2.50,3.26,2.00" )
                << QStringLiteral( "2,1500.00,2200.00,700.00,2.50,3.33,2.00" )
                << QStringLiteral( "2,1500.00,2300.00,800.00,2.50,3.40,2.00" )
                << QStringLiteral( "2,1500.00,2400.00,900.00,2.50,3.47,2.00" )
                << QStringLiteral( "2,1500.00,2500.00,1000.00,2.50,3.54,2.00" )
                << QStringLiteral( "2,1500.00,2600.00,1100.00,2.50,3.33,2.00" )
                << QStringLiteral( "2,1500.00,2700.00,1200.00,2.50,3.14,2.00" )
                << QStringLiteral( "2,1500.00,2800.00,1300.00,2.50,2.97,2.00" )
                << QStringLiteral( "2,1500.00,2900.00,1400.00,2.50,2.82,2.00" )
                << QStringLiteral( "2,1500.00,3000.00,1500.00,2.50,2.69,2.00" )
                << QStringLiteral( "2,1500.00,3100.00,1600.00, , , " )
                << QStringLiteral( "2,1500.00,3200.00,1700.00, , , " );
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
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:filedownloader" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "URL" ), QStringLiteral( "https://version.qgis.org/version.txt" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  // verify that temporary outputs have the URL file extension appended
  QVERIFY( results.value( QStringLiteral( "OUTPUT" ) ).toString().endsWith( QLatin1String( ".txt" ) ) );
}

void TestQgsProcessingAlgsPt2::rasterize()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:rasterize" ) ) );
  QVERIFY( alg != nullptr );

  const QString outputTif = QDir::tempPath() + "/rasterize_output.tif";
  if ( QFile::exists( outputTif ) )
    QFile::remove( outputTif );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "EXTENT" ), QStringLiteral( "-120,-80,15,55" ) );
  parameters.insert( QStringLiteral( "TILE_SIZE" ), 320 );
  parameters.insert( QStringLiteral( "MAP_UNITS_PER_PIXEL" ), 0.125 );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputTif );

  // create a temporary project with three layers, but only two are visible
  // (to test that the algorithm in the default setup without defined LAYERS or MAP_THEME uses only vsisible
  // layers that and in the correct order)
  QgsProject project;
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( dataDir + "/points.shp", QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer *linesLayer = new QgsVectorLayer( dataDir + "/lines.shp", QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( dataDir + "/polys.shp", QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() && linesLayer->isValid() && polygonLayer->isValid() );
  project.addMapLayers( QList<QgsMapLayer *>() << pointsLayer << linesLayer << polygonLayer );
  QgsLayerTreeLayer *nodePolygons = project.layerTreeRoot()->findLayer( polygonLayer );
  QVERIFY( nodePolygons );
  nodePolygons->setItemVisibilityChecked( false );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &project );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( QFile::exists( outputTif ) );

  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "processing_algorithm" ) );
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

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( QStringLiteral( "/home/me/my input file.gpx" ),
      QStringLiteral( "/home/me/my output file.gpx" ),
      QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromRoute,
      processArgs, logArgs );
  QCOMPARE( processArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "/home/me/my input file.gpx" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,wpt=rte,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "/home/me/my output file.gpx" )
  } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "\"/home/me/my input file.gpx\"" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,wpt=rte,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "\"/home/me/my output file.gpx\"" )
  } ) );

  logArgs.clear();
  processArgs.clear();
  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( QStringLiteral( "/home/me/my input file.gpx" ),
      QStringLiteral( "/home/me/my output file.gpx" ),
      QgsConvertGpxFeatureTypeAlgorithm::WaypointsFromTrack,
      processArgs, logArgs );
  QCOMPARE( processArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "/home/me/my input file.gpx" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,wpt=trk,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "/home/me/my output file.gpx" )
  } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "\"/home/me/my input file.gpx\"" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,wpt=trk,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "\"/home/me/my output file.gpx\"" )
  } ) );

  logArgs.clear();
  processArgs.clear();

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( QStringLiteral( "/home/me/my input file.gpx" ),
      QStringLiteral( "/home/me/my output file.gpx" ),
      QgsConvertGpxFeatureTypeAlgorithm::RouteFromWaypoints,
      processArgs, logArgs );
  QCOMPARE( processArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "/home/me/my input file.gpx" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,rte=wpt,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "/home/me/my output file.gpx" )
  } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "\"/home/me/my input file.gpx\"" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,rte=wpt,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "\"/home/me/my output file.gpx\"" )
  } ) );


  logArgs.clear();
  processArgs.clear();

  QgsConvertGpxFeatureTypeAlgorithm::createArgumentLists( QStringLiteral( "/home/me/my input file.gpx" ),
      QStringLiteral( "/home/me/my output file.gpx" ),
      QgsConvertGpxFeatureTypeAlgorithm::TrackFromWaypoints,
      processArgs, logArgs );
  QCOMPARE( processArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "/home/me/my input file.gpx" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,trk=wpt,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "/home/me/my output file.gpx" )
  } ) );
  // when showing the babel command, filenames should be wrapped in "", which is what QProcess does internally (hence the processArgs don't have these)
  QCOMPARE( logArgs, QStringList(
  {
    QStringLiteral( "-i" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-f" ),
    QStringLiteral( "\"/home/me/my input file.gpx\"" ),
    QStringLiteral( "-x" ),
    QStringLiteral( "transform,trk=wpt,del" ),
    QStringLiteral( "-o" ),
    QStringLiteral( "gpx" ),
    QStringLiteral( "-F" ),
    QStringLiteral( "\"/home/me/my output file.gpx\"" )
  } ) );
}

class TestProcessingFeedback : public QgsProcessingFeedback
{
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

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:convertgpsdata" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "%1/GARMIN_ATRK.NVM" ).arg( TEST_DATA_DIR ) );
  parameters.insert( QStringLiteral( "FORMAT" ), QStringLiteral( "garmin_xt" ) );
  parameters.insert( QStringLiteral( "FEATURE_TYPE" ), 0 ); // waypoints
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does not support waypoints, exception should have been raised
  QVERIFY( !ok );

  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "The GPSBabel format \u201Cgarmin_xt\u201D does not support converting waypoints." ) );
  feedback.errors.clear();

  parameters.insert( QStringLiteral( "FEATURE_TYPE" ), 1 ); // routes
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does not support routes, exception should have been raised
  QVERIFY( !ok );
  QCOMPARE( feedback.errors, QStringList() << QStringLiteral( "The GPSBabel format \u201Cgarmin_xt\u201D does not support converting routes." ) );
  feedback.errors.clear();

  parameters.insert( QStringLiteral( "FEATURE_TYPE" ), 2 ); // tracks
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // garmin_xt format does support tracks!
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT_LAYER" ) ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->providerType(), QStringLiteral( "gpx" ) );
  QCOMPARE( resultLayer->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( resultLayer->featureCount(), 1LL );

  // algorithm should also run when given the description for a format, not the format name
  parameters.insert( QStringLiteral( "FORMAT" ), QStringLiteral( "Mobile Garmin XT Track files" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT_LAYER" ) ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->providerType(), QStringLiteral( "gpx" ) );
  QCOMPARE( resultLayer->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( resultLayer->featureCount(), 1LL );

  // try with a format which doesn't exist
  feedback.errors.clear();
  parameters.insert( QStringLiteral( "FORMAT" ), QStringLiteral( "not a format" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( QStringLiteral( "Unknown GPSBabel format \u201Cnot a format\u201D." ) ) );
}

void TestQgsProcessingAlgsPt2::downloadGpsData()
{
  TestProcessingFeedback feedback;

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:downloadgpsdata" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "DEVICE" ), QStringLiteral( "xxx" ) );
  parameters.insert( QStringLiteral( "PORT" ), QStringLiteral( "usb:" ) );
  parameters.insert( QStringLiteral( "FEATURE_TYPE" ), 0 ); // waypoints
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid device
  QVERIFY( !ok );

  QVERIFY( feedback.errors.value( 0 ).startsWith( QStringLiteral( "Unknown GPSBabel device \u201Cxxx\u201D. Valid devices are:" ) ) );
  feedback.errors.clear();

  parameters.insert( QStringLiteral( "DEVICE" ), QStringLiteral( "Garmin serial" ) );
  parameters.insert( QStringLiteral( "PORT" ), QStringLiteral( "not a port" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid port
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( QStringLiteral( "Unknown port \u201Cnot a port\u201D. Valid ports are:" ) ) );
  feedback.errors.clear();
}

void TestQgsProcessingAlgsPt2::uploadGpsData()
{
  TestProcessingFeedback feedback;

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:downloadgpsdata" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "DEVICE" ), QStringLiteral( "xxx" ) );
  parameters.insert( QStringLiteral( "PORT" ), QStringLiteral( "usb:" ) );
  parameters.insert( QStringLiteral( "FEATURE_TYPE" ), 0 ); // waypoints
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "%1/layers.gpx" ).arg( TEST_DATA_DIR ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid device
  QVERIFY( !ok );

  QVERIFY( feedback.errors.value( 0 ).startsWith( QStringLiteral( "Unknown GPSBabel device \u201Cxxx\u201D. Valid devices are:" ) ) );
  feedback.errors.clear();

  parameters.insert( QStringLiteral( "DEVICE" ), QStringLiteral( "Garmin serial" ) );
  parameters.insert( QStringLiteral( "PORT" ), QStringLiteral( "not a port" ) );
  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  // invalid port
  QVERIFY( !ok );
  QVERIFY( feedback.errors.value( 0 ).startsWith( QStringLiteral( "Unknown port \u201Cnot a port\u201D. Valid ports are:" ) ) );
  feedback.errors.clear();
}

void TestQgsProcessingAlgsPt2::transferMainAnnotationLayer()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:transferannotationsfrommain" ) ) );
  QVERIFY( alg != nullptr );

  QgsProject p;
  p.mainAnnotationLayer()->addItem( new QgsAnnotationMarkerItem( QgsPoint( 1, 2 ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &p );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYER_NAME" ), QStringLiteral( "my annotations" ) );
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( p.mainAnnotationLayer()->items().size(), 0 );
  QgsAnnotationLayer *newLayer = qobject_cast< QgsAnnotationLayer * >( p.mapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QCOMPARE( newLayer->name(), QStringLiteral( "my annotations" ) );
  QCOMPARE( newLayer->items().size(), 1 );
}

void TestQgsProcessingAlgsPt2::exportMeshTimeSeries()
{
  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:meshexporttimeseries" ) ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), "mesh layer" );

  QVariantList datasetGroup;
  datasetGroup << 1 << 2 << 3;
  parameters.insert( QStringLiteral( "DATASET_GROUPS" ), datasetGroup );

  QVariantMap datasetStartTime;
  datasetStartTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndexStart;
  datasetIndexStart << 1 << 0;
  datasetStartTime[QStringLiteral( "value" )] = datasetIndexStart;
  parameters.insert( QStringLiteral( "STARTING_TIME" ), datasetStartTime );

  QVariantMap datasetEndTime;
  datasetEndTime[QStringLiteral( "type" )] = QStringLiteral( "dataset-time-step" );
  QVariantList datasetIndexEnd;
  datasetIndexEnd << 1 << 1;
  datasetEndTime[QStringLiteral( "value" )] = datasetIndexEnd;
  parameters.insert( QStringLiteral( "FINISHING_TIME" ), datasetEndTime );

  const QString outputPath = QDir::tempPath() + "/test_mesh_ts.csv";
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPath );

  QgsVectorLayer *layerPoints = new QgsVectorLayer( QStringLiteral( "Point" ),
      QStringLiteral( "points_for_ts" ),
      QStringLiteral( "memory" ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );
  QgsProcessingFeedback feedback;
  QVariantMap results;
  bool ok = false;

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );

  QStringList wktPoints;
  wktPoints << QStringLiteral( "Point (1500 2200)" );
  wktPoints << QStringLiteral( "Point (1500 1500)" );
  wktPoints << QStringLiteral( "Point (2500 2100)" );

  QgsFeatureList flist;
  for ( const QString &wkt : wktPoints )
  {
    QgsFeature feat;
    feat.setGeometry( QgsGeometry::fromWkt( wkt ) );
    flist << feat;
  }
  layerPoints->dataProvider()->addFeatures( flist );
  QgsProject::instance()->addMapLayer( layerPoints );  QgsProject::instance()->addMapLayer( layerPoints );
  parameters.insert( QStringLiteral( "INPUT_POINTS" ), layerPoints->name() );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QFile outputFile( outputPath );
  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  QTextStream textStream( &outputFile );
  QString header = textStream.readLine();
  QCOMPARE( header, QStringLiteral( "fid,x,y,time,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset" ) );

  QStringList expectedLines;
  expectedLines << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:00:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 01:00:00,2.50,3.33,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:00:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 01:00:00,3.50,4.36,3.00" );

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

  parameters.insert( QStringLiteral( "TIME_STEP" ), 0.1 );

  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( outputFile.open( QIODevice::ReadOnly ) );
  header = textStream.readLine();
  QCOMPARE( header, QStringLiteral( "fid,x,y,time,VertexScalarDataset,VertexVectorDataset,FaceScalarDataset" ) );

  expectedLines.clear();
  expectedLines << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:00:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:06:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:12:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:18:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:24:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:30:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:36:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:42:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:48:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 00:54:00,1.50,1.92,1.00" )
                << QStringLiteral( "1,1500.00,2200.00,1950-01-01 01:00:00,2.50,3.33,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:00:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:06:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:12:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:18:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:24:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:30:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:36:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:42:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:48:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 00:54:00,2.50,2.97,2.00" )
                << QStringLiteral( "3,2500.00,2100.00,1950-01-01 01:00:00,3.50,4.36,3.00" );

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
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( mPointLayerPath,
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointsLayer->isValid() );
  project.addMapLayer( pointsLayer );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setSize( 10 );
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.setFormat( format );
  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:extractlabels" ) ) );
  QVERIFY( alg != nullptr );

  QgsReferencedRectangle extent( QgsRectangle( -120, 20, -80, 50 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "EXTENT" ), extent );
  parameters.insert( QStringLiteral( "SCALE" ), 9000000.00 );
  parameters.insert( QStringLiteral( "DPI" ), 96.00 );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( &project );
  QgsProcessingFeedback feedback;

  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *resultLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( resultLayer );
  QCOMPARE( resultLayer->wkbType(), QgsWkbTypes::Point );
  QCOMPARE( resultLayer->featureCount(), 17 );

  QgsFeature feature = resultLayer->getFeature( 1 );
  QVariantMap attributes = feature.attributeMap();
  QCOMPARE( attributes[QStringLiteral( "Layer" )], QStringLiteral( "points" ) );
  QCOMPARE( attributes[QStringLiteral( "FeatureID" )], 1 );
  QCOMPARE( attributes[QStringLiteral( "LabelText" )], QStringLiteral( "Biplane" ) );
  QCOMPARE( attributes[QStringLiteral( "LabelRotation" )], 0.0 );
  QCOMPARE( attributes[QStringLiteral( "Family" )], QStringLiteral( "QGIS Vera Sans" ) );
  QCOMPARE( attributes[QStringLiteral( "Size" )], 9.75 );
  QCOMPARE( attributes[QStringLiteral( "Italic" )], false );
  QCOMPARE( attributes[QStringLiteral( "Bold" )], false );
  QCOMPARE( attributes[QStringLiteral( "FontStyle" )], QStringLiteral( "Roman" ) );
  QCOMPARE( attributes[QStringLiteral( "FontLetterSpacing" )], 0.0 );
  QCOMPARE( attributes[QStringLiteral( "FontWordSpacing" )], 0.0 );
  QCOMPARE( attributes[QStringLiteral( "MultiLineAlignment" )], QStringLiteral( "left" ) );
  QCOMPARE( attributes[QStringLiteral( "MultiLineHeight" )], 1.0 );
}

void TestQgsProcessingAlgsPt2::splitVectorLayer()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 << QVariant() );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (0 0)" ) ) );
  layer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 2 << QStringLiteral( "" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (0 1)" ) ) );
  layer->dataProvider()->addFeature( f );
  f.setAttributes( QgsAttributes() << 3 << QStringLiteral( "value" ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Point (0 2)" ) ) );
  layer->dataProvider()->addFeature( f );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:splitvectorlayer" ) ) );
  QVERIFY( alg != nullptr );

  QDir outputDir( QDir::tempPath() + "/split_vector/" );
  if ( outputDir.exists() )
    outputDir.removeRecursively();

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "FIELD" ), QStringLiteral( "col1" ) );
  parameters.insert( QStringLiteral( "FILE_TYPE" ), QStringLiteral( "gpkg" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputDir.absolutePath() );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QCOMPARE( results.value( QStringLiteral( "OUTPUT_LAYERS" ) ).toList().count(), 3 );
  QDir dataDir( outputDir );
  QStringList entries = dataDir.entryList( QStringList(), QDir::Files | QDir::NoDotAndDotDot );
  QCOMPARE( entries.count(), 3 );
}

void TestQgsProcessingAlgsPt2::buffer()
{
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=col1:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  std::unique_ptr< QgsProcessingAlgorithm > alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:buffer" ) ) );
  QVERIFY( alg != nullptr );

  // buffering empty layer should produce an empty layer
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "DISTANCE" ), 2.0 );
  parameters.insert( QStringLiteral( "SEGMENTS" ), 5 );
  parameters.insert( QStringLiteral( "END_CAP_STYLE" ), 0 );
  parameters.insert( QStringLiteral( "JOIN_STYLE" ), 0 );
  parameters.insert( QStringLiteral( "MITER_LIMIT" ), 0 );
  parameters.insert( QStringLiteral( "DISSOLVE" ), false );
  parameters.insert( QStringLiteral( "OUTPUT" ), QStringLiteral( "memory:" ) );

  bool ok = false;
  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  QgsProcessingFeedback feedback;
  QVariantMap results;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  QgsVectorLayer *bufferedLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( bufferedLayer->isValid() );
  QCOMPARE( bufferedLayer->wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( bufferedLayer->featureCount(), layer->featureCount() );

  // buffering empty layer with dissolve should produce an empty layer
  parameters.insert( QStringLiteral( "DISSOLVE" ), true );

  ok = false;
  results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  bufferedLayer = qobject_cast< QgsVectorLayer * >( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( bufferedLayer->isValid() );
  QCOMPARE( bufferedLayer->wkbType(), QgsWkbTypes::MultiPolygon );
  QCOMPARE( bufferedLayer->featureCount(), layer->featureCount() );
}

bool TestQgsProcessingAlgsPt2::imageCheck( const QString &testName, const QString &renderedImage )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "processing_algorithm" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( renderedImage );
  checker.setSizeTolerance( 3, 3 );
  const bool equal = checker.compareImages( testName, 500 );
  return equal;
}

QGSTEST_MAIN( TestQgsProcessingAlgsPt2 )
#include "testqgsprocessingalgspt2.moc"
