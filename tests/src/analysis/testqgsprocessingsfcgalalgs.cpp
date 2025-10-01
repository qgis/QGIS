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

#include "qgsabstractgeometry.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgstest.h"
#include "qgsnativealgorithms.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingcontext.h"
#include "qgsvectorlayer.h"
#include "qgssfcgalgeometry.h"
#include <qtestcase.h>
#include <memory>

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
    QgsGeometry openWktFile( const QString &wktFile );
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

QgsGeometry TestQgsProcessingSfcgalAlgs::openWktFile( const QString &wktFile )
{
  QString expectedPath = testDataPath( QString( "control_files/expected_sfcgal/expected_%1" ).arg( wktFile ) );
  QFile expectedFile( expectedPath );
  if ( !expectedFile.open( QFile::ReadOnly | QIODevice::Text ) )
  {
    qWarning() << "Unable to open expected data file" << expectedPath;
    return QgsGeometry();
  }

  // remove '\n' from dumped file
  QByteArray expectedBA = expectedFile.readAll();
  QString expectedStr;
  for ( int i = 0; i < expectedBA.length(); i++ )
  {
    if ( expectedBA.at( i ) != '\n' )
      expectedStr += expectedBA.at( i );
  }

  // load geom from corrected wkt
  return QgsGeometry::fromWkt( expectedStr );
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

  // retrieve first feature
  QgsFeatureIterator outputFeatureIterator = outputLayer->dataProvider()->getFeatures();
  QgsFeature outputFeature;
  QVERIFY( outputFeatureIterator.nextFeature( outputFeature ) );

  // check attribute
  QGSCOMPARENEAR( outputFeature.attribute( "length" ).toDouble(), 14.01481, 0.00001 );

  // check geometry
  const QgsGeometry outputGeom( outputFeature.geometry() );
  QVERIFY( outputGeom.wkbType() == Qgis::WkbType::MultiLineString );
  const QgsGeometry expectedGeom = openWktFile( "processing_medial_axis.wkt" );
  QVERIFY( expectedGeom.wkbType() == Qgis::WkbType::MultiLineString );

  QCOMPARE( outputGeom.asWkt( 4 ), expectedGeom.asWkt( 4 ) );

  QgsProject::instance()->removeMapLayer( outputLayer );
}

QGSTEST_MAIN( TestQgsProcessingSfcgalAlgs )
#include "testqgsprocessingsfcgalalgs.moc"
