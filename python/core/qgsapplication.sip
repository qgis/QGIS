%Feature ANDROID

class QgsApplication : QApplication
{
%TypeHeaderCode
#include <qgsapplication.h>
%End

%TypeCode
// Convert a Python argv list to a conventional C argc count and argv array.
static char **qtgui_ArgvToC(PyObject *argvlist, int &argc)
{
    char **argv;

    argc = PyList_GET_SIZE(argvlist);

    // Allocate space for two copies of the argument pointers, plus the
    // terminating NULL.
    if ((argv = (char **)sipMalloc(2 * (argc + 1) * sizeof (char *))) == NULL)
        return NULL;

    // Convert the list.
    for (int a = 0; a < argc; ++a)
    {
        char *arg;

        // Get the argument and allocate memory for it.
        if ((arg = PyString_AsString(PyList_GET_ITEM(argvlist, a))) == NULL ||
            (argv[a] = (char *)sipMalloc(strlen(arg) + 1)) == NULL)
            return NULL;

        // Copy the argument and save a pointer to it.
        strcpy(argv[a], arg);
        argv[a + argc + 1] = argv[a];
    }

    argv[argc + argc + 1] = argv[argc] = NULL;

    return argv;
}


// Remove arguments from the Python argv list that have been removed from the
// C argv array.
static void qtgui_UpdatePyArgv(PyObject *argvlist, int argc, char **argv)
{
    for (int a = 0, na = 0; a < argc; ++a)
    {
        // See if it was removed.
        if (argv[na] == argv[a + argc + 1])
            ++na;
        else
            PyList_SetSlice(argvlist, na, na + 1, NULL);
    }
}
%End



  public:
    //QgsApplication(int argc, char ** argv, bool GUIenabled);
    QgsApplication(SIP_PYLIST argv, bool GUIenabled, QString customConfigPath = QString() ) /PostHook=__pyQtQAppHook__/ [(int &argc, char **argv, bool GUIenabled, QString customConfigPath = QString() )];
%MethodCode
        // The Python interface is a list of argument strings that is modified.

        int argc;
        char **argv;

        // Convert the list.
        if ((argv = qtgui_ArgvToC(a0, argc)) == NULL)
            sipIsErr = 1;
        else
        {
            // Create it now the arguments are right.
            static int nargc = argc;

            sipCpp = new sipQgsApplication(nargc, argv, a1, *a2);

            // Now modify the original list.
            qtgui_UpdatePyArgv(a0, argc, argv);
        }
%End

    virtual ~QgsApplication();

    /** Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use themeName() as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     * @note A basic test will be carried out to ensure the theme search path
     * based on the supplied theme name exists. If it does not the theme name will
     * be reverted to 'default'.
     */
    static void setThemeName( const QString &theThemeName );

    /** Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use this method as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     */
    static const QString themeName();

    //! Returns the path to the authors file.
    static const QString authorsFilePath();

    /** Returns the path to the contributors file.
     * Contributors are people who have submitted patches
     * but don't have commit access. */
    static const QString contributorsFilePath();

    /** Returns the path to the developers map file.
     * The developers map was created by using leaflet framework,
     * it shows the doc/contributors.json file.
     * @note this function was added in version 2.7 */
    static const QString developersMapFilePath();

    /** Returns the path to the sponsors file. */
    static const QString sponsorsFilePath();

    /** Returns the path to the donors file. */
    static const QString donorsFilePath();

    /**
     * Returns the path to the sponsors file.
     */
    static const QString translatorsFilePath();

    /*!
      Returns the path to the licence file.
    */
    static const QString licenceFilePath();

    //! Returns the path to the help application.
    static const QString helpAppPath();

    //! Returns the path to the translation directory.
    static const QString i18nPath();

    //! Returns the path to the master qgis.db file.
    static const QString qgisMasterDbFilePath();

    //! Returns the path to the settings directory in user's home dir
    static const QString qgisSettingsDirPath();

    //! Returns the path to the user qgis.db file.
    static const QString qgisUserDbFilePath();

    //! Returns the path to the splash screen image directory.
    static const QString splashPath();

    //! Returns the path to the icons image directory.
    static const QString iconsPath();

    //! Returns the path to the srs.db file.
    static const QString srsDbFilePath();

    //! Returns the pathes to svg directories.
    static const QStringList svgPaths();

    //! Returns the system environment variables passed to application.
    static const QMap<QString, QString> systemEnvVars();

    //! Returns the path to the application prefix directory.
    static const QString prefixPath();

    //! Returns the path to the application plugin directory.
    static const QString pluginPath();

    //! Returns the common root path of all application data directories.
    static const QString pkgDataPath();

    //! Returns the path to the currently active theme directory.
    static const QString activeThemePath();

