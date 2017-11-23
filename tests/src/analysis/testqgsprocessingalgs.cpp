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

  private:

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
  mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
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


QGSTEST_MAIN( TestQgsProcessingAlgs )
#include "testqgsprocessingalgs.moc"
