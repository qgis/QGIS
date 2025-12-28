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

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsgrass.h"
#include "qgsgrassimport.h"
#include "qgsgrassprovider.h"
#include "qgslinestring.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgsproviderregistry.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QApplication>
#include <QDir>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
}

#define TINY_VALUE std::numeric_limits<double>::epsilon() * 20

// Feature + GRASS primitive type
class TestQgsGrassFeature : public QgsFeature
{
  public:
    TestQgsGrassFeature() { setValid( true ); }
    explicit TestQgsGrassFeature( int type )
      : grassType( type ) { setValid( true ); }

    int grassType = 0;
};

// Command which can be composed of more GRASS features, e.g. boundaries + centroid equivalent
// of simple feature polygon
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

    TestQgsGrassCommand() = default;
    explicit TestQgsGrassCommand( Command c )
      : command( c )
    {}

    QString toString() const;
    Command command = AddFeature;
    // some commands (in case of multiple commands making single change) must not be verified
    bool verify = true;

    QList<TestQgsGrassFeature> grassFeatures;
    QgsFeature expectedFeature; // simple feature for verification
    QgsFeatureId fid = 0;
    QgsField field;
    QgsGeometry *geometry = nullptr;
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
    string += "StartEditing grassLayerCode: " + values[u"grassLayerCode"_s].toString();
    string += " expectedLayerType: " + values[u"expectedLayerType"_s].toString();
  }
  else if ( command == CommitChanges )
  {
    string += "CommitChanges"_L1;
  }
  else if ( command == RollBack )
  {
    string += "RollBack"_L1;
  }
  else if ( command == AddFeature )
  {
    string += "AddFeature "_L1;
    for ( const TestQgsGrassFeature &grassFeature : grassFeatures )
    {
      if ( grassFeature.hasGeometry() )
      {
        string += "grass: " + grassFeature.geometry().asWkt( 1 );
      }
    }

    if ( expectedFeature.hasGeometry() )
    {
      string += "expected: " + expectedFeature.geometry().asWkt( 1 );
    }
  }
  else if ( command == DeleteFeature )
  {
    string += "DeleteFeature "_L1;
    string += u"fid: %1"_s.arg( fid );
  }
  else if ( command == ChangeGeometry )
  {
    string += "ChangeGeometry "_L1;
    string += u"fid: %1 geometry: %2"_s.arg( fid ).arg( geometry->asWkt( 1 ) );
  }
  else if ( command == AddAttribute )
  {
    string += "AddAttribute "_L1;
    string += "name: " + field.name() + " type: " + QVariant::typeToName( field.type() );
  }
  else if ( command == DeleteAttribute )
  {
    string += "DeleteAttribute "_L1;
    string += "name: " + field.name();
  }
  else if ( command == ChangeAttributeValue )
  {
    string += "ChangeAttributeValue "_L1;
    string += "name: " + field.name() + " value: " + value.toString();
  }
  else if ( command == UndoAll )
  {
    string += "UndoAll"_L1;
  }
  else if ( command == RedoAll )
  {
    string += "RedoAll"_L1;
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

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsGrassProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsGrassProvider()
      : QgsTest( u"Grass provider tests"_s ) {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.

    void fatalError();
    void locations();
    void mapsets();
    void maps();
    void vectorLayers();
    void invalidLayer();
    void region();
    void info();
    void crsEpsg3857();
    void rasterImport();
    void vectorImport();
    void edit();

  private:
    void reportRow( const QString &message );
    void reportHeader( const QString &message );
    // verify result and report result
    bool verify( bool ok );
    // compare expected and got string and set ok to false if not equal
    bool compare( const QString &expected, const QString &got, bool &ok );
    // lists are considered equal if contains the same values regardless order
    // set ok to false if not equal
    bool compare( const QStringList &expected, const QStringList &got, bool &ok );
    // compare with tolerance
    bool compare( double expected, double got, bool &ok );
    bool copyRecursively( const QString &srcFilePath, const QString &tgtFilePath, QString *error );
    bool removeRecursively( const QString &filePath, QString *error = nullptr );
    bool copyLocation( QString &tmpGisdbase );
    bool createTmpLocation( QString &tmpGisdbase, QString &tmpLocation, QString &tmpMapset );
    bool equal( QgsFeature feature, QgsFeature expectedFeatures );
    bool compare( QList<QgsFeature> features, QList<QgsFeature> expectedFeatures, bool &ok );
    bool compare( QMap<QString, QgsVectorLayer *> layers, bool &ok );
    bool compare( QString uri, QgsVectorLayer *expectedLayer, bool &ok );
    QList<TestQgsGrassCommandGroup> createCommands();
    QList<QgsFeature> getFeatures( QgsVectorLayer *layer );
    bool setAttributes( QgsFeature &feature, const QMap<QString, QVariant> &attributes );
    QString mGisdbase;
    QString mLocation;
    QString mBuildMapset;
};

#define GVERIFY( x ) QVERIFY( verify( x ) )

void TestQgsGrassProvider::reportRow( const QString &message )
{
  for ( const QString &line : message.split( '\n' ) )
  {
    qDebug() << line;
  }
}
void TestQgsGrassProvider::reportHeader( const QString &message )
{
  qDebug() << "------";
  qDebug() << message;
  qDebug() << "------";
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
  QgsGrass::putEnv( u"QGIS_PROVIDER_FILE"_s, u"grass(?:provider)?%1"_s.arg( GRASS_BUILD_VERSION ) );
  QgsApplication::initQgis();
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n"_L1, "<br />\n"_L1 );
  reportHeader( u"<h1>GRASS %1 provider tests</h1>\n"_s.arg( GRASS_BUILD_VERSION ) );
  reportRow( mySettings );

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
  mGisdbase = QStringLiteral( TEST_DATA_DIR ) + "/grass";
  mLocation = u"wgs84"_s;
  mBuildMapset = u"test%1"_s.arg( GRASS_BUILD_VERSION );
  reportRow( "mGisdbase: " + mGisdbase );
  reportRow( "mLocation: " + mLocation );
  reportRow( "mBuildMapset: " + mBuildMapset );
  qDebug() << "mGisdbase = " << mGisdbase << " mLocation = " << mLocation;
}

bool TestQgsGrassProvider::verify( bool ok )
{
  if ( !ok )
  {
    reportRow( QString() );
    reportRow( u"Test result: "_s + ( ok ? "ok" : "error" ) );
  }
  return ok;
}

bool TestQgsGrassProvider::compare( const QString &expected, const QString &got, bool &ok )
{
  if ( expected != got )
  {
    ok = false;
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( const QStringList &expected, const QStringList &got, bool &ok )
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

bool TestQgsGrassProvider::compare( double expected, double got, bool &ok )
{
  if ( std::fabs( got - expected ) > TINY_VALUE )
  {
    ok = false;
    return false;
  }
  return true;
}

// G_fatal_error() handling
void TestQgsGrassProvider::fatalError()
{
  reportHeader( u"TestQgsGrassProvider::fatalError"_s );
  bool ok = true;
  QString errorMessage = u"test fatal error"_s;
  G_TRY
  {
    G_fatal_error( "%s", errorMessage.toLatin1().data() );
    ok = false; // should not be reached
    reportRow( u"G_fatal_error() did not throw exception"_s );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    reportRow( u"Exception thrown and caught correctly"_s );
    reportRow( "expected error message: " + errorMessage );
    reportRow( "got error message: " + QString( e.what() ) );
    compare( errorMessage, e.what(), ok );
  }
  compare( errorMessage, QgsGrass::errorMessage(), ok );
  GVERIFY( ok );
}

void TestQgsGrassProvider::locations()
{
  const QStringList locations = QgsGrass::locations( mGisdbase );
  QCOMPARE( locations, QStringList() << u"webmerc"_s << u"wgs84"_s );
}

void TestQgsGrassProvider::mapsets()
{
  reportHeader( u"TestQgsGrassProvider::mapsets"_s );
  bool ok = true;

  // User must be owner of mapset if it has to be opened (locked)
  // -> make copy because the source may have different user
  QString tmpGisdbase;
  if ( !copyLocation( tmpGisdbase ) )
  {
    reportRow( u"cannot copy location"_s );
    GVERIFY( false );
    return;
  }

  QStringList expectedMapsets;
  expectedMapsets << u"PERMANENT"_s << u"test"_s << u"test6"_s << u"test7"_s << u"test8"_s;
  QStringList mapsets = QgsGrass::mapsets( tmpGisdbase, mLocation );
  reportRow( "expectedMapsets: " + expectedMapsets.join( ", "_L1 ) );
  reportRow( "mapsets: " + mapsets.join( ", "_L1 ) );
  compare( expectedMapsets, mapsets, ok );
  QgsGrass::setLocation( tmpGisdbase, mLocation ); // for G_is_mapset_in_search_path
  // Disabled because adding of all mapsets to search path was disabled in setLocation()
#if 0
  for ( const QString &expectedMapset : expectedMapsets )
  {
    if ( G_is_mapset_in_search_path( expectedMapset.toLatin1().data() ) != 1 )
    {
      reportRow( "mapset " + expectedMapset + " not in search path" );
      ok = false;
    }
  }
#endif

  // open/close mapset try twice to be sure that lock was not left etc.
  for ( int i = 1; i < 3; i++ )
  {
    reportRow( QString() );
    reportRow( "Open/close mapset " + mBuildMapset + " for the " + QString::number( i ) + ". time" );
    QString error = QgsGrass::openMapset( tmpGisdbase, mLocation, mBuildMapset );
    if ( !error.isEmpty() )
    {
      reportRow( "QgsGrass::openMapset() failed: " + error );
      ok = false;
    }
    else
    {
      reportRow( u"mapset successfully opened"_s );
      if ( !QgsGrass::activeMode() )
      {
        reportRow( u"QgsGrass::activeMode() returns false after openMapset()"_s );
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
        reportRow( u"mapset successfully closed"_s );
      }

      if ( QgsGrass::activeMode() )
      {
        reportRow( u"QgsGrass::activeMode() returns true after closeMapset()"_s );
        ok = false;
      }
    }
  }
  removeRecursively( tmpGisdbase );
  GVERIFY( ok );
}

void TestQgsGrassProvider::maps()
{
  reportHeader( u"TestQgsGrassProvider::maps"_s );
  bool ok = true;
  QStringList expectedVectors;
  expectedVectors << u"test"_s;
  QStringList vectors = QgsGrass::vectors( mGisdbase, mLocation, mBuildMapset );
  reportRow( "expectedVectors: " + expectedVectors.join( ", "_L1 ) );
  reportRow( "vectors: " + vectors.join( ", "_L1 ) );
  compare( expectedVectors, vectors, ok );

  reportRow( QString() );
  QStringList expectedRasters;
  expectedRasters << u"cell"_s << u"dcell"_s << u"fcell"_s;
  QStringList rasters = QgsGrass::rasters( mGisdbase, mLocation, u"test"_s );
  reportRow( "expectedRasters: " + expectedRasters.join( ", "_L1 ) );
  reportRow( "rasters: " + rasters.join( ", "_L1 ) );
  compare( expectedRasters, rasters, ok );
  GVERIFY( ok );
}

void TestQgsGrassProvider::vectorLayers()
{
  reportHeader( u"TestQgsGrassProvider::vectorLayers"_s );
  QString mapset = mBuildMapset;
  QString mapName = u"test"_s;
  QStringList expectedLayers;
  expectedLayers << u"1_point"_s << u"2_line"_s << u"3_polygon"_s;

  reportRow( "mapset: " + mapset );
  reportRow( "mapName: " + mapName );
  reportRow( "expectedLayers: " + expectedLayers.join( ", "_L1 ) );

  bool ok = true;
  G_TRY
  {
    QStringList layers = QgsGrass::vectorLayers( mGisdbase, mLocation, mapset, mapName );
    reportRow( "layers: " + layers.join( ", "_L1 ) );
    compare( expectedLayers, layers, ok );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    ok = false;
    reportRow( u"ERROR: %1"_s.arg( e.what() ) );
  }
  GVERIFY( ok );
}

void TestQgsGrassProvider::invalidLayer()
{
  auto brokenLayer = std::make_unique<QgsVectorLayer>( u"/not/valid"_s, u"test"_s, u"grass"_s );
  QVERIFY( !brokenLayer->isValid() );
  QgsVectorDataProvider *provider = brokenLayer->dataProvider();
  QVERIFY( provider );
  QVERIFY( !provider->isValid() );
  QVERIFY( provider->fields().isEmpty() );
}

void TestQgsGrassProvider::region()
{
  reportHeader( u"TestQgsGrassProvider::region"_s );
  struct Cell_head window;
  struct Cell_head windowCopy;
  bool ok = true;
  try
  {
    QgsGrass::region( mGisdbase, mLocation, u"PERMANENT"_s, &window );
  }
  catch ( QgsGrass::Exception &e )
  {
    Q_UNUSED( e );
    reportRow( u"QgsGrass::region() failed"_s );
    ok = false;
  }

  if ( ok )
  {
    QString expectedRegion = u"proj:3;zone:0;north:90N;south:90S;east:180E;west:180W;cols:1000;rows:500;e-w resol:0:21:36;n-s resol:0:21:36;"_s;
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
  reportHeader( u"TestQgsGrassProvider::info"_s );
  bool ok = true;

  QgsRectangle expectedExtent( -5, -5, 5, 5 );
  QMap<QString, QgsRasterBandStats> expectedStats;
  QgsRasterBandStats es;
  es.minimumValue = -20;
  es.maximumValue = 20;
  expectedStats.insert( u"cell"_s, es );
  es.minimumValue = -20.25;
  es.maximumValue = 20.25;
  expectedStats.insert( u"dcell"_s, es );
  es.minimumValue = -20.25;
  es.maximumValue = 20.25;
  expectedStats.insert( u"fcell"_s, es );
  for ( const QString &map : expectedStats.keys() )
  {
    es = expectedStats.value( map );
    // TODO: QgsGrass::info() may open dialog window on error which blocks tests
    QString error;
    QHash<QString, QString> info = QgsGrass::info( mGisdbase, mLocation, u"test"_s, map, QgsGrassObject::Raster, u"stats"_s, expectedExtent, 10, 10, 5000, error );
    if ( !error.isEmpty() )
    {
      ok = false;
      reportRow( "error: " + error );
      continue;
    }

    reportRow( "map: " + map );
    QgsRasterBandStats s;
    s.minimumValue = info[u"MIN"_s].toDouble();
    s.maximumValue = info[u"MAX"_s].toDouble();

    reportRow( u"expectedStats: min = %1 max = %2"_s.arg( es.minimumValue ).arg( es.maximumValue ) );
    reportRow( u"stats: min = %1 max = %2"_s.arg( s.minimumValue ).arg( s.maximumValue ) );
    compare( es.minimumValue, s.minimumValue, ok );
    compare( es.maximumValue, s.maximumValue, ok );

    QgsRectangle extent = QgsGrass::extent( mGisdbase, mLocation, u"test"_s, map, QgsGrassObject::Raster, error );
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

  reportRow( QString() );
  QgsCoordinateReferenceSystem expectedCrs( u"EPSG:4326"_s );

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

void TestQgsGrassProvider::crsEpsg3857()
{
  QString error;
  const QgsCoordinateReferenceSystem crs = QgsGrass::crs( mGisdbase, u"webmerc"_s, error );
  QCOMPARE( error, QString() );
  QCOMPARE( crs.authid(), u"EPSG:3857"_s );
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
    for ( const QString &fileName : fileNames )
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
                   .arg( QDir::toNativeSeparators( srcFilePath ), QDir::toNativeSeparators( tgtFilePath ) );
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
    QStringList fileNames = dir.entryList( QDir::Files | QDir::Hidden | QDir::System | QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString &fileName : fileNames )
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
bool TestQgsGrassProvider::copyLocation( QString &tmpGisdbase )
{
  // use QTemporaryFile to generate name (QTemporaryDir since 5.0)
  QTemporaryFile *tmpFile = new QTemporaryFile( QDir::tempPath() + "/qgis-grass-test" );
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
bool TestQgsGrassProvider::createTmpLocation( QString &tmpGisdbase, QString &tmpLocation, QString &tmpMapset )
{
  // use QTemporaryFile to generate name (QTemporaryDir since 5.0)
  QTemporaryFile *tmpFile = new QTemporaryFile( QDir::tempPath() + "/qgis-grass-test" );
  tmpFile->open();
  tmpGisdbase = tmpFile->fileName();
  delete tmpFile;
  //tmpGisdbase = QDir::tempPath() + "/qgis-grass-test/test"; // debug
  reportRow( "tmpGisdbase: " + tmpGisdbase );
  tmpLocation = u"test"_s;
  tmpMapset = u"PERMANENT"_s;

  QString tmpMapsetPath = tmpGisdbase + "/" + tmpLocation + "/" + tmpMapset;
  reportRow( "tmpMapsetPath: " + tmpMapsetPath );
  QDir tmpDir = QDir::temp();
  if ( !tmpDir.mkpath( tmpMapsetPath ) )
  {
    reportRow( "cannot create " + tmpMapsetPath );
    return false;
  }

  QStringList cpFiles;
  cpFiles << u"DEFAULT_WIND"_s << u"WIND"_s << u"PROJ_INFO"_s << u"PROJ_UNITS"_s << u"PROJ_SRID"_s;
  QString templateMapsetPath = mGisdbase + "/" + mLocation + "/PERMANENT";
  for ( const QString &cpFile : cpFiles )
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
  reportHeader( u"TestQgsGrassProvider::rasterImport"_s );
  bool ok = true;

  QString tmpGisdbase;
  QString tmpLocation;
  QString tmpMapset;

  if ( !createTmpLocation( tmpGisdbase, tmpLocation, tmpMapset ) )
  {
    reportRow( u"cannot create temporary location"_s );
    GVERIFY( false );
    return;
  }

  QStringList rasterFiles;
  // tenbytenraster.asc does not have CRS, import to EPSG:4326 without reprojection fails
  // in G_adjust_Cell_head() (Illegal latitude for North)
  //rasterFiles << "tenbytenraster.asc";
  rasterFiles << u"landsat.tif"_s << u"raster/band1_byte_ct_epsg4326.tif"_s << u"raster/band1_int16_noct_epsg4326.tif"_s;
  rasterFiles << u"raster/band1_float32_noct_epsg4326.tif"_s << u"raster/band3_int16_noct_epsg4326.tif"_s;

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );
  for ( const QString &rasterFile : rasterFiles )
  {
    QString uri = QStringLiteral( TEST_DATA_DIR ) + "/" + rasterFile;
    QString name = QFileInfo( uri ).baseName();
    reportRow( "input raster: " + uri );
    QgsRasterDataProvider *provider = qobject_cast<QgsRasterDataProvider *>( QgsProviderRegistry::instance()->createProvider( u"gdal"_s, uri ) );
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

    auto pipe = std::make_unique< QgsRasterPipe >();
    pipe->set( provider );

    QgsCoordinateReferenceSystem providerCrs = provider->crs();
    if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
    {
      QgsRasterProjector *projector = new QgsRasterProjector;
      Q_NOWARN_DEPRECATED_PUSH
      projector->setCrs( providerCrs, mapsetCrs );
      Q_NOWARN_DEPRECATED_POP
      projector->destExtentSize( provider->extent(), provider->xSize(), provider->ySize(), newExtent, newXSize, newYSize );

      pipe->set( projector );
    }

    QgsGrassObject rasterObject( tmpGisdbase, tmpLocation, tmpMapset, name, QgsGrassObject::Raster );
    QgsGrassRasterImport *import = new QgsGrassRasterImport( std::move( pipe ), rasterObject, newExtent, newXSize, newYSize );
    if ( !import->import() )
    {
      reportRow( "import failed: " + import->error() );
      ok = false;
    }
    delete import;
  }
  removeRecursively( tmpGisdbase );
  GVERIFY( ok );
}

void TestQgsGrassProvider::vectorImport()
{
  reportHeader( u"TestQgsGrassProvider::vectorImport"_s );
  bool ok = true;

  QString tmpGisdbase;
  QString tmpLocation;
  QString tmpMapset;

  if ( !createTmpLocation( tmpGisdbase, tmpLocation, tmpMapset ) )
  {
    reportRow( u"cannot create temporary location"_s );
    GVERIFY( false );
    return;
  }

  QStringList files;
  files << u"points.shp"_s << u"multipoint.shp"_s << u"lines.shp"_s << u"polys.shp"_s;
  files << u"polys_overlapping.shp"_s << u"bug5598.shp"_s;

  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGisdbase, mLocation );
  for ( const QString &file : files )
  {
    QString uri = QStringLiteral( TEST_DATA_DIR ) + "/" + file;
    QString name = QFileInfo( uri ).baseName();
    reportRow( "input vector: " + uri );
    QgsVectorDataProvider *provider = qobject_cast<QgsVectorDataProvider *>( QgsProviderRegistry::instance()->createProvider( u"ogr"_s, uri ) );
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
      reportRow( "import failed: " + import->error() );
      ok = false;
    }
    delete import;

    QStringList layers = QgsGrass::vectorLayers( tmpGisdbase, tmpLocation, tmpMapset, name );
    reportRow( "created layers: " + layers.join( QLatin1Char( ',' ) ) );
  }
  removeRecursively( tmpGisdbase );
  GVERIFY( ok );
}

QList<TestQgsGrassCommandGroup> TestQgsGrassProvider::createCommands()
{
  QList<TestQgsGrassCommandGroup> commandGroups;

  TestQgsGrassCommandGroup commandGroup;
  TestQgsGrassCommand command;
  TestQgsGrassFeature grassFeature;
  QgsLineString *line = nullptr;
  QgsGeometry *geometry = nullptr;
  QVector<QgsPoint> pointList;

  // Start editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values[u"grassLayerCode"_s] = "1_point";
  command.values[u"expectedLayerType"_s] = "Point";
  commandGroup.commands << command;

  // Add point
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  grassFeature = TestQgsGrassFeature( GV_POINT );
  grassFeature.setId( 1 );
  geometry = new QgsGeometry( new QgsPoint( Qgis::WkbType::Point, 10, 10, 0 ) );
  grassFeature.setGeometry( *geometry );
  delete geometry;
  command.grassFeatures << grassFeature;
  command.expectedFeature = grassFeature;
  commandGroup.commands << command;

  // Change geometry
  command = TestQgsGrassCommand( TestQgsGrassCommand::ChangeGeometry );
  command.fid = 1;
  command.geometry = new QgsGeometry( new QgsPoint( Qgis::WkbType::Point, 20, 20, 0 ) );
  commandGroup.commands << command;

  // Add field
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddAttribute );
  command.field = QgsField( u"field_int"_s, QMetaType::Type::Int, u"integer"_s );
  commandGroup.commands << command;

  // Change attribute
  command = TestQgsGrassCommand( TestQgsGrassCommand::ChangeAttributeValue );
  command.fid = 1;
  command.field.setName( u"field_int"_s );
  command.value = 123;
  commandGroup.commands << command;

  // Delete field
  command = TestQgsGrassCommand( TestQgsGrassCommand::DeleteAttribute );
  command.field = QgsField( u"field_int"_s, QMetaType::Type::Int, u"integer"_s );
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
      commands[i].values[u"grassLayerCode"_s] = "2_point";
    }
  }

  commandGroup.commands << commands;

  commandGroups << commandGroup;

  //------------------------ Group 2 (issue #13726) -------------------------------
  commandGroup = TestQgsGrassCommandGroup();

  // Start editing
  command = TestQgsGrassCommand( TestQgsGrassCommand::StartEditing );
  command.values[u"grassLayerCode"_s] = "1_line";
  command.values[u"expectedLayerType"_s] = "LineString";
  commandGroup.commands << command;

  // Add field
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddAttribute );
  command.field = QgsField( u"field_int"_s, QMetaType::Type::Int, u"integer"_s );
  commandGroup.commands << command;

  // Add line feature with attributes
  command = TestQgsGrassCommand( TestQgsGrassCommand::AddFeature );
  grassFeature = TestQgsGrassFeature( GV_LINE );
  grassFeature.setId( 1 );
  line = new QgsLineString();
  pointList.clear();
  pointList << QgsPoint( Qgis::WkbType::Point, 0, 0, 0 );
  pointList << QgsPoint( Qgis::WkbType::Point, 20, 10, 0 );
  line->setPoints( pointList );
  pointList.clear();
  geometry = new QgsGeometry( line );
  grassFeature.setGeometry( *geometry );
  delete geometry;
  command.grassFeatures << grassFeature;
  command.expectedFeature = grassFeature;
  command.attributes[u"field_int"_s] = 456;
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
  grassFeature.setId( 1 );
  line = new QgsLineString();
  pointList.clear();
  pointList << QgsPoint( QgsWkbTypes::Point, 0, 0, 0 );
  pointList << QgsPoint( QgsWkbTypes::Point, 20, 10, 0 );
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

bool TestQgsGrassProvider::setAttributes( QgsFeature &feature, const QMap<QString, QVariant> &attributes )
{
  bool attributesSet = true;
  for ( const QString &fieldName : attributes.keys() )
  {
    int index = feature.fields().indexFromName( fieldName );
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
  reportHeader( u"TestQgsGrassProvider::edit"_s );
  bool ok = true;

  QString tmpGisdbase;
  QString tmpLocation;
  QString tmpMapset;

  if ( !createTmpLocation( tmpGisdbase, tmpLocation, tmpMapset ) )
  {
    reportRow( u"cannot create temporary location"_s );
    GVERIFY( false );
    return;
  }

  QList<TestQgsGrassCommandGroup> commandGroups = createCommands();

  for ( int i = 0; i < commandGroups.size(); i++ )
  {
    TestQgsGrassCommandGroup &commandGroup = commandGroups[i];

    // Create GRASS vector
    QString name = u"edit_%1"_s.arg( i );
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

    QgsVectorLayer *grassLayer = nullptr;
    QgsGrassProvider *grassProvider = nullptr;
    QgsVectorLayer *expectedLayer = nullptr;

    QList<TestQgsGrassCommand> editCommands; // real edit

    for ( const TestQgsGrassCommand &command : commandGroup.commands )
    {
      reportRow( "command: " + command.toString() );
      bool commandOk = true;

      if ( command.command == TestQgsGrassCommand::StartEditing )
      {
        QString grassUri = mapObject.mapsetPath() + "/" + name + "/" + command.values[u"grassLayerCode"_s].toString();
        reportRow( "grassUri: " + grassUri );
        delete grassLayer;
        grassLayer = new QgsVectorLayer( grassUri, name, u"grass"_s );
        if ( !grassLayer->isValid() )
        {
          reportRow( u"grassLayer is not valid"_s );
          commandOk = false;
          break;
        }
        grassProvider = qobject_cast<QgsGrassProvider *>( grassLayer->dataProvider() );
        if ( !grassProvider )
        {
          reportRow( u"cannot get grassProvider"_s );
          commandOk = false;
          break;
        }
        if ( expectedLayers.keys().contains( grassUri ) )
        {
          reportRow( u"expectedLayer exists"_s );
          expectedLayer = expectedLayers.value( grassUri );
        }
        else
        {
          reportRow( u"create new expectedLayer"_s );
          // Create memory vector for verification, it has no fields until added
          expectedLayer = new QgsVectorLayer( command.values[u"expectedLayerType"_s].toString(), u"test"_s, u"memory"_s );
          if ( !expectedLayer->isValid() )
          {
            reportRow( u"expectedLayer is not valid"_s );
            commandOk = false;
            break;
          }
          expectedLayers.insert( grassUri, expectedLayer );
        }

        grassLayer->startEditing();
        grassProvider->startEditing( grassLayer );

        QVERIFY( expectedLayer );
        expectedLayer->startEditing();
      }

      if ( !grassLayer || !grassProvider || !expectedLayer )
      {
        reportRow( u"grassLayer, grassProvider or expectedLayer is null"_s );
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
        for ( TestQgsGrassFeature grassFeature : command.grassFeatures ) // copy feature, not reference
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
            reportRow( u"cannot add feature"_s );
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
            reportRow( u"cannot add expectedFeature"_s );
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
          reportRow( u"cannot delete feature"_s );
          commandOk = false;
        }
        QgsFeatureId expectedFid = commandGroup.expectedFids.value( command.fid );
        if ( !expectedLayer->deleteFeature( expectedFid ) )
        {
          reportRow( u"cannot delete expected feature"_s );
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
        if ( !grassLayer->changeGeometry( fid, *command.geometry ) )
        {
          reportRow( u"cannot change feature geometry"_s );
          commandOk = false;
        }
        QgsFeatureId expectedFid = command.fid;
        if ( commandGroup.expectedFids.contains( expectedFid ) )
        {
          expectedFid = commandGroup.expectedFids.value( expectedFid );
        }
        if ( !expectedLayer->changeGeometry( expectedFid, *command.geometry ) )
        {
          reportRow( u"cannot change expected feature geometry"_s );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::AddAttribute )
      {
        if ( !grassLayer->addAttribute( command.field ) )
        {
          reportRow( u"cannot add field to layer"_s );
          commandOk = false;
        }
        if ( !expectedLayer->addAttribute( command.field ) )
        {
          reportRow( u"cannot add field to expectedLayer"_s );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::DeleteAttribute )
      {
        int index = grassLayer->fields().indexFromName( command.field.name() );
        if ( !grassLayer->deleteAttribute( index ) )
        {
          reportRow( u"cannot delete field from layer"_s );
          commandOk = false;
        }
        int expectedIndex = expectedLayer->fields().indexFromName( command.field.name() );
        if ( !expectedLayer->deleteAttribute( expectedIndex ) )
        {
          reportRow( u"cannot delete field from expected layer"_s );
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
          reportRow( u"cannot change feature attribute"_s );
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
          reportRow( u"cannot change expected feature attribute"_s );
          commandOk = false;
        }
        editCommands << command;
      }
      else if ( command.command == TestQgsGrassCommand::UndoAll )
      {
        if ( grassLayer->undoStack()->count() != editCommands.size() || grassLayer->undoStack()->count() != expectedLayer->undoStack()->count() )
        {
          reportRow( u"Different undo stack size: %1, expected: %2, editCommands: %3"_s
                       .arg( grassLayer->undoStack()->count() )
                       .arg( expectedLayer->undoStack()->count() )
                       .arg( editCommands.size() ) );
          commandOk = false;
        }
        else
        {
          for ( int j = editCommands.size() - 1; j >= 0; j-- )
          {
            reportRow( "undo command: " + editCommands[j].toString() );
            grassLayer->undoStack()->undo();
            expectedLayer->undoStack()->undo();
            if ( !compare( expectedLayers, ok ) )
            {
              reportRow( u"undo failed"_s );
              commandOk = false;
              break;
            }
          }
        }
      }
      else if ( command.command == TestQgsGrassCommand::RedoAll )
      {
        if ( grassLayer->undoStack()->count() != editCommands.size() || grassLayer->undoStack()->count() != expectedLayer->undoStack()->count() )
        {
          reportRow( u"Different undo stack size: %1, expected: %2, editCommands: %3"_s
                       .arg( grassLayer->undoStack()->count() )
                       .arg( expectedLayer->undoStack()->count() )
                       .arg( editCommands.size() ) );
          commandOk = false;
        }
        else
        {
          for ( int j = 0; j < editCommands.size(); j++ )
          {
            reportRow( "redo command: " + editCommands[j].toString() );
            grassLayer->undoStack()->redo();
            expectedLayer->undoStack()->redo();
            if ( !compare( expectedLayers, ok ) )
            {
              reportRow( u"redo failed"_s );
              commandOk = false;
              break;
            }
          }
        }
      }

      if ( !commandOk )
      {
        reportRow( u"command failed"_s );
        ok = false;
        break;
      }

      if ( !command.verify )
      {
        reportRow( u"partial command not verified"_s );
        continue;
      }

      if ( command.command != TestQgsGrassCommand::UndoAll && command.command != TestQgsGrassCommand::RedoAll )
      {
        if ( !compare( expectedLayers, ok ) )
        {
          reportRow( u"command failed"_s );
          break;
        }
      }
    }

    delete grassLayer;
    for ( QgsVectorLayer *layer : expectedLayers.values() )
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
  QgsGeometry expectedGeom = expectedFeature.geometry();
  if ( !feature.geometry().equals( expectedGeom ) )
  {
    return false;
  }
  // GRASS feature has always additional cat field
  QSet<int> indexes;
  for ( int i = 0; i < feature.fields().size(); i++ )
  {
    QString name = feature.fields().at( i ).name();
    if ( name == "cat"_L1 ) // skip cat
    {
      continue;
    }
    indexes << i;
  }
  for ( int i = 0; i < expectedFeature.fields().size(); i++ )
  {
    QString name = expectedFeature.fields().at( i ).name();
    int index = feature.fields().indexFromName( name );
    if ( index < 0 )
    {
      // not found
      QStringList names;
      for ( int j = 0; j < feature.fields().size(); j++ )
      {
        names << feature.fields().at( j ).name();
      }
      reportRow( u"Attribute %1 not found, feature attributes: %2"_s.arg( name, names.join( QLatin1Char( ',' ) ) ) );
      return false;
    }
    indexes.remove( index );
    if ( feature.attribute( index ) != expectedFeature.attribute( i ) )
    {
      reportRow( u"Attribute name %1, value: '%2' does not match expected value: '%3'"_s
                   .arg( name, feature.attribute( index ).toString(), expectedFeature.attribute( i ).toString() ) );
      return false;
    }
  }
  if ( indexes.size() > 0 )
  {
    // unexpected attribute in feature
    QStringList names;
    for ( int i : indexes )
    {
      names << feature.fields().at( i ).name();
    }
    reportRow( u"feature has %1 unexpected attributes: %2"_s.arg( indexes.size() ).arg( names.join( QLatin1Char( ',' ) ) ) );
    return false;
  }
  return true;
}

bool TestQgsGrassProvider::compare( QList<QgsFeature> features, QList<QgsFeature> expectedFeatures, bool &ok )
{
  bool localOk = true;
  if ( features.size() != expectedFeatures.size() )
  {
    reportRow( u"different number of features (%1) and expected features (%2)"_s.arg( features.size() ).arg( expectedFeatures.size() ) );
    ok = false;
    return false;
  }
  // Check if each expected feature exists in features
  for ( const QgsFeature &expectedFeature : expectedFeatures )
  {
    bool found = false;
    for ( const QgsFeature &feature : features )
    {
      if ( equal( feature, expectedFeature ) )
      {
        found = true;
        break;
      }
    }
    if ( !found )
    {
      reportRow( u"expected feature fid = %1 not found in features"_s.arg( expectedFeature.id() ) );
      ok = false;
      localOk = false;
    }
  }
  return localOk;
}

bool TestQgsGrassProvider::compare( QMap<QString, QgsVectorLayer *> layers, bool &ok )
{
  for ( const QString &grassUri : layers.keys() )
  {
    QgsVectorLayer *layer = layers.value( grassUri );
    Q_ASSERT( layer );
    if ( !compare( grassUri, layer, ok ) )
    {
      reportRow( "comparison failed: " + grassUri );
    }
  }
  return ok;
}

bool TestQgsGrassProvider::compare( QString uri, QgsVectorLayer *expectedLayer, bool &ok )
{
  QgsGrassObject mapObject;
  mapObject.setFromUri( uri );
  QList<QgsFeature> expectedFeatures = getFeatures( expectedLayer );

  // read the map using another layer/provider
  QgsVectorLayer *layer = new QgsVectorLayer( uri, u"test"_s, u"grass"_s );
  if ( !layer->isValid() )
  {
    reportRow( u"shared layer is not valid"_s );
    ok = false;
    return false;
  }
  QList<QgsFeature> features = getFeatures( layer );
  delete layer;
  layer = nullptr;

  bool sharedOk = compare( features, expectedFeatures, ok );
  if ( sharedOk )
  {
    //reportRow( "comparison with shared layer OK" );
  }
  else
  {
    reportRow( u"comparison with shared layer failed"_s );
  }

  // We cannot test attribute table changes with independent layer in GRASS 6, which is using
  // DBF files, which are written when driver is closed (map layer keeps driver open during editing)
  bool independentOk = true;
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


    Vect_build( map );

    //Vect_set_release_support( map );
    Vect_close( map );
    QgsGrass::vectDestroyMapStruct( map );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    reportRow( "Cannot build topology: " + QString( e.what() ) );
    ok = false;
    return false;
  }

  QgsGrassVectorMapStore *mapStore = new QgsGrassVectorMapStore();
  QgsGrassVectorMapStore::setStore( mapStore );

  layer = new QgsVectorLayer( uri, u"test"_s, u"grass"_s );
  if ( !layer->isValid() )
  {
    reportRow( u"independent layer is not valid"_s );
    ok = false;
    return false;
  }
  features = getFeatures( layer );
  delete layer;
  QgsGrassVectorMapStore::setStore( nullptr );
  delete mapStore;

  independentOk = compare( features, expectedFeatures, ok );
  if ( independentOk )
  {
    //reportRow( "comparison with independent layer OK" );
  }
  else
  {
    reportRow( u"comparison with independent layer failed"_s );
  }

  return sharedOk && independentOk;
}

QGSTEST_MAIN( TestQgsGrassProvider )
#include "testqgsgrassprovider.moc"
