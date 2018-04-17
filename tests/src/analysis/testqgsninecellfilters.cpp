/***************************************************************************
  testninecellfilters.cpp
  --------------------------------------
  Date                 : April 2018
  Copyright            : (C) 2018 by Alessandro Pasotti
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
#include "qgsalignraster.h"
#include "qgsaspectfilter.h"
#include "qgsslopefilter.h"
#include "qgshillshadefilter.h"
#include "qgsruggednessfilter.h"
#include "qgstotalcurvaturefilter.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

#include <QDir>


class TestNineCellFilters : public QObject
{
    Q_OBJECT

    QString SRC_FILE;
  private slots:

    void initTestCase();
    void testSlope();
    void testAspect();
    void testHillshade();
    void testRuggedness();
    void testTotalCurvature();
#ifdef HAVE_OPENCL
    void testSlopeCl();
    void testAspectCl();
#endif

  private:

    template <class T> void _testAlg( const QString &name, bool useOpenCl = false );

    static QString referenceFile( const QString &name )
    {
      return QStringLiteral( "%1/analysis/%2.tif" ).arg( TEST_DATA_DIR, name );
    }

    static QString tempFile( const QString &name )
    {
      return QStringLiteral( "%1/ninecellfilterstest-%2.tif" ).arg( QDir::tempPath(), name );
    }
};



void TestNineCellFilters::initTestCase()
{
  GDALAllRegister();

  SRC_FILE = QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem.tif";
  QgsApplication::init(); // needed for CRS database
}

template <class T>
void TestNineCellFilters::_testAlg( const QString &name, bool useOpenCl )
{
#ifdef HAVE_OPENCL
  QgsSettings().setValue( QStringLiteral( "useOpenCl" ), true, QgsSettings::Section::Core );
  QString tmpFile( tempFile( name + ( useOpenCl ? "_opencl" : "" ) ) );
#else
  Q_UNUSED( useOpenCl );
  QString tmpFile( tempFile( name ) );
#endif
  QString refFile( referenceFile( name ) );
  QgsAlignRaster::RasterInfo in( SRC_FILE );
  QSize inSize( in.rasterSize() );
  QSizeF inCellSize( in.cellSize( ) );
  T ninecellsfilter( SRC_FILE, tmpFile, "GTiff" );
  int res = ninecellsfilter.processRaster();
  QVERIFY( res == 0 );

  // Produced file
  QgsAlignRaster::RasterInfo out( tmpFile );
  QVERIFY( out.isValid() );

  // Reference file
  QgsAlignRaster::RasterInfo ref( refFile );
  QSize refSize( ref.rasterSize() );
  QSizeF refCellSize( ref.cellSize( ) );

  QCOMPARE( out.rasterSize(), inSize );
  QCOMPARE( out.cellSize(), inCellSize );
  QCOMPARE( out.rasterSize(), refSize );
  QCOMPARE( out.cellSize(), refCellSize );

  double refId1( ref.identify( 4081812, 2431750 ) );
  double refId2( ref.identify( 4081312, 2431350 ) );
  QVERIFY( qAbs( out.identify( 4081812, 2431750 ) - refId1 ) < 0.0001f );
  QVERIFY( qAbs( out.identify( 4081312, 2431350 ) - refId2 ) < 0.0001f );

}


void TestNineCellFilters::testSlope()
{
  _testAlg<QgsSlopeFilter>( QStringLiteral( "slope" ) );
}


void TestNineCellFilters::testAspect()
{
  _testAlg<QgsAspectFilter>( QStringLiteral( "aspect" ) );
}

#ifdef HAVE_OPENCL
void TestNineCellFilters::testSlopeCl()
{
  _testAlg<QgsSlopeFilter>( QStringLiteral( "slope" ), true );
}


void TestNineCellFilters::testAspectCl()
{
  _testAlg<QgsAspectFilter>( QStringLiteral( "aspect" ), true );
}
#endif

void TestNineCellFilters::testHillshade()
{
  _testAlg<QgsHillshadeFilter>( QStringLiteral( "hillshade" ) );
}


void TestNineCellFilters::testRuggedness()
{
  _testAlg<QgsRuggednessFilter>( QStringLiteral( "ruggedness" ) );
}


void TestNineCellFilters::testTotalCurvature()
{
  _testAlg<QgsTotalCurvatureFilter>( QStringLiteral( "totalcurvature" ) );
}


QGSTEST_MAIN( TestNineCellFilters )

#include "testqgsninecellfilters.moc"
