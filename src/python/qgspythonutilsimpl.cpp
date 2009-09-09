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
#ifdef _MSC_VER
#ifdef _DEBUG
#undef _DEBUG
#endif
#endif
#include <Python.h>

#include "qgspythonutilsimpl.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"

#include <QMessageBox>
#include <QStringList>
#include <QDir>


QgsPythonUtilsImpl::QgsPythonUtilsImpl()
{
  mMainModule = NULL;
  mMainDict = NULL;
  mPythonEnabled = false;
}

QgsPythonUtilsImpl::~QgsPythonUtilsImpl()
{
}

void QgsPythonUtilsImpl::initPython( QgisInterface* interface )
{
  // initialize python
  Py_Initialize();

  mPythonEnabled = true;

  mMainModule = PyImport_AddModule( "__main__" ); // borrowed reference
  mMainDict = PyModule_GetDict( mMainModule ); // borrowed reference

  runString( "import sys" ); // import sys module (for display / exception hooks)

  // expect that bindings are installed locally, so add the path to modules
  // also add path to plugins
  runString( "sys.path = [\"" + pythonPath() + "\", \"" + homePluginsPath()  + "\", \"" + pluginsPath() + "\"] + sys.path" );

  runString( "import traceback" ); // for formatting stack traces
  runString( "import __main__" ); // to access explicitly global variables

  // import SIP
  if ( !runString( "from sip import wrapinstance, unwrapinstance",
                   QObject::tr( "Couldn't load SIP module." ) + "\n" + QObject::tr( "Python support will be disabled." ) ) )
  {
    exitPython();
    return;
  }

  // import Qt bindings
  if ( !runString( "from PyQt4 import QtCore, QtGui",
                   QObject::tr( "Couldn't load PyQt4." ) + "\n" + QObject::tr( "Python support will be disabled." ) ) )
  {
    exitPython();
    return;
  }

  // import QGIS bindings
  QString error_msg = QObject::tr( "Couldn't load PyQGIS." ) + "\n" + QObject::tr( "Python support will be disabled." );
  if ( !runString( "from qgis.core import *", error_msg ) || !runString( "from qgis.gui import *", error_msg ) )
  {
    exitPython();
    return;
  }

  // hook that will show information and traceback in message box
  runString(
    "def qgis_except_hook_msg(type, value, tb, msg):\n"
    "  lst = traceback.format_exception(type, value, tb)\n"
    "  if msg == None: msg = '" + QObject::tr( "An error has occured while executing Python code:" ).replace( "'", "\\'" ) + "'\n"
    "  txt = '<font color=\"red\">'+msg+'</font><br><br>'\n"
    "  for s in lst:\n"
    "    txt += s\n"
    "  txt += '<br>" + QObject::tr( "Python version:" ).replace( "'", "\\'" ) + "<br>' + sys.version + '<br><br>'\n"
    "  txt += '" + QObject::tr( "Python path:" ).replace( "'", "\\'" ) + "' + str(sys.path)\n"
    "  txt = txt.replace('\\n', '<br>')\n"
    "  txt = txt.replace('  ', '&nbsp; ')\n" // preserve whitespaces for nicer output
    "  \n"
    "  msg = QgsMessageOutput.createMessageOutput()\n"
    "  msg.setTitle('" + QObject::tr( "Python error" ).replace( "'", "\\'" ) + "')\n"
    "  msg.setMessage(txt, QgsMessageOutput.MessageHtml)\n"
    "  msg.showMessage()\n" );
  runString(
    "def qgis_except_hook(type, value, tb):\n"
    "  qgis_except_hook_msg(type, value, tb, None)\n" );
  runString(
    "class QgisOutputCatcher:\n"
    "  def __init__(self):\n"
    "    self.data = ''\n"
    "  def write(self, stuff):\n"
    "    self.data += stuff\n"
    "  def get_and_clean_data(self):\n"
    "    tmp = self.data\n"
    "    self.data = ''\n"
    "    return tmp\n" );

  // hook for python console so all output will be redirected
  // and then shown in console
  runString(
    "def console_display_hook(obj):\n"
    "  __main__.__result = obj\n" );

  installErrorHook();

  // initialize 'iface' object
  runString( "iface = wrapinstance(" + QString::number(( unsigned long ) interface ) + ", QgisInterface)" );
  runString( "plugins = {}" );

}

void QgsPythonUtilsImpl::exitPython()
{
  Py_Finalize();
  mMainModule = NULL;
  mMainDict = NULL;
  mPythonEnabled = false;
}


