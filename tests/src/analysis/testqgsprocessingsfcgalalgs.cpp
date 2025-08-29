/***************************************************************************
                         testqgsprocessingsfcgalalgs.cpp
                         ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
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
#include "qgsnativealgorithms.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingcontext.h"
#include "qgsvectorlayer.h"

// #include <QThread>

class TestQgsProcessingSfcgalAlgs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingSfcgalAlgs()
      : QgsTest( QStringLiteral( "Processing SFCGAL Algorithms Test" ) )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void medialAxis();

  private:
    QgsVectorLayer *mPolygonLayer = nullptr;
};

void TestQgsProcessingSfcgalAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsNativeAlgorithms( QgsApplication::processingRegistry() ) );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QString polysFileName = dataDir + "/polys.shp";
  const QFileInfo polyFileInfo( polysFileName );
  mPolygonLayer = new QgsVectorLayer( polyFileInfo.filePath(), QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPolygonLayer->isValid() );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayer( mPolygonLayer );
}

void TestQgsProcessingSfcgalAlgs::cleanupTestCase()
{
  QgsProject::instance()->removeMapLayer( mPolygonLayer );
  QgsApplication::exitQgis();
}

void TestQgsProcessingSfcgalAlgs::init()
{
}

void TestQgsProcessingSfcgalAlgs::cleanup()
{
}

void TestQgsProcessingSfcgalAlgs::medialAxis()
{
  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( QStringLiteral( "native:approximatemedialaxis" ) ) );
  QVERIFY( alg != nullptr );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), QVariant::fromValue( mPolygonLayer ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  QgsProcessingFeedback feedback;
  QVariantMap results = alg->run( parameters, *context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *outputLayer = qobject_cast<QgsVectorLayer *>( context->getMapLayer( results.value( QStringLiteral( "OUTPUT" ) ).toString() ) );
  QVERIFY( outputLayer );
  QVERIFY( outputLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 10 );
  QgsProject::instance()->removeMapLayer( outputLayer );
}

QGSTEST_MAIN( TestQgsProcessingSfcgalAlgs )
#include "testqgsprocessingsfcgalalgs.moc"
