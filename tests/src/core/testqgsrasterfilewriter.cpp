/***************************************************************************
     testqgsrasterfilewriter.cpp
     --------------------------------------
    Date                 : 5 Sep 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QTime>
#include <QDesktopServices>

#include "cpl_conv.h"

//qgis includes...
#include "qgsrasterchecker.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasternuller.h"
#include "qgsrasterprojector.h"
#include "qgsapplication.h"
#include "qgsrasterpipe.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterFileWriter class.
 */
class TestQgsRasterFileWriter : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterFileWriter()
      : QgsTest( QStringLiteral( "Raster File Writer Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void writeTest();
    void testCreateOneBandRaster();
    void testCreateMultiBandRaster();
    void testVrtCreation();

  private:
    bool writeTest( const QString &rasterName );

    QString mTestDataDir;
};

//runs before all tests
void TestQgsRasterFileWriter::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  // disable any PAM stuff to make sure stats are consistent
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );
  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}
//runs after all tests
void TestQgsRasterFileWriter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterFileWriter::writeTest()
{
  const QDir dir( mTestDataDir + "/raster" );

  QStringList filters;
  filters << QStringLiteral( "*.tif" );
  const QStringList rasterNames = dir.entryList( filters, QDir::Files );
  bool allOK = true;
  for ( const QString &rasterName : rasterNames )
  {
    const bool ok = writeTest( "raster/" + rasterName );
    if ( !ok )
      allOK = false;
  }

  QVERIFY( allOK );
}

bool TestQgsRasterFileWriter::writeTest( const QString &rasterName )
{
  const QString oldReport = mReport;

  const QString srcFileName = mTestDataDir + '/' + rasterName;
  const QFileInfo rasterFileInfo( srcFileName );

  QTemporaryDir dir;
  const QString copiedSrc = dir.filePath( rasterFileInfo.fileName() );
  if ( !QFile::copy( srcFileName, copiedSrc ) )
  {
    return false;
  }

  std::unique_ptr<QgsRasterLayer> mpRasterLayer( new QgsRasterLayer( copiedSrc, rasterFileInfo.completeBaseName() ) );

  if ( !mpRasterLayer->isValid() )
    return false;

  // Open provider only (avoid layer)?
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();

  QTemporaryDir outputDir;
  const QString tmpName = outputDir.filePath( rasterFileInfo.fileName() );

  mReport += "temporary output file: " + tmpName + "<br>";

  QgsRasterFileWriter fileWriter( tmpName );
  std::unique_ptr<QgsRasterPipe> pipe = std::make_unique<QgsRasterPipe>();
  if ( !pipe->set( provider->clone() ) )
  {
    appendToReport( QStringLiteral( "Write test" ), QStringLiteral( "Cannot set pipe provider" ) );
    return false;
  }

  // Nuller currently is not really used
  QgsRasterNuller *nuller = new QgsRasterNuller();
  for ( int band = 1; band <= provider->bandCount(); band++ )
  {
    //nuller->setNoData( ... );
  }
  if ( !pipe->insert( 1, nuller ) )
  {
    appendToReport( QStringLiteral( "Write test" ), QStringLiteral( "Cannot set pipe nuller" ) );
    return false;
  }

  // Reprojection not really done
  QgsRasterProjector *projector = new QgsRasterProjector;
  projector->setCrs( provider->crs(), provider->crs(), provider->transformContext() );
  if ( !pipe->insert( 2, projector ) )
  {
    appendToReport( QStringLiteral( "Write test" ), QStringLiteral( "Cannot set pipe projector" ) );
    return false;
  }

  const auto res = fileWriter.writeRaster( pipe.get(), provider->xSize(), provider->ySize(), provider->extent(), provider->crs(), provider->transformContext() );

  pipe.reset();

  if ( res != Qgis::RasterFileWriterResult::Success )
  {
    appendToReport( QStringLiteral( "Write test" ), QStringLiteral( "writeRaster() returned error" ) );
    return false;
  }

  QgsRasterChecker checker;
  const bool ok = checker.runTest( QStringLiteral( "gdal" ), tmpName, QStringLiteral( "gdal" ), rasterFileInfo.filePath() );
  mReport += checker.report();

  if ( ok )
  {
    // don't output reports if test is successful
    mReport = oldReport;
  }

  return ok;
}

