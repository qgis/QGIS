/***************************************************************************
    qgsapplication.h - Accessors for application-wide data
     --------------------------------------
    Date                 : 02-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPLICATION_H
#define QGSAPPLICATION_H

#include "qgis_core.h"
#include <QApplication>
#include <QEvent>
#include <QStringList>

#include "qgis.h"
#include "qgsconfig.h"
#include "qgstranslationcontext.h"

class Qgs3DRendererRegistry;
class QgsActionScopeRegistry;
class QgsRuntimeProfiler;
class QgsTaskManager;
class QgsFieldFormatterRegistry;
class QgsColorSchemeRegistry;
class QgsPaintEffectRegistry;
class QgsProjectStorageRegistry;
class QgsRendererRegistry;
class QgsSvgCache;
class QgsImageCache;
class QgsSymbolLayerRegistry;
class QgsRasterRendererRegistry;
class QgsGpsConnectionRegistry;
class QgsDataItemProviderRegistry;
class QgsPluginLayerRegistry;
class QgsMessageLog;
class QgsProcessingRegistry;
class QgsAnnotationRegistry;
class QgsUserProfile;
class QgsUserProfileManager;
class QgsPageSizeRegistry;
class QgsLayoutItemRegistry;
class QgsAuthManager;
class QgsNetworkContentFetcherRegistry;
class QgsValidityCheckRegistry;
class QTranslator;

/**
 * \ingroup core
 * Extends QApplication to provide access to QGIS specific resources such
 * as theme paths, database paths etc.
 *
 * This is a subclass of QApplication and should be instantiated in place of
  QApplication. Most methods are static in keeping with the design of QApplication.

  This class hides platform-specific path information and provides
  a portable way of referencing specific files and directories.
  Ideally, hard-coded paths should appear only here and not in other modules
  so that platform-conditional code is minimized and paths are easier
  to change due to centralization.
 */

class CORE_EXPORT QgsApplication : public QApplication
{

#ifdef SIP_RUN
    % TypeCode
    // Convert a Python argv list to a conventional C argc count and argv array.
    static char **qtgui_ArgvToC( PyObject *argvlist, int &argc )
    {
      char **argv;

      argc = PyList_GET_SIZE( argvlist );

      // Allocate space for two copies of the argument pointers, plus the
      // terminating NULL.
      if ( ( argv = ( char ** )sipMalloc( 2 * ( argc + 1 ) * sizeof( char * ) ) ) == NULL )
        return NULL;

      // Convert the list.
      for ( int a = 0; a < argc; ++a )
      {
        char *arg;
        // Get the argument and allocate memory for it.
        if ( ( arg = PyBytes_AsString( PyList_GET_ITEM( argvlist, a ) ) ) == NULL ||
             ( argv[a] = ( char * )sipMalloc( strlen( arg ) + 1 ) ) == NULL )
          return NULL;
        // Copy the argument and save a pointer to it.
        strcpy( argv[a], arg );
        argv[a + argc + 1] = argv[a];
      }

      argv[argc + argc + 1] = argv[argc] = NULL;

      return argv;
    }

    // Remove arguments from the Python argv list that have been removed from the
    // C argv array.
    static void qtgui_UpdatePyArgv( PyObject *argvlist, int argc, char **argv )
    {
      for ( int a = 0, na = 0; a < argc; ++a )
      {
        // See if it was removed.
        if ( argv[na] == argv[a + argc + 1] )
          ++na;
        else
          PyList_SetSlice( argvlist, na, na + 1, NULL );
      }
    }
    % End
#endif

    Q_OBJECT

  public:

    static const char *QGIS_ORGANIZATION_NAME;
    static const char *QGIS_ORGANIZATION_DOMAIN;
    static const char *QGIS_APPLICATION_NAME;
#ifndef SIP_RUN
    QgsApplication( int &argc, char **argv, bool GUIenabled, const QString &profileFolder = QString(), const QString &platformName = "desktop" );
#else
    QgsApplication( SIP_PYLIST argv, bool GUIenabled, QString profileFolder = QString(), QString platformName = "desktop" ) / PostHook = __pyQtQAppHook__ / [( int &argc, char **argv, bool GUIenabled, const QString &profileFolder = QString(), const QString &platformName = "desktop" )];
    % MethodCode
    // The Python interface is a list of argument strings that is modified.

