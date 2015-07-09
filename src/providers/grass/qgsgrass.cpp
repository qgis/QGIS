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

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfield.h"
#include "qgsrectangle.h"
#include "qgsconfig.h"
#include "qgslocalec.h"

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
#include <grass/version.h>
#include <grass/gprojects.h>

#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#include <grass/raster.h>
#endif
}

#if GRASS_VERSION_MAJOR >= 7
#define G_get_gdal_link Rast_get_gdal_link
#define G_close_gdal_link Rast_close_gdal_link
#endif

#if !defined(GRASS_VERSION_MAJOR) || \
    !defined(GRASS_VERSION_MINOR) || \
    GRASS_VERSION_MAJOR<6 || \
    (GRASS_VERSION_MAJOR == 6 && GRASS_VERSION_MINOR <= 2)
#define G__setenv(name,value) G__setenv( ( char * ) (name), (char *) (value) )
#endif

#define GRASS_LOCK sMutex.lock();
#define GRASS_UNLOCK sMutex.unlock();

QgsGrassObject::QgsGrassObject( const QString& gisdbase, const QString& location,
                                const QString& mapset, const QString& name, Type type ) :
    mGisdbase( gisdbase )
    , mLocation( location )
    , mMapset( mapset )
    , mName( name )
    , mType( type )
{
}

bool QgsGrassObject::setFromUri( const QString& uri )
{
  QgsDebugMsg( "uri = " + uri );
  QFileInfo fi( uri );

  if ( fi.isFile() )
  {
    QString path = fi.canonicalFilePath();
    QgsDebugMsg( "path = " + path );
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
        QgsDebugMsg( "parsed : " + toString() );
        return QgsGrass::isLocation( mGisdbase + "/" + mLocation );
      }
    }
  }
  return false;
}

QString QgsGrassObject::elementShort() const
{
  if ( mType == Raster )
#if GRASS_VERSION_MAJOR < 7
    return "rast";
#else
    return "raster";
#endif
  else if ( mType == Group )
    return "group";
  else if ( mType == Vector )
#if GRASS_VERSION_MAJOR < 7
    return "vect";
#else
    return "vector";
#endif
  else if ( mType == Region )
    return "region";
  else
    return "";
}

QString QgsGrassObject::elementName() const
{
  return elementName( mType );
}

QString GRASS_LIB_EXPORT QgsGrassObject::elementName( Type type )
{
  if ( type == Raster )
    return "raster";
  else if ( type == Group )
    return "group";
  else if ( type == Vector )
    return "vector";
  else if ( type == Region )
    return "region";
  else
    return "";
}

QString QgsGrassObject::dirName() const
{
  return dirName( mType );
}

QString GRASS_LIB_EXPORT QgsGrassObject::dirName( Type type )
{
  if ( type == Raster )
    return "cellhd";
  else if ( type == Group )
    return "group";
  else if ( type == Vector )
    return "vector";
  else if ( type == Region )
    return "windows";
  else
    return "";
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

QRegExp GRASS_LIB_EXPORT QgsGrassObject::newNameRegExp( Type type )
{
  QRegExp rx;
  if ( type == QgsGrassObject::Vector )
  {
    rx.setPattern( "[A-Za-z_][A-Za-z0-9_]+" );
  }
  else
  {
    rx.setPattern( "[A-Za-z0-9_\\-][A-Za-z0-9_\\-.]+" );
  }
  return rx;
}

bool QgsGrassObject::operator==( const QgsGrassObject& other ) const
{
  return mGisdbase == other.mGisdbase && mLocation == other.mLocation && mMapset == other.mMapset
         && mName == other.mName && mType == other.mType;
}

#ifdef Q_OS_WIN
#include <windows.h>
QString GRASS_LIB_EXPORT QgsGrass::shortPath( const QString &path )
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

void GRASS_LIB_EXPORT QgsGrass::init( void )
{
  // Warning!!!
  // G_set_error_routine() once called from plugin
  // is not valid in provider -> call it always

  // Set error function
  G_set_error_routine( &error_routine );

  if ( initialized )
    return;

  QgsDebugMsg( "do init" );
  QSettings settings;

  // Is it active mode ?
  active = false;
  if ( getenv( "GISRC" ) )
  {
    G_TRY
    {
      // Store default values
      defaultGisdbase = G_gisdbase();
      defaultLocation = G_location();
      defaultMapset = G_mapset();
      active = true;
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      QgsDebugMsg( QString( "GISRC set but cannot get gisdbase/location/mapset: %1" ).arg( e.what() ) );
    }
  }

  // Don't use GISRC file and read/write GRASS variables (from location G_VAR_GISRC) to memory only.
  G_set_gisrc_mode( G_GISRC_MODE_MEMORY );

  // Init GRASS libraries (required)
  G_no_gisinit();  // Doesn't check write permissions for mapset compare to G_gisinit("libgrass++");

  // I think that mask should not be used in QGIS as it can only confuses people,
  // anyway, I don't think anybody is using MASK
  // TODO7: Rast_suppress_masking (see G_suppress_masking() macro above) needs MAPSET
#if GRASS_VERSION_MAJOR < 7
  G_suppress_masking();
#endif

  // Set program name
  G_set_program_name( "QGIS" );

  // Require GISBASE to be set. This should point to the location of
  // the GRASS installation. The GRASS libraries use it to know
  // where to look for things.

  // Look first to see if GISBASE env var is already set.
  // This is set when QGIS is run from within GRASS
  // or when set explicitly by the user.
  // This value should always take precedence.
#ifdef Q_OS_WIN
  QString gisBase = getenv( "WINGISBASE" ) ? getenv( "WINGISBASE" ) : getenv( "GISBASE" );
  gisBase = shortPath( gisBase );
#else
  QString gisBase = getenv( "GISBASE" );
#endif
  QgsDebugMsg( QString( "GRASS gisBase from GISBASE env var is: %1" ).arg( gisBase ) );
  if ( !isValidGrassBaseDir( gisBase ) )
  {
    // Look for gisbase in QSettings
    gisBase = settings.value( "/GRASS/gisbase", "" ).toString();
    QgsDebugMsg( QString( "GRASS gisBase from QSettings is: %1" ).arg( gisBase ) );
  }

  if ( !isValidGrassBaseDir( gisBase ) )
  {
    // Erase gisbase from settings because it does not exists
    settings.setValue( "/GRASS/gisbase", "" );

#ifdef Q_OS_WIN
    // Use the applicationDirPath()/grass
#ifdef _MSC_VER
    gisBase = shortPath( QCoreApplication::applicationDirPath() + ( QgsApplication::isRunningFromBuildDir() ?  + "/../.." : "" ) + "/grass" );
#else
    gisBase = shortPath( QCoreApplication::applicationDirPath() + ( QgsApplication::isRunningFromBuildDir() ?  + "/.." : "" ) + "/grass" );
#endif
    QgsDebugMsg( QString( "GRASS gisBase = %1" ).arg( gisBase ) );
#elif defined(Q_OS_MACX)
    // check for bundled GRASS, fall back to configured path
    gisBase = QCoreApplication::applicationDirPath().append( "/grass" );
    if ( !isValidGrassBaseDir( gisBase ) )
    {
      gisBase = GRASS_BASE;
    }
    QgsDebugMsg( QString( "GRASS gisBase = %1" ).arg( gisBase ) );
#else
    // Use the location specified --with-grass during configure
    gisBase = GRASS_BASE;
    QgsDebugMsg( QString( "GRASS gisBase from configure is: %1" ).arg( gisBase ) );
#endif
  }

  bool userGisbase = false;
  bool valid = false;
  while ( !( valid = isValidGrassBaseDir( gisBase ) ) )
  {

    // ask user if he wants to specify GISBASE
    QMessageBox::StandardButton res = QMessageBox::warning( 0, QObject::tr( "GRASS plugin" ),
                                      QObject::tr( "QGIS couldn't find your GRASS installation.\n"
                                                   "Would you like to specify path (GISBASE) to your GRASS installation?" ),
                                      QMessageBox::Ok | QMessageBox::Cancel );

    if ( res != QMessageBox::Ok )
    {
      userGisbase = false;
      break;
    }

    // XXX Need to subclass this and add explantory message above to left side
    userGisbase = true;
    // For Mac, GISBASE folder may be inside GRASS bundle. Use Qt file dialog
    // since Mac native dialog doesn't allow user to browse inside bundles.
    gisBase = QFileDialog::getExistingDirectory(
                0, QObject::tr( "Choose GRASS installation path (GISBASE)" ), gisBase,
                QFileDialog::DontUseNativeDialog );
    if ( gisBase == QString::null )
    {
      // User pressed cancel. No GRASS for you!
      userGisbase = false;
      break;
    }
#ifdef Q_OS_WIN
    gisBase = shortPath( gisBase );
#endif
  }

  if ( !valid )
  {
    // warn user
    QMessageBox::information( 0, QObject::tr( "GRASS plugin" ),
                              QObject::tr( "GRASS data won't be available if GISBASE is not specified." ) );
  }

  if ( userGisbase )
  {
    settings.setValue( "/GRASS/gisbase", gisBase );
  }

  QgsDebugMsg( QString( "Valid GRASS gisBase is: %1" ).arg( gisBase ) );
  putEnv( "GISBASE", gisBase );

  // Add path to GRASS modules
#ifdef Q_OS_WIN
  QString sep = ";";
#else
  QString sep = ":";
#endif
  QString path = gisBase + "/bin";
  path.append( sep + gisBase + "/scripts" );
  path.append( sep +  QgsApplication::pkgDataPath() + "/grass/scripts/" );

  // On windows the GRASS libraries are in
  // QgsApplication::prefixPath(), we have to add them
  // to PATH to enable running of GRASS modules
  // and database drivers
#ifdef Q_OS_WIN
  // It seems that QgsApplication::prefixPath()
  // is not initialized at this point
  path.append( sep + shortPath( QCoreApplication::applicationDirPath() ) );

  // Add path to MSYS bin
  // Warning: MSYS sh.exe will translate this path to '/bin'
  if ( QFileInfo( QCoreApplication::applicationDirPath() + "/msys/bin/" ).isDir() )
    path.append( sep + shortPath( QCoreApplication::applicationDirPath() + "/msys/bin/" ) );
#endif

  QString p = getenv( "PATH" );
  path.append( sep + p );

  QgsDebugMsg( QString( "set PATH: %1" ).arg( path ) );
  putEnv( "PATH", path );

  // Set PYTHONPATH
  QString pythonpath = gisBase + "/etc/python";
  QString pp = getenv( "PYTHONPATH" );
  pythonpath.append( sep + pp );
  QgsDebugMsg( QString( "set PYTHONPATH: %1" ).arg( pythonpath ) );
  putEnv( "PYTHONPATH", pythonpath );

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
    pagers << "cat";

    for ( int i = 0; i < pagers.size(); i++ )
    {
      int state;

      QProcess p;
      p.start( pagers.at( i ) );
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
      putEnv( "GRASS_PAGER", pager );
    }
  }

  initialized = 1;
}