bool QgsPythonUtilsImpl::isEnabled()
{
  return mPythonEnabled;
}

void QgsPythonUtilsImpl::installErrorHook()
{
  runString( "sys.excepthook = qgis_except_hook" );
}

void QgsPythonUtilsImpl::uninstallErrorHook()
{
  runString( "sys.excepthook = sys.__excepthook__" );
}

void QgsPythonUtilsImpl::installConsoleHooks()
{
  runString( "sys.displayhook = console_display_hook\n" );

  runString( "_old_stdout = sys.stdout\n" );
  runString( "sys.stdout = QgisOutputCatcher()\n" );
}

void QgsPythonUtilsImpl::uninstallConsoleHooks()
{
  runString( "sys.displayhook = sys.__displayhook__" );
  runString( "sys.stdout = _old_stdout" );
}


bool QgsPythonUtilsImpl::runStringUnsafe( const QString& command, bool single )
{
  // TODO: convert special characters from unicode strings u"..." to \uXXXX
  // so that they're not mangled to utf-8
  // (non-unicode strings can be mangled)
  PyRun_String( command.toUtf8().data(), single ? Py_single_input : Py_file_input, mMainDict, mMainDict );
  return ( PyErr_Occurred() == 0 );
}

bool QgsPythonUtilsImpl::runString( const QString& command, QString msgOnError )
{
  bool res = runStringUnsafe( command );
  if ( res )
    return true;

  if ( msgOnError.isEmpty() )
  {
    // use some default message if custom hasn't been specified
    msgOnError = QObject::tr( "An error occured during execution of following code:" ) + "\n<tt>" + command + "</tt>";
  }

  QString traceback = getTraceback();
  QString path, version;
  evalString( "str(sys.path)", path );
  evalString( "sys.version", version );

  QString str = "<font color=\"red\">" + msgOnError + "</font><br><br>" + traceback + "<br>" +
                QObject::tr( "Python version:" ) + "<br>" + version + "<br><br>" +
                QObject::tr( "Python path:" ) + "<br>" + path;
  str.replace( "\n", "<br>" ).replace( "  ", "&nbsp; " );

  QgsMessageOutput* msg = QgsMessageOutput::createMessageOutput();
  msg->setTitle( QObject::tr( "Python error" ) );
  msg->setMessage( str, QgsMessageOutput::MessageHtml );
  msg->showMessage();

  return res;
}


QString QgsPythonUtilsImpl::getTraceback()
{
#define TRACEBACK_FETCH_ERROR(what) {errMsg = what; goto done;}


  QString errMsg;
  QString result;

  PyObject *modStringIO = NULL;
  PyObject *modTB = NULL;
  PyObject *obStringIO = NULL;
  PyObject *obResult = NULL;

  PyObject *type, *value, *traceback;

  PyErr_Fetch( &type, &value, &traceback );
  PyErr_NormalizeException( &type, &value, &traceback );

  modStringIO = PyImport_ImportModule( "cStringIO" );
  if ( modStringIO == NULL )
    TRACEBACK_FETCH_ERROR( "can't import cStringIO" );

  obStringIO = PyObject_CallMethod( modStringIO, ( char* ) "StringIO", NULL );

  /* Construct a cStringIO object */
  if ( obStringIO == NULL )
    TRACEBACK_FETCH_ERROR( "cStringIO.StringIO() failed" );

  modTB = PyImport_ImportModule( "traceback" );
  if ( modTB == NULL )
    TRACEBACK_FETCH_ERROR( "can't import traceback" );

  obResult = PyObject_CallMethod( modTB, ( char* ) "print_exception",
                                  ( char* ) "OOOOO",
                                  type, value ? value : Py_None,
                                  traceback ? traceback : Py_None,
                                  Py_None,
                                  obStringIO );

  if ( obResult == NULL )
    TRACEBACK_FETCH_ERROR( "traceback.print_exception() failed" );
  Py_DECREF( obResult );

  obResult = PyObject_CallMethod( obStringIO, ( char* ) "getvalue", NULL );
  if ( obResult == NULL )
    TRACEBACK_FETCH_ERROR( "getvalue() failed." );

  /* And it should be a string all ready to go - duplicate it. */
  if ( !PyString_Check( obResult ) )
    TRACEBACK_FETCH_ERROR( "getvalue() did not return a string" );

  result = PyString_AsString( obResult );

done:

  // All finished - first see if we encountered an error
  if ( result.isEmpty() && !errMsg.isEmpty() )
  {
    result = errMsg;
  }

  Py_XDECREF( modStringIO );
  Py_XDECREF( modTB );
  Py_XDECREF( obStringIO );
  Py_XDECREF( obResult );
  Py_XDECREF( value );
  Py_XDECREF( traceback );
  Py_XDECREF( type );

  return result;
}

