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
#include <qgsgeometry.h>
#include <qgslinestringv2.h>
#include <qgspointv2.h>
#include <qgspolygonv2.h>
#include <qgsproviderregistry.h>
#include <qgsrasterbandstats.h>
#include <qgsrasterlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsvectorlayer.h>

#include <qgsgrass.h>
#include <qgsgrassimport.h>
#include <qgsgrassprovider.h>

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
}

#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20

// Feature + GRASS primitive type
class TestQgsGrassFeature : public QgsFeature
{
  public:
    TestQgsGrassFeature() : grassType( 0 ) { setValid( true ); }
    explicit TestQgsGrassFeature( int type ) : grassType( type ) { setValid( true ); }

    int grassType;
};

// Command which can be composed of more GRASS features, e.g. boundaries + centroid equivalent
// of simple fature polygon
class TestQgsGrassCommand
{
  public:
    enum Command
    {
      StartEditing,
      CommitChanges,
      RollBack,
      AddFeature,
      DeleteFeature,
      ChangeGeometry,
      AddAttribute,
      DeleteAttribute,
      ChangeAttributeValue,
      UndoAll,
      RedoAll
    };

    TestQgsGrassCommand()
        : command( AddFeature )
        , verify( true )
        , fid( 0 )
        , geometry( 0 )
    {}
    explicit TestQgsGrassCommand( Command c )
        : command( c )
        , verify( true )
        , fid( 0 )
        , geometry( 0 )
    {}

    QString toString() const;
    Command command;
    // some commands (in case of multiple commands making single change) must not be verified
    bool verify;

    QList<TestQgsGrassFeature> grassFeatures;
    QgsFeature expectedFeature; // simple feature for verification
    QgsFeatureId fid;
    QgsField field;
    QgsGeometry *geometry;
    QVariant value;

    QMap<QString, QVariant> values;
    // map of attributes by name
    QMap<QString, QVariant> attributes;
};

QString TestQgsGrassCommand::toString() const
{
  QString string;
  if ( command == StartEditing )
  {
    string += "StartEditing grassLayerCode: " + values["grassLayerCode"].toString();
    string += " expectedLayerType: " + values["expectedLayerType"].toString();
  }
  else if ( command == CommitChanges )
  {
    string += "CommitChanges";
  }
  else if ( command == RollBack )
  {
    string += "RollBack";
  }
  else if ( command == AddFeature )
  {
    string += "AddFeature ";
    Q_FOREACH ( const TestQgsGrassFeature & grassFeature, grassFeatures )
    {
      if ( grassFeature.constGeometry() )
      {
        string += "<br>grass: " + grassFeature.constGeometry()->exportToWkt( 1 );
      }
    }

    if ( expectedFeature.constGeometry() )
    {
      string += "<br>expected: " + expectedFeature.constGeometry()->exportToWkt( 1 );
    }
  }
  else if ( command == DeleteFeature )
  {
    string += "DeleteFeature ";
    string += QString( "fid: %1" ).arg( fid );
  }
  else if ( command == ChangeGeometry )
  {
    string += "ChangeGeometry ";
    string += QString( "fid: %1 geometry: %2" ).arg( fid ).arg( geometry->exportToWkt( 1 ) );
  }
  else if ( command == AddAttribute )
  {
    string += "AddAttribute ";
    string += "name: " + field.name() + " type: " + QVariant::typeToName( field.type() );
  }
  else if ( command == DeleteAttribute )
  {
    string += "DeleteAttribute ";
    string += "name: " + field.name();
  }
  else if ( command == ChangeAttributeValue )
  {
    string += "ChangeAttributeValue ";
    string += "name: " + field.name() + " value: " + value.toString();
  }
  else if ( command == UndoAll )
  {
    string += "UndoAll";
  }
  else if ( command == RedoAll )
  {
    string += "RedoAll";
  }
  return string;
}

// Group of commands for one vector layer
class TestQgsGrassCommandGroup
{
  public:
    QList<TestQgsGrassCommand> commands;

    // mapping of our fid to negative fid assigned by qgis (start from -2, static variable for all layers)
    QMap<QgsFeatureId, QgsFeatureId> fids;
    QMap<QgsFeatureId, QgsFeatureId> expectedFids;
};

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
    void edit();
  private:
    void reportRow( const QString& message );
    void reportHeader( const QString& message );
    // verify result and report result
    bool verify( bool ok );
    // compare expected and got string and set ok to false if not equal
    bool compare( const QString& expected, const QString& got, bool& ok );
    // lists are considered equal if contains the same values regardless order
    // set ok to false if not equal
    bool compare( const QStringList& expected, const QStringList& got, bool& ok );
    // compare with tolerance
    bool compare( double expected, double got, bool& ok );
    bool copyRecursively( const QString &srcFilePath, const QString &tgtFilePath, QString *error );
    bool removeRecursively( const QString &filePath, QString *error = 0 );
    bool copyLocation( QString& tmpGisdbase );
    bool createTmpLocation( QString& tmpGisdbase, QString& tmpLocation, QString& tmpMapset );
    bool equal( QgsFeature feature, QgsFeature expectedFeatures );
    bool compare( QList<QgsFeature> features, QList<QgsFeature> expectedFeatures, bool& ok );
    bool compare( QMap<QString, QgsVectorLayer *> layers, bool& ok );
    bool compare( QString uri, QgsVectorLayer *expectedLayer, bool& ok );
    QList< TestQgsGrassCommandGroup > createCommands();
    QList<QgsFeature> getFeatures( QgsVectorLayer *layer );
    bool setAttributes( QgsFeature & feature, const QMap<QString, QVariant> &attributes );
    QString mGisdbase;
    QString mLocation;
    QString mReport;
    QString mBuildMapset;
};

