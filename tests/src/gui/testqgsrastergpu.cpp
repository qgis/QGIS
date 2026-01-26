/***************************************************************************
  testqgsrastergpu.cpp - Test GPU raster rendering components
  --------------------------------------
  Date                 : January 2026
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsapplication.h"
#include "qgsrastergpushaders.h"
#include "qgsrastertextureformats.h"
#include "qgsrastergputileuploader.h"
#include "qgsrastertilereader.h"

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <gdal.h>

/**
 * \ingroup UnitTests
 * Unit tests for GPU raster rendering components
 */
class TestQgsRasterGPU : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterGPU() : QgsTest( QStringLiteral( "GPU Raster Rendering Tests" ) ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testTextureFormatMapping();
    void testShaderCreation();
    void testShaderCompilation();
    void testTileUploader();

  private:
    bool createGLContext();
    void destroyGLContext();

    QOpenGLContext *mGLContext = nullptr;
    QOffscreenSurface *mSurface = nullptr;
    QString mTestDataDir;
    QString mCogFile;
};

void TestQgsRasterGPU::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR );
  mCogFile = mTestDataDir + "/raster/test_cog.tif";

  // Try to find any GeoTIFF for testing
  if ( !QFile::exists( mCogFile ) )
  {
    QDir testDir( mTestDataDir + "/raster" );
    const QStringList tiffs = testDir.entryList( QStringList() << "*.tif" << "*.tiff", QDir::Files );
    if ( !tiffs.isEmpty() )
    {
      mCogFile = testDir.absoluteFilePath( tiffs.first() );
      qDebug() << "Using test file:" << mCogFile;
    }
  }
}

void TestQgsRasterGPU::cleanupTestCase()
{
  destroyGLContext();
  QgsApplication::exitQgis();
}

bool TestQgsRasterGPU::createGLContext()
{
  if ( mGLContext )
    return true;

  // Create offscreen surface
  mSurface = new QOffscreenSurface();
  mSurface->setFormat( QSurfaceFormat::defaultFormat() );
  mSurface->create();

  if ( !mSurface->isValid() )
  {
    qWarning() << "Failed to create offscreen surface";
    delete mSurface;
    mSurface = nullptr;
    return false;
  }

  // Create OpenGL context
  mGLContext = new QOpenGLContext();
  mGLContext->setFormat( mSurface->format() );

  if ( !mGLContext->create() )
  {
    qWarning() << "Failed to create OpenGL context";
    delete mGLContext;
    mGLContext = nullptr;
    delete mSurface;
    mSurface = nullptr;
    return false;
  }

  if ( !mGLContext->makeCurrent( mSurface ) )
  {
    qWarning() << "Failed to make OpenGL context current";
    delete mGLContext;
    mGLContext = nullptr;
    delete mSurface;
    mSurface = nullptr;
    return false;
  }

  qDebug() << "OpenGL context created:";
  qDebug() << "  Version:" << QString::fromUtf8( reinterpret_cast<const char *>( glGetString( GL_VERSION ) ) );
  qDebug() << "  Vendor:" << QString::fromUtf8( reinterpret_cast<const char *>( glGetString( GL_VENDOR ) ) );
  qDebug() << "  Renderer:" << QString::fromUtf8( reinterpret_cast<const char *>( glGetString( GL_RENDERER ) ) );

  return true;
}

void TestQgsRasterGPU::destroyGLContext()
{
  if ( mGLContext )
  {
    mGLContext->doneCurrent();
    delete mGLContext;
    mGLContext = nullptr;
  }

  if ( mSurface )
  {
    delete mSurface;
    mSurface = nullptr;
  }
}

void TestQgsRasterGPU::testTextureFormatMapping()
{
  // Test format mapping for different data types

  // Byte, single band
  auto format = QgsRasterTextureFormat::getFormat( Qgis::DataType::Byte, 1 );
  QVERIFY( format.isSupported );
  QCOMPARE( format.channelCount, 1 );
  QCOMPARE( format.bytesPerPixel, 1 );
  QCOMPARE( format.shaderType, QString( "u8" ) );

  // UInt16, single band
  format = QgsRasterTextureFormat::getFormat( Qgis::DataType::UInt16, 1 );
  QVERIFY( format.isSupported );
  QCOMPARE( format.channelCount, 1 );
  QCOMPARE( format.bytesPerPixel, 2 );
  QCOMPARE( format.shaderType, QString( "u16" ) );

  // Float32, single band
  format = QgsRasterTextureFormat::getFormat( Qgis::DataType::Float32, 1 );
  QVERIFY( format.isSupported );
  QCOMPARE( format.channelCount, 1 );
  QCOMPARE( format.bytesPerPixel, 4 );
  QCOMPARE( format.shaderType, QString( "f32" ) );

  // Byte, RGBA
  format = QgsRasterTextureFormat::getFormat( Qgis::DataType::Byte, 4 );
  QVERIFY( format.isSupported );
  QCOMPARE( format.channelCount, 4 );
  QCOMPARE( format.bytesPerPixel, 4 );

  qDebug() << "✓ Texture format mapping tests passed";
}

