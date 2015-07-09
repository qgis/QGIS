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

#include <QMutex>

#include <setjmp.h>

// GRASS header files
extern "C"
{
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/form.h>
#include <grass/dbmi.h>
}

#include <stdexcept>
#include "qgsapplication.h"
#include "qgsexception.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include <qgsrectangle.h>
#include <QProcess>
#include <QString>
#include <QMap>
#include <QHash>
#include <QRegExp>
#include <QTemporaryFile>
class QgsCoordinateReferenceSystem;
class QgsRectangle;

// Make the release string because it may be for example 0beta1
#define STR(x) #x
#define EXPAND(x) STR(x)
#define GRASS_VERSION_RELEASE_STRING EXPAND( GRASS_VERSION_RELEASE )

#if (GRASS_VERSION_MAJOR < 7)
#define G_TRY try { if( !setjmp( QgsGrass::jumper ) )
#else
#define G_TRY try { if( !setjmp(*G_fatal_longjmp(1)) )
#endif
#define G_CATCH else { throw QgsGrass::Exception( QgsGrass::errorMessage() ); } } catch

#if GRASS_VERSION_MAJOR >= 7
#define G_available_mapsets G_get_available_mapsets
#define G__mapset_permissions2 G_mapset_permissions2
#define G_suppress_masking Rast_suppress_masking
#define G__get_window(window,element,name,mapset) (G_get_element_window(window,element,name,mapset),0)
#define G__getenv G_getenv_nofatal
#define G__setenv G_setenv_nogisrc
#define BOUND_BOX bound_box
#endif

// Element info container
class GRASS_LIB_EXPORT QgsGrassObject
{
  public:
    //! Element type
    enum Type { None, Raster, Group, Vector, Region };

    QgsGrassObject() : mType( None ) {}
    QgsGrassObject( const QString& gisdbase, const QString& location = QString::null,
                    const QString& mapset = QString::null, const QString& name = QString::null,
                    Type type = None );
    QString gisdbase() const { return mGisdbase; }
    void setGisdbase( const QString& gisdbase ) { mGisdbase = gisdbase; }
    QString location() const { return mLocation; }
    void setLocation( const QString& location ) { mLocation = location; }
    QString locationPath() const { return mGisdbase + "/" + mLocation; }
    QString mapset() const { return mMapset; }
    void setMapset( const QString& mapset ) { mMapset = mapset; }
    QString mapsetPath() const { return mGisdbase + "/" + mLocation + "/" + mMapset; }
    QString name() const { return mName; }
    void setName( const QString& name ) { mName = name; }
    QString fullName() const { return mName + "@" + mMapset; }
    Type type() const { return mType; }
    void setType( Type type ) { mType = type; }
    // set from QGIS layer uri, returns true if set correctly, verifies also if location is a GRASS location
    bool setFromUri( const QString& uri );
    // element name used as modules param, e.g. g.remove element=name
    QString elementShort() const;
    // descriptive full name
    QString elementName() const;
    static QString elementName( Type type );
    // name of directory in GRASS mapset to look for the object (cellhd,vector,window)
    QString dirName() const;
    static QString dirName( Type type );
    QString toString() const;
    // returns true if gisdbase and location are the same
    bool locationIdentical( const QgsGrassObject &other ) const;
    // returns true if gisdbase, location and mapset are the same
    bool mapsetIdentical( const QgsGrassObject &other ) const;
    // get regexp patter for new names, e.g. vectors should not start with number
    static QRegExp newNameRegExp( Type type );

    bool operator==( const QgsGrassObject& other ) const;
  private:
    QString mGisdbase;
    QString mLocation;
    QString mMapset;
    QString mName;  // map name
    Type mType;
};

/*!
   Methods for C library initialization and error handling.
*/
class QgsGrass
{
  public:
    static GRASS_LIB_EXPORT jmp_buf jumper; // used to get back from fatal error

    // This does not work (gcc/Linux), such exception cannot be caught
    // so I have enabled the old version, if you are able to fix it, please
    // check first if it realy works, i.e. can be caught!
    /*
    struct Exception : public QgsException
    {
      Exception( const QString &msg ) : QgsException( msg ) {}
    };
    */
    struct Exception : public std::runtime_error
    {
      //Exception( const std::string &msg ) : std::runtime_error( msg ) {}
      Exception( const QString &msg ) : std::runtime_error( msg.toUtf8().constData() ) {}
    };

    struct Color
    {
      double value1, value2;
      int red1, red2, green1, green2, blue1, blue2;
    };

