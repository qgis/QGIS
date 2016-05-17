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
#include <QPen>

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
#include <QFileSystemWatcher>
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

// try/catch like macros using setjmp
#if (GRASS_VERSION_MAJOR < 7)
#define G_TRY try { if( !setjmp(QgsGrass::jumper) )
#else
#define G_TRY try { if( !setjmp(*G_fatal_longjmp(1)) )
#endif
#define G_CATCH else { throw QgsGrass::Exception( QgsGrass::errorMessage() ); } } catch

// Throw QgsGrass::Exception if G_fatal_error happens when calling F
#if (GRASS_VERSION_MAJOR < 7)
#define G_FATAL_THROW(F) if( !setjmp(QgsGrass::jumper) ) { F; } else { throw QgsGrass::Exception( QgsGrass::errorMessage() ); }
#else
#define G_FATAL_THROW(F) if( !setjmp(*G_fatal_longjmp(1)) ) { F; } else { throw QgsGrass::Exception( QgsGrass::errorMessage() ); }
#endif

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
    enum Type { None, Location, Mapset, Raster, Group, Vector, Region,
                Strds, Stvds, Str3ds, Stds
            };

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
    /** Return full name (map@mapset)
     * @return full name or empty string if map name is empty */
    QString fullName() const;
    /** Parse full name in map@mapset form and set map and mapset. If mapset is not
     * specified, mapset is set to the current mapset. */
    void setFullName( const QString& fullName );
    Type type() const { return mType; }
    void setType( Type type ) { mType = type; }
    // set from QGIS layer uri, returns true if set correctly, verifies also if location is a GRASS location
    bool setFromUri( const QString& uri );
    // element name used as modules param, e.g. g.remove element=name
    static QString elementShort( Type type );
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

/** QString gisdbase()
   Methods for C library initialization and error handling.
*/
class GRASS_LIB_EXPORT QgsGrass : public QObject
{
    Q_OBJECT
  public:
    static jmp_buf jumper; // used to get back from fatal error

    // Parsed module output
    enum ModuleOutput
    {
      OutputNone,
      OutputPercent,
      OutputProgress, // number of items processed if total number is unknown
      OutputMessage,
      OutputWarning,
      OutputError
    };

    // This does not work (gcc/Linux), such exception cannot be caught
    // so I have enabled the old version, if you are able to fix it, please
    // check first if it realy works, i.e. can be caught!
#if 0
    struct Exception : public QgsException
    {
      Exception( const QString &msg ) : QgsException( msg ) {}
    };
#endif
    struct Exception : public std::runtime_error
    {
      //Exception( const std::string &msg ) : std::runtime_error( msg ) {}
      explicit Exception( const QString &msg ) : std::runtime_error( msg.toUtf8().constData() ) {}
    };

    struct Color
    {
      double value1, value2;
      int red1, red2, green1, green2, blue1, blue2;
    };

    QgsGrass();

    /** Get singleton instance of this class. Used as signals proxy between provider and plugin. */
    static QgsGrass* instance();

    /** Global GRASS library lock */
    static void lock();
    static void unlock();

    //! Get info about the mode
    /** QgsGrass may be running in active or passive mode.
     *  Active mode means that GISRC is set up and GISRC file is available,
     *  in that case default GISDBASE, LOCATION and MAPSET may be read by GetDefaul*() functions.
     *  Passive mode means, that GISRC is not available. */
    static bool activeMode();

    //! Get default GISDBASE, returns GISDBASE name or empty string if not in active mode
    static QString getDefaultGisdbase();

    //! Get default LOCATION_NAME, returns LOCATION_NAME name or empty string if not in active mode
    static QString getDefaultLocation();

    static QgsGrassObject getDefaultLocationObject();

    //! Get default path to location (gisdbase/location) or empty string if not in active mode
    static QString getDefaultLocationPath();

    //! Get default MAPSET, returns MAPSET name or empty string if not in active mode
    static QString getDefaultMapset();

    static QgsGrassObject getDefaultMapsetObject();

    //! Get default path to MAPSET (gisdbase/location/mapset) or empty string if not in active mode
    static QString getDefaultMapsetPath();