QString QgsPythonUtilsImpl::getTypeAsString( PyObject* obj )
{
  if ( obj == NULL )
    return NULL;

  if ( PyClass_Check( obj ) )
  {
    QgsDebugMsg( "got class" );
    return QString( PyString_AsString((( PyClassObject* )obj )->cl_name ) );
  }
  else if ( PyType_Check( obj ) )
  {
    QgsDebugMsg( "got type" );
    return QString((( PyTypeObject* )obj )->tp_name );
  }
  else
  {
    QgsDebugMsg( "got object" );
    return PyObjectToQString( obj );
  }
}

bool QgsPythonUtilsImpl::getError( QString& errorClassName, QString& errorText )
{
  if ( !PyErr_Occurred() )
    return false;

  PyObject* err_type;
  PyObject* err_value;
  PyObject* err_tb;

  // get the exception information and clear error
  PyErr_Fetch( &err_type, &err_value, &err_tb );

  // get exception's class name
  errorClassName = getTypeAsString( err_type );

  // get exception's text
  if ( err_value != NULL && err_value != Py_None )
  {
    errorText = PyObjectToQString( err_value );
  }
  else
    errorText.clear();

  // cleanup
  Py_XDECREF( err_type );
  Py_XDECREF( err_value );
  Py_XDECREF( err_tb );

  return true;
}

QString QgsPythonUtilsImpl::getResult()
{
  return getVariableFromMain( "__result" );
}

QString QgsPythonUtilsImpl::PyObjectToQString( PyObject* obj )
{
  QString result;

  // is it None?
  if ( obj == Py_None )
  {
    return QString();
  }

  // check whether the object is already a unicode string
  if ( PyUnicode_Check( obj ) )
  {
    PyObject* utf8 = PyUnicode_AsUTF8String( obj );
    if ( utf8 )
      result = QString::fromUtf8( PyString_AS_STRING( utf8 ) );
    else
      result = "(qgis error)";
    Py_XDECREF( utf8 );
    return result;
  }

  // check whether the object is a classical (8-bit) string
  if ( PyString_Check( obj ) )
  {
    return QString::fromUtf8( PyString_AS_STRING( obj ) );
  }

  // it's some other type of object:
  // convert object to unicode string (equivalent to calling unicode(obj) )

  PyObject* obj_uni = PyObject_Unicode( obj ); // obj_uni is new reference
  if ( obj_uni )
  {
    // get utf-8 representation of unicode string (new reference)
    PyObject* obj_utf8 = PyUnicode_AsUTF8String( obj_uni );
    // convert from utf-8 to QString
    if ( obj_utf8 )
      result = QString::fromUtf8( PyString_AsString( obj_utf8 ) );
    else
      result = "(qgis error)";
    Py_XDECREF( obj_utf8 );
    Py_XDECREF( obj_uni );
    return result;
  }

  // if conversion to unicode failed, try to convert it to classic string, i.e. str(obj)
  PyObject* obj_str = PyObject_Str( obj ); // new reference
  if ( obj_str )
  {
    result = QString::fromUtf8( PyString_AS_STRING( obj_str ) );
    Py_XDECREF( obj_str );
    return result;
  }

  // some problem with conversion to unicode string
  QgsDebugMsg( "unable to convert PyObject to a QString!" );
  return "(qgis error)";
}

QString QgsPythonUtilsImpl::getVariableFromMain( QString name )
{
  PyObject* obj;
  QString output;

  // get the result
  obj = PyDict_GetItemString( mMainDict, name.toUtf8() ); // obj is borrowed reference

  if ( obj != NULL && obj != Py_None )
  {
    output = PyObjectToQString( obj );
  }

  // erase result
  PyDict_SetItemString( mMainDict, name.toUtf8(), Py_None );

  return output;
}

bool QgsPythonUtilsImpl::evalString( const QString& command, QString& result )
{
  PyObject* res = PyRun_String( command.toUtf8().data(), Py_eval_input, mMainDict, mMainDict );

  if ( res != NULL )
  {
    result = PyObjectToQString( res );
    return true;
  }
  return false;
}


