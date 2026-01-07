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

#include <limits>

#include "qgspdalalgorithmbase.h"
#include "qgspdalalgorithms.h"
#include "qgspointcloudlayer.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingregistry.h"
#include "qgstest.h"

#include <QThread>

class TestQgsProcessingPdalAlgs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessingPdalAlgs()
      : QgsTest( u"Processing PDAL Algorithms Test"_s )
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

    void useIndexCopcFile();

  private:
    void updateFileListArg( QStringList &args, const QString &fileName );

    QString mPointCloudLayerPath;
};

void TestQgsProcessingPdalAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
    args[i] = u"--input-file-list=%1"_s.arg( fileName );
  }
}

void TestQgsProcessingPdalAlgs::info()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:info"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );

  QgsProcessingFeedback feedback;

  const QString outputHtml = QDir::tempPath() + "/report.html";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputHtml );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"info"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) );
}

void TestQgsProcessingPdalAlgs::convertFormat()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:convertformat"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/converted.las";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) );

  bool ok;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputPointCloud ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--threads=2"_s );

  // run the alg
  ok = false;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputPointCloud ) );

  // version with run and output to COPC
  const QString outputCopc = QDir::tempPath() + "/converted.copc.las";

  context->setMaximumThreads( 0 );

  parameters.clear();
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputCopc );

  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputCopc ) );

  ok = false;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputPointCloud ) );
}

void TestQgsProcessingPdalAlgs::reproject()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:reproject"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/reprojected.las";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"CRS"_s, u"EPSG:4326"_s );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--transform-crs=%1"_s.arg( "EPSG:4326"_L1 ) );

  // run the alg
  bool ok;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputPointCloud ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--transform-crs=%1"_s.arg( "EPSG:4326"_L1 ) << u"--threads=2"_s );

  // version with run and output to COPC
  QString outputCopcPointCloud = QDir::tempPath() + "/reprojected.copc.laz";

  context->setMaximumThreads( 0 );

  parameters.clear();
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"CRS"_s, u"EPSG:4326"_s );
  parameters.insert( u"OUTPUT"_s, outputCopcPointCloud );

  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputCopcPointCloud ) << u"--transform-crs=%1"_s.arg( "EPSG:4326"_L1 ) );

  ok = false;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputCopcPointCloud ) );
}

