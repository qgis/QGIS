/***************************************************************************
    qgsgrass.cpp  -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qglobal.h>

#ifdef _MSC_VER
// to avoid conflicting SF_UNKNOWN
#define _OLE2_H_
#endif

#include <setjmp.h>

// for Sleep / usleep for debugging
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <time.h>
#endif

#include "qgsgrass.h"
#include "qgsgrassvector.h"

#ifdef HAVE_GUI
#include "qgsgrassoptions.h"
#endif

#include "qgsapplication.h"
#include "qgsconfig.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfields.h"
#include "qgslocalec.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>
#include <QTemporaryFile>
#include <QHash>
#include <QTextCodec>
#include <QElapsedTimer>
#include <QRegExp>

extern "C"
{
#ifndef Q_OS_WIN
#include <unistd.h>
#include <sys/types.h>
#endif
#include <grass/version.h>
#if defined(_MSC_VER) && defined(M_PI_4)
#undef M_PI_4 //avoid redefinition warning
#endif
#include <grass/gprojects.h>
#include <grass/vector.h>
#include <grass/raster.h>
}

#define GRASS_LOCK sMutex.lock();
#define GRASS_UNLOCK sMutex.unlock();

QgsGrassObject::QgsGrassObject( const QString &gisdbase, const QString &location,
                                const QString &mapset, const QString &name, Type type )
  : mGisdbase( gisdbase )
  , mLocation( location )
  , mMapset( mapset )
  , mName( name )
  , mType( type )
{
}

QString QgsGrassObject::fullName() const
{
  if ( mName.isEmpty() )
  {
    return QString();
  }
  if ( mMapset.isEmpty() )
  {
    return mName;
  }
  return mName + "@" + mMapset;
}

void QgsGrassObject::setFullName( const QString &fullName )
{
  QStringList parts = fullName.split( '@' );
  mName = parts.value( 0 );
  mMapset.clear();
  if ( !fullName.isEmpty() )
  {
    mMapset = parts.size() > 1 ? parts.value( 1 ) : QgsGrass::getDefaultMapset();
  }
}

bool QgsGrassObject::setFromUri( const QString &uri )
{
  QgsDebugMsgLevel( "uri = " + uri, 2 );
  QFileInfo fi( uri );

  if ( fi.isFile() )
  {
    QString path = fi.canonicalFilePath();
    QgsDebugMsgLevel( "path = " + path, 2 );
    // /gisdbase_path/location/mapset/cellhd/raster_map
    QRegExp rx( "(.*)/([^/]*)/([^/]*)/cellhd/([^/]*)", Qt::CaseInsensitive );
    if ( rx.indexIn( path ) > -1 )
    {
      mGisdbase = rx.cap( 1 );
      mLocation = rx.cap( 2 );
      mMapset = rx.cap( 3 );
      mName = rx.cap( 4 );
      mType = Raster;
      return QgsGrass::isLocation( mGisdbase + "/" + mLocation );
    }
  }
  else
  {
    // /gisdbase_path/location/mapset/vector_map/layer
    // QFileInfo.canonicalPath() on non existing file does not work (returns empty string)
    // QFileInfo.absolutePath() does not necessarily remove symbolic links or redundant "." or ".."
    QDir dir = fi.dir(); // .../mapset/vector_map - does not exist
    if ( dir.cdUp() ) // .../mapset/
    {
      QString path = dir.canonicalPath();
      QRegExp rx( "(.*)/([^/]*)/([^/]*)" );
      if ( rx.indexIn( path ) > -1 )
      {
        mGisdbase = rx.cap( 1 );
        mLocation = rx.cap( 2 );
        mMapset = rx.cap( 3 );
        mName = fi.dir().dirName();
        mType = Vector;
        QgsDebugMsgLevel( "parsed : " + toString(), 2 );
        return QgsGrass::isLocation( mGisdbase + "/" + mLocation );
      }
    }
  }
  return false;
}

QString QgsGrassObject::elementShort() const
{
  return elementShort( mType );
}

QString QgsGrassObject::elementShort( Type type )
{
  if ( type == Raster )
    return QStringLiteral( "raster" );
  else if ( type == Group )
    return QStringLiteral( "group" );
  else if ( type == Vector )
    return QStringLiteral( "vector" );
  else if ( type == Region )
    return QStringLiteral( "region" );
  else if ( type == Strds )
    return QStringLiteral( "strds" );
  else if ( type == Stvds )
    return QStringLiteral( "stvds" );
  else if ( type == Str3ds )
    return QStringLiteral( "str3ds" );
  else if ( type == Stds )
    return QStringLiteral( "stds" );
  else
    return QString();
}

QString QgsGrassObject::elementName() const
{
  return elementName( mType );
}

QString QgsGrassObject::elementName( Type type )
{
  if ( type == Raster )
    return QStringLiteral( "raster" );
  else if ( type == Group )
    return QStringLiteral( "group" );
  else if ( type == Vector )
    return QStringLiteral( "vector" );
  else if ( type == Region )
    return QStringLiteral( "region" );
  else
    return QString();
}

QString QgsGrassObject::dirName() const
{
  return dirName( mType );
}

QString QgsGrassObject::dirName( Type type )
{
  if ( type == Raster )
    return QStringLiteral( "cellhd" );
  else if ( type == Group )
    return QStringLiteral( "group" );
  else if ( type == Vector )
    return QStringLiteral( "vector" );
  else if ( type == Region )
    return QStringLiteral( "windows" );
  else
    return QString();
}

QString QgsGrassObject::toString() const
{
  return elementName() + " : " + mapsetPath() + " : " + mName;
}

bool QgsGrassObject::locationIdentical( const QgsGrassObject &other ) const
{
  QFileInfo fi( locationPath() );
  QFileInfo otherFi( other.locationPath() );
  return fi == otherFi;
}

bool QgsGrassObject::mapsetIdentical( const QgsGrassObject &other ) const
{
  QFileInfo fi( mapsetPath() );
  QFileInfo otherFi( other.mapsetPath() );
  return fi == otherFi;
}

QString QgsGrassObject::newNameRegExp( Type type )
{
  if ( type == QgsGrassObject::Vector )
  {
    return QStringLiteral( "[A-Za-z_][A-Za-z0-9_]+" );
  }
  else // location, raster, see G_legal_filename
  {
    return QStringLiteral( "[\\w_\\-][\\w_\\-.]+" );
  }
}

bool QgsGrassObject::operator==( const QgsGrassObject &other ) const
{
  return mGisdbase == other.mGisdbase && mLocation == other.mLocation && mMapset == other.mMapset
         && mName == other.mName && mType == other.mType;
}

QString QgsGrass::pathSeparator()
{
#ifdef Q_OS_WIN
  return ";";
#else
  return QStringLiteral( ":" );
#endif
}

#ifdef Q_OS_WIN
#include <windows.h>
QString QgsGrass::shortPath( const QString &path )
{
  TCHAR buf[MAX_PATH];
  int len = GetShortPathName( path.toUtf8().constData(), buf, MAX_PATH );

  if ( len == 0 || len > MAX_PATH )
  {
    QgsDebugMsg( QString( "GetShortPathName('%1') failed with %2: %3" )
                 .arg( path ).arg( len ).arg( GetLastError() ) );
    return path;
  }

  QString res = QString::fromUtf8( buf );
  // GRASS wxpyton GUI fails on paths with backslash, Windows do support forward slashesin paths
  res.replace( "\\", "/" );
  return res;
}
#endif

bool QgsGrass::init( void )
{
  // Do not show warning dialog in this function, it may cause problems in non interactive tests etc.

  // Warning!!!
  // G_set_error_routine() once called from plugin
  // is not valid in provider -> call it always

  if ( sNonInitializable )
  {
    return false;
  }

  if ( sInitialized )
  {
    return true;
  }

  // Set error function
  G_set_error_routine( &error_routine );

  lock();
  QgsDebugMsgLevel( QStringLiteral( "do init" ), 2 );

  sActive = false;
  // Is it active mode ?
  if ( getenv( "GISRC" ) )
  {
    G_TRY
    {
      // Store default values
      sDefaultGisdbase = G_gisdbase();
      sDefaultLocation = G_location();
      sDefaultMapset = G_mapset();
      sActive = true;
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      QgsDebugMsg( QStringLiteral( "GISRC set but cannot get gisdbase/location/mapset: %1" ).arg( e.what() ) );
    }
  }

  // Don't use GISRC file and read/write GRASS variables (from location G_VAR_GISRC) to memory only.
  G_set_gisrc_mode( G_GISRC_MODE_MEMORY );

  // Init GRASS libraries (required)
  // G_no_gisinit() may end with fatal error if QGIS is run with a version of GRASS different from that used for compilation
  G_TRY
  {
    G_no_gisinit();  // Doesn't check write permissions for mapset compare to G_gisinit("libgrass++");
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    sInitError = tr( "Problem in GRASS initialization, GRASS provider and plugin will not work : %1" ).arg( e.what() );
    QgsDebugMsg( sInitError );
    sNonInitializable = true;
    unlock();
    return false;
  }

  // I think that mask should not be used in QGIS as it can confuse users,
  // anyway, I don't think anybody is using MASK
  // TODO7: Rast_suppress_masking (see G_suppress_masking() macro above) needs MAPSET
  // (it should not be necessary, because rasters are read by modules qgis.g.info and qgis.d.rast
  //  where masking is suppressed)
  //G_suppress_masking();

  // Set program name
  G_set_program_name( "QGIS" );

  // Require GISBASE to be set. This should point to the location of
  // the GRASS installation. The GRASS libraries use it to know
  // where to look for things.
  if ( !isValidGrassBaseDir( gisbase() ) )
  {
    sNonInitializable = true;
    sInitError = tr( "GRASS was not found in '%1' (GISBASE), provider and plugin will not work." ).arg( gisbase() );
    QgsDebugMsg( sInitError );
#if 0
    // TODO: how to emit message from provider (which does not know about QgisApp)
    QgisApp::instance()->messageBar()->pushMessage( tr( "GRASS error" ),
        error_message, QgsMessageBar: WARNING );
#endif
    unlock();
    return false;
  }
  else
  {
    QgsDebugMsgLevel( "Valid GRASS gisbase is: " + gisbase(), 2 );
    // GISBASE environment variable must be set because is required by directly called GRASS functions
    putEnv( QStringLiteral( "GISBASE" ), gisbase() );

    // Create list of paths to GRASS modules
    // PATH environment variable is not used to search for modules (since 2.12) because it could
    // create a lot of confusion especially if both GRASS 6 and 7 are installed and path to one version
    // $GISBASE/bin somehow gets to PATH and another version plugin is loaded to QGIS, because if a module
    // is missing in one version, it could be found in another $GISBASE/bin and misleadin error could be reported
    sGrassModulesPaths.clear();
    sGrassModulesPaths << gisbase() + "/bin";
    sGrassModulesPaths << gisbase() + "/scripts";
    sGrassModulesPaths << QgsApplication::pkgDataPath() + "/grass/scripts";
    sGrassModulesPaths << qgisGrassModulePath();

    // On windows the GRASS libraries are in
    // QgsApplication::prefixPath(), we have to add them
    // to PATH to enable running of GRASS modules
    // and database drivers
#ifdef Q_OS_WIN
    // It seems that QgsApplication::prefixPath() is not initialized at this point
    // TODO: verify if this is required and why (PATH to required libs should be set in qgis-grass.bat)
    //mGrassModulesPaths << shortPath( QCoreApplication::applicationDirPath() ) );

    // Add path to MSYS bin
    // Warning: MSYS sh.exe will translate this path to '/bin'
    QString msysBin = QCoreApplication::applicationDirPath() + "/msys/bin/";
    if ( QFileInfo( msysBin ).isDir() )
    {
      sGrassModulesPaths << shortPath( QCoreApplication::applicationDirPath() + "/msys/bin/" );
    }
#endif

    //QString p = getenv( "PATH" );
    //path.append( sep + p );

    QgsDebugMsgLevel( QStringLiteral( "sGrassModulesPaths = " ) + sGrassModulesPaths.join( ',' ), 2 );
    //putEnv( "PATH", path );

    // TODO: move where it is required for QProcess
    // Set GRASS_PAGER if not set, it is necessary for some
    // modules printing to terminal, e.g. g.list
    // We use 'cat' because 'more' is not present in MSYS (Win)
    // and it doesn't work well in built in shell (Unix/Mac)
    // and 'less' is not user friendly (for example user must press
    // 'q' to quit which is definitely difficult for normal user)
    // Also scroling can be don in scrollable window in both
    // MSYS terminal and built in shell.
    if ( !getenv( "GRASS_PAGER" ) )
    {
      QString pager;
      QStringList pagers;
      //pagers << "more" << "less" << "cat"; // se notes above
      pagers << QStringLiteral( "cat" );

      for ( int i = 0; i < pagers.size(); i++ )
      {
        int state;

        QProcess p;
        p.start( pagers.at( i ), QStringList() );
        p.waitForStarted();
        state = p.state();
        p.write( "\004" ); // Ctrl-D
        p.closeWriteChannel();
        p.waitForFinished( 1000 );
        p.kill();

        if ( state == QProcess::Running )
        {
          pager = pagers.at( i );
          break;
        }
      }

      if ( pager.length() > 0 )
      {
        putEnv( QStringLiteral( "GRASS_PAGER" ), pager );
      }
    }
    sInitialized = 1;
  }

  unlock();

  // after unlock because it is using setMapset() which calls init()
  if ( sActive )
  {
    QgsGrass::instance()->loadMapsetSearchPath(); // must be after G_no_gisinit()
    QgsGrass::instance()->setMapsetSearchPathWatcher();
  }

  return true;
}

/*
 * Check if given directory contains a GRASS installation
 */
