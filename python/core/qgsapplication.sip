/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsapplication.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsApplication : QApplication
{
%Docstring
Extends QApplication to provide access to QGIS specific resources such
as theme paths, database paths etc.

This is a subclass of QApplication and should be instantiated in place of
QApplication. Most methods are static in keeping with the design of QApplication.

This class hides platform-specific path information and provides
a portable way of referencing specific files and directories.
Ideally, hard-coded paths should appear only here and not in other modules
so that platform-conditional code is minimized and paths are easier
to change due to centralization.
%End

%TypeHeaderCode
#include "qgsapplication.h"
%End
%TypeCode
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
%End
  public:

    static const char *QGIS_ORGANIZATION_NAME;
    static const char *QGIS_ORGANIZATION_DOMAIN;
    static const char *QGIS_APPLICATION_NAME;
    QgsApplication( SIP_PYLIST argv, bool GUIenabled, QString profileFolder = QString(), QString platformName = "desktop" ) / PostHook = __pyQtQAppHook__ / [( int &argc, char **argv, bool GUIenabled, const QString &profileFolder = QString(), const QString &platformName = "desktop" )];
%MethodCode
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
%End

    ~QgsApplication();

    static QgsApplication *instance();
%Docstring
Returns the singleton instance of the QgsApplication.

.. versionadded:: 3.0
%End


    virtual bool event( QEvent *event );

%Docstring
Watch for QFileOpenEvent.
%End

    virtual bool notify( QObject *receiver, QEvent *event );

%Docstring
Catch exceptions when sending event to receiver.
%End

    static void setFileOpenEventReceiver( QObject *receiver );
%Docstring
Set the FileOpen event receiver
%End

    static void setThemeName( const QString &themeName );
%Docstring
Set the active theme to the specified theme.
The theme name should be a single word e.g. 'default','classic'.
The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
but plugin writers etc can use themeName() as a basis for searching
for resources in their own datastores e.g. a Qt4 resource bundle.

.. note::

   A basic test will be carried out to ensure the theme search path
based on the supplied theme name exists. If it does not the theme name will
be reverted to 'default'.
%End

    static QString themeName();
%Docstring
Set the active theme to the specified theme.
The theme name should be a single word e.g. 'default','classic'.
The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
but plugin writers etc can use this method as a basis for searching
for resources in their own datastores e.g. a Qt4 resource bundle.
%End

    static void setUITheme( const QString &themeName );
%Docstring
 Set the current UI theme used to style the interface.  Use uiThemes() to
find valid themes to use. Variables found in variables.qss will be added to the stylesheet
on load.

:param themeName: The name of the theme.

.. note::

   using an invalid theme name will reset to default
%End

    static QHash<QString, QString> uiThemes();
%Docstring
 All themes found in ~/.qgis3/themes folder.
The path is to the root folder for the theme

:return: A hash of theme name and theme path. Valid theme folders contain style.qss

.. note::

   Valid theme folders must contain a style.qss file.
%End

    static QString authorsFilePath();
%Docstring
Returns the path to the authors file.
%End

    static QString contributorsFilePath();
%Docstring
Returns the path to the contributors file.
Contributors are people who have submitted patches
but don't have commit access. *
%End

    static QString developersMapFilePath();
%Docstring
Returns the path to the developers map file.
The developers map was created by using leaflet framework,
it shows the doc/contributors.json file.

.. versionadded:: 2.7
%End

    static QString sponsorsFilePath();
%Docstring
Returns the path to the sponsors file.
%End

    static QString donorsFilePath();
%Docstring
Returns the path to the donors file.
%End

    static QString serverResourcesPath();
%Docstring
Returns the path to the server resources directory.
%End

    static QString translatorsFilePath();
%Docstring
Returns the path to the sponsors file.
%End

    static QString licenceFilePath();
%Docstring
Returns the path to the licence file.
%End

    static QString i18nPath();
%Docstring
Returns the path to the translation directory.
%End

    static QString metadataPath();
%Docstring
Returns the path to the metadata directory.

.. versionadded:: 3.0
%End

    static QString qgisMasterDatabaseFilePath();
%Docstring
Returns the path to the master qgis.db file.
%End

    static QString qgisSettingsDirPath();
%Docstring
Returns the path to the settings directory in user's home dir
%End

    static QString qgisUserDatabaseFilePath();
%Docstring
Returns the path to the user qgis.db file.
%End

    static QString qgisAuthDatabaseFilePath();
%Docstring
Returns the path to the user authentication database file: qgis-auth.db.
%End

    static QString splashPath();
%Docstring
Returns the path to the splash screen image directory.
%End

    static QString iconsPath();
%Docstring
Returns the path to the icons image directory.
%End

    static QString srsDatabaseFilePath();
%Docstring
Returns the path to the srs.db file.
%End

    static QStringList svgPaths();
%Docstring
Returns the paths to svg directories.
%End

    static QStringList composerTemplatePaths();
%Docstring
Returns the paths to composer template directories
%End

    static QMap<QString, QString> systemEnvVars();
%Docstring
Returns the system environment variables passed to application.
%End

    static QString prefixPath();
%Docstring
Returns the path to the application prefix directory.
%End

    static QString pluginPath();
%Docstring
Returns the path to the application plugin directory.
%End

    static QString pkgDataPath();
%Docstring
Returns the common root path of all application data directories.
%End

    static QString activeThemePath();
%Docstring
Returns the path to the currently active theme directory.
%End

    static QString defaultThemePath();
%Docstring
Returns the path to the default theme directory.
%End

    static QString iconPath( const QString &iconFile );
%Docstring
Returns path to the desired icon file.
First it tries to use the active theme path, then default theme path
%End

    static QIcon getThemeIcon( const QString &name );
%Docstring
Helper to get a theme icon. It will fall back to the
default theme if the active theme does not have the required icon.
%End

    enum Cursor
    {
      ZoomIn,
      ZoomOut,
      Identify,
      CrossHair,
      CapturePoint,
      Select,
      Sampler,
    };

    static QCursor getThemeCursor( const Cursor &cursor );
%Docstring
Helper to get a theme cursor. It will fall back to the
default theme if the active theme does not have the required icon.
Cursors are automatically scaled to look like a 16px cursor on 96dpi
screens.
%End

    static QPixmap getThemePixmap( const QString &name );
%Docstring
Helper to get a theme icon as a pixmap. It will fall back to the
default theme if the active theme does not have the required icon.
%End

    static QString userStylePath();
%Docstring
Returns the path to user's style.
%End

    static QRegExp shortNameRegExp();
%Docstring
Returns the short name regular expression for line edit validator
%End

    static QString userLoginName();
%Docstring
Returns the user's operating system login account name.

.. versionadded:: 2.14

.. seealso:: :py:func:`userFullName`
%End

    static QString userFullName();
%Docstring
Returns the user's operating system login account full display name.

.. versionadded:: 2.14

.. seealso:: :py:func:`userLoginName`
%End

    static QString osName();
%Docstring
Returns a string name of the operating system QGIS is running on.

.. versionadded:: 2.14

.. seealso:: :py:func:`platform`
%End

    static QString platform();
%Docstring
Returns the QGIS platform name, e.g., "desktop" or "server".

.. versionadded:: 2.14

.. seealso:: :py:func:`osName`
%End

    static QString locale();
%Docstring
Returns the QGIS locale.

.. versionadded:: 3.0
%End

    static QString userThemesFolder();
%Docstring
Returns the path to user's themes folder
%End

    static QString defaultStylePath();
%Docstring
Returns the path to default style (works as a starting point).
%End

    static QString defaultThemesFolder();
%Docstring
Returns the path to default themes folder from install (works as a starting point).
%End

    static QString libraryPath();
%Docstring
Returns the path containing qgis_core, qgis_gui, qgispython (and other) libraries
%End

    static QString libexecPath();
%Docstring
Returns the path with utility executables (help viewer, crssync, ...)
%End

    static void setPrefixPath( const QString &prefixPath, bool useDefaultPaths = false );
%Docstring
Alters prefix path - used by 3rd party apps
%End

    static void setPluginPath( const QString &pluginPath );
%Docstring
Alters plugin path - used by 3rd party apps
%End

    static void setPkgDataPath( const QString &pkgDataPath );
%Docstring
Alters pkg data path - used by 3rd party apps
%End

    static void setDefaultSvgPaths( const QStringList &pathList );
%Docstring
Alters default svg paths - used by 3rd party apps.
%End

    static void setAuthDatabaseDirPath( const QString &authDbDirPath );
%Docstring
Alters authentication data base directory path - used by 3rd party apps
%End

    static void initQgis();
%Docstring
loads providers
%End

    static bool createDatabase( QString *errorMessage = 0 );
%Docstring
initialize qgis.db
%End

    static bool createThemeFolder();
%Docstring
Create the users theme folder
%End

    static void exitQgis();
%Docstring
deletes provider registry and map layer registry
%End

    static QString appIconPath();
%Docstring
get application icon
%End

    enum endian_t
    {
      XDR,
      NDR
    };

    static endian_t endian();
%Docstring
Returns whether this machine uses big or little endian
%End


    static QString reportStyleSheet();
%Docstring
 get a standard css style sheet for reports.
Typically you will use this method by doing:
QString myStyle = QgsApplication.reportStyleSheet();
textBrowserReport->document()->setDefaultStyleSheet(myStyle);

:return: QString containing the CSS 2.1 compliant stylesheet.

.. note::

   you can use the special Qt extensions too, for example
the gradient fills for backgrounds.
%End

    static QString showSettings();
%Docstring
Convenience function to get a summary of the paths used in this
application instance useful for debugging mainly.*
%End

    static void registerOgrDrivers();
%Docstring
Register OGR drivers ensuring this only happens once.
This is a workaround for an issue with older gdal versions that
caused duplicate driver name entries to appear in the list
of registered drivers when QgsApplication.registerOgrDrivers was called multiple
times.
%End

    static QString absolutePathToRelativePath( const QString &apath, const QString &targetPath );
%Docstring
Converts absolute path to path relative to target
%End
    static QString relativePathToAbsolutePath( const QString &rpath, const QString &targetPath );
%Docstring
Converts path relative to target to an absolute path
%End

    static bool isRunningFromBuildDir();
%Docstring
Indicates whether running from build directory (not installed)
%End
    static QString buildSourcePath();
%Docstring
Returns path to the source directory. Valid only when running from build directory
%End
    static QString buildOutputPath();
%Docstring
Returns path to the build output directory. Valid only when running from build directory
%End

    static void skipGdalDriver( const QString &driver );
%Docstring
Sets the GDAL_SKIP environment variable to include the specified driver
and then calls GDALDriverManager.AutoSkipDrivers() to unregister it. The
driver name should be the short format of the Gdal driver name e.g. GTIFF.
%End

    static void restoreGdalDriver( const QString &driver );
%Docstring
Sets the GDAL_SKIP environment variable to exclude the specified driver
and then calls GDALDriverManager.AutoSkipDrivers() to unregister it. The
driver name should be the short format of the Gdal driver name e.g. GTIFF.
%End

    static QStringList skippedGdalDrivers();
%Docstring
Returns the list of gdal drivers that should be skipped (based on
GDAL_SKIP environment variable)
%End

    static void applyGdalSkippedDrivers();
%Docstring
Apply the skipped drivers list to gdal

.. seealso:: :py:func:`skipGdalDriver`

.. seealso:: :py:func:`restoreGdalDriver`

.. seealso:: :py:func:`skippedGdalDrivers`
%End

    static int maxThreads();
%Docstring
Get maximum concurrent thread count

.. versionadded:: 2.4
%End

    static void setMaxThreads( int maxThreads );
%Docstring
Set maximum concurrent thread count

.. note::

   must be between 1 and \#cores, -1 means use all available cores

.. versionadded:: 2.4
%End

    static QgsTaskManager *taskManager();
%Docstring
Returns the application's task manager, used for managing application
wide background task handling.

.. versionadded:: 3.0
%End

    static QgsColorSchemeRegistry *colorSchemeRegistry();
%Docstring
Returns the application's color scheme registry, used for managing color schemes.

.. versionadded:: 3.0
%End

    static QgsPaintEffectRegistry *paintEffectRegistry();
%Docstring
Returns the application's paint effect registry, used for managing paint effects.

.. versionadded:: 3.0
%End

    static QgsRendererRegistry *rendererRegistry();
%Docstring
Returns the application's renderer registry, used for managing vector layer renderers.

.. versionadded:: 3.0
%End


    static QgsDataItemProviderRegistry *dataItemProviderRegistry();
%Docstring
Returns the application's data item provider registry, which keeps a list of data item
providers that may add items to the browser tree.

.. versionadded:: 3.0
%End

    static QgsSvgCache *svgCache();
%Docstring
Returns the application's SVG cache, used for caching SVG images and handling parameter replacement
within SVG files.

.. versionadded:: 3.0
%End

    static QgsSymbolLayerRegistry *symbolLayerRegistry();
%Docstring
Returns the application's symbol layer registry, used for managing symbol layers.

.. versionadded:: 3.0
%End

    static QgsLayoutItemRegistry *layoutItemRegistry();
%Docstring
Returns the application's layout item registry, used for layout item types.

.. versionadded:: 3.0
%End

    static QgsGPSConnectionRegistry *gpsConnectionRegistry();
%Docstring
Returns the application's GPS connection registry, used for managing GPS connections.

.. versionadded:: 3.0
%End

    static QgsPluginLayerRegistry *pluginLayerRegistry();
%Docstring
Returns the application's plugin layer registry, used for managing plugin layer types.

.. versionadded:: 3.0
%End

    static QgsMessageLog *messageLog();
%Docstring
Returns the application's message log.

.. versionadded:: 3.0
%End

    static QgsAuthManager *authManager();
%Docstring
Returns the application's authentication manager instance

.. note::

   this can be a null pointer if called before initQgis

.. seealso:: :py:func:`initQgis`

.. versionadded:: 3.0
%End

    static QgsProcessingRegistry *processingRegistry();
%Docstring
Returns the application's processing registry, used for managing processing providers,
algorithms, and various parameters and outputs.

.. versionadded:: 3.0
%End

    static QgsPageSizeRegistry *pageSizeRegistry();
%Docstring
Returns the application's page size registry, used for managing layout page sizes.

.. versionadded:: 3.0
%End


    static QgsActionScopeRegistry *actionScopeRegistry();
%Docstring
Returns the action scope registry.

.. versionadded:: 3.0
%End

    static QgsRuntimeProfiler *profiler();
%Docstring
Returns the application runtime profiler.

.. versionadded:: 3.0
%End

    static QgsFieldFormatterRegistry *fieldFormatterRegistry();
%Docstring
Get the registry of available field formatters.
%End

    static Qgs3DRendererRegistry *renderer3DRegistry();
%Docstring
Returns registry of available 3D renderers.

.. note::

   not available in Python bindings

.. versionadded:: 3.0
%End

    static QString nullRepresentation();
%Docstring
This string is used to represent the value `NULL` throughout QGIS.

In general, when passing values around, prefer to use a null QVariant
`QVariant( field.type() )` or `QVariant( QVariant.Int )`. This value
should only be used in the final presentation step when showing values
in a widget or sending it to a web browser.
%End

    static void setNullRepresentation( const QString &nullRepresentation );
%Docstring
\copydoc nullRepresentation()
%End

    static QVariantMap customVariables();
%Docstring
Custom expression variables for this application.
This does not include generated variables (like system name, user name etc.)

.. seealso:: :py:func:`QgsExpressionContextUtils.globalVariables`

.. versionadded:: 3.0
%End

    static void setCustomVariables( const QVariantMap &customVariables );
%Docstring
Custom expression variables for this application.
Do not include generated variables (like system name, user name etc.)

.. seealso:: :py:func:`QgsExpressionContextUtils.globalVariables`

.. versionadded:: 3.0
%End


    static void setCustomVariable( const QString &name, const QVariant &value );
%Docstring
Set a single custom expression variable.

.. versionadded:: 3.0
%End

%If (ANDROID)
    //dummy method to workaround sip generation issue
    bool x11EventFilter( XEvent *event );
%End

  signals:

    void customVariablesChanged();
%Docstring
Emitted whenever a custom global variable changes.

.. versionadded:: 3.0
%End


    void nullRepresentationChanged();
%Docstring
\copydoc nullRepresentation()
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsapplication.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
