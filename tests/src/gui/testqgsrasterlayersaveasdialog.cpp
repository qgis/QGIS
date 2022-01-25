/***************************************************************************
    testqgsrasterlayersaveasdialog.cpp
     --------------------------------------
    Date                 : May 2019
    Copyright            : (C) 2019 Alessandro Pasotti
    Email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include <QTemporaryFile>

#include "qgscoordinatetransformcontext.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterpipe.h"
#include "qgsgui.h"

class TestQgsRasterLayerSaveAsDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsRasterLayerSaveAsDialog() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void outputLayerExists();
    void filenameWhenNoExtension();

  private:

    QString prepareDb();

};

void TestQgsRasterLayerSaveAsDialog::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsRasterLayerSaveAsDialog::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterLayerSaveAsDialog::init()
{
}

void TestQgsRasterLayerSaveAsDialog::cleanup()
{
}

void TestQgsRasterLayerSaveAsDialog::outputLayerExists()
{
  const QString fileName { prepareDb() };

  // Try to add a raster layer to the DB
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString rasterPath { dataDir + "/landsat.tif" };

  QgsRasterLayer rl( rasterPath, QStringLiteral( "my_raster" ) );
  QVERIFY( rl.isValid() );

  QgsRasterLayerSaveAsDialog d( &rl, rl.dataProvider(), rl.extent(), rl.crs(), rl.crs() );
  d.mFormatComboBox->setCurrentIndex( d.mFormatComboBox->findData( QStringLiteral( "GPKG" ) ) );
  QCOMPARE( d.mFormatComboBox->currentData().toString(), QString( "GPKG" ) );
  QVERIFY( ! d.outputLayerExists() );
  d.mFilename->setFilePath( fileName );
  d.mLayerName->setText( QStringLiteral( "my_imported_raster" ) );
  QVERIFY( ! d.outputLayerExists() );

  // Write the raster into the destination file
  const auto pipe { *rl.pipe() };
  const auto rasterUri { QStringLiteral( "GPKG:%1:%2" ).arg( d.outputFileName() ).arg( d.outputLayerName() ) };
  auto fileWriter { QgsRasterFileWriter( d.outputFileName() ) };
  fileWriter.setCreateOptions( d.createOptions() );
  fileWriter.setOutputFormat( d.outputFormat() );
  fileWriter.setBuildPyramidsFlag( d.buildPyramidsFlag() );
  fileWriter.setPyramidsList( d.pyramidsList() );
  fileWriter.setPyramidsResampling( d.pyramidsResamplingMethod() );
  fileWriter.setPyramidsFormat( d.pyramidsFormat() );
  fileWriter.setPyramidsConfigOptions( d.pyramidsConfigOptions() );
  fileWriter.writeRaster( &pipe, 10, 10, rl.extent(), rl.crs(), rl.transformContext() );
  {
    QVERIFY( QgsRasterLayer( rasterUri, QStringLiteral( "my_raster2" ) ).isValid() );
  }
  QVERIFY( d.outputLayerExists() );
  // Now try to save with the same name of the existing vector layer
  d.mLayerName->setText( QStringLiteral( "test_vector_layer" ) );
  QVERIFY( d.outputLayerExists() );
  auto fileWriter2 { QgsRasterFileWriter( d.outputFileName() ) };
  fileWriter2.writeRaster( &pipe, 10, 10, rl.extent(), rl.crs(), rl.transformContext() );
  {
    const auto rasterUri2 { QStringLiteral( "GPKG:%1:%2" ).arg( d.outputFileName() ).arg( d.outputLayerName() ) };
    QVERIFY( ! QgsRasterLayer( rasterUri2, QStringLiteral( "my_raster2" ) ).isValid() );
  }
}

QString TestQgsRasterLayerSaveAsDialog::prepareDb()
{
  // Preparation: make a test gpk DB with a vector layer in it
  QTemporaryFile tmpFile( QDir::tempPath() +  QStringLiteral( "/test_qgsrasterlayersavesdialog_XXXXXX.gpkg" ) );
  tmpFile.setAutoRemove( false );
  tmpFile.open();
  const QString fileName( tmpFile.fileName( ) );
  QgsVectorLayer vl( QStringLiteral( "Point?field=firstfield:string(1024)" ), "test_vector_layer", "memory" );

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = QStringLiteral( "UTF-8" );
  const std::unique_ptr< QgsVectorFileWriter > writer( QgsVectorFileWriter::create( fileName, vl.fields(), QgsWkbTypes::Point, vl.crs(), QgsCoordinateTransformContext(), saveOptions ) );

  QgsFeature f { vl.fields() };
  f.setAttribute( 0, QString( 1024, 'x' ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "point(9 45)" ) ) );
  vl.startEditing();
  vl.addFeature( f );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = QStringLiteral( "GPKG" );
  options.layerName = QStringLiteral( "test_vector_layer" );
  QString errorMessage;
  QgsVectorFileWriter::writeAsVectorFormatV3(
    &vl,
    fileName,
    vl.transformContext(),
    options,
    &errorMessage,
    nullptr,
    nullptr
  );
  const QgsVectorLayer vl2( QStringLiteral( "%1|layername=test_vector_layer" ).arg( fileName ), "test_vector_layer", "ogr" );
  Q_ASSERT( vl2.isValid() );
  return tmpFile.fileName( );
}

void TestQgsRasterLayerSaveAsDialog::filenameWhenNoExtension()
{
  // Try to add a raster layer to the DB
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString rasterPath { dataDir + "/landsat.tif" };

  QgsRasterLayer rl( rasterPath, QStringLiteral( "my_raster" ) );
  QVERIFY( rl.isValid() );

  QgsRasterLayerSaveAsDialog d( &rl, rl.dataProvider(), rl.extent(), rl.crs(), rl.crs() );
  d.mFormatComboBox->setCurrentIndex( d.mFormatComboBox->findData( QStringLiteral( "ENVI" ) ) );
  QCOMPARE( d.mFormatComboBox->currentData().toString(), QString( "ENVI" ) );

  const QString filename = "filename_without_extension";
  d.mFilename->setFilePath( filename );
  QCOMPARE( d.outputFileName(), filename );
}

QGSTEST_MAIN( TestQgsRasterLayerSaveAsDialog )

#include "testqgsrasterlayersaveasdialog.moc"