#define GVERIFY(x) QVERIFY( verify(x) )

void TestQgsGrassProvider::reportRow( const QString& message )
{
  mReport += message + "<br>\n";
}
void TestQgsGrassProvider::reportHeader( const QString& message )
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
  QgsGrass::putEnv( "QGIS_PROVIDER_FILE", QString( "gdal|ogr|memoryprovider|grassprovider%1" ).arg( GRASS_BUILD_VERSION ) );
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

  QgsGrass::setMute();
  if ( !QgsGrass::init() )
  {
    reportRow( "Cannot init GRASS: " + QgsGrass::initError() );
  }

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

bool TestQgsGrassProvider::compare( const QString& expected, const QString& got, bool &ok )
{
  if ( expected != got )
  {
    ok = false;
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( const QStringList& expected, const QStringList& got, bool &ok )
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

  // User must be owner of mapset if it has to be opened (locked)
  // -> make copy because the source may have different user
  QString tmpGisdbase;
  if ( !copyLocation( tmpGisdbase ) )
  {
    reportRow( "cannot copy location" );
    GVERIFY( false );
    return;
  }

  QStringList expectedMapsets;
  expectedMapsets << "PERMANENT" << "test" << "test6" << "test7";
  QStringList mapsets = QgsGrass::mapsets( tmpGisdbase,  mLocation );
  reportRow( "expectedMapsets: " + expectedMapsets.join( ", " ) );
  reportRow( "mapsets: " + mapsets.join( ", " ) );
  compare( expectedMapsets, mapsets, ok );
  QgsGrass::setLocation( tmpGisdbase,  mLocation ); // for G_is_mapset_in_search_path
  // Disabled because adding of all mapsets to search path was disabled in setLocation()
#if 0
  Q_FOREACH ( QString expectedMapset, expectedMapsets )
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
    QString error = QgsGrass::openMapset( tmpGisdbase, mLocation, mBuildMapset );
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
  removeRecursively( tmpGisdbase );
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
  Q_FOREACH ( const QString& map, expectedStats.keys() )
  {
    es = expectedStats.value( map );
    // TODO: QgsGrass::info() may open dialog window on error which blocks tests
    QString error;
    QHash<QString, QString> info = QgsGrass::info( mGisdbase, mLocation, "test", map, QgsGrassObject::Raster, "stats",
                                   expectedExtent, 10, 10, 5000, error );
    if ( !error.isEmpty() )
    {
      ok = false;
      reportRow( "error: " + error );
      continue;
    }

    reportRow( "map: " + map );
    QgsRasterBandStats s;
    s.minimumValue = info["MIN"].toDouble();
    s.maximumValue = info["MAX"].toDouble();

    reportRow( QString( "expectedStats: min = %1 max = %2" ).arg( es.minimumValue ).arg( es.maximumValue ) ) ;
    reportRow( QString( "stats: min = %1 max = %2" ).arg( s.minimumValue ).arg( s.maximumValue ) ) ;
    compare( es.minimumValue, s.minimumValue, ok );
    compare( es.maximumValue, s.maximumValue, ok );

    QgsRectangle extent = QgsGrass::extent( mGisdbase, mLocation, "test", map, QgsGrassObject::Raster, error );
    reportRow( "expectedExtent: " + expectedExtent.toString() );
    reportRow( "extent: " + extent.toString() );
    if ( !error.isEmpty() )
    {
      ok = false;
      reportRow( "error: " + error );
    }
    if ( extent != expectedExtent )
    {
      ok = false;
    }
  }

  reportRow( "" );
  QgsCoordinateReferenceSystem expectedCrs;
  expectedCrs.createFromOgcWmsCrs( "EPSG:4326" );

  reportRow( "expectedCrs: " + expectedCrs.toWkt() );
  QString error;
  QgsCoordinateReferenceSystem crs = QgsGrass::crs( mGisdbase, mLocation, error );
  if ( !error.isEmpty() )
  {
    ok = false;
    reportRow( "crs: cannot read crs: " + error );
  }
  else
  {
    if ( !crs.isValid() )
    {
      reportRow( "crs: cannot read crs: " + QgsGrass::errorMessage() );
      ok = false;
    }
    else
    {
      reportRow( "crs: " + crs.toWkt() );
      if ( crs != expectedCrs )
      {
        ok = false;
      }
    }
  }
  GVERIFY( ok );
}


// From Qt creator
bool TestQgsGrassProvider::copyRecursively( const QString &srcFilePath, const QString &tgtFilePath, QString *error )
{
  QFileInfo srcFileInfo( srcFilePath );
  if ( srcFileInfo.isDir() )
  {
    QDir targetDir( tgtFilePath );
    targetDir.cdUp();
    if ( !targetDir.mkdir( QFileInfo( tgtFilePath ).fileName() ) )
    {
      if ( error )
      {
        *error = QCoreApplication::translate( "Utils::FileUtils", "Failed to create directory '%1'." )
                 .arg( QDir::toNativeSeparators( tgtFilePath ) );
        return false;
      }
    }
    QDir sourceDir( srcFilePath );
    QStringList fileNames = sourceDir.entryList( QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System );
    Q_FOREACH ( const QString &fileName, fileNames )
    {
      const QString newSrcFilePath
      = srcFilePath + QLatin1Char( '/' ) + fileName;
      const QString newTgtFilePath
      = tgtFilePath + QLatin1Char( '/' ) + fileName;
      if ( !copyRecursively( newSrcFilePath, newTgtFilePath, error ) )
        return false;
    }
  }
  else
  {
    if ( !QFile::copy( srcFilePath, tgtFilePath ) )
    {
      if ( error )
      {
        *error = QCoreApplication::translate( "Utils::FileUtils", "Could not copy file '%1' to '%2'." )
                 .arg( QDir::toNativeSeparators( srcFilePath ),
                       QDir::toNativeSeparators( tgtFilePath ) );
      }
      return false;
    }
  }
  return true;
}

// From Qt creator
bool TestQgsGrassProvider::removeRecursively( const QString &filePath, QString *error )
{
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() )
    return true;
  QFile::setPermissions( filePath, fileInfo.permissions() | QFile::WriteUser );
  if ( fileInfo.isDir() )
  {
    QDir dir( filePath );
    QStringList fileNames = dir.entryList( QDir::Files | QDir::Hidden
                                           | QDir::System | QDir::Dirs | QDir::NoDotAndDotDot );
    Q_FOREACH ( const QString &fileName, fileNames )
    {
      if ( !removeRecursively( filePath + QLatin1Char( '/' ) + fileName, error ) )
        return false;
    }
    dir.cdUp();
    if ( !dir.rmdir( fileInfo.fileName() ) )
    {
      if ( error )
      {
        *error = QCoreApplication::translate( "Utils::FileUtils", "Failed to remove directory '%1'." )
                 .arg( QDir::toNativeSeparators( filePath ) );
      }
      return false;
    }
  }
  else
  {
    if ( !QFile::remove( filePath ) )
    {
      if ( error )
      {
        *error = QCoreApplication::translate( "Utils::FileUtils", "Failed to remove file '%1'." )
                 .arg( QDir::toNativeSeparators( filePath ) );
      }
      return false;
    }
  }
  return true;
}