/*
 * Check if given directory contains a GRASS installation
 */
bool QgsGrass::isValidGrassBaseDir( const QString& gisBase )
{
  QgsDebugMsg( "isValidGrassBaseDir()" );
  // GRASS currently doesn't handle paths with blanks
  if ( gisBase.isEmpty() || gisBase.contains( " " ) )
  {
    return false;
  }

  /* TODO: G_is_gisbase() was added to GRASS 6.1 06-05-24,
           enable its use after some period (others do update) */
#if 0
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
    if ( G_is_gisbase( gisBase.toUtf8().constData() ) )
      return true;
  }
  else
  {
#endif
    QFileInfo gbi( gisBase + "/etc/element_list" );
    if ( gbi.exists() )
      return true;
#if 0
  }
#endif
  return false;
}

bool QgsGrass::activeMode()
{
  init();
  return active;
}

QString QgsGrass::getDefaultGisdbase()
{
  init();
  return defaultGisdbase;
}

QString QgsGrass::getDefaultLocation()
{
  init();
  return defaultLocation;
}

QString QgsGrass::getDefaultMapset()
{
  init();
  return defaultMapset;
}

void QgsGrass::setLocation( QString gisdbase, QString location )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  setMapset( gisdbase, location, "PERMANENT" );
}

void QgsGrass::setMapset( QString gisdbase, QString location, QString mapset )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2 mapset = %3" ).arg( gisdbase ).arg( location ).arg( mapset ) );
  init();

  // Set principal GRASS variables (in memory)
#ifdef Q_OS_WIN
  G__setenv( "GISDBASE", shortPath( gisdbase ).toUtf8().data() );
#else
  G__setenv( "GISDBASE", gisdbase.toUtf8().data() );
#endif
  G__setenv( "LOCATION_NAME", location.toUtf8().data() );
  G__setenv( "MAPSET", mapset.toUtf8().data() );

  // Add all available mapsets to search path
  char **ms = 0;
  G_TRY
  {
    ms = G_available_mapsets();
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( QString( "No available mapsets found: %1" ).arg( e.what() ) );
    return;
  }

  for ( int i = 0; ms[i]; i++ )
    G_add_mapset_to_search_path( ms[i] );
}

jmp_buf QgsGrass::jumper;

int QgsGrass::initialized = 0;

bool QgsGrass::active = 0;

QgsGrass::GERROR QgsGrass::lastError = QgsGrass::OK;

QString QgsGrass::error_message;

QString QgsGrass::defaultGisdbase;
QString QgsGrass::defaultLocation;
QString QgsGrass::defaultMapset;

QString QgsGrass::mMapsetLock;
QString QgsGrass::mGisrc;
QString QgsGrass::mTmp;

QMutex QgsGrass::sMutex;

int QgsGrass::error_routine( char *msg, int fatal )
{
  return error_routine(( const char* ) msg, fatal );
}

