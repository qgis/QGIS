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
#if PYTHON2
        if ((arg = PyString_AsString(PyList_GET_ITEM(argvlist, a))) == NULL ||
            (argv[a] = (char *)sipMalloc(strlen(arg) + 1)) == NULL)
            return NULL;
#else
        if ((arg = PyBytes_AsString(PyList_GET_ITEM(argvlist, a))) == NULL ||
            (argv[a] = (char *)sipMalloc(strlen(arg) + 1)) == NULL)
            return NULL;
#endif
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
    static const char* QGIS_ORGANIZATION_NAME;
    static const char* QGIS_ORGANIZATION_DOMAIN;
    static const char* QGIS_APPLICATION_NAME;

    //QgsApplication(int argc, char ** argv, bool GUIenabled);
    QgsApplication(SIP_PYLIST argv, bool GUIenabled, QString customConfigPath = QString() ) /PostHook=__pyQtQAppHook__/ [(int &argc, char **argv, bool GUIenabled, const QString& customConfigPath = QString() )];
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

    //! Set the FileOpen event receiver
    static void setFileOpenEventReceiver( QObject * receiver );

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
    static QString themeName();

    /**
     * @brief Set the current UI theme used to style the interface.  Use uiThemes() to
     * find valid themes to use. Variabels found in variables.qss will be added to the stylesheet
     * on load.
     * @param themeName The name of the theme.
     * @note using an invalid theme name will reset to default
     */
    static void setUITheme( const QString &themeName );

    /**
     * @brief All themes found in ~/.qgis2/themes folder.
     * The path is to the root folder for the theme
     * @note Valid theme folders must contain a style.qss file.
     * @return A hash of theme name and theme path. Valid theme folders contain style.qss
     */
    static QHash<QString, QString> uiThemes();

    //! Returns the path to the authors file.
    static QString authorsFilePath();

    /** Returns the path to the contributors file.
     * Contributors are people who have submitted patches
     * but don't have commit access. */
    static QString contributorsFilePath();

    /** Returns the path to the developers map file.
     * The developers map was created by using leaflet framework,
     * it shows the doc/contributors.json file.
     * @note this function was added in version 2.7 */
    static QString developersMapFilePath();

    /** Returns the path to the sponsors file. */
    static QString sponsorsFilePath();

    /** Returns the path to the donors file. */
    static QString donorsFilePath();

    /**
     * Returns the path to the sponsors file.
     */
    static QString translatorsFilePath();

    /*!
      Returns the path to the licence file.
     */
    static QString licenceFilePath();

    //! Returns the path to the help application.
    static QString helpAppPath();

    //! Returns the path to the translation directory.
    static QString i18nPath();

    //! Returns the path to the master qgis.db file.
    static QString qgisMasterDbFilePath();

    //! Returns the path to the settings directory in user's home dir
    static QString qgisSettingsDirPath();

    //! Returns the path to the user qgis.db file.
    static QString qgisUserDbFilePath();

    //! Returns the path to the user authentication database file: qgis-auth.db.
    static QString qgisAuthDbFilePath();

    //! Returns the path to the splash screen image directory.
    static QString splashPath();

    //! Returns the path to the icons image directory.
    static QString iconsPath();

    //! Returns the path to the srs.db file.
    static QString srsDbFilePath();

    //! Returns the pathes to svg directories.
    static QStringList svgPaths();

    //! Returns the pathes to composer template directories
    static QStringList composerTemplatePaths();

    //! Returns the system environment variables passed to application.
    static QMap<QString, QString> systemEnvVars();

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

    //! Returns path to the desired icon file.
    //! First it tries to use the active theme path, then default theme path
    static QString iconPath( const QString& iconFile );

    //! Helper to get a theme icon. It will fall back to the
    //! default theme if the active theme does not have the required icon.
    static QIcon getThemeIcon( const QString &theName );

    //! Helper to get a theme icon as a pixmap. It will fall back to the
    //! default theme if the active theme does not have the required icon.
    static QPixmap getThemePixmap( const QString &theName );

    //! Returns the path to user's style.
    static QString userStyleV2Path();

    //! Returns the short name regular expression for line edit validator
    static QRegExp shortNameRegExp();

    /** Returns the user's operating system login account name.
     * @note added in QGIS 2.14
     * @see userFullName()
     */
    static QString userLoginName();

    /** Returns the user's operating system login account full display name.
     * @note added in QGIS 2.14
     * @see userLoginName()
     */
    static QString userFullName();

    /** Returns a string name of the operating system QGIS is running on.
     * @note added in QGIS 2.14
     * @see platform()
     */
    static QString osName();

    /** Returns the QGIS platform name, eg "desktop" or "server".
     * @note added in QGIS 2.14
     * @see osName()
     */
    static QString platform();

    //! Returns the path to user's themes folder
    static QString userThemesFolder();

    //! Returns the path to default style (works as a starting point).
    static QString defaultStyleV2Path();

    //! Returns the path to default themes folder from install (works as a starting point).
    static QString defaultThemesFolder();

    //! Returns the path containing qgis_core, qgis_gui, qgispython (and other) libraries
    static QString libraryPath();

    //! Returns the path with utility executables (help viewer, crssync, ...)
    static QString libexecPath();

    //! Alters prefix path - used by 3rd party apps
    static void setPrefixPath( const QString &thePrefixPath, bool useDefaultPaths = false );

    //! Alters plugin path - used by 3rd party apps
    static void setPluginPath( const QString &thePluginPath );

    //! Alters pkg data path - used by 3rd party apps
    static void setPkgDataPath( const QString &thePkgDataPath );

    //! Alters default svg paths - used by 3rd party apps.
    static void setDefaultSvgPaths( const QStringList& pathList );

    //! Alters authentication data base directory path - used by 3rd party apps
    static void setAuthDbDirPath( const QString& theAuthDbDirPath );

    //! loads providers
    static void initQgis();

    //! initialise qgis.db
    static bool createDB( QString* errorMessage = 0 );

    //! Create the users theme folder
    static bool createThemeFolder();

    //! deletes provider registry and map layer registry
    static void exitQgis();

    //! get application icon
    static QString appIconPath();

    /** Constants for endian-ness */
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

    /** Converts absolute path to path relative to target */
    static QString absolutePathToRelativePath( const QString& apath, const QString& targetPath );
    /** Converts path relative to target to an absolute path */
    static QString relativePathToAbsolutePath( const QString& rpath, const QString& targetPath );

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
    static void skipGdalDriver( const QString& theDriver );

    /** Sets the GDAL_SKIP environment variable to exclude the specified driver
     * and then calls GDALDriverManager::AutoSkipDrivers() to unregister it. The
     * driver name should be the short format of the Gdal driver name e.g. GTIFF.
     */
    static void restoreGdalDriver( const QString& theDriver );

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
