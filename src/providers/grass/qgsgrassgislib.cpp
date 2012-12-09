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

#include "qgsgrassgislibfunctions.h"
#include "qgsgrassgislib.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
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

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/gprojects.h>
#include <grass/Vect.h>
#include <grass/version.h>
}

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
  QString libPath = QString( GRASS_LIBRARY_GIS );
  QgsDebugMsg( "libPath = " + libPath );
  mLibrary.setFileName( libPath );
  if ( !mLibrary.load() )
  {
    QgsDebugMsg( "Cannot load original GRASS library" );
    return;
  }
}

int GRASS_LIB_EXPORT QgsGrassGisLib::errorRoutine( const char *msg, int fatal )
{
  Q_UNUSED( fatal );
  QgsDebugMsg( QString( "error_routine (fatal = %1): %2" ).arg( fatal ).arg( msg ) );
  // qFatal does core dump, useful for backtrace
  qFatal( "Fatal error: %s", msg );
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
  //QgsDebugMsg( QString("symbol = %1").arg(symbol));
  void * fn = mLibrary.resolve( symbol );
  if ( !fn )
  {
    QgsDebugMsg( "Cannot resolve symbol" );
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
  G__init_null_patterns();

  // Read projection if set
  //mCrs.createFromOgcWmsCrs( "EPSG:900913" );
  QString crsStr = getenv( "QGIS_GRASS_CRS" );
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

  // GRASS true lib reads GRASS_REGION environment variable
  G_get_window( &mWindow );

  mExtent = QgsRectangle( mWindow.west, mWindow.south, mWindow.east, mWindow.north );
  mRows = mWindow.rows;
  mColumns = mWindow.cols;


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
  //QgsDebugMsg( "Entered" );
  G_warning_type* fn = ( G_warning_type* ) cast_to_fptr( QgsGrassGisLib::instance()->resolve( "G_warning" ) );
  va_list ap;
  va_start( ap, msg );
  int ret = fn( msg, ap );
  va_end( ap );
  return ret;
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

char GRASS_LIB_EXPORT *G_find_cell2( const char* name, const char *mapset )
{
  return QgsGrassGisLib::instance()->G_find_cell2( name, mapset );
}

char GRASS_LIB_EXPORT *G_find_cell( char * name, const char * mapset )
{
  // Not really sure about differences between G_find_cell and G_find_cell2
  return G_find_cell2( name, mapset );
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

int QgsGrassGisLib::G_open_raster_new( const char *name, RASTER_MAP_TYPE wr_type )
{
  Q_UNUSED( wr_type );
  QString providerKey = "gdal";
  QString dataSource = name;

  Raster raster;
  raster.name = name;
  //raster.writer = new QgsRasterFileWriter( dataSource );

  raster.provider = ( QgsRasterDataProvider* )QgsProviderRegistry::instance()->provider( providerKey, dataSource );
  if ( !raster.provider )
  {
    fatal( "Cannot load raster provider with data source: " + dataSource );
  }

  QString outputFormat = "GTiff";
  int nBands = 1;
  QgsRasterBlock::DataType type = qgisRasterType( wr_type );
  QgsDebugMsg( QString( "type = %1" ).arg( type ) );
  double geoTransform[6];
  geoTransform[0] = mExtent.xMinimum();
  geoTransform[1] = mExtent.width() / mColumns;
  geoTransform[2] = 0.0;
  geoTransform[3] = mExtent.yMaximum();
  geoTransform[4] = 0.0;
  geoTransform[5] = -1. * mExtent.height() / mRows;

  if ( !raster.provider->create( outputFormat, nBands, type, mColumns, mRows, geoTransform, mCrs ) )
  {
    delete raster.provider;
    fatal( "Cannot create output data source: " + dataSource );
  }
  raster.band = 1;
  double noDataValue = std::numeric_limits<double>::quiet_NaN();
  switch ( wr_type )
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
  QgsDebugMsg( QString( "noDataValue = %1" ).arg(( int )noDataValue ) );
  raster.provider->setNoDataValue( raster.band, noDataValue );

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

RASTER_MAP_TYPE QgsGrassGisLib::G_raster_map_type( const char *name, const char *mapset )
{
  Q_UNUSED( mapset );
  Raster rast = raster( name );

  return grassRasterType( rast.provider->dataType( rast.band ) );
}

RASTER_MAP_TYPE G_raster_map_type( const char *name, const char *mapset )
{
  return QgsGrassGisLib::instance()->G_raster_map_type( name, mapset );
}

RASTER_MAP_TYPE QgsGrassGisLib::G_get_raster_map_type( int fd )
{
  Raster rast = mRasters.value( fd );

  return grassRasterType( rast.provider->dataType( rast.band ) );
}

RASTER_MAP_TYPE G_get_raster_map_type( int fd )
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

  QgsRasterBlock::DataType requestedType = qgisRasterType( data_type );

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

int QgsGrassGisLib::G_put_raster_row( int fd, const void *buf, RASTER_MAP_TYPE data_type )
{
  Raster rast = mRasters.value( fd );
  if ( rast.row < 0 || rast.row >= mRows )
  {
    QgsDebugMsg( QString( "row %1 out of range 0 - %2" ).arg( rast.row ).arg( mRows ) );
    return -1;
  }

  QgsRasterBlock::DataType inputType = qgisRasterType( data_type );
  //QgsDebugMsg( QString("data_type = %1").arg(data_type) );
  //QgsDebugMsg( QString("inputType = %1").arg(inputType) );
  //QgsDebugMsg( QString("provider->dataType = %1").arg( rast.provider->dataType( rast.band ) ) );

  double noDataValue = rast.provider->noDataValue( rast.band );
  QgsRasterBlock block( inputType, mColumns, 1, noDataValue );

  memcpy( block.bits( 0 ), buf, QgsRasterBlock::typeSize( inputType )*mColumns );
  block.convert( rast.provider->dataType( rast.band ) );

  // Set no data after converting to output type
  for ( int i = 0; i < mColumns; i++ )
  {
    bool isNoData = false;
    switch ( data_type )
    {
      case CELL_TYPE:
        isNoData = G_is_c_null_value( &(( CELL * ) buf )[i] ) == TRUE;
        break;
      case FCELL_TYPE:
        isNoData = G_is_f_null_value( &(( FCELL * ) buf )[i] ) == TRUE;
        break;
      case DCELL_TYPE:
        isNoData = G_is_d_null_value( &(( DCELL * ) buf )[i] ) == TRUE;
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
  return QgsGrassGisLib::instance()->G_put_raster_row( fd, buf, data_type );
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
      return 0.; // should not be used
    default:
      return 0.;
  }
  return 0;
}

double GRASS_LIB_EXPORT G_database_units_to_meters_factor( void )
{
  return QgsGrassGisLib::instance()->G_database_units_to_meters_factor();
}

int GRASS_LIB_EXPORT G_begin_distance_calculations( void )
{
  return 1; // nothing to do
}

// Distance in meters
double QgsGrassGisLib::G_distance( double e1, double n1, double e2, double n2 )
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
  return QgsGrassGisLib::instance()->G_distance( e1, n1, e2, n2 );
}

int GRASS_LIB_EXPORT G_legal_filename( const char *s )
{
  Q_UNUSED( s );
  return 1;
}

QgsRasterBlock::DataType QgsGrassGisLib::qgisRasterType( RASTER_MAP_TYPE grassType )
{
  switch ( grassType )
  {
    case CELL_TYPE:
      return QgsRasterBlock::Int32;
      break;
    case FCELL_TYPE:
      return QgsRasterBlock::Float32;
      break;
    case DCELL_TYPE:
      return QgsRasterBlock::Float64;
      break;
    default:
      break;
  }
  return QgsRasterBlock::UnknownDataType;
}

RASTER_MAP_TYPE QgsGrassGisLib::grassRasterType( QgsRasterBlock::DataType qgisType )
{
  switch ( qgisType )
  {
    case QgsRasterBlock::Byte:
    case QgsRasterBlock::UInt16:
    case QgsRasterBlock::Int16:
    case QgsRasterBlock::UInt32:
    case QgsRasterBlock::Int32:
      return CELL_TYPE;
    case QgsRasterBlock::Float32:
      return FCELL_TYPE;
    case QgsRasterBlock::Float64:
      return DCELL_TYPE;
      // Not supported types:
    case QgsRasterBlock::CInt16:
    case QgsRasterBlock::CInt32:
    case QgsRasterBlock::CFloat32:
    case QgsRasterBlock::CFloat64:
    case QgsRasterBlock::ARGB32:
    case QgsRasterBlock::ARGB32_Premultiplied:
    default:
      return -1;
  }
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

int GRASS_LIB_EXPORT G_read_raster_cats( const char *name, const char *mapset, struct Categories *pcats )
{
  Q_UNUSED( name );
  Q_UNUSED( mapset );
  G_init_raster_cats( "Cats", pcats );
  return 0;
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