bool QgsGrass::isValidGrassBaseDir( const QString &gisbase )
{
  QgsDebugMsgLevel( QStringLiteral( "isValidGrassBaseDir()" ), 2 );
  // GRASS currently doesn't handle paths with blanks
  if ( gisbase.isEmpty() || gisbase.contains( QLatin1String( " " ) ) )
  {
    return false;
  }

  /* TODO: G_is_gisbase() was added to GRASS 6.1 06-05-24,
           enable its use after some period (others do update) */
#if 0
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
    if ( G_is_gisbase( gisbase.toUtf8().constData() ) )
      return true;
  }
  else
  {
#endif
    QFileInfo gbi( gisbase + "/etc/element_list" );
    if ( gbi.exists() )
      return true;
#if 0
  }
#endif
  return false;
}

QgsGrass *QgsGrass::instance()
{
  static QgsGrass sInstance;
  return &sInstance;
}

void QgsGrass::lock()
{
  QgsDebugMsgLevel( QStringLiteral( "lock" ), 2 );
  sMutex.lock();
}

void QgsGrass::unlock()
{
  QgsDebugMsgLevel( QStringLiteral( "unlock" ), 2 );
  sMutex.unlock();
}

bool QgsGrass::activeMode()
{
  return sActive;
}

QString QgsGrass::getDefaultGisdbase()
{
  return sDefaultGisdbase;
}

QString QgsGrass::getDefaultLocation()
{
  return sDefaultLocation;
}

QgsGrassObject QgsGrass::getDefaultLocationObject()
{
  return QgsGrassObject( sDefaultGisdbase, sDefaultLocation, QString(), QString(), QgsGrassObject::Location );
}

QString QgsGrass::getDefaultLocationPath()
{
  if ( !sActive )
  {
    return QString();
  }
  return sDefaultGisdbase + "/" + sDefaultLocation;
}

QString QgsGrass::getDefaultMapset()
{
  return sDefaultMapset;
}

QgsGrassObject QgsGrass::getDefaultMapsetObject()
{
  return QgsGrassObject( sDefaultGisdbase, sDefaultLocation, sDefaultMapset, QString(), QgsGrassObject::Mapset );
}

QString QgsGrass::getDefaultMapsetPath()
{
  return getDefaultLocationPath() + "/" + sDefaultMapset;
}

void QgsGrass::setLocation( const QString &gisdbase, const QString &location )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );
  setMapset( gisdbase, location, QStringLiteral( "PERMANENT" ) );
}

void QgsGrass::setMapset( const QString &gisdbase, const QString &location, const QString &mapset )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2 mapset = %3" ).arg( gisdbase, location, mapset ), 2 );
  if ( !init() )
  {
    QgsDebugMsg( QgsGrass::initError() );
    return;
  }

  // Set principal GRASS variables (in memory)
#ifdef Q_OS_WIN
  G_setenv_nogisrc( "GISDBASE", shortPath( gisdbase ).toUtf8().constData() );
#else
  G_setenv_nogisrc( "GISDBASE", gisdbase.toUtf8().constData() );
#endif
  G_setenv_nogisrc( "LOCATION_NAME", location.toUtf8().constData() );
  G_setenv_nogisrc( "MAPSET", mapset.toUtf8().constData() );

  // Add all available mapsets to search path
  // Why? Other mapsets should not be necessary.
#if 0
  // G_get_available_mapsets in GRASS 6 returns pointer to memory managed by GRASS,
  // in GRASS 7 it always allocates new memory and caller must free that
  char **ms = 0;
  G_TRY
  {
    ms = G_available_mapsets();
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( QString( "No available mapsets found: %1" ).arg( e.what() ) );
    return;
  }

  // There seems to be no function to clear search path
  for ( int i = 0; ms[i]; i++ )
  {
    G_add_mapset_to_search_path( ms[i] ); // only adds mapset if it is not yet in path
    free( ms[i] );
  }
  free( ms );
#endif
}

void QgsGrass::setMapset( const QgsGrassObject &grassObject )
{
  setMapset( grassObject.gisdbase(), grassObject.location(), grassObject.mapset() );
}

bool QgsGrass::isMapsetInSearchPath( const QString &mapset )
{
  return mMapsetSearchPath.contains( mapset );
}

void QgsGrass::addMapsetToSearchPath( const QString &mapset, QString &error )
{
  QString cmd = gisbase() + "/bin/g.mapsets";
  QStringList arguments;

  arguments << QStringLiteral( "operation=add" ) << "mapset=" + mapset;

  try
  {
    int timeout = -1; // What timeout to use? It can take long time on network or database
    runModule( getDefaultGisdbase(), getDefaultLocation(), getDefaultMapset(), cmd, arguments, timeout, false );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot add mapset %1 to search path:" ).arg( mapset ) + " " + e.what();
  }
}

void QgsGrass::removeMapsetFromSearchPath( const QString &mapset, QString &error )
{
  QString cmd = gisbase() + "/bin/g.mapsets";
  QStringList arguments;

  arguments << QStringLiteral( "operation=remove" ) << "mapset=" + mapset;

  try
  {
    int timeout = -1; // What timeout to use? It can take long time on network or database
    runModule( getDefaultGisdbase(), getDefaultLocation(), getDefaultMapset(), cmd, arguments, timeout, false );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot remove mapset %1 from search path: %2" ).arg( mapset, e.what() );
  }
}

void QgsGrass::loadMapsetSearchPath()
{
  // do not lock, it is called from locked function
  QStringList oldMapsetSearchPath = mMapsetSearchPath;
  mMapsetSearchPath.clear();
  if ( !activeMode() )
  {
    QgsDebugMsgLevel( QStringLiteral( "not active" ), 2 );
    emit mapsetSearchPathChanged();
    return;
  }
  G_TRY
  {
    QgsGrass::setMapset( getDefaultGisdbase(), getDefaultLocation(), getDefaultMapset() );
    const char *mapset = nullptr;
    G_reset_mapsets();
    for ( int i = 0; ( mapset = G_get_mapset_name( i ) ); i++ )
    {
      QgsDebugMsgLevel( QStringLiteral( "mapset = %1" ).arg( mapset ), 2 );
      if ( G_is_mapset_in_search_path( mapset ) )
      {
        mMapsetSearchPath << mapset;
      }
    }
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugMsg( "cannot load mapset search path: " + QString( e.what() ) );
  }
  QgsDebugMsgLevel( QStringLiteral( "mMapsetSearchPath = " ) +  mMapsetSearchPath.join( ',' ), 2 );
  if ( mMapsetSearchPath != oldMapsetSearchPath )
  {
    emit mapsetSearchPathChanged();
  }
}

void QgsGrass::setMapsetSearchPathWatcher()
{
  QgsDebugMsgLevel( QStringLiteral( "entered." ), 4 );
  if ( mMapsetSearchPathWatcher )
  {
    delete mMapsetSearchPathWatcher;
    mMapsetSearchPathWatcher = nullptr;
  }
  if ( !activeMode() )
  {
    return;
  }
  mMapsetSearchPathWatcher = new QFileSystemWatcher( this );

  QString searchFilePath = getDefaultMapsetPath() + "/SEARCH_PATH";

  if ( QFileInfo::exists( searchFilePath ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "add watcher on SEARCH_PATH file " ) + searchFilePath, 2 );
    mMapsetSearchPathWatcher->addPath( searchFilePath );
    connect( mMapsetSearchPathWatcher, &QFileSystemWatcher::fileChanged, this, &QgsGrass::onSearchPathFileChanged );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "add watcher on mapset " ) + getDefaultMapsetPath(), 2 );
    mMapsetSearchPathWatcher->addPath( getDefaultMapsetPath() );
    connect( mMapsetSearchPathWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsGrass::onSearchPathFileChanged );
  }
}

void QgsGrass::onSearchPathFileChanged( const QString &path )
{
  QgsDebugMsgLevel( "path = " + path, 2 );
  QString searchFilePath = getDefaultMapsetPath() + "/SEARCH_PATH";
  if ( path == searchFilePath )
  {
    // changed or removed
    loadMapsetSearchPath();
    if ( !QFileInfo::exists( searchFilePath ) ) // removed
    {
      // reset watcher to mapset
      setMapsetSearchPathWatcher();
    }
  }
  else
  {
    // mapset directory changed
    if ( QFileInfo::exists( searchFilePath ) ) // search path file added
    {
      loadMapsetSearchPath();
      setMapsetSearchPathWatcher();
    }
  }
}

jmp_buf QgsGrass::jumper;

bool QgsGrass::sNonInitializable = false;
int QgsGrass::sInitialized = 0;

bool QgsGrass::sActive = 0;

QgsGrass::GError QgsGrass::sLastError = QgsGrass::OK;

QString QgsGrass::sErrorMessage;
QString QgsGrass::sInitError;

QStringList QgsGrass::sGrassModulesPaths;
QString QgsGrass::sDefaultGisdbase;
QString QgsGrass::sDefaultLocation;
QString QgsGrass::sDefaultMapset;