void TestQgsProcessingPdalAlgs::assignProjection()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:assignprojection"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/reprojected.las";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"CRS"_s, u"EPSG:4326"_s );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--assign-crs=%1"_s.arg( "EPSG:4326"_L1 ) );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--assign-crs=%1"_s.arg( "EPSG:4326"_L1 ) << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::thinByDecimate()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:thinbydecimate"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/thinned.laz";

  // default values
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );
  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=every-nth"_s << u"--step-every-nth=1"_s );

  // set points number
  parameters.insert( u"POINTS_NUMBER"_s, 200 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=every-nth"_s << u"--step-every-nth=200"_s );

  // filter exression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=every-nth"_s << u"--step-every-nth=200"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=every-nth"_s << u"--step-every-nth=200"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=every-nth"_s << u"--step-every-nth=200"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::thinByRadius()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:thinbyradius"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/thinned.laz";

  // default values
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );
  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=sample"_s << u"--step-sample=1"_s );

  // set sampling radius
  parameters.insert( u"SAMPLING_RADIUS"_s, 2.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=sample"_s << u"--step-sample=2.5"_s );

  // filter exression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=sample"_s << u"--step-sample=2.5"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=sample"_s << u"--step-sample=2.5"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"thin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--mode=sample"_s << u"--step-sample=2.5"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::boundary()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:boundary"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputGpkg = QDir::tempPath() + "/boundary.gpkg";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputGpkg );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"boundary"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputGpkg ) );

  // threshold requires resolution parameter
  parameters.insert( u"THRESHOLD"_s, 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  parameters.insert( u"RESOLUTION"_s, 3000 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"boundary"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputGpkg ) << u"--resolution=%1"_s.arg( 3000 ) << u"--threshold=%1"_s.arg( 10 ) );

  // with filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"boundary"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputGpkg ) << u"--resolution=%1"_s.arg( 3000 ) << u"--threshold=%1"_s.arg( 10 ) << u"--filter=Intensity > 50"_s );

  // with filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"boundary"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputGpkg ) << u"--resolution=%1"_s.arg( 3000 ) << u"--threshold=%1"_s.arg( 10 ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );


  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"boundary"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputGpkg ) << u"--resolution=%1"_s.arg( 3000 ) << u"--threshold=%1"_s.arg( 10 ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::density()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:density"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/density.tif";

  QVariantMap parameters;
  // defaults
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=1"_s << u"--tile-size=1000"_s );

  // change resolution
  parameters.insert( u"RESOLUTION"_s, 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=1000"_s );

  // set tile size
  parameters.insert( u"TILE_SIZE"_s, 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s );

  // set X tile origin
  parameters.insert( u"ORIGIN_X"_s, 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( u"ORIGIN_X"_s );
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  parameters.insert( u"ORIGIN_X"_s, 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s );

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"density"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::exportRasterTin()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:exportrastertin"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/raster.tif";

  QVariantMap parameters;
  // defaults
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=1"_s << u"--tile-size=1000"_s );

  // change resolution
  parameters.insert( u"RESOLUTION"_s, 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=1000"_s );

  // set tile size
  parameters.insert( u"TILE_SIZE"_s, 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s );

  // set X tile origin
  parameters.insert( u"ORIGIN_X"_s, 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( u"ORIGIN_X"_s );
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  parameters.insert( u"ORIGIN_X"_s, 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s );

#ifdef HAVE_PDAL_QGIS
#if PDAL_VERSION_MAJOR_INT > 2 || ( PDAL_VERSION_MAJOR_INT == 2 && PDAL_VERSION_MINOR_INT >= 6 )
  parameters.insert( u"MAX_EDGE_LENGTH"_s, 25 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--max_triangle_edge_length=25"_s );
  parameters.remove( u"MAX_EDGE_LENGTH"_s );
#endif
#endif

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster_tin"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::tile()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:tile"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputDir = QDir::tempPath() + "/tiles";
  const QString tempDir = QDir::tempPath() + "/tmp_tiles";

  QString tempFolder = QgsProcessingUtils::tempFolder();

  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputDir );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"tile"_s << u"--length=1000"_s << u"--output=%1"_s.arg( outputDir ) << u"--temp_dir=%1"_s.arg( tempFolder ) << u"--input-file-list=inputFiles.txt"_s );

  // override temp folder
  context->setTemporaryFolder( tempDir );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"tile"_s << u"--length=1000"_s << u"--output=%1"_s.arg( outputDir ) << u"--temp_dir=%1"_s.arg( tempDir ) << u"--input-file-list=inputFiles.txt"_s );

  // set tile length
  parameters.insert( u"LENGTH"_s, 150 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"tile"_s << u"--length=150"_s << u"--output=%1"_s.arg( outputDir ) << u"--temp_dir=%1"_s.arg( tempDir ) << u"--input-file-list=inputFiles.txt"_s );

  // assign crs
  parameters.insert( u"CRS"_s, u"EPSG:4326"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"tile"_s << u"--length=150"_s << u"--output=%1"_s.arg( outputDir ) << u"--temp_dir=%1"_s.arg( tempDir ) << u"--a_srs=EPSG:4326"_s << u"--input-file-list=inputFiles.txt"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"tile"_s << u"--length=150"_s << u"--output=%1"_s.arg( outputDir ) << u"--temp_dir=%1"_s.arg( tempDir ) << u"--a_srs=EPSG:4326"_s << u"--threads=2"_s << u"--input-file-list=inputFiles.txt"_s );
}

void TestQgsProcessingPdalAlgs::exportRaster()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:exportraster"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/raster.tif";

  // defaults
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=Z"_s << u"--resolution=1"_s << u"--tile-size=1000"_s );

  // specify attribute to use
  parameters.insert( u"ATTRIBUTE"_s, u"ReturnNumber"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=1"_s << u"--tile-size=1000"_s );

  // change resolution
  parameters.insert( u"RESOLUTION"_s, 0.5 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=1000"_s );

  // set tile size
  parameters.insert( u"TILE_SIZE"_s, 100 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=100"_s );

  // set X tile origin
  parameters.insert( u"ORIGIN_X"_s, 1 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set Y tile origin
  parameters.remove( u"ORIGIN_X"_s );
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  QVERIFY_EXCEPTION_THROWN( alg->createArgumentLists( parameters, *context, &feedback ), QgsProcessingException );

  // set both X and Y tile origin
  parameters.insert( u"ORIGIN_Y"_s, 10 );
  parameters.insert( u"ORIGIN_X"_s, 1 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s );

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_raster"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=ReturnNumber"_s << u"--resolution=0.5"_s << u"--tile-size=100"_s << u"--tile-origin-x=1"_s << u"--tile-origin-y=10"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::exportVector()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:exportvector"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/points.gpkg";

  // defaults
  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) );

  // set attribute
  parameters.insert( u"ATTRIBUTE"_s, u"Z"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=Z"_s );

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=Z"_s << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=Z"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--attribute=Z"_s << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::merge()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:merge"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/merged.las";

  // default parameters
  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, QStringList() << mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"merge"_s << u"--output=%1"_s.arg( outputFile ) << u"--input-file-list=inputFiles.txt"_s );

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"merge"_s << u"--output=%1"_s.arg( outputFile ) << u"--filter=Intensity > 50"_s << u"--input-file-list=inputFiles.txt"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"merge"_s << u"--output=%1"_s.arg( outputFile ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--input-file-list=inputFiles.txt"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"merge"_s << u"--output=%1"_s.arg( outputFile ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s << u"--input-file-list=inputFiles.txt"_s );

  // version with run and output to COPC
  QString outputCopcPointCloud = QDir::tempPath() + "/merged.copc.laz";

  parameters.clear();
  parameters.insert( u"LAYERS"_s, QStringList() << mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputCopcPointCloud );

  bool ok;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputCopcPointCloud ) );
}

void TestQgsProcessingPdalAlgs::buildVpc()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:virtualpointcloud"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/test.vpc";

  // default parameters
  QVariantMap parameters;
  parameters.insert( u"LAYERS"_s, QStringList() << mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"build_vpc"_s << u"--output=%1"_s.arg( outputFile ) << u"--input-file-list=inputFiles.txt"_s );

  // calculate exact boundaries
  parameters.insert( u"BOUNDARY"_s, true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"build_vpc"_s << u"--output=%1"_s.arg( outputFile ) << u"--boundary"_s << u"--input-file-list=inputFiles.txt"_s );

  // calculate statistics
  parameters.insert( u"STATISTICS"_s, true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"build_vpc"_s << u"--output=%1"_s.arg( outputFile ) << u"--boundary"_s << u"--stats"_s << u"--input-file-list=inputFiles.txt"_s );

  // build overview
  parameters.insert( u"OVERVIEW"_s, true );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"build_vpc"_s << u"--output=%1"_s.arg( outputFile ) << u"--boundary"_s << u"--stats"_s << u"--overview"_s << u"--input-file-list=inputFiles.txt"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  updateFileListArg( args, u"inputFiles.txt"_s );
  QCOMPARE( args, QStringList() << u"build_vpc"_s << u"--output=%1"_s.arg( outputFile ) << u"--boundary"_s << u"--stats"_s << u"--overview"_s << u"--threads=2"_s << u"--input-file-list=inputFiles.txt"_s );
}

void TestQgsProcessingPdalAlgs::clip()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:clip"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputFile = QDir::tempPath() + "/clipped.las";
  const QString polygonsFile = QString( TEST_DATA_DIR ) + "/polys.shp";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OVERLAY"_s, polygonsFile );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"clip"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--polygon=%1"_s.arg( polygonsFile ) );

  // filter expression
  parameters.insert( u"FILTER_EXPRESSION"_s, u"Intensity > 50"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"clip"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--polygon=%1"_s.arg( polygonsFile ) << u"--filter=Intensity > 50"_s );

  // filter extent
  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"clip"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--polygon=%1"_s.arg( polygonsFile ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"clip"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputFile ) << u"--polygon=%1"_s.arg( polygonsFile ) << u"--filter=Intensity > 50"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );

  // version with run and output to COPC
  QString pointCloudLayerPath = QString( TEST_DATA_DIR ) + "/point_clouds/copc/sunshine-coast.copc.laz";
  QString outputCopcPointCloud = QDir::tempPath() + "/clip.copc.laz";
  const QString polygonSunshineCoast = QString( TEST_DATA_DIR ) + "/sunshine-coast-clip.gpkg";

  context->setMaximumThreads( 0 );

  parameters.clear();
  parameters.insert( u"INPUT"_s, pointCloudLayerPath );
  parameters.insert( u"OVERLAY"_s, polygonSunshineCoast );
  parameters.insert( u"OUTPUT"_s, outputCopcPointCloud );

  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"clip"_s << u"--input=%1"_s.arg( pointCloudLayerPath ) << u"--output=%1"_s.arg( outputCopcPointCloud ) << u"--polygon=%1"_s.arg( polygonSunshineCoast ) );

  bool ok;
  alg->run( parameters, *context, &feedback, &ok );

  QVERIFY( ok );
  QVERIFY( QFileInfo::exists( outputCopcPointCloud ) );
}

