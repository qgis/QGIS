/***************************************************************************
    qgspythonutils.cpp - routines for interfacing Python
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

// python should be first include
// otherwise issues some warnings
#include <Python.h>

#include "qgspythonutils.h"

#include "qgsapplication.h"


#include <QMessageBox>

QString QgsPythonUtils::mPluginsPath;
PyObject* QgsPythonUtils::mMainModule;
PyObject* QgsPythonUtils::mMainDict;
bool QgsPythonUtils::mPythonEnabled = false;


void QgsPythonUtils::initPython(QgisInterface* interface)
{
  // initialize python
  Py_Initialize();
  
  mPythonEnabled = true;
  
  mMainModule = PyImport_AddModule("__main__"); // borrowed reference
  mMainDict = PyModule_GetDict(mMainModule); // borrowed reference
  
  // import sys module
  runString("import sys");
  
  // expect that bindings are installed locally, so add the path to modules
  // also add path to plugins
  runString("sys.path = [\"" + pythonPath() + "\", \"" + pluginsPath() + "\"] + sys.path");

  // import SIP
  if (!runString("from sip import wrapinstance, unwrapinstance"))
  {
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load SIP module.\nPython support will be disabled."));
    PyErr_Clear();
    exitPython();
    return;
  }
  
  // import Qt bindings
  if (!runString("from PyQt4 import QtCore, QtGui"))
  {
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load PyQt bindings.\nPython support will be disabled."));
    PyErr_Clear();
    exitPython();
    return;
  }
  
  // import QGIS bindings
  if (!runString("from qgis.core import *") ||
      !runString("from qgis.gui import *"))
  {
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load QGIS bindings.\nPython support will be disabled."));
    PyErr_Clear();
    exitPython();
    return;
  }
  
  runString("import __main__");
  runString("import traceback"); // for formatting stack traces
  
  // hook that will show information and traceback in message box
  // TODO: maybe QgsMessageOutput / QgsMessageViewer should be used instead
  runString(
      "def qgis_except_hook(type, value, tb):\n"
      "  lst = traceback.format_exception(type, value, tb)\n"
      "  str = 'An error has occured while executing Python code:\\n'\n"
      "  for s in lst:\n"
      "    str += s\n"
      "  QtGui.QMessageBox.warning(None, 'Python error', str)\n");

  // hook for python console so all output will be redirected
  // and then shown in console
  runString(
      "def console_display_hook(obj):\n"
      "  __main__.__result = obj\n");
  
  installErrorHook();
  
  // initialize 'iface' object
  runString("iface = wrapinstance(" + QString::number((unsigned long) interface) + ", QgisInterface)");
  runString("plugins = {}");

}

void QgsPythonUtils::exitPython()
{  
  Py_Finalize();
  mMainModule = NULL;
  mMainDict = NULL;
  mPythonEnabled = false;
}


bool QgsPythonUtils::isEnabled()
{
  return mPythonEnabled;
}

void QgsPythonUtils::installErrorHook()
{
  runString("sys.excepthook = qgis_except_hook");
}

void QgsPythonUtils::installConsoleHooks()
{
  runString("sys.displayhook = console_display_hook");
}

void QgsPythonUtils::uninstallConsoleHooks()
{
  runString("sys.displayhook = sys.__displayhook__");
  //installErrorHook();
}


bool QgsPythonUtils::runString(QString command)
{
  PyRun_String(command.toLocal8Bit().data(), Py_single_input, mMainDict, mMainDict);
  
  return (PyErr_Occurred() == 0);
}


bool QgsPythonUtils::getError(QString& errorClassName, QString& errorText)
{
  if (!PyErr_Occurred())
    return false;
  
  PyObject* obj_str;
  PyObject* err_type;
  PyObject* err_value;
  PyObject* err_tb;
  
  // get the exception information
  PyErr_Fetch(&err_type, &err_value, &err_tb);
    
  // get exception's class name
  errorClassName = PyString_AS_STRING(((PyClassObject*)err_type)->cl_name);
    
  // get exception's text
  if (err_value != NULL && err_value != Py_None)
  {
    obj_str = PyObject_Str(err_value); // new reference
    errorText = PyString_AS_STRING(obj_str);
    Py_XDECREF(obj_str);
  }
  else
    errorText.clear();
  
  // clear exception
  PyErr_Clear();
  
  return true;
}

QString QgsPythonUtils::getResult()
{
  PyObject* obj;
  PyObject* obj_str;
  
  QString output;
  
  // get the result
  obj = PyDict_GetItemString(mMainDict, "__result"); // obj is borrowed reference
    
  if (obj != NULL && obj != Py_None)
  {
    obj_str = PyObject_Str(obj); // obj_str is new reference
    if (obj_str != NULL && obj_str != Py_None)
    {
      output = PyString_AsString(obj_str);
    }
    Py_XDECREF(obj_str);
  }
    
  // erase result
  PyDict_SetItemString(mMainDict, "__result", Py_None);

  return output;
}


QString QgsPythonUtils::pythonPath()
{
  return QgsApplication::pkgDataPath() + "/python";
}

QString QgsPythonUtils::pluginsPath()
{
  return pythonPath() + "/plugins";
}

QString QgsPythonUtils::getPluginMetadata(QString pluginName, QString function)
{
  QString command = pluginName + "." + function + "()";
  QString retval = "???";
  
  PyObject* obj = PyRun_String(command.toLocal8Bit().data(), Py_eval_input, mMainDict, mMainDict);
  if (PyErr_Occurred())
  {
    PyErr_Print(); // just print it to console
    PyErr_Clear();
    return "__error__";
  }
  else if (PyString_Check(obj))
  {
    retval = PyString_AS_STRING(obj);
  }
  else
  {
    // bad python return value
    retval = "__error__";
  }
  Py_XDECREF(obj);
  return retval;
}


bool QgsPythonUtils::loadPlugin(QString packageName)
{
  // load plugin's package and ensure that plugin is reloaded when changed
  if (!runString("import " + packageName) ||
      !runString("reload(" + packageName + ")"))
  {
    QString className, errorText;
    getError(className, errorText);
    QString str = className + ": " + errorText;
    
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load plugin due this error:\n") + str);
    return false;
  }

  return true;
}


bool QgsPythonUtils::startPlugin(QString packageName)
{
  QString pluginPythonVar = "plugins['" + packageName + "']";
  
  // create an instance of the plugin
  if (!runString(pluginPythonVar + " = " + packageName + ".classFactory(iface)"))
  {
    PyErr_Print(); // just print to console
    PyErr_Clear();
    
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load plugin ") + packageName +
                         QObject::tr(" due an error when calling its classFactory() method"));
    return false;
  }
  
  // initGui
  if (!runString(pluginPythonVar + ".initGui()"))
  {
    PyErr_Print(); // just print to console
    PyErr_Clear();
    
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Couldn't load plugin ") + packageName +
                         QObject::tr(" due an error when calling its initGui() method"));
    return false;
  }
  
  return true;
}


bool QgsPythonUtils::unloadPlugin(QString packageName)
{
  // unload and delete plugin!
  QString varName = "plugins['" + packageName + "']";
  
  if (!runString(varName + ".unload()") ||
      !runString("del " + varName))
  {
    PyErr_Print(); // just print to console
    PyErr_Clear();
    
    QMessageBox::warning(0, QObject::tr("Python error"),
                         QObject::tr("Error while unloading plugin ") + packageName);
    return false;
  }
  
  return true;
}
