/***************************************************************************
                         testqgsprocessingalgs.cpp
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

#include "qgstest.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsnativealgorithms.h"

class TestQgsProcessingAlgs: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void packageAlg();
    void renameLayerAlg();
    void loadLayerAlg();

  private:

    QString mPointLayerPath;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

};

void TestQgsProcessingAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  QString pointsFileName = dataDir + "/points.shp";
  QFileInfo pointFileInfo( pointsFileName );
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
  QString polysFileName = dataDir + "/polys.shp";
  QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                      QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPolygonLayer );
  QVERIFY( mPolygonLayer->isValid() );
}

void TestQgsProcessingAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingAlgs::packageAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:package" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  QString outputGpkg = QDir::tempPath() + "/package_alg.gpkg";
  if ( QFile::exists( outputGpkg ) )
    QFile::remove( outputGpkg );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << mPointsLayer->id() << mPolygonLayer->id() );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );
  bool ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  context.reset();

  QVERIFY( !results.value( QStringLiteral( "OUTPUT" ) ).toString().isEmpty() );
  std::unique_ptr< QgsVectorLayer > pointLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=points", "points", "ogr" );
  QVERIFY( pointLayer->isValid() );
  QCOMPARE( pointLayer->wkbType(), mPointsLayer->wkbType() );
  QCOMPARE( pointLayer->featureCount(), mPointsLayer->featureCount() );
  std::unique_ptr< QgsVectorLayer > polygonLayer = qgis::make_unique< QgsVectorLayer >( outputGpkg + "|layername=polygons", "polygons", "ogr" );
  QVERIFY( polygonLayer->isValid() );
  QCOMPARE( polygonLayer->wkbType(), mPolygonLayer->wkbType() );
  QCOMPARE( polygonLayer->featureCount(), mPolygonLayer->featureCount() );
}

void TestQgsProcessingAlgs::renameLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:renamelayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QCOMPARE( layer->name(), QStringLiteral( "layer" ) );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( layer ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name" ) );
  QCOMPARE( results.value( "OUTPUT" ), QVariant::fromValue( layer ) );

  // with input layer name as parameter
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "new name" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name2" ) );
  ok = false;
  results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QCOMPARE( layer->name(), QStringLiteral( "new name2" ) );
  // result should use new name as value
  QCOMPARE( results.value( "OUTPUT" ).toString(), QStringLiteral( "new name2" ) );
}

void TestQgsProcessingAlgs::loadLayerAlg()
{
  const QgsProcessingAlgorithm *package( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "native:loadlayer" ) ) );
  QVERIFY( package );

  std::unique_ptr< QgsProcessingContext > context = qgis::make_unique< QgsProcessingContext >();
  QgsProject p;
  context->setProject( &p );

  QgsProcessingFeedback feedback;

  QVariantMap parameters;

  // bad layer
  parameters.insert( QStringLiteral( "INPUT" ), QStringLiteral( "bad layer" ) );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "new name" ) );
  bool ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //invalid name
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QString() );
  ok = false;
  ( void )package->run( parameters, *context, &feedback, &ok );
  QVERIFY( !ok );
  QVERIFY( context->layersToLoadOnCompletion().empty() );

  //good params
  parameters.insert( QStringLiteral( "INPUT" ), mPointLayerPath );
  parameters.insert( QStringLiteral( "NAME" ), QStringLiteral( "my layer" ) );
  ok = false;
  QVariantMap results = package->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );
  QVERIFY( !context->layersToLoadOnCompletion().empty() );
  QString layerId = context->layersToLoadOnCompletion().keys().at( 0 );
  QCOMPARE( results.value( QStringLiteral( "OUTPUT" ) ).toString(), layerId );
  QVERIFY( !layerId.isEmpty() );
  QVERIFY( context->temporaryLayerStore()->mapLayer( layerId ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).name, QStringLiteral( "my layer" ) );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).project, &p );
  QCOMPARE( context->layersToLoadOnCompletion().value( layerId, QgsProcessingContext::LayerDetails( QString(), nullptr, QString() ) ).outputName, QStringLiteral( "my layer" ) );
}


QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