int QgsGrass::error_routine( const char *msg, int fatal )
{
  // G_fatal_error obviously is not thread safe (everything static in GRASS, especially fatal_jmp_buf)
  // it means that anything which may end up with G_fatal_error must use mutex

  // Unfortunately the exceptions thrown here can only be caught if GRASS libraries are compiled
  // with -fexception option on Linux (works on Windows)
  // GRASS developers are reluctant to add -fexception by default
  // https://trac.osgeo.org/grass/ticket/869
  QgsDebugMsg( QString( "error_routine (fatal = %1): %2" ).arg( fatal ).arg( msg ) );

  error_message = msg;

  if ( fatal )
  {
    QgsDebugMsg( "fatal -> longjmp" );
    // Exceptions cannot be thrown from here if GRASS lib is not compiled with -fexceptions
    //throw QgsGrass::Exception( QString::fromUtf8( msg ) );
    lastError = FATAL;

#if (GRASS_VERSION_MAJOR < 7)
    // longjump() is called by G_fatal_error in GRASS >= 7
    longjmp( QgsGrass::jumper, 1 );
#endif
  }
  else
  {
    lastError = WARNING;
  }

  return 1;
}

void GRASS_LIB_EXPORT QgsGrass::resetError( void )
{
  lastError = OK;
}

int GRASS_LIB_EXPORT QgsGrass::error( void )
{
  return lastError;
}

QString GRASS_LIB_EXPORT QgsGrass::errorMessage( void )
{
  return error_message;
}