// copy test location to temporary
bool TestQgsGrassProvider::copyLocation( QString& tmpGisdbase )
{
  // use QTemporaryFile to generate name (QTemporaryDir since 5.0)
  QTemporaryFile* tmpFile = new QTemporaryFile( QDir::tempPath() + "/qgis-grass-test" );
  tmpFile->open();
  tmpGisdbase = tmpFile->fileName();
  delete tmpFile;
  reportRow( "tmpGisdbase: " + tmpGisdbase );

  QString error;
  if ( !copyRecursively( mGisdbase, tmpGisdbase, &error ) )
  {
    reportRow( "cannot copy location " + mGisdbase + " to " + tmpGisdbase + " : " + error );
    return false;
  }
  return true;
}

// create temporary output location
bool TestQgsGrassProvider::createTmpLocation( QString& tmpGisdbase, QString& tmpLocation, QString& tmpMapset )
{
  // use QTemporaryFile to generate name (QTemporaryDir since 5.0)
  QTemporaryFile* tmpFile = new QTemporaryFile( QDir::tempPath() + "/qgis-grass-test" );
  tmpFile->open();
  tmpGisdbase = tmpFile->fileName();
  delete tmpFile;
  //tmpGisdbase = QDir::tempPath() + "/qgis-grass-test/test"; // debug
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
  Q_FOREACH ( const QString& cpFile, cpFiles )
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
  // tenbytenraster.asc does not have CRS, import to EPSG:4326 without reprojection fails
  // in G_adjust_Cell_head() (Illegal latitude for North)
  //rasterFiles << "tenbytenraster.asc";
  rasterFiles << "landsat.tif" << "raster/band1_byte_ct_epsg4326.tif" << "raster/band1_int16_noct_epsg4326.tif";
  rasterFiles << "raster/band1_float32_noct_epsg4326.tif" << "raster/band3_int16_noct_epsg4326.tif";

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );
  Q_FOREACH ( const QString& rasterFile, rasterFiles )
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
  removeRecursively( tmpGisdbase );
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
  Q_FOREACH ( const QString& file, files )
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
  removeRecursively( tmpGisdbase );
  GVERIFY( ok );
}

