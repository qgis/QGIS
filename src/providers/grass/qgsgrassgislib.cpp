/***************************************************************************
    qgsgrassgislib.cpp  -  Fake GRASS gis lib
                             -------------------
    begin                : Nov 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <qmath.h>
#include <QtGlobal>

// If qgsgrassgislibfunctions.h is included on Linux, symbols defined here
// cannot be found (undefined symbol error) even if they are present in
// the library (in code section) - why?
#ifdef Q_OS_WIN
#include "qgsgrassgislibfunctions.h"
extern "C"
{
// defined here because too complex for parser in CMakeLists.txt
  int GRASS_LIB_EXPORT G_cell_stats_histo_eq( struct Cell_stats *statf, CELL min1, CELL max1, CELL min2, CELL max2, int zero, void ( *func )( CELL, CELL, CELL ) );
}
#endif
#include "qgsgrassgislib.h"

#include "qgis.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsgeometry.h"
#include "qgsrectangle.h"
#include "qgsconfig.h"

#include <QByteArray>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QTemporaryFile>
#include <QHash>

#include <QTextCodec>

#if 0
extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/gprojects.h>
#include <grass/Vect.h>
#include <grass/version.h>
}
#endif

#if !defined(GRASS_VERSION_MAJOR) || \
    !defined(GRASS_VERSION_MINOR) || \
    GRASS_VERSION_MAJOR<6 || \
    (GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR <= 2)
#define G__setenv(name,value) G__setenv( ( char * ) (name), (char *) (value) )
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#endif


QgsGrassGisLib *QgsGrassGisLib::_instance = 0;

QgsGrassGisLib GRASS_LIB_EXPORT *QgsGrassGisLib::instance( )
{
  if ( _instance == 0 )
  {
    _instance = new QgsGrassGisLib();
  }
  return _instance;
}

QgsGrassGisLib::QgsGrassGisLib()
{
  // Load original GRASS library

  // GRASS_LIBRARY_GIS (cmake GRASS_LIBRARY_gis) is a path to the GRASS library
  // in the time of compilation, it may be used on runtime (it is the same)
  // on Linux and Mac but on Windows with OSGEO4W the GRASS may be installed
  // in a different directory. qgis.env however calls GRASS etc/env.bat
  // which sets GISBASE and GRASS_LIBRARY_GIS on Windows is path to .lib, e.g
  // grass_gis.lib.  Name of the DLL on Windows is e.g.: libgrass_gis.6.4.3RC1.dll

  QString gisBase = getenv( "GISBASE" );
#ifdef Q_OS_WIN
  if ( gisBase.isEmpty() )
  {
    fatal( "GISBASE environment variable not set" );
  }
  QString libPath = gisBase + "\\lib\\libgrass_gis." + QString( GRASS_VERSION ) + ".dll";
#else
  QString libPath = QString( GRASS_LIBRARY_GIS );
  // Prefere GISBASE if set
  if ( !gisBase.isEmpty() )
  {
    libPath = gisBase + "/lib/" + QFileInfo( libPath ).fileName();
  }
#endif
  QgsDebugMsg( "libPath = " + libPath );
  mLibrary.setFileName( libPath );
  if ( !mLibrary.load() )
  {
    fatal( "Cannot load true GRASS library, path: " + libPath );
    return;
  }
}

int GRASS_LIB_EXPORT QgsGrassGisLib::errorRoutine( const char *msg, int fatal )
{
  Q_UNUSED( fatal );
  QgsDebugMsg( QString( "error_routine (fatal = %1): %2" ).arg( fatal ).arg( msg ) );
  // qFatal does core dump, useful for backtrace
  if ( fatal )
  {
    qFatal( "Fatal: %s", msg );
  }
  else
  {
    qWarning( "Warning: %s", msg );
  }
  return 1;
}

void QgsGrassGisLib::fatal( QString msg )
{
  QgsLogger::fatal( msg );  // calls qFatal which does core dump
}

void QgsGrassGisLib::warning( QString msg )
{
  QgsLogger::warning( msg );
}

void * QgsGrassGisLib::resolve( const char * symbol )
{
  QgsDebugMsgLevel( QString( "symbol = %1" ).arg( symbol ), 5 );
  void * fn = mLibrary.resolve( symbol );
  if ( !fn )
  {
    fatal( "Cannot resolve symbol " + QString( symbol ) );
  }
  return fn;
}

int GRASS_LIB_EXPORT QgsGrassGisLib::G__gisinit( const char * version, const char * programName )
{
  Q_UNUSED( version );
  // We use this function also to init our fake lib
  QgsDebugMsg( QString( "version = %1 programName = %2" ).arg( version ).arg( programName ) );

  // Init providers path
  int argc = 1;
  char **argv = new char*[1];
  argv[0] = qstrdup( programName );


  // unfortunately it seems impossible to get QGIS prefix
  // QCoreApplication::applicationDirPath() returns $GISBASE/lib on Linux
#if 0
  QDir dir( QCoreApplication::applicationDirPath() );
  dir.cdUp();
  QString prefixPath = dir.absolutePath();
#endif

  //QCoreApplication app( argc, argv ); // to init paths
  QgsApplication app( argc, argv, false ); // to init paths

  // TODO: WCS (network) fails with: "QTimer can only be used with threads started
  // with QThread" because QCoreApplication::exec() was not called, but
  // QCoreApplication::exec() goes to loop. We need to start QThread somehow.

  // QGIS_PREFIX_PATH should be loaded by QgsApplication
  //QString prefixPath = getenv( "QGIS_PREFIX_PATH" );
  //if ( prefixPath.isEmpty() )
  //{
  //  fatal( "Cannot get QGIS_PREFIX_PATH" );
  //}
  //QgsApplication::setPrefixPath( prefixPath, true );

  QgsDebugMsg( "Plugin path: " + QgsApplication::pluginPath() );
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );

  QgsDebugMsg( "qgisSettingsDirPath = " + app.qgisSettingsDirPath() );

  G_set_error_routine( &errorRoutine );
  G_set_gisrc_mode( G_GISRC_MODE_MEMORY );
  G_setenv( "OVERWRITE", "1" );  // avoid checking if map exists

  G_suppress_masking();

#if GRASS_VERSION_MAJOR<6 || (GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR <= 4)
  G__init_null_patterns();
#endif

  // Read projection if set
  //mCrs.createFromOgcWmsCrs( "EPSG:900913" );
  QString crsStr = getenv( "QGIS_GRASS_CRS" );

  QgsDebugMsg( "Setting CRS to " + crsStr );

  if ( !crsStr.isEmpty() )
  {
    if ( !mCrs.createFromProj4( crsStr ) )
    {
      fatal( "Cannot create CRS from QGIS_GRASS_CRS: " + crsStr );
    }
  }
  mDistanceArea.setSourceCrs( mCrs.srsid() );

  // Read region fron environment variable
  // QGIS_GRASS_REGION=west,south,east,north,cols,rows
#if 0
  QString regionStr = getenv( "QGIS_GRASS_REGION" );
  QStringList regionList = regionStr.split( "," );
  if ( regionList.size() != 6 )
  {
    fatal( "Cannot read region from QGIS_GRASS_REGION environment variable" );
  }

  double xMin, yMin, xMax, yMax;
  int cols, rows;
  bool xMinOk, yMinOk, xMaxOk, yMaxOk, colsOk, rowsOk;
  xMin = regionList.value( 0 ).toDouble( &xMinOk );
  yMin = regionList.value( 1 ).toDouble( &yMinOk );
  xMax = regionList.value( 2 ).toDouble( &xMaxOk );
  yMax = regionList.value( 3 ).toDouble( &yMaxOk );
  cols = regionList.value( 4 ).toInt( &colsOk );
  rows = regionList.value( 5 ).toInt( &rowsOk );

  if ( !xMinOk || !yMinOk || !xMaxOk || !yMaxOk || !colsOk || !rowsOk )
  {
    fatal( "Cannot parse QGIS_GRASS_REGION" );
  }

  struct Cell_head window;
  window.west = xMin;
  window.south = yMin;
  window.east = xMax;
  window.north = yMax;
  window.rows = rows;
  window.cols = cols;

  char* err = G_adjust_Cell_head( &window, 1, 1 );
  if ( err )
  {
    fatal( QString( err ) );
  }
  G_set_window( &window );
#endif

  QString regionStr = getenv( "GRASS_REGION" );
  if ( regionStr.isEmpty() )
  {
    fatal( "GRASS_REGION environment variable not set" );
  }

  QgsDebugMsg( "Getting region via true lib from GRASS_REGION: " +  regionStr );
  // GRASS true lib reads GRASS_REGION environment variable
  G_get_window( &mWindow );

  mExtent = QgsRectangle( mWindow.west, mWindow.south, mWindow.east, mWindow.north );
  mRows = mWindow.rows;
  mColumns = mWindow.cols;
  mXRes = mExtent.width() / mColumns;
  mYRes = mExtent.height() / mColumns;

  QgsDebugMsg( "End" );
  return 0;
}

int GRASS_LIB_EXPORT G__gisinit( const char * version, const char * programName )
{
  return QgsGrassGisLib::instance()->G__gisinit( version, programName );
}

typedef int G_parser_type( int argc, char **argv );
int GRASS_LIB_EXPORT G_parser( int argc, char **argv )
{
  QgsDebugMsg( "Entered" );
  G_parser_type* fn = ( G_parser_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_parser" ) );
  int ret = fn( argc, argv );

  if ( ret == 0 ) // parsed OK
  {
    // It would be useful to determin region from input raster layers if no one
    // is given by environment variable but there seems to be no way to get
    // access to module options. Everything is in static variables in parser.c
    // and there are no access functions to them.
  }
  return ret;
}

// Defined here just because parser in cmake does not recognize this kind of params
typedef int G_set_error_routine_type( int ( * )( const char *, int ) );
int GRASS_LIB_EXPORT G_set_error_routine( int ( *error_routine )( const char *, int ) )
{
  //QgsDebugMsg( "Entered" );
  G_set_error_routine_type* fn = ( G_set_error_routine_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_set_error_routine" ) );
  return fn( error_routine );
}

typedef int G_warning_type( const char *, ... );
int GRASS_LIB_EXPORT G_warning( const char * msg, ... )
{
  QgsDebugMsg( "Entered" );
  G_warning_type* fn = ( G_warning_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_warning" ) );
  va_list ap;
  va_start( ap, msg );
  int ret = fn( msg, ap );
  va_end( ap );
  return ret;
}

typedef void G_important_message_type( const char *msg, ... );
void GRASS_LIB_EXPORT G_important_message( const char * msg, ... )
{
  G_important_message_type* fn = ( G_important_message_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_important_message" ) );
  va_list ap;
  va_start( ap, msg );
  fn( msg, ap );
  va_end( ap );
}

// G_fatal_error is declared in gisdefs.h as int but noreturn
//typedef int G_fatal_error_type( const char *, ... );
int GRASS_LIB_EXPORT G_fatal_error( const char * msg, ... )
{
  QgsDebugMsg( "Entered" );
  //G_fatal_error_type* fn = ( G_fatal_error_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_fatal_error" ) );
  va_list ap;
  va_start( ap, msg );
  //fn( msg, ap );
  char buffer[2000];
  vsprintf( buffer, msg, ap );
  va_end( ap );

  // qFatal does core dump, useful for backtrace
  qFatal( "Fatal error: %s", buffer );
  exit( 1 ); // must exit to avoid compilation warning
}

typedef int G_snprintf_type( char *, size_t, const char *, ... );
int GRASS_LIB_EXPORT G_snprintf( char *str, size_t size, const char *fmt, ... )
{
  G_snprintf_type* fn = ( G_snprintf_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_snprintf" ) );
  va_list ap;
  va_start( ap, fmt );
  int ret = fn( str, size, fmt, ap );
  va_end( ap );
  return ret;
}

int GRASS_LIB_EXPORT G_done_msg( const char *msg, ... )
{
  Q_UNUSED( msg );
  // TODO
  return 0;
}

char GRASS_LIB_EXPORT *QgsGrassGisLib::G_find_cell2( const char * name, const char * mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  QgsDebugMsg( "name = " + QString( name ) );

  // G_find_cell2 is used by some modules to test if output exists (r.basins.fill)
  // and exits with fatal error if exists -> we must test existence here
  Raster rast = raster( name );
  if ( !rast.provider || !rast.provider->isValid() )
  {
    return 0;
  }
  QString ms = "qgis";
  return qstrdup( ms.toAscii() );  // memory lost
}

char GRASS_LIB_EXPORT *G__file_name( char *path, const char *element, const char *name, const char *mapset )
{
  Q_UNUSED( path );
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char *G__file_name_misc( char *path, const char *dir, const char *element, const char *name, const char *mapset )
{
  Q_UNUSED( path );
  Q_UNUSED( dir );
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char GRASS_LIB_EXPORT *G_find_cell2( const char* name, const char *mapset )
{
  return QgsGrassGisLib::instance()->G_find_cell2( name, mapset );
}

char GRASS_LIB_EXPORT *G_find_cell( char * name, const char * mapset )
{
  // Not really sure about differences between G_find_cell and G_find_cell2
  return G_find_cell2( name, mapset );
}

char GRASS_LIB_EXPORT *G_find_file( const char *element, char *name, const char *mapset )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char GRASS_LIB_EXPORT *G_find_file2( const char *element, const char *name, const char *mapset )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char GRASS_LIB_EXPORT *G_find_file_misc( const char *dir, const char *element, char *name, const char *mapset )
{
  Q_UNUSED( dir );
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char GRASS_LIB_EXPORT *G_find_file2_misc( const char *dir, const char *element, const char *name, const char *mapset )
{
  Q_UNUSED( dir );
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

QgsGrassGisLib::Raster QgsGrassGisLib::raster( QString name )
{
  QgsDebugMsg( "name = " + name );

  foreach ( Raster raster, mRasters )
  {
    if ( raster.name == name ) return raster;
  }

  QString providerKey;
  QString dataSource;
  int band = 1;

  if ( name.contains( "provider=" ) ) // encoded uri
  {
    QgsDataSourceURI uri;
    uri.setEncodedUri( name.toLocal8Bit() );
    if ( uri.hasParam( "band" ) )
    {
      band = uri.param( "band" ).toInt();
    }
    providerKey = uri.param( "provider" );
    if ( providerKey == "gdal" )
    {
      dataSource = uri.param( "path" );
    }
    else
    {
      uri.removeParam( "band" );
      uri.removeParam( "provider" );
      dataSource = uri.encodedUri();
    }
  }
  else // simple GDAL path
  {
    providerKey = "gdal";
    dataSource = name;
    band = 1;
  }
  QgsDebugMsg( "providerKey = " + providerKey );
  QgsDebugMsg( "dataSource = " + dataSource );
  QgsDebugMsg( QString( "band = %1" ).arg( band ) );

  Raster raster;
  raster.name = name;
  raster.band = band;
  raster.provider = ( QgsRasterDataProvider* )QgsProviderRegistry::instance()->provider( providerKey, dataSource );
  if ( !raster.provider || !raster.provider->isValid() )
  {
    // No fatal, it may be used to test file existence
    //fatal( "Cannot load raster provider with data source: " + dataSource );
  }
  else
  {
    raster.input = raster.provider;

    if ( band < 1 || band > raster.provider->bandCount() )
    {
      fatal( "Band out of range" );
    }

    QgsDebugMsg( QString( "mCrs valid = %1 = %2" ).arg( mCrs.isValid() ).arg( mCrs.toProj4() ) );
    QgsDebugMsg( QString( "crs valid = %1 = %2" ).arg( raster.provider->crs().isValid() ).arg( raster.provider->crs().toProj4() ) );
    if ( mCrs.isValid() )
    {
      // GDAL provider loads data without CRS as EPSG:4326!!! Verify, it should give
      // invalid CRS instead.
      if ( !raster.provider->crs().isValid() )
      {
        fatal( "Output CRS specified but input CRS is unknown" );
      }
      if ( mCrs != raster.provider->crs() )
      {
        raster.projector = new QgsRasterProjector();
        raster.projector->setCRS( raster.provider->crs(), mCrs );
        raster.projector->setInput( raster.provider );
        raster.input = raster.projector;
      }
    }
  }

  raster.fd = mRasters.size();
  mRasters.insert( raster.fd, raster );
  return raster;
}

int QgsGrassGisLib::G_open_cell_old( const char *name, const char *mapset )
{
  Q_UNUSED( mapset );

  Raster rast = raster( name );
  return rast.fd;
}

int GRASS_LIB_EXPORT G_open_cell_old( const char *name, const char *mapset )
{
  return QgsGrassGisLib::instance()->G_open_cell_old( name, mapset );
}

int QgsGrassGisLib::G_close_cell( int fd )
{
  Raster rast = mRasters.value( fd );
  delete rast.provider;
  delete rast.projector;
  mRasters.remove( fd );
  return 1;
}

int GRASS_LIB_EXPORT G_close_cell( int fd )
{
  return QgsGrassGisLib::instance()->G_close_cell( fd );
}

int G_unopen_cell( int fd )
{
  // TODO: delete created raster
  QgsGrassGisLib::instance()->G_close_cell( fd );
  return 1; // OK
}

int QgsGrassGisLib::G_open_raster_new( const char *name, RASTER_MAP_TYPE wr_type )
{
  Q_UNUSED( wr_type );
  QString providerKey = "gdal";
  QString dataSource = name;

  Raster raster;
  raster.name = name;
  //raster.writer = new QgsRasterFileWriter( dataSource );

  QString outputFormat = "GTiff";
  int nBands = 1;
  QGis::DataType type = qgisRasterType( wr_type );
  QgsDebugMsg( QString( "type = %1" ).arg( type ) );
  double geoTransform[6];
  geoTransform[0] = mExtent.xMinimum();
  geoTransform[1] = mExtent.width() / mColumns;
  geoTransform[2] = 0.0;
  geoTransform[3] = mExtent.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -1. * mExtent.height() / mRows;

  raster.provider = QgsRasterDataProvider::create( providerKey, dataSource, outputFormat, nBands, type, mColumns, mRows, geoTransform, mCrs );
  if ( !raster.provider || !raster.provider->isValid() )
  {
    if ( raster.provider ) delete raster.provider;
    fatal( "Cannot create output data source: " + dataSource );
  }

  raster.band = 1;
  raster.noDataValue = noDataValueForGrassType( wr_type );
  QgsDebugMsg( QString( "noDataValue = %1" ).arg(( int )raster.noDataValue ) );
  raster.provider->setNoDataValue( raster.band, raster.noDataValue );

  raster.fd = mRasters.size();
  mRasters.insert( raster.fd, raster );
  return raster.fd;
}

int GRASS_LIB_EXPORT G_open_raster_new( const char *name, RASTER_MAP_TYPE wr_type )
{
  return QgsGrassGisLib::instance()->G_open_raster_new( name, wr_type );
}

int GRASS_LIB_EXPORT G_open_cell_new( const char *name )
{
  return QgsGrassGisLib::instance()->G_open_raster_new( name, CELL_TYPE );
}

int GRASS_LIB_EXPORT G_open_fp_cell_new( const char *name )
{
  return QgsGrassGisLib::instance()->G_open_raster_new( name, FCELL_TYPE );
}

RASTER_MAP_TYPE QgsGrassGisLib::G_raster_map_type( const char *name, const char *mapset )
{
  Q_UNUSED( mapset );
  Raster rast = raster( name );

  return grassRasterType( rast.provider->dataType( rast.band ) );
}

RASTER_MAP_TYPE GRASS_LIB_EXPORT G_raster_map_type( const char *name, const char *mapset )
{
  return QgsGrassGisLib::instance()->G_raster_map_type( name, mapset );
}

RASTER_MAP_TYPE QgsGrassGisLib::G_get_raster_map_type( int fd )
{
  Raster rast = mRasters.value( fd );

  return grassRasterType( rast.provider->dataType( rast.band ) );
}

RASTER_MAP_TYPE GRASS_LIB_EXPORT G_get_raster_map_type( int fd )
{
  return QgsGrassGisLib::instance()->G_get_raster_map_type( fd );
}

int GRASS_LIB_EXPORT G_raster_map_is_fp( const char *name, const char *mapset )
{
  RASTER_MAP_TYPE type = QgsGrassGisLib::instance()->G_raster_map_type( name, mapset );
  if ( type == FCELL_TYPE || type == DCELL_TYPE ) return 1;
  return 0;
}

int QgsGrassGisLib::G_read_fp_range( const char *name, const char *mapset, struct FPRange *drange )
{
  Q_UNUSED( mapset );
  Raster rast = raster( name );

  // TODO (no solution): Problem: GRASS has precise min/max values available,
  // in QGIS we can calculate, but it would be slow, so we are using estimated
  // values, which may result in wrong output
  // Hopefully the range is not crutial for most modules, but it is problem certanly
  // for r.rescale .. more?

  // TODO: estimate only for  large rasters
  warning( "The module needs input raster values range, estimated values used." );

  int sampleSize = 250000;
  QgsRasterBandStats stats = rast.provider->bandStatistics( rast.band, QgsRasterBandStats::Min | QgsRasterBandStats::Max, rast.provider->extent(), sampleSize );

  G_init_fp_range( drange );
  // Attention: r.stats prints wrong results if range is wrong
  G_update_fp_range( stats.minimumValue, drange );
  G_update_fp_range( stats.maximumValue, drange );

  return 1;
}

int GRASS_LIB_EXPORT G_read_fp_range( const char *name, const char *mapset, struct FPRange *range )
{
  return QgsGrassGisLib::instance()->G_read_fp_range( name, mapset, range );
}

int GRASS_LIB_EXPORT G_read_range( const char *name, const char *mapset, struct Range *range )
{
  struct FPRange drange;
  QgsGrassGisLib::instance()->G_read_fp_range( name, mapset, &drange );
  G_init_range( range );
  G_update_range(( CELL ) floor( drange.min ), range );
  G_update_range(( CELL ) ceil( drange.max ), range );
  return 1;
}

int GRASS_LIB_EXPORT G_debug( int level, const char *msg, ... )
{
  Q_UNUSED( level );
  va_list ap;
  va_start( ap, msg );
  QString message = QString().vsprintf( msg, ap );
  va_end( ap );
  QgsDebugMsg( message );
  return 1;
}

void GRASS_LIB_EXPORT G_message( const char *msg, ... )
{
  va_list ap;
  va_start( ap, msg );
  QString message = QString().vsprintf( msg, ap );
  va_end( ap );
  QgsDebugMsg( message );
}

void GRASS_LIB_EXPORT G_verbose_message( const char *msg, ... )
{
  va_list ap;
  va_start( ap, msg );
  QString message = QString().vsprintf( msg, ap );
  va_end( ap );
  QgsDebugMsg( message );
}

typedef int G_spawn_type( const char *command, ... );
int GRASS_LIB_EXPORT G_spawn( const char *command, ... )
{
  va_list ap;
  G_spawn_type* fn = ( G_spawn_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_spawn" ) );
  va_start( ap, command );
  int ret = fn( command, ap );
  va_end( ap );
  return ret;
}

typedef int G_spawn_ex_type( const char *command, ... );
int GRASS_LIB_EXPORT G_spawn_ex( const char *command, ... )
{
  va_list ap;
  G_spawn_ex_type* fn = ( G_spawn_ex_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_spawn_ex" ) );
  va_start( ap, command );
  int ret = fn( command, ap );
  va_end( ap );
  return ret;
}

int GRASS_LIB_EXPORT G_set_quant_rules( int fd, struct Quant *q )
{
  Q_UNUSED( fd );
  Q_UNUSED( q );
  return 0;
}

int QgsGrassGisLib::readRasterRow( int fd, void * buf, int row, RASTER_MAP_TYPE data_type, bool noDataAsZero )
{
  if ( row < 0 || row >= mRows )
  {
    QgsDebugMsg( QString( "row %1 out of range 0 - %2" ).arg( row ).arg( mRows ) );
    return 0;
  }

  // TODO: use cached block with more rows
  Raster raster = mRasters.value( fd );
  //if ( !raster.provider ) return -1;
  if ( !raster.input ) return -1;

  // Create extent for current row
  QgsRectangle blockRect = mExtent;
  double yRes = mExtent.height() / mRows;
  double yMax = mExtent.yMaximum() - yRes * row;
  //QgsDebugMsg( QString( "height = %1 mRows = %2" ).arg( mExtent.height() ).arg( mRows ) );
  //QgsDebugMsg( QString( "row = %1 yRes = %2 yRes * row = %3" ).arg( row ).arg( yRes ).arg( yRes * row ) );
  //QgsDebugMsg( QString( "mExtent.yMaximum() = %1 yMax = %2" ).arg( mExtent.yMaximum() ).arg( yMax ) );
  blockRect.setYMaximum( yMax );
  blockRect.setYMinimum( yMax - yRes );

  QgsRasterBlock *block = raster.input->block( raster.band, blockRect, mColumns, 1 );
  if ( !block ) return -1;

  QGis::DataType requestedType = qgisRasterType( data_type );

  //QgsDebugMsg( QString("data_type = %1").arg(data_type) );
  //QgsDebugMsg( QString("requestedType = %1").arg(requestedType) );
  //QgsDebugMsg( QString("requestedType size = %1").arg( QgsRasterBlock::typeSize( requestedType ) ) );
  //QgsDebugMsg( QString("block->dataType = %1").arg( block->dataType() ) );

  block->convert( requestedType );

  memcpy( buf, block->bits( 0 ), QgsRasterBlock::typeSize( requestedType ) * mColumns );

  for ( int i = 0; i < mColumns; i++ )
  {
    QgsDebugMsgLevel( QString( "row = %1 i = %2 val = %3 isNoData = %4" ).arg( row ).arg( i ).arg( block->value( i ) ).arg( block->isNoData( i ) ), 5 );
    //(( CELL * ) buf )[i] = i;
    if ( block->isNoData( 0, i ) )
    {
      if ( noDataAsZero )
      {
        switch ( data_type )
        {
          case CELL_TYPE:
            G_zero(( char * ) &(( CELL * ) buf )[i], G_raster_size( data_type ) );
            break;
          case FCELL_TYPE:
            G_zero(( char * ) &(( FCELL * ) buf )[i], G_raster_size( data_type ) );
            break;
          case DCELL_TYPE:
            G_zero(( char * ) &(( DCELL * ) buf )[i], G_raster_size( data_type ) );
            break;
          default:
            break;
        }
      }
      else
      {
        switch ( data_type )
        {
          case CELL_TYPE:
            G_set_c_null_value( &(( CELL * ) buf )[i], 1 );
            break;
          case FCELL_TYPE:
            G_set_f_null_value( &(( FCELL * ) buf )[i], 1 );
            break;
          case DCELL_TYPE:
            G_set_d_null_value( &(( DCELL * ) buf )[i], 1 );
            break;
          default:
            break;
        }
      }
    }
    //else
    //{
    //memcpy( &( buf[i] ), block->bits( 0, i ), 4 );
    //buf[i] = (int)  block->value( 0, i);
    //QgsDebugMsg( QString("buf[i] = %1").arg(buf[i]));
    //}
  }
  delete block;
  return 1;

}

int GRASS_LIB_EXPORT G_get_raster_row( int fd, void * buf, int row, RASTER_MAP_TYPE data_type )
{
  bool noDataAsZero = false;
  return QgsGrassGisLib::instance()->readRasterRow( fd, buf, row, data_type, noDataAsZero );
}

int GRASS_LIB_EXPORT G_get_raster_row_nomask( int fd, void * buf, int row, RASTER_MAP_TYPE data_type )
{
  return G_get_raster_row( fd, buf, row, data_type );
}

int GRASS_LIB_EXPORT G_get_c_raster_row( int fd, CELL * buf, int row )
{
  return G_get_raster_row( fd, ( void* )buf, row, CELL_TYPE );
}

int GRASS_LIB_EXPORT G_get_c_raster_row_nomask( int fd, CELL * buf, int row )
{
  return G_get_raster_row_nomask( fd, buf, row, CELL_TYPE );
}

int GRASS_LIB_EXPORT G_get_f_raster_row( int fd, FCELL * buf, int row )
{
  return G_get_raster_row( fd, ( void* )buf, row, FCELL_TYPE );
}

int GRASS_LIB_EXPORT G_get_f_raster_row_nomask( int fd, FCELL * buf, int row )
{
  return G_get_raster_row_nomask( fd, ( void* )buf, row, FCELL_TYPE );
}

int GRASS_LIB_EXPORT G_get_d_raster_row( int fd, DCELL * buf, int row )
{
  return G_get_raster_row( fd, ( void* )buf, row, DCELL_TYPE );
}

int GRASS_LIB_EXPORT G_get_d_raster_row_nomask( int fd, DCELL * buf, int row )
{
  return G_get_raster_row_nomask( fd, ( void* )buf, row, DCELL_TYPE );
}

// reads null as zero
int GRASS_LIB_EXPORT G_get_map_row( int fd, CELL * buf, int row )
{
  bool noDataAsZero = true;
  return QgsGrassGisLib::instance()->readRasterRow( fd, ( void* )buf, row, CELL_TYPE, noDataAsZero );
}

int GRASS_LIB_EXPORT G_get_map_row_nomask( int fd, CELL * buf, int row )
{
  return G_get_map_row( fd, buf, row );
}

int QgsGrassGisLib::G_get_null_value_row( int fd, char *flags, int row )
{
  FCELL *buf = G_allocate_f_raster_buf();
  QgsGrassGisLib::instance()->readRasterRow( fd, buf, row, FCELL_TYPE, false );

  for ( int i = 0; i < mColumns; i++ )
  {
    flags[i] = G_is_f_null_value( &buf[i] ) ? 1 : 0;
  }
  G_free( buf );
  return 1;
}

int GRASS_LIB_EXPORT G_get_null_value_row( int fd, char *flags, int row )
{
  return QgsGrassGisLib::instance()->G_get_null_value_row( fd, flags, row );
}

int QgsGrassGisLib::putRasterRow( int fd, const void *buf, RASTER_MAP_TYPE data_type )
{
  Raster rast = mRasters.value( fd );
  if ( rast.row < 0 || rast.row >= mRows )
  {
    QgsDebugMsg( QString( "row %1 out of range 0 - %2" ).arg( rast.row ).arg( mRows ) );
    return -1;
  }

  QGis::DataType inputType = qgisRasterType( data_type );
  //QgsDebugMsg( QString("data_type = %1").arg(data_type) );
  //QgsDebugMsg( QString("inputType = %1").arg(inputType) );
  //QgsDebugMsg( QString("provider->dataType = %1").arg( rast.provider->dataType( rast.band ) ) );

  //double noDataValue = rast.provider->noDataValue( rast.band );
  QgsRasterBlock block( inputType, mColumns, 1, rast.noDataValue );

  memcpy( block.bits( 0 ), buf, QgsRasterBlock::typeSize( inputType )*mColumns );
  block.convert( rast.provider->dataType( rast.band ) );

  // Set no data after converting to output type
  for ( int i = 0; i < mColumns; i++ )
  {
    bool isNoData = false;
    switch ( data_type )
    {
      case CELL_TYPE:
        isNoData = G_is_c_null_value( &(( CELL * ) buf )[i] ) != 0;
        break;
      case FCELL_TYPE:
        isNoData = G_is_f_null_value( &(( FCELL * ) buf )[i] ) != 0;
        break;
      case DCELL_TYPE:
        isNoData = G_is_d_null_value( &(( DCELL * ) buf )[i] ) != 0;
        break;
      default:
        break;
    }
    if ( isNoData )
    {
      block.setIsNoData( i );
    }
  }

  if ( !rast.provider->write( block.bits( 0 ), rast.band, mColumns, 1, 0, rast.row ) )
  {
    fatal( "Cannot write block" );
  }
  mRasters[fd].row++;

  return 1;
}

int GRASS_LIB_EXPORT G_put_raster_row( int fd, const void *buf, RASTER_MAP_TYPE data_type )
{
  return QgsGrassGisLib::instance()->putRasterRow( fd, buf, data_type );
}

int GRASS_LIB_EXPORT G_put_c_raster_row( int fd, const CELL * buf )
{
  return QgsGrassGisLib::instance()->putRasterRow( fd, buf, CELL_TYPE );
}

int GRASS_LIB_EXPORT G_put_f_raster_row( int fd, const FCELL * buf )
{
  return QgsGrassGisLib::instance()->putRasterRow( fd, buf, FCELL_TYPE );
}

int GRASS_LIB_EXPORT G_put_d_raster_row( int fd, const DCELL * buf )
{
  return QgsGrassGisLib::instance()->putRasterRow( fd, buf, DCELL_TYPE );
}

int GRASS_LIB_EXPORT G_check_input_output_name( const char *input, const char *output, int error )
{
  Q_UNUSED( input );
  Q_UNUSED( output );
  Q_UNUSED( error );
  return 0;
}

void QgsGrassGisLib::initCellHead( struct Cell_head *cellhd )
{
  cellhd->format = 0;
  cellhd->rows = 0;
  cellhd->rows3 = 0;
  cellhd->cols = 0;
  cellhd->cols3 = 0;
  cellhd->depths = 1;
  cellhd->proj = -1;
  cellhd->zone = -1;
  cellhd->compressed = -1;
  cellhd->ew_res = 0.0;
  cellhd->ew_res3 = 1.0;
  cellhd->ns_res = 0.0;
  cellhd->ns_res3 = 1.0;
  cellhd->tb_res = 1.0;
  cellhd->north = 0.0;
  cellhd->south = 0.0;
  cellhd->east = 0.0;
  cellhd->west = 0.0;
  cellhd->top = 1.0;
  cellhd->bottom = 0.0;
}

int QgsGrassGisLib::G_get_cellhd( const char *name, const char *mapset, struct Cell_head *cellhd )
{
  Q_UNUSED( mapset );
  initCellHead( cellhd );
  Raster rast = raster( name );

  QgsRasterDataProvider *provider = rast.provider;

  cellhd->rows = provider->ySize();
  cellhd->cols = provider->xSize();
  cellhd->ew_res = provider->extent().width() / provider->xSize();
  cellhd->ns_res = provider->extent().height() / provider->ySize();
  cellhd->north = provider->extent().yMaximum();
  cellhd->south = provider->extent().yMinimum();
  cellhd->east = provider->extent().yMaximum();
  cellhd->west = provider->extent().xMinimum();

  return 0;
}

int GRASS_LIB_EXPORT G_get_cellhd( const char *name, const char *mapset, struct Cell_head *cellhd )
{
  return QgsGrassGisLib::instance()->G_get_cellhd( name, mapset, cellhd );
}


double QgsGrassGisLib::G_database_units_to_meters_factor( void )
{
  switch ( mCrs.mapUnits() )
  {
    case QGis::Meters:
      return 1.;
    case QGis::Feet:
      return .3048;
    case QGis::Degrees:
      return 1.;
    default:
      return 0.;
  }
}

double QgsGrassGisLib::G_area_of_cell_at_row( int row )
{
  double yMax = mExtent.yMaximum() - row * mYRes;
  double yMin = yMax - mYRes;
  QgsRectangle rect( mExtent.xMinimum(), yMin, mExtent.xMinimum() + mXRes, yMax );
  QgsGeometry* geo = QgsGeometry::fromRect( rect );
  double area = mDistanceArea.measure( geo );
  delete geo;
  if ( !mCrs.geographicFlag() )
  {
    area *= qPow( G_database_units_to_meters_factor(), 2 );
  }
  return area;
}

double GRASS_LIB_EXPORT G_area_of_cell_at_row( int row )
{
  return QgsGrassGisLib::instance()->G_area_of_cell_at_row( row );
}

double QgsGrassGisLib::G_area_of_polygon( const double *x, const double *y, int n )
{
  QgsPolyline polyline;
  for ( int i = 0; i < n; i++ )
  {
    polyline.append( QgsPoint( x[i], y[i] ) );
  }
  QgsPolygon polygon;
  polygon.append( polyline );
  QgsGeometry* geo = QgsGeometry::fromPolygon( polygon );
  double area = mDistanceArea.measure( geo );
  delete geo;
  if ( !mCrs.geographicFlag() )
  {
    area *= qPow( G_database_units_to_meters_factor(), 2 );
  }
  return area;
}

double GRASS_LIB_EXPORT G_area_of_polygon( const double *x, const double *y, int n )
{
  return QgsGrassGisLib::instance()->G_area_of_polygon( x, y, n );
}

double GRASS_LIB_EXPORT G_database_units_to_meters_factor( void )
{
  return QgsGrassGisLib::instance()->G_database_units_to_meters_factor();
}

int QgsGrassGisLib::beginCalculations( void )
{
  if ( !mCrs.isValid() ) return 0;
  if ( !mCrs.geographicFlag() ) return 1; // planimetric
  return 2; // non-planimetric
}

int GRASS_LIB_EXPORT G_begin_cell_area_calculations( void )
{
  return QgsGrassGisLib::instance()->beginCalculations();
}

int GRASS_LIB_EXPORT G_begin_distance_calculations( void )
{
  return QgsGrassGisLib::instance()->beginCalculations();
}

int GRASS_LIB_EXPORT G_begin_geodesic_distance( double a, double e2 )
{
  Q_UNUSED( a );
  Q_UNUSED( e2 );
  return 0; // nothing to do
}

int GRASS_LIB_EXPORT G_begin_polygon_area_calculations( void )
{
  return QgsGrassGisLib::instance()->beginCalculations();
}

// Distance in meters
double QgsGrassGisLib::distance( double e1, double n1, double e2, double n2 )
{
  // QgsDistanceArea states that results are in meters, but it does not
  // seem to be true,
  double dist = mDistanceArea.measureLine( QgsPoint( e1, n1 ), QgsPoint( e2, n2 ) );
  if ( !mCrs.geographicFlag() )
  {
    dist *= G_database_units_to_meters_factor();
  }
  return dist;

}

double GRASS_LIB_EXPORT G_distance( double e1, double n1, double e2, double n2 )
{
  return QgsGrassGisLib::instance()->distance( e1, n1, e2, n2 );
}

// TODO: verify if QgsGrassGisLib::distance is OK, in theory
// a module could call distance for latlong even if current projection is projected
double GRASS_LIB_EXPORT G_geodesic_distance( double lon1, double lat1, double lon2, double lat2 )
{
  return QgsGrassGisLib::instance()->distance( lon1, lat1, lon2, lat2 );
}

int GRASS_LIB_EXPORT G_legal_filename( const char *s )
{
  Q_UNUSED( s );
  return 1;
}

int QgsGrassGisLib::G_set_geodesic_distance_lat1( double lat1 )
{
  mLat1 = lat1;
  return 0;
}

int QgsGrassGisLib::G_set_geodesic_distance_lat2( double lat2 )
{
  mLat2 = lat2;
  return 0;
}

int GRASS_LIB_EXPORT G_set_geodesic_distance_lat1( double lat1 )
{
  return QgsGrassGisLib::instance()->G_set_geodesic_distance_lat1( lat1 );
}

int GRASS_LIB_EXPORT G_set_geodesic_distance_lat2( double lat2 )
{
  return QgsGrassGisLib::instance()->G_set_geodesic_distance_lat2( lat2 );
}

double QgsGrassGisLib::G_geodesic_distance_lon_to_lon( double lon1, double lon2 )
{
  double dist = mDistanceArea.measureLine( QgsPoint( lon1, mLat1 ), QgsPoint( lon2, mLat2 ) );
  // TODO: not sure about this
  if ( !mCrs.geographicFlag() )
  {
    dist *= G_database_units_to_meters_factor();
  }
  return dist;
}

double GRASS_LIB_EXPORT G_geodesic_distance_lon_to_lon( double lon1, double lon2 )
{
  return QgsGrassGisLib::instance()->G_geodesic_distance_lon_to_lon( lon1, lon2 );
}

int QgsGrassGisLib::G_get_ellipsoid_parameters( double *a, double *e2 )
{
  // TODO: how to get ellipsoid params from mCrs?
  *a = 6378137.0;
  *e2 = .006694385;
  return 0;
}

int GRASS_LIB_EXPORT G_get_ellipsoid_parameters( double *a, double *e2 )
{
  return QgsGrassGisLib::instance()->G_get_ellipsoid_parameters( a, e2 );
}

QGis::DataType QgsGrassGisLib::qgisRasterType( RASTER_MAP_TYPE grassType )
{
  switch ( grassType )
  {
    case CELL_TYPE:
      return QGis::Int32;
      break;
    case FCELL_TYPE:
      return QGis::Float32;
      break;
    case DCELL_TYPE:
      return QGis::Float64;
      break;
    default:
      break;
  }
  return QGis::UnknownDataType;
}

RASTER_MAP_TYPE QgsGrassGisLib::grassRasterType( QGis::DataType qgisType )
{
  switch ( qgisType )
  {
    case QGis::Byte:
    case QGis::UInt16:
    case QGis::Int16:
    case QGis::UInt32:
    case QGis::Int32:
      return CELL_TYPE;
    case QGis::Float32:
      return FCELL_TYPE;
    case QGis::Float64:
      return DCELL_TYPE;
      // Not supported types:
    case QGis::CInt16:
    case QGis::CInt32:
    case QGis::CFloat32:
    case QGis::CFloat64:
    case QGis::ARGB32:
    case QGis::ARGB32_Premultiplied:
    default:
      return -1;
  }
}

double QgsGrassGisLib::noDataValueForGrassType( RASTER_MAP_TYPE grassType )
{
  double noDataValue = std::numeric_limits<double>::quiet_NaN();
  switch ( grassType )
  {
    case CELL_TYPE:
      noDataValue = -1 * std::numeric_limits<int>::max();
      break;
    case FCELL_TYPE:
      noDataValue = std::numeric_limits<float>::quiet_NaN();
      break;
    case DCELL_TYPE:
      noDataValue = std::numeric_limits<double>::quiet_NaN();
      break;
    default:
      break;
  }
  return noDataValue;
}

typedef int G_vasprintf_type( char **, const char *, va_list );
int G_vasprintf( char **out, const char *fmt, va_list ap )
{
  G_vasprintf_type* fn = ( G_vasprintf_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_vasprintf" ) );
  return fn( out, fmt, ap );
}

typedef int G_asprintf_type( char **, const char *, ... );
int G_asprintf( char **out, const char *fmt, ... )
{
  G_asprintf_type* fn = ( G_asprintf_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_asprintf" ) );
  va_list ap;
  va_start( ap, fmt );
  int ret = fn( out, fmt, ap );
  va_end( ap );
  return ret;
}

typedef int G_lookup_key_value_from_file_type( const char *, const char *, char [], int );
int GRASS_LIB_EXPORT G_lookup_key_value_from_file( const char *file, const char *key, char value[], int n )
{
  G_lookup_key_value_from_file_type *fn = ( G_lookup_key_value_from_file_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_lookup_key_value_from_file" ) );
  return fn( file, key, value, n );
}

typedef int G_cell_stats_histo_eq_type( struct Cell_stats *, CELL, CELL, CELL, CELL, int, void ( * )( CELL, CELL, CELL ) );

int GRASS_LIB_EXPORT G_cell_stats_histo_eq( struct Cell_stats *statf, CELL min1, CELL max1, CELL min2, CELL max2, int zero, void ( *func )( CELL, CELL, CELL ) )
{
  G_cell_stats_histo_eq_type *fn = ( G_cell_stats_histo_eq_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_cell_stats_histo_eq_type" ) );
  return fn( statf, min1, max1, min2, max2, zero, func );
}

int GRASS_LIB_EXPORT G__temp_element( char *element )
{
  Q_UNUSED( element );
  return 0;
}

char GRASS_LIB_EXPORT *G_tempfile( void )
{
  QTemporaryFile file( "qgis-grass-temp.XXXXXX" );
  QString name = file.fileName();
  file.open();
  return name.toAscii().data();
}

char GRASS_LIB_EXPORT *G_mapset( void )
{
  return qstrdup( "qgis" );
}

char GRASS_LIB_EXPORT *G_location( void )
{
  return qstrdup( "qgis" );
}

int GRASS_LIB_EXPORT G__write_colors( FILE * fd, struct Colors *colors )
{
  Q_UNUSED( fd );
  Q_UNUSED( colors );
  return 1; // OK
}

int GRASS_LIB_EXPORT G_write_colors( const char *name, const char *mapset, struct Colors *colors )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  Q_UNUSED( colors );
  return 1;
}

int GRASS_LIB_EXPORT G_quantize_fp_map_range( const char *name, const char *mapset, DCELL d_min, DCELL d_max, CELL min, CELL max )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  Q_UNUSED( d_min );
  Q_UNUSED( d_max );
  Q_UNUSED( min );
  Q_UNUSED( max );
  return 1;
}

int GRASS_LIB_EXPORT G_read_cats( const char *name, const char *mapset, struct Categories *pcats )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  G_init_raster_cats( "Cats", pcats );
  return 0;
}

int GRASS_LIB_EXPORT G_read_raster_cats( const char *name, const char *mapset, struct Categories *pcats )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  G_init_raster_cats( "Cats", pcats );
  return 0;
}

int GRASS_LIB_EXPORT G_write_cats( char *name, struct Categories *cats )
{
  Q_UNUSED( name );
  Q_UNUSED( cats );
  return 1; // OK
}

int GRASS_LIB_EXPORT G_write_raster_cats( char *name, struct Categories *cats )
{
  Q_UNUSED( name );
  Q_UNUSED( cats );
  return 1;
}

int GRASS_LIB_EXPORT G_short_history( const char *name, const char *type, struct History *hist )
{
  Q_UNUSED( name );
  Q_UNUSED( type );
  Q_UNUSED( hist );
  return 1;
}

int GRASS_LIB_EXPORT G_write_history( const char *name, struct History *hist )
{
  Q_UNUSED( name );
  Q_UNUSED( hist );
  return 0;
}

int GRASS_LIB_EXPORT G_maskfd( void )
{
  return -1; // no mask
}

int GRASS_LIB_EXPORT G_command_history( struct History *hist )
{
  Q_UNUSED( hist );
  return 0;
}

int GRASS_LIB_EXPORT G_set_cats_title( const char *title, struct Categories *pcats )
{
  Q_UNUSED( title );
  Q_UNUSED( pcats );
  return 0;
}

int GRASS_LIB_EXPORT G_read_history( const char *name, const char *mapset, struct History *hist )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  Q_UNUSED( hist );
  return 0;
}

int GRASS_LIB_EXPORT G_read_colors( const char *name, const char *mapset, struct Colors *colors )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  Q_UNUSED( colors );
  return 1;
}

int GRASS_LIB_EXPORT G_make_aspect_fp_colors( struct Colors *colors, DCELL min, DCELL max )
{
  Q_UNUSED( colors );
  Q_UNUSED( min );
  Q_UNUSED( max );
  return 1; // OK
}

int GRASS_LIB_EXPORT G_check_overwrite( int argc, char **argv )
{
  Q_UNUSED( argc );
  Q_UNUSED( argv );
  return 1; // overwrite
}

char GRASS_LIB_EXPORT *G_fully_qualified_name( const char *name, const char *mapset )
{
  Q_UNUSED( mapset );
  return G_store( name );
}

char GRASS_LIB_EXPORT *G_ask_cell_new( const char *prompt, char *name )
{
  Q_UNUSED( prompt );
  Q_UNUSED( name );
  return NULL;
}

char GRASS_LIB_EXPORT *G_ask_cell_old( const char *prompt, char *name )
{
  Q_UNUSED( prompt );
  Q_UNUSED( name );
  return NULL;
}

int GRASS_LIB_EXPORT G_remove( const char *element, const char *name )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  return -1; // error
}

int GRASS_LIB_EXPORT G_rename( const char *element, const char *oldname, const char *newname )
{
  Q_UNUSED( element );
  Q_UNUSED( oldname );
  Q_UNUSED( newname );
  return -1; // error
}

char GRASS_LIB_EXPORT *G_get_cell_title( const char *name, const char *mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return qstrdup( "title" );
}

int GRASS_LIB_EXPORT G_put_cell_title( const char *name, const char *title )
{
  Q_UNUSED( name );
  Q_UNUSED( title );
  return 1; // OK
}

int GRASS_LIB_EXPORT G_clear_screen( void )
{
  return 0; // OK
}

char GRASS_LIB_EXPORT *G_find_vector( char *name, const char *mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return qstrdup( "qgis" );
}

char GRASS_LIB_EXPORT *G_find_vector2( const char *name, const char *mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return qstrdup( "qgis" );
}

int GRASS_LIB_EXPORT G__make_mapset_element( const char *p_element )
{
  Q_UNUSED( p_element );
  return 1; // OK
}

char GRASS_LIB_EXPORT *G_location_path( void )
{
  return qstrdup( "qgis" );
}

FILE GRASS_LIB_EXPORT *G_fopen_modify( const char *element, const char *name )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  return NULL;
}

FILE GRASS_LIB_EXPORT *G_fopen_old( const char *element, const char *name, const char *mapset )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return NULL;
}

char GRASS_LIB_EXPORT *G_gisdbase( void )
{
  return NULL;
}

int GRASS_LIB_EXPORT G__name_is_fully_qualified( const char *fullname, char *name, char *mapset )
{
  Q_UNUSED( fullname );
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return 1; // fully qualified
}

int GRASS_LIB_EXPORT G_open_new( const char *element, const char *name )
{
  Q_UNUSED( element );
  Q_UNUSED( name );
  return -1; // cannot open
}

struct Key_Value GRASS_LIB_EXPORT *G_get_projinfo( void )
{
  return NULL;
}

struct Key_Value GRASS_LIB_EXPORT *G_get_projunits( void )
{
  return NULL;
}

int GRASS_LIB_EXPORT G_get_reclass( const char *name, const char *mapset, struct Reclass *reclass )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  reclass->min = 0;
  reclass->max = 0;
  reclass->num = 0;
  reclass->table = NULL;
  return 1; // OK
}

CELL GRASS_LIB_EXPORT G_number_of_cats( const char *name, const char *mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return 0;
}

int GRASS_LIB_EXPORT G_round_fp_map( const char *name, const char *mapset )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  return -1; // error
}

char GRASS_LIB_EXPORT *G_mask_info( void )
{
  return qstrdup( "none" );
}

int GRASS_LIB_EXPORT G_read_quant( const char *name, const char *mapset, struct Quant *quant )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  G_quant_init( quant );
  return 0; // does not exist
}

int GRASS_LIB_EXPORT G_write_fp_range( const char *name, const struct FPRange *range )
{
  Q_UNUSED( name );
  Q_UNUSED( range );
  return 0; // OK
}

int GRASS_LIB_EXPORT G_write_range( const char *name, const struct Range *range )
{
  Q_UNUSED( name );
  Q_UNUSED( range );
  return 0; // OK
}

int GRASS_LIB_EXPORT G_write_raster_units( const char *name, const char *str )
{
  Q_UNUSED( name );
  Q_UNUSED( str );
  return 0; // OK
}

int GRASS_LIB_EXPORT G_open_update( const char *element, const char *name )
{
  // G_open_update is used in r.flow if parm.seg, but parm.seg doesnt seem
  // to be set to 1
  Q_UNUSED( element );
  Q_UNUSED( name );
  qFatal( "G_open_update not imlemented" );
  return -1; // Cannot open
}