QString QgsGrass::sMapsetLock;
QString QgsGrass::sGisrc;
QString QgsGrass::sTmp;

QMutex QgsGrass::sMutex;

bool QgsGrass::sMute = false;

int QgsGrass::error_routine( char *msg, int fatal )
{
  return error_routine( ( const char * ) msg, fatal );
}

int QgsGrass::error_routine( const char *msg, int fatal )
{
  // G_fatal_error obviously is not thread safe (everything static in GRASS, especially fatal_jmp_buf)
  // it means that anything which may end up with G_fatal_error must use mutex

  // Unfortunately the exceptions thrown here can only be caught if GRASS libraries are compiled
  // with -fexception option on Linux (works on Windows)
  // GRASS developers are reluctant to add -fexception by default
  // https://trac.osgeo.org/grass/ticket/869
  QgsDebugMsg( QStringLiteral( "error_routine (fatal = %1): %2" ).arg( fatal ).arg( msg ) );

  sErrorMessage = msg;

  if ( fatal )
  {
    QgsDebugMsg( QStringLiteral( "fatal -> longjmp" ) );
    // Exceptions cannot be thrown from here if GRASS lib is not compiled with -fexceptions
    //throw QgsGrass::Exception( QString::fromUtf8( msg ) );
    sLastError = Fatal;
  }
  else
  {
    sLastError = Warning;
  }

  return 1;
}

void QgsGrass::resetError( void )
{
  sLastError = OK;
}

int QgsGrass::error( void )
{
  return sLastError;
}

QString QgsGrass::errorMessage( void )
{
  return sErrorMessage;
}

bool QgsGrass::isOwner( const QString &gisdbase, const QString &location, const QString &mapset )
{
  QString mapsetPath = gisdbase + "/" + location + "/" + mapset;

  // G_mapset_permissions() (check_owner() in GRASS 7) on Windows consider all mapsets to be owned by user
  // There is more complex G_owner() but that is not used in G_gisinit() (7.1.svn).
  // On Windows and on systems where files do not have owners ownerId() returns ((uint) -2).
#ifndef Q_OS_WIN
  bool owner = QFileInfo( mapsetPath ).ownerId() == getuid();
#else
  bool owner = true;
#endif
  QgsDebugMsgLevel( QStringLiteral( "%1 : owner = %2" ).arg( mapsetPath ).arg( owner ), 2 );
  return owner;
}

QString QgsGrass::openMapset( const QString &gisdbase,
                              const QString &location, const QString &mapset )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1" ).arg( gisdbase.toUtf8().constData() ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "location = %1" ).arg( location.toUtf8().constData() ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "mapset = %1" ).arg( mapset.toUtf8().constData() ), 2 );

  closeMapset(); // close currently opened mapset (if any)

  QString mapsetPath = gisdbase + "/" + location + "/" + mapset;

  // Check if the mapset is in use
  if ( !isValidGrassBaseDir( gisbase() ) )
    return QObject::tr( "GISBASE is not set." );

  QFileInfo fi( mapsetPath + "/WIND" );
  if ( !fi.exists() )
  {
    return QObject::tr( "%1 is not a GRASS mapset." ).arg( mapsetPath );
  }

  QString lock = mapsetPath + "/.gislock";

#ifndef _MSC_VER
  int pid = getpid();
#else
  int pid = GetCurrentProcessId();
#endif

  QgsDebugMsgLevel( QStringLiteral( "pid = %1" ).arg( pid ), 2 );

#ifndef Q_OS_WIN
  QFile lockFile( lock );
  QProcess process;
  QString lockProgram( gisbase() + "/etc/lock" );
  QStringList lockArguments;
  lockArguments << lock << QString::number( pid );
  QString lockCommand = lockProgram + " " + lockArguments.join( QLatin1Char( ' ' ) ); // for debug
  QgsDebugMsgLevel( "lock command: " + lockCommand, 2 );

  process.start( lockProgram, lockArguments );
  if ( !process.waitForStarted( 5000 ) )
  {
    return QObject::tr( "Cannot start %1" ).arg( lockCommand );
  }
  process.waitForFinished( 5000 );

  QString processResult = QStringLiteral( "exitStatus=%1, exitCode=%2, errorCode=%3, error=%4 stdout=%5, stderr=%6" )
                          .arg( process.exitStatus() ).arg( process.exitCode() )
                          .arg( process.error() ).arg( process.errorString(),
                              process.readAllStandardOutput().constData(), process.readAllStandardError().constData() );
  QgsDebugMsgLevel( "processResult: " + processResult, 2 );

  // lock exit code:
  // 0 - OK
  // 1 - error
  // 2 - mapset in use
  if ( process.exitCode() == 2 )
  {
    return QObject::tr( "Mapset is already in use." );
  }

  if ( process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 )
  {
    QString message = QObject::tr( "Mapset lock failed (%1)" ).arg( processResult );
    return message;
  }

#endif // Q_OS_WIN

  // Create temporary directory
  QFileInfo info( mapsetPath );
  QString user = info.owner();

  sTmp = QDir::tempPath() + "/grass-" + user + "-" + QString::number( pid );
  QDir dir( sTmp );
  if ( dir.exists() )
  {
    QFileInfo dirInfo( sTmp );
    if ( !dirInfo.isWritable() )
    {
#ifndef Q_OS_WIN
      lockFile.remove();
#endif
      return QObject::tr( "Temporary directory %1 exists but is not writable" ).arg( sTmp );
    }
  }
  else if ( !dir.mkdir( sTmp ) )
  {
#ifndef Q_OS_WIN
    lockFile.remove();
#endif
    return QObject::tr( "Cannot create temporary directory %1" ).arg( sTmp );
  }

  // Create GISRC file
  QString globalGisrc = QDir::home().path() + "/.grassrc6";
  sGisrc = sTmp + "/gisrc";

  QgsDebugMsgLevel( QStringLiteral( "globalGisrc = %1" ).arg( globalGisrc ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "mGisrc = %1" ).arg( sGisrc ), 2 );

  QFile out( sGisrc );
  if ( !out.open( QIODevice::WriteOnly ) )
  {
#ifndef Q_OS_WIN
    lockFile.remove();
#endif
    return QObject::tr( "Cannot create %1" ).arg( sGisrc );
  }
  QTextStream stream( &out );

  QFile in( globalGisrc );
  QString line;
  bool guiSet = false;
  char buf[1000];
  if ( in.open( QIODevice::ReadOnly ) )
  {
    while ( in.readLine( buf, 1000 ) != -1 )
    {
      line = buf;
      if ( line.contains( QLatin1String( "GISDBASE:" ) ) ||
           line.contains( QLatin1String( "LOCATION_NAME:" ) ) ||
           line.contains( QLatin1String( "MAPSET:" ) ) )
      {
        continue;
      }
      if ( line.contains( QLatin1String( "GRASS_GUI:" ) ) )
        guiSet = true;
      stream << line;
    }
    in.close();
  }
  line = "GISDBASE: " + gisdbase + "\n";
  stream << line;
  line = "LOCATION_NAME: " + location + "\n";
  stream << line;
  line = "MAPSET: " + mapset + "\n";
  stream << line;
  if ( !guiSet )
  {
    stream << "GRASS_GUI: wxpython\n";
  }

  out.close();

  // Set GISRC environment variable
  // Mapset must be set before Vect_close()
  /* _Correct_ putenv() implementation is not making copy! */
  putEnv( QStringLiteral( "GISRC" ), sGisrc );

  // Reinitialize GRASS
  G_setenv_nogisrc( "GISRC", sGisrc.toUtf8().constData() );
#ifdef Q_OS_WIN
  G_setenv_nogisrc( "GISDBASE", shortPath( gisdbase ).toLocal8Bit().data() );
#else
  G_setenv_nogisrc( "GISDBASE", gisdbase.toUtf8().constData() );
#endif
  G_setenv_nogisrc( "LOCATION_NAME", location.toLocal8Bit().data() );
  G_setenv_nogisrc( "MAPSET", mapset.toLocal8Bit().data() );
  sDefaultGisdbase = gisdbase;
  sDefaultLocation = location;
  sDefaultMapset = mapset;

  sActive = true;

  QgsGrass::instance()->loadMapsetSearchPath();
  QgsGrass::instance()->setMapsetSearchPathWatcher();

// closeMapset() added at the beginning
#if 0
#ifndef Q_OS_WIN
  // Close old mapset
  if ( mMapsetLock.length() > 0 )
  {
    QFile file( mMapsetLock );
    file.remove();
  }
#endif
#endif

  sMapsetLock = lock;

  emit QgsGrass::instance()->mapsetChanged();
  return QString();
}

QString QgsGrass::closeMapset()
{

  if ( sMapsetLock.length() > 0 )
  {
#ifndef Q_OS_WIN
    QFile file( sMapsetLock );
    if ( !file.remove() )
    {
      return QObject::tr( "Cannot remove mapset lock: %1" ).arg( sMapsetLock );
    }
#endif
    sMapsetLock.clear();

    putenv( ( char * ) "GISRC" );

    // Reinitialize GRASS
    G_setenv_nogisrc( "GISRC", ( char * ) "" );

    // Temporarily commented because of
    //   http://trac.osgeo.org/qgis/ticket/1900
    //   http://trac.osgeo.org/gdal/ticket/3313
    // it can be uncommented once GDAL with patch gets deployed (probably GDAL 1.8)
    //G_setenv_nogisrc( "GISDBASE", ( char * ) "" );
    //G_setenv_nogisrc( "LOCATION_NAME", ( char * ) "" );
    //G_setenv_nogisrc( "MAPSET", ( char * ) "" );

    sDefaultGisdbase.clear();
    sDefaultLocation.clear();
    sDefaultMapset.clear();
    sActive = 0;

    // Delete temporary dir

    // To be sure that we don't delete '/' for example
    if ( sTmp.left( 4 ) == QLatin1String( "/tmp" ) )
    {
      QDir dir( sTmp );
      for ( unsigned int i = 0; i < dir.count(); i++ )
      {
        if ( dir[i] == QLatin1String( "." ) || dir[i] == QLatin1String( ".." ) )
          continue;

        dir.remove( dir[i] );
        if ( dir.remove( dir[i] ) )
        {
          QgsDebugMsg( QStringLiteral( "Cannot remove temporary file %1" ).arg( dir[i] ) );
        }
      }

      if ( !dir.rmdir( sTmp ) )
      {
        QgsDebugMsg( QStringLiteral( "Cannot remove temporary directory %1" ).arg( sTmp ) );
      }
    }
  }

  QgsGrass::instance()->setMapsetSearchPathWatcher(); // unset watcher
  emit QgsGrass::instance()->mapsetChanged();
  return QString();
}

bool QgsGrass::closeMapsetWarn()
{

  QString err = QgsGrass::closeMapset();

  if ( !err.isNull() )
  {
    warning( tr( "Cannot close mapset. %1" ).arg( err ) );
    return false;
  }
  return true;
}

void QgsGrass::saveMapset()
{

  // Save working mapset in project file
  QgsProject::instance()->writeEntry( QStringLiteral( "GRASS" ), QStringLiteral( "/WorkingGisdbase" ),
                                      QgsProject::instance()->writePath( getDefaultGisdbase() ) );

  QgsProject::instance()->writeEntry( QStringLiteral( "GRASS" ), QStringLiteral( "/WorkingLocation" ),
                                      getDefaultLocation() );

  QgsProject::instance()->writeEntry( QStringLiteral( "GRASS" ), QStringLiteral( "/WorkingMapset" ),
                                      getDefaultMapset() );
}

