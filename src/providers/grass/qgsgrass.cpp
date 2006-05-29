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

#include <iostream>

#include "qstring.h"
#include "q3process.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qfiledialog.h"
#include "qdir.h"
#include "qtextstream.h"
#include "qsettings.h"
#include <QMessageBox>
#include <QCoreApplication>

#include "qgsapplication.h"
#include "qgsgrass.h"

extern "C" {
#include <unistd.h>
#include <grass/gis.h>
#include <grass/Vect.h>
#include <grass/version.h>
}

void QgsGrass::init( void ) 
{
    // Warning!!! 
    // G_set_error_routine() once called from plugin
    // is not valid in provider -> call it always 

    // Set error function
    G_set_error_routine ( &error_routine );

    if ( initialized ) return;

    QSettings settings("QuantumGIS", "qgis");

    // Is it active mode ?
    if ( getenv ("GISRC") ) {
	active = true; 
	// Store default values
	defaultGisdbase = G_gisdbase();
	defaultLocation = G_location();
	defaultMapset = G_mapset();
    } else {
	active = false;
    }
    
    // Don't use GISRC file and read/write GRASS variables (from location G_VAR_GISRC) to memory only.
    G_set_gisrc_mode ( G_GISRC_MODE_MEMORY ); 

    // Init GRASS libraries (required)
    G_no_gisinit();  // Doesn't check write permissions for mapset compare to G_gisinit("libgrass++"); 

    // Set program name
    G_set_program_name ("QGIS");

  // Require GISBASE to be set. This should point to the location of
  // the GRASS installation. The GRASS libraries use it to know
  // where to look for things.

  // Look first to see if GISBASE env var is already set.
  // This is set when QGIS is run from within GRASS
  // or when set explicitly by the user.
  // This value should always take precedence.
  QString gisBase = getenv("GISBASE");
#ifdef QGISDEBUG
  qDebug( "%s:%d GRASS gisBase from GISBASE env var is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  if ( !isValidGrassBaseDir(gisBase) ) {
    // Look for gisbase in QSettings
    gisBase = settings.readEntry("/GRASS/gisbase", "");
#ifdef QGISDEBUG
    qDebug( "%s:%d GRASS gisBase from QSettings is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  }

  if ( !isValidGrassBaseDir(gisBase) ) {
    // Erase gisbase from settings because it does not exists 
    settings.writeEntry("/GRASS/gisbase", "");

#ifdef WIN32
    // Use the applicationDirPath()/grass
    gisBase = QCoreApplication::applicationDirPath() + "/grass";
#ifdef QGISDEBUG
    std::cerr << "GRASS gisBase = " << gisBase.ascii() << std::endl;
#endif

#else
    // Use the location specified --with-grass during configure
    gisBase = GRASS_BASE;
#ifdef QGISDEBUG
    qDebug( "%s:%d GRASS gisBase from configure is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif

#endif
  }

  bool userGisbase = false;
  while ( !isValidGrassBaseDir(gisBase) ) {
    // Keep asking user for GISBASE until we get a valid one
    //QMessageBox::warning( 0, "Warning", "QGIS can't find your GRASS installation,\nGRASS data "
    //    "cannot be used.\nPlease select your GISBASE.\nGISBASE is full path to the\n"
    //    "directory where GRASS is installed." );
    // XXX Need to subclass this and add explantory message above to left side
    userGisbase = true;
    gisBase = QFileDialog::getExistingDirectory(
	0, "Choose GISBASE ...", gisBase);
    if (gisBase == QString::null)
    {
      // User pressed cancel. No GRASS for you!
      userGisbase = false;
      break;
    }
  }

  if ( userGisbase )
  {
      settings.writeEntry("/GRASS/gisbase", gisBase);
  }

#ifdef QGISDEBUG
  qDebug( "%s:%d Valid GRASS gisBase is: %s", __FILE__, __LINE__, (const char*)gisBase );
#endif
  QString gisBaseEnv = "GISBASE=" + gisBase;
  /* _Correct_ putenv() implementation is not making copy! */ 
  char *gisBaseEnvChar = new char[gisBaseEnv.length()+1];
  strcpy ( gisBaseEnvChar, const_cast<char *>(gisBaseEnv.ascii()) ); 
  putenv( gisBaseEnvChar );

    // Add path to GRASS modules
#ifdef WIN32
    QString sep = ";";
#else
    QString sep = ":";
#endif
    QString path = "PATH=" + gisBase + "/bin";
    path.append ( sep + gisBase + "/scripts" );

    // On windows the GRASS libraries are in 
    // QgsApplication::prefixPath(), we have to add them
    // to PATH to enable running of GRASS modules 
    // and database drivers
#ifdef WIN32
    // It seems that QgsApplication::prefixPath() 
    // is not initialized at this point
    path.append ( sep + QCoreApplication::applicationDirPath() );
#endif

#ifdef WIN32
    // Add path to MSYS bin
    // Warning: MSYS sh.exe will translate this path to '/bin'
    path.append ( sep + QCoreApplication::applicationDirPath() 
                  + "/msys/bin/" );
#endif

    QString p = getenv ("PATH");
    path.append ( sep + p );

    #ifdef QGISDEBUG
    std::cerr << "set PATH: " << path.toLocal8Bit().data() << std::endl;
    #endif
    char *pathEnvChar = new char[path.length()+1];
    strcpy ( pathEnvChar, const_cast<char *>(path.ascii()) );
    putenv( pathEnvChar );

    initialized = 1;
}

/*
 * Check if given directory contains a GRASS installation
 */
bool QgsGrass::isValidGrassBaseDir(QString const gisBase)
{
  std::cerr << "isValidGrassBaseDir()" << std::endl;
  if ( gisBase.isEmpty() )
  {
    return FALSE;
  }
 
  /* TODO: G_is_gisbase() was added to GRASS 6.1 06-05-24,
           enable its use after some period (others do update) */ 
  /*
  if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 ) 
  {
      if ( G_is_gisbase( gisBase.toLocal8Bit().constData() ) ) return TRUE;
  } 
  else
  {
  */
      QFileInfo gbi ( gisBase + "/etc/element_list" );
      if ( gbi.exists() ) return TRUE;
  //}
  return FALSE;
}

bool QgsGrass::activeMode( void )
{
    init();
    return active;
}

QString QgsGrass::getDefaultGisdbase ( void ) {
    init();
    return defaultGisdbase;
}

QString QgsGrass::getDefaultLocation ( void ) {
    init();
    return defaultLocation;
}

QString QgsGrass::getDefaultMapset ( void ) {
    init();
    return defaultMapset;
}

void QgsGrass::setLocation( QString gisdbase, QString location )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase.toLocal8Bit().data() << " location = "
	      << location.toLocal8Bit().data() << std::endl;
    #endif
    init();

    // Set principal GRASS variables (in memory)
    G__setenv( "GISDBASE", (char *) gisdbase.ascii() );        
    G__setenv( "LOCATION_NAME", (char *) location.ascii() );
    G__setenv( "MAPSET", "PERMANENT"); // PERMANENT must always exist

    // Add all available mapsets to search path
    char **ms = G_available_mapsets();
    for ( int i = 0; ms[i]; i++ )  G_add_mapset_to_search_path ( ms[i] );
}

void QgsGrass::setMapset( QString gisdbase, QString location, QString mapset )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase.toLocal8Bit().data() << " location = "
	      << location.toLocal8Bit().data() << " mapset = " << mapset.toLocal8Bit().data() << std::endl;
    #endif
    init();

    // Set principal GRASS variables (in memory)
    G__setenv( "GISDBASE", (char *) gisdbase.ascii() );        
    G__setenv( "LOCATION_NAME", (char *) location.ascii() );
    G__setenv( "MAPSET", (char *) mapset.ascii() ); 

    // Add all available mapsets to search path
    char **ms = G_available_mapsets();
    for ( int i = 0; ms[i]; i++ )  G_add_mapset_to_search_path ( ms[i] );
}

int QgsGrass::initialized = 0;

bool QgsGrass::active = 0;

QgsGrass::GERROR QgsGrass::error = QgsGrass::OK;

QString QgsGrass::error_message;

QString QgsGrass::defaultGisdbase;
QString QgsGrass::defaultLocation;
QString QgsGrass::defaultMapset;

QString QgsGrass::mMapsetLock;
QString QgsGrass::mGisrc;
QString QgsGrass::mTmp;

int QgsGrass::error_routine ( char *msg, int fatal) {
    std::cerr << "error_routine (fatal = " << fatal << "): " << msg << std::endl;

    if ( fatal ) error = FATAL;
    else error = WARNING;

    error_message = msg;

    return 1;
}

void QgsGrass::resetError ( void ) {
    error = OK;
}

int QgsGrass::getError ( void ) {
    return error;
}

QString QgsGrass::getErrorMessage ( void ) {
    return error_message;
}

QString QgsGrass::openMapset ( QString gisdbase, QString location, QString mapset )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrass::openMapset" << std::endl;
    std::cerr << "gisdbase = " << gisdbase.local8Bit().data() << std::endl;
    std::cerr << "location = " << location.local8Bit().data() << std::endl;
    std::cerr << "mapset = " << mapset.local8Bit().data() << std::endl;
#endif

    QString mapsetPath = gisdbase + "/" + location + "/" + mapset;
    
    // Check if the mapset is in use
    QString gisBase = getenv("GISBASE");
    if ( gisBase.isNull() ) return "GISBASE is not set.";
     
    QFileInfo fi( mapsetPath+ "/WIND" );
    if ( !fi.exists() )
    {
        return mapsetPath + " is not a GRASS mapset.";
    }
    
    QString lock = mapsetPath + "/.gislock";
    QFile lockFile ( lock );
    Q3Process *process = new Q3Process();
    process->addArgument ( gisBase + "/etc/lock" ); // lock program
    process->addArgument ( lock ); // lock file

    // TODO: getpid() probably is not portable
    int pid = getpid();
#ifdef QGISDEBUG
    std::cerr << "pid = " << pid << std::endl;
#endif
    process->addArgument ( QString::number(pid) ); 

    if ( !process->start() ) 
    {
	return "Cannot start " + gisBase + "/etc/lock";
    }
    
    // TODO better wait 
    while ( process->isRunning () ) { }

    int status = process->exitStatus ();
    delete process;

#ifdef QGISDEBUG
    std::cerr << "status = " << status << std::endl;
#endif

// TODO WIN32 (lock.exe does not work properly?)
#ifndef WIN32
    if ( status > 0 ) return "Mapset is already in use.";
#endif

    // Create temporary directory
    QFileInfo info ( mapsetPath );
    QString user = info.owner();

    mTmp = QDir::tempPath () + "/grass6-" + user + "-" + QString::number(pid);
    QDir dir ( mTmp );
    if ( dir.exists() )
    {
        QFileInfo dirInfo(mTmp);
        if ( !dirInfo.isWritable() )
        {
            lockFile.remove();
	    return "Temporary directory " + mTmp + " exist but is not writable";
        }
    }
    else if ( !dir.mkdir( mTmp ) )
    {
        lockFile.remove();
	return "Cannot create temporary directory " + mTmp;
    }
   
    // Create GISRC file 
    QString globalGisrc =  QDir::home().path() + "/.grassrc6";
    mGisrc = mTmp + "/gisrc";

#ifdef QGISDEBUG
    std::cerr << "globalGisrc = " << globalGisrc.local8Bit().data() << std::endl;
    std::cerr << "mGisrc = " << mGisrc.local8Bit().data() << std::endl;
#endif

    QFile out ( mGisrc );
    if ( !out.open( QIODevice::WriteOnly ) ) 
    {
        lockFile.remove();
	return "Cannot create " + mGisrc; 
    }
    QTextStream stream ( &out );

    QFile in ( globalGisrc );
    QString line;
    char buf[1000];
    if ( in.open( QIODevice::ReadOnly ) ) 
    {
	while ( in.readLine( buf, 1000 ) != -1 ) 
	{
      line = buf;
	    if ( line.contains("GISDBASE:") || 
		 line.contains("LOCATION_NAME:") ||
	         line.contains("MAPSET:") )
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
    char *gisrcEnvChar = new char[gisrcEnv.length()+1];
    strcpy ( gisrcEnvChar, const_cast<char *>(gisrcEnv.ascii()) );
    putenv( gisrcEnvChar );
    
    // Reinitialize GRASS 
    G__setenv( "GISRC", const_cast<char *>(gisrcEnv.ascii()) );        
    G__setenv( "GISDBASE", const_cast<char *>(gisdbase.ascii()) );        
    G__setenv( "LOCATION_NAME", const_cast<char *>(location.ascii()) );
    G__setenv( "MAPSET", const_cast<char *>(mapset.ascii()) );
    defaultGisdbase = gisdbase;
    defaultLocation = location;
    defaultMapset = mapset;

    active = true; 

    // Close old mapset
    if ( mMapsetLock.length() > 0 )
    {
        QFile file ( mMapsetLock );
        file.remove();
    }
    
    mMapsetLock = lock;

    return NULL;
}

QString QgsGrass::closeMapset ( )
{
#ifdef QGISDEBUG
    std::cerr << "QgsGrass::closeMapset" << std::endl;
#endif

    if ( mMapsetLock.length() > 0 )
    {
        QFile file ( mMapsetLock );
        if ( !file.remove() )
        {
	    return "Cannot remove mapset lock: " + mMapsetLock;
        }
        mMapsetLock = "";

	putenv( "GISRC" );
	
	// Reinitialize GRASS 
	G__setenv( "GISRC", "" );        
	G__setenv( "GISDBASE", "" );        
	G__setenv( "LOCATION_NAME", "" );
	G__setenv( "MAPSET", "" );
	defaultGisdbase = "";
	defaultLocation = "";
	defaultMapset = "";
        active = 0;

        // Delete temporary dir
        
        // To be sure that we dont delete '/' for example
        if ( mTmp.left(4) == "/tmp" ) 
        {
	    QDir dir ( mTmp );
	    for ( int i = 0; i < dir.count(); i++ )
	    {
		if ( dir[i] == "." || dir[i] == ".." ) continue;

		dir.remove(dir[i]); 
		if ( dir.remove(dir[i]) )
		{
		    std::cerr << "Cannot remove temporary file " << dir[i].local8Bit().data() << std::endl;
		}
	    }
             
            if ( !dir.rmdir(mTmp) )
            {
                std::cerr << "Cannot remove temporary directory " << mTmp.local8Bit().data() << std::endl;
            }
        } 
    }

    return NULL;
}

QStringList QgsGrass::locations ( QString gisbase )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::locations gisbase = " 
	      << gisbase.ascii() << std::endl;
    #endif

    QStringList list;

    if ( gisbase.isEmpty() ) return list;
    
    QDir d = QDir( gisbase );
    d.setFilter(QDir::NoDotAndDotDot|QDir::Dirs);

    for ( int i = 0; i < d.count(); i++ ) 
    {
        if ( QFile::exists ( gisbase + "/" + d[i] 
		             + "/PERMANENT/DEFAULT_WIND" ) )
	{
    	    list.append(QString(d[i]));
	}
    }
    return list;
}

QStringList QgsGrass::mapsets ( QString gisbase, QString locationName )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::mapsets gisbase = " << gisbase.ascii() 
	      << " locationName = " << locationName.ascii() << std::endl;
    #endif
    
    if ( gisbase.isEmpty() || locationName.isEmpty() )
	return QStringList();

    return QgsGrass::mapsets ( gisbase + "/" + locationName );
}