void TestQgsRasterFileWriter::testCreateOneBandRaster()
{
  // generate unique filename
  QTemporaryDir dir;
  const QString filename = dir.filePath( QStringLiteral( "one_band.tif" ) );

  const QgsRectangle extent( 106.7, -6.2, 106.9, -6.1 );
  int width = 200, height = 100;

  QgsRasterFileWriter writer( filename );
  std::unique_ptr<QgsRasterDataProvider> dp( writer.createOneBandRaster( Qgis::DataType::Byte, width, height, extent, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );
  QVERIFY( dp );
  QCOMPARE( dp->xSize(), width );
  QCOMPARE( dp->ySize(), height );
  QCOMPARE( dp->extent(), extent );
  QCOMPARE( dp->bandCount(), 1 );
  QCOMPARE( dp->dataType( 1 ), Qgis::DataType::Byte );
  QVERIFY( dp->isEditable() );
  dp.reset();

  std::unique_ptr<QgsRasterLayer> rlayer = std::make_unique<QgsRasterLayer>( filename, QStringLiteral( "tmp" ), QStringLiteral( "gdal" ) );
  QVERIFY( rlayer->isValid() );
  QCOMPARE( rlayer->width(), width );
  QCOMPARE( rlayer->height(), height );
  QCOMPARE( rlayer->extent(), extent );
  QCOMPARE( rlayer->bandCount(), 1 );
  QCOMPARE( rlayer->dataProvider()->dataType( 1 ), Qgis::DataType::Byte );
}

void TestQgsRasterFileWriter::testCreateMultiBandRaster()
{
  // generate unique filename
  QTemporaryDir dir;
  const QString filename = dir.filePath( QStringLiteral( "multi_band.tif" ) );

  const QgsRectangle extent( 106.7, -6.2, 106.9, -6.1 );
  int width = 200, height = 100, nBands = 1;

  QgsRasterFileWriter writer( filename );
  std::unique_ptr<QgsRasterDataProvider> dp( writer.createMultiBandRaster( Qgis::DataType::Byte, width, height, extent, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), nBands ) );
  QVERIFY( dp );
  QCOMPARE( dp->xSize(), width );
  QCOMPARE( dp->ySize(), height );
  QCOMPARE( dp->extent(), extent );
  QCOMPARE( dp->bandCount(), 1 );
  QCOMPARE( dp->dataType( 1 ), Qgis::DataType::Byte );
  QVERIFY( dp->isEditable() );
  dp.reset();

  std::unique_ptr<QgsRasterLayer> rlayer = std::make_unique<QgsRasterLayer>( filename, QStringLiteral( "tmp" ), QStringLiteral( "gdal" ) );
  QVERIFY( rlayer->isValid() );
  QCOMPARE( rlayer->width(), width );
  QCOMPARE( rlayer->height(), height );
  QCOMPARE( rlayer->extent(), extent );
  QCOMPARE( rlayer->bandCount(), 1 );
  QCOMPARE( rlayer->dataProvider()->dataType( 1 ), Qgis::DataType::Byte );
  rlayer.reset();

  nBands = 3;
  dp.reset( writer.createMultiBandRaster( Qgis::DataType::Byte, width, height, extent, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), nBands ) );
  QVERIFY( dp );
  QCOMPARE( dp->xSize(), width );
  QCOMPARE( dp->ySize(), height );
  QCOMPARE( dp->extent(), extent );
  QCOMPARE( dp->bandCount(), nBands );
  for ( int i = 1; i <= nBands; i++ )
  {
    QCOMPARE( dp->dataType( i ), Qgis::DataType::Byte );
  }
  QVERIFY( dp->isEditable() );
  dp.reset();

  rlayer = std::make_unique<QgsRasterLayer>( filename, QStringLiteral( "tmp" ), QStringLiteral( "gdal" ) );
  QVERIFY( rlayer->isValid() );
  QCOMPARE( rlayer->width(), width );
  QCOMPARE( rlayer->height(), height );
  QCOMPARE( rlayer->extent(), extent );
  QCOMPARE( rlayer->bandCount(), nBands );
  for ( int i = 1; i <= nBands; i++ )
  {
    QCOMPARE( rlayer->dataProvider()->dataType( i ), Qgis::DataType::Byte );
  }
}