    /** Init or reset GRASS library
     *
     * @param gisdbase full path to GRASS GISDBASE.
     * @param location location name (not path!).
     */
    static void setLocation( QString gisdbase, QString location );

    /*!
     * @param gisdbase full path to GRASS GISDBASE.
     * @param location location name (not path!).
     * @param mapset current mupset. Note that some variables depend on mapset and
     * may influence behaviour of some functions (e.g. search path etc.)
     */
    static void setMapset( QString gisdbase, QString location, QString mapset );

    /** Set mapset according to object gisdbase, location and mapset
     * @param grassObject
     */
    static void setMapset( QgsGrassObject grassObject );

    /** Check if mapset is in search pat set by g.mapsets
     *  @return true if in search path
     */
    bool isMapsetInSearchPath( QString mapset );

    /** Add mapset to search path of currently open mapset */
    void addMapsetToSearchPath( const QString & mapset, QString& error );

    /** Add mapset to search path of currently open mapset */
    void removeMapsetFromSearchPath( const QString & mapset, QString& error );

    //! Error codes returned by error()
    enum GERROR
    {
      OK, /*!< OK. No error. */
      WARNING, /*!< Warning, non fatal error. Should be printed by application. */
      FATAL /*!< Fatal error */
    };

    //! Reset error code (to OK). Call this before a piece of code where an error is expected
    static void resetError( void );  // reset error status

    //! Check if any error occurred in lately called functions. Returns value from ERROR.
    static int error( void );

    //! Get last error message
    static QString errorMessage( void );

    //! Get initialization error
    static QString initError() { return mInitError; }

    /** Test is current user is owner of mapset */
    static bool isOwner( const QString& gisdbase, const QString& location, const QString& mapset );

    /** Open existing GRASS mapset.
     * Emits signal mapsetChanged().
     * \return Empty string or error message
     */
    static QString openMapset( const QString& gisdbase,
                               const QString& location, const QString& mapset );

    /** \brief Close mapset if it was opened from QGIS.
     *         Delete GISRC, lock and temporary directory.
     *         Emits signal mapsetChanged().
     * \param showError show error dialog on error
     * \return Empty string or error message
     */
    static QString closeMapset();

    /** \brief Save current mapset to project file. */
    static void saveMapset();

    /** Create new mapset in existing location */
    static void createMapset( const QString& gisdbase, const QString& location,
                              const QString& mapset, QString& error );

    //! Check if given directory contains a GRASS installation
    static bool isValidGrassBaseDir( const QString& gisbase );

    //! Returns list of locations in given gisbase
    static QStringList locations( const QString& gisdbase );

    //! Returns list of mapsets in location
    static QStringList mapsets( const QString& gisdbase, const QString& locationName );
    static QStringList mapsets( const QString& locationPath );

    //! List of vectors and rasters
    static QStringList vectors( const QString& gisdbase, const QString& locationName,
                                const QString& mapsetName );
    static QStringList vectors( const QString& mapsetPath );

    static QStringList rasters( const QString& gisdbase, const QString& locationName,
                                const QString& mapsetName );
    static QStringList rasters( const QString& mapsetPath );

    // imagery groups
    static QStringList groups( const QString& gisdbase, const QString& locationName,
                               const QString& mapsetName );
    static QStringList groups( const QString& mapsetPath );

    //! Get topo file version 6, 7 or 0 if topo file does not exist
    static bool topoVersion( const QString& gisdbase, const QString& location,
                             const QString& mapset, const QString& mapName, int &major, int &minor );

    //! Get list of vector layers, throws QgsGrass::Exception
    static QStringList vectorLayers( const QString& gisdbase, const QString& location,
                                     const QString& mapset, const QString& mapName );

    //! List of elements
    // TODO rename elements to objects
    static QStringList elements( const QString& gisdbase, const QString& locationName,
                                 const QString& mapsetName, const QString& element );
    static QStringList elements( const QString&  mapsetPath, const QString&  element );

    //! List of existing objects
    static QStringList grassObjects( const QgsGrassObject& mapsetObject, QgsGrassObject::Type type );

