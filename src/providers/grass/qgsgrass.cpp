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
/* $Id$ */

#include "qgsgrass.h"

#include "qgslogger.h"
#include "qgsapplication.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextStream>

#include <QTextCodec>

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/Vect.h>
#include <grass/version.h>
}

#if defined(WIN32)
#include <windows.h>
QString GRASS_EXPORT QgsGrass::shortPath( const QString &path )
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
  return res;
}
#endif

void GRASS_EXPORT QgsGrass::init( void )
{
  // Warning!!!
  // G_set_error_routine() once called from plugin
  // is not valid in provider -> call it always

  // Set error function
  G_set_error_routine( &error_routine );

  if ( initialized )
    return;

  QSettings settings;

  // Is it active mode ?
  if ( getenv( "GISRC" ) )
  {
    active = true;
    // Store default values
    defaultGisdbase = G_gisdbase();
    defaultLocation = G_location();
    defaultMapset = G_mapset();
  }
  else
  {
    active = false;
  }

  // Don't use GISRC file and read/write GRASS variables (from location G_VAR_GISRC) to memory only.
  G_set_gisrc_mode( G_GISRC_MODE_MEMORY );

  // Init GRASS libraries (required)
  G_no_gisinit();  // Doesn't check write permissions for mapset compare to G_gisinit("libgrass++");

  // Set program name
  G_set_program_name( "QGIS" );

  // Require GISBASE to be set. This should point to the location of
  // the GRASS installation. The GRASS libraries use it to know
  // where to look for things.

  // Look first to see if GISBASE env var is already set.
  // This is set when QGIS is run from within GRASS
  // or when set explicitly by the user.
  // This value should always take precedence.
#if WIN32
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

#ifdef WIN32
    // Use the applicationDirPath()/grass
    gisBase = shortPath( QCoreApplication::applicationDirPath() + "/grass" );
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
#if defined(WIN32)
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
  QString gisBaseEnv = "GISBASE=" + gisBase;
  /* _Correct_ putenv() implementation is not making copy! */
  char *gisBaseEnvChar = new char[gisBaseEnv.toUtf8().length()+1];
  strcpy( gisBaseEnvChar, gisBaseEnv.toUtf8().constData() );
  putenv( gisBaseEnvChar );

  // Add path to GRASS modules
#ifdef WIN32
  QString sep = ";";
#else
  QString sep = ":";
#endif
  QString path = "PATH=" + gisBase + "/bin";
  path.append( sep + gisBase + "/scripts" );
  path.append( sep +  QgsApplication::pkgDataPath() + "/grass/scripts/" );

  // On windows the GRASS libraries are in
  // QgsApplication::prefixPath(), we have to add them
  // to PATH to enable running of GRASS modules
  // and database drivers
#ifdef WIN32
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
  char *pathEnvChar = new char[path.toUtf8().length()+1];
  strcpy( pathEnvChar, path.toUtf8().constData() );
  putenv( pathEnvChar );

  // Set PYTHONPATH
  QString pythonpath = "PYTHONPATH=" + gisBase + "/etc/python";
  QString pp = getenv( "PATH" );
  pythonpath.append( sep + pp );
  QgsDebugMsg( QString( "set PYTHONPATH: %1" ).arg( pythonpath ) );
  char *pythonpathEnvChar = new char[pythonpath.toUtf8().length()+1];
  strcpy( pythonpathEnvChar, pythonpath.toUtf8().constData() );
  putenv( pythonpathEnvChar );

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
      pager.prepend( "GRASS_PAGER=" );
      char *pagerEnvChar = new char[pager.length()+1];
      strcpy( pagerEnvChar, pager.toUtf8().constData() );
      putenv( pagerEnvChar );
    }
  }

  initialized = 1;
}

/*
 * Check if given directory contains a GRASS installation
 */
bool QgsGrass::isValidGrassBaseDir( QString const gisBase )
{
  QgsDebugMsg( "isValidGrassBaseDir()" );
  // GRASS currently doesn't handle paths with blanks
  if ( gisBase.isEmpty() || gisBase.contains( " " ) )
  {
    return FALSE;
  }

  /* TODO: G_is_gisbase() was added to GRASS 6.1 06-05-24,
           enable its use after some period (others do update) */
  /*
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
  if ( G_is_gisbase( gisBase.toUtf8().constData() ) ) return TRUE;
  }
  else
  {
  */
  QFileInfo gbi( gisBase + "/etc/element_list" );
  if ( gbi.exists() ) return TRUE;
  //}
  return FALSE;
}