    int argc;
    char **argv;

    // Convert the list.
    if ( ( argv = qtgui_ArgvToC( a0, argc ) ) == NULL )
      sipIsErr = 1;
    else
    {
      // Create it now the arguments are right.
      static int nargc = argc;

      sipCpp = new sipQgsApplication( nargc, argv, a1, *a2, *a3 );

      // Now modify the original list.
      qtgui_UpdatePyArgv( a0, argc, argv );
    }
    % End
#endif

    ~QgsApplication() override;

    /**
     * Returns the singleton instance of the QgsApplication.
     *
     * \since QGIS 3.0
     */
    static QgsApplication *instance();

    /**
     * This method initializes paths etc for QGIS. Called by the ctor or call it manually
        when your app does not extend the QApplication class.
        \note you will probably want to call initQgis too to load the providers in
        the above case.
        \note not available in Python bindings
      */
    static void init( QString profileFolder = QString() ) SIP_SKIP;

    //! Watch for QFileOpenEvent.
    bool event( QEvent *event ) override;

    //! Catch exceptions when sending event to receiver.
    bool notify( QObject *receiver, QEvent *event ) override;

    //! Sets the FileOpen event receiver
    static void setFileOpenEventReceiver( QObject *receiver );

    /**
     * Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use themeName() as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     * \note A basic test will be carried out to ensure the theme search path
     * based on the supplied theme name exists. If it does not the theme name will
     * be reverted to 'default'.
     */
    static void setThemeName( const QString &themeName );

    /**
     * Calculate the application pkg path
     * \return the resolved pkg path
     */
    static QString resolvePkgPath();

    /**
     * Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use this method as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     */
    static QString themeName();

    /**
     * \brief Set the current UI theme used to style the interface.  Use uiThemes() to
     * find valid themes to use. Variables found in variables.qss will be added to the stylesheet
     * on load.
     * \param themeName The name of the theme.
     * \note using an invalid theme name will reset to default
     */
    static void setUITheme( const QString &themeName );

    /**
     * \brief All themes found in ~/.qgis3/themes folder.
     * The path is to the root folder for the theme
     * \returns A hash of theme name and theme path. Valid theme folders contain style.qss
     * \note Valid theme folders must contain a style.qss file.
     */
    static QHash<QString, QString> uiThemes();

    //! Returns the path to the authors file.
    static QString authorsFilePath();

    /**
     * Returns the path to the contributors file.
     * Contributors are people who have submitted patches
     * but don't have commit access. */
    static QString contributorsFilePath();

    /**
     * Returns the path to the developers map file.
     * The developers map was created by using leaflet framework,
     * it shows the contributors.json file.
     * \since QGIS 2.7 */
    static QString developersMapFilePath();

    //! Returns the path to the sponsors file.
    static QString sponsorsFilePath();

    //! Returns the path to the donors file.
    static QString donorsFilePath();

    //! Returns the path to the server resources directory.
    static QString serverResourcesPath();

    /**
     * Returns the path to the sponsors file.
     */
    static QString translatorsFilePath();

    /**
      Returns the path to the licence file.
     */
    static QString licenceFilePath();

    //! Returns the path to the translation directory.
    static QString i18nPath();

    /**
     * Returns the path to the metadata directory.
    * \since QGIS 3.0
    */
    static QString metadataPath();

    //! Returns the path to the master qgis.db file.
    static QString qgisMasterDatabaseFilePath();

    //! Returns the path to the settings directory in user's home dir
    static QString qgisSettingsDirPath();

    //! Returns the path to the user qgis.db file.
    static QString qgisUserDatabaseFilePath();

    //! Returns the path to the user authentication database file: qgis-auth.db.
    static QString qgisAuthDatabaseFilePath();

