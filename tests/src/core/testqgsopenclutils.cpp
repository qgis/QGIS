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
//header for class being tested
#include <qgsopenclutils.h>


class TestQgsOpenClUtils: public QObject
{
    Q_OBJECT
  public:

    //void testRunMakeProgram();

  private slots:

    void TestEnable();
    void TestDisable();
    void TestAvailable();
    void testMakeRunProgram();
    void testContext();

  private:

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

  cl_int err = 0;

  QVERIFY( err == 0 );

  cl::Context ctx = QgsOpenClUtils::context();
  cl::Context::setDefault( ctx );

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

void TestQgsOpenClUtils::testContext()
{
  QVERIFY( QgsOpenClUtils::context()() != nullptr );
}


QGSTEST_MAIN( TestQgsOpenClUtils )
#include "testqgsopenclutils.moc"