QString QgsPythonUtilsImpl::pythonPath()
{
  return QgsApplication::pkgDataPath() + "/python";
}

QString QgsPythonUtilsImpl::pluginsPath()
{
  return pythonPath() + "/plugins";
}

QString QgsPythonUtilsImpl::homePluginsPath()
{
  return QgsApplication::qgisSettingsDirPath() + "/python/plugins";
}

QStringList QgsPythonUtilsImpl::pluginList()
{
  QDir pluginDir( QgsPythonUtilsImpl::pluginsPath(), "*",
                  QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoDotAndDotDot );

  QDir homePluginDir( QgsPythonUtilsImpl::homePluginsPath(), "*",
                      QDir::Name | QDir::IgnoreCase, QDir::Dirs | QDir::NoDotAndDotDot );

  QStringList pluginList = pluginDir.entryList();

  for ( uint i = 0; i < homePluginDir.count(); i++ )
  {
    QString packageName = homePluginDir[i];
    if ( !pluginList.contains( packageName ) )
      pluginList.append( packageName );

  }

  return pluginList;
}

QString QgsPythonUtilsImpl::getPluginMetadata( QString pluginName, QString function )
{
  QString command = pluginName + "." + function + "()";
  QString retval = "???";

  // temporary disable error hook - UI will handle this gracefully
  uninstallErrorHook();
  PyObject* obj = PyRun_String( command.toUtf8().data(), Py_eval_input, mMainDict, mMainDict );

  if ( PyErr_Occurred() )
  {
    PyErr_Print(); // just print it to console
    PyErr_Clear();
    retval = "__error__";
  }
  else if ( PyUnicode_Check( obj ) )
  {
    PyObject* utf8 = PyUnicode_AsUTF8String( obj );
    if ( utf8 )
      retval = QString::fromUtf8( PyString_AS_STRING( utf8 ) );
    else
      retval = "__error__";
    Py_XDECREF( utf8 );
  }
  else if ( PyString_Check( obj ) )
  {
    retval = PyString_AS_STRING( obj );
  }
  else
  {
    // bad python return value
    retval = "__error__";
  }
  Py_XDECREF( obj );

  installErrorHook();
  return retval;
}


bool QgsPythonUtilsImpl::loadPlugin( QString packageName )
{
  // load plugin's package and ensure that plugin is reloaded when changed
  runString(
    "try:\n"
    "  import " + packageName + "\n"
    "  __main__.__plugin_result = 'OK'\n"
    "except:\n"
    "  __main__.__plugin_result = 'ERROR'\n" );

  if ( getVariableFromMain( "__plugin_result" ) == "OK" )
    return true;

  // snake in the grass, we know it's there
  runString( "sys.path_importer_cache.clear()" );

  // retry
  runString(
    "try:\n"
    "  import " + packageName + "\n"
    "  reload(" + packageName + ")\n"
    "  __main__.__plugin_result = 'OK'\n"
    "except:\n"
    "  qgis_except_hook_msg(sys.exc_type, sys.exc_value, sys.exc_traceback, "
    "'Couldn\\'t load plugin \"" + packageName + "\" from [\\'' + '\\', \\''.join(sys.path) + '\\']')\n"
    "  __main__.__plugin_result = 'ERROR'\n" );

  return getVariableFromMain( "__plugin_result" ) == "OK";
}


bool QgsPythonUtilsImpl::startPlugin( QString packageName )
{
  QString pluginPythonVar = "plugins['" + packageName + "']";

  QString errMsg = QObject::tr( "Couldn't load plugin %1" ).arg( packageName );

  // create an instance of the plugin
  if ( !runString( pluginPythonVar + " = " + packageName + ".classFactory(iface)",
                   QObject::tr( "%1 due an error when calling its classFactory() method" ).arg( errMsg ) ) )
    return false;

  // initGui
  if ( !runString( pluginPythonVar + ".initGui()",
                   QObject::tr( "%1 due an error when calling its initGui() method" ).arg( errMsg ) ) )
    return false;

  return true;
}


bool QgsPythonUtilsImpl::unloadPlugin( QString packageName )
{
  // unload and delete plugin!
  QString varName = "plugins['" + packageName + "']";

  QString errMsg = QObject::tr( "Error while unloading plugin %1" ).arg( packageName );

  if ( !runString( varName + ".unload()", errMsg ) )
    return false;
  if ( !runString( "del " + varName, errMsg ) )
    return false;

  return true;
}