QString GRASS_LIB_EXPORT QgsGrass::openMapset( const QString& gisdbase,
    const QString& location, const QString& mapset )
{
  QgsDebugMsg( QString( "gisdbase = %1" ).arg( gisdbase.toUtf8().constData() ) );
  QgsDebugMsg( QString( "location = %1" ).arg( location.toUtf8().constData() ) );
  QgsDebugMsg( QString( "mapset = %1" ).arg( mapset.toUtf8().constData() ) );

  closeMapset(); // close currently opened mapset (if any)

  QString mapsetPath = gisdbase + "/" + location + "/" + mapset;

  // Check if the mapset is in use
  QString gisBase = getenv( "GISBASE" );
  if ( gisBase.isEmpty() )
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

  QgsDebugMsg( QString( "pid = %1" ).arg( pid ) );

#ifndef Q_OS_WIN
  QFile lockFile( lock );
  QProcess process;
  QString lockProgram( gisBase + "/etc/lock" );
  QStringList lockArguments;
  lockArguments << lock << QString::number( pid );
  QString lockCommand = lockProgram + " " + lockArguments.join( " " ); // for debug
  QgsDebugMsg( "lock command: " + lockCommand );

  process.start( lockProgram, lockArguments );
  if ( !process.waitForStarted( 5000 ) )
  {
    return QObject::tr( "Cannot start %1" ).arg( lockCommand );
  }
  process.waitForFinished( 5000 );

  QString processResult = QString( "exitStatus=%1, exitCode=%2, errorCode=%3, error=%4 stdout=%5, stderr=%6" )
                          .arg( process.exitStatus() ).arg( process.exitCode() )
                          .arg( process.error() ).arg( process.errorString() )
                          .arg( process.readAllStandardOutput().data() ).arg( process.readAllStandardError().data() );
  QgsDebugMsg( "processResult: " + processResult );

  // lock exit code:
  // 0 - ok
  // 1 - error
  // 2 - mapset in use
  if ( process.exitCode() == 2 )
  {
    return QObject::tr( "Mapset is already in use." );
  }

  if ( process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0 )
  {
    QString message = QObject::tr( "Mapset lock failed" ) + " (" + processResult + ")";
    return message;
  }

#endif // Q_OS_WIN

  // Create temporary directory
  QFileInfo info( mapsetPath );
  QString user = info.owner();

  mTmp = QDir::tempPath() + "/grass6-" + user + "-" + QString::number( pid );
  QDir dir( mTmp );
  if ( dir.exists() )
  {
    QFileInfo dirInfo( mTmp );
    if ( !dirInfo.isWritable() )
    {
#ifndef Q_OS_WIN
      lockFile.remove();
#endif
      return QObject::tr( "Temporary directory %1 exists but is not writable" ).arg( mTmp );
    }
  }
  else if ( !dir.mkdir( mTmp ) )
  {
#ifndef Q_OS_WIN
    lockFile.remove();
#endif
    return QObject::tr( "Cannot create temporary directory %1" ).arg( mTmp );
  }

  // Create GISRC file
  QString globalGisrc =  QDir::home().path() + "/.grassrc6";
  mGisrc = mTmp + "/gisrc";

  QgsDebugMsg( QString( "globalGisrc = %1" ).arg( globalGisrc ) );
  QgsDebugMsg( QString( "mGisrc = %1" ).arg( mGisrc ) );

  QFile out( mGisrc );
  if ( !out.open( QIODevice::WriteOnly ) )
  {
#ifndef Q_OS_WIN
    lockFile.remove();
#endif
    return QObject::tr( "Cannot create %1" ).arg( mGisrc );
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
      if ( line.contains( "GISDBASE:" ) ||
           line.contains( "LOCATION_NAME:" ) ||
           line.contains( "MAPSET:" ) )
      {
        continue;
      }
      if ( line.contains( "GRASS_GUI:" ) )
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

  /* _Correct_ putenv() implementation is not making copy! */
  putEnv( "GISRC", mGisrc );

  // Reinitialize GRASS
  G__setenv( "GISRC", mGisrc.toUtf8().data() );
#ifdef Q_OS_WIN
  G__setenv( "GISDBASE", shortPath( gisdbase ).toLocal8Bit().data() );
#else
  G__setenv( "GISDBASE", gisdbase.toUtf8().data() );
#endif
  G__setenv( "LOCATION_NAME", location.toLocal8Bit().data() );
  G__setenv( "MAPSET", mapset.toLocal8Bit().data() );
  defaultGisdbase = gisdbase;
  defaultLocation = location;
  defaultMapset = mapset;

  active = true;

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

  mMapsetLock = lock;

  return NULL;
}

QString QgsGrass::closeMapset()
{
  QgsDebugMsg( "entered." );

  if ( mMapsetLock.length() > 0 )
  {
#ifndef Q_OS_WIN
    QFile file( mMapsetLock );
    if ( !file.remove() )
    {
      return QObject::tr( "Cannot remove mapset lock: %1" ).arg( mMapsetLock );
    }
#endif
    mMapsetLock = "";

    putenv(( char * ) "GISRC" );

    // Reinitialize GRASS
    G__setenv( "GISRC", ( char * ) "" );

    // Temporarily commented because of
    //   http://trac.osgeo.org/qgis/ticket/1900
    //   http://trac.osgeo.org/gdal/ticket/3313
    // it can be uncommented once GDAL with patch gets deployed (probably GDAL 1.8)
    //G__setenv( "GISDBASE", ( char * ) "" );
    //G__setenv( "LOCATION_NAME", ( char * ) "" );
    //G__setenv( "MAPSET", ( char * ) "" );

    defaultGisdbase = "";
    defaultLocation = "";
    defaultMapset = "";
    active = 0;

    // Delete temporary dir

    // To be sure that we don't delete '/' for example
    if ( mTmp.left( 4 ) == "/tmp" )
    {
      QDir dir( mTmp );
      for ( unsigned int i = 0; i < dir.count(); i++ )
      {
        if ( dir[i] == "." || dir[i] == ".." )
          continue;

        dir.remove( dir[i] );
        if ( dir.remove( dir[i] ) )
        {
          QgsDebugMsg( QString( "Cannot remove temporary file %1" ).arg( dir[i] ) );
        }
      }

      if ( !dir.rmdir( mTmp ) )
      {
        QgsDebugMsg( QString( "Cannot remove temporary directory %1" ).arg( mTmp ) );
      }
    }
  }

  return NULL;
}

QStringList GRASS_LIB_EXPORT QgsGrass::locations( const QString& gisdbase )
{
  QgsDebugMsg( QString( "gisdbase = %1" ).arg( gisdbase ) );

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

QStringList GRASS_LIB_EXPORT QgsGrass::mapsets( const QString& gisdbase, const QString& locationName )
{
  QgsDebugMsg( QString( "gisbase = %1 locationName = %2" ).arg( gisdbase ).arg( locationName ) );

  if ( gisdbase.isEmpty() || locationName.isEmpty() )
    return QStringList();

  return QgsGrass::mapsets( gisdbase + "/" + locationName );
}

QStringList GRASS_LIB_EXPORT QgsGrass::mapsets( const QString& locationPath )
{
  QgsDebugMsg( QString( "locationPath = %1" ).arg( locationPath ) );

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

QStringList GRASS_LIB_EXPORT QgsGrass::vectors( const QString& gisdbase, const QString& locationName,
    const QString& mapsetName )
{
  QgsDebugMsg( "entered." );

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

QStringList GRASS_LIB_EXPORT QgsGrass::vectors( const QString& mapsetPath )
{
  QgsDebugMsg( QString( "mapsetPath = %1" ).arg( mapsetPath ) );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/vector" );
  d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    /*
    if ( QFile::exists ( mapsetPath + "/vector/" + d[i] + "/head" ) )
    {
    list.append(d[i]);
    }
    */
    list.append( d[i] );
  }
  return list;
}

bool GRASS_LIB_EXPORT QgsGrass::topoVersion( const QString& gisdbase, const QString& location,
    const QString& mapset, const QString& mapName, int &major, int &minor )
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

QStringList GRASS_LIB_EXPORT QgsGrass::vectorLayers( const QString& gisdbase, const QString& location,
    const QString& mapset, const QString& mapName )
{
  GRASS_LOCK
  QgsDebugMsg( QString( "gisdbase = %1 location = %2 mapset = %3 mapName = %4" ).arg( gisdbase ).arg( location ).arg( mapset ).arg( mapName ) );
  QStringList list;

  // Set location
  QgsGrass::setLocation( gisdbase, location );

  /* Open vector */
  QgsGrass::resetError();
  //Vect_set_open_level( 2 );

  // TODO: We are currently using vectDestroyMapStruct in G_CATCH blocks because we know
  // that it does cannot call another G_fatal_error, but once we switch to hypothetical Vect_destroy_map_struct
  // it should be verified if it can still be in G_CATCH
  struct Map_info *map = vectNewMapStruct();
  int level = -1;

  // Vect_open_old_head GRASS is raising fatal error if topo exists but it is in different (older) version.
  // It means that even we could open it on level one, it ends with exception,
  // but we need level 2 anyway to get list of layers, so it does not matter, only the error message may be misleading.

  G_TRY
  {
    level = Vect_open_old_head( map, ( char * ) mapName.toUtf8().data(), ( char * ) mapset.toUtf8().data() );
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsDebugMsg( QString( "Cannot open GRASS vector: %1" ).arg( e.what() ) );
    vectDestroyMapStruct( map );
    GRASS_UNLOCK
    throw e;
  }

  // TODO: Handle errors as exceptions. Do not open QMessageBox here! This method is also used in browser
  // items which are populated in threads and creating dialog QPixmap is causing crash or even X server freeze.
  if ( level == 1 )
  {
    QgsDebugMsg( "Cannot open vector on level 2" );
    // Do not open QMessageBox here!
    //QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot open vector %1 in mapset %2 on level 2 (topology not available, try to rebuild topology using v.build module)." ).arg( mapName ).arg( mapset ) );
    // Vect_close here is correct, it should work, but it seems to cause
    // crash on win http://trac.osgeo.org/qgis/ticket/2003
    // disabled on win test it
#ifndef Q_OS_WIN
    Vect_close( map );
#endif
    vectDestroyMapStruct( map );
    GRASS_UNLOCK
    throw QgsGrass::Exception( QObject::tr( "Cannot open vector on level 2" ) );
  }
  else if ( level < 1 )
  {
    QgsDebugMsg( "Cannot open vector" );
    // Do not open QMessageBox here!
    //QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot open vector %1 in mapset %2" ).arg( mapName ).arg( mapset ) );
    vectDestroyMapStruct( map );
    GRASS_UNLOCK
    throw QgsGrass::Exception( QObject::tr( "Cannot open vector" ) );
  }

  QgsDebugMsg( "GRASS vector successfully opened" );

  G_TRY
  {
    // Get layers
    int ncidx = Vect_cidx_get_num_fields( map );

    for ( int i = 0; i < ncidx; i++ )
    {
      int field = Vect_cidx_get_field_number( map, i );
      QString fs;
      fs.sprintf( "%d", field );

      QgsDebugMsg( QString( "i = %1 layer = %2" ).arg( i ).arg( field ) );

      /* Points */
      int npoints = Vect_cidx_get_type_count( map, field, GV_POINT );
      QgsDebugMsg( QString( "npoints = %1" ).arg( npoints ) );
      if ( npoints > 0 )
      {
        QString l = fs + "_point";
        list.append( l );
      }

      /* Lines */
      /* Lines without category appears in layer 0, but not boundaries */
      int tp;
      if ( field == 0 )
        tp = GV_LINE;
      else
        tp = GV_LINE | GV_BOUNDARY;

      int nlines = Vect_cidx_get_type_count( map, field, tp );
      QgsDebugMsg( QString( "nlines = %1" ).arg( nlines ) );
      if ( nlines > 0 )
      {
        QString l = fs + "_line";
        list.append( l );
      }

      /* Faces */
      int nfaces = Vect_cidx_get_type_count( map, field, GV_FACE );
      QgsDebugMsg( QString( "nfaces = %1" ).arg( nfaces ) );
      if ( nfaces > 0 )
      {
        QString l = fs + "_face";
        list.append( l );
      }

      /* Polygons */
      int nareas = Vect_cidx_get_type_count( map, field, GV_AREA );
      QgsDebugMsg( QString( "nareas = %1" ).arg( nareas ) );
      if ( nareas > 0 )
      {
        QString l = fs + "_polygon";
        list.append( l );
      }
    }
    QgsDebugMsg( "standard layers listed: " + list.join( "," ) );

    // TODO: add option in GUI to set listTopoLayers
    QSettings settings;
    bool listTopoLayers =  settings.value( "/GRASS/listTopoLayers", false ).toBool();
    if ( listTopoLayers )
    {
      // add topology layers
      if ( Vect_get_num_primitives( map, GV_POINTS ) > 0 )
      {
#if GRASS_VERSION_MAJOR < 7 /* no more point in GRASS 7 topo */
        list.append( "topo_point" );
#endif
      }
      if ( Vect_get_num_primitives( map, GV_LINES ) > 0 )
      {
        list.append( "topo_line" );
      }
      if ( Vect_get_num_nodes( map ) > 0 )
      {
        list.append( "topo_node" );
      }
    }
    Vect_close( map );
    vectDestroyMapStruct( map );
    GRASS_UNLOCK
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    QgsDebugMsg( QString( "Cannot get vector layers: %1" ).arg( e.what() ) );
    vectDestroyMapStruct( map );
    GRASS_UNLOCK
    throw e;
  }

  return list;
}

QStringList GRASS_LIB_EXPORT QgsGrass::rasters( const QString& gisdbase, const QString& locationName,
    const QString& mapsetName )
{
  QgsDebugMsg( "entered." );

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

QStringList GRASS_LIB_EXPORT QgsGrass::rasters( const QString& mapsetPath )
{
  QgsDebugMsg( QString( "mapsetPath = %1" ).arg( mapsetPath ) );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/cellhd" );
  d.setFilter( QDir::Files );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    list.append( d[i] );
  }
  return list;
}

QStringList GRASS_LIB_EXPORT QgsGrass::groups( const QString& gisdbase, const QString& locationName,
    const QString& mapsetName )
{
  return elements( gisdbase, locationName, mapsetName, "group" );
}

QStringList GRASS_LIB_EXPORT QgsGrass::groups( const QString& mapsetPath )
{
  return elements( mapsetPath, "group" );
}

QStringList GRASS_LIB_EXPORT QgsGrass::elements( const QString& gisdbase, const QString& locationName,
    const QString& mapsetName, const QString& element )
{
  if ( gisdbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
  {
    return QStringList();
  }

  return QgsGrass::elements( gisdbase + "/" + locationName + "/" + mapsetName, element );
}

QStringList GRASS_LIB_EXPORT QgsGrass::elements( const QString&  mapsetPath, const QString&  element )
{
  QgsDebugMsg( QString( "mapsetPath = %1 element = %2" ).arg( mapsetPath ).arg( element ) );

  QStringList list;

  if ( mapsetPath.isEmpty() )
    return list;

  QDir d = QDir( mapsetPath + "/" + element );
  if ( element == "vector" || element == "group" )
  {
    d.setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
  }
  else
  {
    d.setFilter( QDir::Files );
  }

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    list.append( d[i] );
  }
  return list;
}

QStringList GRASS_LIB_EXPORT QgsGrass::grassObjects( const QString& mapsetPath, QgsGrassObject::Type type )
{
  return QgsGrass::elements( mapsetPath, QgsGrassObject::dirName( type ) );
}

bool GRASS_LIB_EXPORT QgsGrass::objectExists( const QgsGrassObject& grassObject )
{
  QString path = grassObject.mapsetPath() + "/" + QgsGrassObject::dirName( grassObject.type() )
                 + "/" + grassObject.name();
  QFileInfo fi( path );
  return fi.exists();
}

QString GRASS_LIB_EXPORT QgsGrass::regionString( const struct Cell_head *window )
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


bool GRASS_LIB_EXPORT QgsGrass::defaultRegion( const QString& gisdbase, const QString& location,
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
    return false;
  }
}

bool GRASS_LIB_EXPORT QgsGrass::region( const QString& gisdbase,
                                        const QString& location, const QString& mapset,
                                        struct Cell_head *window )
{
  QgsGrass::setLocation( gisdbase, location );

#if GRASS_VERSION_MAJOR < 7
  if ( G__get_window( window, ( char * ) "", ( char * ) "WIND", mapset.toUtf8().data() ) )
  {
    return false;
  }
#else
  // TODO7: unfortunately G__get_window does not return error code and calls G_fatal_error on error
  G__get_window( window, ( char * ) "", ( char * ) "WIND", mapset.toUtf8().data() );
#endif
  return true;
}

bool GRASS_LIB_EXPORT QgsGrass::writeRegion( const QString& gisbase,
    const QString& location, const QString& mapset,
    const struct Cell_head *window )
{
  QgsDebugMsg( "entered." );
  QgsDebugMsg( QString( "n = %1 s = %2" ).arg( window->north ).arg( window->south ) );
  QgsDebugMsg( QString( "e = %1 w = %2" ).arg( window->east ).arg( window->west ) );

  QgsGrass::setMapset( gisbase, location, mapset );

  if ( G_put_window( window ) == -1 )
  {
    return false;
  }

  return true;
}

void GRASS_LIB_EXPORT QgsGrass::copyRegionExtent( struct Cell_head *source,
    struct Cell_head *target )
{
  target->north = source->north;
  target->south = source->south;
  target->east = source->east;
  target->west = source->west;
  target->top = source->top;
  target->bottom = source->bottom;
}

void GRASS_LIB_EXPORT QgsGrass::copyRegionResolution( struct Cell_head *source,
    struct Cell_head *target )
{
  target->ns_res = source->ns_res;
  target->ew_res = source->ew_res;
  target->tb_res = source->tb_res;
  target->ns_res3 = source->ns_res3;
  target->ew_res3 = source->ew_res3;
}

void GRASS_LIB_EXPORT QgsGrass::extendRegion( struct Cell_head *source,
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

void GRASS_LIB_EXPORT QgsGrass::initRegion( struct Cell_head *window )
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

void GRASS_LIB_EXPORT QgsGrass::setRegion( struct Cell_head *window, QgsRectangle rect )
{
  window->west = rect.xMinimum();
  window->south = rect.yMinimum();
  window->east = rect.xMaximum();
  window->north = rect.yMaximum();
}

QString GRASS_LIB_EXPORT QgsGrass::setRegion( struct Cell_head *window, QgsRectangle rect, int rows, int cols )
{
  initRegion( window );
  window->west = rect.xMinimum();
  window->south = rect.yMinimum();
  window->east = rect.xMaximum();
  window->north = rect.yMaximum();
  window->rows = rows;
  window->cols = cols;

  QString error;
#if GRASS_VERSION_MAJOR < 7
  char* err = G_adjust_Cell_head( window, 1, 1 );
  if ( err )
  {
    error = QString( err );
  }
#else
  try
  {
    G_adjust_Cell_head( window, 1, 1 );
  }
  catch ( QgsGrass::Exception &e )
  {
    error = e.what();
  }
#endif
  return error;
}

QgsRectangle GRASS_LIB_EXPORT QgsGrass::extent( struct Cell_head *window )
{
  if ( !window )
  {
    return QgsRectangle();
  }
  return QgsRectangle( window->west, window->south, window->east, window->north );
}

bool GRASS_LIB_EXPORT QgsGrass::mapRegion( QgsGrassObject::Type type, QString gisdbase,
    QString location, QString mapset, QString map,
    struct Cell_head *window )
{
  QgsDebugMsg( "entered." );
  QgsDebugMsg( QString( "map = %1" ).arg( map ) );
  QgsDebugMsg( QString( "mapset = %1" ).arg( mapset ) );

  QgsGrass::setLocation( gisdbase, location );

  if ( type == QgsGrassObject::Raster )
  {

#if GRASS_VERSION_MAJOR < 7
    if ( G_get_cellhd( map.toUtf8().data(),
                       mapset.toUtf8().data(), window ) < 0 )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read raster map region" ) );
      return false;
    }
#else
    // TODO7: unfortunately Rast_get_cellhd does not return error code and calls G_fatal_error on error
    Rast_get_cellhd( map.toUtf8().data(), mapset.toUtf8().data(), window );
#endif
  }
  else if ( type == QgsGrassObject::Vector )
  {
    // Get current projection
    if ( !region( gisdbase, location, mapset, window ) )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read vector map region" ) );
      return false;
    }

    struct Map_info Map;

    int level = Vect_open_old_head( &Map,
                                    map.toUtf8().data(), mapset.toUtf8().data() );

    if ( level < 2 )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read vector map region" ) );
      return false;
    }

    BOUND_BOX box;
    Vect_get_map_box( &Map, &box );
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

    Vect_close( &Map );
  }
  else if ( type == QgsGrassObject::Region )
  {
#if GRASS_VERSION_MAJOR < 7
    if ( G__get_window( window, ( char * ) "windows",
                        map.toUtf8().data(),
                        mapset.toUtf8().data() ) != NULL )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read region" ) );
      return false;
    }
#else
    // TODO7: unfortunately G__get_window does not return error code and calls G_fatal_error on error
    G__get_window( window, ( char * ) "windows", map.toUtf8().data(), mapset.toUtf8().data() );
#endif
  }
  return true;
}