void TestQgsRasterFileWriter::testVrtCreation()
{
  //create a raster layer that will be used in all tests...
  const QString srcFileName = mTestDataDir + QStringLiteral( "ALLINGES_RGF93_CC46_1_1.tif" );
  const QFileInfo rasterFileInfo( srcFileName );
  std::unique_ptr<QgsRasterLayer> srcRasterLayer = std::make_unique<QgsRasterLayer>( rasterFileInfo.absoluteFilePath(), rasterFileInfo.completeBaseName() );

  const QTemporaryDir dir;
  std::unique_ptr<QgsRasterFileWriter> rasterFileWriter = std::make_unique<QgsRasterFileWriter>( dir.path() + '/' + rasterFileInfo.completeBaseName() );

  //2. Definition of the pyramid levels
  QList<int> levelList;
  levelList << 2 << 4 << 8 << 16 << 32 << 64 << 128;
  rasterFileWriter->setPyramidsList( levelList );
  //3. Pyramid format
  rasterFileWriter->setPyramidsFormat( Qgis::RasterPyramidFormat::GeoTiff );
  //4. Resampling method
  rasterFileWriter->setPyramidsResampling( QStringLiteral( "NEAREST" ) );
  //5. Tiled mode => true for vrt creation
  rasterFileWriter->setTiledMode( true );
  //6. Tile size
  rasterFileWriter->setMaxTileWidth( 500 );
  rasterFileWriter->setMaxTileHeight( 500 );
  //7. Coordinate Reference System
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( "EPSG:3946" );
  //8. Prepare raster pipe
  QgsRasterPipe pipe;
  pipe.set( srcRasterLayer->dataProvider()->clone() );
  // Let's do it !
  const Qgis::RasterFileWriterResult res = rasterFileWriter->writeRaster( &pipe, srcRasterLayer->width(), srcRasterLayer->height(), srcRasterLayer->extent(), crs, srcRasterLayer->transformContext() );
  QCOMPARE( res, Qgis::RasterFileWriterResult::Success );

  // Now let's compare the georef of the original raster with the georef of the generated vrt file
  std::unique_ptr<QgsRasterLayer> vrtRasterLayer = std::make_unique<QgsRasterLayer>( dir.path() + '/' + rasterFileInfo.completeBaseName() + '/' + rasterFileInfo.completeBaseName() + QStringLiteral( ".vrt" ), rasterFileInfo.completeBaseName() );

  const double xminVrt = vrtRasterLayer->extent().xMinimum();
  const double yminVrt = vrtRasterLayer->extent().yMaximum();
  const double xminOriginal = srcRasterLayer->extent().xMinimum();
  const double yminOriginal = srcRasterLayer->extent().yMaximum();

  // Let's check if the georef of the original raster with the georef of the generated vrt file
  QGSCOMPARENEAR( xminVrt, xminOriginal, srcRasterLayer->rasterUnitsPerPixelX() / 4 );
  QGSCOMPARENEAR( yminVrt, yminOriginal, srcRasterLayer->rasterUnitsPerPixelY() / 4 );
}

QGSTEST_MAIN( TestQgsRasterFileWriter )
#include "testqgsrasterfilewriter.moc"
