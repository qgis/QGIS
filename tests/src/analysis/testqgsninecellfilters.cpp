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

// If true regenerate raster reference images
const bool REGENERATE_REFERENCES = false;

class TestNineCellFilters : public QObject
{
    Q_OBJECT

    QString SRC_FILE;
  private slots:

    void initTestCase();
    void cleanupTestCase();
    void init();

    void testHillshade();
    void testSlope();
    void testAspect();
    void testRuggedness();
    void testTotalCurvature();
#ifdef HAVE_OPENCL
    void testHillshadeCl();
    void testSlopeCl();
    void testAspectCl();
    void testRuggednessCl();
#endif

  private:

    void _rasterCompare( QgsAlignRaster::RasterInfo &out, QgsAlignRaster::RasterInfo &ref );

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


void TestNineCellFilters::init()
{
#ifdef HAVE_OPENCL
  // Reset to default in case some tests mess it up
  QgsOpenClUtils::setSourcePath( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/opencl_programs" ) ) );
#endif
}

void TestNineCellFilters::initTestCase()
{
  GDALAllRegister();

  SRC_FILE = QStringLiteral( TEST_DATA_DIR ) + "/analysis/dem.tif";
  QgsApplication::init(); // needed for CRS database
}

void TestNineCellFilters::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

template <class T>
void TestNineCellFilters::_testAlg( const QString &name, bool useOpenCl )
{
#ifdef HAVE_OPENCL
  QgsOpenClUtils::setEnabled( useOpenCl );
  const QString tmpFile( tempFile( name + ( useOpenCl ? "_opencl" : "" ) ) );
#else
  QString tmpFile( tempFile( name ) );
#endif
  const QString refFile( referenceFile( name ) );
  T ninecellFilter( SRC_FILE, tmpFile, "GTiff" );
  const int res = ninecellFilter.processRaster();
  QVERIFY( res == 0 );

  // Produced file
  QgsAlignRaster::RasterInfo out( tmpFile );
  QVERIFY( out.isValid() );

  // Regenerate reference rasters
  if ( ! useOpenCl && REGENERATE_REFERENCES )
  {
    if ( QFile::exists( refFile ) )
    {
      QFile::remove( refFile );
    }
    QVERIFY( QFile::copy( tmpFile, refFile ) );
  }

  // Reference
  QgsAlignRaster::RasterInfo ref( refFile );
  //qDebug() << "Comparing " << tmpFile << refFile;
  _rasterCompare( out, ref );

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

void TestNineCellFilters::testHillshadeCl()
{
  _testAlg<QgsHillshadeFilter>( QStringLiteral( "hillshade" ), true );
}

void TestNineCellFilters::testRuggednessCl()
{
  _testAlg<QgsRuggednessFilter>( QStringLiteral( "ruggedness" ), true );
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

void TestNineCellFilters::_rasterCompare( QgsAlignRaster::RasterInfo &out,  QgsAlignRaster::RasterInfo &ref )
{
  const QSize refSize( ref.rasterSize() );
  const QSizeF refCellSize( ref.cellSize( ) );
  const QgsAlignRaster::RasterInfo in( SRC_FILE );
  const QSize inSize( in.rasterSize() );
  const QSizeF inCellSize( in.cellSize( ) );
  QCOMPARE( out.rasterSize(), inSize );
  QCOMPARE( out.cellSize(), inCellSize );
  QCOMPARE( out.rasterSize(), refSize );
  QCOMPARE( out.cellSize(), refCellSize );

  // If the values differ less than tolerance they are considered equal
  const double tolerance = 0.0001;

  // Check three points
  std::map<int, int> controlPoints;
  controlPoints[4081812] = 2431750;
  controlPoints[4081312] = 2431350;
  controlPoints[4080263] = 2429558;
  // South West corner
  controlPoints[4081512] = 2431550;
  // North east corner
  controlPoints[4085367] = 2434940;
  // North west corner
  controlPoints[4078263] = 2434936;
  // South east corner
  controlPoints[4085374] = 2428551;

  for ( const auto &cp : controlPoints )
  {
    const int x = cp.first;
    const int y = cp.second;
    const double outVal = out.identify( x, y );
    const double refVal = ref.identify( x, y );
    const double diff( qAbs( outVal - refVal ) );
    //qDebug() << outVal << refVal;
    //qDebug() << "Identify " <<  x << "," << y << " diff " << diff << " check: < " << tolerance;
    QVERIFY( diff <= tolerance );
  }

}

void TestNineCellFilters::testTotalCurvature()
{
  _testAlg<QgsTotalCurvatureFilter>( QStringLiteral( "totalcurvature" ) );
}


QGSTEST_MAIN( TestNineCellFilters )

#include "testqgsninecellfilters.moc"
