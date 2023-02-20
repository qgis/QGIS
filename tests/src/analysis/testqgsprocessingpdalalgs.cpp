/***************************************************************************
                         testqgsprocessingpdalalgs.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
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
#include "qgsprocessingcontext.h"
#include "processing/pdal/qgspdalalgorithms.h"
#include "processing/pdal/qgspdalalgorithmbase.h"


class TestQgsProcessingPdalAlgs: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void info();
    void convertFormat();
    void reproject();

  private:
    QString mPointCloudLayerPath;
};

void TestQgsProcessingPdalAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::processingRegistry()->addProvider( new QgsPdalAlgorithms( QgsApplication::processingRegistry() ) );

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  const QString pointCloudFileName = dataDir + "/point_clouds/las/cloud.las";
  const QFileInfo pointCloudFileInfo( pointCloudFileName );
  mPointCloudLayerPath = pointCloudFileInfo.filePath();
}

void TestQgsProcessingPdalAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingPdalAlgs::init()
{
  // set max threads to 0 by default
  QgsSettings().setValue( "/Processing/Configuration/MAX_THREADS", 0 );
}

void TestQgsProcessingPdalAlgs::info()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast< const QgsPdalAlgorithmBase * >( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:info" ) ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  const QString outputHtml = QDir::tempPath() + "/report.html";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputHtml );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "info" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) );
}

void TestQgsProcessingPdalAlgs::convertFormat()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast< const QgsPdalAlgorithmBase * >( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:convertformat" ) ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/converted.laz";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" )
            << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath )
            << QStringLiteral( "--output=%1" ).arg( outputPointCloud )
          );

  // set max threads to 2, a --threads argument should be added
  QgsSettings().setValue( QStringLiteral( "/Processing/Configuration/MAX_THREADS" ), 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" )
            << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath )
            << QStringLiteral( "--output=%1" ).arg( outputPointCloud )
            << QStringLiteral( "--threads=2" )
          );
}

void TestQgsProcessingPdalAlgs::reproject()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast< const QgsPdalAlgorithmBase * >( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:reproject" ) ) ) );

  std::unique_ptr< QgsProcessingContext > context = std::make_unique< QgsProcessingContext >();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/reprojected.laz";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "CRS" ), QStringLiteral( "EPSG:4326" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" )
            << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath )
            << QStringLiteral( "--output=%1" ).arg( outputPointCloud )
            << QStringLiteral( "--transform-crs=%1" ).arg( QStringLiteral( "EPSG:4326" ) )
          );

  // set max threads to 2, a --threads argument should be added
  QgsSettings().setValue( QStringLiteral( "/Processing/Configuration/MAX_THREADS" ), 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" )
            << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath )
            << QStringLiteral( "--output=%1" ).arg( outputPointCloud )
            << QStringLiteral( "--transform-crs=%1" ).arg( QStringLiteral( "EPSG:4326" ) )
            << QStringLiteral( "--threads=2" )
          );
}

QGSTEST_MAIN( TestQgsProcessingPdalAlgs )
#include "testqgsprocessingpdalalgs.moc"