bool QgsGrass::activeMode( void )
{
  init();
  return active;
}

QString QgsGrass::getDefaultGisdbase( void )
{
  init();
  return defaultGisdbase;
}

QString QgsGrass::getDefaultLocation( void )
{
  init();
  return defaultLocation;
}

QString QgsGrass::getDefaultMapset( void )
{
  init();
  return defaultMapset;
}

void QgsGrass::setLocation( QString gisdbase, QString location )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2" ).arg( gisdbase ).arg( location ) );
  init();

  // Set principal GRASS variables (in memory)
// #if defined(WIN32)
//  G__setenv(( char * ) "GISDBASE", shortPath( gisdbase ).toLocal8Bit().data() );
//#else
  G__setenv(( char * ) "GISDBASE", gisdbase.toAscii().constData() );
//#endif
  G__setenv(( char * ) "LOCATION_NAME", location.toAscii().constData() );
  G__setenv(( char * ) "MAPSET", ( char * ) "PERMANENT" ); // PERMANENT must always exist

  // Add all available mapsets to search path
  char **ms = G_available_mapsets();
  for ( int i = 0; ms[i]; i++ )  G_add_mapset_to_search_path( ms[i] );
}

void QgsGrass::setMapset( QString gisdbase, QString location, QString mapset )
{
  QgsDebugMsg( QString( "gisdbase = %1 location = %2 mapset = %3" ).arg( gisdbase ).arg( location ).arg( mapset ) );
  init();

  // Set principal GRASS variables (in memory)
// #if defined(WIN32)
//  G__setenv(( char * ) "GISDBASE", shortPath( gisdbase ).toUtf8().data() );
// #else
  G__setenv(( char * ) "GISDBASE", gisdbase.toUtf8().data() );
// #endif
  G__setenv(( char * ) "LOCATION_NAME", location.toUtf8().data() );
  G__setenv(( char * ) "MAPSET", mapset.toUtf8().data() );

  // Add all available mapsets to search path
  char **ms = G_available_mapsets();
  for ( int i = 0; ms[i]; i++ )  G_add_mapset_to_search_path( ms[i] );
}

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

int QgsGrass::error_routine( char *msg, int fatal )
{
  return error_routine(( const char* ) msg, fatal );
}

int QgsGrass::error_routine( const char *msg, int fatal )
{
  QgsDebugMsg( QString( "error_routine (fatal = %1): %2" ).arg( fatal ).arg( msg ) );

  error_message = msg;

  if ( fatal )
  {
    // we have to do a long jump here, otherwise GRASS >= 6.3 will kill our process
    throw QgsGrass::Exception( msg );
  }
  else
    lastError = WARNING;

  return 1;
}

void GRASS_EXPORT QgsGrass::resetError( void )
{
  lastError = OK;
}

int GRASS_EXPORT QgsGrass::error( void )
{
  return lastError;
}

QString GRASS_EXPORT QgsGrass::errorMessage( void )
{
  return error_message;
}