void QgsGrass::createMapset( const QString &gisdbase, const QString &location,
                             const QString &mapset, QString &error )
{
  QString locationPath = gisdbase + "/" + location;
  QDir locationDir( locationPath );

  if ( !locationDir.mkdir( mapset ) )
  {
    error = tr( "Cannot create new mapset directory" );
    return;
  }

  QString src = locationPath + "/PERMANENT/DEFAULT_WIND";
  QString dest = locationPath + "/" + mapset + "/WIND";
  if ( !QFile::copy( src, dest ) )
  {
    error = tr( "Cannot copy %1 to %2" ).arg( src, dest );
  }
}

QStringList QgsGrass::locations( const QString &gisdbase )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1" ).arg( gisdbase ), 2 );

  QStringList list;

  if ( gisdbase.isEmpty() )
    return list;

  QDir d = QDir( gisdbase );
  d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( QFile::exists( gisdbase + "/" + d[i]
                        + "/PERMANENT/DEFAULT_WIND" ) )
    {
      list.append( QString( d[i] ) );
    }
  }
  return list;
}

QStringList QgsGrass::mapsets( const QString &gisdbase, const QString &locationName )
{
  QgsDebugMsgLevel( QStringLiteral( "gisbase = %1 locationName = %2" ).arg( gisdbase, locationName ), 2 );

  if ( gisdbase.isEmpty() || locationName.isEmpty() )
    return QStringList();

  return QgsGrass::mapsets( gisdbase + "/" + locationName );
}

QStringList QgsGrass::mapsets( const QString &locationPath )
{
  QgsDebugMsgLevel( QStringLiteral( "locationPath = %1" ).arg( locationPath ), 2 );

  QStringList list;

  if ( locationPath.isEmpty() )
    return list;

  QDir d = QDir( locationPath );
  d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( QFile::exists( locationPath + "/" + d[i] + "/WIND" ) )
    {
      list.append( d[i] );
    }
  }
  return list;
}

QStringList QgsGrass::vectors( const QString &gisdbase, const QString &locationName,
                               const QString &mapsetName )
{

  if ( gisdbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
    return QStringList();

  /* TODO: G_list() was added to GRASS 6.1 06-05-24,
  enable its use after some period (others do update) */
  /*
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
  QStringList list;

  char **glist = G_list( G_ELEMENT_VECTOR,
  gisbase.toUtf8().constData(),
  locationName.toUtf8().constData(),
  mapsetName.toUtf8().constData() );

  int i = 0;

  while ( glist[i] )
  {
  list.append( QString(glist[i]) );
  i++;
  }

  G_free_list ( glist );

  return list;
  }
  */

  return QgsGrass::vectors( gisdbase + "/" + locationName + "/" + mapsetName );
}

QStringList QgsGrass::vectors( const QString &mapsetPath )
{
  QgsDebugMsgLevel( QStringLiteral( "mapsetPath = %1" ).arg( mapsetPath ), 2 );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/vector" );
  d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );

  list.reserve( d.count() );
  for ( unsigned int i = 0; i < d.count(); ++i )
  {
#if 0
    if ( QFile::exists( mapsetPath + "/vector/" + d[i] + "/head" ) )
    {
      list.append( d[i] );
    }
#endif
    list.append( d[i] );
  }
  return list;
}

bool QgsGrass::topoVersion( const QString &gisdbase, const QString &location,
                            const QString &mapset, const QString &mapName, int &major, int &minor )
{
  QString path = gisdbase + "/" + location + "/" + mapset + "/vector/" + mapName + "/topo";
  QFile file( path );
  if ( !file.exists( path ) || file.size() < 5 )
  {
    return false;
  }
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    return false;
  }
  QDataStream stream( &file );
  quint8 maj, min;
  stream >> maj;
  stream >> min;
  file.close();
  major = maj;
  minor = min;
  return true;
}

QStringList QgsGrass::vectorLayers( const QString &gisdbase, const QString &location,
                                    const QString &mapset, const QString &mapName )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2 mapset = %3 mapName = %4" ).arg( gisdbase, location, mapset, mapName ), 2 );

  QStringList list;
  QgsGrassVector vector( gisdbase, location, mapset, mapName );
  if ( !vector.openHead() )
  {
    throw QgsGrass::Exception( vector.error() );
  }

  QgsDebugMsgLevel( QStringLiteral( "GRASS vector successfully opened" ), 2 );

  // Get layers
  const auto constLayers = vector.layers();
  for ( QgsGrassVectorLayer *layer : constLayers )
  {
    QString fs = QString::number( layer->number() );
    QgsDebugMsgLevel( QStringLiteral( "layer number = " ) + fs, 2 );

    /* Points */
    int npoints = layer->typeCount( GV_POINT );
    QgsDebugMsgLevel( QStringLiteral( "npoints = %1" ).arg( npoints ), 2 );
    if ( npoints > 0 )
    {
      QString l = fs + "_point";
      list.append( l );
    }

    /* Lines */
    /* Lines without category appears in layer 0, but not boundaries */
    int nlines = layer->typeCount( GV_LINE );
    if ( layer->number() > 0 )
    {
      nlines += layer->typeCount( GV_BOUNDARY );
    }
    QgsDebugMsgLevel( QStringLiteral( "nlines = %1" ).arg( nlines ), 2 );
    if ( nlines > 0 )
    {
      QString l = fs + "_line";
      list.append( l );
    }

    /* Faces */
    int nfaces = layer->typeCount( GV_FACE );
    QgsDebugMsgLevel( QStringLiteral( "nfaces = %1" ).arg( nfaces ), 2 );
    if ( nfaces > 0 )
    {
      QString l = fs + "_face";
      list.append( l );
    }

    /* Polygons */
    int nareas = layer->typeCount( GV_AREA );
    QgsDebugMsgLevel( QStringLiteral( "nareas = %1" ).arg( nareas ), 2 );
    if ( nareas > 0 )
    {
      QString l = fs + "_polygon";
      list.append( l );
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "standard layers listed: " ) + list.join( ',' ), 2 );

  // TODO: add option in GUI to set listTopoLayers
  QgsSettings settings;
  bool listTopoLayers = settings.value( QStringLiteral( "GRASS/showTopoLayers" ), false ).toBool();
  if ( listTopoLayers )
  {
    // add topology layers
    if ( vector.typeCount( GV_POINTS ) > 0 )
    {
      /* no more point in GRASS 7 topo */
      //list.append( "topo_point" );
    }
    if ( vector.typeCount( GV_LINES ) > 0 )
    {
      list.append( QStringLiteral( "topo_line" ) );
    }
    if ( vector.nodeCount() > 0 )
    {
      list.append( QStringLiteral( "topo_node" ) );
    }
  }

  return list;
}