    //! Returns the path to the default theme directory.
    static const QString defaultThemePath();

    //! Returns path to the desired icon file.
    //! First it tries to use the active theme path, then default theme path
    static QString iconPath( QString iconFile );

    //! Helper to get a theme icon. It will fall back to the
    //! default theme if the active theme does not have the required icon.
    static QIcon getThemeIcon( const QString &theName );

    //! Helper to get a theme icon as a pixmap. It will fall back to the
    //! default theme if the active theme does not have the required icon.
    static QPixmap getThemePixmap( const QString &theName );

    //! Returns the path to user's style.
    static const QString userStyleV2Path();

    //! Returns the path to default style (works as a starting point).
    static const QString defaultStyleV2Path();

    //! Returns the path containing qgis_core, qgis_gui, qgispython (and other) libraries
    static const QString libraryPath();

    //! Returns the path with utility executables (help viewer, crssync, ...)
    static const QString libexecPath();

    //! Alters prefix path - used by 3rd party apps
    static void setPrefixPath( const QString &thePrefixPath, bool useDefaultPaths = false );

    //! Alters plugin path - used by 3rd party apps
    static void setPluginPath( const QString &thePluginPath );

    //! Alters pkg data path - used by 3rd party apps
    static void setPkgDataPath( const QString &thePkgDataPath );

    //! Alters default svg paths - used by 3rd party apps.
    static void setDefaultSvgPaths( const QStringList& pathList );

    //! loads providers
    static void initQgis();

    //! initialise qgis.db
    static bool createDB( QString* errorMessage = 0 );

    //! deletes provider registry and map layer registry
    static void exitQgis();

    /** constants for endian-ness */
    enum endian_t
    {
      XDR, // network, or big-endian, byte order
      NDR, // little-endian byte order
    };

    //! Returns whether this machine uses big or little endian
    static endian_t endian();

    /** \brief get a standard css style sheet for reports.
     * Typically you will use this method by doing:
     * QString myStyle = QgsApplication::reportStyleSheet();
     * textBrowserReport->document()->setDefaultStyleSheet(myStyle);
     * @return QString containing the CSS 2.1 compliant stylesheet.
     * @note you can use the special Qt extensions too, for example
     * the gradient fills for backgrounds.
     */
    static QString reportStyleSheet();

    /** Convenience function to get a summary of the paths used in this
     * application instance useful for debugging mainly.*/
    static QString showSettings();

    /** Register OGR drivers ensuring this only happens once.
     * This is a workaround for an issue with older gdal versions that
     * caused duplicate driver name entries to appear in the list
     * of registered drivers when QgsApplication::registerOgrDrivers was called multiple
     * times.
     */
    static void registerOgrDrivers();

    /**Converts absolute path to path relative to target */
    static QString absolutePathToRelativePath( QString apath, QString targetPath );
    /**Converts path relative to target to an absolute path */
    static QString relativePathToAbsolutePath( QString rpath, QString targetPath );

    /** Indicates whether running from build directory (not installed) */
    static bool isRunningFromBuildDir();
    /** Returns path to the source directory. Valid only when running from build directory */
    static QString buildSourcePath();
    /** Returns path to the build output directory. Valid only when running from build directory */
    static QString buildOutputPath();

    /** Sets the GDAL_SKIP environment variable to include the specified driver
     * and then calls GDALDriverManager::AutoSkipDrivers() to unregister it. The
     * driver name should be the short format of the Gdal driver name e.g. GTIFF.
     */
    static void skipGdalDriver( QString theDriver );

    /** Sets the GDAL_SKIP environment variable to exclude the specified driver
     * and then calls GDALDriverManager::AutoSkipDrivers() to unregister it. The
     * driver name should be the short format of the Gdal driver name e.g. GTIFF.
     */
    static void restoreGdalDriver( QString theDriver );

    /** Returns the list of gdal drivers that should be skipped (based on
     * GDAL_SKIP environment variable)
     */
    static QStringList skippedGdalDrivers();

    /** Apply the skipped drivers list to gdal
     * @see skipGdalDriver
     * @see restoreGdalDriver
     * @see skippedGdalDrivers */
    static void applyGdalSkippedDrivers();

    /** Get maximum concurrent thread count
     * @note added in 2.4 */
    static int maxThreads();
    /** Set maximum concurrent thread count
     * @note must be between 1 and \#cores, -1 means use all available cores
     * @note added in 2.4 */
    static void setMaxThreads( int maxThreads );

%If(ANDROID)
    //dummy method to workaround sip generation issue issue
    bool x11EventFilter ( XEvent * event );
%End

  signals:
    //! @note not available in python bindings
    // void preNotify( QObject * receiver, QEvent * event, bool * done );
};
