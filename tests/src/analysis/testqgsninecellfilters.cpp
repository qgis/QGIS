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

#include "qgsalignraster.h"
#include "qgsapplication.h"
#include "qgsaspectfilter.h"
#include "qgshillshadefilter.h"
#include "qgsrasterlayer.h"
#include "qgsruggednessfilter.h"
#include "qgsslopefilter.h"
#include "qgstest.h"
#include "qgstotalcurvaturefilter.h"

#ifdef HAVE_OPENCL
#include "qgsopenclutils.h"
#endif

#include <QDir>

// If true regenerate raster reference images
const bool REGENERATE_REFERENCES = false;

class TestNineCellFilters : public QgsTest
{
    Q_OBJECT

  public:
    TestNineCellFilters()
      : QgsTest( u"Nine Cell Filter Tests"_s )
    {}

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

    void testCreationOptions();
    void testNoDataValue();

  private:
    void _rasterCompare( QgsAlignRaster::RasterInfo &out, QgsAlignRaster::RasterInfo &ref );

    template<class T> void _testAlg( const QString &name, bool useOpenCl = false );

    static QString referenceFile( const QString &name )
    {
      return u"%1/analysis/%2.tif"_s.arg( TEST_DATA_DIR, name );
    }

    static QString tempFile( const QString &name )
    {
      return u"%1/ninecellfilterstest-%2.tif"_s.arg( QDir::tempPath(), name );
    }
};


void TestNineCellFilters::init()
{
#ifdef HAVE_OPENCL
  // Reset to default in case some tests mess it up
  QgsOpenClUtils::setSourcePath( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( u"resources/opencl_programs"_s ) );
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

template<class T>
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
  QCOMPARE( static_cast<int>( ninecellFilter.processRaster() ), 0 );

  // Produced file
  QgsAlignRaster::RasterInfo out( tmpFile );
  QVERIFY( out.isValid() );

  // Regenerate reference rasters
  if ( !useOpenCl && REGENERATE_REFERENCES )
  {
    if ( QFile::exists( refFile ) )
    {
      QFile::remove( refFile );
    }
    QVERIFY( QFile::copy( tmpFile, refFile ) );
  }

  // Reference
  QgsAlignRaster::RasterInfo ref( refFile );
  _rasterCompare( out, ref );
}


void TestNineCellFilters::testSlope()
{
  _testAlg<QgsSlopeFilter>( u"slope"_s );
}

void TestNineCellFilters::testAspect()
{
  _testAlg<QgsAspectFilter>( u"aspect"_s );
}

#ifdef HAVE_OPENCL
void TestNineCellFilters::testSlopeCl()
{
  _testAlg<QgsSlopeFilter>( u"slope"_s, true );
}

void TestNineCellFilters::testAspectCl()
{
  _testAlg<QgsAspectFilter>( u"aspect"_s, true );
}

void TestNineCellFilters::testHillshadeCl()
{
  _testAlg<QgsHillshadeFilter>( u"hillshade"_s, true );
}

void TestNineCellFilters::testRuggednessCl()
{
  _testAlg<QgsRuggednessFilter>( u"ruggedness"_s, true );
}

#endif

void TestNineCellFilters::testHillshade()
{
  _testAlg<QgsHillshadeFilter>( u"hillshade"_s );
}

void TestNineCellFilters::testRuggedness()
{
  _testAlg<QgsRuggednessFilter>( u"ruggedness"_s );
}

void TestNineCellFilters::_rasterCompare( QgsAlignRaster::RasterInfo &out, QgsAlignRaster::RasterInfo &ref )
{
  const QSize refSize( ref.rasterSize() );
  const QSizeF refCellSize( ref.cellSize() );
  const QgsAlignRaster::RasterInfo in( SRC_FILE );
  const QSize inSize( in.rasterSize() );
  const QSizeF inCellSize( in.cellSize() );
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
    QGSCOMPARENEAR( outVal, refVal, tolerance );
  }
}

void TestNineCellFilters::testTotalCurvature()
{
  _testAlg<QgsTotalCurvatureFilter>( u"totalcurvature"_s );
}

void TestNineCellFilters::testCreationOptions()
{
  QString tmpFile( tempFile( u"createopts"_s ) );

  QString worldFileName = tmpFile.replace( ".tif"_L1, ".tfw"_L1 );
  QFile worldFile( worldFileName );
  QVERIFY( !worldFile.exists() );

  QgsAspectFilter ninecellFilter( SRC_FILE, tmpFile, "GTiff" );
  ninecellFilter.setCreationOptions( QStringList() << "TFW=YES" );
  QCOMPARE( static_cast<int>( ninecellFilter.processRaster() ), 0 );

  QVERIFY( worldFile.exists() );
  worldFile.remove();
}

void TestNineCellFilters::testNoDataValue()
{
  QString tmpFile( tempFile( u"nodata"_s ) );

  QgsAspectFilter ninecellFilter( SRC_FILE, tmpFile, "GTiff" );
  ninecellFilter.setOutputNodataValue( -5555.0 );
  QCOMPARE( static_cast<int>( ninecellFilter.processRaster() ), 0 );

  //open output file and check results
  const std::unique_ptr<QgsRasterLayer> result = std::make_unique<QgsRasterLayer>( tmpFile, u"raster"_s, u"gdal"_s );
  QVERIFY( result->dataProvider()->sourceHasNoDataValue( 1 ) );
  QCOMPARE( result->dataProvider()->sourceNoDataValue( 1 ), -5555.0 );
}

QGSTEST_MAIN( TestNineCellFilters )

#include "testqgsninecellfilters.moc"
