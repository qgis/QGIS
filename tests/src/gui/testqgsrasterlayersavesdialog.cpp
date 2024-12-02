/***************************************************************************
    testqgsrasterlayersavesdialog.cpp
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

#include "qgsrasterlayersaveasdialog.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterfilewriter.h"

#include "qgsgui.h"

class TestQgsRasterLayerSaveAsDialog : public QObject
{
    Q_OBJECT
  public:
    TestQgsRasterLayerSaveAsDialog() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void outputLayerExists();

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
  QString fileName { prepareDb() };

  // Try to add a raster layer to the DB
  QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString rasterPath { dataDir + "/landsat.tif" };

  QgsRasterLayer rl( rasterPath, QStringLiteral( "my_raster" ) );
  QVERIFY( rl.isValid() );

  QgsRasterLayerSaveAsDialog d( &rl, rl.dataProvider(), rl.extent(), rl.crs(), rl.crs() );
  d.mFormatComboBox->setCurrentIndex( d.mFormatComboBox->findData( QStringLiteral( "GPKG" ) ) );
  QCOMPARE( d.mFormatComboBox->currentData().toString(), QString( "GPKG" ) );
  QVERIFY( !d.outputLayerExists() );
  d.mFilename->setFilePath( fileName );
  d.mLayerName->setText( QStringLiteral( "my_imported_raster" ) );
  QVERIFY( !d.outputLayerExists() );

  // Write the raster into the destination file
  auto pipe { *rl.pipe() };
  auto rasterUri { QStringLiteral( "GPKG:%1:%2" ).arg( d.outputFileName() ).arg( d.outputLayerName() ) };
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
}

QString TestQgsRasterLayerSaveAsDialog::prepareDb()
{
  // Preparation: make a test gpk DB with a vector layer in it
  QTemporaryFile tmpFile( QDir::tempPath() + QStringLiteral( "/test_qgsrasterlayersavesdialog_XXXXXX.gpkg" ) );
  tmpFile.setAutoRemove( false );
  tmpFile.open();
  QString fileName( tmpFile.fileName() );
  QgsVectorLayer vl( QStringLiteral( "Point?field=firstfield:string(1024)" ), "test_layer", "memory" );
  QgsVectorFileWriter w( fileName, QStringLiteral( "UTF-8" ), vl.fields(), QgsWkbTypes::Point, vl.crs() );
  QgsFeature f { vl.fields() };
  f.setAttribute( 0, QString( 1024, 'x' ) );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "point(9 45)" ) ) );
  vl.startEditing();
  vl.addFeature( f );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = QStringLiteral( "GPKG" );
  options.layerName = QStringLiteral( "test_layer" );
  QString errorMessage;
  QgsVectorFileWriter::writeAsVectorFormat(
    &vl,
    fileName,
    options,
    &errorMessage
  );
  QgsVectorLayer vl2( QStringLiteral( "%1|layername=test_layer" ).arg( fileName ), "src_test", "ogr" );
  Q_ASSERT( vl2.isValid() );
  return tmpFile.fileName();
}

QGSTEST_MAIN( TestQgsRasterLayerSaveAsDialog )

#include "testqgsrasterlayersavesdialog.moc"