QStringList QgsGrass::mapsets ( QString locationPath )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::mapsets locationPath = " 
	      << locationPath.ascii() << std::endl;
    #endif

    QStringList list;

    if ( locationPath.isEmpty() ) return list;
    
    QDir d = QDir( locationPath );
    d.setFilter(QDir::NoDotAndDotDot|QDir::Dirs);

    for ( int i = 0; i < d.count(); i++ ) 
    {
        if ( QFile::exists ( locationPath + "/" + d[i] + "/WIND" ) )
	{
    	    list.append(d[i]);
	}
    }
    return list;
}

QStringList QgsGrass::vectors ( QString gisbase, QString locationName,
	                         QString mapsetName)
{
    std::cerr << "QgsGrass::vectors()" << std::endl;

    if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
	return QStringList();

    /* TODO: G_list() was added to GRASS 6.1 06-05-24,
             enable its use after some period (others do update) */ 
    /*
    if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 ) 
    {
	QStringList list;

	char **glist = G_list( G_ELEMENT_VECTOR, 
			      gisbase.toLocal8Bit().constData(), 
			      locationName.toLocal8Bit().constData(), 
			      mapsetName.toLocal8Bit().constData() );

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

    return QgsGrass::vectors ( gisbase + "/" + locationName + "/" + mapsetName );
}

QStringList QgsGrass::vectors ( QString mapsetPath )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::vectors mapsetPath = " 
	      << mapsetPath.ascii() << std::endl;
    #endif

    QStringList list;

    if ( mapsetPath.isEmpty() ) return list;
    
    QDir d = QDir( mapsetPath + "/vector" );
    d.setFilter(QDir::NoDotAndDotDot|QDir::Dirs);

    for ( int i = 0; i < d.count(); i++ ) 
    {
        /*
        if ( QFile::exists ( mapsetPath + "/vector/" + d[i] + "/head" ) )
	{
    	    list.append(d[i]);
	}
        */
    	list.append(d[i]);
    }
    return list;
}