    //! Get info about the mode
    /** QgsGrass may be running in active or passive mode.
     *  Active mode means that GISRC is set up and GISRC file is available,
     *  in that case default GISDBASE, LOCATION and MAPSET may be read by GetDefaul*() functions.
     *  Passive mode means, that GISRC is not available. */
    static GRASS_LIB_EXPORT bool activeMode();

    //! Get default GISDBASE, returns GISDBASE name or empty string if not in active mode
    static GRASS_LIB_EXPORT QString getDefaultGisdbase();

    //! Get default LOCATION_NAME, returns LOCATION_NAME name or empty string if not in active mode
    static GRASS_LIB_EXPORT QString getDefaultLocation();

    //! Get default MAPSET, returns MAPSET name or empty string if not in active mode
    static GRASS_LIB_EXPORT QString getDefaultMapset();

    //! Init or reset GRASS library
    /*!
    \param gisdbase full path to GRASS GISDBASE.
    \param location location name (not path!).
    */
    static GRASS_LIB_EXPORT void setLocation( QString gisdbase, QString location );

    /*!
    \param gisdbase full path to GRASS GISDBASE.
    \param location location name (not path!).
    \param mapset current mupset. Note that some variables depend on mapset and
               may influence behaviour of some functions (e.g. search path etc.)
    */
    static GRASS_LIB_EXPORT void setMapset( QString gisdbase, QString location, QString mapset );

    //! Error codes returned by error()
    enum GERROR
    {
      OK, /*!< OK. No error. */
      WARNING, /*!< Warning, non fatal error. Should be printed by application. */
      FATAL /*!< Fatal error */
    };

    //! Reset error code (to OK). Call this before a piece of code where an error is expected
    static GRASS_LIB_EXPORT void resetError( void );  // reset error status

    //! Check if any error occured in lately called functions. Returns value from ERROR.
    static GRASS_LIB_EXPORT int error( void );

    //! Get last error message
    static GRASS_LIB_EXPORT QString errorMessage( void );

    /** \brief Open existing GRASS mapset
     * \return NULL string or error message
     */
    static GRASS_LIB_EXPORT QString openMapset( const QString& gisdbase,
        const QString& location, const QString& mapset );

    /** \brief Close mapset if it was opened from QGIS.
     *         Delete GISRC, lock and temporary directory
     * \return NULL string or error message
     */
    static GRASS_LIB_EXPORT QString closeMapset();

    //! Check if given directory contains a GRASS installation
    static GRASS_LIB_EXPORT bool isValidGrassBaseDir( const QString& gisBase );

    //! Returns list of locations in given gisbase
    static QStringList GRASS_LIB_EXPORT locations( const QString& gisdbase );

    //! Returns list of mapsets in location
    static GRASS_LIB_EXPORT QStringList mapsets( const QString& gisdbase, const QString& locationName );
    static GRASS_LIB_EXPORT QStringList mapsets( const QString& locationPath );

    //! List of vectors and rasters
    static GRASS_LIB_EXPORT QStringList vectors( const QString& gisdbase, const QString& locationName,
        const QString& mapsetName );
    static GRASS_LIB_EXPORT QStringList vectors( const QString& mapsetPath );

    static GRASS_LIB_EXPORT QStringList rasters( const QString& gisdbase, const QString& locationName,
        const QString& mapsetName );
    static GRASS_LIB_EXPORT QStringList rasters( const QString& mapsetPath );

    // imagery groups
    static GRASS_LIB_EXPORT QStringList groups( const QString& gisdbase, const QString& locationName,
        const QString& mapsetName );
    static GRASS_LIB_EXPORT QStringList groups( const QString& mapsetPath );

    //! Get topo file version 6, 7 or 0 if topo file does not exist
    static GRASS_LIB_EXPORT bool topoVersion( const QString& gisdbase, const QString& location,
        const QString& mapset, const QString& mapName, int &major, int &minor );

    //! Get list of vector layers, throws QgsGrass::Exception
    static GRASS_LIB_EXPORT QStringList vectorLayers( const QString& gisdbase, const QString& location,
        const QString& mapset, const QString& mapName );

    //! List of elements
    // TODO rename elements to objects
    static GRASS_LIB_EXPORT QStringList elements( const QString& gisdbase, const QString& locationName,
        const QString& mapsetName, const QString& element );
    static GRASS_LIB_EXPORT QStringList elements( const QString&  mapsetPath, const QString&  element );

    //! List of existing objects
    static GRASS_LIB_EXPORT QStringList grassObjects( const QString& mapsetPath, QgsGrassObject::Type type );

    // returns true if object (vector, raster, region) exists
    static GRASS_LIB_EXPORT bool objectExists( const QgsGrassObject& grassObject );