QStringList QgsGrass::rasters( const QString &gisdbase, const QString &locationName,
                               const QString &mapsetName )
{

  if ( gisdbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
    return QStringList();


  /* TODO: G_list() was added to GRASS 6.1 06-05-24,
  enable its use after some period (others do update) */
  /*
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
  QStringList list;

  char **glist = G_list( G_ELEMENT_RASTER,
  gisbase.toUtf8().constData(),
  locationName.toUtf8().constData(),
  mapsetName.toUtf8().constData() );

  int i = 0;

  while ( glist[i] )
  {
  list.append( QString(glist[i]) );
  i++;
  }

  G_free_list ( glist );

  return list;
  }
  */

  return QgsGrass::rasters( gisdbase + "/" + locationName + "/" + mapsetName );
}

QStringList QgsGrass::rasters( const QString &mapsetPath )
{
  QgsDebugMsgLevel( QStringLiteral( "mapsetPath = %1" ).arg( mapsetPath ), 2 );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/cellhd" );
  d.setFilter( QDir::Files );

  list.reserve( d.count() );
  for ( unsigned int i = 0; i < d.count(); ++i )
  {
    list.append( d[i] );
  }
  return list;
}

QStringList QgsGrass::groups( const QString &gisdbase, const QString &locationName,
                              const QString &mapsetName )
{
  return elements( gisdbase, locationName, mapsetName, QStringLiteral( "group" ) );
}

QStringList QgsGrass::groups( const QString &mapsetPath )
{
  return elements( mapsetPath, QStringLiteral( "group" ) );
}

QStringList QgsGrass::elements( const QString &gisdbase, const QString &locationName,
                                const QString &mapsetName, const QString &element )
{
  if ( gisdbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
  {
    return QStringList();
  }

  return QgsGrass::elements( gisdbase + "/" + locationName + "/" + mapsetName, element );
}

QStringList QgsGrass::elements( const QString  &mapsetPath, const QString  &element )
{
  QgsDebugMsgLevel( QStringLiteral( "mapsetPath = %1 element = %2" ).arg( mapsetPath, element ), 2 );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/" + element );
  if ( element == QLatin1String( "vector" ) || element == QLatin1String( "group" ) )
  {
    d.setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
  }
  else
  {
    d.setFilter( QDir::Files );
  }

  list.reserve( d.count() );
  for ( unsigned int i = 0; i < d.count(); ++i )
  {
    list.append( d[i] );
  }
  return list;
}

QStringList QgsGrass::grassObjects( const QgsGrassObject &mapsetObject, QgsGrassObject::Type type )
{
  QgsDebugMsgLevel( QStringLiteral( "mapsetPath = " ) + mapsetObject.mapsetPath() + QStringLiteral( " type = " ) +  QgsGrassObject::elementShort( type ), 2 );
  QElapsedTimer time;
  time.start();
  QStringList list;
  if ( !QDir( mapsetObject.mapsetPath() ).isReadable() )
  {
    QgsDebugMsg( QStringLiteral( "mapset is not readable" ) );
    return QStringList();
  }
  else if ( type == QgsGrassObject::Strds || type == QgsGrassObject::Stvds
            || type == QgsGrassObject::Str3ds || type == QgsGrassObject::Stds )
  {
    QString cmd = QStringLiteral( "t.list" );

    QStringList arguments;

    // Running t.list module is quite slow (about 500ms) -> check first if temporal db exists.
    // Also, if tgis/sqlite.db does not exist, it is created by t.list for current mapset!
    // If user is not owner of the mapset (even if has read permission) t.list fails because it checks ownership.
    if ( !QFile( mapsetObject.mapsetPath() + "/tgis/sqlite.db" ).exists() )
    {
      QgsDebugMsg( QStringLiteral( "tgis/sqlite.db does not exist" ) );
    }
    else
    {
      if ( type == QgsGrassObject::Stds )
      {
        arguments << QStringLiteral( "type=strds,stvds,str3ds" );
      }
      else
      {
        arguments << "type=" + QgsGrassObject::elementShort( type );
      }

      int timeout = -1; // What timeout to use? It can take long time on network or database
      try
      {
        QByteArray data = runModule( mapsetObject.gisdbase(), mapsetObject.location(), mapsetObject.mapset(), cmd, arguments, timeout, false );
        for ( QString fullName : QString::fromLocal8Bit( data ).split( '\n' ) )
        {
          fullName = fullName.trimmed();
          if ( !fullName.isEmpty() )
          {
            QStringList nameMapset = fullName.split( '@' );
            if ( nameMapset.value( 1 ) == mapsetObject.mapset() || nameMapset.value( 1 ).isEmpty() )
            {
              list << nameMapset.value( 0 );
            }
          }
        }
      }
      catch ( QgsGrass::Exception &e )
      {
        // TODO: notify somehow user
        QgsDebugMsg( QStringLiteral( "Cannot run %1: %2" ).arg( cmd, e.what() ) );
      }
    }
  }
  else
  {
    list = QgsGrass::elements( mapsetObject.mapsetPath(), QgsGrassObject::dirName( type ) );
  }
  QgsDebugMsgLevel( "list = " + list.join( "," ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "time (ms) = %1" ).arg( time.elapsed() ), 2 );
  return list;
}

bool QgsGrass::objectExists( const QgsGrassObject &grassObject )
{
  if ( grassObject.name().isEmpty() )
  {
    return false;
  }
  QString path = grassObject.mapsetPath() + "/" + QgsGrassObject::dirName( grassObject.type() )
                 + "/" + grassObject.name();
  QFileInfo fi( path );
  return fi.exists();
}

QString QgsGrass::regionString( const struct Cell_head *window )
{
  QString reg;
  int fmt;
  char buf[1024];

  fmt = window->proj;

  // TODO 3D

  reg = "proj:" + QString::number( window->proj ) + ";";
  reg += "zone:" + QString::number( window->zone ) + ";";

  G_format_northing( window->north, buf, fmt );
  reg += "north:" + QString( buf ) + ";";

  G_format_northing( window->south, buf, fmt );
  reg += "south:" + QString( buf ) + ";";

  G_format_easting( window->east, buf, fmt );
  reg += "east:" + QString( buf ) + ";";

  G_format_easting( window->west, buf, fmt );
  reg += "west:" + QString( buf ) + ";";

  reg += "cols:" + QString::number( window->cols ) + ";";
  reg += "rows:" + QString::number( window->rows ) + ";";

  G_format_resolution( window->ew_res, buf, fmt );
  reg += "e-w resol:" + QString( buf ) + ";";

  G_format_resolution( window->ns_res, buf, fmt );
  reg += "n-s resol:" + QString( buf ) + ";";

  return reg;
}


bool QgsGrass::defaultRegion( const QString &gisdbase, const QString &location,
                              struct Cell_head *window )
{
  initRegion( window );
  QgsGrass::setLocation( gisdbase, location );
  try
  {
    G_get_default_window( window );
    return true;
  }
  catch ( QgsGrass::Exception &e )
  {
    Q_UNUSED( e )
    return false;
  }
}

void QgsGrass::region( const QString &gisdbase,
                       const QString &location, const QString &mapset,
                       struct Cell_head *window )
{
  QgsGrass::setLocation( gisdbase, location );

  // In GRASS 7 G__get_window does not return error code and calls G_fatal_error on error
  G_FATAL_THROW( G_get_element_window( window, ( char * ) "", ( char * ) "WIND", mapset.toUtf8().constData() ) );
}

void QgsGrass::region( struct Cell_head *window )
{
  region( getDefaultGisdbase(), getDefaultLocation(), getDefaultMapset(), window );
}

bool QgsGrass::writeRegion( const QString &gisbase,
                            const QString &location, const QString &mapset,
                            const struct Cell_head *window )
{
  if ( !window )
  {
    return false;
  }

  QgsDebugMsgLevel( QStringLiteral( "n = %1 s = %2" ).arg( window->north ).arg( window->south ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "e = %1 w = %2" ).arg( window->east ).arg( window->west ), 2 );

  QgsGrass::setMapset( gisbase, location, mapset );

  if ( G_put_window( window ) == -1 )
  {
    return false;
  }

  return true;
}

void QgsGrass::writeRegion( const struct Cell_head *window )
{
  QString error = tr( "Cannot write region" );
  if ( !activeMode() )
  {
    throw QgsGrass::Exception( error += ", " + tr( "no mapset open" ) );
  }
  if ( !writeRegion( getDefaultGisdbase(), getDefaultLocation(), getDefaultMapset(), window ) )
  {
    throw QgsGrass::Exception( error );
  }
  emit regionChanged();
}

void QgsGrass::copyRegionExtent( struct Cell_head *source,
                                 struct Cell_head *target )
{
  target->north = source->north;
  target->south = source->south;
  target->east = source->east;
  target->west = source->west;
  target->top = source->top;
  target->bottom = source->bottom;
}

void QgsGrass::copyRegionResolution( struct Cell_head *source,
                                     struct Cell_head *target )
{
  target->ns_res = source->ns_res;
  target->ew_res = source->ew_res;
  target->tb_res = source->tb_res;
  target->ns_res3 = source->ns_res3;
  target->ew_res3 = source->ew_res3;
}

void QgsGrass::extendRegion( struct Cell_head *source,
                             struct Cell_head *target )
{
  if ( source->north > target->north )
    target->north = source->north;

  if ( source->south < target->south )
    target->south = source->south;

  if ( source->east > target->east )
    target->east = source->east;

  if ( source->west < target->west )
    target->west = source->west;

  if ( source->top > target->top )
    target->top = source->top;

  if ( source->bottom < target->bottom )
    target->bottom = source->bottom;
}

void QgsGrass::initRegion( struct Cell_head *window )
{
  window->format = 0;
  window->rows = 0;
  window->rows3 = 0;
  window->cols = 0;
  window->cols3 = 0;
  window->depths = 1;
  window->proj = -1;
  window->zone = -1;
  window->compressed = -1;
  window->ew_res = 0.0;
  window->ew_res3 = 1.0;
  window->ns_res = 0.0;
  window->ns_res3 = 1.0;
  window->tb_res = 1.0;
  window->top = 1.0;
  window->bottom = 0.0;
  window->west = 0;
  window->south = 0;
  window->east = 1;
  window->north = 1;
  window->rows = 1;
  window->cols = 1;
}

void QgsGrass::setRegion( struct Cell_head *window, const QgsRectangle &rect )
{
  window->west = rect.xMinimum();
  window->south = rect.yMinimum();
  window->east = rect.xMaximum();
  window->north = rect.yMaximum();
}

QString QgsGrass::setRegion( struct Cell_head *window, const QgsRectangle &rect, int rows, int cols )
{
  initRegion( window );
  window->west = rect.xMinimum();
  window->south = rect.yMinimum();
  window->east = rect.xMaximum();
  window->north = rect.yMaximum();
  window->rows = rows;
  window->cols = cols;

  QString error;
  try
  {
    G_adjust_Cell_head( window, 1, 1 );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = e.what();
  }
  return error;
}

QgsRectangle QgsGrass::extent( struct Cell_head *window )
{
  if ( !window )
  {
    return QgsRectangle();
  }
  return QgsRectangle( window->west, window->south, window->east, window->north );
}

bool QgsGrass::mapRegion( QgsGrassObject::Type type, const QString &gisdbase,
                          const QString &location, const QString &mapset, const QString &map,
                          struct Cell_head *window )
{
  QgsDebugMsgLevel( QStringLiteral( "map = %1" ).arg( map ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "mapset = %1" ).arg( mapset ), 2 );

  QgsGrass::setLocation( gisdbase, location );

  if ( type == QgsGrassObject::Raster )
  {
    QString error = tr( "Cannot read raster map region (%1/%2/%3)" ).arg( gisdbase, location, mapset );
    G_TRY
    {
      Rast_get_cellhd( map.toUtf8().constData(), mapset.toUtf8().constData(), window );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      warning( error + " : " + e.what() );
      return false;
    }
  }
  else if ( type == QgsGrassObject::Vector )
  {
    // Get current projection
    try
    {
      QgsGrass::region( gisdbase, location, mapset, window );
    }
    catch ( QgsGrass::Exception &e )
    {
      warning( e );
      return false;
    }

    struct Map_info *Map = nullptr;
    int level = -1;
    G_TRY
    {
      Map = vectNewMapStruct();
      Vect_set_open_level( 2 );
      level = Vect_open_old_head( Map, map.toUtf8().constData(), mapset.toUtf8().constData() );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      warning( e );
      vectDestroyMapStruct( Map );
      return false;
    }

    if ( level < 2 )
    {
      warning( QObject::tr( "Cannot read vector map region" ) );
      if ( level == 1 )
      {
        G_TRY
        {
          Vect_close( Map );
        }
        G_CATCH( QgsGrass::Exception & e )
        {
          QgsDebugMsg( e.what() );
        }
      }
      vectDestroyMapStruct( Map );
      return false;
    }

    bound_box box;
    Vect_get_map_box( Map, &box );
    window->north = box.N;
    window->south = box.S;
    window->west  = box.W;
    window->east  = box.E;
    window->top  = box.T;
    window->bottom  = box.B;

    // Is this optimal ?
    window->ns_res = ( window->north - window->south ) / 1000;
    window->ew_res = window->ns_res;
    if ( window->top > window->bottom )
    {
      window->tb_res = ( window->top - window->bottom ) / 10;
    }
    else
    {
      window->top = window->bottom + 1;
      window->tb_res = 1;
    }
    G_adjust_Cell_head3( window, 0, 0, 0 );

    Vect_close( Map );
    vectDestroyMapStruct( Map );
  }
  else if ( type == QgsGrassObject::Region )
  {
    // G__get_window does not return error code in GRASS 7 and calls G_fatal_error on error
    G_TRY
    {
      G_get_element_window( window, ( char * ) "windows", map.toUtf8().constData(), mapset.toUtf8().constData() );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      warning( e );
      return false;
    }
  }
  return true;
}

QString QgsGrass::findModule( QString module )
{
  QgsDebugMsgLevel( QStringLiteral( "called." ), 4 );
  if ( QFile::exists( module ) )
  {
    return module;  // full path
  }

  QStringList extensions;
#ifdef Q_OS_WIN
  // On windows try .bat first
  extensions << ".bat" << ".py" << ".exe";
#endif
  // and then try if it's a module without extension (standard on UNIX)
  extensions << QString();

  QStringList paths;
  // Try first full path
  paths << QString();
  paths << QgsGrass::grassModulesPaths();

  // Extensions first to prefer .bat over .exe on Windows
  const auto constExtensions = extensions;
  for ( const QString &ext : constExtensions )
  {
    const auto constPaths = paths;
    for ( const QString &path : constPaths )
    {
      QString full = module + ext;
      if ( !path.isEmpty() )
      {
        full.prepend( path + "/" );
      }
      if ( QFile::exists( full ) )
      {
        QgsDebugMsgLevel( "found " + full, 2 );
        return full;
      }
      else
      {
        QgsDebugMsg( "not found " + full );
      }
    }
  }
  return QString();
}

QProcess *QgsGrass::startModule( const QString &gisdbase, const QString  &location,
                                 const QString  &mapset, const QString &moduleName, const QStringList &arguments,
                                 QTemporaryFile &gisrcFile, bool qgisModule )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );
  QProcess *process = new QProcess();

  QString module = moduleName;
  if ( qgisModule )
  {
    module += QString::number( QgsGrass::versionMajor() );
  }

  QString modulePath = findModule( module );
  if ( modulePath.isEmpty() )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot find module %1" ).arg( module ) );
  }

  // We have to set GISRC file, uff
  if ( !gisrcFile.open() )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot open GISRC file" ) );
  }

  QString error = tr( "Cannot start module" ) + "\n" + tr( "command: %1 %2" ).arg( module, arguments.join( QLatin1Char( ' ' ) ) );

  QTextStream out( &gisrcFile );
  out << "GISDBASE: " << gisdbase << "\n";
  out << "LOCATION_NAME: " << location << "\n";
  if ( mapset.isEmpty() )
  {
    out << "MAPSET: PERMANENT\n";
  }
  else
  {
    out << "MAPSET: " << mapset << "\n";
  }
  out.flush();
  QgsDebugMsgLevel( gisrcFile.fileName(), 2 );
  gisrcFile.close();
  QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
  QStringList paths = QgsGrass::grassModulesPaths();
  // PYTHONPATH necessary for t.list.py
  // PATH necessary for g.parser called by t.list.py
  paths += environment.value( QStringLiteral( "PATH" ) ).split( QgsGrass::pathSeparator() );
  environment.insert( QStringLiteral( "PATH" ), paths.join( QgsGrass::pathSeparator() ) );
  environment.insert( QStringLiteral( "PYTHONPATH" ), QgsGrass::getPythonPath() );
  environment.insert( QStringLiteral( "GISRC" ), gisrcFile.fileName() );
  environment.insert( QStringLiteral( "GRASS_MESSAGE_FORMAT" ), QStringLiteral( "gui" ) );
  // Normally modules must be run in a mapset owned by user, because each module calls G_gisinit()
  // which checks if G_mapset() is owned by user. The check is disabled by GRASS_SKIP_MAPSET_OWNER_CHECK.
  environment.insert( QStringLiteral( "GRASS_SKIP_MAPSET_OWNER_CHECK" ), QStringLiteral( "1" ) );

  process->setProcessEnvironment( environment );

  QgsDebugMsgLevel( modulePath + " " + arguments.join( ' ' ), 2 );
  process->start( modulePath, arguments );
  if ( !process->waitForStarted() )
  {
    throw QgsGrass::Exception( error );
  }
  return process;
}