QList< TestQgsGrassCommandGroup > TestQgsGrassProvider::createCommands()
{
  QList< TestQgsGrassCommandGroup > commandGroups;

  TestQgsGrassCommandGroup commandGroup;
  TestQgsGrassCommand command;
  TestQgsGrassFeature grassFeature;
  QgsLineStringV2 * line;
  QgsGeometry *geometry;
  QList<QgsPointV2> pointList;

  // Start editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values["grassLayerCode"] = "1_point";
  command.values["expectedLayerType"] = "Point";
  commandGroup.commands << command;

  // Add point
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  grassFeature = TestQgsGrassFeature( GV_POINT );
  grassFeature.setFeatureId( 1 );
  geometry = new QgsGeometry( new QgsPointV2( QgsWKBTypes::Point, 10, 10, 0 ) );
  grassFeature.setGeometry( geometry );
  command.grassFeatures << grassFeature;
  command.expectedFeature = grassFeature;
  commandGroup.commands << command;

  // Change geometry
  command = TestQgsGrassCommand( TestQgsGrassCommand::ChangeGeometry );
  command.fid = 1;
  command.geometry = new QgsGeometry( new QgsPointV2( QgsWKBTypes::Point, 20, 20, 0 ) );
  commandGroup.commands << command;

  // Add field
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddAttribute );
  command.field = QgsField( "field_int", QVariant::Int, "integer" );
  commandGroup.commands << command;

  // Change attribute
  command = TestQgsGrassCommand( TestQgsGrassCommand::ChangeAttributeValue );
  command.fid = 1;
  command.field.setName( "field_int" );
  command.value = 123;
  commandGroup.commands << command;

  // Delete field
  command = TestQgsGrassCommand( TestQgsGrassCommand::DeleteAttribute );
  command.field = QgsField( "field_int", QVariant::Int, "integer" );
  commandGroup.commands << command;

  // Delete feature
  command = TestQgsGrassCommand( TestQgsGrassCommand::DeleteFeature );
  command.fid = 1;
  commandGroup.commands << command;

  // Undo
  command = TestQgsGrassCommand( TestQgsGrassCommand::UndoAll );
  commandGroup.commands << command;

  // Redo
  command = TestQgsGrassCommand( TestQgsGrassCommand::RedoAll );
  commandGroup.commands << command;

  // store all commands until commit
  QList<TestQgsGrassCommand> commands = commandGroup.commands;

  // Commit
  command = TestQgsGrassCommand( TestQgsGrassCommand::CommitChanges );
  commandGroup.commands << command;

  // replay the same set of commands with rollback at the end
  commandGroup.commands << commands;

  // Roll back
  command = TestQgsGrassCommand( TestQgsGrassCommand::RollBack );
  commandGroup.commands << command;

  //------------------------ Layer 2 -------------------------------
  // replay the same set of commands for layer 2
  commands = commandGroup.commands;
  for ( int i = 0; i < commands.size(); i++ )
  {
    if ( commands[i].command == TestQgsGrassCommand::StartEditing )
    {
      commands[i].values["grassLayerCode"] = "2_point";
    }
  }

  commandGroup.commands << commands;

  commandGroups << commandGroup;

  //------------------------ Group 2 (issue #13726) -------------------------------
  commandGroup = TestQgsGrassCommandGroup();

  // Start editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values["grassLayerCode"] = "1_line";
  command.values["expectedLayerType"] = "LineString";
  commandGroup.commands << command;

  // Add field
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddAttribute );
  command.field = QgsField( "field_int", QVariant::Int, "integer" );
  commandGroup.commands << command;

  // Add line feature with attributes
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  grassFeature = TestQgsGrassFeature( GV_LINE );
  grassFeature.setFeatureId( 1 );
  line = new QgsLineStringV2();
  pointList.clear();
  pointList << QgsPointV2( QgsWKBTypes::Point, 0, 0, 0 );
  pointList << QgsPointV2( QgsWKBTypes::Point, 20, 10, 0 );
  line->setPoints( pointList );
  pointList.clear();
  geometry = new QgsGeometry( line );
  grassFeature.setGeometry( geometry );
  command.grassFeatures << grassFeature;
  command.expectedFeature = grassFeature;
  command.attributes["field_int"] = 456;
  commandGroup.commands << command;

  // Commit
  command = TestQgsGrassCommand( TestQgsGrassCommand::CommitChanges );
  commandGroup.commands << command;

  commandGroups << commandGroup;

  //------------------------ Group 3 (issue #13739) -------------------------------
  // TODO: resolve issue and enable test