QProcess GRASS_LIB_EXPORT *QgsGrass::startModule( const QString& gisdbase, const QString&  location,
    const QString&  mapset, const QString& moduleName, const QStringList& arguments,
    QTemporaryFile &gisrcFile, bool qgisModule )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  QProcess *process = new QProcess();

  QString module = moduleName;
  if ( qgisModule )
  {
    module += QString::number( QgsGrass::versionMajor() );
  }
#ifdef Q_OS_WIN
  module += ".exe";
#endif

  // We have to set GISRC file, uff
  if ( !gisrcFile.open() )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot open GISRC file" ) );
  }

  QTextStream out( &gisrcFile );
  out << "GISDBASE: " << gisdbase << "\n";
  out << "LOCATION_NAME: " << location << "\n";
  //out << "MAPSET: PERMANENT\n";
  out << "MAPSET: " << mapset << "\n";
  out.flush();
  QgsDebugMsg( gisrcFile.fileName() );
  gisrcFile.close();

  QStringList environment = QProcess::systemEnvironment();
  environment.append( "GISRC=" + gisrcFile.fileName() );

  process->setEnvironment( environment );

  QgsDebugMsg( module + " " + arguments.join( " " ) );
  //process->start( module, arguments, QProcess::Unbuffered );
  process->start( module, arguments );
  if ( !process->waitForStarted() )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot start module" ) + "\n"
                               + QObject::tr( "command: %1 %2" )
                               .arg( module ).arg( arguments.join( " " ) ) );
  }
  return process;
}