    // returns true if object (vector, raster, region) exists
    static bool objectExists( const QgsGrassObject& grassObject );

    //! Initialize GRASS region
    static void initRegion( struct Cell_head *window );
    //! Set region extent
    static void setRegion( struct Cell_head *window, QgsRectangle rect );
    /** Init region, set extent, rows and cols and adjust.
     * Returns error if adjustment failed. */
    static QString setRegion( struct Cell_head *window, QgsRectangle rect, int rows, int cols );

    //! Get extent from region
    static QgsRectangle extent( struct Cell_head *window );

    // ! Get map region
    static bool mapRegion( QgsGrassObject::Type type, QString gisdbase,
                           QString location, QString mapset, QString map,
                           struct Cell_head *window );

    // ! String representation of region
    static QString regionString( const struct Cell_head *window );

    // ! Read location default region (DEFAULT_WIND)
    static bool defaultRegion( const QString& gisdbase, const QString& location,
                               struct Cell_head *window );

    /** Read mapset current region (WIND)
     * @throws QgsGrass::Exception
     */
    static void region( const QString& gisdbase, const QString& location, const QString& mapset,
                        struct Cell_head *window );

    /** Read default mapset current region (WIND)
     * @throws QgsGrass::Exception
     */
    static void region( struct Cell_head *window );

    // ! Write current mapset region
    static bool writeRegion( const QString& gisbase, const QString& location, const QString& mapset,
                             const struct Cell_head *window );

    /** Write current mapset region
     *  throws QgsGrass::Exception
     *  Emits regionChanged */
    void writeRegion( const struct Cell_head *window );

    // ! Set (copy) region extent, resolution is not changed
    static void copyRegionExtent( struct Cell_head *source,
                                  struct Cell_head *target );

    // ! Set (copy) region resolution, extent is not changed
    static void copyRegionResolution( struct Cell_head *source,
                                      struct Cell_head *target );

    // ! Extend region in target to source
    static void extendRegion( struct Cell_head *source,
                              struct Cell_head *target );

    /** Initialize GRASS library. This has to be called before any other function is used.
     * @return true if successfully initialized */
    static bool init( void );

    //! test if the directory is location
    static bool isLocation( const QString& path );

    // ! test if the directory is mapset
    static bool isMapset( const QString& path );

    // ! Get the lock file
    static QString lockFilePath();

    // ! Get current gisrc path
    static QString gisrcFilePath();

    /** Find a module trying to append .bat, .py and .exe on Windows. The module may be a full path
     * without extension or just a module name in which case it is searched in grassModulesPaths().
     * @param module module name or path to module without extension
     * @return full path including extension or empty string */
    static QString findModule( QString module );

    /** Start a GRASS module in any gisdbase/location/mapset.
     * @param mapset if empty a first mapset owned by user will be used, if no mapset is owned
     *               by user, exception is thrown.
     * @param qgisModule append GRASS major version (for modules built in qgis)
     * @throws QgsGrass::Exception */
    static QProcess *startModule( const QString& gisdbase, const QString&  location,
                                  const QString& mapset, const QString&  moduleName,
                                  const QStringList& arguments, QTemporaryFile &gisrcFile,
                                  bool qgisModule = true );

