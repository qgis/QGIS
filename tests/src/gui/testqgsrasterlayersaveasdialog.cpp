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


#include "qgscoordinatetransformcontext.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterpipe.h"
#include "qgstest.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QTemporaryFile>

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

  QgsRasterLayer rl( rasterPath, u"my_raster"_s );
  QVERIFY( rl.isValid() );

  QgsRasterLayerSaveAsDialog d( &rl, rl.dataProvider(), rl.extent(), rl.crs(), rl.crs() );
  d.mFormatComboBox->setCurrentIndex( d.mFormatComboBox->findData( u"GPKG"_s ) );
  QCOMPARE( d.mFormatComboBox->currentData().toString(), QString( "GPKG" ) );
  QVERIFY( !d.outputLayerExists() );
  d.mFilename->setFilePath( fileName );
  d.mLayerName->setText( u"my_imported_raster"_s );
  QVERIFY( !d.outputLayerExists() );

  // Write the raster into the destination file
  const auto pipe { *rl.pipe() };
  const auto rasterUri { u"GPKG:%1:%2"_s.arg( d.outputFileName() ).arg( d.outputLayerName() ) };
  auto fileWriter { QgsRasterFileWriter( d.outputFileName() ) };
  fileWriter.setCreationOptions( d.creationOptions() );
  fileWriter.setOutputFormat( d.outputFormat() );
  fileWriter.setBuildPyramidsFlag( d.buildPyramidsFlag() );
  fileWriter.setPyramidsList( d.pyramidsList() );
  fileWriter.setPyramidsResampling( d.pyramidsResamplingMethod() );
  fileWriter.setPyramidsFormat( d.pyramidsFormat() );
  fileWriter.setPyramidsConfigOptions( d.pyramidsConfigOptions() );
  fileWriter.writeRaster( &pipe, 10, 10, rl.extent(), rl.crs(), rl.transformContext() );
  {
    QVERIFY( QgsRasterLayer( rasterUri, u"my_raster2"_s ).isValid() );
  }
  QVERIFY( d.outputLayerExists() );
  // Now try to save with the same name of the existing vector layer
  d.mLayerName->setText( u"test_vector_layer"_s );
  QVERIFY( d.outputLayerExists() );
  auto fileWriter2 { QgsRasterFileWriter( d.outputFileName() ) };
  fileWriter2.writeRaster( &pipe, 10, 10, rl.extent(), rl.crs(), rl.transformContext() );
  {
    const auto rasterUri2 { u"GPKG:%1:%2"_s.arg( d.outputFileName() ).arg( d.outputLayerName() ) };
    QVERIFY( !QgsRasterLayer( rasterUri2, u"my_raster2"_s ).isValid() );
  }
}

QString TestQgsRasterLayerSaveAsDialog::prepareDb()
{
  // Preparation: make a test gpk DB with a vector layer in it
  QTemporaryFile tmpFile( QDir::tempPath() + u"/test_qgsrasterlayersavesdialog_XXXXXX.gpkg"_s );
  tmpFile.setAutoRemove( false );
  if ( !tmpFile.open() )
  {
    Q_ASSERT( false );
    return QString();
  }
  const QString fileName( tmpFile.fileName() );
  QgsVectorLayer vl( u"Point?field=firstfield:string(1024)"_s, "test_vector_layer", "memory" );

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = u"UTF-8"_s;
  const std::unique_ptr<QgsVectorFileWriter> writer( QgsVectorFileWriter::create( fileName, vl.fields(), Qgis::WkbType::Point, vl.crs(), QgsCoordinateTransformContext(), saveOptions ) );

  QgsFeature f { vl.fields() };
  f.setAttribute( 0, QString( 1024, 'x' ) );
  f.setGeometry( QgsGeometry::fromWkt( u"point(9 45)"_s ) );
  vl.startEditing();
  vl.addFeature( f );
  QgsVectorFileWriter::SaveVectorOptions options;
  options.driverName = u"GPKG"_s;
  options.layerName = u"test_vector_layer"_s;
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
  const QgsVectorLayer vl2( u"%1|layername=test_vector_layer"_s.arg( fileName ), "test_vector_layer", "ogr" );
  Q_ASSERT( vl2.isValid() );
  return tmpFile.fileName();
}

void TestQgsRasterLayerSaveAsDialog::filenameWhenNoExtension()
{
  // Try to add a raster layer to the DB
  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString rasterPath { dataDir + "/landsat.tif" };

  QgsRasterLayer rl( rasterPath, u"my_raster"_s );
  QVERIFY( rl.isValid() );

  QgsRasterLayerSaveAsDialog d( &rl, rl.dataProvider(), rl.extent(), rl.crs(), rl.crs() );
  d.mFormatComboBox->setCurrentIndex( d.mFormatComboBox->findData( u"ENVI"_s ) );
  QCOMPARE( d.mFormatComboBox->currentData().toString(), QString( "ENVI" ) );

  const QString filename = "filename_without_extension";
  d.mFilename->setFilePath( filename );
  QCOMPARE( d.outputFileName(), filename );
}

QGSTEST_MAIN( TestQgsRasterLayerSaveAsDialog )

#include "testqgsrasterlayersaveasdialog.moc"