QByteArray QgsGrass::runModule( const QString &gisdbase, const QString  &location,
                                const QString &mapset, const QString  &moduleName,
                                const QStringList &arguments, int timeOut, bool qgisModule )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2 timeOut = %3" ).arg( gisdbase, location ).arg( timeOut ), 2 );
  QElapsedTimer time;
  time.start();

  QTemporaryFile gisrcFile;
  QProcess *process = startModule( gisdbase, location, mapset, moduleName, arguments, gisrcFile, qgisModule );

  if ( !process->waitForFinished( timeOut )
       || ( process->exitCode() != 0 && process->exitCode() != 255 ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "process->exitCode() = " ) + QString::number( process->exitCode() ), 2 );

    throw QgsGrass::Exception( QObject::tr( "Cannot run module" ) + "\n"
                               + QObject::tr( "command: %1 %2\nstdout: %3\nstderr: %4" )
                               .arg( moduleName, arguments.join( QLatin1Char( ' ' ) ),
                                     process->readAllStandardOutput().constData(),
                                     process->readAllStandardError().constData() ) );
  }
  QByteArray data = process->readAllStandardOutput();
  QgsDebugMsgLevel( QStringLiteral( "time (ms) = %1" ).arg( time.elapsed() ), 2 );
  delete process;
  return data;
}

QString QgsGrass::getInfo( const QString  &info, const QString  &gisdbase,
                           const QString  &location, const QString  &mapset,
                           const QString  &map, const QgsGrassObject::Type type,
                           double x, double y,
                           const QgsRectangle &extent, int sampleRows,
                           int sampleCols, int timeOut )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );

  QStringList arguments;

  QString cmd = qgisGrassModulePath() + "/qgis.g.info";

  arguments.append( "info=" + info );
  if ( !map.isEmpty() )
  {
    QString opt;
    switch ( type )
    {
      case QgsGrassObject::Raster:
        opt = QStringLiteral( "rast" );
        break;
      case QgsGrassObject::Vector:
        opt = QStringLiteral( "vect" );
        break;
      default:
        QgsDebugMsg( QStringLiteral( "unexpected type:%1" ).arg( type ) );
        return QString();
    }
    arguments.append( opt + "=" +  map + "@" + mapset );
  }
  if ( info == QLatin1String( "query" ) )
  {
    arguments.append( QStringLiteral( "coor=%1,%2" ).arg( x ).arg( y ) );
  }
  if ( info == QLatin1String( "stats" ) )
  {
    arguments.append( QStringLiteral( "north=%1" ).arg( extent.yMaximum() ) );
    arguments.append( QStringLiteral( "south=%1" ).arg( extent.yMinimum() ) );
    arguments.append( QStringLiteral( "east=%1" ).arg( extent.xMaximum() ) );
    arguments.append( QStringLiteral( "west=%1" ).arg( extent.xMinimum() ) );
    arguments.append( QStringLiteral( "rows=%1" ).arg( sampleRows ) );
    arguments.append( QStringLiteral( "cols=%1" ).arg( sampleCols ) );
  }

  //QByteArray data = runModule( gisdbase, location, mapset, cmd, arguments, timeOut );
  // Run module with empty mapset so that it tries to find a mapset owned by user
  QByteArray data = runModule( gisdbase, location, QString(), cmd, arguments, timeOut );
  QgsDebugMsgLevel( data, 2 );
  return QString( data );
}

QgsCoordinateReferenceSystem QgsGrass::crs( const QString &gisdbase, const QString &location,
    QString &error )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem();
  try
  {
    QString wkt = getInfo( QStringLiteral( "proj" ), gisdbase, location );
    QgsDebugMsgLevel( "wkt: " + wkt, 2 );
    crs = QgsCoordinateReferenceSystem::fromWkt( wkt );
    QgsDebugMsgLevel( "crs.toWkt: " + crs.toWkt(), 2 );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot get projection" ) + "\n" + e.what();
    QgsDebugMsg( error );
  }

  return crs;
}

QgsCoordinateReferenceSystem QgsGrass::crsDirect( const QString &gisdbase, const QString &location )
{
  QString Wkt;

  struct Cell_head cellhd;

  QgsGrass::resetError();
  QgsGrass::setLocation( gisdbase, location );

  {
    QgsLocaleNumC l;

    G_TRY
    {
      G_get_default_window( &cellhd );
    }
    G_CATCH( QgsGrass::Exception & e )
    {
      Q_UNUSED( e )
      QgsDebugMsg( QStringLiteral( "Cannot get default window: %1" ).arg( e.what() ) );
      return QgsCoordinateReferenceSystem();
    }

    if ( cellhd.proj != PROJECTION_XY )
    {
      struct Key_Value *projinfo = G_get_projinfo();
      struct Key_Value *projunits = G_get_projunits();
      char *wkt = GPJ_grass_to_wkt( projinfo, projunits, 0, 0 );
      Wkt = QString( wkt );
      G_free( wkt );
    }
  }

  QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromWkt( Wkt );

  return srs;
}

QgsRectangle QgsGrass::extent( const QString &gisdbase, const QString &location,
                               const QString &mapset, const QString &map,
                               QgsGrassObject::Type type, QString &error )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );

  try
  {
    QString str = getInfo( QStringLiteral( "window" ), gisdbase, location, mapset, map, type );
    QStringList list = str.split( ',' );
    if ( list.size() != 4 )
    {
      throw QgsGrass::Exception( "Cannot parse GRASS map extent: " + str );
    }
    return QgsRectangle( list[0].toDouble(), list[1].toDouble(), list[2].toDouble(), list[3].toDouble() );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot get raster extent" ) + " : " + e.what();
  }
  return QgsRectangle( 0, 0, 0, 0 );
}

void QgsGrass::size( const QString &gisdbase, const QString &location, const QString &mapset,
                     const QString &map, int *cols, int *rows, QString &error )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );

  *cols = 0;
  *rows = 0;
  try
  {
    QString str = getInfo( QStringLiteral( "size" ), gisdbase, location, mapset, map, QgsGrassObject::Raster );
    QStringList list = str.split( ',' );
    if ( list.size() != 2 )
    {
      throw QgsGrass::Exception( "Cannot parse GRASS map size: " + str );
    }
    *cols = list[0].toInt();
    *rows = list[1].toInt();
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot get raster extent" ) + " : " + e.what();
    QgsDebugMsg( error );
  }

  QgsDebugMsgLevel( QStringLiteral( "raster size = %1 %2" ).arg( *cols ).arg( *rows ), 2 );
}

QHash<QString, QString> QgsGrass::info( const QString &gisdbase, const QString &location,
                                        const QString &mapset, const QString &map,
                                        QgsGrassObject::Type type,
                                        const QString &info,
                                        const QgsRectangle &extent,
                                        int sampleRows, int sampleCols,
                                        int timeOut, QString &error )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );
  QHash<QString, QString> inf;

  try
  {
    QString str = getInfo( info, gisdbase, location, mapset, map, type, 0, 0, extent, sampleRows, sampleCols, timeOut );
    QgsDebugMsgLevel( str, 2 );
    QStringList list = str.split( QStringLiteral( "\n" ) );
    for ( int i = 0; i < list.size(); i++ )
    {
      QStringList keyVal = list[i].split( ':' );
      if ( list[i].isEmpty() )
        continue;
      if ( keyVal.size() != 2 )
      {
        throw QgsGrass::Exception( "Cannot parse GRASS map info key value : " + list[i] + " (" + str + " ) " );
      }
      inf[keyVal[0]] = keyVal[1];
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot get map info" ) + "\n" + e.what();
    QgsDebugMsg( error );
  }
  return inf;
}

QList<QgsGrass::Color> QgsGrass::colors( const QString &gisdbase, const QString &location, const QString &mapset,
    const QString &map, QString &error )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );
  QList<QgsGrass::Color> ct;

  try
  {
    QString str = getInfo( QStringLiteral( "colors" ), gisdbase, location, mapset, map, QgsGrassObject::Raster );
    QgsDebugMsgLevel( str, 2 );
    QStringList list = str.split( QStringLiteral( "\n" ) );
    for ( int i = 0; i < list.size(); i++ )
    {
      QgsGrass::Color c;
      if ( list[i].isEmpty() )
        continue;
      if ( sscanf( list[i].toUtf8().constData(), "%lf %lf %d %d %d %d %d %d", &( c.value1 ), &( c.value2 ), &( c.red1 ), &( c.green1 ), &( c.blue1 ), &( c.red2 ), &( c.green2 ), &( c.blue2 ) ) != 8 )
      {
        throw QgsGrass::Exception( "Cannot parse GRASS colors" + list[i] + " (" + str + " ) " );
      }
      ct.append( c );
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    error = tr( "Cannot get colors" ) + " : " + e.what();
    QgsDebugMsg( error );
  }
  return ct;
}