    //! Returns the path to the splash screen image directory.
    static QString splashPath();

    //! Returns the path to the icons image directory.
    static QString iconsPath();

    //! Returns the path to the srs.db file.
    static QString srsDatabaseFilePath();

    //! Returns the paths to svg directories.
    static QStringList svgPaths();

    /**
     * Returns the paths to layout template directories.
     * \since QGIS 3.0
     */
    static QStringList layoutTemplatePaths();

    //! Returns the system environment variables passed to application.
    static QMap<QString, QString> systemEnvVars() { return ABISYM( mSystemEnvVars ); }

    //! Returns the path to the application prefix directory.
    static QString prefixPath();

    //! Returns the path to the application plugin directory.
    static QString pluginPath();

    //! Returns the common root path of all application data directories.
    static QString pkgDataPath();

    //! Returns the path to the currently active theme directory.
    static QString activeThemePath();

    //! Returns the path to the default theme directory.
    static QString defaultThemePath();

    /**
     * Returns path to the desired icon file.
     * First it tries to use the active theme path, then default theme path
     */
    static QString iconPath( const QString &iconFile );

    /**
     * Helper to get a theme icon. It will fall back to the
     * default theme if the active theme does not have the required icon.
     */
    static QIcon getThemeIcon( const QString &name );

    /**
     * \brief The Cursor enum defines constants for QGIS custom
     * cursors.
     */
    enum Cursor
    {
      ZoomIn, //!< Zoom in
      ZoomOut, //!< Zoom out
      Identify, //!< Identify: obtain information about the object
      CrossHair, //!< Precisely identify a point on the canvas
      CapturePoint, //!< Select and capture a point or a feature
      Select, //!< Select a rectangle
      Sampler, //!< Color/Value picker
    };

    /**
     * Helper to get a theme cursor. It will fall back to the
     * default theme if the active theme does not have the required icon.
     * Cursors are automatically scaled to look like a 16px cursor on 96dpi
     * screens.
     */
    static QCursor getThemeCursor( Cursor cursor );

    /**
     * Helper to get a theme icon as a pixmap. It will fall back to the
     * default theme if the active theme does not have the required icon.
     */
    static QPixmap getThemePixmap( const QString &name );

    //! Returns the path to user's style.
    static QString userStylePath();

    //! Returns the short name regular expression for line edit validator
    static QRegExp shortNameRegExp();

    /**
     * Returns the user's operating system login account name.
     * \see userFullName()
     * \since QGIS 2.14
     */
    static QString userLoginName();

    /**
     * Returns the user's operating system login account full display name.
     * \see userLoginName()
     * \since QGIS 2.14
     */
    static QString userFullName();

    /**
     * Returns a string name of the operating system QGIS is running on.
     * \see platform()
     * \since QGIS 2.14
     */
    static QString osName();

    /**
     * Returns the QGIS platform name, e.g., "desktop" or "server".
     * \see osName()
     * \since QGIS 2.14
     */
    static QString platform();

    /**
     * Returns the QGIS locale.
     * \since QGIS 3.0
     */
    static QString locale();

    //! Returns the path to user's themes folder
    static QString userThemesFolder();

    //! Returns the path to default style (works as a starting point).
    static QString defaultStylePath();

    //! Returns the path to default themes folder from install (works as a starting point).
    static QString defaultThemesFolder();

    //! Returns the path containing qgis_core, qgis_gui, qgispython (and other) libraries
    static QString libraryPath();

    //! Returns the path with utility executables (help viewer, crssync, ...)
    static QString libexecPath();

    /**
     * Returns the path where QML components are installed for QGIS Quick library. Returns
     * empty string when QGIS is built without Quick support
     *
     * \since QGIS 3.2
     */
    static QString qmlImportPath();

    //! Alters prefix path - used by 3rd party apps
    static void setPrefixPath( const QString &prefixPath, bool useDefaultPaths = false );

    //! Alters plugin path - used by 3rd party apps
    static void setPluginPath( const QString &pluginPath );

