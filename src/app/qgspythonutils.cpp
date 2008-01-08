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
#include "qgslogger.h"

#include <QMessageBox>
#include <QStringList>
#include <QDir>

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
  runString("sys.path = [\"" + homePluginsPath()  + "\", \"" + pythonPath() + "\", \"" + pluginsPath() + "\"] + sys.path");

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
  runString(
      "def qgis_except_hook_msg(type, value, tb, msg):\n"
      "  lst = traceback.format_exception(type, value, tb)\n"
      "  if msg == None: msg = 'An error has occured while executing Python code:'\n"
      "  str = '<font color=\"red\">'+msg+'</font><br><br>'\n"
      "  for s in lst:\n"
      "    str += s\n"
      "  str = str.replace('\\n', '<br>')\n"
      "  str = str.replace('  ', '&nbsp;')\n" // preserve whitespaces for nicer output
      "  \n"
      "  msg = QgsMessageOutput.createMessageOutput()\n"
      "  msg.setTitle('Error')\n"
      "  msg.setMessage(str, QgsMessageOutput.MessageHtml)\n"
      "  msg.showMessage()\n");
  runString(
      "def qgis_except_hook(type, value, tb):\n"
      "  qgis_except_hook_msg(type, value, tb, None)\n");
  
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
  runString("sys.displayhook = console_display_hook\n");
  
  runString(
    "class MyOutputCatcher:\n"
    "  def __init__(self):\n"
    "    self.data = ''\n"
    "  def write(self, stuff):\n"
    "    self.data += stuff\n");
  runString("_old_stdout = sys.stdout\n");
  runString("sys.stdout = MyOutputCatcher()\n");
  
}

void QgsPythonUtils::uninstallConsoleHooks()
{
  runString("sys.displayhook = sys.__displayhook__");
  runString("sys.stdout = _old_stdout");
  
  // TODO: uninstalling stdout redirection doesn't work

  //installErrorHook();
}


bool QgsPythonUtils::runString(const QString& command)
{
  PyRun_String(command.toLocal8Bit().data(), Py_single_input, mMainDict, mMainDict);
  
  return (PyErr_Occurred() == 0);
}


QString QgsPythonUtils::getTypeAsString(PyObject* obj)
{
  if (obj == NULL)
    return NULL;

  if (PyClass_Check(obj))
  {
    QgsDebugMsg("got class");
    return QString(PyString_AsString(((PyClassObject*)obj)->cl_name));
  }
  else if (PyType_Check(obj))
  {
    QgsDebugMsg("got type");
	return QString(((PyTypeObject*)obj)->tp_name);
  }
  else
  {
    QgsDebugMsg("got object");
    PyObject* s = PyObject_Str(obj);
    QString str;
    if (s && PyString_Check(s))
      str = QString(PyString_AsString(s));
    Py_XDECREF(s);
    return str;
  }
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
  errorClassName = getTypeAsString(err_type);
    
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
  return getVariableFromMain("__result");
}

QString QgsPythonUtils::getVariableFromMain(QString name)
{
  PyObject* obj;
  PyObject* obj_str;
  
  QString output;
  
  // get the result
  obj = PyDict_GetItemString(mMainDict, name); // obj is borrowed reference
  
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
  PyDict_SetItemString(mMainDict, name, Py_None);
  
  return output;
}

bool QgsPythonUtils::evalString(const QString& command, QString& result)
{
  PyObject* res = PyRun_String(command.toLocal8Bit().data(), Py_eval_input, mMainDict, mMainDict);
  
  if (res != NULL && PyString_Check(res))
  {
    result = PyString_AsString(res);
    Py_XDECREF(res);
    return true;
  }
  Py_XDECREF(res);
  return false;
}


QString QgsPythonUtils::pythonPath()
{
  return QgsApplication::pkgDataPath() + "/python";
}

QString QgsPythonUtils::pluginsPath()
{
  return pythonPath() + "/plugins";
}

QString QgsPythonUtils::homePluginsPath()
{
  return QgsApplication::qgisSettingsDirPath() + "/python/plugins";
}

QStringList QgsPythonUtils::pluginList()
{
  QDir pluginDir(QgsPythonUtils::pluginsPath(), "*",
                 QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoDotAndDotDot);

  QDir homePluginDir(QgsPythonUtils::homePluginsPath(), "*",
		     QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoDotAndDotDot);

  QStringList pluginList = pluginDir.entryList();

  for (uint i = 0; i < homePluginDir.count(); i++)
  {
    QString packageName = homePluginDir[i];
    if(!pluginList.contains(packageName))
        pluginList.append(packageName); 

  }

  return pluginList;
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
  runString(
       "try:\n"
       "  import " + packageName + "\n"
       "  reload(" + packageName + ")\n"
       "  __main__.__plugin_result = 'OK'\n"
       "except:\n"
       "  qgis_except_hook_msg(sys.exc_type, sys.exc_value, sys.exc_traceback, "
       "                       'Couldn\\'t load plugin \"" + packageName + "\"')\n"
       "  __main__.__plugin_result = 'ERROR'\n");
  
  return (getVariableFromMain("__plugin_result") == "OK");
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