    //! Initialize GRASS region
    static GRASS_LIB_EXPORT void initRegion( struct Cell_head *window );
    //! Set region extent
    static GRASS_LIB_EXPORT void setRegion( struct Cell_head *window, QgsRectangle rect );
    /** Init region, set extent, rows and cols and adjust.
     * Returns error if adjustment failed. */
    static GRASS_LIB_EXPORT QString setRegion( struct Cell_head *window, QgsRectangle rect, int rows, int cols );
    //! Get extent from region
    static GRASS_LIB_EXPORT QgsRectangle extent( struct Cell_head *window );

    // ! Get map region
    static GRASS_LIB_EXPORT bool mapRegion( QgsGrassObject::Type type, QString gisdbase,
                                            QString location, QString mapset, QString map,
                                            struct Cell_head *window );

    // ! String representation of region
    static GRASS_LIB_EXPORT QString regionString( const struct Cell_head *window );

    // ! Read location default region (DEFAULT_WIND)
    static GRASS_LIB_EXPORT bool defaultRegion( const QString& gisdbase, const QString& location,
        struct Cell_head *window );

    // ! Read current mapset region
    static GRASS_LIB_EXPORT bool region( const QString& gisdbase, const QString& location, const QString& mapset,
                                         struct Cell_head *window );

    // ! Write current mapset region
    static GRASS_LIB_EXPORT bool writeRegion( const QString& gisbase, const QString& location, const QString& mapset,
        const struct Cell_head *window );

    // ! Set (copy) region extent, resolution is not changed
    static GRASS_LIB_EXPORT void copyRegionExtent( struct Cell_head *source,
        struct Cell_head *target );

    // ! Set (copy) region resolution, extent is not changed
    static GRASS_LIB_EXPORT void copyRegionResolution( struct Cell_head *source,
        struct Cell_head *target );

    // ! Extend region in target to source
    static GRASS_LIB_EXPORT void extendRegion( struct Cell_head *source,
        struct Cell_head *target );

    static GRASS_LIB_EXPORT void init( void );

    //! test if the directory is location
    static GRASS_LIB_EXPORT bool isLocation( const QString& path );;

    // ! test if the directory is mapset
    static GRASS_LIB_EXPORT bool isMapset( const QString& path );

    // ! Get the lock file
    static GRASS_LIB_EXPORT QString lockFilePath();

    // ! Get current gisrc path
    static GRASS_LIB_EXPORT QString gisrcFilePath();

    // ! Start a GRASS module in any gisdbase/location/mapset
    // @param qgisModule append GRASS major version (for modules built in qgis)
    static GRASS_LIB_EXPORT QProcess *startModule( const QString& gisdbase, const QString&  location,
        const QString& mapset, const QString&  moduleName,
        const QStringList& arguments, QTemporaryFile &gisrcFile,
        bool qgisModule = true );

    // ! Run a GRASS module in any gisdbase/location
    static GRASS_LIB_EXPORT QByteArray runModule( const QString& gisdbase, const QString&  location,
        const QString& mapset, const QString&  moduleName,
        const QStringList& arguments, int timeOut = 30000,
        bool qgisModule = true );

    /** \brief Get info string from qgis.g.info module
     * @param info info type
     * @gisdbase GISBASE path
     * @location location name
     * @mapset mapset name
     * @map map name
     * @type map type
     * @x x coordinate for query
     * @y y coordinate for query
     * @extent extent for statistics
     * @sampleSize sample size for statistics
     * @timeOut timeout
     */
    static GRASS_LIB_EXPORT QString getInfo( const QString&  info, const QString&  gisdbase,
        const QString&  location, const QString&  mapset = "PERMANENT",
        const QString&  map = QString::null, const QgsGrassObject::Type type = QgsGrassObject::None,
        double x = 0.0, double y = 0.0,
        const QgsRectangle& extent = QgsRectangle(), int sampleRows = 0,
        int sampleCols = 0, int timeOut = 30000 );

    // ! Get location projection
    static GRASS_LIB_EXPORT QgsCoordinateReferenceSystem crs( const QString& gisdbase, const QString& location, bool interactive = true );

    // ! Get location projection calling directly GRASS library
    static GRASS_LIB_EXPORT QgsCoordinateReferenceSystem crsDirect( const QString& gisdbase, const QString& location );

    // ! Get map extent
    // @param interactive - show warning dialog on error
    static GRASS_LIB_EXPORT QgsRectangle extent( const QString& gisdbase, const QString& location,
        const QString& mapset, const QString& map,
        QgsGrassObject::Type type = QgsGrassObject::None, bool interactive = true );