    //! Alters pkg data path - used by 3rd party apps
    static void setPkgDataPath( const QString &pkgDataPath );

    //! Alters default svg paths - used by 3rd party apps.
    static void setDefaultSvgPaths( const QStringList &pathList );

    //! Alters authentication data base directory path - used by 3rd party apps
    static void setAuthDatabaseDirPath( const QString &authDbDirPath );

    //! loads providers
    static void initQgis();

    //! initialize qgis.db
    static bool createDatabase( QString *errorMessage = nullptr );

    //! Create the users theme folder
    static bool createThemeFolder();

    //! deletes provider registry and map layer registry
    static void exitQgis();

    //! Gets application icon
    static QString appIconPath();

    //! Constants for endian-ness
    enum endian_t
    {
      XDR = 0,  // network, or big-endian, byte order
      NDR = 1   // little-endian byte order
    };

    //! Returns whether this machine uses big or little endian
    static endian_t endian();

    /**
     * Swap the endianness of the specified value.
     * \note not available in Python bindings
     */
#ifndef SIP_RUN
    template<typename T>
    static void endian_swap( T &value )
    {
      char *data = reinterpret_cast<char *>( &value );
      std::size_t n = sizeof( value );
      for ( std::size_t i = 0, m = n / 2; i < m; ++i )
      {
        std::swap( data[i], data[n - 1 - i] );
      }
    }
#endif

    /**
     * Returns a standard css style sheet for reports.
     *
     * Typically you will use this method by doing:
     * QString myStyle = QgsApplication::reportStyleSheet();
     * textBrowserReport->document()->setDefaultStyleSheet(myStyle);
     *
     * \returns QString containing the CSS 2.1 compliant stylesheet.
     * \note you can use the special Qt extensions too, for example
     * the gradient fills for backgrounds.
     */
    static QString reportStyleSheet();

    /**
     * Convenience function to get a summary of the paths used in this
     * application instance useful for debugging mainly.*/
    static QString showSettings();

    /**
     * Register OGR drivers ensuring this only happens once.
     * This is a workaround for an issue with older gdal versions that
     * caused duplicate driver name entries to appear in the list
     * of registered drivers when QgsApplication::registerOgrDrivers was called multiple
     * times.
     */
    static void registerOgrDrivers();

    //! Converts absolute path to path relative to target
    static QString absolutePathToRelativePath( const QString &apath, const QString &targetPath );
    //! Converts path relative to target to an absolute path
    static QString relativePathToAbsolutePath( const QString &rpath, const QString &targetPath );

    //! Indicates whether running from build directory (not installed)
    static bool isRunningFromBuildDir() { return ABISYM( mRunningFromBuildDir ); }
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    static QString cfgIntDir() { return ABISYM( mCfgIntDir ); } SIP_SKIP
#endif
    //! Returns path to the source directory. Valid only when running from build directory
    static QString buildSourcePath() { return ABISYM( mBuildSourcePath ); }
    //! Returns path to the build output directory. Valid only when running from build directory
    static QString buildOutputPath() { return ABISYM( mBuildOutputPath ); }

    /**
     * Sets the GDAL_SKIP environment variable to include the specified driver
     * and then calls GDALDriverManager::AutoSkipDrivers() to unregister it. The
     * driver name should be the short format of the Gdal driver name e.g. GTIFF.
     */
    static void skipGdalDriver( const QString &driver );

    /**
     * Sets the GDAL_SKIP environment variable to exclude the specified driver
     * and then calls GDALDriverManager::AutoSkipDrivers() to unregister it. The
     * driver name should be the short format of the Gdal driver name e.g. GTIFF.
     */
    static void restoreGdalDriver( const QString &driver );

    /**
     * Returns the list of gdal drivers that should be skipped (based on
     * GDAL_SKIP environment variable)
     */
    static QStringList skippedGdalDrivers() { return ABISYM( mGdalSkipList ); }