QByteArray GRASS_LIB_EXPORT QgsGrass::runModule( const QString& gisdbase, const QString&  location,
    const QString& mapset, const QString&  moduleName,
    const QStringList& arguments, int timeOut, bool qgisModule )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2 timeOut = %3" ).arg( gisdbase ).arg( location ).arg( timeOut ) );

  QTemporaryFile gisrcFile;
  QProcess *process = QgsGrass::startModule( gisdbase, location, mapset, moduleName, arguments, gisrcFile, qgisModule );

  if ( !process->waitForFinished( timeOut )
       || ( process->exitCode() != 0 && process->exitCode() != 255 ) )
  {
    QgsDebugMsg( "process->exitCode() = " + QString::number( process->exitCode() ) );

    throw QgsGrass::Exception( QObject::tr( "Cannot run module" ) + "\n"
                               + QObject::tr( "command: %1 %2\nstdout: %3\nstderr: %4" )
                               .arg( moduleName ).arg( arguments.join( " " ) )
                               .arg( process->readAllStandardOutput().constData() )
                               .arg( process->readAllStandardError().constData() ) );
  }
  QByteArray data = process->readAllStandardOutput();
  delete process;
  return data;
}

QString GRASS_LIB_EXPORT QgsGrass::getInfo( const QString&  info, const QString&  gisdbase,
    const QString&  location, const QString&  mapset,
    const QString&  map, const QgsGrassObject::Type type,
    double x, double y,
    const QgsRectangle& extent, int sampleRows,
    int sampleCols, int timeOut )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );

  QStringList arguments;

  QString cmd = qgisGrassModulePath() + "/qgis.g.info";

  arguments.append( "info=" + info );
  if ( !map.isEmpty() )
  {
    QString opt;
    switch ( type )
    {
      case QgsGrassObject::Raster:
        opt = "rast";
        break;
      case QgsGrassObject::Vector:
        opt = "vect";
        break;
      default:
        QgsDebugMsg( QString( "unexpected type:%1" ).arg( type ) );
        return "";
    }
    arguments.append( opt + "=" +  map + "@" + mapset );
  }
  if ( info == "query" )
  {
    arguments.append( QString( "coor=%1,%2" ).arg( x ).arg( y ) );
  }
  if ( info == "stats" )
  {
    arguments.append( QString( "north=%1" ).arg( extent.yMaximum() ) );
    arguments.append( QString( "south=%1" ).arg( extent.yMinimum() ) );
    arguments.append( QString( "east=%1" ).arg( extent.xMaximum() ) );
    arguments.append( QString( "west=%1" ).arg( extent.xMinimum() ) );
    arguments.append( QString( "rows=%1" ).arg( sampleRows ) );
    arguments.append( QString( "cols=%1" ).arg( sampleCols ) );
  }

  QByteArray data =  QgsGrass::runModule( gisdbase, location, mapset, cmd, arguments, timeOut );
  QgsDebugMsg( data );
  return QString( data );
}

QgsCoordinateReferenceSystem GRASS_LIB_EXPORT QgsGrass::crs( const QString& gisdbase, const QString& location,
    bool interactive )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem();
  try
  {
    QString wkt = QgsGrass::getInfo( "proj", gisdbase, location );
    QgsDebugMsg( "wkt: " + wkt );
    crs.createFromWkt( wkt );
    QgsDebugMsg( "crs.toWkt: " + crs.toWkt() );
  }
  catch ( QgsGrass::Exception &e )
  {
    if ( interactive )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot get projection " ) + "\n" + e.what() );
    }
  }

  return crs;
}

QgsCoordinateReferenceSystem GRASS_LIB_EXPORT QgsGrass::crsDirect( const QString& gisdbase, const QString& location )
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
    G_CATCH( QgsGrass::Exception &e )
    {
      Q_UNUSED( e );
      QgsDebugMsg( QString( "Cannot get default window: %1" ).arg( e.what() ) );
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

  QgsCoordinateReferenceSystem srs;
  srs.createFromWkt( Wkt );

  return srs;
}