QString GRASS_EXPORT QgsGrass::openMapset( QString gisdbase, QString location, QString mapset )
{
  QgsDebugMsg( QString( "gisdbase = %1" ).arg( gisdbase.toUtf8().constData() ) );
  QgsDebugMsg( QString( "location = %1" ).arg( location.toUtf8().constData() ) );
  QgsDebugMsg( QString( "mapset = %1" ).arg( mapset.toUtf8().constData() ) );

  QString mapsetPath = gisdbase + "/" + location + "/" + mapset;

  // Check if the mapset is in use
  QString gisBase = getenv( "GISBASE" );
  if ( gisBase.isNull() ) return QObject::tr( "GISBASE is not set." );

  QFileInfo fi( mapsetPath + "/WIND" );
  if ( !fi.exists() )
  {
    return QObject::tr( "%1 is not a GRASS mapset." ).arg( mapsetPath );
  }

  QString lock = mapsetPath + "/.gislock";
  QFile lockFile( lock );
#if WIN32
  // lock on Windows doesn't support locking (see #808)
  if ( lockFile.exists() )
    return QObject::tr( "Mapset is already in use." );

  lockFile.open( QIODevice::WriteOnly );
#ifndef _MSC_VER
  int pid = getpid();
#else
  int pid = GetCurrentProcessId();
#endif
  lockFile.write( QString( "%1" ).arg( pid ).toUtf8() );
  lockFile.close();
#else
  QProcess *process = new QProcess();
  QString lockProgram( gisBase + "/etc/lock" );

  // TODO: getpid() probably is not portable
#ifndef _MSC_VER
  int pid = getpid();
#else
  int pid = GetCurrentProcessId();
#endif
  QgsDebugMsg( QString( "pid = %1" ).arg( pid ) );

  process->start( lockProgram, QStringList() << lock << QString::number( pid ) );
  if ( !process->waitForStarted() )
  {
    delete process;
    return QObject::tr( "Cannot start %1/etc/lock" ).arg( gisBase );
  }

  process->waitForFinished( -1 );

  int status = process->exitStatus();
  QgsDebugMsg( QString( "status = %1" ).arg( status ) );
  delete process;

  if ( status > 0 )
    return QObject::tr( "Mapset is already in use." );
#endif // !WIN32

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
      lockFile.remove();
      return QObject::tr( "Temporary directory %1 exists but is not writable" ).arg( mTmp );
    }
  }
  else if ( !dir.mkdir( mTmp ) )
  {
    lockFile.remove();
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
    lockFile.remove();
    return QObject::tr( "Cannot create %1" ).arg( mGisrc );
  }
  QTextStream stream( &out );

  QFile in( globalGisrc );
  QString line;
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

  out.close();

  // Set GISRC enviroment variable

  /* _Correct_ putenv() implementation is not making copy! */
  QString gisrcEnv = "GISRC=" + mGisrc;
  char *gisrcEnvChar = new char[gisrcEnv.toUtf8().length()+1];
  strcpy( gisrcEnvChar, gisrcEnv.toLocal8Bit().constData() );
  putenv( gisrcEnvChar );

  // Reinitialize GRASS
  G__setenv(( char * ) "GISRC", gisrcEnv.toUtf8().data() );
#if defined(WIN32)
  G__setenv(( char * ) "GISDBASE", shortPath( gisdbase ).toLocal8Bit().data() );
#else
  G__setenv(( char * ) "GISDBASE", gisdbase.toUtf8().data() );
#endif
  G__setenv(( char * ) "LOCATION_NAME", location.toLocal8Bit().data() );
  G__setenv(( char * ) "MAPSET", mapset.toLocal8Bit().data() );
  defaultGisdbase = gisdbase;
  defaultLocation = location;
  defaultMapset = mapset;

  active = true;

  // Close old mapset
  if ( mMapsetLock.length() > 0 )
  {
    QFile file( mMapsetLock );
    file.remove();
  }

  mMapsetLock = lock;

  return NULL;
}