    /**
     * Apply the skipped drivers list to gdal
     * \see skipGdalDriver
     * \see restoreGdalDriver
     * \see skippedGdalDrivers */
    static void applyGdalSkippedDrivers();

    /**
     * Gets maximum concurrent thread count
     * \since QGIS 2.4 */
    static int maxThreads() { return ABISYM( mMaxThreads ); }

    /**
     * Set maximum concurrent thread count
     * \note must be between 1 and \#cores, -1 means use all available cores
     * \since QGIS 2.4 */
    static void setMaxThreads( int maxThreads );

    /**
     * Returns the application's task manager, used for managing application
     * wide background task handling.
     * \since QGIS 3.0
     */
    static QgsTaskManager *taskManager();

    /**
     * Returns the application's color scheme registry, used for managing color schemes.
     * \since QGIS 3.0
     */
    static QgsColorSchemeRegistry *colorSchemeRegistry();

    /**
     * Returns the application's paint effect registry, used for managing paint effects.
     * \since QGIS 3.0
     */
    static QgsPaintEffectRegistry *paintEffectRegistry();

    /**
     * Returns the application's renderer registry, used for managing vector layer renderers.
     * \since QGIS 3.0
     */
    static QgsRendererRegistry *rendererRegistry();

    /**
     * Returns the application's raster renderer registry, used for managing raster layer renderers.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    static QgsRasterRendererRegistry *rasterRendererRegistry() SIP_SKIP;

    /**
     * Returns the application's data item provider registry, which keeps a list of data item
     * providers that may add items to the browser tree.
     * \since QGIS 3.0
     */
    static QgsDataItemProviderRegistry *dataItemProviderRegistry();

    /**
     * Returns the application's SVG cache, used for caching SVG images and handling parameter replacement
     * within SVG files.
     *
     * \see imageCache()
     * \since QGIS 3.0
     */
    static QgsSvgCache *svgCache();

    /**
     * Returns the application's image cache, used for caching resampled versions of raster images.
     *
     * \see svgCache()
     * \since QGIS 3.6
     */
    static QgsImageCache *imageCache();

    /**
     * Returns the application's network content registry used for fetching temporary files during QGIS session
     * \since QGIS 3.2
     */
    static QgsNetworkContentFetcherRegistry *networkContentFetcherRegistry();

    /**
     * Returns the application's validity check registry, used for managing validity checks.
     * \since QGIS 3.6
     */
    static QgsValidityCheckRegistry *validityCheckRegistry();

    /**
     * Returns the application's symbol layer registry, used for managing symbol layers.
     * \since QGIS 3.0
     */
    static QgsSymbolLayerRegistry *symbolLayerRegistry();

    /**
     * Returns the application's layout item registry, used for layout item types.
     * \since QGIS 3.0
     */
    static QgsLayoutItemRegistry *layoutItemRegistry();

    /**
     * Returns the application's GPS connection registry, used for managing GPS connections.
     * \since QGIS 3.0
     */
    static QgsGpsConnectionRegistry *gpsConnectionRegistry();

    /**
     * Returns the application's plugin layer registry, used for managing plugin layer types.
     * \since QGIS 3.0
     */
    static QgsPluginLayerRegistry *pluginLayerRegistry();

    /**
     * Returns the application's message log.
     * \since QGIS 3.0
     */
    static QgsMessageLog *messageLog();

    /**
     * Returns the application's authentication manager instance
     * \note this can be a null pointer if called before initQgis
     * \see initQgis
     * \since QGIS 3.0
     */
    static QgsAuthManager *authManager();

    /**
     * Returns the application's processing registry, used for managing processing providers,
     * algorithms, and various parameters and outputs.
     * \since QGIS 3.0
     */
    static QgsProcessingRegistry *processingRegistry();

    /**
     * Returns the application's page size registry, used for managing layout page sizes.
     * \since QGIS 3.0
     */
    static QgsPageSizeRegistry *pageSizeRegistry();

    /**
     * Returns the application's annotation registry, used for managing annotation types.
     * \note not available in Python bindings
     * \since QGIS 3.0
     */
    static QgsAnnotationRegistry *annotationRegistry() SIP_SKIP;