void TestQgsRasterGPU::testShaderCreation()
{
  // Test shader source generation (doesn't require GL context)

  // Vertex shader should be non-empty
  const QString vertexSource = QgsRasterGPUShaders::vertexShaderSource();
  QVERIFY( !vertexSource.isEmpty() );
  QVERIFY( vertexSource.contains( "gl_Position" ) );
  QVERIFY( vertexSource.contains( "vTexCoord" ) );

  // Fragment shader for byte data
  const QString byteFragSource = QgsRasterGPUShaders::fragmentShaderSource( QgsRasterGPUShaders::ShaderType::Byte );
  QVERIFY( !byteFragSource.isEmpty() );
  QVERIFY( byteFragSource.contains( "texture" ) );
  QVERIFY( byteFragSource.contains( "uTileTexture" ) );

  // Fragment shader for float32 data
  const QString floatFragSource = QgsRasterGPUShaders::fragmentShaderSource( QgsRasterGPUShaders::ShaderType::Float32 );
  QVERIFY( !floatFragSource.isEmpty() );
  QVERIFY( floatFragSource.contains( "sampler2D" ) );

  // Fragment shader for RGBA
  const QString rgbaFragSource = QgsRasterGPUShaders::fragmentShaderSource( QgsRasterGPUShaders::ShaderType::RGBA8 );
  QVERIFY( !rgbaFragSource.isEmpty() );
  QVERIFY( rgbaFragSource.contains( "texture" ) );

  qDebug() << "✓ Shader source generation tests passed";
}

void TestQgsRasterGPU::testShaderCompilation()
{
  // This test requires OpenGL context
  if ( !createGLContext() )
  {
    QSKIP( "OpenGL context not available" );
  }

  // Test shader compilation for different data types
  QgsRasterGPUShaders::ShaderConfig config;

  // Test Byte shader
  config.type = QgsRasterGPUShaders::ShaderType::Byte;
  config.colorRamp = {
    {0.0f, QColor( 0, 0, 0 )},
    {1.0f, QColor( 255, 255, 255 )}
  };
  config.minValue = 0.0f;
  config.maxValue = 255.0f;
  config.opacity = 1.0f;

  QOpenGLShaderProgram *program = QgsRasterGPUShaders::createShaderProgram( config );
  QVERIFY( program != nullptr );
  QVERIFY( program->isLinked() );
  delete program;

  qDebug() << "  ✓ Byte shader compiled successfully";

  // Test Float32 shader
  config.type = QgsRasterGPUShaders::ShaderType::Float32;
  config.minValue = -100.0f;
  config.maxValue = 100.0f;

  program = QgsRasterGPUShaders::createShaderProgram( config );
  QVERIFY( program != nullptr );
  QVERIFY( program->isLinked() );
  delete program;

  qDebug() << "  ✓ Float32 shader compiled successfully";

  // Test RGBA shader (simpler, no color ramp)
  config.type = QgsRasterGPUShaders::ShaderType::RGBA8;

  program = QgsRasterGPUShaders::createShaderProgram( config );
  QVERIFY( program != nullptr );
  QVERIFY( program->isLinked() );
  delete program;

  qDebug() << "  ✓ RGBA8 shader compiled successfully";

  qDebug() << "✓ Shader compilation tests passed";
}

void TestQgsRasterGPU::testTileUploader()
{
  // Test tile uploader functionality
  if ( !QFile::exists( mCogFile ) )
  {
    QSKIP( "No test COG file available" );
  }

  if ( !createGLContext() )
  {
    QSKIP( "OpenGL context not available" );
  }

  // Open dataset
  GDALDatasetH dataset = GDALOpen( mCogFile.toUtf8().constData(), GA_ReadOnly );
  QVERIFY( dataset != nullptr );

  // Create COG reader
  QgsRasterTileReader reader( dataset );
  QVERIFY( reader.isValid() );

  // Create tile uploader
  QgsRasterGPUTileUploader uploader( &reader );

  // Test upload of first tile
  const auto gpuTile = uploader.uploadTile( 0, 0, 0, 1 );
  QVERIFY( gpuTile.isValid );
  QVERIFY( gpuTile.textureId != 0 );
  QVERIFY( gpuTile.width > 0 );
  QVERIFY( gpuTile.height > 0 );

  qDebug() << "  ✓ Tile uploaded successfully:";
  qDebug() << "    Texture ID:" << gpuTile.textureId;
  qDebug() << "    Size:" << gpuTile.width << "x" << gpuTile.height;

  // Test tile caching
  const auto cachedTile = uploader.getTile( 0, 0, 0, 1, 1 );
  QVERIFY( cachedTile.isValid );
  QCOMPARE( cachedTile.textureId, gpuTile.textureId );  // Should be same texture (cached)

  qDebug() << "  ✓ Tile caching works correctly";

  // Test cache statistics
  int cachedCount = 0;
  qint64 memoryBytes = 0;
  uploader.getCacheStats( cachedCount, memoryBytes );
  QVERIFY( cachedCount > 0 );
  QVERIFY( memoryBytes > 0 );

  qDebug() << "  ✓ Cache stats:";
  qDebug() << "    Cached tiles:" << cachedCount;
  qDebug() << "    Memory used:" << ( memoryBytes / 1024.0 ) << "KB";

  // Test tile extent calculation
  const QgsRectangle extent = uploader.tileExtent( 0, 0, 0 );
  QVERIFY( !extent.isEmpty() );
  QVERIFY( extent.width() > 0 );
  QVERIFY( extent.height() > 0 );

  qDebug() << "  ✓ Tile extent calculation works";

  // Clean up
  uploader.clearCache();
  uploader.getCacheStats( cachedCount, memoryBytes );
  QCOMPARE( cachedCount, 0 );
  QCOMPARE( memoryBytes, static_cast<qint64>( 0 ) );

  qDebug() << "  ✓ Cache cleared successfully";

  GDALClose( dataset );

  qDebug() << "✓ Tile uploader tests passed";
}

QGSTEST_MAIN( TestQgsRasterGPU )
#include "testqgsrastergpu.moc"