void TestQgsProcessingPdalAlgs::filter()
{
  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:filter"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setProject( QgsProject::instance() );
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  const QString outputPointCloud = QDir::tempPath() + "/filtered.laz";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, mPointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputPointCloud );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) );

  parameters.insert( u"FILTER_EXPRESSION"_s, u"Classification = 7 OR Classification = 8"_s );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--filter=Classification == 7 || Classification == 8"_s );

  parameters.insert( u"FILTER_EXTENT"_s, QgsRectangle( 1, 2, 3, 4 ) );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--filter=Classification == 7 || Classification == 8"_s << u"--bounds=([1, 3], [2, 4])"_s );

  // set max threads to 2, a --threads argument should be added
  context->setMaximumThreads( 2 );
  args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"translate"_s << u"--input=%1"_s.arg( mPointCloudLayerPath ) << u"--output=%1"_s.arg( outputPointCloud ) << u"--filter=Classification == 7 || Classification == 8"_s << u"--bounds=([1, 3], [2, 4])"_s << u"--threads=2"_s );
}

void TestQgsProcessingPdalAlgs::useIndexCopcFile()
{
  const QString pointCloudFileName = QString( TEST_DATA_DIR ) + "/point_clouds/las/cloud.las";
  const QFileInfo pointCloudFileInfo( pointCloudFileName );
  const QString pointCloudLayerPath = pointCloudFileInfo.filePath();
  const QString copcIndexFileName = pointCloudFileInfo.absolutePath() + "/" + pointCloudFileInfo.completeBaseName() + ".copc.laz";

  QgsPdalAlgorithmBase *alg = const_cast<QgsPdalAlgorithmBase *>( static_cast<const QgsPdalAlgorithmBase *>( QgsApplication::processingRegistry()->algorithmById( u"pdal:exportvector"_s ) ) );

  auto context = std::make_unique<QgsProcessingContext>();
  context->setMaximumThreads( 0 );

  QgsProcessingFeedback feedback;

  // generate index for use in algorithm
  QgsPointCloudLayer *lyr = new QgsPointCloudLayer( pointCloudLayerPath, "layer", "pdal" );
  Q_UNUSED( lyr );

  //wait for index to be generated
  while ( !QFileInfo::exists( copcIndexFileName ) )
  {
    QThread::sleep( 1 );
  }
  QVERIFY( QFileInfo::exists( copcIndexFileName ) );

  const QString outputFile = QDir::tempPath() + "/points.gpkg";

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, pointCloudLayerPath );
  parameters.insert( u"OUTPUT"_s, outputFile );

  QStringList args = alg->createArgumentLists( parameters, *context, &feedback );
  QCOMPARE( args, QStringList() << u"to_vector"_s << u"--input=%1"_s.arg( copcIndexFileName ) << u"--output=%1"_s.arg( outputFile ) );
  QVERIFY( args.at( 1 ).endsWith( "copc.laz" ) );
}

QGSTEST_MAIN( TestQgsProcessingPdalAlgs )
#include "testqgsprocessingpdalalgs.moc"