QgsRectangle GRASS_LIB_EXPORT QgsGrass::extent( const QString& gisdbase, const QString& location,
    const QString& mapset, const QString& map,
    QgsGrassObject::Type type, bool interactive )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );

  try
  {
    QString str = QgsGrass::getInfo( "window", gisdbase, location, mapset, map, type );
    QStringList list = str.split( "," );
    if ( list.size() != 4 )
    {
      throw QgsGrass::Exception( "Cannot parse GRASS map extent: " + str );
    }
    return QgsRectangle( list[0].toDouble(), list[1].toDouble(), list[2].toDouble(), list[3].toDouble() );
  }
  catch ( QgsGrass::Exception &e )
  {
    if ( interactive )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot get raster extent" ) + "\n" + e.what() );
    }
  }
  return QgsRectangle( 0, 0, 0, 0 );
}

void GRASS_LIB_EXPORT QgsGrass::size( const QString& gisdbase, const QString& location,
                                      const QString& mapset, const QString& map, int *cols, int *rows )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );

  *cols = 0;
  *rows = 0;
  try
  {
    QString str = QgsGrass::getInfo( "size", gisdbase, location, mapset, map, QgsGrassObject::Raster );
    QStringList list = str.split( "," );
    if ( list.size() != 2 )
    {
      throw QgsGrass::Exception( "Cannot parse GRASS map size: " + str );
    }
    *cols = list[0].toInt();
    *rows = list[1].toInt();
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot get raster extent" ) + "\n" + e.what() );
  }

  QgsDebugMsg( QString( "raster size = %1 %2" ).arg( *cols ).arg( *rows ) );
}

QHash<QString, QString> GRASS_LIB_EXPORT QgsGrass::info( const QString& gisdbase, const QString& location,
    const QString& mapset, const QString& map,
    QgsGrassObject::Type type,
    const QString& info,
    const QgsRectangle& extent,
    int sampleRows, int sampleCols,
    int timeOut, bool interactive )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  QHash<QString, QString> inf;

  try
  {
    QString str = QgsGrass::getInfo( info, gisdbase, location, mapset, map, type, 0, 0, extent, sampleRows, sampleCols, timeOut );
    QgsDebugMsg( str );
    QStringList list = str.split( "\n" );
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
    if ( interactive )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot get map info" ) + "\n" + e.what() );
    }
  }
  return inf;
}

QList<QgsGrass::Color> GRASS_LIB_EXPORT QgsGrass::colors( QString gisdbase, QString location, QString mapset, QString map )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  QList<QgsGrass::Color> ct;

  try
  {
    QString str = QgsGrass::getInfo( "colors", gisdbase, location, mapset, map, QgsGrassObject::Raster );
    QgsDebugMsg( str );
    QStringList list = str.split( "\n" );
    for ( int i = 0; i < list.size(); i++ )
    {
      QgsGrass::Color c;
      if ( list[i].isEmpty() )
        continue;
      if ( sscanf( list[i].toUtf8().data(), "%lf %lf %d %d %d %d %d %d", &( c.value1 ), &( c.value2 ), &( c.red1 ), &( c.green1 ), &( c.blue1 ), &( c.red2 ), &( c.green2 ), &( c.blue2 ) ) != 8 )
      {
        throw QgsGrass::Exception( "Cannot parse GRASS colors" + list[i] + " (" + str + " ) " );
      }
      ct.append( c );
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot get colors" ) + "\n" + e.what() );
  }
  return ct;
}

QMap<QString, QString> GRASS_LIB_EXPORT QgsGrass::query( QString gisdbase, QString location, QString mapset, QString map, QgsGrassObject::Type type, double x, double y )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );

  QMap<QString, QString> result;
  // TODO: multiple values (more rows)
  try
  {
    QString str = QgsGrass::getInfo( "query", gisdbase, location, mapset, map, type, x, y );
    QStringList list = str.trimmed().split( ":" );
    if ( list.size() == 2 )
    {
      result[list[0]] = list[1];
    }
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot query raster " ) + "\n" + e.what() );
  }
  return result;
}

void GRASS_LIB_EXPORT QgsGrass::renameObject( const QgsGrassObject & object, const QString& newName )
{
  QgsDebugMsg( "entered" );
  QString cmd = "g.rename";
  QStringList arguments;

  arguments << object.elementShort() + "=" + object.name() + "," + newName;

  int timeout = -1; // What timeout to use? It can take long time on network or database
  // throws QgsGrass::Exception
  QgsGrass::runModule( object.gisdbase(), object.location(), object.mapset(), cmd, arguments, timeout, false );
}

void GRASS_LIB_EXPORT QgsGrass::copyObject( const QgsGrassObject & srcObject, const QgsGrassObject & destObject )
{
  QgsDebugMsg( "srcObject = " + srcObject.toString() );
  QgsDebugMsg( "destObject = " + destObject.toString() );

  if ( !srcObject.locationIdentical( destObject ) ) // should not happen
  {
    throw QgsGrass::Exception( QObject::tr( "Attempt to copy from different location." ) );
  }

  QString cmd = "g.copy";
  QStringList arguments;

  arguments << srcObject.elementShort() + "=" + srcObject.name() + "@" + srcObject.mapset() + "," + destObject.name();

  int timeout = -1; // What timeout to use? It can take long time on network or database
  // throws QgsGrass::Exception
  // TODO: g.copy does not seem to return error code if fails (6.4.3RC1)
  QgsGrass::runModule( destObject.gisdbase(), destObject.location(), destObject.mapset(), cmd, arguments, timeout, false );
}

bool GRASS_LIB_EXPORT QgsGrass::deleteObject( const QgsGrassObject & object )
{
  QgsDebugMsg( "entered" );

  // TODO: check if user has permissions

  /*
  if ( QMessageBox::question( this, tr( "Question" ),
                              tr( "Are you sure you want to delete %n selected layer(s)?", "number of layers to delete", indexes.size() ),
                              QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
  {
    return false;
  }
  */

  // path to g.remove should be in PATH (added by QgsGrass::init())
  QString cmd = "g.remove";
  QStringList arguments;

#if GRASS_VERSION_MAJOR < 7
  arguments << object.elementShort() + "=" + object.name();
#else
  arguments << "-f" << "type=" + object.elementShort() << "name=" + object.name();
#endif

  try
  {
    QgsGrass::runModule( object.gisdbase(), object.location(), object.mapset(), cmd, arguments, 5000, false );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Cannot delete" ) + " " + object.elementName()
                          + " " + object.name() + ": " + e.what() );
    return false;
  }
  return true;
}