#if 0
  commandGroup = TestQgsGrassCommandGroup();

  // Start editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values["grassLayerCode"] = "1_line";
  command.values["expectedLayerType"] = "LineString";
  commandGroup.commands << command;

  // Add field
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddAttribute );
  command.field = QgsField( "field_int", QVariant::Int, "integer" );
  commandGroup.commands << command;

  // Add grass boundary feature without attributes and expected line feature with attributes
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  command.verify = false;
  grassFeature = TestQgsGrassFeature( GV_BOUNDARY );
  grassFeature.setFeatureId( 1 );
  line = new QgsLineStringV2();
  pointList.clear();
  pointList << QgsPointV2( QgsWKBTypes::Point, 0, 0, 0 );
  pointList << QgsPointV2( QgsWKBTypes::Point, 20, 10, 0 );
  line->setPoints( pointList );
  pointList.clear();
  geometry = new QgsGeometry( line );
  grassFeature.setGeometry( geometry );
  command.grassFeatures << grassFeature;
  commandGroup.commands << command;

  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  command.expectedFeature = grassFeature;
  command.attributes["field_int"] = 123;
  command.verify = false;
  commandGroup.commands << command;

  // Commit
  command = TestQgsGrassCommand( TestQgsGrassCommand::CommitChanges );
  command.verify = false;
  commandGroup.commands << command;

  // Restart editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values["grassLayerCode"] = "1_line";
  command.values["expectedLayerType"] = "LineString";
  command.verify = false;
  commandGroup.commands << command;

  // Change attribute
  command = TestQgsGrassCommand( TestQgsGrassCommand::ChangeAttributeValue );
  // hack: it will only change attribute in GRASS layer, the feature in expected layer
  // with this fid does not exist and changeAttributeValue() on non existing fid does not fail
  command.fid = 1000000000;
  command.field.setName( "field_int" );
  command.value = 123;
  commandGroup.commands << command;

  // Roll back
  command = TestQgsGrassCommand( TestQgsGrassCommand::RollBack );
  commandGroup.commands << command;

  commandGroups << commandGroup;
#endif
  return commandGroups;
}

bool TestQgsGrassProvider::setAttributes( QgsFeature & feature, const QMap<QString, QVariant> &attributes )
{
  bool attributesSet = true;
  Q_FOREACH ( const QString fieldName, attributes.keys() )
  {
    int index = feature.fields()->indexFromName( fieldName );
    if ( index < 0 )
    {
      attributesSet = false;
      reportRow( "cannot find index of attribute " + fieldName );
    }
    else
    {
      feature.setAttribute( index, attributes.value( fieldName ) );
    }
  }
  return attributesSet;
}