    // ! Run a GRASS module in any gisdbase/location
    static QByteArray runModule( const QString& gisdbase, const QString&  location,
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
    static QString getInfo( const QString&  info, const QString&  gisdbase,
                            const QString& location, const QString&  mapset = "PERMANENT",
                            const QString& map = QString::null, const QgsGrassObject::Type type = QgsGrassObject::None,
                            double x = 0.0, double y = 0.0,
                            const QgsRectangle& extent = QgsRectangle(), int sampleRows = 0,
                            int sampleCols = 0, int timeOut = 30000 );

    // ! Get location projection
    static QgsCoordinateReferenceSystem crs( const QString& gisdbase, const QString& location, QString &error );

    // ! Get location projection calling directly GRASS library
    static QgsCoordinateReferenceSystem crsDirect( const QString& gisdbase, const QString& location );

    // ! Get map extent
    // @param error set to error if happens
    static QgsRectangle extent( const QString& gisdbase, const QString& location,
                                const QString& mapset, const QString& map,
                                QgsGrassObject::Type type, QString &error );

    // ! Get raster map size
    static void size( const QString& gisdbase, const QString& location, const QString& mapset,
                      const QString& map, int *cols, int *rows, QString &error );

    // ! Get raster info, info is either 'info' or 'stats'
    //   extent and sampleSize are stats options
    // @param error set to error if happens
    static QHash<QString, QString> info( const QString& gisdbase, const QString& location,
                                         const QString& mapset, const QString& map,
                                         QgsGrassObject::Type type,
                                         const QString& info,
                                         const QgsRectangle& extent,
                                         int sampleRows, int sampleCols,
                                         int timeOut, QString &error );

    // ! List of Color
    static QList<QgsGrass::Color> colors( QString gisdbase, QString location, QString mapset,
                                          QString map, QString& error );

    // ! Get map value / feature info
    static QMap<QString, QString> query( QString gisdbase, QString location,
                                         QString mapset, QString map, QgsGrassObject::Type type, double x, double y );

    // ! Rename GRASS object, throws QgsGrass::Exception
    static void renameObject( const QgsGrassObject & object, const QString& newName );

    // ! Copy GRASS object, throws QgsGrass::Exception
    static void copyObject( const QgsGrassObject & srcObject, const QgsGrassObject & destObject );

    // ! Delete map
    static bool deleteObject( const QgsGrassObject & object );

    /** Ask user confirmation to delete a map
     *  @return true if confirmed
     */
    static bool deleteObjectDialog( const QgsGrassObject & object );

    /** Create new vector map
     * @param object GRASS object specifying location/mapset/map
     * @param error */
    static void createVectorMap( const QgsGrassObject & object, QString &error );

    /** Create new table. Throws  QgsGrass::Exception */
    static void createTable( dbDriver *driver, const QString tableName, const QgsFields &fields );

    /** Insert row to table. Throws  QgsGrass::Exception */
    static void insertRow( dbDriver *driver, const QString tableName,
                           const QgsAttributes& attributes );

    /** Returns true if object is link to external data (created by r.external) */
    static bool isExternal( const QgsGrassObject & object );

    /** Adjust cell header, G_adjust_Cell_head wrapper
     * @throws QgsGrass::Exception */
    static void adjustCellHead( struct Cell_head *cellhd, int row_flag, int col_flag );

    /** Get map of vector types / names */
    static QMap<int, QString> vectorTypeMap();

    /** Get GRASS vector type from name
     * @param point,centroid,line,boundary,area,face,kernel
     * @returns type GV_POINT, GV_CENTROID, GV_LINE, GV_BOUNDARY, GV_AREA, GV_FACE,GV_KERNEL  */
    static int vectorType( const QString & name );

    /** Get name for vector primitive type
     * @param type GV_POINT, GV_CENTROID, GV_LINE, GV_BOUNDARY, GV_AREA, GV_FACE, GV_KERNEL  */
    static QString vectorTypeName( int type );

    //! Library version
    static int versionMajor();
    static int versionMinor();
    static int versionRelease();
    static QString versionString();

    // files case sensitivity (insensitive on windows)
    static Qt::CaseSensitivity caseSensitivity();
    // set environment variable
    static void putEnv( QString name, QString value );

    // platform dependent PATH separator
    static QString pathSeparator();
#ifdef Q_OS_WIN
    static QString shortPath( const QString &path );
#endif

    // Dirs where GRASS modules (executables or scripts) should be searched for
    // On windows it also includes path to msys/bin/ for commands like sed, grep etc. (should be separated?)
    // It does not contain paths from PATH environment variable
    static QStringList grassModulesPaths()
    {
      return mGrassModulesPaths;
    }

    // path to QGIS GRASS modules like qgis.g.info etc.
    static QString qgisGrassModulePath()
    {
#ifdef _MSC_VER
      if ( QgsApplication::isRunningFromBuildDir() )
      {
        return QCoreApplication::applicationDirPath() + "/../../grass/modules/" + QgsApplication::cfgIntDir();
      }
#endif
      return QgsApplication::libexecPath() + "grass/modules";
    }

    // Get PYTHONPATH with paths to GRASS Python modules
    static QString getPythonPath();

    // path to GRASS installation
    static QString defaultGisbase();

    // current path to GRASS installation dir (default or custom)
    static QString gisbase();

    // set custom path to GRASS installation, emits gisbaseChanged
    void setGisbase( bool custom, const QString &customDir );

    // path to default modules interface config dir
    static QString modulesConfigDefaultDirPath();

    // path to modules interface config dir (default or custom)
    static QString modulesConfigDirPath();

    void setModulesConfig( bool custom, const QString &customDir );

    static QPen regionPen();

    /** Store region pen in settings, emits regionPenChanged */
    void setRegionPen( const QPen & pen );

    // Modules UI debug
    static bool modulesDebug();

    // Switch modules UI debug
    void setModulesDebug( bool debug );

    /** Show warning dialog with message */
    static void warning( const QString &message );

    /** Show warning dialog with exception message */
    static void warning( QgsGrass::Exception &e );

    /** Set mute mode, if set, warning() does not open dialog but prints only
     * debug message and sets the error which returns errorMessage() */
    static void setMute() { mMute = true; }

    /** Allocate struct Map_info. Call to this function may result in G_fatal_error
     * and must be surrounded by G_TRY/G_CATCH. */
    static struct Map_info * vectNewMapStruct();
    // Free struct Map_info
    static void vectDestroyMapStruct( struct Map_info *map );

    // Sleep miliseconds (for debugging), does not work on threads(?)
    static void sleep( int ms );

    void emitNewLayer( QString uri, QString name ) { emit newLayer( uri, name ); }

    /** Parse single line of output from GRASS modules run with GRASS_MESSAGE_FORMAT=gui
     * @param input input string read from module stderr
     * @param text parsed text
     * @param html html formated parsed text, e.g. + icons
     * @param value percent 0-100 or progress as absolute number if total is unknown*/
    static ModuleOutput parseModuleOutput( const QString & input, QString &text, QString &html, int &value );

  public slots:
    /** Close mapset and show warning if closing failed */
    bool closeMapsetWarn();

    void openOptions();

    /** Read mapset search path from GRASS location */
    void loadMapsetSearchPath();

    void setMapsetSearchPathWatcher();
    void onSearchPathFileChanged( const QString & path );

  signals:
    /** Signal emitted  when user changed GISBASE */
    void gisbaseChanged();

    /** Signal emitted after mapset was opened */
    void mapsetChanged();

    /** Signal emitted when mapset search path changed (SEARCH_PATH file changed and it was loaded to mMapsetSearchPath) */
    void mapsetSearchPathChanged();

    /** Emitted when path to modules config dir changed */
    void modulesConfigChanged();

    /** Emitted when modules debug mode changed */
    void modulesDebugChanged();

    /** Emitted when current region changed
     *  TODO: currently only emitted when writeRegion is called, add file system watcher
     *  to get also changes done outside QGIS or by modules.
     */
    void regionChanged();

    /** Emitted when region pen changed */
    void regionPenChanged();

    /** Request from browser to open a new layer for editing, the plugin should connect
     * to this signal and add the layer to canvas and start editing. */
    void newLayer( QString uri, QString name );

  private:
    static bool mNonInitializable;
    static int initialized; // Set to 1 after initialization
    static bool active; // is active mode
    static QStringList mGrassModulesPaths;
    static QString defaultGisdbase;
    static QString defaultLocation;
    static QString defaultMapset;

    // Mapsets in current search path
    QStringList mMapsetSearchPath;
    QFileSystemWatcher *mMapsetSearchPathWatcher;

    /* last error in GRASS libraries */
    static GERROR lastError;         // static, because used in constructor
    static QString error_message;
    // error set in init() if it failed
    static QString mInitError;

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
    // Mute mode, do not show warning dialogs.
    static bool mMute;
};

#endif // QGSGRASS_H