bool GRASS_LIB_EXPORT QgsGrass::deleteObjectDialog( const QgsGrassObject & object )
{
  QgsDebugMsg( "entered" );

  return QMessageBox::question( 0, QObject::tr( "Delete confirmation" ),
                                QObject::tr( "Are you sure you want to delete %1 %2?" ).arg( object.elementName() ).arg( object.name() ),
                                QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes;
}

void GRASS_LIB_EXPORT QgsGrass::createTable( dbDriver *driver, const QString tableName, const QgsFields &fields )
{
  if ( !driver ) // should not happen
  {
    throw QgsGrass::Exception( "driver is null" );
  }

  QStringList fieldsStringList;
  for ( int i = 0; i < fields.size(); i++ )
  {
    QgsField field = fields.field( i );
    QString name = field.name().toLower().replace( " ", "_" );
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
        typeName = "integer";
        break;
      case QVariant::Double:
        typeName = "double precision";
        break;
        // TODO: verify how is it with spatialite/dbf support for date, time, datetime, v.in.ogr is using all
      case QVariant::Date:
        typeName = "date";
        break;
      case QVariant::Time:
        typeName = "time";
        break;
      case QVariant::DateTime:
        typeName = "datetime";
        break;
      case QVariant::String:
        typeName = QString( "varchar (%1)" ).arg( field.length() );
        break;
      default:
        typeName = QString( "varchar (%1)" ).arg( field.length() > 0 ? field.length() : 255 );
    }
    fieldsStringList <<  name + " " + typeName;
  }
  QString sql = QString( "create table %1 (%2);" ).arg( tableName ).arg( fieldsStringList.join( ", " ) );

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  int result = db_execute_immediate( driver, &dbstr );
  db_free_string( &dbstr );
  if ( result != DB_OK )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot create table" ) + ": " + QString::fromLatin1( db_get_error_msg() ) );
  }
}

void GRASS_LIB_EXPORT QgsGrass::insertRow( dbDriver *driver, const QString tableName,
    const QgsAttributes& attributes )
{
  if ( !driver ) // should not happen
  {
    throw QgsGrass::Exception( "driver is null" );
  }

  QStringList valuesStringList;
  foreach ( QVariant attribute, attributes )
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
        valueString = attribute.toBool() ? "1" : "0";
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
    valueString.replace( "'", "''" );

    if ( quote )
    {
      valueString = "'" + valueString + "'";
    }

    valuesStringList <<  valueString;
  }
  QString sql = QString( "insert into %1 values (%2);" ).arg( tableName ).arg( valuesStringList.join( ", " ) );

  dbString dbstr;
  db_init_string( &dbstr );
  db_set_string( &dbstr, sql.toLatin1().data() );

  int result = db_execute_immediate( driver, &dbstr );
  db_free_string( &dbstr );
  if ( result != DB_OK )
  {
    throw QgsGrass::Exception( QObject::tr( "Cannot insert, statement" ) + ": " + sql
                               + QObject::tr( "error" ) + ": " + QString::fromLatin1( db_get_error_msg() ) );
  }
}

bool GRASS_LIB_EXPORT QgsGrass::isExternal( const QgsGrassObject & object )
{
  if ( object.type() != QgsGrassObject::Raster )
  {
    return false;
  }
  bool isExternal = false;
  QgsGrass::setLocation( object.gisdbase(), object.location() );
  struct GDAL_link *gdal;
  gdal = G_get_gdal_link( object.name().toUtf8().data(), object.mapset().toUtf8().data() );
  if ( gdal )
  {
    isExternal = true;
    G_close_gdal_link( gdal );
  }
  return isExternal;
}

// GRASS version constants have been changed on 26.4.2007
// http://freegis.org/cgi-bin/viewcvs.cgi/grass6/include/version.h.in.diff?r1=1.4&r2=1.5
// The following lines workaround this change

int GRASS_LIB_EXPORT QgsGrass::versionMajor()
{
#ifdef GRASS_VERSION_MAJOR
  return GRASS_VERSION_MAJOR;
#else
  return QString( GRASS_VERSION_MAJOR ).toInt();
#endif
}

int GRASS_LIB_EXPORT QgsGrass::versionMinor()
{
#ifdef GRASS_VERSION_MINOR
  return GRASS_VERSION_MINOR;
#else
  return QString( GRASS_VERSION_MINOR ).toInt();
#endif
}

int GRASS_LIB_EXPORT QgsGrass::versionRelease()
{
#ifdef GRASS_VERSION_RELEASE
#define QUOTE(x)  #x
  return QString( QUOTE( GRASS_VERSION_RELEASE ) ).toInt();
#else
  return QString( GRASS_VERSION_RELEASE ).toInt();
#endif
}
QString GRASS_LIB_EXPORT QgsGrass::versionString()
{
  return QString( GRASS_VERSION_STRING );
}

Qt::CaseSensitivity GRASS_LIB_EXPORT QgsGrass::caseSensitivity()
{
#ifdef Q_OS_WIN
  return Qt::CaseInsensitive;
#else
  return Qt::CaseSensitive;
#endif
}

bool GRASS_LIB_EXPORT QgsGrass::isLocation( const QString& path )
{
  return G_is_location( path.toUtf8().constData() ) == 1;
}

bool GRASS_LIB_EXPORT QgsGrass::isMapset( const QString& path )
{
  return G_is_mapset( path.toUtf8().constData() ) == 1;
}

QString GRASS_LIB_EXPORT QgsGrass::lockFilePath()
{
  return mMapsetLock;
}

QString GRASS_LIB_EXPORT QgsGrass::gisrcFilePath()
{
  if ( mGisrc.isEmpty() )
  {
    // Started from GRASS shell
    if ( getenv( "GISRC" ) )
    {
      return QString( getenv( "GISRC" ) );
    }
  }
  return mGisrc;
}

void GRASS_LIB_EXPORT QgsGrass::putEnv( QString name, QString value )
{
  QString env = name + "=" + value;
  /* _Correct_ putenv() implementation is not making copy! */
  char *envChar = new char[env.toUtf8().length()+1];
  strcpy( envChar, env.toUtf8().constData() );
  putenv( envChar );
}

struct Map_info GRASS_LIB_EXPORT *QgsGrass::vectNewMapStruct()
{
  // In OSGeo4W there is GRASS compiled by MinGW while QGIS compiled by MSVC, the compilers
  // may have different sizes of types, see issue #13002. Because there is no Vect_new_map_struct (GRASS 7.0.0, July 2015)
  // the structure is allocated here using doubled (should be enough) space.
  // TODO: replace by Vect_new_map_struct once it appears in GRASS
#ifdef Q_OS_WIN
  return ( struct Map_info* ) qgsMalloc( 2*sizeof( struct Map_info ) );
#else
  return ( struct Map_info* ) qgsMalloc( sizeof( struct Map_info ) );
#endif
}

void GRASS_LIB_EXPORT QgsGrass::vectDestroyMapStruct( struct Map_info *map )
{
  // TODO: replace by Vect_destroy_map_struct once it appears in GRASS
  // TODO: until switch to hypothetical Vect_destroy_map_struct verify that Vect_destroy_map_struct cannot
  // call G_fatal_error, otherwise check and remove use of vectDestroyMapStruct from G_CATCH blocks
  qgsFree( map );
  map = 0;
}

void GRASS_LIB_EXPORT QgsGrass::sleep( int ms )
{
// Stolen from QTest::qSleep
#ifdef Q_OS_WIN
  Sleep( uint( ms ) );
#else
  struct timespec ts = { ms / 1000, ( ms % 1000 ) * 1000 * 1000 };
  nanosleep( &ts, NULL );
#endif
}
