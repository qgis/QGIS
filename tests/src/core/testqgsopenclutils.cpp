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

//header for class being tested
#include <qgsopenclutils.h>


class TestQgsOpenClUtils: public QObject
{
    Q_OBJECT
  public:

    //void testRunMakeProgram();

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

  private:

    void _testMakeRunProgram();

    cl::Program buildProgram( const cl::Context &context, const QString &source )
    {
      cl::Program program( context, source.toStdString( ) );
      program.build( "-cl-std=CL1.1" );
      return program;
    }

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
};


void TestQgsOpenClUtils::init()
{
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

  cl_int err = 0;

  QVERIFY( err == 0 );

  cl::Context ctx = QgsOpenClUtils::context();
  cl::Context::setDefault( ctx );
  cl::CommandQueue queue( ctx );

  std::vector<float> a_vec = {1, 10, 100};
  std::vector<float> b_vec = {1, 10, 100};
  std::vector<float> c_vec = {-1, -1, -1};
  cl::Buffer a_buf( a_vec.begin(), a_vec.end(), true );
  cl::Buffer b_buf( b_vec.begin(), b_vec.end(), true );
  cl::Buffer c_buf( c_vec.begin(), c_vec.end(), false );

  cl::Program program = QgsOpenClUtils::buildProgram( ctx, QString::fromStdString( source() ) );

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

  cl::copy( c_buf, c_vec.begin(), c_vec.end() );
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
  QString baseName = tmpFile.fileName().replace( ".cl", "" ).replace( QDir::tempPath(), "" );
  QVERIFY( ! QgsOpenClUtils::sourceFromBaseName( baseName ).isEmpty() );
}

void TestQgsOpenClUtils::testContext()
{
  QVERIFY( QgsOpenClUtils::context()() != nullptr );
}


QGSTEST_MAIN( TestQgsOpenClUtils )
#include "testqgsopenclutils.moc"
