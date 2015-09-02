/***************************************************************************
     testqgsgrassprovider.cpp
     --------------------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include <QApplication>
#include <QDir>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsgrass.h>
#include <qgsgrassimport.h>
#include <qgsproviderregistry.h>
#include <qgsrasterbandstats.h>
#include <qgsrasterlayer.h>
#include <qgsvectordataprovider.h>

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
}

#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsGrassProvider: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void fatalError();
    void locations();
    void mapsets();
    void maps();
    void vectorLayers();
    void region();
    void info();
    void rasterImport();
    void vectorImport();
  private:
    void reportRow( QString message );
    void reportHeader( QString message );
    // verify result and report result
    bool verify( bool ok );
    // compare expected and got string and set ok to false if not equal
    bool compare( QString expected, QString got, bool& ok );
    // lists are considered equal if contains the same values regardless order
    // set ok to false if not equal
    bool compare( QStringList expected, QStringList got, bool& ok );
    // compare with tolerance
    bool compare( double expected, double got, bool& ok );
    bool createTmpLocation( QString& tmpGisdbase, QString& tmpLocation, QString& tmpMapset );
    QString mGisdbase;
    QString mLocation;
    QString mReport;
    QString mBuildMapset;
};

#define GVERIFY(x) QVERIFY( verify(x) )

void TestQgsGrassProvider::reportRow( QString message )
{
  mReport += message + "<br>\n";
}
void TestQgsGrassProvider::reportHeader( QString message )
{
  mReport += "<h2>" + message + "</h2>\n";
}

//runs before all tests
void TestQgsGrassProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  // QgsApplication::initQgis() calls QgsProviderRegistry::instance() which registers providers.
  // Because providers are linked in build directory with rpath, it would also try to load GRASS providers
  // in version different form which we are testing here and it would also load GRASS libs in different version
  // and result in segfault when __do_global_dtors_aux() is called.
  // => we must set QGIS_PROVIDER_FILE before QgsApplication::initQgis() to avoid loading GRASS provider in different version
  QgsGrass::putEnv( "QGIS_PROVIDER_FILE", "gdal|ogr" );
  QgsApplication::initQgis();
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />\n" );
  mReport += QString( "<h1>GRASS %1 provider tests</h1>\n" ).arg( GRASS_BUILD_VERSION );
  mReport += "<p>" + mySettings + "</p>\n";

#ifndef Q_OS_WIN
  reportRow( "LD_LIBRARY_PATH: " + QString( getenv( "LD_LIBRARY_PATH" ) ) );
#else
  reportRow( "PATH: " + QString( getenv( "PATH" ) ) );
#endif

  QgsGrass::init();

  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mGisdbase = QString( TEST_DATA_DIR ) + "/grass";
  mLocation = "wgs84";
  mBuildMapset = QString( "test%1" ).arg( GRASS_BUILD_VERSION );
  reportRow( "mGisdbase: " + mGisdbase );
  reportRow( "mLocation: " + mLocation );
  reportRow( "mBuildMapset: " + mBuildMapset );
  qDebug() << "mGisdbase = " << mGisdbase << " mLocation = " << mLocation;
}

//runs after all tests
void TestQgsGrassProvider::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  //QgsApplication::exitQgis();
}

bool TestQgsGrassProvider::verify( bool ok )
{
  reportRow( "" );
  reportRow( QString( "Test result: " ) + ( ok ? "ok" : "error" ) );
  return ok;
}

bool TestQgsGrassProvider::compare( QString expected, QString got, bool &ok )
{
  if ( expected != got )
  {
    ok = false;
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( QStringList expected, QStringList got, bool &ok )
{
  QStringList e = expected;
  QStringList g = got;
  e.sort();
  g.sort();
  if ( e != g )
  {
    ok = false;
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( double expected, double got, bool& ok )
{
  if ( qAbs( got - expected ) > TINY_VALUE )
  {
    ok = false;
    return false;
  }
  return true;
}

// G_fatal_error() handling
void TestQgsGrassProvider::fatalError()
{
  reportHeader( "TestQgsGrassProvider::fatalError" );
  bool ok = true;
  QString errorMessage = "test fatal error";
  G_TRY
  {
    G_fatal_error( "%s", errorMessage.toAscii().data() );
    ok = false; // should not be reached
    reportRow( "G_fatal_error() did not throw exception" );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    reportRow( QString( "Exception thrown and caught correctly" ) );
    reportRow( "expected error message: " + errorMessage );
    reportRow( "got error message: " + QString( e.what() ) );
    compare( errorMessage, e.what(), ok );
  }
  compare( errorMessage, QgsGrass::errorMessage(), ok );
  GVERIFY( ok );
}

void TestQgsGrassProvider::locations()
{
  reportHeader( "TestQgsGrassProvider::locations" );
  bool ok = true;
  QStringList expectedLocations;
  expectedLocations << "wgs84";
  QStringList locations = QgsGrass::locations( mGisdbase );
  reportRow( "expectedLocations: " + expectedLocations.join( ", " ) );
  reportRow( "locations: " + locations.join( ", " ) );
  compare( expectedLocations, locations, ok );
  GVERIFY( ok );
}

void TestQgsGrassProvider::mapsets()
{
  reportHeader( "TestQgsGrassProvider::mapsets" );
  bool ok = true;
  QStringList expectedMapsets;
  expectedMapsets << "PERMANENT" << "test" << "test6" << "test7";
  QStringList mapsets = QgsGrass::mapsets( mGisdbase,  mLocation );
  reportRow( "expectedMapsets: " + expectedMapsets.join( ", " ) );
  reportRow( "mapsets: " + mapsets.join( ", " ) );
  compare( expectedMapsets, mapsets, ok );
  QgsGrass::setLocation( mGisdbase,  mLocation ); // for G_is_mapset_in_search_path
  // Disabled because adding of all mapsets to search path was disabled in setLocation()
#if 0
  foreach ( QString expectedMapset, expectedMapsets )
  {
    if ( G_is_mapset_in_search_path( expectedMapset.toAscii().data() ) != 1 )
    {
      reportRow( "mapset " + expectedMapset + " not in search path" );
      ok = false;
    }
  }
#endif

  // open/close mapset try twice to be sure that lock was not left etc.
  for ( int i = 1; i < 3; i++ )
  {
    reportRow( "" );
    reportRow( "Open/close mapset " + mBuildMapset + " for the " + QString::number( i ) + ". time" );
    QString error = QgsGrass::openMapset( mGisdbase, mLocation, mBuildMapset );
    if ( !error.isEmpty() )
    {
      reportRow( "QgsGrass::openMapset() failed: " + error );
      ok = false;
    }
    else
    {
      reportRow( "mapset successfully opened" );
      if ( !QgsGrass::activeMode() )
      {
        reportRow( "QgsGrass::activeMode() returns false after openMapset()" );
        ok = false;
      }

      error = QgsGrass::closeMapset();

      if ( !error.isEmpty() )
      {
        reportRow( "QgsGrass::close() failed: " + error );
        ok = false;
      }
      else
      {
        reportRow( "mapset successfully closed" );
      }

      if ( QgsGrass::activeMode() )
      {
        reportRow( "QgsGrass::activeMode() returns true after closeMapset()" );
        ok = false;
      }
    }
  }
  GVERIFY( ok );
}

void TestQgsGrassProvider::maps()
{
  reportHeader( "TestQgsGrassProvider::maps" );
  bool ok = true;
  QStringList expectedVectors;
  expectedVectors << "test";
  QStringList vectors = QgsGrass::vectors( mGisdbase,  mLocation, mBuildMapset );
  reportRow( "expectedVectors: " + expectedVectors.join( ", " ) );
  reportRow( "vectors: " + vectors.join( ", " ) );
  compare( expectedVectors, vectors, ok );

  reportRow( "" );
  QStringList expectedRasters;
  expectedRasters << "cell" << "dcell" << "fcell";
  QStringList rasters = QgsGrass::rasters( mGisdbase,  mLocation, "test" );
  reportRow( "expectedRasters: " + expectedRasters.join( ", " ) );
  reportRow( "rasters: " + rasters.join( ", " ) );
  compare( expectedRasters, rasters, ok );
  GVERIFY( ok );
}

void TestQgsGrassProvider::vectorLayers()
{
  reportHeader( "TestQgsGrassProvider::vectorLayers" );
  QString mapset = mBuildMapset;
  QString mapName = "test";
  QStringList expectedLayers;
  expectedLayers << "1_point" << "2_line" << "3_polygon";

  reportRow( "mapset: " + mapset );
  reportRow( "mapName: " + mapName );
  reportRow( "expectedLayers: " + expectedLayers.join( ", " ) );

  bool ok = true;
  G_TRY
  {
    QStringList layers = QgsGrass::vectorLayers( mGisdbase, mLocation, mapset, mapName );
    reportRow( "layers: " + layers.join( ", " ) );
    compare( expectedLayers, layers, ok );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    ok = false;
    reportRow( QString( "ERROR: %1" ).arg( e.what() ) );
  }
  GVERIFY( ok );
}

void TestQgsGrassProvider::region()
{
  reportHeader( "TestQgsGrassProvider::region" );
  struct Cell_head window;
  struct Cell_head windowCopy;
  bool ok = true;
  try
  {
    QgsGrass::region( mGisdbase, mLocation, "PERMANENT", &window );
  }
  catch ( QgsGrass::Exception &e )
  {
    Q_UNUSED( e );
    reportRow( "QgsGrass::region() failed" );
    ok = false;
  }

  if ( ok )
  {
    QString expectedRegion = "proj:3;zone:0;north:90N;south:90S;east:180E;west:180W;cols:1000;rows:500;e-w resol:0:21:36;n-s resol:0:21:36;";
    QString region = QgsGrass::regionString( &window );
    reportRow( "expectedRegion: " + expectedRegion );
    reportRow( "region: " + region );
    compare( expectedRegion, region, ok );
    windowCopy.proj = window.proj;
    windowCopy.zone = window.zone;
    windowCopy.rows = window.rows;
    windowCopy.cols = window.cols;
    QgsGrass::copyRegionExtent( &window, &windowCopy );
    QgsGrass::copyRegionResolution( &window, &windowCopy );
    QString regionCopy = QgsGrass::regionString( &windowCopy );
    reportRow( "regionCopy: " + regionCopy );
    compare( expectedRegion, regionCopy, ok );
  }
  GVERIFY( ok );
}

void TestQgsGrassProvider::info()
{
  // info() -> getInfo() -> runModule() -> startModule()
  reportHeader( "TestQgsGrassProvider::info" );
  bool ok = true;
  QgsRectangle expectedExtent( -5, -5, 5, 5 );
  QMap<QString, QgsRasterBandStats> expectedStats;
  QgsRasterBandStats es;
  es.minimumValue = -20;
  es.maximumValue = 20;
  expectedStats.insert( "cell", es );
  es.minimumValue = -20.25;
  es.maximumValue = 20.25;
  expectedStats.insert( "dcell", es );
  es.minimumValue = -20.25;
  es.maximumValue = 20.25;
  expectedStats.insert( "fcell", es );
  foreach ( QString map, expectedStats.keys() )
  {
    es = expectedStats.value( map );
    // TODO: QgsGrass::info() may open dialog window on error which blocks tests
    QHash<QString, QString> info = QgsGrass::info( mGisdbase, mLocation, "test", map, QgsGrassObject::Raster, "stats",
                                   expectedExtent, 10, 10, 5000, false );
    reportRow( "map: " + map );
    QgsRasterBandStats s;
    s.minimumValue = info["MIN"].toDouble();
    s.maximumValue = info["MAX"].toDouble();

    reportRow( QString( "expectedStats: min = %1 max = %2" ).arg( es.minimumValue ).arg( es.maximumValue ) ) ;
    reportRow( QString( "stats: min = %1 max = %2" ).arg( s.minimumValue ).arg( s.maximumValue ) ) ;
    compare( es.minimumValue, s.minimumValue, ok );
    compare( es.maximumValue, s.maximumValue, ok );

    QgsRectangle extent = QgsGrass::extent( mGisdbase, mLocation, "test", map, QgsGrassObject::Raster, false );
    reportRow( "expectedExtent: " + expectedExtent.toString() );
    reportRow( "extent: " + extent.toString() );
    if ( extent != expectedExtent )
    {
      ok = false;
    }
  }

  reportRow( "" );
  QgsCoordinateReferenceSystem expectedCrs;
  expectedCrs.createFromOgcWmsCrs( "EPSG:4326" );
  QgsCoordinateReferenceSystem crs = QgsGrass::crs( mGisdbase, mLocation );
  reportRow( "expectedCrs: " + expectedCrs.toWkt() );
  reportRow( "crs: " + crs.toWkt() );
  if ( crs != expectedCrs )
  {
    ok = false;
  }

  GVERIFY( ok );
}

// create temporary output location
bool TestQgsGrassProvider::createTmpLocation( QString& tmpGisdbase, QString& tmpLocation, QString& tmpMapset )
{
  // use QTemporaryFile to generate name (QTemporaryDir since 5.0)
  QTemporaryFile* tmpFile = new QTemporaryFile( QDir::tempPath() + "/qgis-grass-test" );
  tmpFile->open();
  tmpGisdbase = tmpFile->fileName();
  delete tmpFile;
  reportRow( "tmpGisdbase: " + tmpGisdbase );
  tmpLocation = "test";
  tmpMapset = "PERMANENT";

  QString tmpMapsetPath = tmpGisdbase + "/" + tmpLocation + "/" + tmpMapset;
  reportRow( "tmpMapsetPath: " + tmpMapsetPath );
  QDir tmpDir = QDir::temp();
  if ( !tmpDir.mkpath( tmpMapsetPath ) )
  {
    reportRow( "cannot create " + tmpMapsetPath );
    return false;
  }

  QStringList cpFiles;
  cpFiles << "DEFAULT_WIND" << "WIND" << "PROJ_INFO" << "PROJ_UNITS";
  QString templateMapsetPath = mGisdbase + "/" + mLocation + "/PERMANENT";
  foreach ( QString cpFile, cpFiles )
  {
    if ( !QFile::copy( templateMapsetPath + "/" + cpFile, tmpMapsetPath + "/" + cpFile ) )
    {
      reportRow( "cannot copy " + cpFile );
      return false;
    }
  }
  return true;
}

void TestQgsGrassProvider::rasterImport()
{
  reportHeader( "TestQgsGrassProvider::rasterImport" );
  bool ok = true;

  QString tmpGisdbase;
  QString tmpLocation;
  QString tmpMapset;

  if ( !createTmpLocation( tmpGisdbase, tmpLocation, tmpMapset ) )
  {
    reportRow( "cannot create temporary location" );
    GVERIFY( false );
    return;
  }

  QStringList rasterFiles;
  rasterFiles << "tenbytenraster.asc" << "landsat.tif" << "raster/band1_byte_ct_epsg4326.tif" << "raster/band1_int16_noct_epsg4326.tif";
  rasterFiles << "raster/band1_float32_noct_epsg4326.tif" << "raster/band3_int16_noct_epsg4326.tif";

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );
  foreach ( QString rasterFile, rasterFiles )
  {
    QString uri = QString( TEST_DATA_DIR ) + "/" + rasterFile;
    QString name = QFileInfo( uri ).baseName();
    reportRow( "input raster: " + uri );
    QgsRasterDataProvider* provider = qobject_cast<QgsRasterDataProvider*>( QgsProviderRegistry::instance()->provider( "gdal", uri ) );
    if ( !provider )
    {
      reportRow( "Cannot create provider " + uri );
      ok = false;
      continue;
    }
    if ( !provider->isValid() )
    {
      reportRow( "Provider is not valid " + uri );
      ok = false;
      continue;
    }

    QgsRectangle newExtent = provider->extent();
    int newXSize = provider->xSize();
    int newYSize = provider->ySize();

    QgsRasterPipe* pipe = new QgsRasterPipe();
    pipe->set( provider );

    QgsCoordinateReferenceSystem providerCrs = provider->crs();
    if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
    {
      QgsRasterProjector * projector = new QgsRasterProjector;
      projector->setCRS( providerCrs, mapsetCrs );
      projector->destExtentSize( provider->extent(), provider->xSize(), provider->ySize(),
                                 newExtent, newXSize, newYSize );

      pipe->set( projector );
    }

    QgsGrassObject rasterObject( tmpGisdbase, tmpLocation, tmpMapset, name, QgsGrassObject::Raster );
    QgsGrassRasterImport *import = new QgsGrassRasterImport( pipe, rasterObject,
        newExtent, newXSize, newYSize );
    if ( !import->import() )
    {
      reportRow( "import failed: " +  import->error() );
      ok = false;
    }
    delete import;
  }

  GVERIFY( ok );
}

void TestQgsGrassProvider::vectorImport()
{
  reportHeader( "TestQgsGrassProvider::vectorImport" );
  bool ok = true;

  QString tmpGisdbase;
  QString tmpLocation;
  QString tmpMapset;

  if ( !createTmpLocation( tmpGisdbase, tmpLocation, tmpMapset ) )
  {
    reportRow( "cannot create temporary location" );
    GVERIFY( false );
    return;
  }

  QStringList files;
  files << "points.shp" << "multipoint.shp" << "lines.shp" << "polys.shp";
  files << "polys_overlapping.shp" << "bug5598.shp";

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );
  foreach ( QString file, files )
  {
    QString uri = QString( TEST_DATA_DIR ) + "/" + file;
    QString name = QFileInfo( uri ).baseName();
    reportRow( "input vector: " + uri );
    QgsVectorDataProvider* provider = qobject_cast<QgsVectorDataProvider*>( QgsProviderRegistry::instance()->provider( "ogr", uri ) );
    if ( !provider )
    {
      reportRow( "Cannot create provider " + uri );
      ok = false;
      continue;
    }
    if ( !provider->isValid() )
    {
      reportRow( "Provider is not valid " + uri );
      ok = false;
      continue;
    }

    QgsGrassObject vectorObject( tmpGisdbase, tmpLocation, tmpMapset, name, QgsGrassObject::Vector );
    QgsGrassVectorImport *import = new QgsGrassVectorImport( provider, vectorObject );
    if ( !import->import() )
    {
      reportRow( "import failed: " +  import->error() );
      ok = false;
    }
    delete import;

    QStringList layers = QgsGrass::vectorLayers( tmpGisdbase, tmpLocation, tmpMapset, name );
    reportRow( "created layers: " + layers.join( "," ) );
  }
  GVERIFY( ok );
}

QTEST_MAIN( TestQgsGrassProvider )
#include "testqgsgrassprovider.moc"
