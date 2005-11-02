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
#include <iostream>

#include "qstring.h"
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
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase << " location = "
	      << location  << std::endl;
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
    std::cerr << "QgsGrass::setLocation(): gisdbase = " << gisdbase << " location = "
	      << location << " mapset = " << mapset << std::endl;
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