void TestQgsGrassProvider::edit()
{
  reportHeader( "TestQgsGrassProvider::edit" );
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

  QList< TestQgsGrassCommandGroup > commandGroups = createCommands();

  for ( int i = 0; i < commandGroups.size(); i++ )
  {
    TestQgsGrassCommandGroup & commandGroup = commandGroups[i];

    // Create GRASS vector
    QString name = QString( "edit_%1" ).arg( i );
    QgsGrassObject mapObject = QgsGrassObject( tmpGisdbase, tmpLocation, tmpMapset, name, QgsGrassObject::Vector );
    reportRow( "create new map: " + mapObject.toString() );
    QString error;
    QgsGrass::createVectorMap( mapObject, error );
    if ( !error.isEmpty() )
    {
      reportRow( error );
      ok = false;
      break;
    }

    // map of expected layers with grass uri as key
    QMap<QString, QgsVectorLayer *> expectedLayers;

    QgsVectorLayer *grassLayer = 0;
    QgsGrassProvider *grassProvider = 0;
    QgsVectorLayer *expectedLayer = 0;

    QList<TestQgsGrassCommand> editCommands; // real edit

    Q_FOREACH ( const TestQgsGrassCommand & command, commandGroup.commands )
    {
      reportRow( "<br>command: " + command.toString() );
      bool commandOk = true;

      if ( command.command == TestQgsGrassCommand::StartEditing )
      {
        QString grassUri = mapObject.mapsetPath() + "/" + name + "/" + command.values["grassLayerCode"].toString();
        reportRow( "grassUri: " + grassUri );
        delete grassLayer;
        grassLayer = new QgsVectorLayer( grassUri, name, "grass" );
        if ( !grassLayer->isValid() )
        {
          reportRow( "grassLayer is not valid" );
          commandOk = false;
          break;
        }
        grassProvider = qobject_cast<QgsGrassProvider *>( grassLayer->dataProvider() );
        if ( !grassProvider )
        {
          reportRow( "cannot get grassProvider" );
          commandOk = false;
          break;
        }
        if ( expectedLayers.keys().contains( grassUri ) )
        {
          reportRow( "expectedLayer exists" );
          expectedLayer = expectedLayers.value( grassUri );
        }
        else
        {
          reportRow( "create new expectedLayer" );
          // Create memory vector for verification, it has no fields until added
          expectedLayer = new QgsVectorLayer( command.values["expectedLayerType"].toString(), "test", "memory" );
          if ( !expectedLayer->isValid() )
          {
            reportRow( "expectedLayer is not valid" );
            commandOk = false;
            break;
          }
          expectedLayers.insert( grassUri, expectedLayer );
        }

        grassLayer->startEditing();
        grassProvider->startEditing( grassLayer );

        expectedLayer->startEditing();
      }

      if ( !grassLayer || !grassProvider || !expectedLayer )
      {
        reportRow( "grassLayer, grassProvider or expectedLayer is null" );
        commandOk = false;
        break;
      }
      else if ( command.command == TestQgsGrassCommand::CommitChanges )
      {
        grassLayer->commitChanges();
        expectedLayer->commitChanges();
        editCommands.clear();
        commandGroup.fids.clear();
        commandGroup.expectedFids.clear();
      }
      else if ( command.command == TestQgsGrassCommand::RollBack )
      {
        grassLayer->rollBack();
        expectedLayer->rollBack();
        editCommands.clear();
        commandGroup.fids.clear();
        commandGroup.expectedFids.clear();
      }
      else if ( command.command == TestQgsGrassCommand::AddFeature )
      {
        Q_FOREACH ( TestQgsGrassFeature grassFeature, command.grassFeatures ) // copy feature, not reference
        {
          QgsFeatureId fid = grassFeature.id();
          grassProvider->setNewFeatureType( grassFeature.grassType );
          grassFeature.setFields( grassLayer->fields() );
          grassFeature.initAttributes( grassLayer->fields().size() ); // attributes must match layer fields
          if ( !setAttributes( grassFeature, command.attributes ) )
          {
            commandOk = false;
            break;
          }

          if ( !grassLayer->addFeature( grassFeature ) )
          {
            reportRow( "cannot add feature" );
            commandOk = false;
            break;
          }
          commandGroup.fids.insert( fid, grassFeature.id() );
        }

        QgsFeature expectedFeature = command.expectedFeature;
        // some features (e.g. boundaries for future line features) are added only to grass layer
        // in that case expectedFeature is invalid
        if ( expectedFeature.isValid() )
        {
          QgsFeatureId expectedFid = expectedFeature.id();
          expectedFeature.setFields( expectedLayer->fields() );
          expectedFeature.initAttributes( expectedLayer->fields().size() );
          if ( !setAttributes( expectedFeature, command.attributes ) )
          {
            commandOk = false;
            break;
          }
          if ( !expectedLayer->addFeature( expectedFeature ) )
          {
            reportRow( "cannot add expectedFeature" );
            commandOk = false;
          }
          commandGroup.expectedFids.insert( expectedFid, expectedFeature.id() );
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::DeleteFeature )
      {
        QgsFeatureId fid = commandGroup.fids.value( command.fid );
        if ( !grassLayer->deleteFeature( fid ) )
        {
          reportRow( "cannot delete feature" );
          commandOk = false;
        }
        QgsFeatureId expectedFid = commandGroup.expectedFids.value( command.fid );
        if ( !expectedLayer->deleteFeature( expectedFid ) )
        {
          reportRow( "cannot delete expected feature" );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::ChangeGeometry )
      {
        QgsFeatureId fid = command.fid;
        if ( commandGroup.fids.contains( fid ) )
        {
          fid = commandGroup.fids.value( fid );
        }
        if ( !grassLayer->changeGeometry( fid, command.geometry ) )
        {
          reportRow( "cannot change feature geometry" );
          commandOk = false;
        }
        QgsFeatureId expectedFid = command.fid;
        if ( commandGroup.expectedFids.contains( expectedFid ) )
        {
          expectedFid = commandGroup.expectedFids.value( expectedFid );
        }
        if ( !expectedLayer->changeGeometry( expectedFid, command.geometry ) )
        {
          reportRow( "cannot change expected feature geometry" );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::AddAttribute )
      {
        if ( !grassLayer->addAttribute( command.field ) )
        {
          reportRow( "cannot add field to layer" );
          commandOk = false;
        }
        if ( !expectedLayer->addAttribute( command.field ) )
        {
          reportRow( "cannot add field to expectedLayer" );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::DeleteAttribute )
      {
        int index = grassLayer->fields().indexFromName( command.field.name() );
        if ( !grassLayer->deleteAttribute( index ) )
        {
          reportRow( "cannot delete field from layer" );
          commandOk = false;
        }
        int expectedIndex = expectedLayer->fields().indexFromName( command.field.name() );
        if ( !expectedLayer->deleteAttribute( expectedIndex ) )
        {
          reportRow( "cannot delete field from expected layer" );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::ChangeAttributeValue )
      {
        QgsFeatureId fid = command.fid;
        if ( commandGroup.fids.contains( fid ) )
        {
          fid = commandGroup.fids.value( fid );
        }
        int index = grassLayer->fields().indexFromName( command.field.name() );
        if ( !grassLayer->changeAttributeValue( fid, index, command.value ) )
        {
          reportRow( "cannot change feature attribute" );
          commandOk = false;
        }
        QgsFeatureId expectedFid = command.fid;
        if ( commandGroup.expectedFids.contains( expectedFid ) )
        {
          expectedFid = commandGroup.expectedFids.value( expectedFid );
        }
        int expectedIndex = expectedLayer->fields().indexFromName( command.field.name() );
        if ( !expectedLayer->changeAttributeValue( expectedFid, expectedIndex, command.value ) )
        {
          reportRow( "cannot change expected feature attribute" );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::UndoAll )
      {
        if ( grassLayer->undoStack()->count() !=  editCommands.size() ||
             grassLayer->undoStack()->count() != expectedLayer->undoStack()->count() )
        {
          reportRow( QString( "Different undo stack size: %1, expected: %2, editCommands: %3" )
                     .arg( grassLayer->undoStack()->count() ).arg( expectedLayer->undoStack()->count() ).arg( editCommands.size() ) );
          commandOk = false;
        }
        else
        {
          for ( int j = editCommands.size() - 1; j >= 0; j-- )
          {
            reportRow( "<br>undo command: " + editCommands[j].toString() );
            grassLayer->undoStack()->undo();
            expectedLayer->undoStack()->undo();
            if ( !compare( expectedLayers, ok ) )
            {
              reportRow( "undo failed" );
              commandOk = false;
              break;
            }
            else
            {
              reportRow( "undo ok" );
            }
          }
        }
      }
      else if ( command.command == TestQgsGrassCommand::RedoAll )
      {
        if ( grassLayer->undoStack()->count() !=  editCommands.size() ||
             grassLayer->undoStack()->count() != expectedLayer->undoStack()->count() )
        {
          reportRow( QString( "Different undo stack size: %1, expected: %2, editCommands: %3" )
                     .arg( grassLayer->undoStack()->count() ).arg( expectedLayer->undoStack()->count() ).arg( editCommands.size() ) );
          commandOk = false;
        }
        else
        {
          for ( int j = 0; j < editCommands.size(); j++ )
          {
            reportRow( "<br>redo command: " + editCommands[j].toString() );
            grassLayer->undoStack()->redo();
            expectedLayer->undoStack()->redo();
            if ( !compare( expectedLayers, ok ) )
            {
              reportRow( "redo failed" );
              commandOk = false;
              break;
            }
            else
            {
              reportRow( "redo ok" );
            }
          }
        }
      }

      if ( !commandOk )
      {
        reportRow( "command failed" );
        ok = false;
        break;
      }

      if ( !command.verify )
      {
        reportRow( "partial command not verified" );
        continue;
      }

      if ( command.command != TestQgsGrassCommand::UndoAll && command.command != TestQgsGrassCommand::RedoAll )
      {
        if ( !compare( expectedLayers, ok ) )
        {
          reportRow( "command failed" );
          break;
        }
        else
        {
          reportRow( "command ok" );
        }
      }
    }

    delete grassLayer;
    Q_FOREACH ( QgsVectorLayer *layer, expectedLayers.values() )
    {
      delete layer;
    }
  }

  removeRecursively( tmpGisdbase );
  GVERIFY( ok );
}

QList<QgsFeature> TestQgsGrassProvider::getFeatures( QgsVectorLayer *layer )
{
  QgsFeatureIterator iterator = layer->getFeatures( QgsFeatureRequest() );
  QgsFeature feature;
  QList<QgsFeature> features;
  while ( iterator.nextFeature( feature ) )
  {
    features << feature;
  }
  iterator.close();
  return features;
}

bool TestQgsGrassProvider::equal( QgsFeature feature, QgsFeature expectedFeature )
{
  if ( !feature.constGeometry()->equals( expectedFeature.constGeometry() ) )
  {
    return false;
  }
  // GRASS feature has always additional cat field
  QSet<int> indexes;
  for ( int i = 0; i < feature.fields()->size(); i++ )
  {
    QString name = feature.fields()->at( i ).name();
    if ( name == "cat" ) // skip cat
    {
      continue;
    }
    indexes << i;
  }
  for ( int i = 0; i < expectedFeature.fields()->size(); i++ )
  {
    QString name = expectedFeature.fields()->at( i ).name();
    int index = feature.fields()->indexFromName( name );
    if ( index < 0 )
    {
      // not found
      QStringList names;
      for ( int j = 0; j < feature.fields()->size(); j++ )
      {
        names << feature.fields()->at( j ).name();
      }
      reportRow( QString( "Attribute %1 not found, feature attributes: %2" ).arg( name, names.join( "," ) ) );
      return false;
    }
    indexes.remove( index );
    if ( feature.attribute( index ) != expectedFeature.attribute( i ) )
    {
      reportRow( QString( "Attribute name %1, value: '%2' does not match expected value: '%3'" )
                 .arg( name, feature.attribute( index ).toString(), expectedFeature.attribute( i ).toString() ) );
      return false;
    }
  }
  if ( indexes.size() > 0 )
  {
    // unexpected attribute in feature
    QStringList names;
    Q_FOREACH ( int i, indexes )
    {
      names << feature.fields()->at( i ).name();
    }
    reportRow( QString( "feature has %1 unexpected attributes: %2" ).arg( indexes.size() ).arg( names.join( "," ) ) );
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( QList<QgsFeature> features, QList<QgsFeature> expectedFeatures, bool& ok )
{
  bool localOk = true;
  if ( features.size() != expectedFeatures.size() )
  {
    reportRow( QString( "different number of features (%1) and expected features (%2)" ).arg( features.size() ).arg( expectedFeatures.size() ) );
    ok = false;
    return false;
  }
  // Check if each expected feature exists in features
  Q_FOREACH ( const QgsFeature& expectedFeature,  expectedFeatures )
  {
    bool found = false;
    Q_FOREACH ( const QgsFeature& feature, features )
    {
      if ( equal( feature, expectedFeature ) )
      {
        found = true;
        break;
      }
    }
    if ( !found )
    {
      reportRow( QString( "expected feature fid = %1 not found in features" ).arg( expectedFeature.id() ) );
      ok = false;
      localOk = false;
    }
  }
  return localOk;
}

bool TestQgsGrassProvider::compare( QMap<QString, QgsVectorLayer *> layers, bool& ok )
{
  Q_FOREACH ( const QString & grassUri, layers.keys() )
  {
    if ( !compare( grassUri, layers.value( grassUri ), ok ) )
    {
      reportRow( "comparison failed: " + grassUri );
    }
    else
    {
      reportRow( "comparison ok: " + grassUri );
    }
  }
  return ok;
}

bool TestQgsGrassProvider::compare( QString uri, QgsVectorLayer *expectedLayer, bool& ok )
{
  QgsGrassObject mapObject;
  mapObject.setFromUri( uri );
  QList<QgsFeature> expectedFeatures = getFeatures( expectedLayer );

  // read the map using another layer/provider
  QgsVectorLayer *layer = new QgsVectorLayer( uri, "test", "grass" );
  if ( !layer->isValid() )
  {
    reportRow( "shared layer is not valid" );
    ok = false;
    return false;
  }
  QList<QgsFeature> features = getFeatures( layer );
  delete layer;
  layer = 0;

  bool sharedOk = compare( features, expectedFeatures, ok );
  if ( sharedOk )
  {
    //reportRow( "comparison with shared layer ok" );
  }
  else
  {
    reportRow( "comparison with shared layer failed" );
  }

  // We cannot test attribute table changes with independent layer in GRASS 6, which is using
  // DBF files, which are written when driver is closed (map layer keeps driver open during editing)
  bool independentOk = true;
#if GRASS_VERSION_MAJOR >= 7
  // Open an independent layer which does not share data with edited one
  // build topology
  G_TRY
  {
    struct Map_info *map = QgsGrass::vectNewMapStruct();
    QgsGrass::setMapset( mapObject );
    Vect_set_open_level( 1 ); // otherwise Vect_open_old fails if cannot open on level 2
    int result = Vect_open_old( map, mapObject.name().toUtf8().data(), mapObject.mapset().toUtf8().data() );

    if ( result == -1 )
{
  QgsGrass::vectDestroyMapStruct( map );
    throw QgsGrass::Exception( "Cannot open map " + mapObject.name() );
  }

#if ( GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR >= 4 ) || GRASS_VERSION_MAJOR > 6
  Vect_build( map );
#else
  Vect_build( map, stderr );
#endif
  //Vect_set_release_support( map );
  Vect_close( map );
  QgsGrass::vectDestroyMapStruct( map );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    reportRow( "Cannot build topology: " + QString( e.what() ) );
    ok = false;
    return false;
  }

  QgsGrassVectorMapStore * mapStore = new QgsGrassVectorMapStore();
  QgsGrassVectorMapStore::setStore( mapStore );

  layer = new QgsVectorLayer( uri, "test", "grass" );
  if ( !layer->isValid() )
  {
    reportRow( "independent layer is not valid" );
    ok = false;
    return false;
  }
  features = getFeatures( layer );
  delete layer;
  QgsGrassVectorMapStore::setStore( 0 );
  delete mapStore;

  independentOk = compare( features, expectedFeatures, ok );
  if ( independentOk )
  {
    //reportRow( "comparison with independent layer ok" );
  }
  else
  {
    reportRow( "comparison with independent layer failed" );
  }
#endif // GRASS_VERSION_MAJOR >= 7

  return sharedOk && independentOk;
}

QTEST_MAIN( TestQgsGrassProvider )
#include "testqgsgrassprovider.moc"
