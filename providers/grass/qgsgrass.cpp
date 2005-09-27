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
#include "qprocess.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdir.h"
#include "qtextstream.h"
#include "qgsgrass.h"

void QgsGrass::init( void ) {
    if ( !initialized ) {
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

        // Set error function
        G_set_error_routine ( &error_routine );

	// Set program name
	G_set_program_name ("QGIS");

        // Add path to GRASS modules
        // TODO: do that portable
        QString gisBase = getenv("GISBASE");
        QString path = "PATH=" + gisBase + "/bin";
        path.append ( ":" + gisBase + "/scripts" );

        QString p = getenv ("PATH");
        path.append ( ":" + p );

	#ifdef QGISDEBUG
	std::cerr << "set PATH: " << path.local8Bit() << std::endl;
	#endif
	char *pathEnvChar = new char[path.length()+1];
	strcpy ( pathEnvChar, const_cast<char *>(path.ascii()) );
	putenv( pathEnvChar );

	initialized = 1;
    }
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
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase.local8Bit() << " location = "
	      << location.local8Bit() << std::endl;
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
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase.local8Bit() << " location = "
	      << location.local8Bit() << " mapset = " << mapset.local8Bit() << std::endl;
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

QgsGrass::ERROR QgsGrass::error = QgsGrass::OK;

QString QgsGrass::error_message;

QString QgsGrass::defaultGisdbase;
QString QgsGrass::defaultLocation;
QString QgsGrass::defaultMapset;

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
    std::cerr << "gisdbase = " << gisdbase << std::endl;
    std::cerr << "location = " << location << std::endl;
    std::cerr << "mapset = " << mapset << std::endl;
#endif

    QString mapsetPath = gisdbase + "/" + location + "/" + mapset;
    
    // Check if the mapset is in use
    QString gisBase = getenv("GISBASE");
    if ( gisBase.isNull() ) return "GISBASE is not set.";
    
    QString lock = mapsetPath + "/.gislock";
    QFile lockFile ( lock );
    QProcess *process = new QProcess();
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
    if ( status > 0 ) return "Mapset is already in use.";

    // Create temporary directory
    QFileInfo info ( mapsetPath );
    QString user = info.owner();

    mTmp = "/tmp/grass6-" + user + "-" + QString::number(pid);
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
    std::cerr << "globalGisrc = " << globalGisrc << std::endl;
    std::cerr << "mGisrc = " << mGisrc << std::endl;
#endif

    QFile out ( mGisrc );
    if ( !out.open( IO_WriteOnly ) ) 
    {
        lockFile.remove();
	return "Cannot create " + mGisrc; 
    }
    QTextStream stream ( &out );

    QFile in ( globalGisrc );
    QString line;
    if ( in.open( IO_ReadOnly ) ) 
    {
	while ( in.readLine( line, 1000 ) != -1 ) 
	{
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
		    std::cerr << "Cannot remove temporary file " << dir[i] << std::endl;
		}
	    }
             
            if ( !dir.rmdir(mTmp) )
            {
                std::cerr << "Cannot remove temporary directory " << mTmp << std::endl;
            }
        } 
    }

    return NULL;
}
