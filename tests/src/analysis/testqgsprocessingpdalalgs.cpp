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
#include "qgspdalalgorithms.h"
#include "qgspdalalgorithmbase.h"


class TestQgsProcessingPdalAlgs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingPdalAlgs()
      : QgsTest( QStringLiteral( "Processing PDAL Algorithms Test" ) )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void assignProjection();
    void boundary();
    void buildVpc();
    void clip();
    void convertFormat();
    void density();
    void exportRaster();
    void exportRasterTin();
    void exportVector();
    void filter();
    void info();
    void merge();
    void reproject();
    void thinByDecimate();
    void thinByRadius();
    void tile();

  private:
    void updateFileListArg( QStringList &args, const QString &fileName );

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

  const QString pointCloudFileName = dataDir + "/point_clouds/copc/rgb.copc.laz";
  const QFileInfo pointCloudFileInfo( pointCloudFileName );
  mPointCloudLayerPath = pointCloudFileInfo.filePath();
}

void TestQgsProcessingPdalAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessingPdalAlgs::init()
{
}

void TestQgsProcessingPdalAlgs::updateFileListArg( QStringList &args, const QString &fileName )
{
  int i = 0;
  bool found = false;
  for ( const QString &arg : args )
  {
    if ( arg.contains( fileName, Qt::CaseInsensitive ) )
    {
      found = true;
      break;
    }
    i++;
  }

  if ( found )
  {
    args[i] = QStringLiteral( "--input-file-list=%1" ).arg( fileName );
  }
}

