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

#include "qgis.h"
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
  runString( "import os" ); // import os module (for user paths)

  // construct a list of plugin paths
  // plugin dirs passed in QGIS_PLUGINPATH env. variable have highest priority (usually empty)
  // locally installed plugins have priority over the system plugins
  QStringList pluginpaths = extraPluginsPaths();
  pluginpaths << homePluginsPath() << pluginsPath();

  // expect that bindings are installed locally, so add the path to modules
  // also add path to plugins
  QStringList newpaths;
  newpaths << pythonPath() << homePythonPath();
  newpaths += pluginpaths;
  runString( "sys.path = [\"" + newpaths.join( "\", \"" ) + "\"] + sys.path" );

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

  // import QGIS utils
  error_msg = QObject::tr( "Couldn't load QGIS utils." ) + "\n" + QObject::tr( "Python support will be disabled." );
  if ( !runString( "import qgis.utils", error_msg ) )
  {
    exitPython();
    return;
  }

  // tell the utils script where to look for the plugins
  runString( "qgis.utils.plugin_paths = [\"" + pluginpaths.join( "\",\"" ) + "\"]" );

  // initialize 'iface' object
  runString( "qgis.utils.initInterface(" + QString::number(( unsigned long ) interface ) + ")" );
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
  runString( "qgis.utils.installErrorHook()" );
}

void QgsPythonUtilsImpl::uninstallErrorHook()
{
  runString( "qgis.utils.uninstallErrorHook()" );
}



bool QgsPythonUtilsImpl::runStringUnsafe( const QString& command, bool single )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  // TODO: convert special characters from unicode strings u"..." to \uXXXX
  // so that they're not mangled to utf-8
  // (non-unicode strings can be mangled)
  PyRun_String( command.toUtf8().data(), single ? Py_single_input : Py_file_input, mMainDict, mMainDict );

  bool res = ( PyErr_Occurred() == 0 );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return res;
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

  // TODO: use python implementation

  QString traceback = getTraceback();
  QString path, version;
  evalString( "str(sys.path)", path );
  evalString( "sys.version", version );

  QString str = "<font color=\"red\">" + msgOnError + "</font><br><pre>\n" + traceback + "\n</pre>"
                + QObject::tr( "Python version:" ) + "<br>" + version + "<br><br>"
                + QObject::tr( "QGIS version:" ) + "<br>" + QString( "%1 '%2', %3" ).arg( QGis::QGIS_VERSION ).arg( QGis::QGIS_RELEASE_NAME ).arg( QGis::QGIS_DEV_VERSION ) + "<br><br>"
                + QObject::tr( "Python path:" ) + "<br>" + path;
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

  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

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

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

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
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  if ( !PyErr_Occurred() )
  {
    PyGILState_Release( gstate );
    return false;
  }

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

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return true;
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


bool QgsPythonUtilsImpl::evalString( const QString& command, QString& result )
{
  // acquire global interpreter lock to ensure we are in a consistent state
  PyGILState_STATE gstate;
  gstate = PyGILState_Ensure();

  PyObject* res = PyRun_String( command.toUtf8().data(), Py_eval_input, mMainDict, mMainDict );
  bool success = ( res != NULL );

  // TODO: error handling

  if ( success )
    result = PyObjectToQString( res );

  // we are done calling python API, release global interpreter lock
  PyGILState_Release( gstate );

  return success;
}


QString QgsPythonUtilsImpl::pythonPath()
{
  return QgsApplication::pkgDataPath() + "/python";
}

QString QgsPythonUtilsImpl::pluginsPath()
{
  return pythonPath() + "/plugins";
}

QString QgsPythonUtilsImpl::homePythonPath()
{
  return QgsApplication::qgisSettingsDirPath() + "python";
}

QString QgsPythonUtilsImpl::homePluginsPath()
{
  return homePythonPath() + "/plugins";
}

QStringList QgsPythonUtilsImpl::extraPluginsPaths()
{
  const char* cpaths = getenv( "QGIS_PLUGINPATH" );
  if ( cpaths == NULL )
    return QStringList();

  QString paths = QString::fromLocal8Bit( cpaths );
#ifndef Q_OS_WIN
  if ( paths.contains( ':' ) )
    return paths.split( ':', QString::SkipEmptyParts );
#endif
  if ( paths.contains( ';' ) )
    return paths.split( ';', QString::SkipEmptyParts );
  else
    return QStringList( paths );
}


QStringList QgsPythonUtilsImpl::pluginList()
{
  runString( "qgis.utils.updateAvailablePlugins()" );

  QString output;
  evalString( "'\\n'.join(qgis.utils.available_plugins)", output );
  return output.split( QChar( '\n' ) );
}

QString QgsPythonUtilsImpl::getPluginMetadata( QString pluginName, QString function )
{
  QString res;
  QString str = "qgis.utils.pluginMetadata('" + pluginName + "', '" + function + "')";
  evalString( str, res );
  //QgsDebugMsg("metadata "+pluginName+" - '"+function+"' = "+res);
  return res;
}

bool QgsPythonUtilsImpl::loadPlugin( QString packageName )
{
  QString output;
  evalString( "qgis.utils.loadPlugin('" + packageName + "')", output );
  return ( output == "True" );
}

bool QgsPythonUtilsImpl::startPlugin( QString packageName )
{
  QString output;
  evalString( "qgis.utils.startPlugin('" + packageName + "')", output );
  return ( output == "True" );
}

bool QgsPythonUtilsImpl::canUninstallPlugin( QString packageName )
{
  QString output;
  evalString( "qgis.utils.canUninstallPlugin('" + packageName + "')", output );
  return ( output == "True" );
}

bool QgsPythonUtilsImpl::unloadPlugin( QString packageName )
{
  QString output;
  evalString( "qgis.utils.unloadPlugin('" + packageName + "')", output );
  return ( output == "True" );
}

bool QgsPythonUtilsImpl::isPluginLoaded( QString packageName )
{
  QString output;
  evalString( "qgis.utils.isPluginLoaded('" + packageName + "')", output );
  return ( output == "True" );
}

QStringList QgsPythonUtilsImpl::listActivePlugins()
{
  QString output;
  evalString( "'\\n'.join(qgis.utils.active_plugins)", output );
  return output.split( QChar( '\n' ) );
}