QString QgsGrass::closeMapset( )
{
  QgsDebugMsg( "entered." );

  if ( mMapsetLock.length() > 0 )
  {
    QFile file( mMapsetLock );
    if ( !file.remove() )
    {
      return QObject::tr( "Cannot remove mapset lock: %1" ).arg( mMapsetLock );
    }
    mMapsetLock = "";

    putenv(( char * ) "GISRC" );

    // Reinitialize GRASS
    G__setenv(( char * ) "GISRC", ( char * ) "" );
    G__setenv(( char * ) "GISDBASE", ( char * ) "" );
    G__setenv(( char * ) "LOCATION_NAME", ( char * ) "" );
    G__setenv(( char * ) "MAPSET", ( char * ) "" );
    defaultGisdbase = "";
    defaultLocation = "";
    defaultMapset = "";
    active = 0;

    // Delete temporary dir

    // To be sure that we dont delete '/' for example
    if ( mTmp.left( 4 ) == "/tmp" )
    {
      QDir dir( mTmp );
      for ( unsigned int i = 0; i < dir.count(); i++ )
      {
        if ( dir[i] == "." || dir[i] == ".." ) continue;

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

QStringList GRASS_EXPORT QgsGrass::locations( QString gisbase )
{
  QgsDebugMsg( QString( "gisbase = %1" ).arg( gisbase ) );

  QStringList list;

  if ( gisbase.isEmpty() ) return list;

  QDir d = QDir( gisbase );
  d.setFilter( QDir::NoDotAndDotDot | QDir::Dirs );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( QFile::exists( gisbase + "/" + d[i]
                        + "/PERMANENT/DEFAULT_WIND" ) )
    {
      list.append( QString( d[i] ) );
    }
  }
  return list;
}

QStringList GRASS_EXPORT QgsGrass::mapsets( QString gisbase, QString locationName )
{
  QgsDebugMsg( QString( "gisbase = %1 locationName = %2" ).arg( gisbase ).arg( locationName ) );

  if ( gisbase.isEmpty() || locationName.isEmpty() )
    return QStringList();

  return QgsGrass::mapsets( gisbase + "/" + locationName );
}

QStringList GRASS_EXPORT QgsGrass::mapsets( QString locationPath )
{
  QgsDebugMsg( QString( "locationPath = %1" ).arg( locationPath ) );

  QStringList list;

  if ( locationPath.isEmpty() ) return list;

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

QStringList GRASS_EXPORT QgsGrass::vectors( QString gisbase, QString locationName,
    QString mapsetName )
{
  QgsDebugMsg( "entered." );

  if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
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

  return QgsGrass::vectors( gisbase + "/" + locationName + "/" + mapsetName );
}

QStringList GRASS_EXPORT QgsGrass::vectors( QString mapsetPath )
{
  QgsDebugMsg( QString( "mapsetPath = %1" ).arg( mapsetPath ) );

  QStringList list;

  if ( mapsetPath.isEmpty() ) return list;

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

QStringList GRASS_EXPORT QgsGrass::rasters( QString gisbase, QString locationName,
    QString mapsetName )
{
  QgsDebugMsg( "entered." );

  if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
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

  return QgsGrass::rasters( gisbase + "/" + locationName + "/" + mapsetName );
}

QStringList GRASS_EXPORT QgsGrass::rasters( QString mapsetPath )
{
  QgsDebugMsg( QString( "mapsetPath = %1" ).arg( mapsetPath ) );

  QStringList list;

  if ( mapsetPath.isEmpty() ) return list;

  QDir d = QDir( mapsetPath + "/cellhd" );
  d.setFilter( QDir::Files );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    list.append( d[i] );
  }
  return list;
}

QStringList GRASS_EXPORT QgsGrass::elements( QString gisbase, QString locationName,
    QString mapsetName, QString element )
{
  if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
    return QStringList();

  return QgsGrass::elements( gisbase + "/" + locationName + "/" + mapsetName,
                             element );
}

QStringList GRASS_EXPORT QgsGrass::elements( QString mapsetPath, QString element )
{
  QgsDebugMsg( QString( "mapsetPath = %1" ).arg( mapsetPath ) );

  QStringList list;

  if ( mapsetPath.isEmpty() ) return list;

  QDir d = QDir( mapsetPath + "/" + element );
  d.setFilter( QDir::Files );

  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    list.append( d[i] );
  }
  return list;
}

QString GRASS_EXPORT QgsGrass::regionString( struct Cell_head *window )
{
  QString reg;
  int fmt;
  char buf[1024];

  fmt = window->proj;

  // TODO 3D

  reg = "proj:" + QString::number( window->proj ) + ";" ;
  reg += "zone:" + QString::number( window->zone ) + ";" ;

  G_format_northing( window->north, buf, fmt );
  reg += "north:" + QString( buf ) + ";" ;

  G_format_northing( window->south, buf, fmt );
  reg += "south:" + QString( buf ) + ";" ;

  G_format_easting( window->east, buf, fmt );
  reg += "east:" + QString( buf ) + ";" ;

  G_format_easting( window->west, buf, fmt );
  reg += "west:" + QString( buf ) + ";" ;

  reg += "cols:" + QString::number( window->cols ) + ";" ;
  reg += "rows:" + QString::number( window->rows ) + ";" ;

  G_format_resolution( window->ew_res, buf, fmt );
  reg += "e-w resol:" + QString( buf ) + ";" ;

  G_format_resolution( window->ns_res, buf, fmt );
  reg += "n-s resol:" + QString( buf ) + ";" ;

  return reg;
}

bool GRASS_EXPORT QgsGrass::region( QString gisbase,
                                    QString location, QString mapset,
                                    struct Cell_head *window )
{
  QgsGrass::setLocation( gisbase, location );

  if ( G__get_window( window, ( char * ) "", ( char * ) "WIND", mapset.toUtf8().data() ) )
  {
    return false;
  }
  return true;
}

bool GRASS_EXPORT QgsGrass::writeRegion( QString gisbase,
    QString location, QString mapset,
    struct Cell_head *window )
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

void GRASS_EXPORT QgsGrass::copyRegionExtent( struct Cell_head *source,
    struct Cell_head *target )
{
  target->north = source->north;
  target->south = source->south;
  target->east = source->east;
  target->west = source->west;
  target->top = source->top;
  target->bottom = source->bottom;
}

void GRASS_EXPORT QgsGrass::copyRegionResolution( struct Cell_head *source,
    struct Cell_head *target )
{
  target->ns_res = source->ns_res;
  target->ew_res = source->ew_res;
  target->tb_res = source->tb_res;
  target->ns_res3 = source->ns_res3;
  target->ew_res3 = source->ew_res3;
}

void GRASS_EXPORT QgsGrass::extendRegion( struct Cell_head *source,
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

bool GRASS_EXPORT QgsGrass::mapRegion( int type, QString gisbase,
                                       QString location, QString mapset, QString map,
                                       struct Cell_head *window )
{
  QgsDebugMsg( "entered." );
  QgsDebugMsg( QString( "map = %1" ).arg( map ) );
  QgsDebugMsg( QString( "mapset = %1" ).arg( mapset ) );

  QgsGrass::setLocation( gisbase, location );

  if ( type == Raster )
  {

    if ( G_get_cellhd( map.toUtf8().data(),
                       mapset.toUtf8().data(), window ) < 0 )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read raster map region" ) );
      return false;
    }
  }
  else if ( type == Vector )
  {
    // Get current projection
    region( gisbase, location, mapset, window );

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
  else if ( type == Region )
  {
    if ( G__get_window( window, ( char * ) "windows",
                        map.toUtf8().data(),
                        mapset.toUtf8().data() ) != NULL )
    {
      QMessageBox::warning( 0, QObject::tr( "Warning" ),
                            QObject::tr( "Cannot read region" ) );
      return false;
    }
  }
  return true;
}

// GRASS version constants have been changed on 26.4.2007
// http://freegis.org/cgi-bin/viewcvs.cgi/grass6/include/version.h.in.diff?r1=1.4&r2=1.5
// The following lines workaround this change

int GRASS_EXPORT QgsGrass::versionMajor()
{
#ifdef GRASS_VERSION_MAJOR
  return GRASS_VERSION_MAJOR;
#else
  return QString( GRASS_VERSION_MAJOR ).toInt();
#endif
}

int GRASS_EXPORT QgsGrass::versionMinor()
{
#ifdef GRASS_VERSION_MINOR
  return GRASS_VERSION_MINOR;
#else
  return QString( GRASS_VERSION_MINOR ).toInt();
#endif
}

int GRASS_EXPORT QgsGrass::versionRelease()
{
#ifdef GRASS_VERSION_RELEASE
#define QUOTE(x)  #x
  return QString( QUOTE( GRASS_VERSION_RELEASE ) ).toInt();
#else
  return QString( GRASS_VERSION_RELEASE ).toInt();
#endif
}
QString GRASS_EXPORT QgsGrass::versionString()
{
  return QString( GRASS_VERSION_STRING );
}

bool GRASS_EXPORT QgsGrass::isMapset( QString path )
{
  /* TODO: G_is_mapset() was added to GRASS 6.1 06-05-24,
  enable its use after some period (others do update) */
  /*
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
  {
  if ( G_is_mapset( path.toUtf8().constData() ) ) return true;
  }
  else
  {
  */
  QString windf = path + "/WIND";
  if ( QFile::exists( windf ) ) return true;
  //}

  return false;
}

QString GRASS_EXPORT QgsGrass::lockFilePath()
{
  return mMapsetLock;
}