void TestQgsProcessingPdalAlgs::info()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:info" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
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
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:convertformat" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/converted.las";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::reproject()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:reproject" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/reprojected.las";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "CRS" ), QStringLiteral( "EPSG:4326" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--transform-crs=%1" ).arg( QLatin1String( "EPSG:4326" ) ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--transform-crs=%1" ).arg( QLatin1String( "EPSG:4326" ) ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::assignProjection()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:assignprojection" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/reprojected.las";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "CRS" ), QStringLiteral( "EPSG:4326" ) );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--assign-crs=%1" ).arg( QLatin1String( "EPSG:4326" ) ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--assign-crs=%1" ).arg( QLatin1String( "EPSG:4326" ) ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::thinByDecimate()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:thinbydecimate" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/thinned.laz";

  // default values
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );
  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=every-nth" ) << QStringLiteral( "--step-every-nth=1" ) );

  // set points number
  parameters.insert( QStringLiteral( "POINTS_NUMBER" ), 200 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=every-nth" ) << QStringLiteral( "--step-every-nth=200" ) );

  // filter exression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=every-nth" ) << QStringLiteral( "--step-every-nth=200" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=every-nth" ) << QStringLiteral( "--step-every-nth=200" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=every-nth" ) << QStringLiteral( "--step-every-nth=200" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::thinByRadius()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:thinbyradius" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/thinned.laz";

  // default values
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );
  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=sample" ) << QStringLiteral( "--step-sample=1" ) );

  // set sampling radius
  parameters.insert( QStringLiteral( "SAMPLING_RADIUS" ), 2.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=sample" ) << QStringLiteral( "--step-sample=2.5" ) );

  // filter exression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=sample" ) << QStringLiteral( "--step-sample=2.5" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=sample" ) << QStringLiteral( "--step-sample=2.5" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "thin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--mode=sample" ) << QStringLiteral( "--step-sample=2.5" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::boundary()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:boundary" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputGpkg = QDir::tempPath() + "/boundary.gpkg";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputGpkg );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "boundary" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputGpkg ) );

  // threshold requires resolution parameter
  parameters.insert( QStringLiteral( "THRESHOLD" ), 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  parameters.insert( QStringLiteral( "RESOLUTION" ), 3000 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "boundary" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputGpkg ) << QStringLiteral( "--resolution=%1" ).arg( 3000 ) << QStringLiteral( "--threshold=%1" ).arg( 10 ) );

  // with filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "boundary" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputGpkg ) << QStringLiteral( "--resolution=%1" ).arg( 3000 ) << QStringLiteral( "--threshold=%1" ).arg( 10 ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // with filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "boundary" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputGpkg ) << QStringLiteral( "--resolution=%1" ).arg( 3000 ) << QStringLiteral( "--threshold=%1" ).arg( 10 ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );


  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "boundary" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputGpkg ) << QStringLiteral( "--resolution=%1" ).arg( 3000 ) << QStringLiteral( "--threshold=%1" ).arg( 10 ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::density()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:density" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/density.tif";

  QVariantMap parameters;
  // defaults
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=1" ) << QStringLiteral( "--tile-size=1000" ) );

  // change resolution
  parameters.insert( QStringLiteral( "RESOLUTION" ), 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=1000" ) );

  // set tile size
  parameters.insert( QStringLiteral( "TILE_SIZE" ), 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) );

  // set X tile origin
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( QStringLiteral( "ORIGIN_X" ) );
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "density" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::exportRasterTin()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:exportrastertin" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/raster.tif";

  QVariantMap parameters;
  // defaults
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=1" ) << QStringLiteral( "--tile-size=1000" ) );

  // change resolution
  parameters.insert( QStringLiteral( "RESOLUTION" ), 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=1000" ) );

  // set tile size
  parameters.insert( QStringLiteral( "TILE_SIZE" ), 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) );

  // set X tile origin
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( QStringLiteral( "ORIGIN_X" ) );
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster_tin" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::tile()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:tile" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputDir = QDir::tempPath() + "/tiles";
  const QString tempDir = QDir::tempPath() + "/tmp_tiles";

  QString tempFolder = QgsProcessingUtils::tempFolder();

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputDir );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "tile" ) << QStringLiteral( "--length=1000" ) << QStringLiteral( "--output=%1" ).arg( outputDir ) << QStringLiteral( "--temp_dir=%1" ).arg( tempFolder ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // override temp folder
  context->setTemporaryFolder( tempDir );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "tile" ) << QStringLiteral( "--length=1000" ) << QStringLiteral( "--output=%1" ).arg( outputDir ) << QStringLiteral( "--temp_dir=%1" ).arg( tempDir ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // set tile length
  parameters.insert( QStringLiteral( "LENGTH" ), 150 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "tile" ) << QStringLiteral( "--length=150" ) << QStringLiteral( "--output=%1" ).arg( outputDir ) << QStringLiteral( "--temp_dir=%1" ).arg( tempDir ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // assign crs
  parameters.insert( QStringLiteral( "CRS" ), QStringLiteral( "EPSG:4326" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "tile" ) << QStringLiteral( "--length=150" ) << QStringLiteral( "--output=%1" ).arg( outputDir ) << QStringLiteral( "--temp_dir=%1" ).arg( tempDir ) << QStringLiteral( "--a_srs=EPSG:4326" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "tile" ) << QStringLiteral( "--length=150" ) << QStringLiteral( "--output=%1" ).arg( outputDir ) << QStringLiteral( "--temp_dir=%1" ).arg( tempDir ) << QStringLiteral( "--a_srs=EPSG:4326" ) << QStringLiteral( "--threads=2" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );
}

void TestQgsProcessingPdalAlgs::exportRaster()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:exportraster" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/raster.tif";

  // defaults
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=Z" ) << QStringLiteral( "--resolution=1" ) << QStringLiteral( "--tile-size=1000" ) );

  // specify attribute to use
  parameters.insert( QStringLiteral( "ATTRIBUTE" ), QStringLiteral( "ReturnNumber" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=1" ) << QStringLiteral( "--tile-size=1000" ) );

  // change resolution
  parameters.insert( QStringLiteral( "RESOLUTION" ), 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=1000" ) );

  // set tile size
  parameters.insert( QStringLiteral( "TILE_SIZE" ), 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) );

  // set X tile origin
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( QStringLiteral( "ORIGIN_X" ) );
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( QStringLiteral( "ORIGIN_Y" ), 10 );
  parameters.insert( QStringLiteral( "ORIGIN_X" ), 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_raster" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=ReturnNumber" ) << QStringLiteral( "--resolution=0.5" ) << QStringLiteral( "--tile-size=100" ) << QStringLiteral( "--tile-origin-x=1" ) << QStringLiteral( "--tile-origin-y=10" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::exportVector()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:exportvector" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/points.gpkg";

  // defaults
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_vector" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) );

  // set attribute
  parameters.insert( QStringLiteral( "ATTRIBUTE" ), QStringLiteral( "Z" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_vector" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=Z" ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_vector" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=Z" ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_vector" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=Z" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "to_vector" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--attribute=Z" ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::merge()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:merge" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/merged.las";

  // default parameters
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "merge" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "merge" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "merge" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "merge" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );
}

void TestQgsProcessingPdalAlgs::buildVpc()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:virtualpointcloud" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/test.vpc";

  // default parameters
  QVariantMap parameters;
  parameters.insert( QStringLiteral( "LAYERS" ), QStringList() << mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "build_vpc" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // calculate exact boundaries
  parameters.insert( QStringLiteral( "BOUNDARY" ), true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "build_vpc" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--boundary" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // calculate statistics
  parameters.insert( QStringLiteral( "STATISTICS" ), true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "build_vpc" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--boundary" ) << QStringLiteral( "--stats" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // build overview
  parameters.insert( QStringLiteral( "OVERVIEW" ), true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "build_vpc" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--boundary" ) << QStringLiteral( "--stats" ) << QStringLiteral( "--overview" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, QStringLiteral( "inputFiles.txt" ) );
  QCOMPARE( args, QStringList() << QStringLiteral( "build_vpc" ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--boundary" ) << QStringLiteral( "--stats" ) << QStringLiteral( "--overview" ) << QStringLiteral( "--threads=2" ) << QStringLiteral( "--input-file-list=inputFiles.txt" ) );
}

void TestQgsProcessingPdalAlgs::clip()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:clip" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/clipped.las";
  const QString polygonsFile = QString( TEST_DATA_DIR ) + "/polys.shp";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OVERLAY" ), polygonsFile );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "clip" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--polygon=%1" ).arg( polygonsFile ) );

  // filter expression
  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Intensity > 50" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "clip" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--polygon=%1" ).arg( polygonsFile ) << QStringLiteral( "--filter=Intensity > 50" ) );

  // filter extent
  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "clip" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--polygon=%1" ).arg( polygonsFile ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "clip" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputFile ) << QStringLiteral( "--polygon=%1" ).arg( polygonsFile ) << QStringLiteral( "--filter=Intensity > 50" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

void TestQgsProcessingPdalAlgs::filter()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( QStringLiteral( "pdal:filter" ) ) ) );

  std::unique_ptr<QgsProcessingContext> context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/filtered.laz";

  QVariantMap parameters;
  parameters.insert( QStringLiteral( "INPUT" ), mPointCloudLayerPath );
  parameters.insert( QStringLiteral( "OUTPUT" ), outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) );

  parameters.insert( QStringLiteral( "FILTER_EXPRESSION" ), QStringLiteral( "Classification = 7 OR Classification = 8" ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--filter=Classification == 7 || Classification == 8" ) );

  parameters.insert( QStringLiteral( "FILTER_EXTENT" ), QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--filter=Classification == 7 || Classification == 8" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << QStringLiteral( "translate" ) << QStringLiteral( "--input=%1" ).arg( mPointCloudLayerPath ) << QStringLiteral( "--output=%1" ).arg( outputPointCloud ) << QStringLiteral( "--filter=Classification == 7 || Classification == 8" ) << QStringLiteral( "--bounds=([1, 3], [2, 4])" ) << QStringLiteral( "--threads=2" ) );
}

QGSTEST_MAIN( TestQgsProcessingPdalAlgs )
#include "testqgsprocessingpdalalgs.moc"