QMap<QString, QString> QgsGrass::query( const QString &gisdbase, const QString &location, const QString &mapset, const QString &map, QgsGrassObject::Type type, double x, double y )
{
  QgsDebugMsgLevel( QStringLiteral( "gisdbase = %1 location = %2" ).arg( gisdbase, location ), 2 );

  QMap<QString, QString> result;
  // TODO: multiple values (more rows)
  try
  {
    QString str = getInfo( QStringLiteral( "query" ), gisdbase, location, mapset, map, type, x, y );
    QStringList list = str.trimmed().split( ':' );
    if ( list.size() == 2 )
    {
      result[list[0]] = list[1];
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    warning( tr( "Cannot query raster\n%1" ).arg( e.what() ) );
  }
  return result;
}

void QgsGrass::renameObject( const QgsGrassObject &object, const QString &newName )
{
  QString cmd = gisbase() + "/bin/g.rename";
  QStringList arguments;

  arguments << object.elementShort() + "=" + object.name() + "," + newName;

  int timeout = -1; // What timeout to use? It can take long time on network or database
  // throws QgsGrass::Exception
  runModule( object.gisdbase(), object.location(), object.mapset(), cmd, arguments, timeout, false );
}

void QgsGrass::copyObject( const QgsGrassObject &srcObject, const QgsGrassObject &destObject )
{
  QgsDebugMsgLevel( "srcObject = " + srcObject.toString(), 2 );
  QgsDebugMsgLevel( "destObject = " + destObject.toString(), 2 );

  if ( !srcObject.locationIdentical( destObject ) ) // should not happen
  {
    throw QgsGrass::Exception( QObject::tr( "Attempt to copy from different location." ) );
  }

  QString cmd = gisbase() + "/bin/g.copy";
  QStringList arguments;

  arguments << srcObject.elementShort() + "=" + srcObject.name() + "@" + srcObject.mapset() + "," + destObject.name();

  int timeout = -1; // What timeout to use? It can take long time on network or database
  // throws QgsGrass::Exception
  // TODO: g.copy does not seem to return error code if fails (6.4.3RC1)
  runModule( destObject.gisdbase(), destObject.location(), destObject.mapset(), cmd, arguments, timeout, false );
}

bool QgsGrass::deleteObject( const QgsGrassObject &object )
{

  // TODO: check if user has permissions

  /*
  if ( QMessageBox::question( this, tr( "Question" ),
                              tr( "Are you sure you want to delete %n selected layer(s)?", "number of layers to delete", indexes.size() ),
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
  {
    return false;
  }
  */

  QString cmd = gisbase() + "/bin/g.remove";
  QStringList arguments;

  arguments << QStringLiteral( "-f" ) << "type=" + object.elementShort() << "name=" + object.name();

  try
  {
    runModule( object.gisdbase(), object.location(), object.mapset(), cmd, arguments, 5000, false );
  }
  catch ( QgsGrass::Exception &e )
  {
    warning( tr( "Cannot delete %1 %2: %3" ).arg( object.elementName(), object.name(), e.what() ) );
    return false;
  }
  return true;
}

bool QgsGrass::deleteObjectDialog( const QgsGrassObject &object )
{

  return QMessageBox::question( nullptr, QObject::tr( "Delete confirmation" ),
                                QObject::tr( "Are you sure you want to delete %1 %2?" ).arg( object.elementName(), object.name() ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes;
}

void QgsGrass::createVectorMap( const QgsGrassObject &object, QString &error )
{

  QgsGrass::setMapset( object );

  struct Map_info *Map = nullptr;
  QgsGrass::lock();
  G_TRY
  {
    Map = vectNewMapStruct();
    Vect_open_new( Map, object.name().toUtf8().constData(), 0 );
    Vect_build( Map );
    Vect_set_release_support( Map );
    Vect_close( Map );
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    error = tr( "Cannot create new vector: %1" ).arg( e.what() );
  }
  QgsGrass::vectDestroyMapStruct( Map );
  QgsGrass::unlock();
}

void QgsGrass::createTable( dbDriver *driver, const QString &tableName, const QgsFields &fields )
{
  if ( !driver ) // should not happen
  {
    throw QgsGrass::Exception( QStringLiteral( "driver is null" ) );
  }

  QStringList fieldsStringList;
  for ( const QgsField &field : fields )
  {
    QString name = field.name().toLower().replace( QLatin1String( " " ), QLatin1String( "_" ) );
    if ( name.at( 0 ).isDigit() )
    {
      name = "_" + name;
    }
    QString typeName;
    switch ( field.type() )
    {
      case QVariant::Int:
      case QVariant::LongLong:
      case QVariant::Bool:
        typeName = QStringLiteral( "integer" );
        break;
      case QVariant::Double:
        typeName = QStringLiteral( "double precision" );
        break;
      // TODO: verify how is it with SpatiaLite/dbf support for date, time, datetime, v.in.ogr is using all
      case QVariant::Date:
        typeName = QStringLiteral( "date" );
        break;
      case QVariant::Time:
        typeName = QStringLiteral( "time" );
        break;
      case QVariant::DateTime:
        typeName = QStringLiteral( "datetime" );
        break;
      case QVariant::String:
        typeName = QStringLiteral( "varchar (%1)" ).arg( field.length() );
        break;
      default:
        typeName = QStringLiteral( "varchar (%1)" ).arg( field.length() > 0 ? field.length() : 255 );
    }
    fieldsStringList <<  name + " " + typeName;
  }
  QString sql = QStringLiteral( "create table %1 (%2);" ).arg( tableName, fieldsStringList.join( QLatin1String( ", " ) ) );

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  int result = db_execute_immediate( driver, &dbstr );
  db_free_string( &dbstr );
  if ( result != DB_OK )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot create table: %1" ).arg( QString::fromLatin1( db_get_error_msg() ) ) );
  }
}

void QgsGrass::insertRow( dbDriver *driver, const QString &tableName,
                          const QgsAttributes &attributes )
{
  if ( !driver ) // should not happen
  {
    throw QgsGrass::Exception( QStringLiteral( "driver is null" ) );
  }

  QStringList valuesStringList;
  const auto constAttributes = attributes;
  for ( const QVariant &attribute : constAttributes )
  {
    QString valueString;

    bool quote = true;
    switch ( attribute.type() )
    {
      case QVariant::Int:
      case QVariant::Double:
      case QVariant::LongLong:
        valueString = attribute.toString();
        quote = false;
        break;
      // TODO: use rbool according to driver
      case QVariant::Bool:
        valueString = attribute.toBool() ? QStringLiteral( "1" ) : QStringLiteral( "0" );
        quote = false;
        break;
      case QVariant::Date:
        valueString = attribute.toDate().toString( Qt::ISODate );
        break;
      case QVariant::Time:
        valueString = attribute.toTime().toString( Qt::ISODate );
        break;
      case QVariant::DateTime:
        valueString = attribute.toDateTime().toString( Qt::ISODate );
        break;
      default:
        valueString = attribute.toString();
    }
    valueString.replace( QLatin1String( "'" ), QLatin1String( "''" ) );

    if ( quote )
    {
      valueString = "'" + valueString + "'";
    }

    valuesStringList <<  valueString;
  }
  QString sql = QStringLiteral( "insert into %1 values (%2);" ).arg( tableName, valuesStringList.join( QLatin1String( ", " ) ) );

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  int result = db_execute_immediate( driver, &dbstr );
  db_free_string( &dbstr );
  if ( result != DB_OK )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot insert, statement: '%1' error: '%2'" ).arg( sql, QString::fromLatin1( db_get_error_msg() ) ) );
  }
}

bool QgsGrass::isExternal( const QgsGrassObject &object )
{
  if ( object.type() != QgsGrassObject::Raster )
  {
    return false;
  }
  lock();
  bool isExternal = false;
  G_TRY
  {
    QgsGrass::setLocation( object.gisdbase(), object.location() );
    struct GDAL_link *gdal;
    gdal = Rast_get_gdal_link( object.name().toUtf8().constData(), object.mapset().toUtf8().constData() );
    if ( gdal )
    {
      isExternal = true;
      Rast_close_gdal_link( gdal );
    }
  }
  G_CATCH( QgsGrass::Exception & e )
  {
    QgsDebugMsg( "error getting external link: " + QString( e.what() ) );
  }
  unlock();
  return isExternal;
}

void QgsGrass::adjustCellHead( struct Cell_head *cellhd, int row_flag, int col_flag )
{
  G_FATAL_THROW( G_adjust_Cell_head( cellhd, row_flag, col_flag ) );
}

// Map of vector types

QMap<int, QString> QgsGrass::vectorTypeMap()
{
  static QMap<int, QString> sVectorTypes;
  static QMutex sMutex;
  if ( sVectorTypes.isEmpty() )
  {
    sMutex.lock();
    if ( sVectorTypes.isEmpty() )
    {
      sVectorTypes.insert( GV_POINT, QStringLiteral( "point" ) );
      sVectorTypes.insert( GV_CENTROID, QStringLiteral( "centroid" ) );
      sVectorTypes.insert( GV_LINE, QStringLiteral( "line" ) );
      sVectorTypes.insert( GV_BOUNDARY, QStringLiteral( "boundary" ) );
      sVectorTypes.insert( GV_AREA, QStringLiteral( "area" ) );
      sVectorTypes.insert( GV_FACE, QStringLiteral( "face" ) );
      sVectorTypes.insert( GV_KERNEL, QStringLiteral( "kernel" ) );
    }
    sMutex.unlock();
  }
  return sVectorTypes;
}

int QgsGrass::vectorType( const QString &typeName )
{
  return QgsGrass::vectorTypeMap().key( typeName );
}

QString QgsGrass::vectorTypeName( int type )
{
  return QgsGrass::vectorTypeMap().value( type );
}

// GRASS version constants have been changed on 26.4.2007
// http://freegis.org/cgi-bin/viewcvs.cgi/grass6/include/version.h.in.diff?r1=1.4&r2=1.5
// The following lines workaround this change

int QgsGrass::versionMajor()
{
#ifdef GRASS_VERSION_MAJOR
  return GRASS_VERSION_MAJOR;
#else
  return QString( GRASS_VERSION_MAJOR ).toInt();
#endif
}

int QgsGrass::versionMinor()
{
#ifdef GRASS_VERSION_MINOR
  return GRASS_VERSION_MINOR;
#else
  return QString( GRASS_VERSION_MINOR ).toInt();
#endif
}

int QgsGrass::versionRelease()
{
#ifdef GRASS_VERSION_RELEASE
#define QUOTE(x)  #x
  return QStringLiteral( QUOTE( GRASS_VERSION_RELEASE ) ).toInt();
#else
  return QString( GRASS_VERSION_RELEASE ).toInt();
#endif
}
QString QgsGrass::versionString()
{
  return QStringLiteral( GRASS_VERSION_STRING );
}

Qt::CaseSensitivity QgsGrass::caseSensitivity()
{
#ifdef Q_OS_WIN
  return Qt::CaseInsensitive;
#else
  return Qt::CaseSensitive;
#endif
}

bool QgsGrass::isLocation( const QString &path )
{
  return G_is_location( path.toUtf8().constData() ) == 1;
}

bool QgsGrass::isMapset( const QString &path )
{
  return G_is_mapset( path.toUtf8().constData() ) == 1;
}

QString QgsGrass::lockFilePath()
{
  return sMapsetLock;
}

QString QgsGrass::gisrcFilePath()
{
  if ( sGisrc.isEmpty() )
  {
    // Started from GRASS shell
    if ( getenv( "GISRC" ) )
    {
      return QString( getenv( "GISRC" ) );
    }
  }
  return sGisrc;
}

void QgsGrass::putEnv( const QString &name, const QString &value )
{
  QString env = name + "=" + value;
  /* _Correct_ putenv() implementation is not making copy! */
  char *envChar = new char[env.toUtf8().length() + 1];
  strcpy( envChar, env.toUtf8().constData() );
  putenv( envChar );
}

QString QgsGrass::getPythonPath()
{
  QString pythonpath = getenv( "PYTHONPATH" );
  pythonpath += pathSeparator() + gisbase() + "/etc/python";
  pythonpath += pathSeparator() + gisbase() + "/gui/wxpython";
  QgsDebugMsgLevel( "pythonpath = " + pythonpath, 2 );
  return pythonpath;
}

QString QgsGrass::defaultGisbase()
{
  // Look first to see if GISBASE env var is already set.
  // This is set when QGIS is run from within GRASS
  // or when set explicitly by the user.
  // This value should always take precedence.
  QString gisbase;
#ifdef Q_OS_WIN
  gisbase = getenv( "WINGISBASE" ) ? getenv( "WINGISBASE" ) : getenv( "GISBASE" );
  gisbase = shortPath( gisbase );
#else
  gisbase = getenv( "GISBASE" );
#endif
  QgsDebugMsgLevel( "gisbase from envar = " + gisbase, 2 );

  if ( !gisbase.isEmpty() )
  {
    return gisbase;
  }

#ifdef Q_OS_WIN
  // Use the applicationDirPath()/grass
#ifdef _MSC_VER
  gisbase = shortPath( QCoreApplication::applicationDirPath() + ( QgsApplication::isRunningFromBuildDir() ?  + "/../.." : "" ) + "/grass" );
#else
  gisbase = shortPath( QCoreApplication::applicationDirPath() + ( QgsApplication::isRunningFromBuildDir() ?  + "/.." : "" ) + "/grass" );
#endif
  // Use the location specified by WITH_GRASS during configure
#elif defined(Q_OS_MACX)
  // check for bundled GRASS, fall back to configured path
  gisbase = QCoreApplication::applicationDirPath().append( "/grass" );
  if ( !isValidGrassBaseDir( gisbase ) )
  {
    gisbase = GRASS_BASE;
  }
#else
  gisbase = GRASS_BASE;
#endif

  QgsDebugMsgLevel( "gisbase = " + gisbase, 2 );
  return gisbase;
}


QString QgsGrass::gisbase()
{
  QgsSettings settings;
  bool customGisbase = settings.value( QStringLiteral( "GRASS/gidbase/custom" ), false ).toBool();
  QString customGisdbaseDir = settings.value( QStringLiteral( "GRASS/gidbase/customDir" ) ).toString();

  QString gisbase;
  if ( customGisbase && !customGisdbaseDir.isEmpty() )
  {
    gisbase = customGisdbaseDir;
  }
  else
  {
    gisbase = defaultGisbase();
  }
#ifdef Q_OS_WIN
  gisbase = shortPath( gisbase );
#endif
  return gisbase;
}

void QgsGrass::setGisbase( bool custom, const QString &customDir )
{
  QgsDebugMsgLevel( QStringLiteral( "custom = %1 customDir = %2" ).arg( custom ).arg( customDir ), 2 );
  QgsSettings settings;

  bool previousCustom = settings.value( QStringLiteral( "GRASS/gidbase/custom" ), false ).toBool();
  QString previousCustomDir = settings.value( QStringLiteral( "GRASS/gidbase/customDir" ) ).toString();
  settings.setValue( QStringLiteral( "GRASS/gidbase/custom" ), custom );
  settings.setValue( QStringLiteral( "GRASS/gidbase/customDir" ), customDir );

  if ( custom != previousCustom || ( custom && customDir != previousCustomDir ) )
  {
    sNonInitializable = false;
    sInitialized = false;
    sInitError.clear();
    if ( !QgsGrass::init() )
    {
      QgsDebugMsg( "cannot init : " + QgsGrass::initError() );
    }
    emit gisbaseChanged();
  }
}


QString QgsGrass::modulesConfigDefaultDirPath()
{
  if ( QgsApplication::isRunningFromBuildDir() )
  {
    return QgsApplication::buildSourcePath() + "/src/plugins/grass/modules";
  }

  return QgsApplication::pkgDataPath() + "/grass/modules";
}

QString QgsGrass::modulesConfigDirPath()
{
  QgsSettings settings;
  bool customModules = settings.value( QStringLiteral( "GRASS/modules/config/custom" ), false ).toBool();
  QString customModulesDir = settings.value( QStringLiteral( "GRASS/modules/config/customDir" ) ).toString();

  if ( customModules && !customModulesDir.isEmpty() )
  {
    return customModulesDir;
  }
  else
  {
    return modulesConfigDefaultDirPath();
  }
}

void QgsGrass::setModulesConfig( bool custom, const QString &customDir )
{
  QgsSettings settings;

  bool previousCustom = settings.value( QStringLiteral( "GRASS/modules/config/custom" ), false ).toBool();
  QString previousCustomDir = settings.value( QStringLiteral( "GRASS/modules/config/customDir" ) ).toString();
  settings.setValue( QStringLiteral( "GRASS/modules/config/custom" ), custom );
  settings.setValue( QStringLiteral( "GRASS/modules/config/customDir" ), customDir );

  if ( custom != previousCustom || ( custom && customDir != previousCustomDir ) )
  {
    emit modulesConfigChanged();
  }
}

QPen QgsGrass::regionPen()
{
  QgsSettings settings;
  QPen pen;
  pen.setColor( QColor( settings.value( QStringLiteral( "GRASS/region/color" ), "#ff0000" ).toString() ) );
  pen.setWidthF( settings.value( QStringLiteral( "GRASS/region/width" ), 0 ).toFloat() );
  return pen;
}

void QgsGrass::setRegionPen( const QPen &pen )
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "GRASS/region/color" ), pen.color().name() );
  settings.setValue( QStringLiteral( "GRASS/region/width" ), pen.widthF() );
  emit regionPenChanged();
}