    // ! Get raster map size
    static GRASS_LIB_EXPORT void size( const QString& gisdbase, const QString& location,
                                       const QString& mapset, const QString& map, int *cols, int *rows );

    // ! Get raster info, info is either 'info' or 'stats'
    //   extent and sampleSize are stats options
    // @param interactive - show warning dialog on error
    static GRASS_LIB_EXPORT QHash<QString, QString> info( const QString& gisdbase, const QString& location,
        const QString& mapset, const QString& map,
        QgsGrassObject::Type type,
        const QString& info = "info",
        const QgsRectangle& extent = QgsRectangle(),
        int sampleRows = 0, int sampleCols = 0,
        int timeOut = 30000, bool interactive = true );

    // ! List of Color
    static GRASS_LIB_EXPORT QList<QgsGrass::Color> colors( QString gisdbase, QString location,
        QString mapset, QString map );

    // ! Get map value / feature info
    static GRASS_LIB_EXPORT QMap<QString, QString> query( QString gisdbase, QString location,
        QString mapset, QString map, QgsGrassObject::Type type, double x, double y );

    // ! Rename GRASS object, throws QgsGrass::Exception
    static GRASS_LIB_EXPORT void renameObject( const QgsGrassObject & object, const QString& newName );

    // ! Copy GRASS object, throws QgsGrass::Exception
    static GRASS_LIB_EXPORT void copyObject( const QgsGrassObject & srcObject, const QgsGrassObject & destObject );

    // ! Delete map
    static GRASS_LIB_EXPORT bool deleteObject( const QgsGrassObject & object );

    /** Ask user confirmation to delete a map
     *  @return true if confirmed
     */
    static GRASS_LIB_EXPORT bool deleteObjectDialog( const QgsGrassObject & object );

    /** Create new table. Throws  QgsGrass::Exception */
    static GRASS_LIB_EXPORT void createTable( dbDriver *driver, const QString tableName, const QgsFields &fields );

    /** Insert row to table. Throws  QgsGrass::Exception */
    static GRASS_LIB_EXPORT void insertRow( dbDriver *driver, const QString tableName,
                                            const QgsAttributes& attributes );

    /** Returns true if object is link to external data (created by r.external) */
    static GRASS_LIB_EXPORT bool isExternal( const QgsGrassObject & object );

    //! Library version
    static GRASS_LIB_EXPORT int versionMajor();
    static GRASS_LIB_EXPORT int versionMinor();
    static GRASS_LIB_EXPORT int versionRelease();
    static GRASS_LIB_EXPORT QString versionString();

    // files case sensitivity (insensitive on windows)
    static GRASS_LIB_EXPORT Qt::CaseSensitivity caseSensitivity();
    // set environment variable
    static GRASS_LIB_EXPORT void putEnv( QString name, QString value );

#ifdef Q_OS_WIN
    static GRASS_LIB_EXPORT QString shortPath( const QString &path );
#endif

    // path to QGIS GRASS modules like qgis.g.info etc.
    static GRASS_LIB_EXPORT QString qgisGrassModulePath()
    {
#ifdef _MSC_VER
      if ( QgsApplication::isRunningFromBuildDir() )
      {
        return QCoreApplication::applicationDirPath() + "/../../grass/modules/" + QgsApplication::cfgIntDir();
      }
#endif
      return QgsApplication::libexecPath() + "grass/modules";
    }

    // Allocate struct Map_info
    static GRASS_LIB_EXPORT struct Map_info * vectNewMapStruct();
    // Free struct Map_info
    static GRASS_LIB_EXPORT void vectDestroyMapStruct( struct Map_info *map );

    // Sleep miliseconds (for debugging)
    static GRASS_LIB_EXPORT void sleep( int ms );

  private:
    static int initialized; // Set to 1 after initialization
    static bool active; // is active mode
    static QString defaultGisdbase;
    static QString defaultLocation;
    static QString defaultMapset;

    /* last error in GRASS libraries */
    static GERROR lastError;         // static, because used in constructor
    static QString error_message;

    // G_set_error_routine has two versions of the function's first argument it expects:
    // - char* msg - older version
    // - const char* msg - in CVS from 04/2007
    // this way compiler chooses suitable call
    static int error_routine( const char *msg, int fatal ); // static because pointer to this function is set later
    static int error_routine( char *msg, int fatal ); // static because pointer to this function is set later

    // Current mapset lock file path
    static QString mMapsetLock;
    // Current mapset GISRC file path
    static QString mGisrc;
    // Temporary directory where GISRC and sockets are stored
    static QString mTmp;
    // Mutex for common locking when calling GRASS functions which are mostly non thread safe
    static QMutex sMutex;
};

#endif // QGSGRASS_H