QStringList QgsGrass::rasters ( QString gisbase, QString locationName,
	                         QString mapsetName)
{
    std::cerr << "QgsGrass::rasters()" << std::endl;

    if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
	return QStringList();


    /* TODO: G_list() was added to GRASS 6.1 06-05-24,
             enable its use after some period (others do update) */ 
    /*
    if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 ) 
    {
	QStringList list;

	char **glist = G_list( G_ELEMENT_RASTER, 
			      gisbase.toLocal8Bit().constData(), 
			      locationName.toLocal8Bit().constData(), 
			      mapsetName.toLocal8Bit().constData() );

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
	
    return QgsGrass::rasters ( gisbase + "/" + locationName + "/" + mapsetName );
}

QStringList QgsGrass::rasters ( QString mapsetPath )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::rasters mapsetPath = " 
	      << mapsetPath.ascii() << std::endl;
    #endif

    QStringList list;

    if ( mapsetPath.isEmpty() ) return list;
    
    QDir d = QDir( mapsetPath + "/cellhd" );
    d.setFilter(QDir::Files);

    for ( int i = 0; i < d.count(); i++ ) 
    {
        list.append(d[i]);
    }
    return list;
}

QStringList QgsGrass::elements ( QString gisbase, QString locationName,
	                         QString mapsetName, QString element)
{
    if ( gisbase.isEmpty() || locationName.isEmpty() || mapsetName.isEmpty() )
	return QStringList();

    return QgsGrass::elements ( gisbase + "/" + locationName + "/" + mapsetName, 
                                element );
}

QStringList QgsGrass::elements ( QString mapsetPath, QString element )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::elements mapsetPath = " 
	      << mapsetPath.ascii() << std::endl;
    #endif

    QStringList list;

    if ( mapsetPath.isEmpty() ) return list;
    
    QDir d = QDir( mapsetPath + "/" + element );
    d.setFilter(QDir::Files);

    for ( int i = 0; i < d.count(); i++ ) 
    {
        list.append(d[i]);
    }
    return list;
}

QString QgsGrass::regionString( struct Cell_head *window )
{
    QString reg;
    int fmt;
    char buf[1024];

    fmt = window->proj;    

    // TODO 3D

    reg = "proj:" + QString::number(window->proj) + ";" ;
    reg += "zone:" + QString::number(window->zone) + ";" ;

    G_format_northing (window->north,buf,fmt);
    reg += "north:" + QString(buf) + ";" ;

    G_format_northing (window->south,buf,fmt);
    reg += "south:" + QString(buf) + ";" ;

    G_format_easting (window->east,buf,fmt);
    reg += "east:" + QString(buf) + ";" ;

    G_format_easting (window->west,buf,fmt);
    reg += "west:" + QString(buf) + ";" ;

    reg += "cols:" + QString::number(window->cols) + ";" ;
    reg += "rows:" + QString::number(window->rows) + ";" ;

    G_format_resolution (window->ew_res,buf,fmt);
    reg += "e-w resol:" + QString(buf) + ";" ;

    G_format_resolution (window->ns_res,buf,fmt);
    reg += "n-s resol:" + QString(buf) + ";" ;

    return reg;
}

bool QgsGrass::region( QString gisbase, 
           QString location, QString mapset,
           struct Cell_head *window )
{
    QgsGrass::setLocation( gisbase, location );

    if ( G__get_window ( window, "", "WIND", mapset.toLocal8Bit().data() ) )
    {
        return false;
    }
    return true;
}

bool QgsGrass::writeRegion( QString gisbase, 
           QString location, QString mapset,
           struct Cell_head *window )
{
    std::cerr << "QgsGrass::writeRegion()" << std::endl;
    std::cerr << "n = " << window->north << " s = " << window->south << std::endl;
    std::cerr << "e = " << window->east << " w = " << window->west << std::endl;

    QgsGrass::setMapset( gisbase, location, mapset );

    if ( G_put_window(window) == -1 ) {
        return false;
    }

    return true;
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

bool QgsGrass::mapRegion( int type, QString gisbase, 
           QString location, QString mapset, QString map,
           struct Cell_head *window )
{
    #ifdef QGISDEBUG
    std::cerr << "QgsGrass::mapRegion()" << std::endl;
    std::cerr << "map = " << map.toLocal8Bit().data() << std::endl;
    std::cerr << "mapset = " << mapset.toLocal8Bit().data() << std::endl;
    #endif

    QgsGrass::setLocation( gisbase, location );

    if ( type == Raster )
    {

	if ( G_get_cellhd ( map.toLocal8Bit().data(), 
		      mapset.toLocal8Bit().data(), window) < 0 )
	{
	    QMessageBox::warning( 0, "Warning", 
		     "Cannot read raster map region" ); 
	    return false;
	}
    }
    else if ( type == Vector )
    {
        // Get current projection
        region( gisbase, location, mapset, window );

	struct Map_info Map;

	int level = Vect_open_old_head ( &Map, 
	      map.toLocal8Bit().data(), mapset.toLocal8Bit().data());

	if ( level < 2 ) 
	{ 
	    QMessageBox::warning( 0, "Warning", 
		     "Cannot read vector map region" ); 
	    return false;
	}

	BOUND_BOX box;
	Vect_get_map_box (&Map, &box );
	window->north = box.N;
	window->south = box.S;
	window->west  = box.W;
	window->east  = box.E;
	window->top  = box.T;
	window->bottom  = box.B;

        // Is this optimal ?
        window->ns_res = (window->north-window->south)/1000;
        window->ew_res = window->ns_res;
        if ( window->top > window->bottom ) 
        { 
            window->tb_res = (window->top-window->bottom)/10;
        }
        else
        {
            window->top = window->bottom + 1;
            window->tb_res = 1;
        }
        G_adjust_Cell_head3 ( window, 0, 0, 0 );
	
	Vect_close (&Map);
    } 
    else if ( type == Region )
    {
	if (  G__get_window (window, "windows", 
		  map.toLocal8Bit().data(), 
		  mapset.toLocal8Bit().data() ) != NULL )
	{
	    QMessageBox::warning( 0, "Warning", 
		     "Cannot read region" ); 
	    return false;
	}
    }
    return true;
}

int QgsGrass::versionMajor()
{
    return QString(GRASS_VERSION_MAJOR).toInt();
}
int QgsGrass::versionMinor()
{
    return QString(GRASS_VERSION_MINOR).toInt();
}

bool QgsGrass::isMapset ( QString path )
{
    /* TODO: G_is_mapset() was added to GRASS 6.1 06-05-24,
             enable its use after some period (others do update) */
    /*
    if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
    {
        if ( G_is_mapset( path.toLocal8Bit().constData() ) ) return true;
    }
    else
    {
    */
        QString windf = path + "/WIND";
        if ( QFile::exists ( windf ) ) return true;
    //}

    return false;
}

