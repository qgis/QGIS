/***************************************************************************
    qgsgrass.h  -  Data provider for GRASS format
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
#ifndef QGSGRASS_H
#define QGSGRASS_H

// GRASS header files
extern "C" {
#include <gis.h>
#include <form.h>
}

/*!
   Methods for C library initialization and error handling.
*/
class QgsGrass {

public:  
    //! Get info about the mode 
    /*! QgsGrass may be running in active or passive mode. 
     *  Active mode means that GISRC is set up and GISRC file is available, 
     *  in that case default GISDBASE, LOCATION and MAPSET may be read by GetDefaul*() functions.
     *  Passive mode means, that GISRC is not available. */
    static bool activeMode ( void );
    
    //! Get default GISDBASE, returns GISDBASE name or empty string if not in active mode
    static QString getDefaultGisdbase ( void ); 

    //! Get default LOCATION_NAME, returns LOCATION_NAME name or empty string if not in active mode
    static QString getDefaultLocation ( void ); 

    //! Get default MAPSET, returns MAPSET name or empty string if not in active mode
    static QString getDefaultMapset ( void ); 

    //! Init or reset GRASS library 
    /*!
	\param gisdbase full path to GRASS GISDBASE.
	\param location location name (not path!).
    */
    static void setLocation( QString gisdbase, QString location);

    /*!
	\param gisdbase full path to GRASS GISDBASE.
	\param location location name (not path!).
	\param mapset current mupset. Note that some variables depend on mapset and
	              may influence behaviour of some functions (e.g. search path etc.) 
    */
    static void setMapset( QString gisdbase, QString location, QString mapset);

    //! Error codes returned by GetError() 
    enum ERROR { OK, /*!< OK. No error. */  
	         WARNING, /*!< Warning, non fatal error. Should be printed by application. */ 
		 FATAL /*!< Fatal error. Function faild. */ 
               };

    //! Reset error code (to OK). Call this before a piece of code where an error is expected 
    static void resetError ( void ); // reset error status

    //! Check if any error occured in lately called functions. Returns value from ERROR.
    static int getError ( void ); 

    //! Get last error message 
    static QString getErrorMessage ( void ); 

    /** \brief Open existing GRASS mapset
     * \return NULL string or error message
     */
    static QString openMapset ( QString gisdbase, 
                   QString location, QString mapset );

    /** \brief Close mapset if it was opened from QGIS.
     *         Delete GISRC, lock and temporary directory
     * \return NULL string or error message
     */
    static QString closeMapset ();

private:
    static int initialized; // Set to 1 after initialization 
    static bool active; // is active mode
    static void init (void); 
    static QString defaultGisdbase;
    static QString defaultLocation;
    static QString defaultMapset;

    /* last error in GRASS libraries */
    static ERROR error;         // static, because used in constructor
    static QString error_message;

    static int error_routine ( char *msg, int fatal); // static because pointer to this function is set later
};
    // Current mapset lock file path
    static QString mMapsetLock;  
    // Current mapset GISRC file path
    static QString mGisrc;  
    // Temporary directory where GISRC and sockets are stored
    static QString mTmp;  

#endif // QGSGRASS_H