    /**
     * Returns the action scope registry.
     *
     * \since QGIS 3.0
     */
    static QgsActionScopeRegistry *actionScopeRegistry();

    /**
     * Returns the application runtime profiler.
     * \since QGIS 3.0
     */
    static QgsRuntimeProfiler *profiler();

    /**
     * Gets the registry of available field formatters.
     */
    static QgsFieldFormatterRegistry *fieldFormatterRegistry();

    /**
     * Returns registry of available 3D renderers.
     * \since QGIS 3.0
     */
    static Qgs3DRendererRegistry *renderer3DRegistry();

    /**
     * Returns registry of available project storage implementations.
     * \since QGIS 3.2
     */
    static QgsProjectStorageRegistry *projectStorageRegistry();

    /**
     * This string is used to represent the value `NULL` throughout QGIS.
     *
     * In general, when passing values around, prefer to use a null QVariant
     * `QVariant( field.type() )` or `QVariant( QVariant::Int )`. This value
     * should only be used in the final presentation step when showing values
     * in a widget or sending it to a web browser.
     */
    static QString nullRepresentation();

    /**
     * \copydoc nullRepresentation()
     */
    static void setNullRepresentation( const QString &nullRepresentation );

    /**
     * Custom expression variables for this application.
     * This does not include generated variables (like system name, user name etc.)
     *
     * \see QgsExpressionContextUtils::globalScope().
     * \since QGIS 3.0
     */
    static QVariantMap customVariables();

    /**
     * Custom expression variables for this application.
     * Do not include generated variables (like system name, user name etc.)
     *
     * \see QgsExpressionContextUtils::globalScope().
     * \since QGIS 3.0
     */
    static void setCustomVariables( const QVariantMap &customVariables );

    /**
     * Set a single custom expression variable.
     *
     * \since QGIS 3.0
     */
    static void setCustomVariable( const QString &name, const QVariant &value );

    /**
     * The maximum number of concurrent connections per connections pool.
     *
     * \note QGIS may in some situations allocate more than this amount
     *       of connections to avoid deadlocks.
     *
     * \since QGIS 3.4
     */
    int maxConcurrentConnectionsPerPool() const;

    /**
     * Set translation
     *
     * \since QGIS 3.4
     */
    static void setTranslation( const QString &translation ) { sTranslation = translation; }

    /**
     * Emits the signal to collect all the strings of .qgs to be included in ts file
     *
     * \since QGIS 3.4
     */
    void collectTranslatableObjects( QgsTranslationContext *translationContext );

#ifdef SIP_RUN
    SIP_IF_FEATURE( ANDROID )
    //dummy method to workaround sip generation issue
    bool x11EventFilter( XEvent *event );
    SIP_END
#endif

  signals:
    //! \note not available in Python bindings
    void preNotify( QObject *receiver, QEvent *event, bool *done ) SIP_SKIP;

    /**
     * Emitted whenever a custom global variable changes.
     * \since QGIS 3.0
     */
    void customVariablesChanged();


    /**
     * \copydoc nullRepresentation()
     */
    void nullRepresentationChanged();

    /**
     * Emitted when project strings which require translation are being collected for inclusion in a .ts file.
     * In order to register translatable strings, connect to this signal and register the strings within the specified \a translationContext.
     *
     * \since QGIS 3.4
     */
    void requestForTranslatableObjects( QgsTranslationContext *translationContext );

  private:

    static void copyPath( const QString &src, const QString &dst );
    static QObject *ABISYM( mFileOpenEventReceiver );
    static QStringList ABISYM( mFileOpenEventList );

    static QString ABISYM( mProfilePath );
    static QString ABISYM( mUIThemeName );
    static QString ABISYM( mPrefixPath );
    static QString ABISYM( mPluginPath );
    static QString ABISYM( mPkgDataPath );
    static QString ABISYM( mLibraryPath );
    static QString ABISYM( mLibexecPath );
    static QString ABISYM( mQmlImportPath );
    static QString ABISYM( mThemeName );
    static QStringList ABISYM( mDefaultSvgPaths );
    static QMap<QString, QString> ABISYM( mSystemEnvVars );

