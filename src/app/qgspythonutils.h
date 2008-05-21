/***************************************************************************
    qgspythonutils.h - routines for interfacing Python
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include <QString>
#include <QStringList>

// forward declaration for PyObject
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

class QgisInterface;

/**
 All calls to Python functions in QGIS come here.
 
 All functions here are static - it's not needed to create an instance of this class

 For now, default path for python plugins is QgsApplication::pluginPath() + "/python"

 */
class QgsPythonUtils
{
  public:
    
    /* general purpose functions */

    //! initialize python and import bindings
    static void initPython(QgisInterface* interface);
    
    //! close python interpreter
    static void exitPython();

    //! returns true if python support is ready to use (must be inited first)
    static bool isEnabled();
    
    //! returns path where QGIS python stuff is located
    static QString pythonPath();
    
    //! run a statement (wrapper for PyRun_String)
    //! this command is more advanced as enables error checking etc.
    //! when an exception is raised, it shows dialog with exception details
    //! @return true if no error occured
    static bool runString(const QString& command, QString msgOnError = QString());
    
    //! run a statement, error reporting is not done
    //! @return true if no error occured
    static bool runStringUnsafe(const QString& command);
    
    static bool evalString(const QString& command, QString& result);
    
    //! @return object's type name as a string
    static QString getTypeAsString(PyObject* obj);

    //! get information about error to the supplied arguments
    //! @return false if there was no python error
    static bool getError(QString& errorClassName, QString& errorText);
    
    //! get variable from main dictionary
    static QString getVariableFromMain(QString name);

    /* python console related functions */
    
    //! change displayhook and excepthook
    //! our hooks will just save the result to special variables
    //! and those can be used in the program
    static void installConsoleHooks();
    
    //! get back to the original settings (i.e. write output to stdout)
    static void uninstallConsoleHooks();
    
    //! get result from the last statement as a string
    static QString getResult();

    /* plugins related functions */
    
    //! return current path for python plugins
    static QString pluginsPath();

    //! return current path for home directory python plugins
    static QString homePluginsPath();

    //! return list of all available python plugins
    static QStringList pluginList();
        
    //! load python plugin (import)
    static bool loadPlugin(QString packageName);
    
    //! start plugin: add to active plugins and call initGui()
    static bool startPlugin(QString packageName);
    
    //! helper function to get some information about plugin
    //! @param function one of these strings: name, tpye, version, description
    static QString getPluginMetadata(QString pluginName, QString function);

    //! unload plugin
    static bool unloadPlugin(QString packageName);

  protected:
    
    static void installErrorHook();

    
    //! path where 
    static QString mPluginsPath;
    
    //! reference to module __main__
    static PyObject* mMainModule;
    
    //! dictionary of module __main__
    static PyObject* mMainDict;
    
    //! flag determining that python support is enabled
    static bool mPythonEnabled;
};