bool QgsGrass::modulesDebug()
{
  QgsSettings settings;
  return settings.value( QStringLiteral( "GRASS/modules/debug" ), false ).toBool();
}

void QgsGrass::setModulesDebug( bool debug )
{
  QgsSettings settings;
  bool previous = modulesDebug();
  settings.setValue( QStringLiteral( "GRASS/modules/debug" ), debug );
  if ( previous != debug )
  {
    emit modulesDebugChanged();
  }
}

void QgsGrass::openOptions()
{
#ifdef HAVE_GUI
  QgsGrassOptions dialog;
  dialog.exec();
#endif
}

void QgsGrass::warning( const QString &message )
{
  if ( !sMute )
  {
    QMessageBox::warning( nullptr, QObject::tr( "Warning" ), message );
  }
  else
  {
    sErrorMessage = message;
    QgsDebugMsg( message );
  }
}

void QgsGrass::warning( QgsGrass::Exception &e )
{
  warning( e.what() );
}

struct Map_info *QgsGrass::vectNewMapStruct()
{
  // In OSGeo4W there is GRASS compiled by MinGW while QGIS compiled by MSVC, the compilers
  // may have different sizes of types, see issue #13002. Because there is no Vect_new_map_struct (GRASS 7.0.0, July 2015)
  // the structure is allocated here using doubled (should be enough) space.

  // The same problem was also reported for QGIS 2.11-master compiled on Xubuntu 14.04 LTS
  // using GRASS 7.0.1-2~ubuntu14.04.1 from https://launchpad.net/~grass/+archive/ubuntu/grass-stable
  // -> allocate more space on all systems

  // TODO: replace by Vect_new_map_struct once it appears in GRASS
  // Patch supplied: https://trac.osgeo.org/grass/ticket/2729
#if GRASS_VERSION_MAJOR > 999
  G_TRY
  {
    return Vect_new_map_struct();
  }
  G_CATCH( QgsGrass::Exception & e ) // out of memory
  {
    warning( e );
    return 0;
  }
#else
  return ( struct Map_info * ) qgsMalloc( 2 * sizeof( struct Map_info ) );
#endif
}

void QgsGrass::vectDestroyMapStruct( struct Map_info *map )
{
  // TODO: replace by Vect_destroy_map_struct once it appears in GRASS
  // TODO: until switch to hypothetical Vect_destroy_map_struct verify that Vect_destroy_map_struct cannot
  // call G_fatal_error, otherwise check and remove use of vectDestroyMapStruct from G_CATCH blocks
  QgsDebugMsgLevel( QStringLiteral( "free map = %1" ).arg( ( quint64 )map ), 2 );
  qgsFree( map );
}

void QgsGrass::sleep( int ms )
{
// Stolen from QTest::qSleep
#ifdef Q_OS_WIN
  Sleep( uint( ms ) );
#else
  struct timespec ts = { ms / 1000, ( ms % 1000 ) * 1000 * 1000 };
  nanosleep( &ts, nullptr );
#endif
}

QgsGrass::ModuleOutput QgsGrass::parseModuleOutput( const QString &input, QString &text, QString &html, int &value )
{
  QgsDebugMsgLevel( "input = " + input, 2 );
#ifdef QGISDEBUG
  QString ascii;
  for ( int i = 0; i < input.size(); i++ )
  {
    int c = input.at( i ).toLatin1();
    ascii += QStringLiteral( "%1 " ).arg( c, 0, 16 );
  }
  QgsDebugMsgLevel( "ascii = " + ascii, 2 );
#endif

  QRegExp rxpercent( "GRASS_INFO_PERCENT: (\\d+)" );
  QRegExp rxmessage( "GRASS_INFO_MESSAGE\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxwarning( "GRASS_INFO_WARNING\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxerror( "GRASS_INFO_ERROR\\(\\d+,\\d+\\): (.*)" );
  QRegExp rxend( "GRASS_INFO_END\\(\\d+,\\d+\\)" );
  // GRASS added G_progress() which does not support GRASS_MESSAGE_FORMAT=gui
  // and it is printing fprintf(stderr, "%10ld\b\b\b\b\b\b\b\b\b\b", n);
  // Ticket created https://trac.osgeo.org/grass/ticket/2751
  QRegExp rxprogress( " +(\\d+)\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b" );

  // We return simple messages in html non formatted, monospace text should be set on widget
  // where it is used because output may be formatted assuming fixed width font
  if ( input.trimmed().isEmpty() )
  {
    return OutputNone;
  }
  else if ( rxpercent.indexIn( input ) != -1 )
  {
    value = rxpercent.cap( 1 ).toInt();
    return OutputPercent;
  }
  else if ( rxmessage.indexIn( input ) != -1 )
  {
    text = rxmessage.cap( 1 );
    html = text;
    return OutputMessage;
  }
  else if ( rxwarning.indexIn( input ) != -1 )
  {
    text = rxwarning.cap( 1 );
    QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_warning.png";
    html = "<img src=\"" + img + "\">" + text;
    return OutputWarning;
  }
  else if ( rxerror.indexIn( input ) != -1 )
  {
    text = rxerror.cap( 1 );
    QString img = QgsApplication::pkgDataPath() + "/themes/default/grass/grass_module_error.png";
    html = "<img src=\"" + img + "\">" + text;
    return OutputError;
  }
  else if ( rxend.indexIn( input ) != -1 )
  {
    return OutputNone;
  }
  else if ( rxprogress.indexIn( input ) != -1 )
  {
    value = rxprogress.cap( 1 ).toInt();
    return OutputProgress;
  }
  else // some plain text which cannot be parsed
  {
    text = input;
    html = text;
    return OutputMessage;
  }
}