    static QString ABISYM( mConfigPath );

    static bool ABISYM( mInitialized );

    //! True when running from build directory, i.e. without 'make install'
    static bool ABISYM( mRunningFromBuildDir );
    //! Path to the source directory. valid only when running from build directory.
    static QString ABISYM( mBuildSourcePath );
#if defined(_MSC_VER) && !defined(USING_NMAKE) && !defined(USING_NINJA)
    //! Configuration internal dir
    static QString ABISYM( mCfgIntDir );
#endif
    //! Path to the output directory of the build. valid only when running from build directory
    static QString ABISYM( mBuildOutputPath );

    /**
     * List of gdal drivers to be skipped. Uses GDAL_SKIP to exclude them.
     * \see skipGdalDriver, restoreGdalDriver */
    static QStringList ABISYM( mGdalSkipList );

    /**
     * \since QGIS 2.4 */
    static int ABISYM( mMaxThreads );

    /**
     * \since QGIS 2.12 */
    static QString ABISYM( mAuthDbDirPath );

    static QString sUserName;
    static QString sUserFullName;
    static QString sPlatformName;
    static QString sTranslation;

    QMap<QString, QIcon> mIconCache;
    QMap<Cursor, QCursor> mCursorCache;

    QTranslator *mQgisTranslator = nullptr;
    QTranslator *mQtTranslator = nullptr;

    QgsDataItemProviderRegistry *mDataItemProviderRegistry = nullptr;
    QgsAuthManager *mAuthManager = nullptr;

    struct ApplicationMembers
    {
      Qgs3DRendererRegistry *m3DRendererRegistry = nullptr;
      QgsActionScopeRegistry *mActionScopeRegistry = nullptr;
      QgsAnnotationRegistry *mAnnotationRegistry = nullptr;
      QgsColorSchemeRegistry *mColorSchemeRegistry = nullptr;
      QgsFieldFormatterRegistry *mFieldFormatterRegistry = nullptr;
      QgsGpsConnectionRegistry *mGpsConnectionRegistry = nullptr;
      QgsNetworkContentFetcherRegistry *mNetworkContentFetcherRegistry = nullptr;
      QgsValidityCheckRegistry *mValidityCheckRegistry = nullptr;
      QgsMessageLog *mMessageLog = nullptr;
      QgsPaintEffectRegistry *mPaintEffectRegistry = nullptr;
      QgsPluginLayerRegistry *mPluginLayerRegistry = nullptr;
      QgsProcessingRegistry *mProcessingRegistry = nullptr;
      QgsProjectStorageRegistry *mProjectStorageRegistry = nullptr;
      QgsPageSizeRegistry *mPageSizeRegistry = nullptr;
      QgsRasterRendererRegistry *mRasterRendererRegistry = nullptr;
      QgsRendererRegistry *mRendererRegistry = nullptr;
      QgsRuntimeProfiler *mProfiler = nullptr;
      QgsSvgCache *mSvgCache = nullptr;
      QgsImageCache *mImageCache = nullptr;
      QgsSymbolLayerRegistry *mSymbolLayerRegistry = nullptr;
      QgsTaskManager *mTaskManager = nullptr;
      QgsLayoutItemRegistry *mLayoutItemRegistry = nullptr;
      QgsUserProfileManager *mUserConfigManager = nullptr;
      QString mNullRepresentation;

      ApplicationMembers();
      ~ApplicationMembers();
    };

    // Applications members which belong to an instance of QgsApplication
    ApplicationMembers *mApplicationMembers = nullptr;
    // ... but in case QgsApplication is never instantiated (eg with custom designer widgets), we fall back to static members
    static ApplicationMembers *sApplicationMembers;

    static ApplicationMembers *members();
};

// clazy:excludeall=qstring-allocations

#endif
