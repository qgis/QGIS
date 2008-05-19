
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
    QgsApplication(SIP_PYLIST argv, bool GUIenabled) /PostHook=__pyQtQAppHook__/ [(int &argc, char **argv, bool GUIenabled)];
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
        
            sipCpp = new sipQgsApplication(nargc, argv, a1);
        
            // Now modify the original list.
            qtgui_UpdatePyArgv(a0, argc, argv);
        }
%End

    virtual ~QgsApplication();

    //! Set the theme path to the specified theme.
    static void selectTheme(const QString theThemeName);

    //! Returns the path to the authors file.
    static const QString authorsFilePath();

    //! Returns the path to the developer image directory.
    static const QString developerPath();

    //! Returns the path to the help application.
    static const QString helpAppPath();

    //! Returns the path to the mapserver export application.
    static const QString msexportAppPath();

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

    //! Returns the path to the svg directory.
    static const QString svgPath();

    //! Returns the path to the application prefix directory.
    static const QString prefixPath();

    //! Returns the path to the application plugin directory.
    static const QString pluginPath();

    //! Returns the common root path of all application data directories.
    static const QString pkgDataPath();

    //! Returns the path to the current theme directory.
    static const QString themePath();
    
    //! Alters prefix path - used by 3rd party apps
    static void setPrefixPath(const QString& thePrefixPath, bool useDefaultPaths = FALSE);
    
    //! Alters plugin path - used by 3rd party apps
    static void setPluginPath(const QString thePluginPath);

    //! Alters pkg data path - used by 3rd party apps
    static void setPkgDataPath(const QString& thePkgDataPath);
    
    //! loads providers
    static void initQgis();

    //! deletes provider registry and map layer registry
    static void exitQgis();
    
    /** constants for endian-ness */
    enum endian_t
    {
      XDR = 0,  // network, or big-endian, byte order
      NDR = 1   // little-endian byte order
    };
    
    //! Returns whether this machine uses big or little endian
    static endian_t endian();

};

