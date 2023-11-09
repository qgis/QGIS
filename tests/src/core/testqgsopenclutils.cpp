/***************************************************************************
 testqgsopenclutils.cpp - TestQgsOpenClUtils

 ---------------------
 begin                : 11.4.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
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
#include <QTemporaryFile>
#include <qgsapplication.h>
#include <chrono>

//header for class being tested
#include <qgsopenclutils.h>
#include <qgshillshaderenderer.h>
#include <qgsrasterlayer.h>

class TestQgsOpenClUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void TestEnable();
    void TestDisable();
    void TestAvailable();
    void testMakeRunProgram();
    void testProgramSource();
    void testContext();
    void testDevices();
    void testActiveDeviceVersion();

    // For benchmarking performance testing
    void testHillshadeCPU();
    void testHillshadeGPU();

  private:

    void _testMakeRunProgram();
    void _testMakeHillshade( const int loops );

    std::string source()
    {
      std::string pgm = R"CL(
       __kernel void vectorAdd(__global float *a, __global float *b, __global float *c)
           {
              const int id = get_global_id(0);
              c[id] = a[id] + b[id];
           }
       )CL";
      return pgm;

    }

    QgsRasterLayer *mFloat32RasterLayer = nullptr;
};


void TestQgsOpenClUtils::init()
{
  // Reset to default in case some tests mess it up
  QgsOpenClUtils::setSourcePath( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/opencl_programs" ) ) );
}

void TestQgsOpenClUtils::cleanup()
{
}

void TestQgsOpenClUtils::initTestCase()
{
  // Runs once before any tests are run

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  const QString float32FileName = QStringLiteral( TEST_DATA_DIR ) + '/' + "/raster/band1_float32_noct_epsg4326.tif";
  const QFileInfo float32RasterFileInfo( float32FileName );
  mFloat32RasterLayer = new QgsRasterLayer( float32RasterFileInfo.filePath(),
      float32RasterFileInfo.completeBaseName() );
}


void TestQgsOpenClUtils::cleanupTestCase()
{
  // Runs once after all tests are run
  QgsApplication::exitQgis();
}


void TestQgsOpenClUtils::TestEnable()
{
  QgsOpenClUtils::setEnabled( true );
  QVERIFY( QgsOpenClUtils::enabled() );
}

void TestQgsOpenClUtils::TestDisable()
{
  QgsOpenClUtils::setEnabled( false );
  QVERIFY( !QgsOpenClUtils::enabled() );
}

void TestQgsOpenClUtils::TestAvailable()
{
  QVERIFY( QgsOpenClUtils::available() );
}


void TestQgsOpenClUtils::testMakeRunProgram()
{
  // Run twice to check for valid command queue in the second call
  _testMakeRunProgram();
  _testMakeRunProgram();
}

void TestQgsOpenClUtils::_testMakeRunProgram()
{

  const cl_int err = 0;

  QVERIFY( err == 0 );

  const cl::Context ctx = QgsOpenClUtils::context();
  cl::CommandQueue queue = QgsOpenClUtils::commandQueue();

  std::vector<float> a_vec = {1, 10, 100};
  std::vector<float> b_vec = {1, 10, 100};
  std::vector<float> c_vec = {-1, -1, -1};
  cl::Buffer a_buf( queue, a_vec.begin(), a_vec.end(), true );
  cl::Buffer b_buf( queue, b_vec.begin(), b_vec.end(), true );
  cl::Buffer c_buf( queue, c_vec.begin(), c_vec.end(), false );

  const cl::Program program = QgsOpenClUtils::buildProgram( QString::fromStdString( source() ) );

  auto kernel =
    cl::KernelFunctor <
    cl::Buffer &,
    cl::Buffer &,
    cl::Buffer &
    > ( program, "vectorAdd" );

  kernel( cl::EnqueueArgs(
            queue,
            cl::NDRange( 3 )
          ),
          a_buf,
          b_buf,
          c_buf
        );

  cl::copy( queue, c_buf, c_vec.begin(), c_vec.end() );
  for ( size_t i = 0; i < c_vec.size(); ++i )
  {
    QCOMPARE( c_vec[i], a_vec[i] + b_vec[i] );
  }
}

void TestQgsOpenClUtils::testProgramSource()
{
  QgsOpenClUtils::setSourcePath( QDir::tempPath() );
  QTemporaryFile tmpFile( QDir::tempPath() + "/XXXXXX.cl" );
  tmpFile.open( );
  tmpFile.write( QByteArray::fromStdString( source( ) ) );
  tmpFile.flush();
  const QString baseName = tmpFile.fileName().replace( ".cl", "" ).replace( QDir::tempPath(), "" );
  QVERIFY( ! QgsOpenClUtils::sourceFromBaseName( baseName ).isEmpty() );
}

void TestQgsOpenClUtils::testContext()
{
  QVERIFY( QgsOpenClUtils::context()() != nullptr );
}

void TestQgsOpenClUtils::testDevices()
{
  std::vector<cl::Device> _devices( QgsOpenClUtils::devices( ) );
  QVERIFY( _devices.size() > 0 );
  qDebug() << QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Name, _devices.at( 0 ) );
  qDebug() << QgsOpenClUtils::deviceInfo( QgsOpenClUtils::Info::Type, _devices.at( 0 ) );
}

void TestQgsOpenClUtils::testActiveDeviceVersion()
{
  const QString version = QgsOpenClUtils::activePlatformVersion();
  qDebug() << "OPENCL VERSION" << version;
  QVERIFY( version.length() == 3 );
}

void TestQgsOpenClUtils::_testMakeHillshade( const int loops )
{
  for ( int i = 0 ; i < loops;  i++ )
  {
    QgsHillshadeRenderer renderer( mFloat32RasterLayer->dataProvider(), 1, 35.0, 5000.0 );
    // Note: CPU time grows linearly with raster dimensions while GPU time is roughly constant
    // 200x200 px gives even times on my testing machine
    renderer.block( 0, mFloat32RasterLayer->extent(), 200, 200 );
  }
}

void TestQgsOpenClUtils::testHillshadeGPU()
{
  QgsOpenClUtils::setEnabled( true );
  QBENCHMARK
  {
    _testMakeHillshade( 1 );
  }
}

void TestQgsOpenClUtils::testHillshadeCPU()
{
  QgsOpenClUtils::setEnabled( false );
  QBENCHMARK
  {
    _testMakeHillshade( 1 );
  }
}



QGSTEST_MAIN( TestQgsOpenClUtils )
#include "testqgsopenclutils.moc"
